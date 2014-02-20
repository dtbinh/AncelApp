#ifndef SkeletonView_h_
#define SkeletonView_h_

#include "RenderBox.h"
#include "RenderBoxScene.h"
#include <OgreSingleton.h>
 

namespace AncelApp
{
	class SkeletonView: public Ogre::Singleton<SkeletonView>
	{
	public:
		SkeletonView();
		void createScene();
		void updateViewport();
		virtual void setCanvas(MyGUI::Canvas* _value);
	protected:
		virtual void eventPreTextureChanges(MyGUI::Canvas* _canvas);
		virtual void requestUpdateCanvas(MyGUI::Canvas* _canvas, MyGUI::Canvas::Event _event);

		void setTrackTarget(Ogre::SceneNode* node);
		void notifyMouseDrag(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
 		void notifyMouseButtonPressed(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
  		void notifyMouseButtonReleased(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
 
		void setViewport(Ogre::Camera* _value);
		void removeViewport();
		 
		void removeTexture();
		virtual void destroy();
 	private:
	  
		MyGUI::Canvas* mCanvas;
		
		Ogre::Camera*  mCamera;
		Ogre::RenderTarget* mRenderTarget;
		Ogre::ColourValue mColour;
 
		Ogre::SceneNode*    mNode;
		Ogre::SceneNode*    mCameraNode;
		Ogre::Entity*		mEntity;
	};
}

#endif