/*
#include "ExampleApplication.h"


class TutorialFrameListener:public ExampleFrameListener,public OIS::MouseListener,public OIS::KeyListener
{
public:
	TutorialFrameListener(RenderWindow* win,Camera* cam,SceneManager* sceneMgr):ExampleFrameListener(win,cam,true,true)
	{
		mCamNode = cam->getParentSceneNode();
		mSceneMgr = sceneMgr;
		mRotate = 0.13;
		mMove = 250;
		mContinue = true;
		mMouse->setEventCallback(this);
		mKeyboard->setEventCallback(this);
		mDirection = Vector3::ZERO;
	}
	bool frameStarted(const FrameEvent &evt)
	{
		mCamNode->translate(mDirection*evt.timeSinceLastFrame,Node::TS_LOCAL);
		if(mMouse)
			mMouse->capture();
		if(mKeyboard)
			mKeyboard->capture();
		return mContinue;
	}
	bool mouseMoved(const OIS::MouseEvent &e)
	{	
		if (e.state.buttonDown(OIS::MB_Right))
		{
			mCamNode->yaw(Degree(-mRotate * e.state.X.rel), Node::TS_WORLD);
			mCamNode->pitch(Degree(-mRotate * e.state.Y.rel), Node::TS_LOCAL);
		}
		return true;
	}
	bool mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id )
	{
		Light *light = mSceneMgr->getLight("Light1");
		switch (id)
		{
		case OIS::MB_Left:
			light->setVisible(! light->isVisible());
			break;
		}
		return true;
	}
	bool mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id )
	{
		return true;
	}	
	bool keyPressed(const OIS::KeyEvent &e)
	{
		switch(e.key)
		{
		case OIS::KC_ESCAPE:
			mContinue = false;
			break;

		case OIS::KC_1:
			mCamera->getParentSceneNode()->detachObject(mCamera);
			mCamNode = mSceneMgr->getSceneNode("CamNode2");
			mCamNode->attachObject(mCamera);
			break;

		case OIS::KC_2:
			mCamera->getParentSceneNode()->detachObject(mCamera);
			mCamNode = mSceneMgr->getSceneNode("CamNode1");
			mCamNode->attachObject(mCamera);
			break;

		case OIS::KC_UP:
		case OIS::KC_W:
			mDirection.z -= mMove;
			break;

		case OIS::KC_DOWN:
		case OIS::KC_S:
			mDirection.z += mMove;
			break;

		case OIS::KC_LEFT:
		case OIS::KC_A:
			mDirection.x -= mMove;
			break;

		case OIS::KC_RIGHT:
		case OIS::KC_D:
			mDirection.x += mMove;
			break;

		case OIS::KC_PGDOWN:
		case OIS::KC_E:
			mDirection.y -= mMove;
			break;

		case OIS::KC_PGUP:
		case OIS::KC_Q:
			mDirection.y += mMove;
			break;
		}
		return true;
	}
	bool keyReleased( const OIS::KeyEvent &e)
	{
		switch (e.key)
		{
		case OIS::KC_UP:
		case OIS::KC_W:
			mDirection.z += mMove;
			break;

		case OIS::KC_DOWN:
		case OIS::KC_S:
			mDirection.z -= mMove;
			break;

		case OIS::KC_LEFT:
		case OIS::KC_A:
			mDirection.x += mMove;
			break;

		case OIS::KC_RIGHT:
		case OIS::KC_D:
			mDirection.x -= mMove;
			break;

		case OIS::KC_PGDOWN:
		case OIS::KC_E:
			mDirection.y += mMove;
			break;

		case OIS::KC_PGUP:
		case OIS::KC_Q:
			mDirection.y -= mMove;
			break;
		} // switch
		return true;
	}
protected:
	Real mRotate;
	Real mMove;
	SceneManager *mSceneMgr;
	SceneNode *mCamNode;
	bool mContinue;
	Vector3 mDirection;
};

class TutorialApplication: public ExampleApplication
{
public:
	TutorialApplication()
	{

	}
	~TutorialApplication()
	{

	}
protected:
	void createCamera()
	{
		mCamera = mSceneMgr->createCamera("PlayerCam");
		mCamera->setNearClipDistance(5);
	}
	void createScene()
	{
		mSceneMgr->setAmbientLight(ColourValue(0.25,0.25,0.25));
		Entity *ent = mSceneMgr->createEntity("Ninja","ninja.mesh");
		SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("NinjaNode");
		node->attachObject(ent);

		Light *light = mSceneMgr->createLight("Light1");
		light->setType(Light::LT_POINT);
		light->setPosition(Vector3(250,150,250));
		light->setDiffuseColour(ColourValue::White);
		light->setSpecularColour(ColourValue::White);

		// Create the scene node
		node = mSceneMgr->getRootSceneNode()->createChildSceneNode("CamNode1", Vector3(-400, 200, 400));
		node->yaw(Degree(-45));
		node = node->createChildSceneNode("PitchNode1");
		node->attachObject(mCamera);

		// create the second camera node
		node = mSceneMgr->getRootSceneNode()->createChildSceneNode("CamNode2", Vector3(0, 200, 400));
		node = node->createChildSceneNode("PitchNode2");
	}
	void createFrameListener()
	{
		mFrameListener = new TutorialFrameListener(mWindow,mCamera,mSceneMgr);
		mRoot->addFrameListener(mFrameListener);
		mFrameListener->showDebugOverlay(true);
	}
};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
	// Create application object
	TutorialApplication app; 

	try {
		app.go();
	} catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 
		MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
		fprintf(stderr, "An exception has occured: %s\n",
			e.getFullDescription().c_str());
#endif
	}

	return 0;
}*/



