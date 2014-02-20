#include "MotionSynthesisPanel.h"
#include "MotionManager.h"
#include "AppDemo.h"
#include "AppUtility.h"
#include "CommandManager.h"
#include "StatePanel.h"

using namespace AncelApp;

template<> MotionSynthesisPanel* Ogre::Singleton<MotionSynthesisPanel>::msSingleton = 0;

MotionSynthesisPanel::MotionSynthesisPanel()
	:mCBFactorA(nullptr),
	 mCBFactorB(nullptr),
	  wraps::BaseLayout("MotionSynthesisPanel.layout")
{
	CommandManager::getInstance().registerCommand("Command_ShowPanel", MyGUI::newDelegate(this, &MotionSynthesisPanel::showPanel));
	mMainWidget->setVisible(false);
	MyGUI::WindowPtr winPtr = static_cast<MyGUI::WindowPtr>(mMainWidget);
	winPtr->eventWindowButtonPressed += MyGUI::newDelegate(this,&MotionSynthesisPanel::windowButtonPressed);

	assignWidget(mCBModel,"CB_MODEL");

	assignWidget(mCBMotion,"CB_MOTION");

	assignWidget(mCBFactorA,"CB_FACTOR_A");
	assignWidget(mCBFactorB,"CB_FACTOR_B");
	
	assignWidget(mCBActor,"CB_ACTOR_LIST");
	assignWidget(mCBContent,"CB_CONTENT_LIST");



	assignWidget(mEBWightFactorA,"EB_FACTORA_WEIGHT");
	assignWidget(mEBWightFactorB,"EB_FACTORB_WEIGHT");

	assignWidget(mEBInterval,"EB_INTERVAL");
	assignWidget(mEBMotionLength,"EB_MOTION_LENGTH");
  
	assignWidget(mListsContent,"IB_CONTENTS");

	assignWidget(mBtReconstruction,"BT_RECONSTRUCTION");
	assignWidget(mBtSynthesisMotion,"BT_SYNTHESIS");
	assignWidget(mBtSynthesisSmooth,"BT_SYN_TRANSITION");

	assignWidget(mBtLoad,"BT_LOAD");
	assignWidget(mBtRemove,"BT_REMOVE");
	assignWidget(mBtAdd, "BT_ADD");

	mBtReconstruction->eventMouseButtonClick += MyGUI::newDelegate(this, &MotionSynthesisPanel::reconstruction);
	mBtSynthesisMotion->eventMouseButtonClick +=  MyGUI::newDelegate(this, &MotionSynthesisPanel::synInterpolation);
	mBtSynthesisSmooth->eventMouseButtonClick +=  MyGUI::newDelegate(this, &MotionSynthesisPanel::synTransition);

	mCBModel->eventEditSelectAccept += MyGUI::newDelegate(this, &MotionSynthesisPanel::selectedChanged);
  	
	mBtAdd->eventMouseButtonClick += MyGUI::newDelegate(this, &MotionSynthesisPanel::addContent);
	mBtLoad->eventMouseButtonClick += MyGUI::newDelegate(this, &MotionSynthesisPanel::loadModel);
	mBtRemove->eventMouseButtonClick += MyGUI::newDelegate(this, &MotionSynthesisPanel::removeModel);	
}

void MotionSynthesisPanel::windowButtonPressed(MyGUI::Window* _sender, const std::string& _name)
{
	mMainWidget->setVisible(false);
}
MotionSynthesisPanel::~MotionSynthesisPanel()
{
	/*std::vector<ResModel::HMGPModel*>::iterator it = mModelList.begin();
	for (it ; it != mModelList.end(); it++)
	{
		if(*it != nullptr)
			delete *it;
	}*/
}
void MotionSynthesisPanel::showPanel(const MyGUI::UString& commandName, bool& result)
{
 	mMainWidget->setVisible(!mMainWidget->getVisible());
}
void MotionSynthesisPanel::initialise()
{
	 
 //	MyGUI::Button* btLoad = nullptr;
	//assignWidget(    btLoad,    "BTLoad", false);
	//assignWidget(  mCBModel,   "CBModel", false);
	//assignWidget(mCBFactorA, "CBFactorA", false);
	//assignWidget(mCBFactorB, "CBFactorB", false);
	//assignWidget(mBtGeneration,  "BTGen", false);

	//mCBModel->eventEditSelectAccept += MyGUI::newDelegate(this, &MotionSynthesisPanel::selectedChanged);
	//btLoad->eventMouseButtonClick +=   MyGUI::newDelegate(this, &MotionSynthesisPanel::loadModel);
	//mBtGeneration->eventMouseButtonClick += MyGUI::newDelegate(this, &MotionSynthesisPanel::generateMotion);
	//
	//assignWidget(    btLoad,    "BTVis", false);
	//btLoad->eventMouseButtonClick += MyGUI::newDelegate(this, &MotionSynthesisPanel::visualX);
}
void MotionSynthesisPanel::shutdown()
{

}

void MotionSynthesisPanel::selectedChanged(MyGUI::Widget* sender)
{
	/*std::size_t index = mCBModel->getIndexSelected();
	if(index != MyGUI::ITEM_NONE)
	{
		const std::vector<std::string> nameALs = mModelList[index]->getFactorAList();
		const std::vector<std::string> nameBLs = mModelList[index]->getFactorBList();
		
		mCBFactorA->deleteAllItems();
		for(std::size_t i = 0; i < nameALs.size(); i++)
		{
			mCBFactorA->addItem(nameALs[i]);
		}
		assert(nameALs.size() > 0);
		mCBFactorA->setIndexSelected(0);
		
		mCBFactorB->deleteAllItems();
		for(std::size_t i = 0; i < nameBLs.size(); i++)
		{
			mCBFactorB->addItem(nameBLs[i]);
		}
		assert(nameBLs.size() > 0);
		mCBFactorB->setIndexSelected(0);	
	}*/
}
void MotionSynthesisPanel::removeModel(MyGUI::Widget* sender)
{
	std::size_t index = mCBModel->getIndexSelected();
	if (index != MyGUI::ITEM_NONE)
	{
		mCBModel->removeItemAt(index);
		mCBModel->clearIndexSelected();

		mCBMotion->removeAllItems();
		mCBMotion->clearIndexSelected();

		mCBActor->removeAllItems();
		mCBActor->clearIndexSelected();
		
		mCBContent->removeAllItems();
		mCBContent->clearIndexSelected();
		
		mCBFactorA->removeAllItems();
		mCBFactorA->clearIndexSelected();
		
		mCBFactorB->removeAllItems();
		mCBFactorB->clearIndexSelected();

		mModelList.erase(mModelList.begin() + index);
	}
}
void MotionSynthesisPanel::addContent(MyGUI::Widget* sender)
{
	if(mCBContent->getItemCount() > 0 && mCBContent->getItemIndexSelected() != MyGUI::ITEM_NONE)
	{
		mListsContent->addItem(mCBContent->getCaption(),MyGUI::Any(mCBContent->getIndexSelected()));
	}
}
void MotionSynthesisPanel::loadModel(MyGUI::Widget* sender)
{
	unsigned long hWnd;
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

	std::string fileName = AncelApp::loadFile("MGPM", HWND(hWnd));
	if(!fileName.empty())
	{
		ResModel::MotionSyn * synthesizer = new ResModel::MotionSyn(fileName);
		mModelList.push_back(synthesizer);

		mCBModel->deleteAllItems();
		for(std::size_t i = 0; i < mModelList.size(); i++)
		{
			mCBModel->addItem(mModelList[i]->getModelName());
		}
		mCBModel->setIndexSelected(mCBModel->getItemCount() - 1);

		for(std::size_t i = 0; i < synthesizer->getMotionNameList().size(); i++)
		{
			mCBMotion->addItem(synthesizer->getMotionNameList()[i]);
		}
		mCBMotion->setIndexSelected(0);

		for(std::size_t i = 0; i < synthesizer->getFactorAList().size(); i++)
		{
			mCBFactorA->addItem(synthesizer->getFactorAList()[i]);
		}
		mCBFactorA->setIndexSelected(0);

		for(std::size_t i = 0; i < synthesizer->getFactorBList().size(); i++)
		{
			mCBFactorB->addItem(synthesizer->getFactorBList()[i]);
		}
		mCBFactorB->setIndexSelected(0);


		for(std::size_t i = 0; i < synthesizer->getFactorAList().size(); i++)
		{
			mCBActor->addItem(synthesizer->getFactorAList()[i]);
		}
		mCBActor->setIndexSelected(0);

		for(std::size_t i = 0; i < synthesizer->getFactorBList().size(); i++)
		{
			mCBContent->addItem(synthesizer->getFactorBList()[i]);
		}
		mCBContent->setIndexSelected(0);
		synthesizer->initSynthesis();
	}
	//	
	//	//when deal with different model, A and B represent different thing
	//	//For example when model is style-identity, then A is style and B is identity
 //		const std::vector<std::string> nameALs = mModelList[mModelList.size() - 1]->getFactorAList();
	//	const std::vector<std::string> nameBLs = mModelList[mModelList.size() - 1]->getFactorBList();
	//	
	//	mCBFactorA->deleteAllItems();
	//	for(std::size_t i = 0; i < nameALs.size(); i++)
	//	{
	//		mCBFactorA->addItem(nameALs[i]);
	//	}
	//	assert(nameALs.size() > 0);
	//	mCBFactorA->setIndexSelected(0);
	//	
	//	mCBFactorB->deleteAllItems();
	//	for(std::size_t i = 0; i < nameBLs.size(); i++)
	//	{
	//		mCBFactorB->addItem(nameBLs[i]);
	//	}
	//	assert(nameBLs.size() > 0);
	//	mCBFactorB->setIndexSelected(0);
 // 	}
}

void MotionSynthesisPanel::reconstruction(MyGUI::Widget* sender)
{
	std::size_t index = mCBModel->getIndexSelected();
	if(index != MyGUI::ITEM_NONE)
	{
		ResUtil::MMatrix modata = mModelList[index]->reconstruct(mCBMotion->getIndexSelected());
		
		Eigen::MatrixXd mdata(modata.sizeRow(),modata.sizeCol());
		for(std::size_t i = 0; i < modata.sizeRow(); i++)
			for(std::size_t j = 0; j < modata.sizeCol(); j++)
				mdata(i,j) = modata.get(i,j);

		Motion *mo = new Motion(mdata, "R_" + mCBMotion->getItem(mCBMotion->getIndexSelected()));
		StatePanel::getSingletonPtr()->addMotion(mo);
	}
}
void MotionSynthesisPanel::synTransition(MyGUI::Widget* sender)
{
	std::size_t index = mCBModel->getIndexSelected();
	if(index != MyGUI::ITEM_NONE)
	{
		int interval = MyGUI::utility::parseInt(mEBInterval->getCaption());
		std::vector<std::size_t> contents;
		for(std::size_t i = 0; i < mListsContent->getItemCount(); i++)
		{
 			std::size_t* content = mListsContent->getItemDataAt<std::size_t>(i);
			contents.push_back(*content);
		}
		if(contents.size() == 0) return;
		//ResUtil::MMatrix modata = mModelList[index]->syc(indexA, indexB);
		
		ResUtil::MMatrix modata = mModelList[index]->generate(mCBActor->getIndexSelected(),contents,interval);
		Eigen::MatrixXd mdata(modata.sizeRow(),modata.sizeCol());
		for(std::size_t i = 0; i < modata.sizeRow(); i++)
			for(std::size_t j = 0; j < modata.sizeCol(); j++)
				mdata(i,j) = modata.get(i,j);

		Motion *mo = new Motion(mdata, "ST_");
		StatePanel::getSingletonPtr()->addMotion(mo);
	}
}
void MotionSynthesisPanel::synInterpolation(MyGUI::Widget* sender)
{
	std::size_t index = mCBModel->getIndexSelected();
	if(index != MyGUI::ITEM_NONE)
	{
		std::size_t indexA = mCBFactorA->getIndexSelected();
		std::size_t indexB = mCBFactorB->getIndexSelected();
		std::size_t length = MyGUI::utility::parseInt(mEBMotionLength->getCaption());
		ResUtil::MMatrix modata = mModelList[index]->synthesis(indexA, indexB, length);
		
		Eigen::MatrixXd mdata(modata.sizeRow(),modata.sizeCol());
		for(std::size_t i = 0; i < modata.sizeRow(); i++)
			for(std::size_t j = 0; j < modata.sizeCol(); j++)
				mdata(i,j) = modata.get(i,j);

		Motion *mo = new Motion(mdata, "I_" + mCBFactorA->getItem(mCBFactorA->getIndexSelected()) + mCBFactorB->getItem(mCBFactorB->getIndexSelected()));
		StatePanel::getSingletonPtr()->addMotion(mo);
	}
}


void MotionSynthesisPanel::generateMotion(MyGUI::Widget* sender)
{
	//std::size_t index = mCBModel->getIndexSelected();
	//if(index != MyGUI::ITEM_NONE)
	//{
	//	std::size_t indexA = mCBFactorA->getIndexSelected();
	//	std::size_t indexB = mCBFactorB->getIndexSelected();
	//	
	//	std::vector<size_t> segments =  mModelList[index]->getSegments();

	////	ResUtil::Matrix modata = mModelList[index]->generate(indexA,indexB,500);//reconstruction(3);
 //
	//	Motion *mo = new Motion(modata,mCBModel->getSelectedText() + "_" + mCBFactorA->getItem(indexA) + mCBFactorB->getItem(indexB));
	//	
	//	MotionManager::getSingleton().addMotion(mo);

	//	//TO update the avaliable motion name of  of UI
	//	MotionPanel::getSingleton().upateMotionList();
	//}
}
void MotionSynthesisPanel::visualX(MyGUI::Widget* sender)
{
	std::size_t index = mCBModel->getIndexSelected();
	index = 1;
	if(index != MyGUI::ITEM_NONE)
	{
		
		Ogre::ManualObject *mo = AppDemo::getSingleton().mSceneMgr->createManualObject("X_Trace");
//		std::vector<size_t> segments =  mModelList[index]->getSegments();
#define Num 40
		double delta = 7.05/Num;
		double ann[80];
	/*	std::ifstream fin("scripts/tri.ann",std::ios::binary|std::ios::in);
		int m = Num,n = 2;
		fin.read((char*)&m,sizeof(int));
		fin.read((char*)&n,sizeof(int));
 		fin.read((char*)&ann[0],sizeof(double)*Num*2);*/

		mo->begin("visX", Ogre::RenderOperation::OT_LINE_STRIP);
		for(std::size_t i = 0; i < Num; i++)
		{
			Ogre::Vector3 pos(Ogre::Vector3((1+i*0.051)*sin(-1.57 + delta*i),5,(1+i*0.051)*cos(-1.57 + delta*i)));
			ann[i] = pos.x;
			ann[i+Num] = pos.z;

		//    Ogre::Vector3 pos(ann[i], 5, ann[i+Num]); 
			mo->position(pos);
			mo->colour(1.0f, 0.0f, 0.2f);			
		}
		mo->end();

		std::ofstream fout("scripts/circle.ann",std::ios::binary|std::ios::out);
		int m = Num,n = 2;
		fout.write((char*)&m,sizeof(int));
		fout.write((char*)&n,sizeof(int));
		//for(std::size_t i = 0; i < 60; i++)
		//	std::cout << ann[i] << std::endl;
		fout.write((char*)&ann[0],sizeof(double)*Num*2);

#ifdef VISPRE		
		ResUtil::Matrix X = mModelList[index]->predictX(1,1,segments[1]);
		mo->begin("visX", Ogre::RenderOperation::OT_LINE_STRIP);
		for(std::size_t i = 0; i < X.sizeRow(); i++)
		{
			
			Ogre::Vector3 pos(Ogre::Vector3(X.get(i,0)*50,X.get(i,1)*50 + 5,X.get(i,2)*50));
			
			mo->position(pos);
			std::cout << pos << std::endl;
			mo->colour(0.2 , 0.8, 1 - 0.2);			
		}
		mo->end();

//#ifndef VISPRE
		/*ResUtil::Matrix */
		X = mModelList[index]->getX();
		for(std::size_t t = 1; t < segments.size(); t++)
		{
			mo->begin("visX",Ogre::RenderOperation::OT_LINE_STRIP);
			for(std::size_t i = segments[t-1]; i < segments[t]; i++)
			{
				Ogre::Vector3 pos(Ogre::Vector3(X.get(i,0)*50,X.get(i,1)*50 + 5*t,X.get(i,2)*50));
				mo->position(pos);
			//	mo->position(Ogre::Vector3(sin(3.1415/10.0*(i-segments[t-1])+0.8)*5,sin(3.1415/10.0*(i-segments[t-1]))*4 + 10,sin(3.1415/10.0*(i-segments[t-1])+1.0)*5));
				mo->colour(0.2 * t, 0.8, 1 - 0.2 * t);
			}
			mo->end();
		}
#endif
//#endif
		Ogre::SceneNode * node= AppDemo::getSingleton().mSceneMgr->getRootSceneNode()->createChildSceneNode("Node_Vis_X");
		node->attachObject(mo);
	}
}