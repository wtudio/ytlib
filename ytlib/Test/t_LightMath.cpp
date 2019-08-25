#include "t_LightMath.h"

using namespace std;
namespace ytlib
{
	void test_Complex() {
		Complex c;
		Complex c1(0.5, 6.3);
		Complex c2(c1);

		c = c1 + c2;

		Complex c3 = c;
		cout << c3 << endl;

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
			cout << cc[ii] << endl;
		}
		conjugate_complex(count, cc);
		for (int32_t ii = 0; ii < count; ++ii) {
			cout << cc[ii] << endl;
		}

		Complex *cc0 = new Complex[count];
		get_complex(count, ff, cc0);
		for (int32_t ii = 0; ii < count; ++ii) {
			cout << cc0[ii] << endl;
		}

		fft(count, cc);
		for (int32_t ii = 0; ii < count; ++ii) {
			cout << cc[ii] << endl;
		}
		ifft(count, cc);
		for (int32_t ii = 0; ii < count; ++ii) {
			cout << cc[ii] << endl;
		}
		fftshift(count, cc);
		for (int32_t ii = 0; ii < count; ++ii) {
			cout << cc[ii] << endl;
		}

		delete[] cc;
		delete[] cc0;

		
	}

	void test_Matrix() {
		int32_t count = 9;
		Matrix m;
		cout << m << endll;
		Matrix m1(3, 2);
		cout << m1 << endll;
		tfloat *f = new tfloat[count];
		for (int32_t ii = 0; ii < count; ++ii) {
			f[ii] = ii*ii;
		}

		Matrix m2(3, 3, f);
		cout << m2 << endll;
		Matrix m3(m2);
		cout << m3 << endll;

		m = m3;
		cout << m << endll;

		tfloat *f2 = new tfloat[count];
		m3.getData(f2, 1, 1, 1, 2);
		for (int32_t ii = 0; ii < count; ++ii) {
			cout << f2[ii] << endl;
		}

		Matrix m4 = m3.getMat(0, 0);
		cout << m4 << endll;

		Matrix m5(5, 6);
		m5.setMat(m4);
		cout << m5 << endll;
		m5.setMat(m4, 2, 2);
		cout << m5 << endll;

		m5.setVal(55.5, 4, 4);
		cout << m5 << endll;

		m5.setDiag(77.5);
		cout << m5 << endll;

		

		cout << m << endll;
		std::swap(m5, m);
		cout << m5 << endll;
		cout << m << endll;

		int32_t ii[3] = { 1,2,3 };
		Matrix m6 = m.extractCols(ii, 3);
		cout << m6 << endll;

		m5.zero();
		cout << m5 << endll;

		Matrix m7 = Matrix::eye(5);
		cout << m7 << endll;

		m.eye();
		cout << m << endll;

		Matrix m8 = Matrix::ones(6, 7);
		cout << m8 << endll;

		Matrix m9 = Matrix::reshape(m, 6, 5);
		cout << m9 << endll;

		cout << Matrix::rotMatX(0.5) << endll;
		cout << Matrix::rotMatY(0.5) << endll;
		cout << Matrix::rotMatZ(0.5) << endll;

		Matrix m10 = Matrix::ones(4, 4);
		m10 = Matrix::pow(m10, 3);
		cout << m10 << endll;

		Matrix m11 = Matrix::ones(4, 4);
		cout << m11 << endll;

		cout << m11 + m10 << endl;
		m11 += m10;
		cout << m11 << endll;

		cout << m11 - m10 << endl;
		m11 -= m10;
		cout << m11 << endll;

		cout << m11 * 3 << endl;
		m11 *=3;
		cout << m11 << endll;

		cout << m11 / 3 << endl;
		m11 /= 3;
		cout << m11 << endll;

		cout << -m11 << endll;


		Matrix m12 = Matrix::ones(2, 3);
		cout << m12 << endll;
		Matrix m13 = Matrix::ones(3, 4);
		cout << m13 << endll;

		cout << m12 * m13 << endl;

		m12 *= m13;
		cout << m12 << endll;

		cout << ~m12 << endll;



		delete[] f;
		delete[] f2;
		
	}
	void test_Matrix_c() {

		
	}
	void test_tools() {
		cout << gcd(42, 30) << endl;
		cout << gcd(770, 26) << endl;
		cout << gcd(121, 132) << endl;

		std::vector<uint64_t> v;
		factoring(60, v);
		for (uint32_t ii = 0; ii < v.size(); ++ii) {
			cout << v[ii] << " ";
		}
		cout << endl;
		std::map<uint64_t, uint64_t> m;
		factoring(60, m);
		for (std::map<uint64_t, uint64_t>::iterator ii = m.begin(); ii != m.end(); ++ii) {
			cout << ii->first << ":" << ii->second << " ";
		}
		cout << endl;


		cout << Mul(5) << endl;//120
		cout << Mul(9, 5) << endl;//15120
		cout << Arn(9, 2) << endl;//72
		cout << Crn(9, 2) << endl;//36

		cout << SumAP(1.0, 2.0, 3) << endl;//9
		cout << SumGP(2.0, 1.0, 10) << endl;//20
		cout << SumGP(2.0, 3.0, 4) << endl;//80

		
	}
	void test_bignum() {

		BigNum a((int64_t(1) << 32) + 1);
		cout << a << endl;
		BigNum b((int64_t(1) << 32)+ 1, 10);
		cout << b << endl;

		BigNum c("123456789ABCDEF");
		cout << c << endl;

		BigNum d("123456789123456789", 10);
		cout << d << endl;


		BigNum e("-123456789123456789", 10);
		cout << e << endl;

		BigNum f = e + d;
		cout << f << endl;
		
		BigNum g("9999", 10);
		BigNum h("111", 10);
		cout << g + h << endl;

		BigNum g1("10000", 10);
		BigNum h1("-111", 10);
		cout << g1 + h1 << endl;

		cout << g * h << endl;
		g *= h;
		cout << g << endl;



		
	}
}