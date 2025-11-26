// dllmain.cpp : Defines the entry point for the DLL application.
#ifdef _WIN32
	#include <Windows.h>

	#include "Console.h"
	#include "MessageBox.h"

	
	HMODULE _hModule;

	BOOL APIENTRY DllMain(HMODULE hModule,
		DWORD  ul_reason_for_call,
		LPVOID lpReserved
	)
	{
		switch (ul_reason_for_call)
		{
		case DLL_PROCESS_ATTACH:
			_hModule = hModule;

	#ifdef debugConsule
			RedirectIOToConsole();
	#endif
			std::cout << std::endl << "------------------------------------------------------------------"
					<< std::endl << get_millis() << " dllmain:: DLL loaded." << std::endl;
			break;

		case DLL_THREAD_ATTACH:
			SDRthreadCount++;
			std::cout << get_millis() << " dllmain: Thread attach." << SDRthreadCount << std::endl;
			break;
		case DLL_THREAD_DETACH:
			SDRthreadCount--;
			std::cout << get_millis() << " dllmain: Thread detach." << SDRthreadCount << std::endl;
			break;
		case DLL_PROCESS_DETACH:
		
			std::cout << get_millis() << " dllmain: DLL Process Detach .... we are DONE here!" << std::endl;
			//DeallocateMyConsole();			//leave attached to SDR so dll unloads cleanly
			break;
		}
		return TRUE;
	}

	HMODULE GetHModule()
	{
		return _hModule;
	}
#endif