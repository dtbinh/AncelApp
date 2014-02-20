#include "ActorAttributesExplorer.h"

using namespace AncelApp;

template<> ActorAttributesExplorer* Ogre::Singleton<ActorAttributesExplorer>::ms_Singleton = 0;

ActorAttributesExplorer::ActorAttributesExplorer()
	:Explorer("media/layouts/ActorAttributes.layout")
{

}

ActorAttributesExplorer::~ActorAttributesExplorer()
{

}