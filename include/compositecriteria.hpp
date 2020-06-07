#ifndef _COMPOSITECRITERIA_H
#define _COMPOSITECRITERIA_H

#include <vector>
#include <string>
#include "criteria.hpp"
#include "SimpleCriteria.hpp"

namespace nmath {

    template <typename T>
	class CompositeCriteria: public Criteria<T> {
		private:
			std::vector<nmath::Criteria<T>* > list;

			int logicOp; // AND OR

			Criteria<T>* andSimpleSelf(SimpleCriteria<T> &c);
			Criteria<T>* andCompositeSelf(CompositeCriteria<T>& c);
			
		public:
			CompositeCriteria();
			~CompositeCriteria();
			void release();
			Criteria<T>* clone() override ;

			int logicOperator() const { return logicOp; }
			void setOperator(int op)	{ logicOp = op; }

			size_t size() const { return list.size(); }

            T getLeftValue() const override { return (T)-1; }
            T getRightValue() const override { return (T)-1; };

			bool check(const double* values);

			void add(Criteria<T>* c);

			Criteria<T>& normalize(std::vector<NMAST<T>* > variables) override ;
			
			bool containsVar(const std::string &var) override;

			Criteria<T>* andSelf(Criteria<T>& c) override;
			CompositeCriteria<T>* andCriteria(SimpleCriteria<T> &c);
			CompositeCriteria<T>* andCriteria(CompositeCriteria<T> &c);
			CompositeCriteria<T>* orCriteria(SimpleCriteria<T> &c);
			CompositeCriteria<T>* orCriteria(CompositeCriteria<T> &c);
			
			Criteria<T>* operator [](int index) const;
			Criteria<T>* get(int index) const { return list[index]; }

			Criteria<T>* operator |(Criteria<T> &) override ;
			Criteria<T>* operator &(Criteria<T> &) override ;
			CompositeCriteria<T>& operator |=(Criteria<T> &);
			CompositeCriteria<T>& operator &=(Criteria<T> &);
			CompositeCriteria<T>& operator =(CompositeCriteria<T> &);

			Criteria<T>* getInterval(const T *values, const std::vector<std::string> &variables) override {
				Criteria<T> *listIn;
				int i;
				auto *out = new CompositeCriteria<T>();
				out->setOperator(logicOp);
				
				for (i = 0; i<list.size(); i++) {
					listIn = list[i]->getInterval(values, variables);
					if (listIn != nullptr) {
						out->add(listIn);
					}
				}

				return out;
			}
	};


	template <typename T>
	CompositeCriteria<T>::CompositeCriteria(): Criteria<T>(NMathCClassType::COMPOSITE) {
	}

	template <typename T>
	CompositeCriteria<T>::~CompositeCriteria() {
		if (list.empty()) return;
		for (auto &i : list)
	        delete i;
		list.clear();
	}

	template <typename T>
	void CompositeCriteria<T>::release() {
	    if (list.empty()) return;
		for (auto &i : list)
	        delete i;
		list.clear();
	}

	template <typename T>
	bool CompositeCriteria<T>::containsVar(const std::string &var) {
		for (auto &i : list) {
			if(i->containsVar(var))
				return true;
		}
		return false;
	}

	template <typename T>
	Criteria<T>* CompositeCriteria<T>::clone() {
		auto *out = new CompositeCriteria<T>();
		out->setOperator(this->logicOp);
		for (auto &i : list) {
			out->add(i->clone());
		}
		return out;
	}

	template <typename T>
	void CompositeCriteria<T>::add(Criteria<T>* c) {
		list.push_back(c);
	}

	template <typename T>
	Criteria<T>* CompositeCriteria<T>::operator [](int index) const {
		return (index >= list.size())? nullptr : list[index];
	}

	template <typename T>
	bool CompositeCriteria<T>::check(const double* values) {
		for(auto &c: list) {
			if( c->check(values) )
				return true;
		}
		return false;
	}

	template <typename T>
	Criteria<T>* CompositeCriteria<T>::andSelf(Criteria<T> &c) {

		if (c.getCClassType() == SIMPLE)
			return andSimpleSelf((SimpleCriteria<T>&)c);

		return andCompositeSelf((CompositeCriteria<T>&)c);

	}

	/**
	 * Implement an AND operator between this criteria and c
	 * @param c an other CompositeCriteria
	 * @return
	 */
	template <typename T>
	Criteria<T>* CompositeCriteria<T>::andCompositeSelf(CompositeCriteria<T> &c) {
		CompositeCriteria<T> *out;
		Criteria<T> *tmp;
		int i;

		if (c.logicOperator() == AND) {
			if (logicOp == AND) {
				for (i = 0; i<c.size(); i++) {
					this->andSelf(*c[i]);
				}
				out = this;
			}
			else {
				//AND and OR
				for (auto &itm: list) {
					tmp = itm->andSelf(c);
					if (tmp != nullptr) {
						itm = tmp;
					}
				}
				out = this;
			}
		}
		else {
			if (logicOp == AND) { // OR & AND
				out = new CompositeCriteria<T>();
				out->setOperator(OR);
				for (i = 0; i<c.size()-1; i++) {
	                auto *cc1 = (CompositeCriteria<T>*)clone();
					tmp = cc1->andSelf(*c[i]);
					if (tmp != nullptr) {
						out->add(tmp);
					}
				}
				
				tmp = this->andSelf(*c[i]);
				if (tmp != nullptr) {
					out->add(tmp);
				}
			}
			else { //OR & OR
				for (i = 0; i<c.size(); i++) {
					this->add(c[i]->clone());
				}
				out = this;
			}
		}
		return out;
	}

	/*
		Add directly to this object, don't clone it
	*/
	template <typename T>
	Criteria<T>* CompositeCriteria<T>::andSimpleSelf(SimpleCriteria<T> &c) {
		int i;
		Criteria<T> *itm;

		if (logicOp == AND){
			for(i=0; i<list.size(); i++) {
				if(list[i]->containsVar(c.getVariable())) {
					itm = list[i]->andSelf(c);
					list[i] = itm;
					return this;
				}
			}
			
			add(c.clone());
			return this;
		}
		
		//OR
		for(i=0; i<list.size(); i++) {
			itm = list[i]->andSelf(c);
			list[i] = itm;
		}
		
		return this;
	}

	template <typename T>
	CompositeCriteria<T>* CompositeCriteria<T>::andCriteria(SimpleCriteria<T> &c) {
		Criteria<T> *tmp;
		CompositeCriteria<T> *out;
		int i;
		if (logicOp == AND){
			out = (CompositeCriteria<T>*)clone();
			out->add(c.clone());
			return out;
		}

		out = new CompositeCriteria<T>();
		out->setOperator(OR);
		for (i = 0; i<list.size(); i++) {

			if(list[i]->getCClassType() == SIMPLE) {
			}

			tmp = (*list[i]) & (Criteria<T>&)c;
			if (tmp != nullptr) {
				out->add(tmp);
			}
		}
		
		return out;
	}

	template <typename T>
	CompositeCriteria<T>* CompositeCriteria<T>::andCriteria(CompositeCriteria<T> &c) {
		CompositeCriteria<T> *out;
		Criteria<T> *tmp;
		int i;

		if (c.logicOperator() == AND) {
			if (logicOp == AND) {
				out = (CompositeCriteria<T>*)this->clone();
				for (i = 0; i<c.size(); i++) {
					out->add(c[i]->clone());
				}
			}
			else {
				//AND and OR
				out = new CompositeCriteria<T>();
				out->setOperator(OR);
				for (i = 0; i<list.size(); i++) {
					tmp = (*list[i]) & c;
					if (tmp != nullptr) {
						out->add(tmp);
					}
				}
			}
		}
		else {
			if (logicOp == AND) { // OR & AND
				out = new CompositeCriteria<T>();
				out->setOperator(OR);
				for (i = 0; i<c.size(); i++) {
					tmp = (*this) & (*c[i]);
					if (tmp != nullptr) {
						out->add(tmp);
					}
				}
			}
			else { //OR & OR
				out = (CompositeCriteria<T>*)this->clone();
				for (i = 0; i<c.size(); i++) {
					out->add(c[i]->clone());
				}
			}
		}
		return out;
	}

	template <typename T>
	Criteria<T>* CompositeCriteria<T>::operator &(Criteria<T> &c) {
		Criteria<T>* out = nullptr;
		switch(c.getCClassType()) {
			case SIMPLE:
				out = this->andCriteria((SimpleCriteria<T>&)c);
				break;

			case COMPOSITE:
				out = this->andCriteria((CompositeCriteria<T>&)c);
				break;
		}

		return out;
	}

	template <typename T>
	CompositeCriteria<T>* CompositeCriteria<T>::orCriteria(SimpleCriteria<T> &c) {
		CompositeCriteria<T>* out;

		if (this->logicOp == AND) {
			//AND | Simple = OR (AND, Simple)
			out = new CompositeCriteria<T>();
			out->setOperator(OR);
			out->add(clone());
			out->add(c.clone());
		}
		else {
			// OR | Simple = add Simple into this->clone()
			out = (CompositeCriteria<T>*)clone();
			out->add(c.clone());
		}

		return out;
	}

	template <typename T>
	CompositeCriteria<T>* CompositeCriteria<T>::orCriteria(CompositeCriteria<T> &c) {
		int i;
		CompositeCriteria<T> *out;
		if (this->logicOp == AND) {
			if (c.logicOperator() == AND) {
				// AND | AND = OR (this->clone, c.clone())
				out = new CompositeCriteria<T>();
				out->setOperator(OR);
				out->add(this->clone());
				out->add(c.clone());
			}
			else {
				// AND | OR
				out = (CompositeCriteria<T>*)c.clone();
				out->add(this->clone());
			}
		}
		else {
			if (c.logicOperator() == AND) {
				out = (CompositeCriteria<T>*)clone();
				out->add(c.clone());
			}
			else {
				out = (CompositeCriteria<T>*)clone();
				for (i = 0; i < c.size(); i++){
					out->add(c[i]->clone());
				}
			}
		}

		return out;
	}

	template <typename T>
	Criteria<T>* CompositeCriteria<T>::operator |(Criteria<T>& c) {
		switch(c.getCClassType()){
			case SIMPLE:
				return andCriteria((SimpleCriteria<T>&)c);
			
			case COMPOSITE:
				return andCriteria((CompositeCriteria<T>&)c);

	        default:
	            return nullptr;
		}
	}

	/**
	values [IN]
	This is a matrix N rows x 2 columns which N equals varCount

	@param outInterval
	This output parameter, it's a matrix N row and M columns which each row is for each continuous space for the expression
	It means that each row will hold a combined-interval for n-tule variables and M equal varCount * 2
	*/
	/*
	Criteria* CompositeCriteria::getIntervalF(const float *values, const char* var, int varCount) {
		Criteria *listIn;
		int i;
		CompositeCriteria *out = new CompositeCriteria();
		out->setOperator(logicOp);

		for (i = 0; i<mSize; i++) {
			listIn = list[i]->getIntervalF(values, var, varCount);
			if (listIn != NULL) {
				out->add(listIn);
			}
		}

		return out;
	}

	Criteria* CompositeCriteria::getInterval(const double *values, const char* var, int varCount) {
		Criteria *listIn;
		int i;
		CompositeCriteria *out = new CompositeCriteria();
		out->setOperator(logicOp);
		
		for (i = 0; i<mSize; i++) {
			listIn = list[i]->getInterval(values, var, varCount);
			if (listIn != NULL) {
				out->add(listIn);
			}
		}

		return out;
	}
	*/
	template <typename T>
	Criteria<T>& CompositeCriteria<T>::normalize(std::vector<NMAST<T>* > variables) {
		int k, i;
		CompositeCriteria<T> *ncc;
		SimpleCriteria<T> *sc;

		for (i = 0; i<list.size(); i++) {
			for (k = 0; k < variables.size(); k++) {
				if (!list[i]->containsVar(variables[k]->text)){
					sc = new SimpleCriteria<T>(GTE_LTE, variables[k]->text, (T)0, (T)0, true, true);
					ncc = new CompositeCriteria<T>();
					ncc->setOperator(AND);

					ncc->add(list[i]);
					ncc->add(sc);

					list[i] = ncc;
				}
			}
		}

		return *this;
	}
}

#endif
