#ifndef QUATERNION_H
#define QUATERNION_H

#include "Vector3D.h"
#ifndef M_PI 
#define M_PI 3.141592653589
#endif 
namespace ResUtil
{
	typedef CVector3D<double> EulerAngle;
	typedef CVector3D<double> ExpontialMap;

	class CQuaternion
	{
	public:
		CQuaternion();
		CQuaternion(double W,double I,double J,double K);
		CQuaternion(const CQuaternion & Q);

		EulerAngle	 ToEulerAngle()   const;
		ExpontialMap ToExpontialMap() const;
 		friend CQuaternion EAToQuaternion(const EulerAngle & EA);
		friend CQuaternion EMToQuaternion(const ExpontialMap & EM);
	public:

		double m_W,m_I,m_J,m_K;
	};
	//------------------------------inline function--------------------------------------------------------
	inline CQuaternion::CQuaternion()
	{
		m_W = 1.0;
		m_I = 0.0;
		m_J = 0.0;
		m_K = 0.0;
	}
	inline CQuaternion::CQuaternion(double W,double I,double J,double K)
		:m_W(W),m_I(I),m_J(J),m_K(K)
	{
	}
	inline CQuaternion::CQuaternion(const CQuaternion & Q)
		:m_W(Q.m_W),m_I(Q.m_I),m_J(Q.m_J),m_K(Q.m_K)
	{
	}
	inline ExpontialMap CQuaternion::ToExpontialMap() const
	{
//		double norm = sqrt(m_W*m_W + m_I*m_I + m_J*m_J + m_K*m_K);
//		assert(norm != 0.0);
		double w = m_W ;// norm;
		double i = m_I ;// norm;
		double j = m_J ;// norm;
		double k = m_K ;// norm;

		double halfTheta = acos(w);
		double sinc = 1.0;
		if(halfTheta < 1e-6)
			 sinc = 0.5 + (halfTheta*halfTheta)/12;
		else 
			 sinc = sin(halfTheta)/(halfTheta*2);

		ExpontialMap EM;
		EM.x = i / sinc; 
		EM.y = j / sinc; 
		EM.z = k / sinc; 

		
		//EM.x = exp(cosPhiX);// * halfTheta;
		//EM.y = exp(cosPhiY);// * halfTheta;
		//EM.z = exp(cosPhiZ);// * halfTheta;
	/*	EM.x = cosPhiX * halfTheta;
		EM.y = cosPhiY * halfTheta;
		EM.z = cosPhiZ * halfTheta;*/

		return EM;
 	}
	inline CQuaternion EAToQuaternion(const EulerAngle & EA)
	{
		double halfPhi   = EA.x/2;// * M_PI /360;
		double halfTheta = EA.y/2;// * M_PI /360;
		double halfPsi   = EA.z/2;// * M_PI /360;
		double cosPhi    = cos(halfPhi);
		double sinPhi    = sin(halfPhi);
		double cosTheta  = cos(halfTheta);
		double sinTheta  = sin(halfTheta);
		double cosPsi    = cos(halfPsi);
		double sinPsi    = sin(halfPsi);
		
		CQuaternion Q;
		Q.m_W = (cosPhi * cosTheta * cosPsi) + (sinPhi * sinTheta * sinPsi);
		Q.m_I = (sinPhi * cosTheta * cosPsi) - (cosPhi * sinTheta * sinPsi);
		Q.m_J = (cosPhi * sinTheta * cosPsi) + (sinPhi * cosTheta * sinPsi);
		Q.m_K = (cosPhi * cosTheta * sinPsi) - (sinPhi * sinTheta * cosPsi);
		
		return Q;
	}
	inline CQuaternion EMToQuaternion(const ExpontialMap & EM)
	{
		double theta = EM.GetLength();
		CVector3D<double> v = EM*(sin(theta/2)/theta);
		CQuaternion Q;
		Q.m_W = cos(theta/2);
		Q.m_I = v.x;
		Q.m_J = v.y;
		Q.m_K = v.z;
		return Q;
	}
	inline EulerAngle CQuaternion::ToEulerAngle() const
	{
		EulerAngle EA;
		EA.x = atan2(2*(m_W*m_I + m_J * m_K),1 - 2*(m_I * m_I + m_J * m_J));
		EA.y = asin(2 *(m_W * m_J - m_I * m_K));
		EA.z = atan2(2*(m_W * m_K + m_I * m_J),1 - 2*(m_J * m_J + m_K * m_K));
		
		return EA;
	}
}
#endif