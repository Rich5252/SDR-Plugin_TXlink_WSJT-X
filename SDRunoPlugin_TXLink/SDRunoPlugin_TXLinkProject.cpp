#include <iunoplugin.h>
#include <thread>
#include <utility> // For std::move

#include "SDRunoPlugin_TXLink.h"
#include "SDRunoPlugin_TXLinkUi.h"

extern "C"
{
	//DLL entry points called from SDRUno

	UNOPLUGINAPI IUnoPlugin* UNOPLUGINCALL CreatePlugin(IUnoPluginController& controller)
	{
	
		std::cout << get_millis() << "SDRunoPlugin_TXLink: CreatePlugin proceeding to create TXLink instance." << std::endl;
		SDRunoPlugin_TXLink* plugin = new SDRunoPlugin_TXLink(controller);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		std::cout << get_millis() << "SDRunoPlugin_TXLink: CreatePlugin returning plugin (TXLink) instance." << std::endl;
		return plugin;
	}

	UNOPLUGINAPI void UNOPLUGINCALL DestroyPlugin(IUnoPlugin* pPlugin)
	{
		std::cout << std::endl << "------------------------------------------------------------------------------" <<std::endl
					<< get_millis() << "SDRunoPlugin_TXLinkProject: SDRUno DestroyPlugin: proceeding to delete plugin instance." << std::endl;

		delete pPlugin;
	}


	UNOPLUGINAPI unsigned int UNOPLUGINCALL GetPluginApiLevel()
	{
		return UNOPLUGINAPIVERSION;
	}
}

