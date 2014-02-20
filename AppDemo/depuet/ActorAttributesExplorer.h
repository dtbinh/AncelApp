#ifndef __ActorAttributesExplorer_h
#define __ActorAttributesExplorer_h

namespace AncelApp
{
	
	class ActorAttributesExplorer: 
		public Ogre::Singleton<ActorAttributesExplorer>, public Explorer
	{
	public:
		ActorAttributesExplorer();
		~ActorAttributesExplorer();
	private:
	};
}

#endif