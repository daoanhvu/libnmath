#ifndef _COMPLEX_H_
#define _COMPLEX_H_

/*
 * z = image*i + real
 * i^2 = -1
 * */
 
namespace nmath {
    template <typename T>
	class Complex {
    private:
        T real;
        T image;

    public:
        Complex(T r, T img);

        Complex& operator =(const Complex& c);

        friend Complex& operator +(double r, const Complex& c);
        friend Complex& operator +(float r, const Complex& c);
        friend Complex& operator +(const Complex& c, double r);
        friend Complex& operator +(const Complex& c1, const Complex& c2);
	};

    template <typename T>
    Complex<T>::Complex(T r, T img): real(r), image(img) {
    }

    template <typename T>
    Complex<T>& Complex<T>::operator =(const Complex<T>& c){
        this->real = c.real;
        this->image = c.image;

        return *this;
    }

    template <typename T>
    Complex<T>& operator +(double r, const Complex<T>& c) {
        Complex result((T)r + c.real, c.image);
        return result;
    }

    template <typename T>
    Complex<T>& operator +(float r, const Complex<T>& c) {
        Complex result((T)r + c.real, c.image);
        return result;
    }

    template <typename T>
    Complex<T>& operator +(const Complex<T>& c, double r) {
        Complex result((T)r + c.real, c.image);
        return result;
    }

    template <typename T>
    Complex<T>& operator +(const Complex<T>& c1, const Complex<T>& c2) {
        Complex result(c1.real + c2.real, c1.image + c2.image);
        return result;
    }
}
#endif
