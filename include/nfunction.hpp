#ifndef _NFUNCTION_H
#define _NFUNCTION_H

#include <iostream>
#include <vector>
#include <string>
#include "criteria.hpp"
#include "common_data.h"
#include "imagedata.hpp"
#include "nlablexer.h"
#include "nlabparser.hpp"
#include "compositecriteria.hpp"
#include "calculus.hpp"

namespace nmath {
	template <typename T>
	class NFunction {
	private:
		char *text;
		unsigned int textLen;

		std::vector<Token*> mTokens;
		std::vector<NMAST<T>* > prefix;
		std::vector<Criteria<T>*> criteria;
		std::vector<NMAST<T>* > variables;
		std::vector<string> strVars;

		int errorCode;
		int errorColumn;

        std::vector<ImageData<T>*> getSpaceFor2UnknownVariables(const T *inputInterval, T epsilon, bool needNormalVector) {
            std::vector<ImageData<T>*> lstData;
            ImageData<T> **tempList;
            ImageData<T> *sp;
            DParam<T> param;
            DParam<T> reduceParam;
            CompositeCriteria<T> *outCriteria;
            SimpleCriteria<T> *sc;
            NMAST<T> *df[2] = {0, 0};

            CompositeCriteria<T> cc;
            cc.setOperator(AND);
            sc = new SimpleCriteria<T>(GTE_LTE, variables[0]->text, inputInterval[0], inputInterval[1], false, false);
            cc.add(sc);
            sc = new SimpleCriteria<T>(GTE_LTE, variables[1]->text, inputInterval[2], inputInterval[3], false, false);
            cc.add(sc);

            for(int i=0; i<prefix.size(); i++) {
                if(needNormalVector) {
                    df[0] = getDerivativeByVariable(i, 0);
                    reduceParam.t = df[0];
                    reduce_t<T>((void*)&reduceParam);
                    df[0] = reduceParam.t;

                    df[1] = getDerivativeByVariable(i, 1);
                    reduceParam.t = df[1];
                    reduce_t<T>((void*)&reduceParam);
                    df[1] = reduceParam.t;
                }

                if(criteria[i] == nullptr) {
                    if (needNormalVector)
                        sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, &cc, df);
                    else
                        sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, &cc, nullptr);
                    lstData.push_back(sp);
                } else {
                    outCriteria = (CompositeCriteria<T>*)criteria[i]->getInterval(inputInterval, this->strVars);
                    switch(outCriteria->logicOperator()) {
                        case AND:
                            if (needNormalVector)
                                sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, outCriteria, df);
                            else
                                sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, outCriteria, 0);
                            lstData.push_back(sp);
                            break;

                        case OR:
                            for(int j=0; j<outCriteria->size(); j++) {
                                if(needNormalVector)
                                    sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, (CompositeCriteria<T>*)(outCriteria->get(j)), df);
                                else
                                    sp = getSpaceFor2WithANDComposite(i, inputInterval, epsilon, (CompositeCriteria<T>*)(outCriteria->get(j)), 0);
                                lstData.push_back(sp);
                            }
                            break;

                        default:
                            break;
                    }
                }

                //free derivative trees
                if(df[0] != 0) {
                    clearTree(df);
                }

                if(df[1] != 0) {
                    clearTree(df+1);
                }

            } //end for

            return lstData;
        }
        /**
         *
         * @param prefixIndex
         * @param inputInterval
         * @param epsilon
         * @param c the AND CompositeCriteria, because every SimpleCriteria holds criteria for a single variable
         * so the number of SimpleCriteria in c is most likely the number of variable this function.
         * Example: f(x,y) = sin(x) + y^2, x > 0 AND 0 < y < 0.5
         * @param df the derivative of this function, this is used for calculating normal vectors
         * @return an object of FData, if df is not null then the returned object consists of x, y, z
         */
        ImageData<T>* getSpaceFor2WithANDComposite(int prefixIndex, const T *inputInterval,
                                    T epsilon, const CompositeCriteria<T>* c, NMAST<T> **df) {
            ImageData<T> *sp;
            DParam<T> param;
            DParam<T> dparam0;
            DParam<T> dparam1;
            T min[2];
            T max[2];
            T z;
            int j, k, elementOnRow;
            SimpleCriteria<T> *sc;
            std::string currentVar;

            /*
                We want the normal vector to be a unit vector 
                so this value is used for normalizing normal vector
            */
            float mod;

            for(k=0; k<variables.size(); k++) {
                currentVar = variables[k]->text;
                //search for criteria that bounds the current variable
                for(j=0; j<c->size(); j++) {
                    sc = (SimpleCriteria<T>*)c->get(j);
                    if(sc->getVariable() == currentVar) {
                        min[k] = sc->getLeftValue();
                        if(sc->getType() == GT_LT || sc->getType() == GT_LTE)
                            min[k] = sc->getLeftValue() + epsilon;

                        max[k] = sc->getRightValue();
                        if(sc->getType() == GT_LT || sc->getType() == GTE_LT)
                            max[k] = sc->getRightValue() - epsilon;

                        break;
                    }
                }
            }

            /**
             * 2 Unknown variable means we are in three dimension space
             * for each point in the value domain, we need 3 values for x, y and z.
             * And if we need to calculate normal vector at each point, we have to add 3 values
             * to hold nx, ny and nz, this case we have 6 values per point
             */
            sp = new ImageData<T>(((df == nullptr)?3:6));

            param.t = prefix[prefixIndex];
            param.variables[0] = variables[0]->text;
            param.variables[1] = variables[1]->text;
            param.error = 0;

            param.values[0] = min[0];
            while(param.values[0] < max[0] ) {
                elementOnRow = 0;
                param.values[1] = min[1];
                while(param.values[1] < max[1]) {
                    calc_t<T>((void*)&param);
                    z = param.retv;

                    /*
                        This method is used for 2 unknown variables function only
                        so we know that we need 3 float for a point
                    */
                    sp->addData(param.values[0]); // OFFSET = 0
                    sp->addData(param.values[1]); // OFFSET = 1
                    sp->addData(z);               // OFFSET = 2

                    /******** Now, calculate the normal vector at x, y, z **************/
                    if(df != nullptr) {
                        dparam0.t = df[0];
                        dparam0.variables[0] = variables[0]->text;
                        dparam0.variables[1] = variables[1]->text;
                        dparam0.error = 0;
                        dparam0.values[0] = param.values[0];
                        dparam0.values[1] = param.values[1];
                        calc_t<T>((void*)&dparam0);

                        dparam1.t = df[1];
                        dparam1.variables[0] = variables[0]->text;
                        dparam1.variables[1] = variables[1]->text;
                        dparam1.error = 0;
                        dparam1.values[0] = param.values[0];
                        dparam1.values[1] = param.values[1];
                        calc_t<T>((void*)&dparam1);

                        mod = sqrt(dparam0.retv*dparam0.retv + dparam1.retv*dparam1.retv + 1);
                        // sp->data[sp->dataSize++] = dparam0.retv/mod;
                        // sp->data[sp->dataSize++] = dparam1.retv/mod;
                        // sp->data[sp->dataSize++] = -1/mod;
                        sp->addData(dparam0.retv/mod);
                        sp->addData(dparam1.retv/mod);
                        sp->addData(-1/mod);
                    }
                    /*******************************************/

                    elementOnRow++;
                    param.values[1] += epsilon;
                }

                sp->addRow(elementOnRow);
                param.values[0] += epsilon;
            }
            sp->calculateVertexCount();
            if(df != nullptr) {
                sp->setNormalOffset(3);
            }

            return sp;
        }

	public:
		NFunction(): text(nullptr), textLen(0), errorCode(0), errorColumn(-1) {
		}

		~NFunction() {
			release();
		}
		
		int getErrorCode() { return errorCode; }
		int getErrorColumn() { return errorColumn; }


        int toString(char *str, int buflen) {
            int start = 0;
            nmath::toString(prefix[0], str, &start, buflen);
            return start;
        }

        int parse(const char *str, unsigned int len, NLabLexer *mLexer, NLabParser<T> *mParser) {
            int i;
            Criteria<T> *c;
            CompositeCriteria<T> *cc;
            std::vector<NMAST<T>* > domain;

            delete[] text;

            text = new char[len+1];
            std::memcpy(text, str, len);
            textLen = len;
            text[len] = '\0';

            mLexer->lexicalAnalysis(text, textLen, false, 0, mTokens, nullptr);
            errorColumn = mLexer->getErrorColumn();
            errorCode = mLexer->getErrorCode();
            if (errorCode != NMATH_NO_ERROR) {
                return errorCode;
            }

            //TODO: Need to release prefix and domain before call parseFunctionExpression from NLabParser
            mParser->parseFunctionExpression(mTokens, prefix,
                                             domain, variables, &errorCode, &errorColumn);

            if (errorCode == NMATH_NO_ERROR) {

                for (i = 0; i < domain.size(); i++) {
                    c = nullptr;
                    if (domain[i] != nullptr) {
                        c = nmath::buildCriteria(domain[i]);
                        //atemp to normalize criteria so that it hold criteria for all variable in every it's element
                        if ( (c!=nullptr) && c->getCClassType() == COMPOSITE &&
                             ((CompositeCriteria<T>*)c)->logicOperator() == OR) {
                            cc = (CompositeCriteria<T>*)c;
                            cc->normalize(variables);
                        }
                    }
                    criteria.push_back(c);
                }
            }


            for(i=0; i<domain.size(); i++) {
                nmath::clearTree(&(domain[i]));
            }
            domain.clear();

            return errorCode;
        }

        int parse(std::string str, NLabLexer *mLexer, NLabParser<T> *mParser) {
            int i;
            Criteria<T> *c;
            CompositeCriteria<T> *cc;
            std::vector<NMAST<T>* > domain;

            delete[] text;
            size_t len = str.length() + 1;

            text = new char[len];
            std::memcpy(text, str.c_str(), str.length());
            textLen = str.length();
            text[textLen] = '\0';

            mLexer->lexicalAnalysis(text, textLen, false, 0, mTokens, nullptr);
            errorColumn = mLexer->getErrorColumn();
            errorCode = mLexer->getErrorCode();
            if (errorCode != NMATH_NO_ERROR) {
                return errorCode;
            }

            //TODO: Need to release prefix and domain to free up memory
            //before calling parseFunctionExpression from NLabParser
            mParser->parseFunctionExpression(mTokens, prefix,
                                             domain, variables, &errorCode, &errorColumn);
            if (errorCode == NMATH_NO_ERROR) {
                for (i = 0; i < domain.size(); i++) {
                    c = nullptr;
                    if (domain[i] != nullptr) {
                        c = nmath::buildCriteria(domain[i]);
                        //atemp to normalize criteria so that it hold criteria for all variable in every it's element
                        if ( (c!=nullptr) && c->getCClassType() == COMPOSITE &&
                             ((CompositeCriteria<T>*)c)->logicOperator() == OR) {
                            cc = (CompositeCriteria<T>*)c;
                            cc->normalize(variables);
                        }
                    }
                    criteria.push_back(c);
                }
            }


            for(i=0; i<domain.size(); i++) {
                nmath::clearTree(&(domain[i]));
            }
            domain.clear();

            return errorCode;
        }

        /**
            Parse the input string in object f to NMAST tree
        */
        int parse(std::vector<Token*> mTokens, NLabParser<T> *mParser) {
            int i, k;
            Criteria<T> *c;
            CompositeCriteria<T> *cc;
            std::vector<NMAST<T>* > domain;

            //TODO: Need to release prefix and domain before call parseFunctionExpression from NLabParser
            mParser->parseFunctionExpression(mTokens, prefix, domain, variables, &errorCode, &errorColumn);
            if (errorCode == NMATH_NO_ERROR) {
                for(i=0; i<variables.size(); i++) {
                    this->strVars[i] = variables[i]->text;
                }
                for (i = 0; i < domain.size(); i++) {
                    c = nullptr;
                    if (domain[i] != nullptr) {
                        c = nmath::buildCriteria(domain[i]);
                        //atemp to normalize criteria so that it hold criteria for all variable in every it's element
                        if ( (c!=nullptr) && c->getCClassType() == COMPOSITE &&
                             ((CompositeCriteria<T>*)c)->logicOperator() == OR) {
                            cc = (CompositeCriteria<T>*)c;
                            cc->normalize(variables);
                        }
                    }
                    criteria.push_back(c);
                }
            }

            for(i=0; i<domain.size(); i++) {
                nmath::clearTree(&(domain[i]));
            }

            domain.clear();

            return errorCode;
        }

		void release() {
			int i;
			if (text != nullptr) {
				delete[] text;
				text = nullptr;
				textLen = 0;
			}

			for(i=0; i<prefix.size(); i++) {
                if(prefix[i] != nullptr) {
                    nmath::clearTree(&(prefix[i]));
                }
			}
			prefix.clear();

			for (i = 0; i < criteria.size(); i++) {
				if (criteria[i] != nullptr)
					delete criteria[i];
			}
			criteria.clear();

            for (i = 0; i < mTokens.size(); i++) {
                if (mTokens[i] != nullptr)
                    delete mTokens[i];
            }
			mTokens.clear();
		}

        int reduce() {
            DParam<T> dp;
            int i;

            for(i=0; i<prefix.size(); i++) {
                dp.t = prefix[i];
                dp.error = 0;
                nmath::reduce_t<T>(&dp);
                prefix[i] = dp.t;
            }
            return 0;
        }

        T getPrefixValue(int idx) {
            return prefix[idx]->value;
        }

        size_t prefixSize() const {
            return prefix.size();
        }

        /**
         *
         * @param values value of each variable
         * @return
         */
        T calc(const T *values) {
            /*
                First we need to check if the input value is in the domain of this function
            */
            if(!this->criteria[0]->check(values)) {
                this->errorCode = ERROR_OUT_OF_DOMAIN;
                return (T)0;
            }

            DParam<T> rp;
            rp.error = 0;
            rp.t = prefix[0];
            memcpy(rp.values, values, variables.size() * sizeof(T));
            auto i = 0;
            for(auto var: variables) {
                rp.variables[i++] = var->text;
                i++;
            }
            rp.varCount = variables.size();
            calc_t<T>((void*)&rp);
            this->errorCode = rp.error;
            return rp.retv;
        }

        std::vector<ImageData<T>*> getSpace(const T *inputInterval, T epsilon, bool needNormalVector) {
            std::vector<ImageData<T>*> lstData;
            ImageData<T> *sp;
            DParam<T> param;
            SimpleCriteria<T>* sc;
            CompositeCriteria<T>* cc;
            Criteria<T>* outCriteria;
            T y, lastX, rightVal;
            int elementOnRow = 0;
            auto valLen = this->variables.size();

            switch (valLen) {
                case 1L:
                    if (criteria.size() <= 0L) {

                        sp = new ImageData<T>(2);

                        param.error = NMATH_NO_ERROR;
                        param.variables[0] = variables[0]->text;
                        param.values[0] = inputInterval[0];
                        param.t = this->prefix[0];
                        while (param.values[0] <= inputInterval[1]) {
                            calc_t<T>((void*)&param);
                            y = param.retv;

                            sp->addData(param.values[0]);
                            sp->addData(y);
                            elementOnRow++;
                            lastX = param.values[0];
                            param.values[0] += epsilon;
                        }

                        if ((lastX < inputInterval[1]) && (param.values[0] > inputInterval[1])) {
                            param.values[0] = inputInterval[1];
                            calc_t<T>((void*)&param);
                            y = param.retv;
                            sp->addData(param.values[0]);
                            sp->addData(y);
                            elementOnRow++;
                        }
                        sp->addRow(elementOnRow);
                        sp->calculateVertexCount();
                        lstData.push_back(sp);
                        return lstData;
                    }

                    outCriteria = criteria[0]->getInterval(inputInterval, strVars);
                    if (outCriteria == nullptr) return lstData;

                    /*
                        Because this is an one-unknown variable function so output criteria is SIMPLE or
                        OR-COMPOSITE criteria

                        if out criteria is SIMPLE: this function is continueous on the output interval
                        otherwise this function is not continueous on the output interval, in other word, it's continueous
                        in output criteria partially
                    */
                    switch (outCriteria->getCClassType()) {
                        case SIMPLE:
                            sc = (SimpleCriteria<T>*)outCriteria;

                            sp = new ImageData<T>(2);

                            param.error = NMATH_NO_ERROR;
                            param.variables[0] = variables[0]->text;
                            param.values[0] = sc->getLeftValue();
                            param.t = prefix[0];
                            rightVal = sc->getRightValue();
                            while (param.values[0] <= rightVal) {
                                calc_t<T>((void*)&param);
                                y = param.retv;
                                sp->addData(param.values[0]);
                                sp->addData(y);
                                elementOnRow++;
                                lastX = param.values[0];
                                param.values[0] += epsilon;
                            }

                            if ((lastX < rightVal) && (param.values[0] > rightVal)) {
                                param.values[0] = rightVal;
                                calc_t<T>((void*)&param);
                                y = param.retv;
                                sp->addData(param.values[0]);
                                sp->addData(y);
                                elementOnRow++;
                            }
                            sp->addRow(elementOnRow);
                            sp->calculateVertexCount();
                            lstData.push_back(sp);
                            break;

                        case COMPOSITE: //OR-COMPOSITE criteria
                            int i;
                            cc = (CompositeCriteria<T>*)outCriteria;
                            for (i = 0; i < cc->size(); i++) {
                                sc = (SimpleCriteria<T>*)cc->get(i);

                                sp = new ImageData<T>(2);
                                param.error = NMATH_NO_ERROR;
                                param.variables[0] = variables[0]->text;
                                param.values[0] = sc->getLeftValue();
                                param.t = prefix[0];
                                rightVal = sc->getRightValue();

                                while (param.values[0] <= rightVal) {
                                    calc_t<T>(&param);
                                    y = param.retv;
                                    sp->addData(param.values[0]);
                                    sp->addData(y);
                                    elementOnRow++;
                                    lastX = param.values[0];
                                    param.values[0] += epsilon;
                                }

                                if ((lastX < rightVal) && (param.values[0] > rightVal)) {
                                    param.values[0] = rightVal;
                                    calc_t<T>((void*)&param);
                                    y = param.retv;
                                    sp->addData(param.values[0]);
                                    sp->addData(y);
                                    elementOnRow++;
                                }
                                sp->addRow(elementOnRow);
                                sp->calculateVertexCount();
                                lstData.push_back(sp);
                            }
                            break;
                    }

                    break;

                case 2L:
                    lstData = getSpaceFor2UnknownVariables(inputInterval, epsilon, needNormalVector);
                    break;

                case 3L:
                    break;

                default:
                    break;
            }

            return lstData;
        }

        NMAST<T>* getVariable(int index) { return variables[index]; }
		char* getText() const { return text; }
		size_t getVarCount() const { return variables.size(); }

		/**
		    fidx NFunction expression index (the index of prefix.list)
		    vidx variable index
            // TODO: derivative of a function will be a function so this method should 
            return a NFunction
		*/
        NMAST<T>* getDerivativeByVariable(int fidx, int vidx) {
            DParam<T> d;
            d.t = prefix[fidx];
            d.error = 0;
            d.returnValue = nullptr;
            d.variables[0] = variables[vidx]->text;
            nmath::derivative<T>(&d);
            return d.returnValue;
        }

        /**
		    PARAM variable - This is the name of variable that we want to calculate the partial derivative
            // TODO: derivative of a function will be a function so this method should 
            return a NFunction
		*/
        NMAST<T>* getDerivative(const char* variable) {
            DParam<T> d;
            d.t = prefix[0];
            d.error = 0;
            d.returnValue = nullptr;
            d.variables[0] = variable;
            nmath::derivative<T>(&d);
            return d.returnValue;
        }
		
        /**
         * TODO:
         * */
		NMAST<T>* getPrefixList() const { return prefix[0]; }
        NMAST<T>* getPrefix(int idx) const { return prefix[idx]; }
		Criteria<T>* getCriteria(int index) const { return criteria[index]; }

        friend std::ostream& operator<< (std::ostream& os, const NFunction& f) {
            os << "\n Function: " << f.getText() << "\n";
            os << "Number of variable: " << (int)f.getVarCount() << "\n";
            size_t sz = f.prefixSize();
            for(size_t i=0; i<sz; i++) {
                os << "Prefix Expresion "<< i <<": \t \n";
                    printNMAST(f.getPrefix(i), 0, os);
            //         if(f.getCriteriaList()->list[i] != NULL) {
            //             os << "\n \t Criteria: \n";
            //             os << (*f.getCriteriaList()->list[i]) << "\n";
            //         }
            }

            return os;
        }
	};
}

#endif
