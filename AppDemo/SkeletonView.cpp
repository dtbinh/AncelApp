#include "SkeletonView.h"
#include <MyGUI.h>
#include "AppDemo.h"

using namespace AncelApp;

template<> SkeletonView* Ogre::Singleton<SkeletonView>::msSingleton = 0;
SkeletonView::SkeletonView()
	:mCanvas(nullptr),
	mRenderTarget(nullptr),
	mColour(Ogre::ColourValue::ZERO)
{

}
void SkeletonView::createScene()
{
	const MyGUI::IntSize& size = MyGUI::RenderManager::getInstance().getViewSize();
	MyGUI::Window* window = MyGUI::Gui::getInstance().createWidget<MyGUI::Window>("PanelSkin", MyGUI::IntCoord(0, size.height - 276, 360, 276), MyGUI::Align::Left|MyGUI::Align::Bottom, "Main","SkeletonViewWnd");
	 
	window->setCaption("Skeleton view");
 	MyGUI::Canvas* canvas = window->createWidget<MyGUI::Canvas>("Canvas", MyGUI::IntCoord(5, 5, window->getClientCoord().width-10, window->getClientCoord().height-10), MyGUI::Align::Stretch);
	canvas->setPointer("hand");
 	 
	setCanvas(canvas);
	 	 
	/*Ogre::Vector3 dir(-1, -1, 0.5);
	dir.normalise();
	Ogre::Light* light = mSceneMgr->createLight(MyGUI::utility::toString(this, "_LightRenderBox"));
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(dir);*/

	std::string camera(MyGUI::utility::toString(this, "_CameraRenderBox"));
	mCamera = AppDemo::getSingleton().mSceneMgr->createCamera(camera);
	mCamera->setNearClipDistance(1);
	mCamera->setPosition(0,10,50);
	mCamera->lookAt(0,0,0);
  	mCamera->setAspectRatio( float(mCanvas->getWidth()) / float(mCanvas->getHeight()) );

	setViewport(mCamera);
}

void SkeletonView::updateViewport()
{
	// §á§â§Ú §ß§å§Ý§Ö §Ó§í§Ý§Ö§ä§Ñ§Ö§ä
	if ((mCanvas->getWidth() <= 1) || (mCanvas->getHeight() <= 1))
		return;
}

void SkeletonView::setCanvas(MyGUI::Canvas* _value)
{
	destroy();

	mCanvas = _value;
	mCanvas->createTexture(MyGUI::Canvas::TRM_PT_VIEW_ALL, MyGUI::TextureUsage::RenderTarget);
	mCanvas->eventPreTextureChanges += MyGUI::newDelegate(this, &SkeletonView::eventPreTextureChanges);
	mCanvas->requestUpdateCanvas = MyGUI::newDelegate(this, &SkeletonView::requestUpdateCanvas);

	mCanvas->updateTexture();

	mCanvas->eventMouseDrag += newDelegate(this, &SkeletonView::notifyMouseDrag);
	mCanvas->eventMouseButtonPressed += newDelegate(this, &SkeletonView::notifyMouseButtonPressed);
	mCanvas->eventMouseButtonReleased += newDelegate(this, &SkeletonView::notifyMouseButtonReleased);

}
void SkeletonView::eventPreTextureChanges(MyGUI::Canvas* _canvas)
{
	removeTexture();
}


void SkeletonView::destroy()
{
 	if (mCanvas)
	{
  		mCanvas->eventMouseDrag -= newDelegate(this, &SkeletonView::notifyMouseDrag);
		mCanvas->eventMouseButtonPressed -= newDelegate(this, &SkeletonView::notifyMouseButtonPressed);
		mCanvas->eventMouseButtonReleased -= newDelegate(this, &SkeletonView::notifyMouseButtonReleased);
		mCanvas->eventPreTextureChanges -= MyGUI::newDelegate(this, &SkeletonView::eventPreTextureChanges);
		mCanvas->requestUpdateCanvas = nullptr;
		mCanvas->destroyTexture();
		mCanvas = nullptr;
 	}
}

void SkeletonView::requestUpdateCanvas(MyGUI::Canvas* _canvas, MyGUI::Canvas::Event _event)
{
	if (!mCamera)
		return;

	if (!(_event.textureChanged || _event.requested))
		return;

	Ogre::TexturePtr texture = static_cast<MyGUI::OgreTexture*>(mCanvas->getTexture())->getOgreTexture();
	Ogre::RenderTexture* target = texture->getBuffer()->getRenderTarget();

	if (mRenderTarget != target
		&& target != nullptr
		&& mCamera != nullptr )
	{
		mRenderTarget = target;

		mRenderTarget->removeAllViewports();
		Ogre::Viewport* viewport = mRenderTarget->addViewport(mCamera);
		viewport->setBackgroundColour(mColour);
		viewport->setClearEveryFrame(true);
		viewport->setOverlaysEnabled(false);
 	}
}

void SkeletonView::notifyMouseDrag(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
{
	static int left = _left;
	static int right = _top;
	int rel_x = _left - left;
	int rel_y = _top -  right;
	if(abs(rel_x) < 50 && abs(rel_y))
	{
		if(_id == MyGUI::MouseButton::Left)
		{
			
		}
 	}

	left = _left;
	right = _top;
	 
	 
}

void SkeletonView::notifyMouseButtonPressed(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
{
	std::cout << 4 << std::endl;
	/*if (mMouseRotation)
	{
		if (_id == MyGUI::MouseButton::Left)
		{
			const MyGUI::IntPoint& point = MyGUI::InputManager::getInstance().getLastPressedPosition(MyGUI::MouseButton::Left);
			mLastPointerX = point.left;
			mMousePressed = true;
		}
		if (_id == MyGUI::MouseButton::Right)
		{
			const MyGUI::IntPoint& point = MyGUI::InputManager::getInstance().getLastPressedPosition(MyGUI::MouseButton::Right);
			mLastPointerX = point.left;
			mMousePressed = true;
		}
	}*/
}

void SkeletonView::notifyMouseButtonReleased(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
{
	std::cout << 3 << std::endl;
	//mMousePressed = false;
}
void SkeletonView::setViewport(Ogre::Camera* _value)
{
	removeViewport();
	mCamera = _value;

	if (mCanvas)
		mCanvas->updateTexture();
}
void SkeletonView::removeViewport()
{
	if (mCamera)
	{
		removeTexture();
		mCamera = nullptr;
	}
}
void SkeletonView::setTrackTarget(Ogre::SceneNode* node)
{
 	mNode = node;
	mNode->attachObject(mCamera);
	mCamera->lookAt(node->getPosition());
	
}
void SkeletonView::removeTexture()
{
	if (mRenderTarget != nullptr)
	{
		mRenderTarget->removeAllViewports();
		mRenderTarget = nullptr;
		if (mCanvas)
			Ogre::Root::getSingleton().getRenderSystem()->destroyRenderTexture(mCanvas->getTexture()->getName());
	}
}