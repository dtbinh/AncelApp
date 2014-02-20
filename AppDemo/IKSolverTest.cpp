#include "IKSolverTest.h"
#include "AppDemo.h"
#include "CommandManager.h"

using namespace AncelApp;

template<> IKSolverTest* Ogre::Singleton<IKSolverTest>::msSingleton = nullptr;

IKSolverTest::IKSolverTest()
	:mSkeleton(nullptr),
	mVisibility(false),
	mControlChianNum(0),
	mSphereNode(nullptr),
	mCubeNode(nullptr)
{
 	CommandManager::getInstance().registerCommand("Command_InitActorChain", MyGUI::newDelegate(this, &IKSolverTest::initSkeletonChain));
	CommandManager::getInstance().registerCommand("Command_InitSingleChain", MyGUI::newDelegate(this, &IKSolverTest::initSingleChain));
	CommandManager::getInstance().registerCommand("Command_SetVisibility",MyGUI::newDelegate(this, &IKSolverTest::setVisibility));
	CommandManager::getInstance().registerCommand("Command_ChangeControlChain",MyGUI::newDelegate(this, &IKSolverTest::changeControlChain));
}

IKSolverTest::~IKSolverTest()
{
	delete mSkeleton;
}
void IKSolverTest::initSkeletonChain(const MyGUI::UString& commandName, bool& result)
{
	if(mSkeleton && mSkeleton->getName().find("testskel") != std::string::npos)
		return;
	if(mSkeleton) delete mSkeleton;
	
	if(mCubeNode) mCubeNode->setVisible(false);
	mSkeleton = new AncelApp::Skeleton("testskel");
	mSkeleton->loadSkeletonFromXML("actor\\114.xml");
  
	mSkeleton->attachSkeletonToScene(AppDemo::getSingletonPtr()->mSceneMgr);
	double frame[] = {0.265295, 16.5109, 0.523677, 176.028, -3.74942, -173.601, -0.309412, -2.46091, 3.39509, 3.76268, -3.77892, 3.1325, 4.05949, -1.95106, 1.18222, -6.87132, -9.01508, -22.1276, 4.81544, -11.7956, 28.942, 5.98124, -4.85315, 12.5752, -6.6744, -3.41132, -8.93307, 24.0449, -19.1384, -20.2579, -20.6157, 14.2062, 21.4294, 42.73, -3.16124, -0.194867, 28.0709, 30.7565, -16.9985, -16.4969, -3.75153, 1.52122, - 12.0332, 30.8148, -16.2715, -6.13846};
	std::vector<double> vframe(frame, frame + 46);
 	mSkeleton->update(vframe);

	
 	mIkSolver.initSlover(mSkeleton);
	mIkSolver.removeAllChain();
	
	AncelIK::IKChain* chain = new AncelIK::IKChain();
	chain->initChain(mSkeleton->getRoot(), mSkeleton->getBone("lfoot"));
	mIkSolver.addChain(chain);

	chain = new AncelIK::IKChain();
	chain->initChain(mSkeleton->getRoot(), mSkeleton->getBone("rfoot"));
	mIkSolver.addChain(chain);

	chain = new AncelIK::IKChain();
	chain->initChain(mSkeleton->getRoot(), mSkeleton->getBone("head"));
	mIkSolver.addChain(chain);

	/*chain = new AncelIK::IKChain();
 	chain->initChain(mSkeleton->getBone("lclavicle"), mSkeleton->getBone("lwrist")); 
	mIkSolver.addChain(chain);

	chain = new AncelIK::IKChain();
 	chain->initChain(mSkeleton->getBone("rclavicle"), mSkeleton->getBone("rwrist")); 
	mIkSolver.addChain(chain);*/

	mControlChianNum = 0;
	if(!mSphereNode)
	{
 		mSphereNode = AppDemo::getSingletonPtr()->mSceneMgr->getRootSceneNode()->createChildSceneNode(MyGUI::utility::toString(this)+"Sphere");
	 	Ogre::Entity *ent = AppDemo::getSingletonPtr()->mSceneMgr->createEntity(MyGUI::utility::toString(this) + "_ENSP_", "Sphere_x.mesh");
		ent->setMaterialName("OrangePath");
		ent->setQueryFlags(EQM_TUBE_MASK);
		ent->setUserAny(Ogre::Any(UserAnyPair(Ogre::Any(static_cast<PickableObject*>(this)),Ogre::Any(),Ogre::Any())));
		AncelIK::IKChain *ch = mIkSolver.getChain(0);
		mSphereNode->scale(2,2,2);
		mSphereNode->attachObject(ent);
		mSphereNode->setPosition(ch->getEndEffector()->getGlobalPos() + 5);
 	}
}

void IKSolverTest::initSingleChain(const MyGUI::UString& commandName, bool& result)
{
	if(mSkeleton && mSkeleton->getName().find("SingleChain") != std::string::npos)
		return;
	if(mSkeleton) 		delete mSkeleton;

	mSkeleton = new AncelApp::Skeleton("SingleChain");
	
	Bone* root = mSkeleton->createBone();
  	root->type() = Bone::JT_Rxyz;
	root->name() = "Root";
	root->id() = 0;
	root->boneLength() = 1.0f;
	root->limitsBox(0).x = -180;
	root->limitsBox(0).y =  180;
	root->limitsBox(1).x = -180;
	root->limitsBox(1).y =  180;
    root->limitsBox(2).x = -180;
	root->limitsBox(2).y =  180;
	root->initPose().Q = Ogre::Quaternion();
	root->initPose().T = Ogre::Vector3(0,10,0);
	root->meshName() = "sphere.mesh";
	root->meshOrientation().FromAngleAxis(Ogre::Degree(180),Ogre::Vector3(1.0,0.0,0.0));
  
	Bone* parent = root;
	for(std::size_t i = 1; i < 10; i++)
	{
		Bone* b1 = mSkeleton->createBone(parent);
		b1->type() = Bone::JT_Rxyz;
		b1->name() = "bone_" + MyGUI::utility::toString(i);
		b1->id() = i;
		b1->boneLength() = i;
		b1->limitsBox(0).x = -40;
		b1->limitsBox(0).y =  40;
		b1->limitsBox(1).x = -40;
		b1->limitsBox(1).y =  40;
		b1->limitsBox(2).x = -40;
		b1->limitsBox(2).y =  40;

		b1->initPose().Q = Ogre::Quaternion();
		b1->initPose().T = Ogre::Vector3(0,i,0);
		
		b1->meshName() = "sphere.mesh";
		b1->meshOrientation().FromAngleAxis(Ogre::Degree(180),Ogre::Vector3(1.0,0.0,0.0));
 
		parent = b1;
	}

	mSkeleton->getRoot()->computePosition();
	mSkeleton->attachSkeletonToScene(AppDemo::getSingletonPtr()->mSceneMgr);
	
	mSkeleton->getRoot()->getTheta()[1] = 10;
	mIkSolver.initSlover(mSkeleton);
 	mIkSolver.removeAllChain();
	mIkSolver.addAllChain();
 
	AncelIK::IKChain *ch = mIkSolver.getChain(0);
	ch->localPosition() = Ogre::Vector3(0,10,0);

	if(!mSphereNode)
	{
 		mSphereNode = AppDemo::getSingletonPtr()->mSceneMgr->getRootSceneNode()->createChildSceneNode(MyGUI::utility::toString(this)+"Sphere");
	 	Ogre::Entity *ent = AppDemo::getSingletonPtr()->mSceneMgr->createEntity(MyGUI::utility::toString(this) + "_ENSP_", "Sphere_x.mesh");
		ent->setMaterialName("OrangePath");
		ent->setQueryFlags(EQM_TUBE_MASK);
		ent->setUserAny(Ogre::Any(UserAnyPair(Ogre::Any(static_cast<PickableObject*>(this)),Ogre::Any(),Ogre::Any())));
	  	mSphereNode->scale(2,2,2);
		mSphereNode->attachObject(ent);
		mSphereNode->setPosition(Ogre::Vector3(5,10,0));
 	}

	if(!mCubeNode)
	{
		mCubeNode = AppDemo::getSingletonPtr()->mSceneMgr->getRootSceneNode()->createChildSceneNode(MyGUI::utility::toString(this)+"Cube");
		Ogre::Entity *ent = AppDemo::getSingletonPtr()->mSceneMgr->createEntity(MyGUI::utility::toString(this) + "_CUBE_", "Cube.mesh");
		ent->setMaterialName("Skeleton/Bone/Gold");
		ent->setQueryFlags(EQM_NO_MASK);
		mCubeNode->attachObject(ent);
		mCubeNode->scale(5,5,5);
		mCubeNode->setPosition(0,3,0);
 	}
	mCubeNode->setVisible(true);
	mSphereNode->setVisible(true);
	mControlChianNum = 0;
}

void IKSolverTest::setVisibility(const MyGUI::UString& commandName, bool& result)
{
	if(mSkeleton)
	{
		if(mCubeNode)
		{
			if(mSkeleton->getName().find("SingleChain") == std::string::npos)
				 mCubeNode->setVisible(false);
			else
				mCubeNode->setVisible(!mVisibility);
		}
 		mSkeleton->setVisibility(!mVisibility);
		mSphereNode->setVisible(!mVisibility);
 		mVisibility = !mVisibility;
 	}
}

bool  IKSolverTest::notifyReleased()
{
	return true;
}
bool  IKSolverTest::notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray)
{
 	Ogre::Vector3 position = ray.getPoint(mCollsionDepth) + mPickedNodeOffset;
	
	/*position.x = mSphereNode->getPosition().x;
	position.z = mSphereNode->getPosition().z;*/

	mSphereNode->setPosition(position);	
	mIkSolver.setEndEffectorGoal(mControlChianNum,position);
  
	/*static Ogre::Vector3 c0 = mIkSlover.getChain(0)->getEndEffector()->getGlobalPos();
	static Ogre::Vector3 c1 = mIkSlover.getChain(1)->getEndEffector()->getGlobalPos();
	static Ogre::Vector3 c2 = mIkSlover.getChain(2)->getEndEffector()->getGlobalPos();
	static Ogre::Vector3 c4 = mIkSlover.getChain(4)->getEndEffector()->getGlobalPos();
	mIkSlover.getChain(0)->weightPos() = 5;
	mIkSlover.getChain(1)->weightPos() = 5;
	mIkSlover.getChain(2)->weightPos() = 5;
	mIkSlover.getChain(4)->weightPos() = 15;
	mIkSlover.setEndEffectorGoal(0,c0);
	mIkSlover.setEndEffectorGoal(1,c1);
	mIkSlover.setEndEffectorGoal(2,c2);
	mIkSlover.setEndEffectorGoal(4,c4);*/
//	mIkSlover.setEndEffectorGoal(5,mIkSlover.getChain(5)->getEndEffector()->getGlobalPos());
	mIkSolver.solve();
 	return true;
}

void IKSolverTest::changeControlChain(const MyGUI::UString& commandName, bool& result)
{
	if(mSkeleton)
		mControlChianNum = (mControlChianNum + 1)%(mIkSolver.getChianNum());
}

bool  IKSolverTest::notifyPicked(Ogre::RaySceneQueryResultEntry &entry,const Ogre::Vector3& offset)
{
	if(!mSkeleton) return false;

	mCollsionDepth = entry.distance;
 	mPickedNodeOffset = offset;
 
	return true;
}
