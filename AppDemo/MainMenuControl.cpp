#include "MainMenuControl.h"
#include "CommandManager.h"

using namespace AncelApp;

template <> MainMenuControl* MyGUI::Singleton<MainMenuControl>::msInstance = nullptr;
template <> const char* MyGUI::Singleton<MainMenuControl>::mClassTypeName = "MainMenuControl";

MainMenuControl::MainMenuControl(MyGUI::Widget* _parent) :
		wraps::BaseLayout("MainMenuControl.layout", _parent),
		mBar(nullptr),
		mPopupMenuWidgets(nullptr)
{
		assignWidget(mBar, "_Main");

		createMainMenu();

}
MainMenuControl::~MainMenuControl()
{

}

void MainMenuControl::createMainMenu()
{
  	mBar->eventMenuCtrlAccept += newDelegate(this, &MainMenuControl::notifyPopupMenuAccept);
}
void MainMenuControl::notifyPopupMenuAccept(MyGUI::MenuControl* _sender, MyGUI::MenuItem* _item)
{
	MyGUI::UString* data = _item->getItemData<MyGUI::UString>(false);
		if (data != nullptr)
			CommandManager::getInstance().setCommandData(*data);

	const std::string& command = _item->getItemId();
	if (command.size() > 8 && command.substr(0, 8) == "Command_")
	{
		CommandManager::getInstance().executeCommand(command);
	}
}
