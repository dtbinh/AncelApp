#include "VideoTexture.h"
#include "CommandManager.h"
#include "AppDemo.h"
#include "AppUtility.h"
#include <OgreSceneNode.h>

using namespace AncelApp;

template<> VideoTexture *Ogre::Singleton<VideoTexture>::msSingleton = nullptr;

VideoTexture::VideoTexture() :
	mVideoCapture(nullptr),
	mVideoFrame(nullptr),
	mVideoScreen(nullptr),
	mNumFrames(0),
	countFrame(0),
	wraps::BaseLayout("VideoTexturePanel.layout")
{
	CommandManager::getInstance().registerCommand("Command_LoadVideo", MyGUI::newDelegate(this, &VideoTexture:: loadVideo));
	CommandManager::getInstance().registerCommand("Command_LoadImage", MyGUI::newDelegate(this, &VideoTexture:: loadImage));
	mMainWidget->setVisible(false);
	MyGUI::WindowPtr winptr = static_cast<MyGUI::WindowPtr>(mMainWidget);
	winptr->eventWindowButtonPressed += MyGUI::newDelegate(this, &VideoTexture::windowButtonPressed);
	AppDemo::getSingleton().mRoot->addFrameListener(this);
}

VideoTexture::~VideoTexture()
{
	cvReleaseCapture(&mVideoCapture);
	cvDestroyWindow("img");
	delete mVideoScreen;
	mVideoScreen = nullptr;
	delete mVideoFrame;
	mVideoFrame = nullptr;
}

void VideoTexture::createVideoScene()
{
	//video texture
	mVideoCapture = cvCreateFileCapture("media\\Video\\Record_2013_12_06_15_24_23_896.avi");
	
	createVideoTexture();

	//implememting a video screen
	mVideoScreen = new Ogre::Rectangle2D(true);
	//mVideoScreen->setCorners(-1.0f, 0.75f, 1.0f, -0.75f);
	//mVideoScreen->setCorners(-0.0f, 1.0f, 1.0f, -0.0f);
	mVideoScreen->setCorners(0.0f, 1.0f, 1.0f, 0.0f);
	mVideoScreen->setBoundingBox(Ogre::AxisAlignedBox(-100000.0f * Ogre::Vector3::UNIT_SCALE, 100000.0f * Ogre::Vector3::UNIT_SCALE));
	mVideoScreen->setMaterial(material->getName());
	Ogre::SceneManager *tempSceneMgr = AppDemo::getSingletonPtr()->mSceneMgr;
	Ogre::SceneNode *videoScreenNode = tempSceneMgr->getRootSceneNode()->createChildSceneNode("VideoScreenNode");
	videoScreenNode->attachObject(mVideoScreen);
}

void VideoTexture::loadVideo(const MyGUI::UString& commandName,bool& result)
{
	unsigned long hWnd;
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

	std::string filename = AncelApp::loadFile("avi", HWND(hWnd));
	if(filename != "")
	{
		const char *cstr = filename.c_str();
		//std::string videoName = readVideo(filename);
		mVideoCapture = cvCreateFileCapture(cstr);
		updateVodeoTexture();
	}
}

void VideoTexture::loadImage(const MyGUI::UString& commandName,bool& result)
{
	unsigned long hWnd;
	AppDemo::getSingleton().mRenderWnd->getCustomAttribute("WINDOW", static_cast<void*>(&hWnd));

	std::string filename = AncelApp::loadFile("jpg", HWND(hWnd));
	if(filename != "")
	{
		const char *cstr = filename.c_str();
		mImageFrame = cvLoadImage(cstr);
		cvShowImage("img", mImageFrame);
	}
}


void VideoTexture::windowButtonPressed(MyGUI::Window* _sender, const std::string& _name)
{
	mMainWidget->setVisible(false);
}

void VideoTexture::createVideoTexture()
{
	mVideoFrame = cvQueryFrame(mVideoCapture);
	if(!mVideoFrame)
	{
		return;
	}

	//get the property of video
	mNumFrames = cvGetCaptureProperty(mVideoCapture, CV_CAP_PROP_FRAME_COUNT);

	// Implementing a video texture
	texture = Ogre::TextureManager::getSingleton().createManual(
		"VideoTexture",      // name
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D,   // type
		mVideoFrame->width,  // width
		mVideoFrame->height, // height
		0,                   // number of mipmaps
		Ogre::PF_B8G8R8A8,   // pixel format
		Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE // usage, for textures updated very often
		);
	// Get the pixel buffer
	pixelBuffer = texture->getBuffer();
	// Lock the pixel buffer and get a pixel box
	unsigned char* buffer = static_cast<unsigned char*>(
		pixelBuffer->lock(0, mVideoFrame->width*mVideoFrame->height*4, Ogre::HardwareBuffer::HBL_DISCARD) );
	for(int y = 0; y < mVideoFrame->height; ++y)
	{
		for(int x = 0; x < mVideoFrame->width; ++x)
		{
			buffer[ ((y*mVideoFrame->width)+x)*4 + 0 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 0 ]; // B
			buffer[ ((y*mVideoFrame->width)+x)*4 + 1 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 1 ]; // G
			buffer[ ((y*mVideoFrame->width)+x)*4 + 2 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 2 ]; // R
			buffer[ ((y*mVideoFrame->width)+x)*4 + 3 ] = 255;                                                        // A
		}
	}
	// Unlock the pixel buffer
	pixelBuffer->unlock();

	// Create a materail using the texture
	material = Ogre::MaterialManager::getSingleton().create(
		"DynamicTextureMaterial",//"VideoTextureMaterial", // name
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	material->getTechnique(0)->getPass(0)->createTextureUnitState("VideoTexture");
	material->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
	material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
	material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
}

void VideoTexture::updateVodeoTexture()
{
	mVideoFrame = cvQueryFrame(mVideoCapture);
	if(!mVideoFrame)
	{
		return;
	}
	//opencv show
	//cvShowImage("img", mVideoFrame);
	//mImage = mVideoFrame;

	cv::Mat img(mVideoFrame, 0);
	mImage = img;

	//cv::imshow("Display Window",mImage);
	
	//std::cout<< "numshu: " << mNumFrames<<std::endl;
	//std::cout<< "-----" << countFrame <<std::endl;
	// ÐÐÈË¼ì²â
/*
	if (countFrame % 30 == 0)
	{
		std::vector<cv::Rect> found, found_filtered;
		cv::HOGDescriptor people_dectect_hog;
		people_dectect_hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
		people_dectect_hog.detectMultiScale(mImage, found, 0, cv::Size(8, 8), cv::Size(32, 32), 1.05, 2);

		std::size_t i, j;
		for (i = 0; i < found.size(); i++)
		{
			cv::Rect r = found[i];
			for( j = 0; j < found.size(); j++ )  
				if( j != i && (r & found[j]) == r)  
					break;  
			if( j == found.size() )  
				found_filtered.push_back(r);  
		}

		for (i = 0; i < found_filtered.size(); i++)
		{
			cv::Rect r = found_filtered[i];
			r.x += cvRound(r.width*0.1);
			r.width = cvRound(r.width*0.8);
			r.y += cvRound(r.height*0.07);
			r.height = cvRound(r.height*0.8);
			cv::rectangle(mImage, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);	
		}
	}*/

	// Lock the pixel buffer
	unsigned char* buffer = static_cast<unsigned char*>(pixelBuffer->lock(0, mVideoFrame->width*mVideoFrame->height*4, Ogre::HardwareBuffer::HBL_DISCARD) );
	//const PixelBox &pb = pixelBuffer->getCurrentLock();

	for(int y = 0; y < mVideoFrame->height; ++y)
	{
		for(int x = 0; x < mVideoFrame->width; ++x)
		{
			buffer[ ((y*mVideoFrame->width)+x)*4 + 0 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 0 ]; // B
			buffer[ ((y*mVideoFrame->width)+x)*4 + 1 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 1 ]; // G
			buffer[ ((y*mVideoFrame->width)+x)*4 + 2 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 2 ]; // R
			buffer[ ((y*mVideoFrame->width)+x)*4 + 3 ] = 255;                                                        // A
		}
	}
	// Unlock the pixel buffer
	pixelBuffer->unlock();
}

bool VideoTexture::frameRenderingQueued(const Ogre::FrameEvent &evt)
{
	if(!mVideoFrame)
	{
		return false;
	}
	else
	{
		countFrame++;
		updateVodeoTexture();
	}
	return true;
}

void VideoTexture::createFrameListener(void)
{
	//windowResized(mRenderWnd);
	Ogre::Root* tempRoot = AppDemo::getSingletonPtr()->mRoot;
	tempRoot->addFrameListener(this);
}
