#include "MyGUI.h"
#include "MyGUI_OgrePlatform.h"


MyGUI::Gui* mGUI;
MyGUI::OgrePlatform* mPlatform = new MyGUI::OgrePlatform();
mPlatform->initialise(mWindow,mSceneManager);
mGUI = new MyGUI::Gui();
mGUI->initialise();

class CLASS_NAME: public OIS::MouseListener,public OIS::KeyListener

bool CLASS_NAME::