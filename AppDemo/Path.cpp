#include "Path.h"

#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreStringConverter.h>
#include <OgreEntity.h>
#include <OgreMeshManager.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>
#include <OgreSubMesh.h>

#include "AppDemo.h"
#include "MousePicker.h"
#include <MyGUI.h>
using namespace AncelApp;

Path::Path(
		Ogre::SceneManager* sceneMgr, 
		PickableObject *handle,
        const Ogre::uint numberOfSides /*= 0*/, 
        const Ogre::Real radius /*= 0.0*/,
        const Ogre::uint sphereRings /*= 0*/,
        const Ogre::uint sphereSegments /*= 0*/,
        const Ogre::Real sphereRadius /*= 0.0*/,
        const Ogre::Real sphereMaxVisibilityDistance /*= 0.0*/ )
        : mSceneMgr(sceneMgr),
		mPickEventhandle(handle),
        mSideCount(numberOfSides),
        mRadius(radius),
        mPathObject(0),
        mUniqueMaterial(false),
        mSphereRings(sphereRings),
        mSphereSegments(sphereSegments),
        mSphereRadius(sphereRadius),
        mSphereMaxVisDistance(sphereMaxVisibilityDistance),
        mSceneNode(0),
		mNumHandlePoint(0),
		mPickedNode(0),
		mNeedToUpdate(false)
		
{
	 
}

Path::~Path()
{
	 _destroy();
}

//bool  Path::notifyReleased()
//{
//	if(mPickedNode != NULL)
//	{
//		mPickedNode->showBoundingBox(false);
//		mPickedNode = 0;
//		return true;
//	}
//	return false;
//}
//
//bool  Path::notifyMoved(const OIS::MouseEvent &evt, const Ogre::Ray &ray)
//{
//	if(mPickedNode && evt.state.buttonDown(OIS::MB_Left))
//	{
//		Ogre::Vector3 pos = ray.getPoint(mCollsionDepth) + mPickedNodeOffset;
//		this->update_(mPickedIndex, pos);
// 		return true;
//	}
//	return false;
//}
//
//bool  Path::notifyPicked(Ogre::RaySceneQueryResultEntry &entry, const Ogre::Vector3& offset, PickableObject *obj)
//{
//	if(entry.movable->getQueryFlags() != EQM_TUBE_MASK) 
//		return false;
// 
//	mCollsionDepth    =  entry.distance;
//	mPickedNodeOffset =  offset;
//	mPickedNode       =  entry.movable->getParentSceneNode();
// 	mPickedIndex      =  Ogre::any_cast<std::size_t>(Ogre::any_cast<UserAnyPair>(entry.movable->getUserAny()).second);
//	mPickedNode->showBoundingBox(true);
//	
//	return true;
//}
void Path::setInit3DPath(const Eigen::MatrixXd &path3D)
{
	mPath3D = path3D;

	Eigen::MatrixXd path2DXZ(mPath3D.rows(),3);
	Eigen::MatrixXd path2DLY(mPath3D.rows(),3);

 	double L = 0;//sqrt(mPath3D(0,0) * mPath3D(0,0) + mPath3D(0,2) * mPath3D(0,2));
 	
	path2DXZ.block(0,0,mPath3D.rows(),1) = path3D.block(0,0,mPath3D.rows(),1);
	path2DXZ.block(0,1,mPath3D.rows(),1) = path3D.block(0,2,mPath3D.rows(),1);
	path2DXZ.block(0,2,mPath3D.rows(),1) = path3D.block(0,3,mPath3D.rows(),1);

	path2DLY.block(0,1,mPath3D.rows(),1) = path3D.block(0,1,mPath3D.rows(),1);
	path2DLY.block(0,2,mPath3D.rows(),1) = path3D.block(0,3,mPath3D.rows(),1);
	
	for(int i = 0; i < mPath3D.rows(); i++)
	{
	 	if(i == 0)
		{
			path2DLY(0,0) = 0; 
		}
		else
		{
			double deltaX = mPath3D(i,0) - mPath3D(i-1,0); 
			double deltaZ = mPath3D(i,2) - mPath3D(i-1,2); 
			
			L += sqrt(deltaX * deltaX + deltaZ * deltaZ);

			path2DLY(i,0) = L;
		}
 	}
 
	mManipulatorXZ.setInit2DPath(path2DXZ);
	mManipulatorLY.setInit2DPath(path2DLY);
}

void Path::update_(const std::size_t pointIndex, const Ogre::Vector3& pos)
{
	//horizontal
	Eigen::MatrixXd path2DXZ = mManipulatorXZ.update(Ogre::Vector2(pos.x,pos.z), pointIndex);
 	//vertical
	
 	Eigen::MatrixXd path2DLY = mManipulatorLY.update(Ogre::Vector2(mManipulatorLY.getPoint(pointIndex).x, pos.y),pointIndex);
 
 	//transfomrs back to 3D curved manifold
 	for(int i = 0; i < mPath3D.rows(); i++)
	{
		int k = 0;
		while(k < path2DLY.rows() && mManipulatorLY.getInitPoint(k).x < path2DLY(i,0))
			k++;
 		if(k != 0)
		{
			k--;
			if(k + 1 == path2DLY.rows()) 
				k--;
		}
		double lilk = path2DLY(i,0) - mManipulatorLY.getInitPoint(k).x;
		double lkli = mManipulatorLY.getInitPoint(k+1).x - path2DLY(i,0);
		// for more detial see equation 6
		mPath3D(i,0) = (lilk * path2DXZ(k+1,0) + lkli * path2DXZ(k,0)) / (lilk + lkli);
		mPath3D(i,1) = path2DLY(i,1);
		mPath3D(i,2) = (lilk * path2DXZ(k+1,1) + lkli *  path2DXZ(k,1)) / (lilk + lkli);
 	}
	mNeedToUpdate = true;
	_update();
}

void Path::update(const std::vector<Ogre::Vector3>& vPos, const std::vector<int>& vIndex)
{
	std::vector<Ogre::Vector2> vPosXZ;
	std::vector<Ogre::Vector2> vPosLY;

	for(std::size_t i = 0; i < vIndex.size(); i++)
	{
		vPosXZ.push_back(Ogre::Vector2(vPos[i].x,vPos[i].y));
		vPosLY.push_back(Ogre::Vector2(mManipulatorLY.getInitPoint(vIndex[i]).x,vPos[i].y));
	}

	//horizontal
	Eigen::MatrixXd path2DXZ  = mManipulatorXZ.update(vPosXZ, vIndex);
 	//vertical
	Eigen::MatrixXd path2DLY  = mManipulatorLY.update(vPosLY,vIndex);
 
	//transfomrs back to 3D curved manifold

	for(int i = 0; i < mPath3D.rows(); i++)
	{
		int k = 0;
		while(k < path2DLY.rows() && mManipulatorLY.getInitPoint(k).x < path2DLY(i,0))
			k++;
 		if(k != 0)
		{
			k--;
			if(k + 1 == path2DLY.rows()) 
				k--;
		}
		float lilk = path2DLY(i,0) - mManipulatorLY.getInitPoint(k).x;
		float lkli = mManipulatorLY.getInitPoint(k+1).x - path2DLY(i,0);
		// for more detial see equation 6
		mPath3D(i,0)= (lilk *  path2DXZ(k+1,0) + lkli *  path2DXZ(k,0)) / (lilk + lkli);
		mPath3D(i,1) = path2DLY(i,1);
		mPath3D(i,2) = (lilk * path2DXZ(k+1,1) + lkli *  path2DXZ(k,1)) / (lilk + lkli);
 	}
	mNeedToUpdate = true;
	_update();
}
 
void Path::updateControlPoint(const int index)
{
	if(index > 0 && index < mPath3D.rows() - 1)
	{
		mPath3D(index,3) = mPath3D(index,3) > 0 ?  0:1;
		mManipulatorXZ.updateControlPoint(index);
		mManipulatorLY.updateControlPoint(index);
		_update();
	}
}
Ogre::ManualObject* Path::createPath( 
        const Ogre::String& name, 
        const Ogre::String& materialName, 
        bool uniqueMaterial,
        bool isDynamic,
        bool disableUVs, 
        bool disableNormals)
 {
        if (mPathObject)
            return mPathObject;
 
     	std::string sceneNodeName = MyGUI::utility::toString(this) + "_SceneNode";
		mSceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(sceneNodeName);
	 
        mMaterial = Ogre::MaterialManager::getSingleton().getByName(materialName);
 
        mUniqueMaterial = uniqueMaterial;
 
        if (mUniqueMaterial)
            mMaterial = mMaterial->clone(materialName + "_" + name);
 
 
        mPathObject = mSceneMgr->createManualObject(name);
        mPathObject->setDynamic(isDynamic);
		mPathObject->setQueryFlags(AncelApp::EQM_NO_MASK);
	    _update(disableUVs,disableNormals);
		
        mSceneNode->attachObject(mPathObject);

		mVisibility = true;
	    return mPathObject;
}

void Path::_update(bool disableUVs /*= false*/, bool disableNormals /*= false*/)
{
	 if (mPathObject == 0 || mPath3D.rows() < 2)
            return;
 
        if (mPathObject->getDynamic() == true && mPathObject->getNumSections() > 0)
            mPathObject->beginUpdate(0);
        else
            mPathObject->begin(mMaterial->getName());
 
        Ogre::Quaternion qRotation(Ogre::Degree(360.0/(Ogre::Real)mSideCount),Ogre::Vector3::UNIT_Z);
 
        const Ogre::uint iVertCount = mSideCount + 1;
 
        Ogre::Vector3* vCoreVerts = new Ogre::Vector3[iVertCount];
        Ogre::Vector3 vPos = Ogre::Vector3::UNIT_Y * mRadius;
 
 
		for (std::size_t i = 0;i < iVertCount; i++)
        {
            vCoreVerts[i] = vPos;
            vPos = qRotation * vPos;
        }
 
        Ogre::Vector3 vLineVertA, vLineVertB;
        Ogre::Vector3 vLine;
        Ogre::Real dDistance;
        int A,B,C,D;
        int iOffset;
 
        Ogre::Vector3* vCylinderVerts = new Ogre::Vector3[iVertCount * 2];
 
		for (int i = 1; i < mPath3D.rows(); i++)
        {
			vLineVertA = Ogre::Vector3(mPath3D(i-1,0),mPath3D(i-1,1),mPath3D(i-1,2));
            vLineVertB = Ogre::Vector3(mPath3D(i,0), mPath3D(i,1),mPath3D(i,2));
 
            vLine = vLineVertB - vLineVertA;
            dDistance = vLine.normalise();
 
            qRotation = Ogre::Vector3::UNIT_Z.getRotationTo(vLine);
 
			for (std::size_t j = 0; j < iVertCount; j++)
            {
                vCylinderVerts[j] = (qRotation * vCoreVerts[j]);
                vCylinderVerts[j + iVertCount] = (qRotation * (vCoreVerts[j] + (Ogre::Vector3::UNIT_Z * dDistance)));
            }
 
            Ogre::Real u,v,delta;
            delta = 1.0 / (Ogre::Real)(iVertCount - 1);
            u = 0.0;
            v = 1.0;
			
			for (std::size_t j = 0; j < (iVertCount * 2); j++)
            {
                mPathObject->position(vCylinderVerts[j] + vLineVertA);
                if (disableNormals == false)
                {
	 				 Ogre::Vector3 v = vCylinderVerts[j] - vLineVertA;
 					 mPathObject->normal(v.normalisedCopy());
                }
                if (disableUVs == false)
                {
                    if (j == iVertCount){
                        u = 0.0;
                        v = 0.0;
                    }
                    mPathObject->textureCoord(u,v);
                    u += delta;
                }
            }
 
            iOffset = (i-1) * (iVertCount*2);

			for (std::size_t j = 0; j < iVertCount; j++)
            {
                // End A: 0-(Sides-1)
                // End B: Sides-(Sides*2-1)
 
                // Verts:
                /*
 
                A = (j+1)%Sides        C = A + Sides
                B = j                D = B + Sides
 
                */
   				A = ((j+1) % iVertCount);
                B = j;
                C = A + iVertCount;
                D = B + iVertCount;
 
                A += iOffset;
                B += iOffset;
                C += iOffset;
                D += iOffset;
 
                // Tri #1
                // C,B,A
 
                mPathObject->triangle(C,B,A);
 
                // Tri #2
                // C,D,B
 
                mPathObject->triangle(C,D,B);
 
            }
        }
  
		delete[] vCoreVerts;
        delete[] vCylinderVerts;
        vCoreVerts = 0;
        vCylinderVerts = 0;
 
        if (mSphereMesh.isNull() == true)
            _createSphere(mPathObject->getName() + "_SphereMesh");
 
        if (mSceneNode)
		{
			for(std::size_t i = 0; i < mSperesSceneNodes.size();i++)
			{
				mSperesSceneNodes[i]->removeAndDestroyAllChildren();
				mSperesSceneNodes[i]->detachAllObjects();
			}
            //mSceneNode->removeAndDestroyAllChildren();
		}
 
        Ogre::Entity* pEnt = 0;
        Ogre::SceneNode* pChildNode = 0;
        std::size_t cnt_hanle_num = 0;
		for (int i = 0; i < mPath3D.rows(); i++)
        {
            if ((int)mSpheresJoints.size() <= i)
            {
                pEnt = mSceneMgr->createEntity(mPathObject->getName() + "_SphereEnt" + Ogre::StringConverter::toString(i),mSphereMesh->getName());
                pEnt->setMaterialName(mMaterial->getName());
				pEnt->setQueryFlags(AncelApp::EQM_TUBE_MASK);
 				
				pEnt->setUserAny(Ogre::Any(UserAnyPair(Ogre::Any(mPickEventhandle),Ogre::Any(this),Ogre::Any(i))));
                
				mSpheresJoints.push_back(pEnt);
            }
            else
            {
                pEnt = mSpheresJoints[i];
            }
            pEnt->setRenderingDistance(mSphereMaxVisDistance);
 
			if (mSceneNode /*&& mPath3D(i,3)*/)
            {
				cnt_hanle_num ++;
				if(mSperesSceneNodes.size() < cnt_hanle_num)
				{
					pChildNode = mSceneNode->createChildSceneNode();
					mSperesSceneNodes.push_back(pChildNode);
				}
				
				pEnt->setMaterialName("Skeleton/Bone/Pearl");
				pEnt->setQueryFlags(AncelApp::EQM_TUBE_MASK);
				
				Ogre::Vector3 pos(mPath3D(i,0),mPath3D(i,1),mPath3D(i,2));
				mSperesSceneNodes[cnt_hanle_num-1]->setPosition(pos);
				mSperesSceneNodes[cnt_hanle_num-1]->attachObject(pEnt);
				mSperesSceneNodes[cnt_hanle_num-1]->setScale(1.2f, 1.2f, 1.2f); 
			  
				if(mPath3D(i,3) <= 0)
				{
				 	pEnt->setMaterial(mMaterial);
					pEnt->setQueryFlags(AncelApp::EQM_TUBE_MASK); 
					mSperesSceneNodes[cnt_hanle_num-1]->setScale(0.4f, 0.4f, 0.4f); 
				}
             }
        }
        mPathObject->end();
}

void Path::_destroy()
 {
        if (mPathObject)
        {
            if (mPathObject->getParentSceneNode())
                mPathObject->getParentSceneNode()->detachObject(mPathObject);
 
            mSceneMgr->destroyManualObject(mPathObject);
        }
  
        if (mUniqueMaterial)
        {
            Ogre::MaterialManager::getSingleton().remove(mMaterial->getName());
        }
        mMaterial.setNull();
 
        if (mSpheresJoints.size() > 0)
        {
            Ogre::Entity* pEnt = 0;
            SphereStorage::iterator it = mSpheresJoints.begin();
            for (; it != mSpheresJoints.end(); ++it)
            {
                pEnt = (*it);
				Ogre::SceneNode* parentNode = pEnt->getParentSceneNode();
				if(parentNode != NULL)
					parentNode->detachObject(pEnt);
                mSceneMgr->destroyEntity(pEnt);
            }
        }
 
        if (mSphereMesh.isNull() == false)
        {
            Ogre::MeshManager::getSingleton().remove(mSphereMesh->getName());
            mSphereMesh.setNull();
        }
 
        if (mSceneNode)
        {
            mSceneNode->removeAndDestroyAllChildren();
            mSceneNode->getParentSceneNode()->removeAndDestroyChild(mSceneNode->getName());
            mSceneNode = 0;
        }
 
}

void Path::_createSphere( const Ogre::String& strName)
 {
        mSphereMesh = Ogre::MeshManager::getSingleton().createManual(strName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::SubMesh *pSphereVertex = mSphereMesh->createSubMesh();
 
        mSphereMesh->sharedVertexData = new Ogre::VertexData();
        Ogre::VertexData* vertexData = mSphereMesh->sharedVertexData;
 
        // define the vertex format
        Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
        size_t currOffset = 0;
        // positions
        vertexDecl->addElement(0, currOffset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
        currOffset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // normals
        vertexDecl->addElement(0, currOffset, Ogre::VET_FLOAT3,Ogre::VES_NORMAL);
        currOffset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // two dimensional texture coordinates
        vertexDecl->addElement(0, currOffset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0);
        currOffset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
 
        // allocate the vertex buffer
        vertexData->vertexCount = (mSphereRings + 1) * (mSphereSegments+1);
        Ogre::HardwareVertexBufferSharedPtr vBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;
        binding->setBinding(0, vBuf);
        float* pVertex = static_cast<float*>(vBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
 
        // allocate index buffer
        pSphereVertex->indexData->indexCount = 6 * mSphereRings * (mSphereSegments + 1);
        pSphereVertex->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, pSphereVertex->indexData->indexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        Ogre::HardwareIndexBufferSharedPtr iBuf = pSphereVertex->indexData->indexBuffer;
        unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
 
        float fDeltaRingAngle = (Ogre::Math::PI / mSphereRings);
        float fDeltaSegAngle = (2 * Ogre::Math::PI / mSphereSegments);
        unsigned short wVerticeIndex = 0 ;
 
        // Generate the group of rings for the sphere
		for(std::size_t ring = 0; ring <= mSphereRings; ring++ ) {
            float r0 = mSphereRadius * sinf (ring * fDeltaRingAngle);
            float y0 = mSphereRadius * cosf (ring * fDeltaRingAngle);
 
            // Generate the group of segments for the current ring
			for(std::size_t seg = 0; seg <= mSphereSegments; seg++) {
                float x0 = r0 * sinf(seg * fDeltaSegAngle);
                float z0 = r0 * cosf(seg * fDeltaSegAngle);
 
                // Add one vertex to the strip which makes up the sphere
                *pVertex++ = x0;
                *pVertex++ = y0;
                *pVertex++ = z0;
 
                Ogre::Vector3 vNormal = Ogre::Vector3(x0, y0, z0).normalisedCopy();
                *pVertex++ = vNormal.x;
                *pVertex++ = vNormal.y;
                *pVertex++ = vNormal.z;
 
                *pVertex++ = (float) seg / (float) mSphereSegments;
                *pVertex++ = (float) ring / (float) mSphereRings;
 
                if (ring != mSphereRings) {
                    // each vertex (except the last) has six indices pointing to it
                    *pIndices++ = wVerticeIndex + mSphereSegments + 1;
                    *pIndices++ = wVerticeIndex;               
                    *pIndices++ = wVerticeIndex + mSphereSegments;
                    *pIndices++ = wVerticeIndex + mSphereSegments + 1;
                    *pIndices++ = wVerticeIndex + 1;
                    *pIndices++ = wVerticeIndex;
                    wVerticeIndex ++;
                }
            }; // end for seg
        } // end for ring
 
        // Unlock
        vBuf->unlock();
        iBuf->unlock();
        // Generate face list
        pSphereVertex->useSharedVertices = true;
 
        // the original code was missing this line:
        mSphereMesh->_setBounds( 
            Ogre::AxisAlignedBox( 
                Ogre::Vector3(-mSphereRadius, -mSphereRadius, -mSphereRadius), 
                Ogre::Vector3(mSphereRadius, mSphereRadius, mSphereRadius) 
            ), false );
        mSphereMesh->_setBoundingSphereRadius(mSphereRadius);
        // this line makes clear the mesh is loaded (avoids memory leaks)
		
		mSphereMesh->load();
 }
std::pair<int,int> Path::getRange(int index)
{
	std::pair<int,int> ad_range(-1,-1);
	int cnt = index-1;
	while(cnt >= 0)
	{
		if(mPath3D(cnt,3) > 0)
		{
			ad_range.first = cnt;
			break;
		}
		cnt--;
	}
	if(cnt < 0 && ad_range.first < 0)
		ad_range.first = 0;
	cnt = index + 1;
	
	while(cnt < mPath3D.rows())
	{
		if(mPath3D(cnt,3) > 0)
		{
			ad_range.second = cnt;
			break;
		}
		cnt ++;
	}
	if(cnt >= mPath3D.rows() && ad_range.second < 0)
		ad_range.second = mPath3D.rows() - 1;
  	
	return ad_range;
}
void Path::setVisbility(bool flag)
{
	mVisibility = flag;
	mSceneNode->setVisible(flag);
}
bool Path::getVisbility() const
{
	return mVisibility;
}
bool Path::isHandle(const int index)
{
	return mPath3D(index,3) > 0 ? true: false;
}