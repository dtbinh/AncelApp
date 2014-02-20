#ifndef __MMatrix_h_
#define __MMatrix_h_

#include <vector>
#include <iostream>
#include <cassert>
#include "cblaswrap.h"

namespace ResUtil
{
 	using std::size_t;

	class MMatrix
	{
	public:
 		MMatrix();
		MMatrix(double val);
 		MMatrix(size_t nRows, size_t nCols);
		MMatrix(size_t nRows, size_t nCols, double val);
		MMatrix(size_t nRows, size_t nCols, double* pvals);
		MMatrix(size_t nRows, size_t nCols, std::vector<double> vVals);
		MMatrix(size_t nRows, size_t nCols, char cType);
		MMatrix(size_t nRows, size_t nCols, double val,char cType);
  		MMatrix(const MMatrix& matA);
		~MMatrix();
		//------------------------------------------------------------------------------------
		void copy(const MMatrix& matA);
	 
		void axpy(const MMatrix& matX, double alpha);
		void axpyRowRow(size_t i, const MMatrix& x,size_t k, double alpha);
		void syrk(const MMatrix& matA, double alpha, double beta, const char* type, const char* trans);
		void syr(const MMatrix& matA, double alpha,const char* trans);
		void symm(const MMatrix& A, const MMatrix& B, double alpha, double beta, const char* upperLower, const char* side);
 		int  sysv(const MMatrix& A, const char* uplo, int lwork=-1);
		//void gels(const MMatrix& A,const MMatrix& B,const char* trans);
		 // y:= alpha*A*x + beta*y
		void symv(const MMatrix& A, const MMatrix& x, double alpha, double beta, const char* upperOrLower);
 		// y(i, :)' := alpha A*x(k, :)' + beta*y(i, :)';
		void symvRowRow(size_t i, const MMatrix& A, const MMatrix& x, size_t k, double alpha, double beta, const char* upperOrLower);
		// y(i, :)' := alpha A*x(:, j)  + beta*y(i, :)';
		void symvRowCol(size_t i, const MMatrix& A, const MMatrix& x, size_t j, double alpha, double beta, const char* upperOrLower);
		// y(:, j)  := alpha A*x(:, k)  + beta*y(:, j);
		void symvColCol(size_t j, const MMatrix& A, const MMatrix& x, size_t k, double alpha, double beta, const char* upperOrLower);
		// y(:, j)  := alpha A*x(i, :)' + beta*y(:, j);
		void symvColRow(size_t j, const MMatrix& A, const MMatrix& x, size_t i, double alpha, double beta, const char* upperOrLower);
		// y(yr1:_, j) <- alpha A*x(xr1:_, k) + beta*y(yr1:_, j);
 
		int		syev(MMatrix& eigVals, const char* jobz, const char* uplo, size_t lwork = 0);
		void	gemm(const MMatrix& matA, const MMatrix& matB, double alpha, double beta, const char* transa, const char* transb);
		void	gemvRowRow(size_t i, const MMatrix& A, const MMatrix& matX,size_t k, double alpha, double beta, const char* trans);
		void	chol(const char* type);
		void	potrf(const char* type);
		void	potri(const char* type);
		void    pdinv(const MMatrix& U);
		void	repmat(const MMatrix &X,size_t M,size_t N);
		void    submat(const MMatrix &X,size_t row,size_t M,size_t cols,size_t N);
		void    pca(MMatrix &eigenVector,MMatrix &eigenValue);
		void	dotOp(const MMatrix&X, double factor);
		void	dotOp2(const MMatrix&X, double factor);
 		void    dotMul(const MMatrix&X);
		size_t  sizeRow() const;
		size_t  sizeCol() const;
		size_t  getElemNum() const;

		MMatrix kron(const MMatrix &mat);
		
		double	LogDet() const;
		
		double* gets();
		void	resize(size_t nRows,size_t nCols);
		const	double* gets() const;
	 	
 		void assign(double val);
		void assign(double val, size_t unIndex);
		void assign(double val, size_t unRows, size_t unCols);
		
		double get(size_t unRows, size_t unCols) const;
		double get(size_t unIndex) const;

		
		void add(double val, size_t unIndex);
		void add(double val, size_t unRows, size_t unCols);

 		const bool isAnyNaN() const;
		const bool isAnyINF() const;
		const bool isSquare() const;
		const bool isTriangular() const;
		const bool isSymmetric() const;

		void setTriangular(const bool bVal);
		void setSymmetric (const bool bVal);

		const bool dimensionsMatch(const MMatrix& matA) const;
		const bool rowsMatch(const MMatrix& matA) const;
		const bool colsMatch(const MMatrix& matA) const;

		
		double trace() const;
		double sumElement() const;

		void sumCol(const MMatrix& matA, const double alpha,const double beta);
		void SumRow(const MMatrix& matA, const double alpha,const double beta);

		void copycol(size_t i,const MMatrix &X,size_t k,int ystart = 0,int incy = 1,int xstart = 0,int incx = 1,int xend = -1);
		void copySymmetric(const char type);
 		void copyRowRow(size_t unI, const MMatrix& X, size_t unK);
	    void copyColCol(size_t unJ, const MMatrix& X, size_t unK);
		void copyMMatrix(size_t uRow,size_t uCol,const MMatrix & A,size_t rowMin,size_t rowMax,size_t colMin,size_t colMax);
		
		MMatrix subMMatrix(std::size_t row,std::size_t col,std::size_t rowNum = 1,std::size_t colNum = 1);

		void scale(double alpha);
		void scaleDiag(double alpha);
 		void scaleCol(size_t unCol, double alpha);
		void scaleRow(size_t unRow, double alpha);

		double max() const;
		double min() const;
		int  minIndex() const;
		void negate();
		void transpose();
		void oneElements();
		void zeroElements();
		void identity();

		MMatrix sumCol();
		MMatrix varCol();
		MMatrix varCol2();
		MMatrix meanCol();

		MMatrix sumRow();
		MMatrix meanRow();
		MMatrix varRow();

		void pow(int times);
		void addCol(size_t col,double val);
	 	bool Equals(const MMatrix& matA, double tol) const;
		
		void setMMatrix(size_t uRow,size_t uCol,const MMatrix & A);

		double rowNorm(size_t nRow) const;
		double colNorm(size_t nCol) const;
		double rowNorm2(size_t nRow) const;
		double colNorm2(size_t nCol) const;

		double rowDist2(size_t i, const MMatrix& A, size_t k) const;
		double colDist2(size_t i, const MMatrix& A, size_t k) const;

		double dotRowRow(size_t i, const MMatrix& A, size_t k) const;
		double dotRowCol(size_t i, const MMatrix& A, size_t k) const;
		double dotColCol(size_t i, const MMatrix& A, size_t k) const;
		double dotColRow(size_t i, const MMatrix& A, size_t k) const;
		
		MMatrix operator*(const MMatrix& mat); 
		void operator = (const MMatrix& mat);
		void operator += (const MMatrix& mat);
		void operator -= (const MMatrix& mat);
		static double invertMMatrix(MMatrix&mat);
		friend std::ostream& operator << (std::ostream &os,const MMatrix & Mat);
 	private:
		bool memoryAllocate();
	private:
 		double *mVals;

		bool mSymmetric;
		bool mTriangular;

		size_t mRows;
		size_t mCols;
	};
	//----------------------------------------------------inline functions-------------------------------
	inline MMatrix::MMatrix()
	{
		mRows = 1;
		mCols = 1;
		mSymmetric  = false;
		mTriangular = false;
		memoryAllocate();
		mVals[0] = 0;
 	}
	inline MMatrix::MMatrix(double val)
	{
		mRows = 1;
		mCols = 1;
		mSymmetric  = false;
		mTriangular = false;
		memoryAllocate();
		mVals[0] = val;
	}
 	inline MMatrix::MMatrix(size_t nRows, size_t nCols)
		:mRows(nRows),mCols(nCols)
	{
	 	mSymmetric  = false;
		mTriangular = false;
		memoryAllocate();
 	}
	inline MMatrix::MMatrix(size_t nRows, size_t nCols, double val)
		:mRows(nRows),mCols(nCols)	
	{
		mSymmetric  = false;
		mTriangular = false;
		memoryAllocate();
		assign(val);
	}
	inline MMatrix::MMatrix(size_t nRows, size_t nCols, double* pvals)
		:mRows(nRows),mCols(nCols)
	{
 		mSymmetric  = false;
		mTriangular = false;
		memoryAllocate();
		memcpy(mVals,pvals,nRows*nCols);
 	}
	inline MMatrix::MMatrix(size_t nRows, size_t nCols, std::vector<double> vVals)
		:mRows(nRows),mCols(nCols)
	{
 		mSymmetric  = false;
		mTriangular = false;
		memoryAllocate();
		for(size_t t = 0; t < vVals.size(); t++)
			mVals[t] = vVals[t];
  	}
	inline MMatrix::MMatrix(size_t nRows, size_t nCols, char cType)
		:mRows(nRows),mCols(nCols)
	{
		mSymmetric  = false;
		mTriangular = false;
		memoryAllocate();
		switch(cType)
		{
		case 'I':
			if(nRows != nCols) break;
			memset(mVals,0,mRows*mCols*sizeof(double));
			for(size_t i = 0; i < mCols; i++)
				mVals[i*mRows + i] = 1.0;
			mSymmetric  = false;
			break;
		default:
			break;
 		};
	}
	inline MMatrix::MMatrix(size_t nRows, size_t nCols, double val,char cType)
	{
		mSymmetric  = false;
		mTriangular = false;
		memoryAllocate();
		switch(cType)
		{
		case 'I':
			if(nRows != nCols) break;
			memset(mVals,0,mRows*mCols*sizeof(double));
			for(size_t i = 0; i < mCols; i++)
				mVals[i*mRows + i] = val;
			mSymmetric  = false;
			break;
		default:
			break;
 		};
 	}
 	inline MMatrix::MMatrix(const MMatrix& matA)
		:mRows(matA.mRows),mCols(matA.mCols),
		 mSymmetric(matA.mSymmetric),mTriangular(matA.mTriangular)
	{
 		memoryAllocate();
		memcpy(mVals,matA.mVals,mRows*mCols*sizeof(double));
	}
	inline MMatrix::~MMatrix()
	{
		delete []mVals;
		mVals = NULL;
	}
	//------------------------------------------------------------------------------------------------

	inline size_t MMatrix::sizeRow() const
	{
		return mRows;
	}
	inline size_t MMatrix::sizeCol() const
	{
		return mCols;
	}
	inline size_t MMatrix::getElemNum() const
	{
		return mRows * mCols;
	}
	inline double* MMatrix::gets()
	{
		return mVals;
	}
	inline const double* MMatrix::gets() const
	{
		return mVals;
	}
 	inline double MMatrix::get(size_t unRows, size_t unCols) const
	{
		return mVals[unRows + unCols*mRows];
	}
	inline double MMatrix::get(size_t unIndex) const
	{
		return mVals[unIndex];
	}
	inline void MMatrix::assign(double val)
	{
		size_t eleMem = mRows * mCols;
		for(size_t t = 0; t < eleMem; t++)
			mVals[t] = val;
	}
	inline void MMatrix::assign(double val, size_t unIndex)
	{
		mVals[unIndex] = val;
	}
	inline void MMatrix::assign(double val, size_t unRows, size_t unCols)
	{
		mVals[unRows + unCols*mRows] = val;
	}
	inline void MMatrix::add(double val, size_t unIndex)
	{
		mVals[unIndex] += val;
	}
	inline void MMatrix::add(double val, size_t unRows, size_t unCols)
	{
		mVals[unRows + unCols*mRows] += val;
	}

	inline const bool MMatrix::isAnyNaN() const
	{
		size_t eleMem = mRows * mCols;
		for(size_t t = 0; t < eleMem; t++)
		{
			if(_isnan(mVals[t]))
				return true; 
		}
		return false;
	}
	inline const bool MMatrix::isAnyINF() const
	{
		size_t eleMem = mRows * mCols;
		for(size_t t = 0; t < eleMem; t++)
		{
			if(!_finite(mVals[t]))
				return true; 
		}
		return false;
 	}
	inline const bool MMatrix::isSquare() const
	{
		return mRows == mCols;
	}
	inline const bool MMatrix::isTriangular() const
	{
		return mTriangular;
	}
	inline const bool MMatrix::isSymmetric() const
	{
		return mSymmetric;
	}
	inline void MMatrix::setTriangular(const bool bVal)
	{
		mTriangular = bVal;
	}
	inline void MMatrix::setSymmetric (const bool bVal)
	{
		mSymmetric = bVal;
	}
 	inline const bool MMatrix::dimensionsMatch(const MMatrix& matA) const
	{
		return ((matA.mRows == mRows) && (matA.mCols == mCols));
	}
	inline const bool MMatrix::rowsMatch(const MMatrix& matA) const
	{
		return (matA.mRows == mRows);
	}
	inline const bool MMatrix::colsMatch(const MMatrix& matA) const
	{
		return (matA.mCols == mCols);
	}
	inline std::ostream& operator << (std::ostream &os, const MMatrix & Mat)
	{
		for(size_t i = 0; i < Mat.sizeRow(); i++)
		{
			os << i << " ";
			for(size_t j = 0; j < Mat.sizeCol(); j++)
					os << Mat.get(i,j) << " ";
			os << std::endl;
		}
		return os;
	}
	inline void MMatrix::operator = (const MMatrix& mat)
	{
		resize(mat.mRows,mat.mCols);
		copy(mat);
	}
	inline MMatrix MMatrix::operator*(const MMatrix& mat)
	{
		MMatrix C(mRows, mat.mCols);
		C.gemm(*this, mat, 1.0, 0.0, "n", "n");
		return C;
	}
	inline void MMatrix::operator += (const MMatrix& mat)
	{
		assert(dimensionsMatch(mat));
		daxpy_(mCols*mRows,1.0,mat.gets(),1,mVals,1);
	}
	inline void MMatrix::operator -= (const MMatrix& mat)
	{
		assert(dimensionsMatch(mat));
		daxpy_(mCols*mRows,-1.0,mat.gets(),1,mVals,1);
	}
};
#endif