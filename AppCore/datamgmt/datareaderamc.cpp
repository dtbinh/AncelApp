#include "datareaderamc.h"
#include "mathutils.h"
#include "QPBO.h"
#include "debugprint.h"
#include "supplementaryamc.h"

#define OVERALL_SCALE 1.0
#define QUAT_SCALE  1.0
#define ANG_SCALE   (1.0/180.0)
 
// Scaling constants.
#define POSITION_SCALE          0.01
#define ROOT_SCALE              0.25
#define ROOT_ROTATION_SCALE     (1.0/180.0)
#define ROTATION_SCALE          (1.0/180.0)
#define ROOT_VELOCITY_SCALE     10.0
#define VELOCITY_SCALE          1.0
#define OVERALL_SCALE           1.0

// For flip fixes, cost of violation between each node.
#define CONST_WEIGHTS           1.0

// For flip fixes, cost of violation between neighbors.
#define NEIGHBOR_WEIGHTS        1.0

//TODO constant reference is more efficient than pass by value
GPCMDataReaderAMC::GPCMDataReaderAMC()
{

}

GPCMDataReaderAMC::~GPCMDataReaderAMC()
{

}
MatrixXd GPCMDataReaderAMC::loadAuxFile(std::string filename)
{
	return MatrixXd();
}

MatrixXd GPCMDataReaderAMC::loadFile(std::string filename)
{
	std::string loadedfilename = filename;
	if(loadedfilename.find(".amc") != std::string::npos)
	{
		std::string command = "ToBin " + filename;
		system(command.c_str());
		loadedfilename = filename.substr(0,filename.find(".")) + ".ann";
	}

	std::ifstream amcstream(loadedfilename.c_str(),std::ios::binary|std::ios::in);
 
	int rows,cols;
	amcstream.read((char*)&rows,sizeof(int));
	amcstream.read((char*)&cols,sizeof(int));

	MatrixXd channels(rows,cols);

	amcstream.read((char*)channels.data(),sizeof(double)*cols*rows);
	amcstream.close();
	
	/*std::ofstream fout("check.txt");
	fout << channels << std::endl;*/

	alignDataMatrix(channels);
	convertRotations(channels);

	if (supplementary) 
		delete supplementary;
	
	supplementary = new GPCMSupplementaryAMC(channels.cols());
 
	return channels;
}
void GPCMDataReaderAMC::alignDataMatrix(MatrixXd &dataMatrix)
{
	const static int mask[] = {7, 7, 7, 7, 7, 7, 7, 7, 4, 2, 7, 4, 2, 7, 4, 5, 7, 4, 5};
	const static int bone_size = sizeof(mask)/sizeof(int);
	
	//three dimmensions for each joint, and three dimmensions for root position
	MatrixXd channels(dataMatrix.rows(),bone_size * 3 + 3);

	channels.block(0,0,channels.rows(),3) = dataMatrix.block(0,0,dataMatrix.rows(),3);

	int currentOffset = 3;
	for (std::size_t i = 0; i < bone_size; i++)
	{
		if(mask[i] & 4)
			channels.block(0, (i+1) * 3, channels.rows(), 1) = dataMatrix.block(0, currentOffset++, dataMatrix.rows(), 1);
		else
			channels.block(0, (i+1) * 3, channels.rows(), 1) = MatrixXd::Zero(dataMatrix.rows(), 1);
		if(mask[i] & 2)
			channels.block(0, (i+1) * 3 + 1, channels.rows(), 1) = dataMatrix.block(0, currentOffset++, dataMatrix.rows(), 1);
		else
			channels.block(0, (i+1) * 3 + 1, channels.rows(), 1) = MatrixXd::Zero(dataMatrix.rows(), 1);
		if(mask[i] & 1)
			channels.block(0, (i+1) * 3 + 2, channels.rows(), 1) = dataMatrix.block(0, currentOffset++, dataMatrix.rows(), 1);
		else
			channels.block(0, (i+1) * 3 + 2, channels.rows(), 1) = MatrixXd::Zero(dataMatrix.rows(), 1);

	}
 	dataMatrix = channels;
}
void GPCMDataReaderAMC::convertRotations(MatrixXd &channels)
{
	MatrixXd newChannels(channels.rows(),((channels.cols()-3)*4)/3 + 3);
    newChannels.block(0,0,channels.rows(),3) = channels.block(0,0,channels.rows(),3);

    // Set number of entries.
    //positionEntries = 4;
    //velocityEntries = 3;
    //rotationScale = QUAT_SCALE;
    //rotationVelocityScale = ANG_SCALE;

	int order[3] = {2,1,0};
    for (int t = 0; t < channels.rows(); t++)
    {
        for (int j = 3; j < channels.cols(); j+=3)
        {
                Vector3d euler;
                euler << channels(t,j),
						 channels(t,j+1),
                         channels(t,j+2);
                Quaterniond quat = eulerToQuat(euler,order);
                // Determine the correct output indices.
          
				int jntIdx = (j-3)/3*4 + 3;
                newChannels(t,jntIdx+0) = quat.x();
                newChannels(t,jntIdx+1) = quat.y();
                newChannels(t,jntIdx+2) = quat.z();
                newChannels(t,jntIdx+3) = quat.w();
                // Check if we need to flip.
                if (t > 0)
                {
                    double thisDist = 0.0;
                    double flipDist = 0.0;
                    for (int k = 0; k < 4; k++)
                    {
                        thisDist += pow(newChannels(t-1,jntIdx+k)-newChannels(t,jntIdx+k),2);
                        flipDist += pow(newChannels(t-1,jntIdx+k)+newChannels(t,jntIdx+k),2);
                    }
                    if (flipDist < thisDist)
                    { // Flip quaternion to face the other way.
                        for (int k = 0; k < 4; k++)
                            newChannels(t,jntIdx+k) = -newChannels(t,jntIdx+k);
                    }
                }
            }
     }
	
    // Return result.
    channels = newChannels;
}

MatrixXd GPCMDataReaderAMC::computeVelocities(MatrixXd &channels, double frameTime)
{
	  // Create new channels structure for output.
    MatrixXd newChannels(channels.rows(),channels.cols() + ((channels.cols()-3)*3)/4);
    newChannels.block(0,0,channels.rows(),channels.cols()) = channels;

	int order[3] = {2,1,0};
    for (int t = 0; t < channels.rows(); t++)
    {
		for (int j = 3; j < channels.cols(); j += 4)
        {
            int curridx = t;
            int previdx = t-1;
            if (previdx < 0)
            { // Don't have a previous frame, so use the next one.
                curridx = t+1;
                previdx = t;
            }

            // Get indices.
            int jntIdx = j;

            // Compute angular velocity.
            Vector4d prev, curr;
            prev << newChannels(previdx,jntIdx+0),
                    newChannels(previdx,jntIdx+1),
                    newChannels(previdx,jntIdx+2),
                    newChannels(previdx,jntIdx+3);
            curr << newChannels(curridx,jntIdx+0),
                    newChannels(curridx,jntIdx+1),
                    newChannels(curridx,jntIdx+2),
                    newChannels(curridx,jntIdx+3);

            // Compute difference between two quaternions.
            Quaterniond prevQuat(prev);
            Quaterniond currQuat(curr);
            Quaterniond diffQuat = prevQuat.inverse()*currQuat;
            diffQuat.w() = fabs(diffQuat.w());

            // Convert difference to exponential map.
            Vector3d diff = quatToExp(diffQuat);
            assert(diff.norm() <= 180.0);

            // Divide by frameTime.
            diff /= frameTime;

            // Compute index and store.
            jntIdx = ((j - 3)*3)/4 + channels.cols();
            newChannels(t,jntIdx+0) = diff(0);
            newChannels(t,jntIdx+1) = diff(1);
            newChannels(t,jntIdx+2) = diff(2);
          }
    }
	std::ofstream fout("check.txt");
	fout << channels << std::endl;
	fout << std::endl;
	fout << newChannels << std::endl;
    // Return result.
    return newChannels;
}
MatrixXd GPCMDataReaderAMC::flipJoint(const MatrixXd &unflipped)
{
	return -unflipped;
}
void GPCMDataReaderAMC::postProcessData()
{
	// Get supplementary.    
    //GPCMSupplementaryBVH *bvh = dynamic_cast<GPCMSupplementaryBVH*>(supplementary);
    //GPCMSkeletonData &skeleton = bvh->getSkeleton();

    // Construct neighbor edge weights.
    MatrixXd edgeWeights(Y.rows(),Y.rows());
    edgeWeights.setConstant(Y.rows(),Y.rows(),CONST_WEIGHTS/((double)Y.rows()));
    
	int start = 0;
    for (unsigned i = 0; i < sequence.size(); i++)
    {
        int end = sequence[i];

        // Step over each time step and add weights to neighbor.
        for (int t = start+1; t < end; t++)
        {
            edgeWeights(t,t-1) = NEIGHBOR_WEIGHTS;
            edgeWeights(t-1,t) = NEIGHBOR_WEIGHTS;
        }

        start = end;
    }

    // Flip root heading and each joint as necessary to minimize overall differences.
    for (int i = 3; i < Y.cols(); i+= 4)
    {
        int startIdx = i;
        int endIdx = i + 4;
        startIdx = i;
        endIdx = i + 4;
 
        // Compute flipped and unflipped matrices using seperate method.
        MatrixXd unflipped = Y.block(0,startIdx,Y.rows(),endIdx-startIdx);
        MatrixXd flipped = flipJoint(unflipped);

        // Compute differences between all flipped and unflipped combinations.
        MatrixXd unflippedDist = pairwiseDistance(unflipped);
        MatrixXd flippedDist = pairwiseDistance(flipped);
        MatrixXd rowsFlippedDist = pairwiseDistance(flipped,unflipped);
        MatrixXd colsFlippedDist = pairwiseDistance(unflipped,flipped);

        // Solve with QPBO to get optimal flips.
        KQPBO::QPBO *qpbo = new KQPBO::QPBO(Y.rows(),Y.rows()*Y.rows());
        qpbo->AddNode(Y.rows()); // Add all nodes.
        
        // Add unary terms.
        for (int i = 0; i < Y.rows(); i++)
            qpbo->AddUnaryTerm(i,0,0);

        // Add pairwise terms.
        for (int i = 0; i < Y.rows(); i++)
        {
            for (int j = 0; j < Y.rows(); j++)
            {
                if (i != j)
                {
                    qpbo->AddPairwiseTerm(i,j,edgeWeights(i,j)*unflippedDist(i,j),
                                              edgeWeights(i,j)*colsFlippedDist(i,j),
                                              edgeWeights(i,j)*rowsFlippedDist(i,j),
                                              edgeWeights(i,j)*flippedDist(i,j));
                }
            }
        }

        // Solve.
        qpbo->Solve();
        qpbo->ComputeWeakPersistencies();

        // Flip terms as desired.
        for (int i = 0; i < Y.rows(); i++)
        {
            int flip = qpbo->GetLabel(i);
            if (flip)
                Y.block(i,startIdx,1,endIdx-startIdx) = -Y.block(i,startIdx,1,endIdx-startIdx);
        }

        // Clean up.
        delete qpbo;
    }

    // Store old Y.
    MatrixXd oldY = Y;

    // Step over each clip.
    start = 0;
    for (unsigned i = 0; i < sequence.size(); i++)
    {
        int end = sequence[i];

        // Get channels.
        MatrixXd channels = oldY.block(start,0,end-start,oldY.cols());

        // Compute velocities.
        channels = computeVelocities(channels,0.48);

        // Validate that all channels look reasonable.
        validateChannels(channels);

        // Set initial scales.
        setInitScales(oldY.cols(),channels.cols());

        // Store in new Y.
        if (i == 0)
        {
            Y = channels;
        }
        else
        {
            MatrixXd pY = Y;
            Y.resize(Y.rows()+channels.rows(),Y.cols());
            Y << pY,channels;
        }

        start = end;
    }
}

void GPCMDataReaderAMC::validateChannels(const MatrixXd &channels)
{
	MatrixXd sd = channels.block(2,0,channels.rows()-2,channels.cols())-
                  channels.block(1,0,channels.rows()-2,channels.cols())*2.0+
                  channels.block(0,0,channels.rows()-2,channels.cols());

    // Compute variance.
    int T = sd.rows();
    MatrixXd means = sd.colwise().sum()/T;
    MatrixXd vars = (sd - means.replicate(T,1)).colwise().squaredNorm()/T;

    // Step over all channels.
    for (int t = 0; t < sd.rows(); t++)
    {
        // Check all channels.
        for (int c = 0; c < channels.cols(); c++)
        {
            if (fabs(sd(t,c)) > vars(0,c)*4.0 && fabs(sd(t,c)) > SMALL_ANGLE_ACC_VAR)
                DBWARNING("Channel " << c << " at step " << (t+1) <<
                    " contains a discontinuity: sd=" << fabs(sd(t,c)) <<
                    " vs var=" << vars(0,c));
        }
    }
}

void GPCMDataReaderAMC::setInitScales(int config, int chans)
{
	//// Set initial scales.
 //   // Initial scales are based on joint length in meters
 //   // Rotational joints are rescaled to 1.0/180.0
 //   // Root "length" is taken to be 1
 //   // Root velocity is scaled to meters and multiplied by 10
	 MatrixXd scales(1,chans);
	 scales = MatrixXd::Ones(1,chans);

 //   // First set the root.
 //   scales(0,0) = POSITION_SCALE*ROOT_SCALE*ROOT_VELOCITY_SCALE;
 //   scales(0,1) = POSITION_SCALE*ROOT_SCALE;
 //   scales(0,2) = POSITION_SCALE*ROOT_SCALE*ROOT_VELOCITY_SCALE;
 //   scales(0,3) = ROOT_ROTATION_SCALE*ROOT_SCALE*ROOT_VELOCITY_SCALE;
 //   for (int k = 4; k < 4+positionEntries; k++)
 //   {
 //       scales(0,k) = rotationScale*ROOT_SCALE;
 //   }
 //   for (int k = config; k < config+velocityEntries; k++)
 //   {
 //       scales(0,k) = rotationVelocityScale*ROOT_SCALE*VELOCITY_SCALE;
 //   }

 //   // Next set all of the joints.
 //   Vector3d u;
 //   double *ulen = new double[skeleton.jointCount];
 //   ulen[0] = 0.5*ROTATION_SCALE;
 //   for (int i = 1; i < skeleton.jointCount; i++)
 //   {
 //       const int *order = skeleton.joints[i].getOrder();
 //       const int *rotInd = skeleton.joints[i].getRotInd();
 //       u = skeleton.joints[i].getOffset();
 //       ulen[i] = u.norm()*POSITION_SCALE;
 //       if (ulen[i] < 1e-8)
 //           ulen[i] = ulen[skeleton.joints[i].getParent()]; // Zero length joints like the pelvis are actually quite high in weight.
 //       if (rotInd)
 //       {
 //           // Determine index of first position and velocity entries.
 //           int firstPos = ((rotInd[order[0]]-3)/3)*positionEntries + 4;
 //           int firstVel = config+((rotInd[order[0]]-3)/3)*velocityEntries;
 //           for (int k = firstPos; k < firstPos+positionEntries; k++)
 //           {
 //               scales(0,k) = ulen[i]*rotationScale;
 //           }
 //           for (int k = firstVel; k < firstVel+velocityEntries; k++)
 //           {
 //               scales(0,k) = ulen[i]*rotationVelocityScale*VELOCITY_SCALE;
 //           }
 //       }
 //   }
 //   delete[] ulen;
 //   
 //   // Store scales in supplement.
     supplementary->getScale() = scales*OVERALL_SCALE;
}