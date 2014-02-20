#ifndef __datareaderamc_h_
#define __datareaderamc_h_ 

#include "datareader.h"
#include "joint.h"
#include <fstream>

class GPCMDataReaderAMC: public GPCMDataReader
{
public:
	// Crate AMC data reader;
	GPCMDataReaderAMC();
	//destructor
	~GPCMDataReaderAMC();
    // Load a single file.
    virtual MatrixXd loadFile(std::string filename);
    // Load a corresponding auxiliary data file.
    virtual MatrixXd loadAuxFile(std::string filename);

protected:
	//make all the data matrix to be 
	void alignDataMatrix(MatrixXd &dataMatrix);
	   // Convert rotations to exponential maps.
	void validateChannels(const MatrixXd &channels);
	virtual MatrixXd flipJoint(const MatrixXd &unflipped);
    virtual void postProcessData();
    virtual void convertRotations(MatrixXd &channels);
 	//compute velocities terms;
	virtual MatrixXd computeVelocities(MatrixXd &channels, double frameTime);
	//
	virtual void setInitScales(int config, int chans);

};




#endif
