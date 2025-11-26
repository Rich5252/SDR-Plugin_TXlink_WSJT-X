#pragma once
namespace nana {
	namespace detail {
		class bedrock;
	}
}
nana::detail::bedrock& get_bedrock_instance();

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>

#include <iunoplugin.h>
#include <unoevent.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <future>
#include <functional>

#include <iunoplugincontroller.h>
#include "SDRunoPlugin_TXLinkForm.h"
#include "UDP_Server.h"



// Forward reference
class SDRunoPlugin_TXLink;

class SDRunoPlugin_TXLinkUi
{

public:
	SDRunoPlugin_TXLinkUi(SDRunoPlugin_TXLink& parent, IUnoPluginController& controller);
	~SDRunoPlugin_TXLinkUi();

	void HandleEvent(const UnoEvent& evt);
	void RequestDllUnload();

	void StartFormThread();			//starts the form thread if not already started
	void RunForm();				// Show and execute the form on its own thread

	void _updateTX_AnnoEntry(std::string tag, std::string tim, long long freq);
	void _updateRX_AnnoEntry(std::string tag, std::string tim, long long freq);

	std::shared_ptr<SDRunoPlugin_TXLinkForm> m_form;

	std::atomic<bool> FormPrepareUnloadRequest = false;		//used to signal form ops for closure
	std::atomic<bool> FormExitAllRequest = false;		//used to signal form ops for closure
	void ShutdownFormThread();
	std::thread m_thread;

	// Member function to get the future.
	std::future<void> get_exit_future() {
		return m_exitPromise.get_future();
	}
	std::promise<void> m_exitPromise;

private:
	
	SDRunoPlugin_TXLink & m_parent;

	std::function<void()> m_cleanup_task;

	bool m_started;

	std::mutex m_lock;
	std::mutex m_lock1;

	IUnoPluginController & m_controller;

	
	std::future<void> m_exitFuture;

	std::string UnoEventTypeToString(UnoEvent::UnoEventType eventType);
};
