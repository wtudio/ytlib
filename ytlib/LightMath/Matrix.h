#pragma once

#include <string>
#include <iostream>
#include <cstring>

#include <ytlib/LightMath/Mathbase.h>
#include <ytlib/LightMath/complex.h>


#define MAXSIZE 100000


namespace ytlib
{

	//改造成模板形式，可以放复数、int、float、double等等
	template <typename T>
	class Basic_Matrix {
	public:
		// direct data access
		T   **val;
		int32_t   m, n;//m行n列

		Basic_Matrix() :m(0), n(0), val(0) {}
		Basic_Matrix(const int32_t m_, const int32_t n_) {
			_allocateMemory(m_, n_);
		}
		Basic_Matrix(const int32_t m_, const int32_t n_, const T* val_,int32_t N_=0) {
			_allocateMemory(m_, n_);
			if (N_ == 0) N_ = m*n;
			int32_t k = 0;
			for (int32_t i = 0; i < m_; ++i)
				for (int32_t j = 0; j < n_; ++j) {
					if (k >= N_) return;
					val[i][j] = val_[k++];
				}
					
		}
		Basic_Matrix(const Basic_Matrix &M) {
			_allocateMemory(M.m, M.n);
			memcpy(val[0], M.val[0], M.n*M.m * sizeof(T));
		}
		~Basic_Matrix() {
			_releaseMemory();
		}
		Basic_Matrix& operator= (const Basic_Matrix &M) {
			if (this != &M) {
				if (M.m != m || M.n != n) {
					_releaseMemory();
					_allocateMemory(M.m, M.n);
				}
				memcpy(val[0], M.val[0], M.n*M.m * sizeof(T));
			}
			return *this;
		}
		//右值引用
		Basic_Matrix(Basic_Matrix &&M) :val(M.val), m(M.m), n(M.n) {
			M.val = NULL;
			M.m = M.n = 0;
		}
		Basic_Matrix& operator=(Basic_Matrix&& M) {
			if (this != &M) {
				//先释放自己的资源
				_releaseMemory();
				val = M.val;
				m = M.m; n = M.n;
				M.val = NULL;
				M.m = M.n = 0;
			}
			return *this;
		}

		// copies submatrix of M into array 'val', default values copy whole row/column/matrix
		void getData(T* val_, int32_t i1 = 0, int32_t j1 = 0, int32_t i2 = -1, int32_t j2 = -1, int32_t N_ = 0) const {
			if (i2 == -1) i2 = m - 1;
			if (j2 == -1) j2 = n - 1;
			if (i1 < 0 || i2 >= m || j1 < 0 || j2 >= n) {
				std::cerr << "Index exceeds matrix dimensions." << std::endl;
				return;
			}
			if (i2 < i1 || j2 < j1) {
				std::cerr << "Invalid Input Range." << std::endl;
				return;
			}
			if (N_ == 0) N_ = (i2 - i1 + 1)*(j2 - j1 + 1);
			int32_t k = 0;
			for (int32_t i = i1; i <= i2; ++i)
				for (int32_t j = j1; j <= j2; ++j) {
					if (k >= N_) return;
					val_[k++] = val[i][j];
				}
					
		}

		// set or get submatrices of current matrix
		Basic_Matrix getMat(int32_t i1, int32_t j1, int32_t i2 = -1, int32_t j2 = -1) const {
			if (i2 == -1) i2 = m - 1;
			if (j2 == -1) j2 = n - 1;
			if (i1 < 0 || i2 >= m || j1 < 0 || j2 >= n) {
				std::cerr << "Index exceeds matrix dimensions." << std::endl;
				return Basic_Matrix();
			}
			if (i2 < i1 || j2 < j1) {
				std::cerr << "Invalid Input Range." << std::endl;
				return Basic_Matrix();
			}
			Basic_Matrix M(i2 - i1 + 1, j2 - j1 + 1);
			for (int32_t i = 0; i < M.m; ++i)
				for (int32_t j = 0; j < M.n; ++j)
					M.val[i][j] = val[i1 + i][j1 + j];
			return M;
		}

		void setMat(const Basic_Matrix &M, const int32_t i1=0, const int32_t j1=0) {
			if (i1<0 || j1<0 || (i1 + M.m)>m || (j1 + M.n)>n) {
				std::cerr << "Index exceeds matrix dimensions." << std::endl;
				return;
			}
			for (int32_t i = 0; i < M.m; ++i)
				for (int32_t j = 0; j < M.n; ++j)
					val[i1 + i][j1 + j] = M.val[i][j];
		}

		// set sub-matrix to scalar (default 0), -1 as end replaces whole row/column/matrix
		void setVal(const T& s, int32_t i1 = 0, int32_t j1 = 0, int32_t i2 = -1, int32_t j2 = -1) {
			if (i2 == -1) i2 = m - 1;
			if (j2 == -1) j2 = n - 1;
			if (i1 < 0 || i2 >= m || j1 < 0 || j2 >= n) {
				std::cerr << "Index exceeds matrix dimensions." << std::endl;
				return;
			}
			if (i2 < i1 || j2 < j1) {
				std::cerr << "Invalid Input Range." << std::endl;
				return;
			}
			for (int32_t i = i1; i <= i2; ++i)
				for (int32_t j = j1; j <= j2; ++j)
					val[i][j] = s;
		}

		// set (part of) diagonal to scalar, -1 as end replaces whole diagonal
		void setDiag(const T& s, int32_t i1 = 0, int32_t i2 = -1) {
			if (i2 == -1) i2 = min(m, n) - 1;
			if (i1 < 0 || i2 >= min(m, n)) {
				std::cerr << "Index exceeds matrix dimensions." << std::endl;
				return;
			}
			if (i2 < i1) {
				std::cerr << "Invalid Input Range." << std::endl;
				return;
			}
			for (int32_t i = i1; i <= i2; ++i)
				val[i][i] = s;
		}

		// clear matrix
		void zero() {
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					val[i][j] = T();
		}
		Basic_Matrix& swap(Basic_Matrix &value) {
			if (this != &value) {
				int32_t tmp = value.m;
				value.m = this->m; this->m = tmp;
				tmp = value.n;
				value.n = this->n; this->n = tmp;
				T ** ptmp = value.val;
				value.val = this->val; this->val = ptmp;
			}
			return *this;
		}
		// extract columns with given index
		Basic_Matrix extractCols(const int32_t* idx,const int32_t N_) const {
			Basic_Matrix M(m, N_);
			for (int32_t j = 0; j < N_; ++j)
				if ((idx[j] < n) && (idx[j] > 0)) {
					for (int32_t i = 0; i < m; ++i)
						M.val[i][j] = val[i][idx[j]];
				}
				else {
					std::cerr << "Index exceeds matrix dimensions." << std::endl;
					return Basic_Matrix();
				}
					
			return M;
		}
		// create identity matrix
		static Basic_Matrix eye(const int32_t m) {
			Basic_Matrix M(m, m);
			for (int32_t i = 0; i < m; ++i)
				M.val[i][i] = T(1);
			return M;
		}

		void eye() {
			zero();
			int32_t min_d = min(m, n);
			for (int32_t i = 0; i < min_d; ++i)
				val[i][i] = T(1);
		}

		// create matrix with ones
		static Basic_Matrix ones(const int32_t m, const int32_t n) {
			Basic_Matrix M(m, n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					M.val[i][j] = T(1);
			return M;
		}
		// create diagonal matrix with nx1 or 1xn matrix M as elements
		static Basic_Matrix diag(const Basic_Matrix &M) {
			if (M.m > 1 && M.n == 1) {
				Basic_Matrix D(M.m, M.m);
				for (int32_t i = 0; i < M.m; ++i)
					D.val[i][i] = M.val[i][0];
				return D;
			}
			else if (M.m == 1 && M.n > 1) {
				Basic_Matrix D(M.n, M.n);
				for (int32_t i = 0; i < M.n; ++i)
					D.val[i][i] = M.val[0][i];
				return D;
			}
			std::cerr << "ERROR: Trying to create diagonal matrix from vector of size (" << M.m << "x" << M.n << ")" << std::endl;
			return Basic_Matrix();
		}

		// returns the m-by-n matrix whose elements are taken column-wise from M
		static Basic_Matrix reshape(const Basic_Matrix &M, const int32_t m_, const int32_t n_) {
			if (M.m*M.n != m_*n_) {
				std::cerr << "ERROR: Trying to reshape a matrix of size (" << M.m << "x" << M.n <<
					") to size (" << m_ << "x" << n_ << ")" << std::endl;
				return Basic_Matrix();
			}
			Basic_Matrix M2(m_, n_);
			int32_t size_ = m_*n_;
			for (int32_t k = 0; k < size_; ++k) {
				M2.val[k / n_][k%n_] = M.val[k / M.n][k%M.n];
			}
			return M2;
		}

		// create 3x3 rotation matrices (convention: http://en.wikipedia.org/wiki/Rotation_matrix)
		static Basic_Matrix rotMatX(const tfloat angle) {
			tfloat s = sin(angle);
			tfloat c = cos(angle);
			Basic_Matrix R(3, 3);
			R.val[0][0] = T(+1);
			R.val[1][1] = T(+c);
			R.val[1][2] = T(-s);
			R.val[2][1] = T(+s);
			R.val[2][2] = T(+c);
			return R;
		}
		static Basic_Matrix rotMatY(const tfloat angle) {
			tfloat s = sin(angle);
			tfloat c = cos(angle);
			Basic_Matrix R(3, 3);
			R.val[0][0] = T(+c);
			R.val[0][2] = T(+s);
			R.val[1][1] = T(+1);
			R.val[2][0] = T(-s);
			R.val[2][2] = T(+c);
			return R;
		}
		static Basic_Matrix rotMatZ(const tfloat angle) {
			tfloat s = sin(angle);
			tfloat c = cos(angle);
			Basic_Matrix R(3, 3);
			R.val[0][0] = T(+c);
			R.val[0][1] = T(-s);
			R.val[1][0] = T(+s);
			R.val[1][1] = T(+c);
			R.val[2][2] = T(+1);
			return R;
		}


		/////todo:测试到这。还需要添加随机矩阵生成方法。再看看matlab里还有什么常用矩阵

		// simple arithmetic operations
		// add matrix
		Basic_Matrix  operator+ (const Basic_Matrix &M) const {
			const Basic_Matrix &A = *this;
			const Basic_Matrix &B = M;
			if (A.m != B.m || A.n != B.n) {
				std::cerr << "ERROR: Trying to add matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << std::endl;
				exit(0);
			}
			Basic_Matrix C(A.m, A.n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[i][j] = A.val[i][j] + B.val[i][j];
			return C;
		}
		Basic_Matrix&  operator+= (const Basic_Matrix &M) {
			const Basic_Matrix &A = *this;
			const Basic_Matrix &B = M;
			if (A.m != B.m || A.n != B.n) {
				std::cerr << "ERROR: Trying to add matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << std::endl;
				exit(0);
			}
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					A.val[i][j] += B.val[i][j];
			return *this;
		}
		// subtract matrix
		Basic_Matrix  operator- (const Basic_Matrix &M) const {
			const Basic_Matrix &A = *this;
			const Basic_Matrix &B = M;
			if (A.m != B.m || A.n != B.n) {
				std::cerr << "ERROR: Trying to subtract matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << std::endl;
				exit(0);
			}
			Basic_Matrix C(A.m, A.n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[i][j] = A.val[i][j] - B.val[i][j];
			return C;
		}
		Basic_Matrix&  operator-= (const Basic_Matrix &M) {
			const Basic_Matrix &A = *this;
			const Basic_Matrix &B = M;
			if (A.m != B.m || A.n != B.n) {
				std::cerr << "ERROR: Trying to subtract matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << std::endl;
				exit(0);
			}
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					A.val[i][j] -= B.val[i][j];
			return *this;
		}
		// multiply with matrix
		Basic_Matrix  operator* (const Basic_Matrix &M) const {
			const Basic_Matrix &A = *this;
			const Basic_Matrix &B = M;
			if (A.n != B.m) {
				std::cerr << "ERROR: Trying to multiply matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << std::endl;
				exit(0);
			}
			Basic_Matrix C(A.m, B.n);
			for (int32_t i = 0; i < A.m; ++i)
				for (int32_t j = 0; j < B.n; ++j)
					for (int32_t k = 0; k < A.n; ++k)
						C.val[i][j] += A.val[i][k] * B.val[k][j];
			return C;
		}
		Basic_Matrix&  operator*= (const Basic_Matrix &M) {
			const Basic_Matrix &A = *this;
			const Basic_Matrix &B = M;
			if (A.n != B.m) {
				std::cerr << "ERROR: Trying to multiply matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << std::endl;
				exit(0);
			}
			Basic_Matrix C(A.m, B.n);
			for (int32_t i = 0; i < A.m; ++i)
				for (int32_t j = 0; j < B.n; ++j)
					for (int32_t k = 0; k < A.n; ++k)
						C.val[i][j] += A.val[i][k] * B.val[k][j];

			return this->swap(C);
		}
		// multiply with scalar
		Basic_Matrix  operator* (const T &s) const {
			Basic_Matrix C(m, n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[i][j] = val[i][j] * s;
			return C;
		}
		Basic_Matrix&  operator*= (const T &s) {
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					val[i][j] *= s;
			return *this;
		}
		// divide elementwise by matrix (or vector)
		Basic_Matrix  operator/ (const Basic_Matrix &M) const {
			const Basic_Matrix &A = *this;
			const Basic_Matrix &B = M;

			if (A.m == B.m && A.n == B.n) {
				Basic_Matrix C(A.m, A.n);
				for (int32_t i = 0; i < A.m; ++i)
					for (int32_t j = 0; j < A.n; ++j)
						if (B.val[i][j] != 0)
							C.val[i][j] = A.val[i][j] / B.val[i][j];
				return C;

			}
			else if (A.m == B.m && B.n == 1) {
				Basic_Matrix C(A.m, A.n);
				for (int32_t i = 0; i < A.m; ++i)
					for (int32_t j = 0; j < A.n; ++j)
						if (B.val[i][0] != 0)
							C.val[i][j] = A.val[i][j] / B.val[i][0];
				return C;

			}
			else if (A.n == B.n && B.m == 1) {
				Basic_Matrix C(A.m, A.n);
				for (int32_t i = 0; i < A.m; ++i)
					for (int32_t j = 0; j < A.n; ++j)
						if (B.val[0][j] != 0)
							C.val[i][j] = A.val[i][j] / B.val[0][j];
				return C;

			}
			else {
				std::cerr << "ERROR: Trying to divide matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << std::endl;
				exit(0);
			}
		}
		// divide by scalar
		Basic_Matrix  operator/ (const T &s) const {
			if (abs(s) < 1e-20) {
				std::cerr << "ERROR: Trying to divide by zero!" << std::endl;
				exit(0);
			}
			Basic_Matrix C(m, n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[i][j] = val[i][j] / s;
			return C;
		}

		// negative matrix
		Basic_Matrix  operator- () const {
			Basic_Matrix C(m, n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[i][j] = -val[i][j];
			return C;
		}
		// transpose
		Basic_Matrix  operator~ () const {
			Basic_Matrix C(n, m);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[j][i] = val[i][j];
			return C;
		}
		// pow
		static Basic_Matrix pow(const Basic_Matrix &value, uint32_t n) {
			Basic_Matrix tmp = value, re(value.m, value.n);
			re.eye();
			for (; n; n >>= 1) {
				if (n & 1)
					re *= tmp;
				tmp *= tmp;
			}
			return re;
		}
		// euclidean norm (vectors) / frobenius norm (matrices)
		T   l2norm() const {
			T norm = 0;
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					norm += val[i][j] * val[i][j];
			return sqrt(norm);
		}
		// mean of all elements in matrix
		T   mean() const {
			T mean = 0;
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					mean += val[i][j];
			return mean / (tfloat)(m*n);
		}

		// complex arithmetic operations
		// cross product of two vectors
		static Basic_Matrix cross(const Basic_Matrix &a, const Basic_Matrix &b) {
			if (a.m != 3 || a.n != 1 || b.m != 3 || b.n != 1) {
				std::cerr << "ERROR: Cross product vectors must be of size (3x1)" << std::endl;
				exit(0);
			}
			Basic_Matrix c(3, 1);
			c.val[0][0] = a.val[1][0] * b.val[2][0] - a.val[2][0] * b.val[1][0];
			c.val[1][0] = a.val[2][0] * b.val[0][0] - a.val[0][0] * b.val[2][0];
			c.val[2][0] = a.val[0][0] * b.val[1][0] - a.val[1][0] * b.val[0][0];
			return c;
		}
		// invert matrix M
		static Basic_Matrix inv(const Basic_Matrix &M) {
			if (M.m != M.n) {
				std::cerr << "ERROR: Trying to invert matrix of size (" << M.m << "x" << M.n << ")" << std::endl;
				exit(0);
			}
			Basic_Matrix A(M);
			Basic_Matrix B = eye(M.m);
			B.solve(A);
			return B;
		}
		// invert this matrix
		bool   inv() const {
			if (m != n) {
				std::cerr << "ERROR: Trying to invert matrix of size (" << m << "x" << n << ")" << std::endl;
				exit(0);
			}
			Basic_Matrix A(*this);
			eye();
			solve(A);
			return true;
		}
		// returns determinant of matrix
		T  det() const {

			if (m != n) {
				std::cerr << "ERROR: Trying to compute determinant of a matrix of size (" << m << "x" << n << ")" << std::endl;
				exit(0);
			}

			Basic_Matrix A(*this);
			int32_t *idx = (int32_t*)malloc(m * sizeof(int32_t));
			T d;
			A.lu(idx, d);
			for (int32_t i = 0; i < m; ++i)
				d *= A.val[i][i];
			free(idx);
			return d;
		}
		
		// replace *this by lower upper decomposition
		bool   lu(int32_t *idx, tfloat &d, tfloat eps = 1e-20) const {

			if (m != n) {
				std::cerr << "ERROR: Trying to LU decompose a matrix of size (" << m << "x" << n << ")" << std::endl;
				exit(0);
			}

			int32_t i, imax, j, k;
			tfloat   big, dum, sum, temp;
			tfloat* vv = (tfloat*)malloc(n * sizeof(tfloat)); // vv stores the implicit scaling of each row.
			d = 1.0;
			for (i = 0; i < n; ++i) { // Loop over rows to get the implicit scaling information.
				big = 0.0;
				for (j = 0; j < n; ++j)
					if ((temp = abs(val[i][j])) > big)
						big = temp;
				if (big == 0.0) { // No nonzero largest element.
					free(vv);
					return false;
				}
				vv[i] = 1.0 / big; // Save the scaling.
			}
			for (j = 0; j < n; ++j) { // This is the loop over columns of Crout’s method.
				for (i = 0; i < j; ++i) { // This is equation (2.3.12) except for i = j.
					sum = val[i][j];
					for (k = 0; k < i; ++k)
						sum -= val[i][k] * val[k][j];
					val[i][j] = sum;
				}
				big = 0.0; // Initialize the search for largest pivot element.
				for (i = j; i < n; ++i) {
					sum = val[i][j];
					for (k = 0; k < j; ++k)
						sum -= val[i][k] * val[k][j];
					val[i][j] = sum;
					if ((dum = vv[i] * std::abs(sum)) >= big) {
						big = dum;
						imax = i;
					}
				}
				if (j != imax) { // Do we need to interchange rows?
					for (k = 0; k < n; ++k) { // Yes, do so...
						dum = val[imax][k];
						val[imax][k] = val[j][k];
						val[j][k] = dum;
					}
					d = -d;     // ...and change the parity of d.
					vv[imax] = vv[j]; // Also interchange the scale factor.
				}
				idx[j] = imax;
				if (j != n - 1) { // Now, finally, divide by the pivot element.
					dum = 1.0 / val[j][j];
					for (i = j + 1; i < n; ++i)
						val[i][j] *= dum;
				}
			} // Go back for the next column in the reduction.

			  // success
			free(vv);
			return true;
		}
		
		// print matrix to stream
		friend std::ostream& operator<< (std::ostream& out, const Basic_Matrix& M) {
			if (M.m == 0 || M.n == 0) {
				out << "[empty matrix]";
			}
			else {
				for (int32_t i = 0; i < M.m; ++i) {
					for (int32_t j = 0; j < M.n; ++j) {
						out << M.val[i][j];
						if (j != M.n - 1) out << "\t";
					}
					out << std::endl;
				}
			}
			return out;
		}

	private:
		void _allocateMemory(const int32_t m_, const int32_t n_) {
			assert((m_*n_ < MAXSIZE) && val == NULL);
			m = m_; n = n_;
			if (m == 0 || n == 0) return;

			val = (T**)malloc(m * sizeof(T*));
			val[0] = (T*)calloc(m*n, sizeof(T));
			for (int32_t i = 1; i < m; ++i)
				val[i] = val[i - 1] + n;
		}

		void _releaseMemory() {
			if (val != NULL) {
				free(val[0]);
				free(val);
			}
		}

	};

	template <typename T>
	void swap(Basic_Matrix<T>& a, Basic_Matrix<T>& b) {
		a.swap(b);
	}


	typedef Basic_Matrix<tfloat> Matrix;
	typedef Basic_Matrix<int32_t> Matrix_i;
	typedef Basic_Matrix<Complex> Matrix_c;
}
