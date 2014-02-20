#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <math.h>

namespace ResUtil
{
	template<class T>
	class CVector3D
	{
	public:
		CVector3D(void);
		CVector3D(T m1,T m2,T m3);
		CVector3D(const CVector3D<T> & v3D);
		~CVector3D(void);
	public:
	
		CVector3D<T> operator = (const CVector3D<T> & v3D);
		CVector3D<T> operator - (const CVector3D<T> & v3D) const;
		CVector3D<T> operator + (const CVector3D<T> & v3D) const;
		CVector3D<T> operator * (const T val) const;

	public: 
		bool  operator == (const CVector3D<T>& v3D) const;
		bool  operator != (const CVector3D<T>& v3D) const;

	public:
		double GetLength() const;
		T GetL2Length() const;

		CVector3D<T> Normalize();
		CVector3D<T> GetNomral() const;

		T DotProduct(const CVector3D<T> & v3D) const;
		CVector3D<T> CrossProduct(const CVector3D<T> & v3D) const;
	
	public:
		union {
			T x;
			T R;
		};
		union{
			T y;
			T G;
		};
		union {
			T z;
			T B;
		};
	};

	template <class T>
	CVector3D<T>::CVector3D(void)
	{
			x =  
			y =  
			z = T(0);
	}
	//-------------------------------------------------------------------------
	template <class T>
	CVector3D<T>::CVector3D(T m1,T m2,T m3)
	{
		this->x = m1;
		this->y = m2;
		this->z = m3;
	}
	//-------------------------------------------------------------------------
	template <class T>
	CVector3D<T>::CVector3D(const CVector3D<T> & v3D)
	{
		this->x = v3D.x;
		this->y = v3D.y;
		this->z = v3D.z;
	}
	//-------------------------------------------------------------------------
	template <class T>
	CVector3D<T>::~CVector3D(void)
	{
	}
	//-------------------------------------------------------------------------
	template <class T>
	CVector3D<T> CVector3D<T>::operator * (const T val) const
	{
		return CVector3D<T>(x * val,y * val,z * val);
	}

	//-------------------------------------------------------------------------
	template <class T>
	CVector3D<T> CVector3D<T>::operator = (const CVector3D<T> & v3D)
	{
		this->x = v3D.x;
		this->y = v3D.y;
		this->z = v3D.z;
		return (*this);
	}

	//-------------------------------------------------------------------------
	template <class T>
	CVector3D<T> CVector3D<T>::operator - (const CVector3D<T> & v3D) const
	{
		return CVector3D<T>(x - v3D.x,y - v3D.y,z - v3D.z);
	}
	//-------------------------------------------------------------------------
	template <class T>
	CVector3D<T> CVector3D<T>::operator + (const CVector3D<T> & v3D) const
	{
		return CVector3D<T>(x + v3D.x,y + v3D.y,z + v3D.z);
	}
	//-------------------------------------------------------------------------
	template <class T>
	bool  CVector3D<T>::operator == (const CVector3D<T>& v3D) const
	{
		return (x == v3D.x && y == v3D.y && z == v3D.z);
	}
	//-------------------------------------------------------------------------
	template <class T>
	bool  CVector3D<T>::operator != (const CVector3D<T>& v3D) const
	{
		return (x != v3D.x || y != v3D.y || z != v3D.z);
	}
	//-------------------------------------------------------------------------
	template <class T>
	double CVector3D<T>::GetLength() const
	{
		return sqrt(x * x + y * y + z * z);
	}
	//-------------------------------------------------------------------------
	template <class T>
	T CVector3D<T>::GetL2Length() const
	{
		return (x * x + y * y + z * z);
	}
	//-------------------------------------------------------------------------
	template <class T>
	CVector3D<T> CVector3D<T>::Normalize() 
	{
		double Length = sqrt(x * x + y * y + z * z);
		if(Length > 0.0){
			x /= Length;
			y /= Length;
			z /= Length;
		}
		return (*this);
	}
	//-------------------------------------------------------------------------
	template <class T>
	CVector3D<T> CVector3D<T>::GetNomral() const
	{
		double Length = sqrt(x * x + y * y + z * z);
		if(Length > 0.0){
			T tX = x / Length;
			T tY = y / Length;
			T tZ = z / Length;
			return CVector3D<T>(tX,tY,tZ);
		}
		return (*this);
	}
	//-------------------------------------------------------------------------
	template <class T>
	T CVector3D<T>::DotProduct(const CVector3D<T> & v3D) const
	{
		return ((x * v3D.x) + (y * v3D.y) + (z* v3D.z));
	}
	//-------------------------------------------------------------------------
	template <class T>
	CVector3D<T> CVector3D<T>::CrossProduct(const CVector3D<T> & v3D) const
	{
		return CVector3D<T>(y*v3D.z - z*v3D.y,z*v3D.x - x*v3D.z, x*v3D.y - y* v3D.x);
	}
	//-------------------------------------------------------------------------
}
#endif