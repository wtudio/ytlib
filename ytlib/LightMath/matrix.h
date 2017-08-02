#ifndef MATRIX_H
#define MATRIX_H

#include <ytlib/LightMath/mathbase.h>
#include <ytlib/LightMath/complex.h>
#include <string>
#include <iostream>


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
			for (int32_t i = 0; i < M.m; ++i)
				memcpy(val[i], M.val[i], M.n * sizeof(T));
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
				if (M.n > 0)
					for (int32_t i = 0; i < M.m; ++i)
						memcpy(val[i], M.val[i], M.n * sizeof(T));
			}
			return *this;
		}

		// copies submatrix of M into array 'val', default values copy whole row/column/matrix
		void getData(T* val_, int32_t i1 = 0, int32_t j1 = 0, int32_t i2 = -1, int32_t j2 = -1, int32_t N_ = 0) {
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
		Basic_Matrix getMat(int32_t i1, int32_t j1, int32_t i2 = -1, int32_t j2 = -1) {
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
			if (i2 == -1) i2 = min(m - 1, n - 1);
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
		Basic_Matrix extractCols(const int32_t* idx,const int32_t N_) {
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


		/////tmp:测试到这。还需要添加随机矩阵生成方法。再看看matlab里还有什么常用矩阵

		// simple arithmetic operations
		// add matrix
		Basic_Matrix  operator+ (const Basic_Matrix &M) {
			const Basic_Matrix &A = *this;
			const Basic_Matrix &B = M;
			if (A.m != B.m || A.n != B.n) {
				std::cerr << "ERROR: Trying to add matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << endl;
				exit(0);
			}
			Basic_Matrix C(A.m, A.n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[i][j] = A.val[i][j] + B.val[i][j];
			return C;
		}
		// subtract matrix
		Basic_Matrix  operator- (const Basic_Matrix &M) {
			const Basic_Matrix &A = *this;
			const Basic_Matrix &B = M;
			if (A.m != B.m || A.n != B.n) {
				std::cerr << "ERROR: Trying to subtract matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << endl;
				exit(0);
			}
			Basic_Matrix C(A.m, A.n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[i][j] = A.val[i][j] - B.val[i][j];
			return C;
		}
		// multiply with matrix
		Basic_Matrix  operator* (const Basic_Matrix &M) {
			const Basic_Matrix &A = *this;
			const Basic_Matrix &B = M;
			if (A.n != B.m) {
				std::cerr << "ERROR: Trying to multiply matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << endl;
				exit(0);
			}
			Basic_Matrix C(A.m, B.n);
			for (int32_t i = 0; i < A.m; ++i)
				for (int32_t j = 0; j < B.n; ++j)
					for (int32_t k = 0; k < A.n; ++k)
						C.val[i][j] += A.val[i][k] * B.val[k][j];
			return C;
		}
		// multiply with scalar
		Basic_Matrix  operator* (const T &s) {
			Basic_Matrix C(m, n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[i][j] = val[i][j] * s;
			return C;
		}

		// divide elementwise by matrix (or vector)
		Basic_Matrix  operator/ (const Basic_Matrix &M) {
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
					") and (" << B.m << "x" << B.n << ")" << endl;
				exit(0);
			}
		}
		// divide by scalar
		Basic_Matrix  operator/ (const T &s) {
			if (abs(s) < 1e-20) {
				std::cerr << "ERROR: Trying to divide by zero!" << endl;
				exit(0);
			}
			Basic_Matrix C(m, n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[i][j] = val[i][j] / s;
			return C;
		}

		// negative matrix
		Basic_Matrix  operator- () {
			Basic_Matrix C(m, n);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[i][j] = -val[i][j];
			return C;
		}
		// transpose
		Basic_Matrix  operator~ () {
			Basic_Matrix C(n, m);
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					C.val[j][i] = val[i][j];
			return C;
		}
		// euclidean norm (vectors) / frobenius norm (matrices)
		T   l2norm() {
			T norm = 0;
			for (int32_t i = 0; i < m; ++i)
				for (int32_t j = 0; j < n; ++j)
					norm += val[i][j] * val[i][j];
			return sqrt(norm);
		}
		// mean of all elements in matrix
		T   mean() {
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
				std::cerr << "ERROR: Cross product vectors must be of size (3x1)" << endl;
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
				std::cerr << "ERROR: Trying to invert matrix of size (" << M.m << "x" << M.n << ")" << endl;
				exit(0);
			}
			Basic_Matrix A(M);
			Basic_Matrix B = eye(M.m);
			B.solve(A);
			return B;
		}
		// invert this matrix
		bool   inv() {
			if (m != n) {
				std::cerr << "ERROR: Trying to invert matrix of size (" << m << "x" << n << ")" << endl;
				exit(0);
			}
			Basic_Matrix A(*this);
			eye();
			solve(A);
			return true;
		}
		// returns determinant of matrix
		T  det() {

			if (m != n) {
				std::cerr << "ERROR: Trying to compute determinant of a matrix of size (" << m << "x" << n << ")" << endl;
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
		// solve linear system M*x=B, replaces *this and M
		bool   solve(const Basic_Matrix &M, tfloat eps = 1e-20) {

			// substitutes
			const Basic_Matrix &A = M;
			Basic_Matrix &B = *this;

			if (A.m != A.n || A.m != B.m || A.m < 1 || B.n < 1) {
				std::cerr << "ERROR: Trying to eliminate matrices of size (" << A.m << "x" << A.n <<
					") and (" << B.m << "x" << B.n << ")" << endl;
				exit(0);
			}

			// index vectors for bookkeeping on the pivoting
			int32_t* indxc = new int32_t[m];
			int32_t* indxr = new int32_t[m];
			int32_t* ipiv = new int32_t[m];

			// loop variables
			int32_t i, icol, irow, j, k, l, ll;
			tfloat big, dum, pivinv, temp;

			// initialize pivots to zero
			for (j = 0; j < m; ++j) ipiv[j] = 0;

			// main loop over the columns to be reduced
			for (i = 0; i < m; ++i) {

				big = 0.0;

				// search for a pivot element
				for (j = 0; j < m; ++j)
					if (ipiv[j] != 1)
						for (k = 0; k < m; ++k)
							if (ipiv[k] == 0)
								if (abs(A.val[j][k]) >= big) {
									big = abs(A.val[j][k]);
									irow = j;
									icol = k;
								}
				++(ipiv[icol]);

				// We now have the pivot element, so we interchange rows, if needed, to put the pivot
				// element on the diagonal. The columns are not physically interchanged, only relabeled.
				if (irow != icol) {
					for (l = 0; l < m; ++l) SWAP(A.val[irow][l], A.val[icol][l])
						for (l = 0; l < n; ++l) SWAP(B.val[irow][l], B.val[icol][l])
				}

				indxr[i] = irow; // We are now ready to divide the pivot row by the
				indxc[i] = icol; // pivot element, located at irow and icol.

								 // check for singularity
				if (abs(A.val[icol][icol]) < eps) {
					delete[] indxc;
					delete[] indxr;
					delete[] ipiv;
					return false;
				}

				pivinv = 1.0 / A.val[icol][icol];
				A.val[icol][icol] = 1.0;
				for (l = 0; l < m; ++l) A.val[icol][l] *= pivinv;
				for (l = 0; l < n; ++l) B.val[icol][l] *= pivinv;

				// Next, we reduce the rows except for the pivot one
				for (ll = 0; ll < m; ll++)
					if (ll != icol) {
						dum = A.val[ll][icol];
						A.val[ll][icol] = 0.0;
						for (l = 0; l < m; ++l) A.val[ll][l] -= A.val[icol][l] * dum;
						for (l = 0; l < n; ++l) B.val[ll][l] -= B.val[icol][l] * dum;
					}
			}

			// This is the end of the main loop over columns of the reduction. It only remains to unscramble
			// the solution in view of the column interchanges. We do this by interchanging pairs of
			// columns in the reverse order that the permutation was built up.
			for (l = m - 1; l >= 0; l--) {
				if (indxr[l] != indxc[l])
					for (k = 0; k < m; ++k)
						SWAP(A.val[k][indxr[l]], A.val[k][indxc[l]])
			}

			// success
			delete[] indxc;
			delete[] indxr;
			delete[] ipiv;
			return true;
		}
		// replace *this by lower upper decomposition
		bool   lu(int32_t *idx, tfloat &d, tfloat eps = 1e-20) {

			if (m != n) {
				std::cerr << "ERROR: Trying to LU decompose a matrix of size (" << m << "x" << n << ")" << endl;
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
					if ((dum = vv[i] * abs(sum)) >= big) {
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
		// singular value decomposition *this = U*diag(W)*V^T
		void   svd(Basic_Matrix &U, Basic_Matrix &W, Basic_Matrix &V) {

			Basic_Matrix U = Basic_Matrix(*this);
			U2 = Basic_Matrix(m, m);
			V = Basic_Matrix(n, n);

			T* w = (T*)malloc(n * sizeof(T));
			T* rv1 = (T*)malloc(n * sizeof(T));

			int32_t flag, i, its, j, jj, k, l, nm;
			tfloat   anorm, c, f, g, h, s, scale, x, y, z;

			g = scale = anorm = 0.0; // Householder reduction to bidiagonal form.
			for (i = 0; i < n; ++i) {
				l = i + 1;
				rv1[i] = scale*g;
				g = s = scale = 0.0;
				if (i < m) {
					for (k = i; k < m; ++k) scale += abs(U.val[k][i]);
					if (scale) {
						for (k = i; k < m; ++k) {
							U.val[k][i] /= scale;
							s += U.val[k][i] * U.val[k][i];
						}
						f = U.val[i][i];
						g = -SIGN(sqrt(s), f);
						h = f*g - s;
						U.val[i][i] = f - g;
						for (j = l; j < n; ++j) {
							for (s = 0.0, k = i; k < m; ++k) s += U.val[k][i] * U.val[k][j];
							f = s / h;
							for (k = i; k < m; ++k) U.val[k][j] += f*U.val[k][i];
						}
						for (k = i; k < m; ++k) U.val[k][i] *= scale;
					}
				}
				w[i] = scale*g;
				g = s = scale = 0.0;
				if (i < m && i != n - 1) {
					for (k = l; k < n; ++k) scale += abs(U.val[i][k]);
					if (scale) {
						for (k = l; k < n; ++k) {
							U.val[i][k] /= scale;
							s += U.val[i][k] * U.val[i][k];
						}
						f = U.val[i][l];
						g = -SIGN(sqrt(s), f);
						h = f*g - s;
						U.val[i][l] = f - g;
						for (k = l; k < n; ++k) rv1[k] = U.val[i][k] / h;
						for (j = l; j < m; ++j) {
							for (s = 0.0, k = l; k < n; ++k) s += U.val[j][k] * U.val[i][k];
							for (k = l; k < n; ++k) U.val[j][k] += s*rv1[k];
						}
						for (k = l; k < n; ++k) U.val[i][k] *= scale;
					}
				}
				anorm = FMAX(anorm, (abs(w[i]) + abs(rv1[i])));
			}
			for (i = n - 1; i >= 0; i--) { // Accumulation of right-hand transformations.
				if (i < n - 1) {
					if (g) {
						for (j = l; j < n; ++j) // Double division to avoid possible underflow.
							V.val[j][i] = (U.val[i][j] / U.val[i][l]) / g;
						for (j = l; j < n; ++j) {
							for (s = 0.0, k = l; k < n; ++k) s += U.val[i][k] * V.val[k][j];
							for (k = l; k < n; ++k) V.val[k][j] += s*V.val[k][i];
						}
					}
					for (j = l; j < n; ++j) V.val[i][j] = V.val[j][i] = 0.0;
				}
				V.val[i][i] = 1.0;
				g = rv1[i];
				l = i;
			}
			for (i = IMIN(m, n) - 1; i >= 0; i--) { // Accumulation of left-hand transformations.
				l = i + 1;
				g = w[i];
				for (j = l; j < n; ++j) U.val[i][j] = 0.0;
				if (g) {
					g = 1.0 / g;
					for (j = l; j < n; ++j) {
						for (s = 0.0, k = l; k < m; ++k) s += U.val[k][i] * U.val[k][j];
						f = (s / U.val[i][i])*g;
						for (k = i; k < m; ++k) U.val[k][j] += f*U.val[k][i];
					}
					for (j = i; j < m; ++j) U.val[j][i] *= g;
				}
				else for (j = i; j < m; ++j) U.val[j][i] = 0.0;
				++U.val[i][i];
			}
			for (k = n - 1; k >= 0; k--) { // Diagonalization of the bidiagonal form: Loop over singular values,
				for (its = 0; its < 30; ++its) { // and over allowed iterations.
					flag = 1;
					for (l = k; l >= 0; l--) { // Test for splitting.
						nm = l - 1;
						if ((T)(abs(rv1[l]) + anorm) == anorm) { flag = 0; break; }
						if ((T)(abs(w[nm]) + anorm) == anorm) { break; }
					}
					if (flag) {
						c = 0.0; // Cancellation of rv1[l], if l > 1.
						s = 1.0;
						for (i = l; i <= k; ++i) {
							f = s*rv1[i];
							rv1[i] = c*rv1[i];
							if ((T)(abs(f) + anorm) == anorm) break;
							g = w[i];
							h = pythag(f, g);
							w[i] = h;
							h = 1.0 / h;
							c = g*h;
							s = -f*h;
							for (j = 0; j < m; ++j) {
								y = U.val[j][nm];
								z = U.val[j][i];
								U.val[j][nm] = y*c + z*s;
								U.val[j][i] = z*c - y*s;
							}
						}
					}
					z = w[k];
					if (l == k) { // Convergence.
						if (z < 0.0) { // Singular value is made nonnegative.
							w[k] = -z;
							for (j = 0; j < n; ++j) V.val[j][k] = -V.val[j][k];
						}
						break;
					}
					if (its == 29)
						std::cerr << "ERROR in SVD: No convergence in 30 iterations" << endl;
					x = w[l]; // Shift from bottom 2-by-2 minor.
					nm = k - 1;
					y = w[nm];
					g = rv1[nm];
					h = rv1[k];
					f = ((y - z)*(y + z) + (g - h)*(g + h)) / (2.0*h*y);
					g = pythag(f, 1.0);
					f = ((x - z)*(x + z) + h*((y / (f + SIGN(g, f))) - h)) / x;
					c = s = 1.0; // Next QR transformation:
					for (j = l; j <= nm; ++j) {
						i = j + 1;
						g = rv1[i];
						y = w[i];
						h = s*g;
						g = c*g;
						z = pythag(f, h);
						rv1[j] = z;
						c = f / z;
						s = h / z;
						f = x*c + g*s;
						g = g*c - x*s;
						h = y*s;
						y *= c;
						for (jj = 0; jj < n; jj++) {
							x = V.val[jj][j];
							z = V.val[jj][i];
							V.val[jj][j] = x*c + z*s;
							V.val[jj][i] = z*c - x*s;
						}
						z = pythag(f, h);
						w[j] = z; // Rotation can be arbitrary if z = 0.
						if (z) {
							z = 1.0 / z;
							c = f*z;
							s = h*z;
						}
						f = c*g + s*y;
						x = c*y - s*g;
						for (jj = 0; jj < m; jj++) {
							y = U.val[jj][j];
							z = U.val[jj][i];
							U.val[jj][j] = y*c + z*s;
							U.val[jj][i] = z*c - y*s;
						}
					}
					rv1[l] = 0.0;
					rv1[k] = f;
					w[k] = x;
				}
			}

			// sort singular values and corresponding columns of u and v
			// by decreasing magnitude. Also, signs of corresponding columns are
			// flipped so as to maximize the number of positive elements.
			int32_t s2, inc = 1;
			tfloat   sw;
			T* su = (T*)malloc(m * sizeof(T));
			T* sv = (T*)malloc(n * sizeof(T));
			do { inc *= 3; inc++; } while (inc <= n);
			do {
				inc /= 3;
				for (i = inc; i < n; ++i) {
					sw = w[i];
					for (k = 0; k < m; ++k) su[k] = U.val[k][i];
					for (k = 0; k < n; ++k) sv[k] = V.val[k][i];
					j = i;
					while (w[j - inc] < sw) {
						w[j] = w[j - inc];
						for (k = 0; k < m; ++k) U.val[k][j] = U.val[k][j - inc];
						for (k = 0; k < n; ++k) V.val[k][j] = V.val[k][j - inc];
						j -= inc;
						if (j < inc) break;
					}
					w[j] = sw;
					for (k = 0; k < m; ++k) U.val[k][j] = su[k];
					for (k = 0; k < n; ++k) V.val[k][j] = sv[k];
				}
			} while (inc > 1);
			for (k = 0; k < n; ++k) { // flip signs
				s2 = 0;
				for (i = 0; i < m; ++i) if (U.val[i][k] < 0.0) ++s2;
				for (j = 0; j < n; ++j) if (V.val[j][k] < 0.0) ++s2;
				if (s2 > (m + n) / 2) {
					for (i = 0; i < m; ++i) U.val[i][k] = -U.val[i][k];
					for (j = 0; j < n; ++j) V.val[j][k] = -V.val[j][k];
				}
			}

			// create vector and copy singular values
			W = Basic_Matrix(min(m, n), 1, w);

			// extract mxm submatrix U
			U2.setMat(U.getMat(0, 0, m - 1, min(m - 1, n - 1)), 0, 0);

			// release temporary memory
			free(w);
			free(rv1);
			free(su);
			free(sv);
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
						if (j != M.n - 1) out << " | ";
					}
					if (i < M.m - 1)
						out << std::endl;
				}
			}
			return out;
		}

	private:
		void _allocateMemory(const int32_t m_, const int32_t n_) {
			assert((m_*n_ < MAXSIZE) && (m_ >= 0) && (n_ >= 0));
			m = m_; n = n_;
			if (m == 0 || n == 0) {
				val = 0;
				return;
			}
			val = (T**)malloc(m * sizeof(T*));
			val[0] = (T*)calloc(m*n, sizeof(T));
			for (int32_t i = 1; i < m; ++i)
				val[i] = val[i - 1] + n;
		}

		void _releaseMemory() {
			if (val != 0) {
				free(val[0]);
				free(val);
			}
		}

		T pythag(T a, T b) {
			tfloat absa, absb;
			absa = abs(a);
			absb = abs(b);
			if (absa > absb)
				return absa*sqrt(1.0 + SQR(absb / absa));
			else
				return (absb == 0.0 ? 0.0 : absb*sqrt(1.0 + SQR(absa / absb)));
		}


	};

	typedef Basic_Matrix<tfloat> Matrix;
	typedef Basic_Matrix<Complex> Matrix_c;
}
#endif // MATRIX_H
