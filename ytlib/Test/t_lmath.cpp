#include "t_lmath.h"


namespace ytlib
{
	bool test_Complex() {
		Complex c;
		Complex c1(0.5, 6.3);
		Complex c2(c1);

		c = c1 + c2;

		Complex c3 = c;
		std::cout << c3 << std::endl;

		Complex c4(c3 += c);
		c.swap(c4);

		Complex c5 = Complex::conj(c4);

		tfloat a = Complex::abs(c5);
		tfloat b = Complex::angle(c5);

		Complex c6 = Complex::sqrt(c5);

		int32_t count = 16;
		Complex *cc = new Complex[count];
		tfloat *ff = new tfloat[count];

		for (int32_t ii = 0; ii < count; ++ii) {
			ff[ii] = std::pow(ii, 1.7);
			cc[ii].real = std::sqrt(ii * 3);
			cc[ii].imag = std::pow(ii, 1.5);
			std::cout << cc[ii] << std::endl;
		}
		conjugate_complex(count, cc);
		for (int32_t ii = 0; ii < count; ++ii) {
			std::cout << cc[ii] << std::endl;
		}

		Complex *cc0 = new Complex[count];
		get_complex(count, ff, cc0);
		for (int32_t ii = 0; ii < count; ++ii) {
			std::cout << cc0[ii] << std::endl;
		}

		fft(count, cc);
		for (int32_t ii = 0; ii < count; ++ii) {
			std::cout << cc[ii] << std::endl;
		}
		ifft(count, cc);
		for (int32_t ii = 0; ii < count; ++ii) {
			std::cout << cc[ii] << std::endl;
		}
		fftshift(count, cc);
		for (int32_t ii = 0; ii < count; ++ii) {
			std::cout << cc[ii] << std::endl;
		}

		delete[] cc;
		delete[] cc0;

		return true;
	}

	bool test_Matrix() {
		int32_t count = 9;
		Matrix m;
		std::cout << m << endll;
		Matrix m1(3, 2);
		std::cout << m1 << endll;
		tfloat *f = new tfloat[count];
		for (int32_t ii = 0; ii < count; ++ii) {
			f[ii] = ii*ii;
		}

		Matrix m2(3, 3, f);
		std::cout << m2 << endll;
		Matrix m3(m2);
		std::cout << m3 << endll;

		m = m3;
		std::cout << m << endll;

		tfloat *f2 = new tfloat[count];
		m3.getData(f2, 1, 1, 1, 2);
		for (int32_t ii = 0; ii < count; ++ii) {
			std::cout << f2[ii] << std::endl;
		}

		Matrix m4 = m3.getMat(0, 0);
		std::cout << m4 << endll;

		Matrix m5(5, 6);
		m5.setMat(m4);
		std::cout << m5 << endll;
		m5.setMat(m4,2,2);
		std::cout << m5 << endll;
		m5.setMat(m4, 4, 4);
		std::cout << m5 << endll;

		m5.setVal(55.5, 4, 4);
		std::cout << m5 << endll;

		m5.setDiag(77.5);
		std::cout << m5 << endll;

		

		std::cout << m << endll;
		std::swap(m5, m);
		std::cout << m5 << endll;
		std::cout << m << endll;

		int32_t ii[3] = { 1,2,3 };
		Matrix m6 = m.extractCols(ii, 3);
		std::cout << m6 << endll;

		m5.zero();
		std::cout << m5 << endll;

		Matrix m7 = Matrix::eye(5);
		std::cout << m7 << endll;

		m.eye();
		std::cout << m << endll;

		Matrix m8 = Matrix::ones(6, 7);
		std::cout << m8 << endll;

		Matrix m9 = Matrix::reshape(m, 6, 5);
		std::cout << m9 << endll;

		std::cout << Matrix::rotMatX(0.5) << endll;
		std::cout << Matrix::rotMatY(0.5) << endll;
		std::cout << Matrix::rotMatZ(0.5) << endll;

		Matrix m10 = Matrix::ones(4, 4);
		m10 = Matrix::pow(m10, 10);
		std::cout << m10 << endll;


		delete[] f;
		delete[] f2;
		return true;
	}
	bool test_Matrix_c() {

		return true;
	}
	bool test_tools() {
		std::cout << gcd(42, 30) << std::endl;
		std::cout << gcd(770, 26) << std::endl;
		std::cout << gcd(121, 132) << std::endl;
	}
}