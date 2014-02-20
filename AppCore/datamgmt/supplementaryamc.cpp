#include "supplementaryamc.h"

GPCMSupplementaryAMC::GPCMSupplementaryAMC(int configChannels)
	:mConfigChannels(configChannels)
{

}
void GPCMSupplementaryAMC::splitVelocity(MatrixXd &positions, MatrixXd &velocities)
{
	  // Determine where positions end and velocities start.
    // This excludes the root velocity terms.
    int firstVelocityIndex = 0;
    for (int j = 0; j < positions.cols(); j++)
    {
        if (variableIndices[j] >= mConfigChannels)
        {
            firstVelocityIndex = j;
            break;
        }
    }

    // Now copy out the velocities.
    velocities.resize(positions.rows(),positions.cols()-firstVelocityIndex+3);
    // Copy over the root velocities.
    velocities.block(0,0,positions.rows(),1) = positions.block(0,0,positions.rows(),1);
    velocities.block(0,1,positions.rows(),1) = positions.block(0,1,positions.rows(),1);
    velocities.block(0,2,positions.rows(),1) = positions.block(0,2,positions.rows(),1);
    // Copy over remaining velocity terms.
    for (int j = firstVelocityIndex; j < positions.cols(); j++)
        velocities.block(0,j-firstVelocityIndex+3,positions.rows(),1) = positions.block(0,j,positions.rows(),1);

    // Create new positions matrix.
    MatrixXd newPositions(positions.rows(),firstVelocityIndex-3);
    // Copy over height.
    // newPositions.block(0,0,positions.rows(),1) = positions.block(0,1,positions.rows(),1);
    // Copy over joint positions.
    for (int j = 3; j < firstVelocityIndex; j++)
        newPositions.block(0,j-3,positions.rows(),1) = positions.block(0,j,positions.rows(),1);

    // Swap out matrices to output positions.
    positions = newPositions;

    // Fill in reconstruction vectors.
    isVelocity[0] = 1;
    isVelocity[1] = 1;
    isVelocity[2] = 1;
    for (unsigned i = firstVelocityIndex; i < isVelocity.size(); i++) 
		isVelocity[i] = 1;
    
	//fullToSplit[1] = 0;
    for (int i = 3; i < firstVelocityIndex; i++) 
		fullToSplit[i] = i-3;
    fullToSplit[0] = 0;
    fullToSplit[1] = 1;
    fullToSplit[2] = 2;
    for (unsigned i = firstVelocityIndex; i < isVelocity.size(); i++) 
		fullToSplit[i] = i-firstVelocityIndex+3; 
     
    // Set position and velocity indices
    positionIndices = getPositionIndicesInFullPose();
    velocityIndices = getVelocityIndicesInFullPose();
	
}