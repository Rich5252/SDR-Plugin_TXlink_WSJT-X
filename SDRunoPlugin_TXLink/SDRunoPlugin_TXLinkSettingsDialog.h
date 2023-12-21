#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/dragger.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>


// TODO: Change these numbers to the height and width of your form
#define dialogFormWidth (297)
#define dialogFormHeight (240)

class SDRunoPlugin_TXLinkUi;

class SDRunoPlugin_TemplateSettingsDialog : public nana::form
{

public:

	SDRunoPlugin_TemplateSettingsDialog(SDRunoPlugin_TXLinkUi& parent, IUnoPluginController& controller);
	~SDRunoPlugin_TemplateSettingsDialog();

	void Run();

private:

	void Setup();
	int LoadX();
	int LoadY();
	int LoadFreqCal();
	int LoadUPfreq();
	int LoadTXhold();
	int LoadFanHold();

	// TODO: Now add your UI controls here
	nana::label freqCalLbl{ *this, nana::rectangle(20, 38, 85, 20) };
	nana::textbox freqCalTb{ *this, nana::rectangle(110, 35, 105, 20) };

	nana::label UPfreqLbl{ *this, nana::rectangle(20, 78, 85, 20) };
	nana::textbox UPfreqTb{ *this, nana::rectangle(110, 75, 105, 20) };

	nana::label TXholdLbl{ *this, nana::rectangle(20, 118, 85, 20) };
	nana::textbox TXholdTb{ *this, nana::rectangle(110, 115, 105, 20) };

	nana::label FanHoldLbl{ *this, nana::rectangle(20, 158, 85, 20) };
	nana::textbox FanHoldTb{ *this, nana::rectangle(110, 155, 105, 20) };

	SDRunoPlugin_TXLinkUi & m_parent;
	IUnoPluginController & m_controller;

	std::mutex m_lock;
};

