#include <stdlib.h>
#include <stdio.h>
#include "nlabparser2.h"

/** Global variables those are used in parsing */
int currentIdx = -1;
int errorIdx = -1;
int errorCode = 0;
Function *returnFunction = NULL;
NMAST *returnAst = NULL;

/**
	access right: public
*/	
AST parseFunctionExpression(TokenList *tokens){
	AST fd = null;
	if(currentToken.getType() == NAME && 
			(tokens.get(currentIdx + 1).getType()==LPAREN)){
		fd = functionNotation();
		if( error>=0 ) return null;
			
		if(currentToken.getType() == EQ){
			currentIdx++;
			currentToken = tokens.get(currentIdx);
			if(expression(currentIdx)>currentIdx)
				fd.addChild(returnedAst);
			//parseDomain();
		} else {
			//Exception;
			error = currentToken.getColumn();
		}
	}
	return fd;
}

/**
	access right: public
*/	
int parseExpression() {
	return 0;
}
/**
	functionNotation: NAME LPAREN NAME (COMA NAME)* PRARENT;
	@return if
*/
int functionNotation(TokenList *tokens, int index){
	int i, k, varsize = 0;
	int oldIndex = index;
	Token *next = NULL;
	NMAST *functionDefNode = NULL;
	char vars[4];

	if(index >= tokens->size)
		return index;
	
	errorCode = ERROR_NOT_A_FUNCTION;
	if(tokens->list[index]->type == NAME ){
		errorCode = ERROR_PARENTHESE_MISSING;
		errorIdx = tokens->list[index]->column;
		if(tokens->list[index+1]->type == LPAREN){
			errorCode = ERROR_MISSING_VARIABLE;
			errorIdx = tokens->list[index+1]->column;
			if(tokens->list[index+2]->type == NAME){
				vars[varsize++] = (tokens->list[index+2])->text[0];
				index += 3;
				while( (index+1<tokens->size) && (tokens->list[index]->type == COMMA)
							&& (tokens->list[index+1]->type == NAME || tokens->list[index+1]->type == VARIABLE ) ){
					vars[varsize++] = (tokens->list[index+1])->text[0];
					index += 2;
				}

				errorCode = ERROR_PARENTHESE_MISSING;
				errorIdx = tokens->list[index]->column;
				if( (index<tokens->size) && (tokens->list[index]->type == RPAREN)){
					returnFunction = (Function*)malloc(sizeof(Function));
					//Should use memcpy here
					for(i=0;i<varsize;i++)
						returnFunction->variable[i] = vars[i];
						
					errorCode = NO_ERROR;
					errorIdx = -1;
					return (index + 1);
				}
			}
		}
	}
	errorIdx = tokens->list[index-2]->column;
	return oldIndex;
}//done

NMAST* popASTStack(NMASTList *sk){
	NMAST* ele;
	if(sk == NULL || sk->size == 0)
		return NULL;
	
	ele = sk->list[sk->size-1];
	(sk->size)--;
	
	return ele;
}

void pushASTStack(NMASTList *sk, NMAST* ele){
	//NMAST **lst;
	if(sk == NULL)
		return;
		
	if(sk->size == sk->loggedSize){
		sk->loggedSize += INCLEN;
		/**
			IMPORTANT: this is maybe not secure here
		*/
		sk->list = (NMAST*)realloc(sk->list, sizeof(NMAST) * sk->loggedSize);
		
		//if(lst != NULL)
		//	sk->list = lst;
	}
	sk->list[sk->size] = ele;
	(sk->size)++;
}

/**
 * expression: LPAREN? expressionWithoutParenthese (( + | - | * | / | ^) expressionWithoutParenthese)* LPAREN?
 * @param index
 * @return
 */
int expression(int index){
	int k, l;
	int oldIndex = index;
	boolean fParen = false;
	NMASTList rs = (NMASTList*)malloc(sizeof(NMASTList));
	NMASTList sk = (NMASTList*)malloc(sizeof(NMASTList));
	AST itm, op1, op2;
	
	Token *tk = tokens->list[index];
		
	if(tk.getType() == Token.LPAREN){
		addToTokenStack(tk);
		index++;
	}
		
	returnedAst = NULL;
	if( (k = expressionWithoutParenthese(index)) > index){
		rs.add(returnedAst);
		if( k < tokens->size ) {
			tk = tokens->list[k];
		}else{
			//If we got a RPAREN here, maybe it belong to parent rule
			return k;
		}
			
		while(tk != NULL){
			if(setNumericOperators.contains(tk) 
					&& (  (l = expressionWithoutParenthese(k+1)) > (k+1) ) ){
				//Operator
				op1 = (NMAST*)malloc(sizeof(NMAST));
				op1->type =  tk->type;
				op1->sign =  1;
				op1->variable =  0;
				op1->parent = op1->left = op1->right;
				op1->value = 0.0;
				op1->valueType = TYPE_FLOATING_POINT;
				
				while( sk->size>0 && (sk->list[sk->size-1]->priority >= op1->priority )) {
					//pop
					op2 = popASTStack(sk);
					rs.add(op2);
				}
				pushASTStack(sk, op1);
					
				//expressionWithoutParenthese
				pushASTStack(rs, returnedAst);
				k = l;
				tk = tokens->list[k];
			}else if(tk.getType() == RPAREN){
				/**
				 * it's is possible that this RPAREN comes from parent rule
				 */
				if(fParen)
					k++;
					
				tk = null;
			}else{
				if(fParen) //match )
					return oldIndex;
						
				tk = null;
			}
		}
			
		while(!sk.isEmpty()){
			rs.add(sk.pop());
		}
			
		if(rs.size() > 0){
			//Build tree from infix
			while(rs.size() > 1){
				l = 2;
				itm = rs.get(l);
				while(!setNumericOperators.contains(itm.getType())){
					l++;
					itm = rs.get(l);
				}
				AST operand1 = rs.get(l-2);
				AST operand2 = rs.get(l-1);
				rs.remove(operand1);
				rs.remove(operand2);
				itm.addChild(operand1);
				itm.addChild(operand2);
			}
			if(rs.get(0).getChildrent()!=null && rs.get(0).getChildrent().size()>0){
				if(rs.get(0).getChildrent().size()==1)
					returnedAst = rs.get(0).getChildrent().get(0);
				else{
					returnedAst = new AST(AbstractAST.EXPRESSION, "EXPRESSION",rs.get(0).getColumn());
					returnedAst.addChild(rs.get(0));
				}
			}else
				returnedAst = rs.get(0);
		}
			
		return k;
	}
	return oldIndex;
}
	
/**
 * expressionWithoutParenthese: LPAREN? parseExpressionElement (( + | - | * | / | ^) parseExpressionElement)* LPAREN?
 * TODO: fix this
 * @param index
 * @return
 */
int expressionWithoutParenthese(int index) {
	int k, l;
	int oldIndex = index;
	boolean fParen = false;
	List<AST> rs = new ArrayList<AST>();
	Stack<AST> sk = new Stack<AST>();
	AST itm, op1, op2;
	Token tk = tokens.get(index);
		
	if(tk.getType() == Token.LPAREN){
		fParen = true;
		index++;
	}
		
	returnedAst = null;
	if( (k = expressionElement(index)) > index){
		rs.add(returnedAst);
		//k++;
		if( k < tokens.size() ) {
			tk = tokens.get(k);
		}else{
			if(fParen)
				return oldIndex; // << missing RPAREN
					
			return k;
		}
			
		while(tk != null){
			if(setNumericOperators.contains(tk) 
					&& (  (l = expressionElement(k+1)) > (k+1) ) ){
				//Operator
				op1 = new AST(tk.getType(), tk);
				while( !sk.isEmpty() && (sk.peek().getPriority() >= op1.getPriority())) {
					op2 = sk.pop();
					rs.add(op2);
				}
				sk.push(op1);
					
				//expressionElement
				rs.add(returnedAst);
				k = l;
				tk = null;
				if(k < tokens.size())
					tk = tokens.get(k);
					
			}else if(tk.getType() == Token.RPAREN){ //match )
				if(fParen) 
					k++;
					
				tk = null;
			}else{ // NOT match )
				if(fParen)
					return oldIndex;
						
				tk = null;
			}
		}
			
		while(!sk.isEmpty()){
			rs.add(sk.pop());
		}
			
		if(rs.size() > 0){
			//Build tree from infix
			while(rs.size() > 1){
				l = 2;
				itm = rs.get(l);
				while(!setNumericOperators.contains(itm.getType())){
					l++;
					itm = rs.get(l);
				}
				AST operand1 = rs.get(l-2);
				AST operand2 = rs.get(l-1);
				rs.remove(operand1);
				rs.remove(operand2);
				itm.addChild(operand1);
				itm.addChild(operand2);
			}
			if(rs.get(0).getChildrent()!=null && rs.get(0).getChildrent().size()>0){
				if(rs.get(0).getChildrent().size()==1)
					returnedAst = rs.get(0).getChildrent().get(0);
				else{
					returnedAst = new AST(AbstractAST.EXPRESSION, "EXPRESSION",rs.get(0).getColumn());
					returnedAst.addChild(rs.get(0));
				}
			}else
				returnedAst = rs.get(0);
		}
		
		return k;
	}
	return oldIndex;
}
	
/**
 * TODO: implement this routine
 * expressionElement: NUMBER | VARIABLE | CONSTANT | FUNCTION_CALL
 * @return
 */
int expressionElement(int index) {
	int k;
	Token tk = tokens.get(index);
	returnedAst = null;
	if(tk.getType() == Token.NUMBER || tk.getType() == Token.VARIABLE || tk.getType() == Token.NAME || 
			tk.getType() == Token.CONSTANT){
		returnedAst = new AST(tk.getType(), tk.getText(), tk.getColumn());
		return (index + 1);
	}else if( (k = functionCall(index)) > index ){
		return k;
	}
	return index;
}
	
	/**
	 * functionCall: NAME LPAREN expression RPARENT
	 * @return
	 */
	public AST parseFunctionCall(){
		int k;
		stackCheckParen.clear();
		if( (k=functionCall(currentIdx))>currentIdx){
			currentIdx = k+1;
			currentToken = null;
			if(currentIdx < tokens.size())
				currentToken = tokens.get(currentIdx);
			return returnedAst;
		}
		return null;
	}
	
	/**
	 * 
	 * @param index the start token index where we want to check if its a function call
	 * @return index if its not a function call or the last token index of the function call
	 */
	private int functionCall(int index){
		int k;
		Token tk = tokens.get(index);
		AST f;
		if( /*(tk.getType() == Token.NAME) &&*/ setFunctionNames.contains(tk)){
			if( tokens.get(index + 1).getType() == Token.LPAREN ){
				if( (k=expression(index + 2)) > (index+2)){
					if(k<tokens.size() && tokens.get(k).getType() == Token.RPAREN){
						if(checkParenthese(0, tokens.size()-1)){ 		//if(checkParenthese(index, k)){
							f = new AST(Token.FUNCTION_CALL, tk);
							f.addChild(returnedAst);
							returnedAst = f;
							return k+1;
						}else
							error = tokens.get(index + 1).getColumn();
					}else
						error = tokens.get(index + 1).getColumn();
				}
			}
		}
		//error = tokens.get(index + 1).getColumn();
		return index;
	}
	
	/**
	 * 
	 * @return
	 */
	private int interval(int idx){
		int nextIdx;
		if( (nextIdx = intervalWithBoundaries(idx)) > idx ){
			return nextIdx;
		}else if( (nextIdx = intervalElementOf(idx)) > idx){
			return nextIdx;
		}else if( (nextIdx = simpleInterval(idx)) > idx){
			return nextIdx;
		}
		return idx;
	}
	
	/**
	 * simpleInterval : LPAREN? NAME (LT | LTE | GT | GTE) NUMBER RPARENT?
	 * @param idx
	 * @return
	 */
	private int simpleInterval(int idx){
		Token tk = tokens.get(idx);
		int oldIdx = idx;
		boolean hasParenthese = false;
		if(tk.getType() == AbstractAST.LPAREN){
			hasParenthese = true;
			idx++;
			tk = tokens.get(idx);
		}
		
		if(tk.getType() == AbstractAST.NAME){
			if(tokens.get(idx+1).getType()==Token.LT ||
					tokens.get(idx+1).getType()==Token.LTE || tokens.get(idx+1).getType()==Token.GT ||
					tokens.get(idx+1).getType()==Token.GTE) {
				if(tokens.get(idx+2).getType() == Token.NUMBER) {
					
					if(hasParenthese) {
						if(tokens.get(idx + 3).getType() != AbstractAST.RPAREN){
							return oldIdx;
						}
						idx++;
					}
					
					returnedAst = new AST(Token.INTERVAL, "INTERVAL", tk.getColumn());
					returnedAst.addChild(new AST(Token.NAME, tk));
					returnedAst.addChild(new AST(tokens.get(idx+1).getType(), tokens.get(idx+1)));
					AST rightBound = new AST(tokens.get(idx+2).getType(), tokens.get(idx+2).getText(), tokens.get(idx+2).getColumn());
					returnedAst.addChild(rightBound);
					return (idx+3);
				}
			}
		}
		
		return idx;
	}
	
	/**
	 * intervalElementOf: NAME ELEMENT_OF (LPAREN | LPRACKET) NUMBER COMMA NUMBER (RPAREN | RPRACKET);
	 * @param idx
	 * @return
	 */
	private int intervalElementOf(int idx){
		Token tk = tokens.get(idx);
		returnedAst = null;
		if(tk.getType() == AbstractAST.NAME){
			Token tokenK1 = tokens.get(idx + 1);
			if(tokenK1.getType()==Token.ELEMENT_OF) {
				Token tokenK2 = tokens.get(idx + 2);
				if(tokenK2.getType() == Token.LPAREN
							|| tokenK2.getType() == Token.LPRACKET ){
					Token tokenK3 = tokens.get(idx + 3);
					if(tokenK3.getType() == Token.NUMBER){
						Token tokenK4 = tokens.get(idx + 4);
						if(tokenK4.getType() == Token.COMMA){
							Token tokenK5 = tokens.get(currentIdx + 5);
							if(tokenK5.getType() == Token.NUMBER){
								Token tokenK6 = tokens.get(idx + 6);
								if(tokenK6.getType() == Token.RPAREN
													|| tokenK6.getType() == Token.RPRACKET ){
									returnedAst = new AST(Token.INTERVAL, "INTERVAL", tk.getColumn());
									//TODO: Something not clear here
									returnedAst.addChild(new AST(Token.NAME, tk));
									AST leftBound = new AST(tokenK2.getType(), tokenK3.getText(), tokenK3.getColumn());
									returnedAst.addChild(leftBound);
									AST rightBound = new AST(tokenK6.getType(), tokenK5.getText(), tokenK5.getColumn());
									returnedAst.addChild(rightBound);
									return (idx + 7);
								}
							}
						}
					}
				}
			}
		}
		return idx;
	}
	
	/**
	 * intervalWithLowerAndUpper: LPAREN? NUMBER (LT | LTE | GT | GTE) NAME (LT | LTE | GT | GTE) NUMBER RPARENT? ;
	 * @return
	 */
	private int intervalWithBoundaries(int idx){
		Token tk = tokens.get(idx);
		boolean hasParenthese = false;
		String op1Text="", textTypeName = "";
		int oldIdx = idx;
		if(tk.getType() == AbstractAST.LPAREN){
			hasParenthese = true;
			idx++;
			tk = tokens.get(idx);
		}
		
		if(tk.getType() == AbstractAST.NUMBER){
			Token tokenK1 = tokens.get(idx + 1);
			if(setComparationOperators.contains(tokenK1)){
				Token tokenK2 = tokens.get(idx + 2);
				if(tokenK2.getType() == AbstractAST.NAME){
					Token tokenK3 = tokens.get(idx + 3);
					if(setComparationOperators.contains(tokenK3)){
						Token tokenK4 = tokens.get(idx + 4);
						if(tokenK4.getType() == AbstractAST.NUMBER){
						
							if(hasParenthese) {
								if(tokens.get(idx + 5).getType() != AbstractAST.RPAREN){
									return oldIdx;
								}
								idx++;
							}
							
							/**
								The reason for doing this is that we always
								read the comparation in the order that 
								variable comes first.
								Example: -1 < x < 3
									x is greater than -1 and x is less than 3
							*/
							if("LT".equals(tokenK1.getTypeName()))
								op1Text = "GT";
							else if("GT".equals(tokenK1.getTypeName()))
								op1Text = "LT";
							else if("LTE".equals(tokenK1.getTypeName()))
								op1Text = "GTE";
							else if("GTE".equals(tokenK1.getTypeName()))
								op1Text = "LTE";
								
							textTypeName = op1Text + "_" + tokenK3.getTypeName();
							returnedAst = new AST(AbstractAST.INTERVAL, textTypeName, tk.getColumn());
							AST nameNode = new AST(tokenK2.getType(), tokenK2.getText(), tokenK2.getColumn());
							returnedAst.addChild(nameNode);
							AST lBound = new AST(tk.getType(), tk.getText(), tk.getColumn());
							returnedAst.addChild(lBound);
							AST rBound = new AST(tokenK4.getType(), tokenK4.getText(), tokenK4.getColumn());
							returnedAst.addChild(rBound);
							
							return idx + 5;
						}
					}
				}
			}
		}
		return oldIdx;
	}
	
	/**
	 * simple_domain: LPAREN? interval ( (OR | AND) interval)* RPAREN? ;
	 */
	private int simpleDomain(int index){
		int k, l;
		int oldIndex = index;
		List<AST> rs = new ArrayList<AST>();
		Stack<AST> sk = new Stack<AST>();
		AST itm, op1, op2;
		Token tk = tokens.get(index);
		
		if(tk.getType() == Token.LPAREN){
			stackCheckParen.push(tk);
			index++;
		}
		
		returnedAst = null;
		if( (k=interval(index)) > index ){
			rs.add(returnedAst);
			if( k < tokens.size() ){
				tk = tokens.get(k);
			}else{
				return k;
			}
			
			while( tk != null ){
				if(((op1 = parseOR_AND()) != null) && ((l = interval(k+1))>(k+1))) {
					while((sk.peek().getType()==AbstractAST.OR||sk.peek().getType()==AbstractAST.AND)
							&&(sk.peek().getPriority() >= op1.getPriority())){
						op2 = sk.pop();
						rs.add(op2);
					}
					sk.push(op1);
					
					rs.add(returnedAst);
					k = l;
					tk = tokens.get(k);
				}else if(tk.getType() == Token.RPAREN){
					/**
					 * it's is possible that this RPAREN comes from parent rule
					 */
					if(stackCheckParen.isEmpty()){
						return oldIndex;
					}
					stackCheckParen.pop();
					k++;
					tk = null;
				}else{
					tk = null;
				}
			}
			
			while(!sk.isEmpty()){
				rs.add(sk.pop());
			}
			
			if(rs.size() > 0){
				//Build tree from infix
				while(rs.size() > 1){
					l = 2;
					itm = rs.get(l);
					while(itm.getType() != AbstractAST.OR && itm.getType() != AbstractAST.AND){
						l++;
						itm = rs.get(l);
					}
					AST operand1 = rs.get(l-2);
					AST operand2 = rs.get(l-1);
					rs.remove(operand1);
					rs.remove(operand2);
					itm.addChild(operand1);
					itm.addChild(operand2);
				}
				if(rs.get(0).getType() != AbstractAST.INTERVAL){
					if(rs.get(0).getChildrent().size()==1)
						returnedAst = rs.get(0).getChildrent().get(0);
					else{
						returnedAst = new AST(AbstractAST.DOMAIN, "DOMAIN",rs.get(0).getColumn());
						returnedAst.addChild(rs.get(0));
					}
				}else
					returnedAst = rs.get(0);
			}
			
			return k;
		}
		return oldIndex;
	}
	
	private AST parseOR_AND() {
		if(currentToken.getType() == Token.OR || 
				currentToken.getType() == Token.AND){
			AST ast = new AST(currentToken.getType(), 
									currentToken.getText(), currentToken.getColumn());
			currentIdx++;
			currentToken = null;
			if(currentIdx < tokens.size())
				currentToken = tokens.get(currentIdx);
			return ast;
		}
		return null;
	}
	
	public AST parseDomain(){
		int k;
		stackCheckParen.clear();
		if( (k=domain(currentIdx))>currentIdx){
			currentIdx = k;
			currentToken = null;
			if(currentIdx < tokens.size())
				currentToken = tokens.get(currentIdx);
			if(stackCheckParen.isEmpty())
				return returnedAst;
		}
		return null;
	}

	/**
	 * domain: simple_domain ( (OR | AND) simple_domain)* ;
	 */
	private int domain(int index){
		int k, l;
		int oldIndex = index;
		List<AST> rs = new ArrayList<AST>();
		Stack<AST> sk = new Stack<AST>();
		AST itm, op1, op2;
		Token tk = tokens.get(index);
		
		if(tk.getType() == Token.LPAREN){
			stackCheckParen.push(tk);
			index++;
		}
		
		returnedAst = null;
		if( (k=simpleDomain(index)) > index ){
			rs.add(returnedAst);
			if( k < tokens.size() ){
				tk = tokens.get(k);
			}else{
				return k;
			}
			
			while( tk != null ){
				if((tk.getType()==AbstractAST.OR||tk.getType()==AbstractAST.AND) && ((l = simpleDomain(k+1))>(k+1))) {
					while(!sk.isEmpty() && (sk.peek().getType()==AbstractAST.OR||sk.peek().getType()==AbstractAST.AND)
							&&(sk.peek().getPriority() >= tk.getPriority())){
						op2 = sk.pop();
						rs.add(op2);
					}
					sk.push(new AST(tk.getType(), tk));
					
					rs.add(returnedAst);
					k = l;
					tk = (k<tokens.size())?tokens.get(k):null;
				}else if(tk.getType() == Token.RPAREN){
					/**
					 * it's is possible that this RPAREN comes from parent rule
					 */
					if(stackCheckParen.isEmpty())
						return oldIndex; //<< missing LPAREN
					stackCheckParen.pop();
					k++;
					tk = null;
				}else{
					tk = null;
				}
			}
			
			while(!sk.isEmpty()){
				rs.add(sk.pop());
			}
			
			if(rs.size() > 0){
				//Build tree from infix
				while(rs.size() > 1){
					l = 2;
					itm = rs.get(l);
					while(itm.getType() != AbstractAST.OR && itm.getType() != AbstractAST.AND){
						l++;
						itm = rs.get(l);
					}
					AST operand1 = rs.get(l-2);
					AST operand2 = rs.get(l-1);
					rs.remove(operand1);
					rs.remove(operand2);
					itm.addChild(operand1);
					itm.addChild(operand2);
				}
				if(rs.get(0).getChildrent()!=null && rs.get(0).getChildrent().size()>0){
					if(rs.get(0).getChildrent().size()==1)
						returnedAst = rs.get(0).getChildrent().get(0);
					else{
						returnedAst = new AST(AbstractAST.DOMAIN, "DOMAIN",rs.get(0).getColumn());
						returnedAst.addChild(rs.get(0));
					}
				}else
					returnedAst = rs.get(0);
			}
			
			return k;
		}
		return oldIndex;
	}
	/**
	 *	A function notation is like f(x, y)
	 *
	 */
	public AST functionNotation(){
		int i, k;
		Token next = null;
		AST functionDefNode = null;
		
		for(i=currentIdx; i<tokens.size();i++){
			if( tokens.get(i).getType() == Token.EQ )
				break;
		}
		
		functionDefNode = new AST(Token.FUNCTION_NOTATION, currentToken);
		currentIdx++;
		currentToken = null;
		if(currentIdx < tokens.size())
			currentToken = tokens.get(currentIdx);
		else return null;
		
		if(currentToken.getType() == Token.LPAREN){
			currentIdx++;
			currentToken = tokens.get(currentIdx);
			if(currentToken.getType() == Token.VARIABLE || currentToken.getType() == Token.NAME){
				AST v = new AST(Token.VARIABLE, currentToken);
				functionDefNode.addChild(v);
				
				currentIdx++;
				currentToken = tokens.get(currentIdx);
				k = currentIdx + 1;
				next = tokens.get(k);
				while(true){
					if( (currentToken.getType() == Token.COMMA) && 
												( next.getType() == Token.VARIABLE || next.getType() == Token.NAME)	) {
						v = new AST(Token.VARIABLE, next);
						functionDefNode.addChild(v);
						
						currentIdx += 2;
						currentToken = tokens.get(currentIdx);
						k = currentIdx + 1;
						next = tokens.get(k);
					} else{
						if(currentToken.getType() == Token.RPAREN){
							currentIdx++;
							currentToken = tokens.get(currentIdx);
						}else{
							//Exception
							error = currentToken.getColumn();
						}
					
						return functionDefNode;
					}
				}
			}
		}
		
		return functionDefNode;
	}
	
	private boolean checkParenthese(int from, int to){
		int i;
		Token tk;
		stackCheckParen.clear();		
		for(i = from; i<=to; i++){
			tk = tokens.get(i);
			if(tk.getType() == Token.LPAREN)
				stackCheckParen.push(tk);
			else if(tk.getType() == Token.RPAREN){
				if(stackCheckParen.isEmpty() ||
						stackCheckParen.peek().getType() != Token.LPAREN) 
					return false; // << missing LPAREN
				stackCheckParen.pop();
			}
		}
		return stackCheckParen.isEmpty()?true:false;
	}

	public int getError() {
		return error;
	}
	
	public boolean hasError() {
		return error<0?false:true;
	}