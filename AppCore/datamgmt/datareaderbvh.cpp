// Data loader for BVH motion capture files.

#include "QPBO.h"

#include "mathutils.h"
#include "debugprint.h"
#include "datareaderbvh.h"
#include "supplementary.h"
#include "supplementarybvh.h"

#include <Eigen/Geometry>

#include <boost/tokenizer.hpp>

#include <list>
#include <iterator>
#include <stdint.h>

// Desired root rotation order.
// Currently set to YZX.
static const int desiredRootOrder[] = { 1, 2, 0 };

// First frame to read from BVH file.
#define FIRST_FRAME 1

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

// Create the data reader.
GPCMDataReaderBVH::GPCMDataReaderBVH()
{
    supplementary = NULL;
    rotationParam = "exp";
}

// Convert the root rotation to desired joint order.
void GPCMDataReaderBVH::transformRoot(
    GPCMSkeletonData &skeleton,             // Current skeleton.
    MatrixXd &channels,                     // Untransformed, global-root channels.
    const int *order                        // Desired root joint order.
    )
{
    // Get index of first rotation DoF for root.
    int rootrot = INT_MAX;
    for (int k = 0; k < 3; k++)
        if (skeleton.joints[0].getRotIndex(k) < rootrot)
            rootrot = skeleton.joints[0].getRotIndex(k);

    // Convert Euler angles for each frame.
    for (int t = 0; t < channels.rows(); t++)
    {
        channels.block(t,rootrot,1,3) = convertEuler(Vector3d(channels.block(t,rootrot,1,3).transpose()),
                                                     skeleton.joints[0].getOrder(),order).transpose();
    }

    // Store new root order.
    skeleton.joints[0].switchOrder(order);
}

// Smooth out all rotations.
void GPCMDataReaderBVH::smoothRotations(
    const GPCMSkeletonData &skeleton,       // Current skeleton.
    MatrixXd &channels                      // Untransformed, global-root channels.
    )
{
    // TODO: implement.
}

// Transform channels to have relative root positions.
void GPCMDataReaderBVH::makeRootRelative(
    const GPCMSkeletonData &skeleton,       // Current skeleton.
    MatrixXd &channels,                     // Untransformed, global-root channels.
    double frameTime                        // Length of each frame.
    )
{
    // Step over each frame.
    MatrixXd oldChannels = channels;
    for (int t = 0; t < channels.rows(); t++)
    {
        // Get indices of current and previous frame.
        int curridx = t;
        int previdx = t-1;
        if (previdx < 0)
        { // Don't have a previous frame, so use the next one.
            curridx = t+1;
            previdx = t;
        }

        // Obtain translation and rotation from current and previous frames.
        Vector2d prevTranslation;
        Vector2d currTranslation;
        double prevRotation = oldChannels(previdx,3);
        double currRotation = oldChannels(curridx,3);
        double rotation = oldChannels(t,3);
        prevTranslation << oldChannels(previdx,0),oldChannels(previdx,2);
        currTranslation << oldChannels(curridx,0),oldChannels(curridx,2);
        Vector2d delta = (currTranslation-prevTranslation)/frameTime;

        // Remove root rotation from horizontal translation.
        delta = Rotation2Dd(DEG_TO_RAD(rotation))*delta;
        channels(t,0) = delta(0);
        channels(t,2) = delta(1);
        channels(t,3) = modDeg(currRotation-prevRotation)/frameTime;
    }
}

// Convert rotations to exponential maps.
void GPCMDataReaderBVH::convertRotations(
    const GPCMSkeletonData &skeleton,       // Current skeleton.
    MatrixXd &channels                      // Untransformed, relative-root channels.
    )
{
    // Create new channels structure for output.
    MatrixXd newChannels(channels.rows(),channels.cols()+1);
    newChannels.block(0,0,channels.rows(),4) = channels.block(0,0,channels.rows(),4);

    // Set number of entries.
    positionEntries = 3;
    velocityEntries = 3;
    rotationScale = ROTATION_SCALE;
    rotationVelocityScale = ROTATION_SCALE;

    for (int t = 0; t < channels.rows(); t++)
    {
        for (int j = 0; j < skeleton.jointCount; j++)
        {
            const int *order = skeleton.joints[j].getOrder();
            const int *rotInd = skeleton.joints[j].getRotInd();
            if (rotInd)
            { // If this joint has a rotation, convert it.
                if (j == 0)
                { // Use zero for yaw.
                    Vector3d euler;
                    euler << 0,
                             channels(t,rotInd[order[1]]),
                             channels(t,rotInd[order[2]]);
                    Vector3d exp = eulerToExp(euler,order);
                    newChannels(t,rotInd[order[0]]+1) = exp(0);
                    newChannels(t,rotInd[order[1]]+1) = exp(1);
                    newChannels(t,rotInd[order[2]]+1) = exp(2);
                }
                else
                { // Simply convert in place.
                    Vector3d euler;
                    euler << channels(t,rotInd[order[0]]),
                             channels(t,rotInd[order[1]]),
                             channels(t,rotInd[order[2]]);
                    Vector3d exp = eulerToExp(euler,order);
                    assert(exp.norm() <= 180.0);
                    newChannels(t,rotInd[order[0]]+1) = exp(0);
                    newChannels(t,rotInd[order[1]]+1) = exp(1);
                    newChannels(t,rotInd[order[2]]+1) = exp(2);
                }
            }
        }
    }

    // Return result.
    channels = newChannels;
}

// Augment vector of positions with their velocities.
MatrixXd GPCMDataReaderBVH::computeVelocities(
    const GPCMSkeletonData &skeleton,       // Current skeleton.
    MatrixXd &channels,                     // Transformed, relative-root channels.
    double frameTime                        // Length of each frame.
    )
{
    // Create new channels structure for output.
    MatrixXd newChannels(channels.rows(),channels.cols()*2-4);
    newChannels.block(0,0,channels.rows(),channels.cols()) = channels;

    for (int t = 0; t < channels.rows(); t++)
    {
        for (int j = 0; j < skeleton.jointCount; j++)
        {
            const int *order = skeleton.joints[j].getOrder();
            const int *rotInd = skeleton.joints[j].getRotInd();
            if (rotInd)
            { // If this joint has a rotation, differentiate it.
                // Figure out indices.
                int curridx = t;
                int previdx = t-1;
                if (previdx < 0)
                { // Don't have a previous frame, so use the next one.
                    curridx = t+1;
                    previdx = t;
                }

                // Compute angular velocity.
                Vector3d prev, curr;
                prev << newChannels(previdx,rotInd[order[0]]+1),
                        newChannels(previdx,rotInd[order[1]]+1),
                        newChannels(previdx,rotInd[order[2]]+1);
                curr << newChannels(curridx,rotInd[order[0]]+1),
                        newChannels(curridx,rotInd[order[1]]+1),
                        newChannels(curridx,rotInd[order[2]]+1);

                // Compute difference between two rotations.
                double aprev = prev.norm();
                double acurr = curr.norm();
                if (fabs(aprev) < EPS_ANGLE)
                    prev = Vector3d::Unit(0);
                else
                    prev = prev/aprev;
                if (fabs(acurr) < EPS_ANGLE)
                    curr = Vector3d::Unit(0);
                else
                    curr = curr/acurr;
                AngleAxisd diffquat = AngleAxisd(AngleAxisd(DEG_TO_RAD(acurr),curr)*
                                                 AngleAxisd(-DEG_TO_RAD(aprev),prev));
                Vector3d diff = diffquat.axis()*RAD_TO_DEG(modAngle(diffquat.angle()))/frameTime;

                // Output result.
                assert(rotInd[order[0]]-3 >= 0);
                assert(rotInd[order[1]]-3 >= 0);
                assert(rotInd[order[2]]-3 >= 0);
                newChannels(t,rotInd[order[0]]+channels.cols()-3) = diff(0);
                newChannels(t,rotInd[order[1]]+channels.cols()-3) = diff(1);
                newChannels(t,rotInd[order[2]]+channels.cols()-3) = diff(2);
            }
        }
    }

    // Return result.
    return newChannels;
}

// Check that there are no discontinuities in any of the channels.
void GPCMDataReaderBVH::validateChannels(
    const GPCMSkeletonData &skeleton,       // Current skeleton.
    const MatrixXd &channels                // Final channels.
    )
{
    // Compute second derivative.
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

// Read in data about the skeleton.
GPCMSkeletonData GPCMDataReaderBVH::loadSkeleton(
    std::ifstream &bvhstream                // File stream to load skeleton info from.
    )
{
    // Start the parser.
    bvhstream >> std::noskipws;
    std::istream_iterator<char> istr(bvhstream);
    std::istream_iterator<char> eos;
    boost::tokenizer<boost::char_delimiters_separator<char>,std::istream_iterator<char> >
        tokenizer(istr,eos,boost::char_delimiters_separator<char>(false,"",0));
    boost::tokenizer<boost::char_delimiters_separator<char>,std::istream_iterator<char> >
        ::iterator itr = tokenizer.begin();
    GPCMSkeletonData skeleton;
    std::list<GPCMJointData> joints;
    skeleton.jointCount = 0;
    skeleton.joints = NULL;

    // Read hierarchy key word.
    if (itr == tokenizer.end() || itr->compare("HIERARCHY"))
    {
        DBERROR("Missing HIERARCHY header.");
        return skeleton;
    }
    ++itr;

    // Read root key word.
    if (itr == tokenizer.end() || itr->compare("ROOT"))
    {
        DBERROR("Missing ROOT.");
        return skeleton;
    }
    ++itr;

    // Create joints.
    int curIdx = 0;
    std::list<int> parents;
    parents.push_back(-1);
    while (itr != tokenizer.end() && !parents.empty())
    {
        // Variables.
        int parent = parents.back();
        int order[3];
        int *orderPtr = NULL;
        int rotInd[3];
        int posInd[3];
        int *rotIndPtr = NULL;
        int *posIndPtr = NULL;

        // Push back new parent.
        parents.push_back(joints.size());

        // Read the name of the joint.
        std::string name = *itr;
        ++itr;
        // Read opening brace.
        if (itr == tokenizer.end() || itr->compare("{"))
        {
            DBERROR("Missing opening brace.");
            return skeleton;
        }
        ++itr;

        // Read "OFFSET".
        if (itr == tokenizer.end() || itr->compare("OFFSET"))
        {
            DBERROR("Missing OFFSET keyword.");
            return skeleton;
        }
        ++itr;
        // Parse offset.
        Vector3d offset;
        for (int i = 0; i < 3; i++)
        {
            if (itr == tokenizer.end())
            {
                DBERROR("Missing offset values.");
                return skeleton;
            }
            if (parent != -1) // No offset if this is the root.
                offset(i) = atof(itr->c_str());
            else
                offset(i) = 0;
            ++itr; 
        }

        // Read CHANNELS.
        if (itr == tokenizer.end())
        {
            DBERROR("Unexpected end of file.");
            return skeleton;
        }
        if (!itr->compare("CHANNELS"))
        { // Channels are present.
            rotIndPtr = rotInd;
            orderPtr = order;
            ++itr;
            // Read number of channels.
            if (itr == tokenizer.end())
            {
                DBERROR("Missing channel count.");
                return skeleton;
            }
            int numchans = atoi(itr->c_str());
            ++itr;
            assert(numchans == 3 || numchans == 6); // Only two legitimate settings.
            if (numchans > 3) // Check if position channels are present.
                posIndPtr = posInd;
            for (int i = 0; i < numchans; i++)
            {
                if (itr == tokenizer.end())
                {
                    DBERROR("Unexpected end of file.");
                    return skeleton;
                }
                const char *cstr = itr->c_str();
                if (!strcmp(cstr+1,"position"))
                { // Position.
                    switch (cstr[0])
                    {
                    case 'X':
                        posInd[0] = curIdx+i;
                        break;
                    case 'Y':
                        posInd[1] = curIdx+i;
                        break;
                    case 'Z':
                        posInd[2] = curIdx+i;
                        break;
                    }
                }
                else
                { // Rotation.
                    switch (cstr[0])
                    {
                    case 'X':
                        rotInd[0] = curIdx+i;
                        order[i%3] = 0;
                        break;
                    case 'Y':
                        rotInd[1] = curIdx+i;
                        order[i%3] = 1;
                        break;
                    case 'Z':
                        rotInd[2] = curIdx+i;
                        order[i%3] = 2;
                        break;
                    }
                }
                ++itr;
            }
            curIdx = curIdx + numchans;
        }

        // Create the joint.
        joints.push_back(GPCMJointData(
            name,       // Name.
            parent,     // Parent index.
            offset,     // Offset.
            orderPtr,   // Joint order.
            rotIndPtr,  // Rotation indices.
            posIndPtr   // Position indices.
            ));

        // Figure out what comes next.
        if (itr == tokenizer.end())
        {
            DBERROR("Unexpected end of file.");
            return skeleton;
        }

        // Pop the required number of parents.
        while (!itr->compare("}"))
        {
            // Pop a parent.
            parents.pop_back();
            if (parents.empty())
                break;
            ++itr;
        }
        if (parents.empty() || parents.back() == -1)
            break;

        // Test for the beginning of a new joint.
        if (!itr->compare("JOINT") || !itr->compare("End"))
        {
            ++itr;
        }
        else
        {
            DBERROR("Unexpected token " << *itr << " encountered.");
            return skeleton;
        }
    }

    // Convert joints list into skeleton structure.
    skeleton.channelCount = curIdx;
    skeleton.jointCount = joints.size();
    skeleton.joints = new GPCMJointData[skeleton.jointCount];
    int i = 0;
    for (std::list<GPCMJointData>::iterator itr = joints.begin();
         itr != joints.end(); ++itr)
    {
        skeleton.joints[i] = *itr;
        i++;
    }

    // Return result.
    return skeleton;
}

// Load a corresponding auxiliary data file.
MatrixXd GPCMDataReaderBVH::loadAuxFile(
    std::string filename                    // By default, there is no auxiliary data.
    )
{
    // Look for the annotation file corresponding to this BVH file.
    filename.replace(filename.length()-3,3,"ann");
    std::ifstream annstream(filename.c_str(),std::ios_base::binary);
    if (!annstream)
    {
        return MatrixXd();
    }
    
    // Load annotation file.
    uint32_t cols;
    uint32_t rows;
    annstream.read((char*)&cols,sizeof(uint32_t));
    annstream.read((char*)&rows,sizeof(uint32_t));

    // Create output matrix.
    MatrixXd data(cols,rows);

    // Read the data.
    annstream.read((char*)(data.data()),sizeof(double)*cols*rows);
    
    // Clean up.
    annstream.close();

    // Clip data.
    MatrixXd clippedData = data.block(FIRST_FRAME,0,data.rows()-FIRST_FRAME,data.cols());

    // Return result.
    return clippedData;
}

// Load a single file.
MatrixXd GPCMDataReaderBVH::loadFile(
    std::string filename                    // File to load from.
    )
{
    // Open the file.
    std::ifstream bvhstream(filename.c_str());
    if (!bvhstream)
    {
        DBERROR("Failed to open file " << filename);
        return MatrixXd(0,0);
    }

    // Read in the skeleton information.
    GPCMSkeletonData skeleton = loadSkeleton(bvhstream);

    // Create tokenizer.
    std::istream_iterator<char> istr(bvhstream);
    std::istream_iterator<char> eos;
    boost::tokenizer<boost::char_delimiters_separator<char>,std::istream_iterator<char> >
        tokenizer(istr,eos,boost::char_delimiters_separator<char>(false,"",0));
    boost::tokenizer<boost::char_delimiters_separator<char>,std::istream_iterator<char> >
        ::iterator itr = tokenizer.begin();

    // Read frame rate and count.
    if (itr == tokenizer.end() || itr->compare("Frames:"))
    {
        DBERROR("Missing Frames.");
        return MatrixXd(0,0);
    }
    ++itr;
    if (itr == tokenizer.end())
    {
        DBERROR("Missing frame count.");
        return MatrixXd(0,0);
    }
    int frameCount = atoi(itr->c_str());
    ++itr;
    if (itr == tokenizer.end() || itr->compare("Frame"))
    {
        DBERROR("Missing Frame.");
        return MatrixXd(0,0);
    }
    ++itr;
    if (itr == tokenizer.end() || itr->compare("Time:"))
    {
        DBERROR("Missing Time.");
        return MatrixXd(0,0);
    }
    ++itr;
    if (itr == tokenizer.end())
    {
        DBERROR("Missing frame time.");
        return MatrixXd(0,0);
    }
    double frameTime = atof(itr->c_str());
    ++itr;

    // Read in the joint angles.
    MatrixXd channels(frameCount-FIRST_FRAME,skeleton.channelCount);
    for (int f = 0; f < frameCount; f++)
    {
        for (int j = 0; j < skeleton.channelCount; j++)
        {
            if (itr == tokenizer.end())
            {
                DBERROR("Unexpected end of file.");
                return MatrixXd(0,0);
            }
            if (f >= FIRST_FRAME)
                channels(f-FIRST_FRAME,j) = atof(itr->c_str());
            ++itr;
        }
    }

    // Convert the root rotation to have yaw first.
    transformRoot(skeleton,channels,desiredRootOrder);

    // Smooth out all rotations.
    smoothRotations(skeleton,channels);

    // Transform the root to be relative.
    makeRootRelative(skeleton,channels,frameTime);

    // Convert all rotations into exponential maps.
    convertRotations(skeleton,channels);

    // Create supplementary data.
    if (supplementary) delete supplementary;
    supplementary = new GPCMSupplementaryBVH(skeleton,frameTime,channels.cols(),rotationParam);

    // Return result.
    return channels;
}

// Flip a single joint at each time step.
MatrixXd GPCMDataReaderBVH::flipJoint(
    const MatrixXd &unflipped
    )
{
    DBERROR("Euler angle representation does not yet support flipping!");
    return unflipped;
}

// Apply whatever post-processing is required to the data.
void GPCMDataReaderBVH::postProcessData()
{
    // Get supplementary.    
    GPCMSupplementaryBVH *bvh = dynamic_cast<GPCMSupplementaryBVH*>(supplementary);
    GPCMSkeletonData &skeleton = bvh->getSkeleton();

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
    for (int i = 0; i < skeleton.jointCount; i++)
    {
        int startIdx;
        int endIdx;
        const int *order = skeleton.joints[i].getOrder();
        const int *rotInd = skeleton.joints[i].getRotInd();
        if (rotInd)
        {
            startIdx = ((rotInd[order[0]]-3)/3)*positionEntries + 4;
            endIdx = startIdx + positionEntries;
        }
        else
        {
            continue; // Nothing to do for this joint.
        }

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
        channels = computeVelocities(skeleton,channels,bvh->getFrameTime());

        // Validate that all channels look reasonable.
        validateChannels(skeleton,channels);

        // Set initial scales.
        setInitScales(skeleton,oldY.cols(),channels.cols());

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

// Compute initial scales.
void GPCMDataReaderBVH::setInitScales(
    GPCMSkeletonData &skeleton,             // Skeleton to compute scales for.
    int config,                             // Number of nonvelocity joints.
    int chans                               // Total number of channels.
    )
{
    // Set initial scales.
    // Initial scales are based on joint length in meters
    // Rotational joints are rescaled to 1.0/180.0
    // Root "length" is taken to be 1
    // Root velocity is scaled to meters and multiplied by 10
    MatrixXd scales(1,chans);

    // First set the root.
    scales(0,0) = POSITION_SCALE*ROOT_SCALE*ROOT_VELOCITY_SCALE;
    scales(0,1) = POSITION_SCALE*ROOT_SCALE;
    scales(0,2) = POSITION_SCALE*ROOT_SCALE*ROOT_VELOCITY_SCALE;
    scales(0,3) = ROOT_ROTATION_SCALE*ROOT_SCALE*ROOT_VELOCITY_SCALE;
    for (int k = 4; k < 4+positionEntries; k++)
    {
        scales(0,k) = rotationScale*ROOT_SCALE;
    }
    for (int k = config; k < config+velocityEntries; k++)
    {
        scales(0,k) = rotationVelocityScale*ROOT_SCALE*VELOCITY_SCALE;
    }

    // Next set all of the joints.
    Vector3d u;
    double *ulen = new double[skeleton.jointCount];
    ulen[0] = 0.5*ROTATION_SCALE;
    for (int i = 1; i < skeleton.jointCount; i++)
    {
        const int *order = skeleton.joints[i].getOrder();
        const int *rotInd = skeleton.joints[i].getRotInd();
        u = skeleton.joints[i].getOffset();
        ulen[i] = u.norm()*POSITION_SCALE;
        if (ulen[i] < 1e-8)
            ulen[i] = ulen[skeleton.joints[i].getParent()]; // Zero length joints like the pelvis are actually quite high in weight.
        if (rotInd)
        {
            // Determine index of first position and velocity entries.
            int firstPos = ((rotInd[order[0]]-3)/3)*positionEntries + 4;
            int firstVel = config+((rotInd[order[0]]-3)/3)*velocityEntries;
            for (int k = firstPos; k < firstPos+positionEntries; k++)
            {
                scales(0,k) = ulen[i]*rotationScale;
            }
            for (int k = firstVel; k < firstVel+velocityEntries; k++)
            {
                scales(0,k) = ulen[i]*rotationVelocityScale*VELOCITY_SCALE;
            }
        }
    }
    delete[] ulen;
    
    // Store scales in supplement.
    supplementary->getScale() = scales*OVERALL_SCALE;
}

// Destructor.
GPCMDataReaderBVH::~GPCMDataReaderBVH()
{
}
