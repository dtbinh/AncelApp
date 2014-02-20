#include "MotionSyn.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include "Quaternion.h"
#include "ResUtility.h"

using namespace ResModel;
using namespace ResUtil;

MotionSyn::MotionSyn(std::string modelName)
	:mNumFactorDim(0)
{
  	bool ret = loadModel(modelName);
	assert(ret);
	
	/*std::ofstream fout("factor.txt");
	for(std::size_t t = 0; t < mFactors.size(); t++)
	{
		fout << *mFactors[t];
		fout << std::endl;
	}*/
	
	initStoreage();
}

MotionSyn::~MotionSyn()
{
	if(mKernel != NULL)
		delete mKernel;
	 
 	for (size_t i = 0; i < mFactors.size(); i++)
	{
		delete mFactors[i];
		delete mFakeFactors[i];
	}
 
	for (size_t i = 0; i < mGradTs.size(); i++)
		delete mGradTs[i];
   
	delete mGPM;
}

void MotionSyn::initSynthesis()
{
 	computeFakeFactor();
	mKernel->computeKernel(mK,mFakeFactors);
 	mInvK = mK;
	MMatrix::invertMMatrix(mInvK);
 	computeBilinearMapping();

	//std::ofstream fout("fa.txt");
	//for(std::size_t i = 0; i < mFactors[0]->sizeRow(); i++)
	//{
 //			for(std::size_t t = mSegments[i]; t < mSegments[i+1]; t ++)
	//		{
	//			fout << mFactors[0]->get(i,0) + mFactors[0]->get(i,1)* (t - mSegments[i]) << ", ";
	//		}
	//		fout << std::endl;
 //	}
}
bool MotionSyn::loadModel(std::string modelName)
{
	std::ifstream loader(modelName,std::ios::binary|std::ios::in);
	
	if(loader.fail()) 
		return false;
 
	std::size_t numActor;
	loader.read((char*)(&numActor),sizeof(size_t));
	mActorLs.resize(numActor);
	for(std::size_t i = 0; i < numActor; i++)
	{
		char actor[100];
		loader.read(actor,100);
		std::stringstream converter(actor);
 		converter >> mActorLs[i];
	}

	//action
	std::size_t numAction;
	loader.read((char*)(&numAction),sizeof(size_t));
	mActionLs.resize(numAction);
 	for(std::size_t i = 0; i < numAction; i++)
	{
		char actor[100];
		loader.read(actor,100);
		std::stringstream converter(actor);
 		converter >> mActionLs[i];
	}
 
	//motion Name Ls
	std::size_t numMotion;
	loader.read((char*)(&numMotion),sizeof(size_t));
	mMotionNameLs.resize(numMotion);
 	for(std::size_t i = 0; i < numMotion; i++)
	{
		char motion[100];
		loader.read(motion,100);
		std::stringstream converter(motion);
 		converter >> mMotionNameLs[i];
	}

	char actor[100];
	loader.read(actor,100);
	std::stringstream converter(actor);
	converter >> mModelName;
	
	std::size_t row,col;
	
	//mY
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mY.resize(row,col);
	loader.read((char*)(mY.gets()),sizeof(double) * mY.getElemNum());
	
	//mCentredY
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mCentredY.resize(row,col);
	loader.read((char*)(mCentredY.gets()),sizeof(double) * mCentredY.getElemNum());

	//mMeanY
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mMeanY.resize(row,col);
	loader.read((char*)(mMeanY.gets()),sizeof(double) * mMeanY.getElemNum());

	//mVarY
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mVarY.resize(row,col);
	loader.read((char*)(mVarY.gets()),sizeof(double) * mVarY.getElemNum());

	//mW
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mW.resize(row,col);
	loader.read((char*)(mW.gets()),sizeof(double) * mW.getElemNum());
	
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mInitY.resize(row,col);
	loader.read((char*)(mInitY.gets()),sizeof(double) * mInitY.getElemNum());

	//TODO constant value 3
	// Factors
	mFactors.resize(3);
	for(std::size_t i = 0; i < 3; i++)
	{
		loader.read((char*)(&row),sizeof(size_t));
		loader.read((char*)(&col),sizeof(size_t));
		mFactors[i] = new MMatrix(row,col);
		loader.read((char*)(mFactors[i]->gets()),sizeof(double) * mFactors[i]->getElemNum());
 	}
	//params
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	MMatrix tempG(row,col);
	loader.read((char*)(tempG.gets()),sizeof(double) * tempG.getElemNum());
	
	mKernel = new ResCore::MGPKernel(mY.sizeRow());
	mKernel->setPrams(tempG);

	std::size_t size;
	loader.read((char*)(&size),sizeof(size_t));
	mSegments.resize(size);
	loader.read((char*)(&mSegments[0]),sizeof(size_t)*size);
   
	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mSegmentLabel.resize(row);
 	for(std::size_t i = 0; i < row; i++)
	{
		mSegmentLabel[i].resize(col);
		loader.read((char*)(&mSegmentLabel[i][0]),sizeof(size_t)*col);
 	}

	loader.read((char*)(&row),sizeof(size_t));
	loader.read((char*)(&col),sizeof(size_t));
	mFacIndex.resize(row);

	for(std::size_t i = 0; i < row; i++)
	{
		mFacIndex[i].resize(col);
		loader.read((char*)(&mFacIndex[i][0]),sizeof(size_t)*col);
  	}

	//std::cout << mW << std::endl;
	return true;
}
void MotionSyn::computeFakeFactor()
{
	for(size_t i = 0; i < mFactors.size(); i++)
	{
		if(i == 0)
			toCircle(*mFakeFactors[i]);
		else 
		{
			for(size_t j = 0; j < mNumData; j++)
			{
				for(size_t k = 0; k < mFactors[i]->sizeCol(); k++)
 					mFakeFactors[i]->assign(mFactors[i]->get(mFacIndex[j][i],k), j, k);
  	 		}
		}
 	}
}
void MotionSyn::toCircle(MMatrix & mat) const
{
	for (size_t i = 1; i < mSegments.size(); i++)
	{
		double theta = mFactors[0]->get(i-1, 0);
		double delta = mFactors[0]->get(i-1, 1);

		for (size_t j = mSegments[i-1]; j < mSegments[i]; j++)
		{
			double cosTheta = cos(theta + (j - mSegments[i-1]) * delta);
			double sinTheta = sin(theta + (j - mSegments[i-1]) * delta);

			mat.assign(cosTheta, j, 0);
			mat.assign(sinTheta, j, 1);

			//precompute for saving time
			//mGradTs[0]->assign(-sinTheta, j, 0);
			//mGradTs[0]->assign( cosTheta, j, 1);
			//mGradTs[1]->assign(-sinTheta*(j - mSegments[i-1]), j, 0);
			//mGradTs[1]->assign( cosTheta*(j - mSegments[i-1]), j, 1);
		}
	}
}
void MotionSyn::initStoreage()
{
	mNumDim  = mY.sizeCol();
	mNumData = mY.sizeRow();
	
	mK.resize(mNumData, mNumData);
	mInvK.resize(mNumData, mNumData);
  
	mGradTs.push_back(new MMatrix(mNumData, 2));
	mGradTs.push_back(new MMatrix(mNumData, 2));
  	 
	for (std::size_t i = 0; i < mFactors.size(); i++)
	{
		mFakeFactors.push_back(new MMatrix(mNumData,	mFactors[i]->sizeCol()));
	 	mNumFactorDim += mFactors[i]->sizeCol();
	}
 
	//std::cout << *mFactors[0] << std::endl;
	//std::cout << *mFactors[1] << std::endl;
	//std::cout << *mFactors[2] << std::endl;
	mFactorsInvCov.resize(mNumFactorDim, mNumFactorDim);
	mFactorsMean.resize(1 ,mNumFactorDim);
	mFactorsSample.resize(mSegmentLabel.size(),mNumFactorDim);
 }

void MotionSyn::computeBilinearMapping()
{
  	MMatrix A(mSegments.size() - 1, mFactors[1]->sizeCol() * mFactors[2]->sizeCol());
	MMatrix B(mSegments.size() - 1, 1);
	
	size_t col1 = mFactors[1]->sizeRow();
	size_t col2 = mFactors[2]->sizeRow();

	int curIndex = 0;
	for (size_t i = 0; i < col1; i++)
	{
		for (size_t j = 0; j < col2; j++)
		{
			int  index = -1;
			for (std::size_t t = 0; t < mSegmentLabel.size(); t++)
			{
				if (i == mSegmentLabel[t][0] && j == mSegmentLabel[t][1])
				{
					index = int(t);
					break;
				}
 			}
 			if (index >= 0)
			{
				for (size_t t = 0; t < mFactors[1]->sizeCol(); t++)
				{
					for (size_t k = 0; k < mFactors[2]->sizeCol(); k++)
					{
						double val = mFactors[1]->get(i, t) * mFactors[2]->get(j, k);
						A.assign(val, curIndex, t * mFactors[2]->sizeCol() + k);
					}
				}
		 		B.assign(mFactors[0]->get(index, 1), curIndex);
				curIndex++;
			}
		}
	}
//	std::cout << A << std::endl; 
	mGPM  = new GuaasinProcessModel(B,A);
	mGPM->optimize(100);
	MMatrix ret = mGPM->predict(A);
}
ResUtil::MMatrix MotionSyn::testTransitionSyn()
{
	std::vector<MMatrix> xStar(3);
	int length = 200;
	
	xStar[0].resize(length,2);
	xStar[1].resize(length, mFactors[1]->sizeCol());
	xStar[2].resize(length, mFactors[2]->sizeCol());
 
	return MMatrix();
}
MMatrix MotionSyn::generate(std::size_t identity,vector<std::size_t> contents,std::size_t interval)
{
	std::size_t state_size = contents.size() * 2 - 1;
 	std::size_t length = interval * state_size;
	
	MMatrix motion(length,mInitY.sizeCol());
	
	std::vector<MMatrix> xStar(3);
	xStar[0].resize(length, 2);
	xStar[1].resize(length, mFactors[1]->sizeCol());
	xStar[2].resize(length, mFactors[2]->sizeCol());
 
	double current_state = 0;
	for (std::size_t i = 0; i < state_size; i++)
	{
		MMatrix kron(1,mFactors[1]->sizeCol() * mFactors[2]->sizeCol());
		if (i % 2 == 0)
		{
 			MMatrix mat1 = mFactors[1]->subMMatrix(identity, 0, 1, mFactors[1]->sizeCol());
			MMatrix mat2 = mFactors[2]->subMMatrix(contents[i/2], 0, 1, mFactors[2]->sizeCol());
 
			kron = mat1.kron(mat2);
 			MMatrix val = mGPM->predict(kron);

			double step = val.get(0);

			for (std::size_t t = 0; t < interval; t++)
			{
				xStar[0].assign(cos(current_state + double(t*step)), t + i * interval, 0);
				xStar[0].assign(sin(current_state + double(t*step)), t + i * interval, 1);
  				xStar[1].copyRowRow(t + i * interval, *mFactors[1], identity);
				xStar[2].copyRowRow(t + i * interval, *mFactors[2], contents[i/2]);
			}
			current_state += step * interval;
 		}
		else
		{
			for (std::size_t t = 0; t < interval; t++)
			{
				MMatrix linearIpconent(1,mFactors[2]->sizeCol());
				for (std::size_t k = 0; k < linearIpconent.sizeCol(); k++)
				{
					double val = (1 - double(t) / interval) * mFactors[2]->get(contents[(i-1)/2], k) 
								   + (double(t) / interval) * mFactors[2]->get(contents[(i+1)/2], k); 

					linearIpconent.assign(val, 0, k);
				}
		 
 				MMatrix mat = mFactors[1]->subMMatrix(identity, 0, 1, mFactors[1]->sizeCol());
				
				kron = mat.kron(linearIpconent);
 
				MMatrix val = mGPM->predict(kron);
				double step = val.get(0);

				current_state += step;

				xStar[0].assign(cos(current_state), t + i * interval, 0);
				xStar[0].assign(sin(current_state), t + i * interval, 1);

				xStar[1].copyRowRow(t + i * interval, *mFactors[1], identity);
				xStar[2].copyRowRow(t + i * interval, linearIpconent, 0);
			}
		}
	}
 	return meanPrediction(xStar,CVector3D<double>(0,mInitY.get(identity,1),0));
}

void MotionSyn::calcFactorsPrior()
{
  	mFactorsSample.assign(0);
	
	// construct samples of joint probability distribution (the factors's probability distribution)
	for (std::size_t i = 0; i < mSegmentLabel.size(); i++)
	{
		std::size_t offset = 0;

		for (std::size_t j = 0; j < mSegmentLabel[i].size(); j++)
		{
			mFactorsSample.copyMMatrix(i, offset, *mFactors[j],
				mSegmentLabel[i][j], mSegmentLabel[i][j] + 1, 0, mFactors[j]->sizeCol());
			offset += mFactors[j]->sizeCol();
		}
	}
	
	mFactorsMean = mFactorsSample.meanCol();

	// calculate  covariance of joint probability distribution
	std::size_t col = mFactorsSample.sizeRow();
	mFactorsInvCov.gemm(mFactorsSample, mFactorsSample, 1.0/double(col), 0, "T", "N");
	mFactorsInvCov.gemm(mFactorsMean, mFactorsMean, -1.0, 1.0, "T", "N");
	MMatrix::invertMMatrix(mFactorsInvCov);

}

MMatrix MotionSyn::synTrainsiton(const std::size_t identity, const std::size_t content1,const std::size_t content2,
								const std::size_t length, CVector3D<double> initPos, double &curState)
{
	std::vector<MMatrix> xStar(3);
  
	xStar[0].resize(length,2);
	xStar[1].resize(length, mFactors[1]->sizeCol());
	xStar[2].resize(length, mFactors[2]->sizeCol());
	
	MMatrix actor = mFactors[1]->subMMatrix(identity,0,1,mFactors[1]->sizeCol());

	/*std::vector<double> steps;
	double total = 0.0;
	for (std::size_t i = 0; i < length; i++)
	{
		MMatrix newContent(1,mFactors[2]->sizeCol());

		for(std::size_t t = 0; t < newContent.sizeCol(); t++)
		{
				double val =  (1 - double(t)/length) * mFactors[2]->get(content1,t) 
			  				  +		double(t)/length * mFactors[2]->get(content2,t);
				newContent.assign(val,t);
		}
		MMatrix kron = actor.kron(newContent);
		MMatrix val = mGPM->predict(kron);
		total += val.get(0);
		steps.push_back(val.get(0));
		xStar[1].copyRowRow(i,*mFactors[1],identity);
		xStar[2].copyRowRow(i,newContent,0);
	}
	total = 6.28 - total;
	for (std::size_t i = 0; i < length; i++)
	{
		total += steps[i];
		 xStar[0].assign(cos(6.28-steps[i]*(length-i)), i, 0);
		 xStar[0].assign(sin(6.28-steps[i]*(length-i)), i, 1);
	}*/
 
	for (std::size_t i = 0; i < length; i++)
	{
	 	if(i < 50)
		{
			MMatrix newContent(1,mFactors[2]->sizeCol());
			for(std::size_t t = 0; t < newContent.sizeCol(); t++)
			{
				double val =  (1 - double(t)/50) * mFactors[2]->get(content1,t) 
								+ double(t)/50 * mFactors[2]->get(content2,t);
				newContent.assign(val,t);
			}
	 		MMatrix kron = actor.kron(newContent);
			MMatrix val = mGPM->predict(kron);
			double step  = val.get(0);
			curState += step;
			xStar[2].copyRowRow(i,newContent,0);


		/*	MMatrix content = mFactors[2]->subMMatrix(content1,0,1,mFactors[2]->sizeCol());
			MMatrix kron = actor.kron(content);
			MMatrix val = mGPM->predict(kron);
			double step  = val.get(0);
			curState += step;
			xStar[2].copyRowRow(i,content,0);*/

 		}
		else
		{
			MMatrix content = mFactors[2]->subMMatrix(content2,0,1,mFactors[2]->sizeCol());
			MMatrix kron = actor.kron(content);
			MMatrix val = mGPM->predict(kron);
			double step  = val.get(0);
			curState += step;
			xStar[2].copyRowRow(i,content,0);
		}

		xStar[0].assign(cos(curState), i, 0);
		xStar[0].assign(sin(curState), i, 1);
		xStar[1].copyRowRow(i,*mFactors[1],identity);
		
	}
	return meanPrediction(xStar,initPos);
}

MMatrix MotionSyn::evaluateFactors(MMatrix unknownMotion)
{
	return MMatrix();
}

MMatrix MotionSyn::meanPrediction(const std::vector<MMatrix> &X ,CVector3D<double> initPos)
{
	std::vector<const MMatrix*> tempX;
	
	for(std::size_t t = 0; t < X.size(); t++)
		tempX.push_back(&X[t]);

	std::size_t motionLen = X[0].sizeRow();
	MMatrix Kx(motionLen, mNumData);
	mKernel->computeKernel(Kx, tempX, mFakeFactors);
	 
	MMatrix &Ymean = Kx * mInvK * mCentredY;
	
	for(std::size_t i = 0; i < Ymean.sizeCol(); i++)
	{
		Ymean.scaleCol(i, sqrt(mVarY.get(i)));
 	}

	MMatrix meanData;
	meanData.repmat(mMeanY, motionLen, 1);
 	
 	Ymean += meanData; 
 
	
	MMatrix motion(Ymean.sizeRow(), mInitY.sizeCol());
	motion.copyMMatrix(0, 0, Ymean, 0, Ymean.sizeRow(), 0, mInitY.sizeCol());
	
	//TODO
	motion.assign(initPos.x, 0, 0);
	motion.assign(initPos.y, 0, 1);
	motion.assign(initPos.z, 0, 2);
	 
	for (std::size_t i = 1; i < motionLen; i++)
	{
	/*	motion.assign(motion.get(i, 0), i, 0);
		motion.assign(motion.get(i, 1), i, 1);
		motion.assign(motion.get(i, 2), i, 2);*/

		motion.add(motion.get(i-1, 0), i, 0);
		motion.add(motion.get(i-1, 1), i, 1);
		motion.add(motion.get(i-1, 2), i, 2);
	}
  	
	mocapToEulerAngle(motion);
  
	for(std::size_t i = 3; i < motion.sizeCol(); i++)
	{
		motion.scaleCol(i, 180.0/M_PI);
 	}

	//for(std::size_t i = 0; i < motion.sizeRow(); i++)
	//{
	//	std::cout << motion.get(i,0) <<  " ,";
	//	std::cout << motion.get(i,1) <<  " ,";
	//	std::cout << motion.get(i,2) <<  " ,";
	//	std::cout << motion.get(i,3) <<  " ,";
	//	std::cout << motion.get(i,4) <<  " ,";
	//	std::cout << motion.get(i,5) <<  std::endl;
 //	}
	//
	return motion;
}

MMatrix MotionSyn::syc()
{
	int subject =0;

	MMatrix stylemat(1,mFactors[2]->sizeCol());
	stylemat.copyRowRow(0,*mFactors[2],0);
	
	stylemat.scale(0.5);
	stylemat.axpyRowRow(0,*mFactors[2],1,0.5);
	std::cout << stylemat << std::endl;
	assert(mFactors.size() == 3);
	//assert(subject < mFactors[1]->sizeRow() && style < mFactors[2]->sizeRow());

	MMatrix mat(1,mFactors[1]->sizeCol() * mFactors[2]->sizeCol());

	for(size_t t = 0; t < mFactors[1]->sizeCol(); t++)
	{
		for(size_t k = 0; k < mFactors[2]->sizeCol(); k++)
		{
			double val = mFactors[1]->get(subject,t) * stylemat.get(0,k);
			mat.assign(val,t * stylemat.sizeCol() + k);
		}
	}

	MMatrix val = mGPM->predict(mat);
	double step  = val.get(0);
	int length = int(2*3.141592653589/step);

	std::vector<MMatrix> X(3);
	X[0].resize(length,2);
	X[1].resize(length, mFactors[1]->sizeCol());
	X[2].resize(length, mFactors[2]->sizeCol());

	for(int i = 0; i < length; i++)
	{
		X[0].assign(cos(double(i * step)), i, 0);
		X[0].assign(sin(double(i * step)), i, 1);
   		X[1].copyRowRow(i, *mFactors[1], subject);
		X[2].copyRowRow(i, stylemat, 0);
	}

	return meanPrediction(X,CVector3D<double>(0,mInitY.get(subject,1),0));
}

MMatrix MotionSyn::syc(size_t subject, size_t style)
{
	assert(mFactors.size() == 3);
	assert(subject < mFactors[1]->sizeRow() && style < mFactors[2]->sizeRow());

	MMatrix mat(1,mFactors[1]->sizeCol() * mFactors[2]->sizeCol());

	for(size_t t = 0; t < mFactors[1]->sizeCol(); t++)
	{
		for(size_t k = 0; k < mFactors[2]->sizeCol(); k++)
		{
			double val = mFactors[1]->get(subject,t) * mFactors[2]->get(style,k);
			mat.assign(val,t * mFactors[2]->sizeCol() + k);
		}
	}
	MMatrix val = mGPM->predict(mat);
	double step  = val.get(0);

	int length = int(2*3.141592653589/step);

	std::vector<MMatrix> X(3);
	X[0].resize(length,2);
	X[1].resize(length, mFactors[1]->sizeCol());
	X[2].resize(length, mFactors[2]->sizeCol());
 
	for(int i = 0; i < length; i++)
	{
		X[0].assign(cos(double(i * step)), i, 0);
		X[0].assign(sin(double(i * step)), i, 1);
   		X[1].copyRowRow(i, *mFactors[1], subject);
		X[2].copyRowRow(i, *mFactors[2], style);
	}
 
	return meanPrediction(X,CVector3D<double>(0,mInitY.get(subject,1),0));

}
MMatrix MotionSyn::synthesis(size_t subject,size_t gait,size_t length)
{
  	assert(mFactors.size() == 3);
	assert(subject < mFactors[1]->sizeRow() && gait < mFactors[2]->sizeRow());

	MMatrix mat(1,mFactors[1]->sizeCol() * mFactors[2]->sizeCol());

	for(size_t t = 0; t < mFactors[1]->sizeCol(); t++)
	{
		for(size_t k = 0; k < mFactors[2]->sizeCol(); k++)
		{
			double val = mFactors[1]->get(subject,t) * mFactors[2]->get(gait,k);
			mat.assign(val,t * mFactors[2]->sizeCol() + k);
		}
	}
	MMatrix val = mGPM->predict(mat);
	double step  = val.get(0);
 
	std::vector<MMatrix> X(3);
	X[0].resize(length,2);
	X[1].resize(length, mFactors[1]->sizeCol());
	X[2].resize(length, mFactors[2]->sizeCol());
 
	for(size_t i = 0; i < length; i++)
	{
		X[0].assign(cos(double(i * step)), i, 0);
		X[0].assign(sin(double(i * step)), i, 1);
   		X[1].copyRowRow(i, *mFactors[1], subject);
		X[2].copyRowRow(i, *mFactors[2], gait);
	}
 
	return meanPrediction(X,CVector3D<double>(0,mInitY.get(subject,1),0));
}
MMatrix MotionSyn::reconstruct(std::size_t motionIndex)
{
	assert(motionIndex < mSegmentLabel.size());
	std::size_t identity =  mSegmentLabel[motionIndex][0];
	std::size_t content  =  mSegmentLabel[motionIndex][1];

	std::size_t length = mSegments[motionIndex + 1] - mSegments[motionIndex];

	double step = mFactors[0]->get(motionIndex,1);

	std::vector<MMatrix> X(3);
	X[0].resize(length,2);
	X[1].resize(length, mFactors[1]->sizeCol());
	X[2].resize(length, mFactors[2]->sizeCol());

	for(size_t i = 0; i < length; i++)
	{
		X[0].assign(cos(mFactors[0]->get(motionIndex,0) + double(i * step)), i, 0);
		X[0].assign(sin(mFactors[0]->get(motionIndex,0) + double(i * step)), i, 1);
   		X[1].copyRowRow(i, *mFactors[1], identity);
		X[2].copyRowRow(i, *mFactors[2], content);
	}
	//std::cout << X[0] << std::endl;
	return meanPrediction(X,CVector3D<double>(0,mInitY.get(motionIndex,1),0));
}
std::size_t MotionSyn::getMotionSemgemtnNum()
{
	return mSegmentLabel.size();
}

const std::string& MotionSyn::getModelName() const
{
	return mModelName;
}

const std::vector<std::string>& MotionSyn::getFactorAList() const
{
	return mActorLs;
}
const std::vector<std::string>& MotionSyn::getFactorBList() const
{
	return mActionLs;
}
const std::vector<std::string>& MotionSyn::getMotionNameList() const
{
	return mMotionNameLs;
}