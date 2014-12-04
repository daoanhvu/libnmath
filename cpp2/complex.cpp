#include "complex.h"

using namespace nmath;

Complex::Complex(double r, double img): real(r), image(img) {
}

Complex::~Complex() {
}

Complex& operator +(double r, const Complex& c) {
}