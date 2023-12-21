#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <iunoplugin.h>
#include <iostream>
#include <iomanip>
#include <sstream>

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
	void FormClosed();
	void FormUnload();

	void ShowUi();

	int LoadX();
	int LoadY();
	void SaveLocation();
	string LoadCOM();
	string LoadTXlevel();
	string LoadPAlevel();
	void SaveCOM(string strCOM);
	void SaveTXlevel(string strTXlevel);
	void SavePAlevel(string strPAlevel);
	void FixFreqButtonClicked();
	void SetStandbyModeClicked();

	bool TrackFreq = true;
	bool StandbyMode = true;

	void _updateTX_AnnoEntry(std::string tag, std::string tim, long long freq);
	void _updateRX_AnnoEntry(std::string tag, std::string tim, long long freq);


private:
	
	SDRunoPlugin_TXLink & m_parent;
	std::thread m_thread;
	std::shared_ptr<SDRunoPlugin_TXLinkForm> m_form;

	bool m_started;

	std::mutex m_lock;

	IUnoPluginController & m_controller;
};
