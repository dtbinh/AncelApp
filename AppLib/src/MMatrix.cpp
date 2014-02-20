
#include "MMatrix.h"

#include <cassert>
#include <fstream>

using namespace ResUtil;

bool MMatrix::memoryAllocate()
{
	if(mRows > 0 && mCols > 0)
	{

		mVals = new double[mCols * mRows];
		if(mVals == NULL) 
			throw std::bad_alloc();
		return true;
	}
	return false;
}

void MMatrix::copy(const MMatrix& matA)
{
	assert(dimensionsMatch(matA));
	dcopy_(mCols * mRows,matA.mVals,1,mVals,1);
	mSymmetric = matA.isSymmetric();
	mTriangular = matA.isTriangular();
}
void MMatrix::axpy(const MMatrix& matX, double alpha)
{
	assert(dimensionsMatch(matX));
	daxpy_(mCols*mRows,alpha,matX.gets(),1,mVals,1);
}
void MMatrix::syrk(const MMatrix& matA, double alpha, double beta, const char* type, const char* trans)
{
	assert(isSymmetric() || beta == 0.0);
	int n(0),k(0);
	if(trans[0] == 'n' || trans[0] == 'N')
	{
		n = mCols;
		k = matA.sizeCol();
		assert(n == matA.sizeRow());
	}
	else if(trans[0] == 't' || trans[0] == 'T')
	{
		n = mRows;
		k = matA.sizeRow();
		assert(n == matA.sizeCol());
 	}
	dsyrk_(type, trans, n, k, alpha, matA.gets(),matA.sizeRow(), beta,mVals, mRows);
	copySymmetric(type[0]);
}
void MMatrix::syr(const MMatrix& matA, double alpha,const char* type)
{
	assert(isSymmetric());
    assert(matA.mCols==1);
    assert(matA.mRows==mRows);
	dsyr_(type, mRows, alpha, matA.mVals, 1, mVals, mRows);
    copySymmetric(type[0]);
}
int MMatrix::syev(MMatrix& eigVals, const char* jobz, const char* uplo, size_t lwork)
{
	assert(jobz[0]=='v' || jobz[0]=='V' || jobz[0]=='n' || jobz[0]=='N');
	assert(uplo[0]=='l' || uplo[0]=='L' || uplo[0]=='u' || uplo[0]=='U');
	assert(isSymmetric());
	assert(eigVals.getElemNum() == mRows);
	if(lwork < 3*mCols - 1)
		lwork = 3*mCols - 1;
	MMatrix work(1, lwork);
	int info = 0;
#ifndef _NOSYSEV
	dsyev_(jobz, uplo, mCols,  mVals,  mRows, eigVals.mVals, work.mVals, lwork, info);
	if(jobz[0]=='V' || jobz[0]=='v')
		setSymmetric(false);
	assert(info == 0);
#else // _NOSYSEV is defined
	 throw ndlexceptions::Error("Not able to access lapack DSYEV routine on this machine.");
#endif
	 return (int)work.get(0);
}
void MMatrix::axpyRowRow(size_t i, const MMatrix& x,size_t k, double alpha)
{
	assert(i < mRows);
    assert(k < x.mRows);
    assert(x.mCols== mCols);
    daxpy_(mCols, alpha, x.mVals + k, x.mRows, mVals + i, mRows);
}
void MMatrix::gemm(const MMatrix& matA, const MMatrix& matB, double alpha, 
						double beta, const char* transa, const char* transb)
{
	setSymmetric(false);
	int m(0),n(0),k(0);
	switch(transa[0])
	{
	case 'n':
	case 'N':
		m = matA.sizeRow();
		k = matA.sizeCol();
		break;
	case 'T':
	case 't':
		m = matA.sizeCol();
		k = matA.sizeRow();
		break;
	default:
		break;
	};
	switch(transb[0])
	{
	case 'n':
	case 'N':
		n = matB.sizeCol();
		assert(k == matB.sizeRow());
		break;
	case 't':
	case 'T':
		n = matB.sizeRow();
		assert(k == matB.sizeCol());
		break;
	default:
		break;
 	};
	assert(n == mCols && m == mRows);
	dgemm_(transa,transb,m,n,k,alpha,matA.gets(),matA.sizeRow(),
			matB.gets(),matB.sizeRow(),beta,mVals,mRows);
 
}

void MMatrix::gemvRowRow(size_t i, const MMatrix& A, const MMatrix& matX,size_t k, double alpha, double beta, const char* trans)
{
	if((trans[0] == 'n' || trans[0] == 'N')&&(A.mCols == matX.mCols && A.mRows == mCols)
		|| (trans[0] == 't' || trans[0] == 'T')&&(A.mRows == matX.mCols && A.mCols == mCols))
	{
		dgemv_(trans,A.mRows,mCols,alpha,A.mVals,A.mRows,
				matX.mVals + k,matX.mRows,beta,mVals + i,mRows);
 	}
}
void MMatrix::sumCol(const MMatrix& matA, const double alpha,const double beta)
{
	if((colsMatch(matA) && mRows == 1) || (mRows == matA.sizeCol()&& mCols == 1))
	{
		for(size_t i = 0; i < matA.sizeCol(); i++)
		{
			double sumVal = 0.0;
			for(size_t j = 0; j < matA.sizeRow(); j++)
			{
				sumVal += matA.get(j,i);
			}
			assign(get(i)*beta + sumVal*alpha,i);
 		}
	}
}
void MMatrix::SumRow(const MMatrix& matA, const double alpha,const double beta)
{
	if((rowsMatch(matA) && mCols == 1) || (mCols == matA.sizeRow()&& mRows == 1))
	{
 		for(size_t i = 0; i < matA.sizeRow(); i++)
		{
			double sumVal = 0.0;
			for(size_t j = 0; j < matA.sizeCol(); j++)
			{
				sumVal += matA.get(i,j);
			}
			assign(get(i)*beta + sumVal*alpha, i);
		}
	}
 }

void MMatrix::copySymmetric(const char type)
{
	switch(type)
	{
	case 'U':
	case 'u':
		for(size_t i = 0; i < mRows; i++)
			for(size_t j = 0; j < i; j++)
				mVals[i + j*mRows] = mVals[i*mCols + j];
		break;
	case 'L':
	case 'l':
		 for(size_t j = 0; j < mCols; j++)
			 for(size_t i = 0; i < j;  i++)
				mVals[i + mRows * j] = mVals[j + mCols*i];
 		break;
	default:
		break;
	}
}
void MMatrix::copyRowRow(size_t uI, const MMatrix& X, size_t uK)
{
	assert(X.mCols== mCols);
	assert(uI < mRows);
	assert(uK < X.mRows);
	dcopy_(mCols, X.mVals + uK, X.mRows, mVals+ uI, mRows);	
}
void MMatrix::copyColCol(size_t uJ, const MMatrix& X, size_t uK)
{
	assert(X.mRows== mRows);
	assert(uJ < mCols);
	assert(uK < X.mCols);
	dcopy_(mRows, X.mVals + uK* X.mRows, 1, mVals+ uJ * X.mRows, 1);
}
void MMatrix::scale(double alpha)
{
	size_t sumEle = mRows * mCols;
	for(size_t i = 0; i < sumEle; i++)
	{
		mVals[i] = mVals[i]*alpha;
	}
}
void MMatrix::scaleDiag(double alpha)
{
	assert(mRows == mCols);
	for(size_t i = 0; i < mCols; i++)
	{
		mVals[i*mRows + i] *= alpha;
	}
}
void MMatrix::scaleCol(size_t unCol, double alpha)
{
	for(size_t i = 0; i < mRows; i++)
	{
		mVals[unCol*mRows + i] = mVals[unCol*mRows + i] * alpha;
	}
}
void MMatrix::scaleRow(size_t unRow, double alpha)
{
	for(size_t i = 0; i < mCols; i++)
	{
		mVals[unRow + i*mRows] = mVals[unRow + i*mRows] * alpha;
	}
}
double MMatrix::max() const
{
	size_t uSize = mRows * mCols;
	assert(uSize > 0);

	double val = mVals[0];
	for(size_t t = 1; t < uSize; t++)
	{
		if(val < mVals[t])
			val = mVals[t];
	}
	return val;
}
double MMatrix::min() const
{
	size_t uSize = mRows * mCols;
	assert(uSize > 0);

	double val = mVals[0];
	for(size_t t = 1; t < uSize; t++)
	{
		if(val > mVals[t])
			val = mVals[t];
	}
	return val;
}
int  MMatrix::minIndex() const
{
	size_t uSize = mRows * mCols;
	assert(uSize > 0);

	int index = 0;
 	for(size_t t = 1; t < uSize; t++)
	{
		if(mVals[index] > mVals[t])
	 		index = t;
 	}
	return index;
}
MMatrix MMatrix::sumCol()
{
	 MMatrix S(1,sizeCol());
	 for(size_t i = 0; i < mCols; i++)
	 {
		 double sum = 0;
		 for(size_t j = 0; j < mRows; j++)
		 {
			 sum += get(j,i);
		 }
	 	 S.assign(sum,i);
	 }
	 return S;
}
MMatrix MMatrix::varCol()  // 方差的简化公式  
{
	MMatrix M = meanCol();
	double InvRows = 1.0/double(mRows);
	for(size_t i = 0; i < mCols; i++)
	{
	 	M.assign(colNorm2(i)*InvRows - M.get(i)*M.get(i),0,i);
 	}
	return M;
}
MMatrix MMatrix::varCol2()  // 方差的简化公式 
{
	MMatrix M = meanCol();
	MMatrix tep;
	tep = *this;
	for(size_t i = 0; i < tep.sizeRow(); i++)
	{
		tep.axpyRowRow(i,M,0,-1.0);
	}

	double InvRows = 1.0/double(mRows-1);

	for(size_t i = 0; i < mCols; i++)
	{
	 	M.assign(tep.colNorm2(i)*(InvRows),0,i);
 	}
	return M;
}
MMatrix MMatrix::meanCol()
{
	MMatrix M = sumCol();
	double InvRows = 1.0/double(mRows);
	for(size_t i = 0; i < mCols; i++)
	{
 		M.assign(M.get(0,i) * InvRows,0,i);
	}
	return M;
}
MMatrix MMatrix::sumRow()
{
	MMatrix S(mRows,1);
	for(size_t i = 0; i < mRows; i++)
	{
		 double sum = 0;
		 for(size_t j = 0; j < mCols; j++)
		 {
			 sum += get(i,j);
		 }
	 	 S.assign(sum,i,0);
	}
	return S;
}
MMatrix MMatrix::meanRow()
{
	//TODO
	MMatrix S(mRows,1);
	return S;
}
MMatrix MMatrix::varRow()
{
	//TODO
	MMatrix S(mRows,1);
	return S;
}
void MMatrix::addCol(size_t col,double val)
{
	for(size_t i=0; i< mRows; i++)
		mVals[i+ mRows*col] += val;
}
void MMatrix::transpose()
{
	if(mRows != 1 && mCols != 1)
	{
		MMatrix tep = *this;
		for(size_t i = 0; i < mCols; i++)
			for(size_t j = 0; j < mRows;j++)
				mVals[i + j * mCols] = tep.get(j,i);
	}
	if(!isSquare())
	{
		std::swap(mRows,mCols);
	}
}
void MMatrix::negate()
{
	scale(-1.0);
}
void MMatrix::zeroElements()
{
	size_t sumEle = mRows * mCols;
	for(size_t i = 0; i < sumEle; i++)
	{
		mVals[i] = 0.0;
	}
}
void MMatrix::oneElements()
{
	size_t sumEle = mRows * mCols;
	for(size_t i = 0; i < sumEle; i++)
	{
 		mVals[i] = 1.0;
	}
}
void MMatrix::resize(size_t nRows,size_t nCols)
{
	if(nRows != mRows || nCols != mCols)
	{
		if(mVals != NULL)
			   delete []mVals;
		mRows = nRows;
		mCols = nCols;
		memoryAllocate();
	}
}
double MMatrix::rowNorm(size_t nRow) const
{
	return dnrm2_(mCols, mVals + nRow, mRows);
}
double MMatrix::colNorm(size_t nCol) const
{
 	return dnrm2_(mRows, mVals +nCol*mRows,1);
}
double MMatrix::rowNorm2(size_t nRow) const
{
	double val = dnrm2_(mCols, mVals + nRow, mRows);
	return (val * val);
}
double MMatrix::colNorm2(size_t nCol) const
{
	double val = dnrm2_(mRows, mVals + nCol*mRows,1);
	return (val * val);
}	
double MMatrix::rowDist2(size_t i, const MMatrix& A, size_t k) const
{
	return rowNorm2(i) + A.rowNorm2(k) - 2.0*dotRowRow(i,A,k);
}
double MMatrix::colDist2(size_t i, const MMatrix& A, size_t k) const
{
	return colNorm2(i) + A.colNorm2(k) - 2.0*dotColCol(i,A,k);
}
double MMatrix::dotRowRow(size_t i, const MMatrix& A, size_t k) const
{
	return ddot_(mCols, A.mVals + k, A.mRows, mVals + i, mRows);
}
double MMatrix::dotRowCol(size_t i, const MMatrix& A, size_t k) const
{
	return ddot_(mCols, A.mVals + k * mRows, 1, mVals + i, mRows);
}
double MMatrix::dotColCol(size_t i, const MMatrix& A, size_t k) const
{
	return ddot_(mRows, A.mVals + k * A.mRows, 1, mVals + (i * mRows), 1);
}
double MMatrix::dotColRow(size_t i, const MMatrix& A, size_t k) const
{
	return ddot_(mRows, A.mVals + k, A.mRows, mVals + (i * mRows), 1);
}
// y:= alpha*A*x + beta*y
void MMatrix::symv(const MMatrix& A, const MMatrix& x, double alpha, double beta, const char* upperOrLower)
{
	assert(A.isSymmetric());
	assert(mCols==1 && x.mCols==1);
 	assert(upperOrLower[0]=='u' || upperOrLower[0]=='U' || upperOrLower[0]=='l' || upperOrLower[0]=='L');
	assert(mRows==A.mRows);
	assert(mRows==x.mRows);
  
	dsymv_(upperOrLower, A.mCols, alpha, A.mVals, A.mRows, 
			x.mVals, 1, beta, mVals, 1);
}
// y(i, :)' := alpha A*x(k, :)' + beta*y(i, :)';
void MMatrix::symvRowRow(size_t i, const MMatrix& A, const MMatrix& x, size_t k, double alpha, double beta, const char* upperOrLower)
{
	assert(A.isSymmetric());
	assert(i < mRows);
	assert(k < x.mRows);
	assert(upperOrLower[0]=='u' || upperOrLower[0]=='U' || upperOrLower[0]=='l' || upperOrLower[0]=='L');
	assert(mCols==A.mRows);
	assert(mCols==x.mCols);
	dsymv_(upperOrLower, A.mCols,alpha, A.mVals, A.mRows, 
		   x.mVals + k, x.mRows, beta, mVals + i,mRows);
}
// y(i, :)' := alpha A*x(:, j)  + beta*y(i, :)';
void MMatrix::symvRowCol(size_t i, const MMatrix& A, const MMatrix& x, size_t j, double alpha, double beta, const char* upperOrLower)
{
	 // Level 2 Blas operation, symmetric A,  y(i, :)' <- alpha A*x(:, j) + beta*y(i, :)';
	assert(A.isSymmetric());
	assert(i < mRows);
	assert(j < x.mRows);
 	assert(upperOrLower[0]=='u' || upperOrLower[0]=='U' || upperOrLower[0]=='l' || upperOrLower[0]=='L');
	assert(mCols==A.mRows);
	assert(mCols==x.mCols);

	dsymv_(upperOrLower, A.mCols, alpha, A.mVals, A.mRows, 
		   x.mVals + j * x.mRows, 1, beta, mVals + i, mRows);
}
// y(:, j)  := alpha A*x(:, k)  + beta*y(:, j);
void MMatrix::symvColCol(size_t j, const MMatrix& A, const MMatrix& x, size_t k, double alpha, double beta, const char* upperOrLower)
{
	assert(A.isSymmetric());
	assert(j < mCols);
	assert(k < x.mCols);
	assert(upperOrLower[0] == 'u' || upperOrLower[0] == 'U' || upperOrLower[0] == 'l' || upperOrLower[0] == 'L');
	assert(mRows == A.mRows);
	assert(A.mCols == A.mRows);
	assert(mRows == x.mRows);
	dsymv_(upperOrLower, A.mCols, alpha, A.mVals, A.mRows, 
			x.mVals + k * x.mRows, 1, beta, mVals + j*mRows, 1);
}
// y(:, j)  := alpha A*x(i, :)' + beta*y(:, j);
void MMatrix::symvColRow(size_t j, const MMatrix& A, const MMatrix& x, size_t i, double alpha, double beta, const char* upperOrLower)
{
	assert(A.isSymmetric());
	assert(j < mCols);
	assert(i < x.mCols);
	assert(upperOrLower[0] == 'u' || upperOrLower[0] == 'U' || upperOrLower[0] == 'l' || upperOrLower[0] == 'L');
	assert(mRows == A.mRows);
 	assert(mRows == x.mCols);
	dsymv_(upperOrLower, A.mCols, alpha, A.mVals, A.mRows, 
		   x.mVals + i, x.mRows, beta, mVals + j * mRows, 1);
}
// y(yr1:_, j) <- alpha A*x(xr1:_, k) + beta*y(yr1:_, j);

double MMatrix::sumElement() const
{
	double sum = 0.0;
	size_t sumEle = mRows * mCols;
	for(size_t i = 0; i < sumEle; i++)
		sum += mVals[i];
	return sum;
}
double MMatrix::trace() const
{
	if(isSquare()) {
 		double ttrace = 0.0;
 		for(size_t i = 0; i < mCols; i++)
			ttrace += mVals[i + mRows*i];
		return ttrace;
	}
	return 0;
}

void MMatrix::chol(const char* type)
{
	assert(isSymmetric()); // matrix should be symmetric.
	potrf(type);
	switch((int)type[0]) 
	{
	case 'L':
		for(size_t j = 0; j < mCols; j++)
			for(size_t i = 0; i<j; i++)
				mVals[i + mRows * j] = 0.0;
		break;
	case 'U':
		for(size_t i = 0; i< mRows; i++)
			for(size_t j = 0; j < i; j++)
				mVals[i + mRows * j] = 0.0;
		break;
	}
	setTriangular(true);    
}
void MMatrix::potrf(const char* type)
{
	assert(isSymmetric());
	int info;
	
	dpotrf_(type, mRows, mVals, mCols, info);
	
	setSymmetric(false);
	setTriangular(true);
	assert(info == 0);
}
void MMatrix::potri(const char* type)
{
	assert(isSquare());
	int info;
	dpotri_(type, mRows, mVals, mCols, info);
	assert(info == 0);

}
double	MMatrix::LogDet() const
{
	assert(isTriangular()); // should be chol decomp
	double logDet = 0.0;
	for(size_t i = 0; i < sizeRow(); i++)
		logDet += std::log(get(i, i));
	logDet *= 2;
	return logDet;
}
double MMatrix::invertMMatrix(MMatrix& mat)
{
	mat.setSymmetric(true);
	MMatrix LcholK = mat;
	LcholK.chol("U");
  	mat.pdinv(LcholK);
	return LcholK.LogDet();
}
void   MMatrix::pdinv(const MMatrix& U)
{
	assert(U.isTriangular()); // U should be cholesky decomposition.
	assert(isSymmetric()); // matrix should be symmetric.
	*this = U;
	potri("U");
  // make matrix symmetric.
	for(size_t i = 0; i< mRows; i++)
	{
		for(size_t j = 0; j < i; j++)
		{
			mVals[i + mRows*j] = mVals[j + mCols*i];
		}
	}
	setSymmetric(true);
}

void MMatrix::setMMatrix(size_t uRow,size_t uCol,const MMatrix & A)
{
	assert(uRow+A.mRows <= mRows);
    assert(uCol+A.mCols <= mCols);

	for(size_t i = 0; i < A.mRows; i++)
	{
		for(size_t j = 0; j < A.mCols; j++)
		{
			mVals[i + uRow + mRows*(j + uCol)] = A.mVals[i + A.mRows * j];
		}
	}
}
void MMatrix::copyMMatrix(size_t uRow,size_t uCol,const MMatrix & A,size_t rowMin,size_t rowMax,size_t colMin,size_t colMax)
{
	assert(uRow + (rowMax - rowMin) <= mRows);
	assert(uCol + (colMax - colMin) <= mCols);
 
	for(size_t i = rowMin; i < rowMax; i++)
	{
		for(size_t j = colMin; j < colMax; j++)
		{
			mVals[(i-rowMin) + uRow + mRows*((j-colMin) + uCol)] = A.mVals[i + A.mRows * j];
		}
	}
}

MMatrix MMatrix::subMMatrix(std::size_t row,std::size_t col,std::size_t rowNum,std::size_t colNum)
{
	assert(row + rowNum <= mRows);
	assert(col + colNum <= mCols);

	MMatrix retMat(rowNum,colNum);
	for(size_t i = 0; i < rowNum; i++)
	{
		for(size_t j = 0; j < colNum; j++)
		{
			retMat.mVals[i * colNum + j] = mVals[(row + i) * mCols + col + j];
		}
	}
	return retMat;
}

int  MMatrix::sysv(const MMatrix& A, const char* uplo, int lwork)
{
	assert(A.isSymmetric());
	assert(uplo[0] == 'L' || uplo[0] == 'l' || uplo[0] == 'U' || uplo[0] == 'u');
	assert(mRows == A.mRows);
	if(lwork < 0)
		lwork = 3*mRows;
	int info;
	std::vector<int> ipivv(mRows);
	int *ipiv = &ipivv[0];
	MMatrix work(1, lwork);

	dsysv_(uplo, mRows, mCols, A.mVals, A.mRows, ipiv, mVals, mRows, work.mVals, lwork, info);
	assert(info == 0);
	return (int)work.get(0); // optimal value for lwork
}
//void MMatrix::gels(const MMatrix& A,const MMatrix&B,const char* trans)
//{
//	assert(trans == "T" || trans == "N");
//	int M,N;
//	if(trans == "N")
//	{
//		M = A.sizeRow();
//		N = A.sizeCol();
//	}
//	else 
//	{
//		M = A.sizeCol();
//		N = A.sizeRow();
// 	}
//	int info = -1;
//	for(std::size_t i = 0;i < B.mRows; i++)
//		mVals[i] = B.mVals[i];
//	int lwork = 3*mRows;
//	MMatrix work(1, lwork);
//	dgels_(trans,M,N,B.mCols,A.mVals,A.mRows,mVals,mRows,work.mVals,lwork,info);
//	assert(info == 0);
//}
void MMatrix::symm(const MMatrix& A, const MMatrix& B, double alpha, double beta, const char* type, const char* side)
{
	assert(A.isSymmetric());
	assert(side[0]=='L' || side[0]=='l' || side[0]=='R' || side[0]=='r');
	assert(type[0]=='L' || type[0]=='l' || type[0]=='U' || type[0]=='u');
	switch(side[0])
    {
	case 'L':
    case 'l':
		assert(A.mRows == mRows);
		assert(B.mRows == mRows);
		assert(B.mCols == mCols);
		break;
    case 'R':
    case 'r':
		assert(A.mCols == mCols);
		assert(B.mCols == A.mRows);
		assert(B.mRows == mRows);
		break;
    default:
		break;
    }
	dsymm_(side, type, mRows, mCols, alpha, A.mVals, A.mRows, B.mVals, B.mRows, beta, mVals, mRows);
}
void MMatrix::repmat(const MMatrix &X,size_t M,size_t N)
{
	resize(X.sizeRow()*M,X.sizeCol()*N);
	for(size_t i = 0; i < X.sizeCol(); i++)
	{
 		for(size_t j = 0; j < N; j++)
		{
			for(size_t k = 0; k < M; k++)
			{
				size_t offset = i*X.sizeRow()*M + (k + j*X.sizeCol()*M)*X.sizeRow();
				dcopy_(X.sizeRow(),X.mVals + i*X.sizeRow(),1,mVals + offset,1);
			}
		}
	}
}
void  MMatrix::submat(const MMatrix &X,size_t row,size_t M,size_t col,size_t N)
{
	assert(row + M <= X.sizeRow() && col + N <= X.sizeCol());
	resize(M,N);

	for(size_t i = 0; i < N;i++)
	{
		int offset = row + (i+col)*X.sizeRow();
		dcopy_(M,X.mVals + offset,1,mVals + i*M,1);
	}
}
void  MMatrix::pca(MMatrix &eigenVector,MMatrix &eigenValue)
{
	MMatrix meanVal = meanCol();
	eigenVector.resize(sizeCol(),sizeCol());
	eigenVector.setSymmetric(true);
	eigenVector.syrk(*this, 1.0 / (double)sizeRow(), 0.0, "u", "t");
 
	meanVal.transpose();
	eigenVector.syr(meanVal, -1.0, "u");
 
	eigenValue.resize(1,eigenVector.sizeCol());
	eigenVector.syev(eigenValue, "v", "u");
}
void MMatrix::dotOp(const MMatrix&X,double factor)
{
	assert(X.sizeCol() == sizeCol() && X.sizeRow() == sizeRow());
 	daxpy_(mCols*mRows, factor,X.mVals,1,mVals,1);
}
void MMatrix::dotOp2(const MMatrix&X, double factor)
{
	assert(X.sizeCol() == sizeCol() && X.sizeRow() == sizeRow());
 	daxpy_(mCols*mRows,factor,X.mVals,1,mVals,1);
	ddot_(mCols*mRows,mVals,1,mVals,1);
}
void  MMatrix::dotMul(const MMatrix&X)
{
	assert(dimensionsMatch(X));
	size_t sumElem = mRows * mCols;
	for(size_t i = 0; i < sumElem; i++)
		mVals[i] *= X.mVals[i];
}
void MMatrix::pow(int times)
{
	size_t sumElem = mRows * mCols;
	for(size_t i = 0; i < sumElem; i++)
	{	
		double val = mVals[i];
		for(int j = 0; j < times; j++)
			mVals[i] *= val;
	}
}
void MMatrix::identity()
{
	assert(mCols == mRows);
	zeroElements();
	for(size_t t = 0; t < mCols; t++)
 		assign(1.0,t,t);
}
bool MMatrix::Equals(const MMatrix& matA, double tol) const
{
	return true;
}
void MMatrix::copycol(size_t i,const MMatrix &X,size_t k,int ystart,int incy,int xstart,int incx,int xend)
{
	assert(xend < (int)X.mRows && xstart >=0 && xstart < (int)X.mRows);
	assert(ystart < (int)mRows && ystart >=0 && ystart < (int)mRows);
	assert(incx > 0 && incy > 0);
	if(xend < 0)
		dcopy_(X.mRows - xstart,X.mVals + xstart,incx,mVals + ystart,incy);
	else
		dcopy_(xend - xstart,X.mVals + xstart,incx,mVals + ystart,incy);
}
MMatrix MMatrix::kron(const MMatrix &mat)
{
	MMatrix ret(sizeRow() * mat.sizeRow(), sizeCol() * mat.sizeCol());
	for (std::size_t i = 0; i < mRows; i++)
	{
		for (std::size_t j = 0; j < mCols; j++)
		{
			MMatrix temp = mat;
			temp.scale(get(i,j));
			ret.setMMatrix(i * mat.sizeRow(), j * mat.sizeCol(), temp);
		}
	}
	return ret;
}