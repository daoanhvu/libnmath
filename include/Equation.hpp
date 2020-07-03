#ifndef _NLABEQUATION_HPP_
#define _NLABEQUATION_HPP_

namespace nmath {
    typedef struct tagTSolution {
        
    } TSolution;

    template<typename T>
    class NLabEquation {
        private:
            NMAST<T> *leftSide;
            NMAST<T> *rightSide;
        
        public:
            TSolution solve();
    };

    template<typename T>
    TSolution NLabEquation<T>::solve() {

    }
} // namespace nmath 


#endif