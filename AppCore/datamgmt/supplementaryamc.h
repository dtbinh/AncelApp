#ifndef _supplementaryamc_h_
#define _supplementaryamc_h_

#include "supplementary.h"


class GPCMSupplementaryAMC: public GPCMSupplementaryData
{
protected:
	int mConfigChannels;
public:
	GPCMSupplementaryAMC(int configChannels);
	virtual void splitVelocity(MatrixXd &positions, MatrixXd &velocities);

};


#endif