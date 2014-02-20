#ifndef __VIDEO_TEXTURE_H__
#define __VIDEO_TEXTURE_H__

#include <OgreSingleton.h>
#include <PanelView/BasePanelViewItem.h>
#include <BaseLayout/BaseLayout.h>


#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreWindowEventUtilities.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreSceneNode.h>

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

//HOG
#include <core/core_c.h>
#include <gl/glut.h>
#include <imgproc/imgproc.hpp>
#include <objdetect/objdetect.hpp>
#include <ml.h>
#include <WinBase.h>


namespace AncelApp
{
	class VideoTexture : public Ogre::Singleton<VideoTexture>, public Ogre::FrameListener, public wraps::BaseLayout
	{
		public:
			VideoTexture();
			~VideoTexture();
			void loadVideo(const MyGUI::UString& commandName,bool& result);
			void loadImage(const MyGUI::UString& commandName,bool& result);
			void windowButtonPressed(MyGUI::Window* _sender, const std::string& _name);

			void createVideoTexture();
			void updateVodeoTexture();
			bool frameRenderingQueued(const Ogre::FrameEvent &evt);
			void createFrameListener(void);

			void createVideoScene();
			Ogre::SceneNode*	getRootNode()	const {return mActorNode;}	
			bool attachSkeletonToScene(Ogre::SceneManager	*sceneMgr);

		protected:
			Ogre::SceneManager		*mBelongTo;
			Ogre::SceneNode		*mActorNode;
			//** Added for test video texture
			Ogre::Rectangle2D* mVideoScreen;
			Ogre::MaterialPtr material;
			Ogre::TexturePtr texture;
			Ogre::HardwarePixelBufferSharedPtr pixelBuffer;

			IplImage*  mVideoFrame;
			CvCapture* mVideoCapture;

			IplImage*  mImageFrame;

			int mNumFrames;		//总帧数
			int countFrame;		//计数到达第几帧

			//HOG
			cv::Mat mImage;
	};
}

#endif