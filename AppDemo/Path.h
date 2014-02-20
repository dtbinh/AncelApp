/**
*-----------------------------------------------------------------------------
*Filename:  Path.h
*-----------------------------------------------------------------------------
*File Description: this class be used to manipulate the motion path
*-----------------------------------------------------------------------------
*Author: Ancel         2011/11/21               alwssimple@gmail.com
*-----------------------------------------------------------------------------
*/

#ifndef __PathManipulater_h_
#define __PathManipulater_h_

#include <OgrePrerequisites.h>
#include <OgreMaterial.h>
#include <OgreMesh.h>

#include <OgreMatrix4.h>
#include "Path2DManipulator.h"
#include "MousePicker.h"

#include <Eigen\Eigen>

using namespace Eigen;

namespace AncelApp
{
	class Path
	{
	public:
		Path(Ogre::SceneManager* sceneMgr,
			PickableObject *handle,
            const Ogre::uint numberOfSides = 0, 
            const Ogre::Real radius = 0.0,
            const Ogre::uint sphereRings = 0,
            const Ogre::uint sphereSegments = 0,
            const Ogre::Real sphereRadius = 0.0,
            const Ogre::Real sphereMaxVisibilityDistance = 0.0);
	    ~Path();

		Ogre::ManualObject* createPath( 
						const Ogre::String& name, 
						const Ogre::String& materialName, 
						bool  uniqueMaterial = false,
						bool  isDynamic = false,
						bool  disableUVs = false, 
						bool  disableNormals = false);
  
     	void update_(const std::size_t pointIndex, const Ogre::Vector3& pos);
		void update(const std::vector<Ogre::Vector3>& vPos, const std::vector<int>& vIndex);
 		 	
		bool getUpdateFlag()	const {return mNeedToUpdate;}
		void resetUpdateFlag()		 {mNeedToUpdate = false;}
		void setInit3DPath(const Eigen::MatrixXd& path3D);
		
		std::pair<int,int> getRange(int index);

		const Eigen::MatrixXd& getUpdatedPath() const {return mPath3D;}
 
 
		void setRadius(const Ogre::Real radius)      {mRadius = radius;}
        void setSides(const Ogre::uint numberOfSides){mSideCount = numberOfSides;}
 
        const Ogre::Real getRadius(){return mRadius;}
        const Ogre::uint getSides(){return mSideCount;}
 
        void setSceneNode(Ogre::SceneNode* sceneNode){mSceneNode = sceneNode;}
        Ogre::SceneNode* getSceneNode(){return mSceneNode;}
 
        Ogre::MaterialPtr getMaterial(){return mMaterial;}
		void setVisbility(bool flag);
		bool getVisbility() const;

		void updateControlPoint(const int index);
		bool isHandle(const int index);
   protected:
	
	    void _update(bool disableUVs = false, bool disableNormals = false);
        void _createSphere(const Ogre::String& strName);
        void _destroy();
   
   private:
	 	PickableObject*     mPickEventhandle;

		Ogre::Real			mCollsionDepth;
		Ogre::Vector3		mPickedNodeOffset;
		Ogre::SceneNode*    mPickedNode;
		std::size_t			mPickedIndex;
		Ogre::SceneManager* mSceneMgr;
  
		bool				mNeedToUpdate;
//		std::vector<Ogre::Vector4> mLineVertices;
		
		Ogre::Matrix4 mViewMatrix;
        Ogre::uint mSideCount;
        Ogre::Real mRadius;
        bool mUniqueMaterial;
		bool mVisibility;
        Ogre::uint mSphereRings;
        Ogre::uint mSphereSegments;
        Ogre::Real mSphereRadius;
        Ogre::Real mSphereMaxVisDistance;
 
        Ogre::MaterialPtr mMaterial;
        Ogre::ManualObject* mPathObject;
 
        typedef std::vector<Ogre::Entity*> SphereStorage;
        SphereStorage mSpheresJoints;
		std::vector<Ogre::SceneNode*> mSperesSceneNodes;
        Ogre::MeshPtr mSphereMesh;
 
        Ogre::SceneNode* mSceneNode;
		//----------------------------------------
		
		Path2DManipulator mManipulatorXZ;
		Path2DManipulator mManipulatorLY;

		//----------------------------------------
		std::size_t mNumHandlePoint;
 
		Eigen::MatrixXd mPath3D;
		Eigen::MatrixXd mInitPath3D;
  	};
}
#endif