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
	void SaveLocation();
	int LoadX();
	int LoadY();
	int LoadFreqCal();
	int LoadUPfreq();
	int LoadTXhold();
	int LoadFanHold();
	int LoadFanLowPWM();
	int LoadstrFT8txDelay();
	int LoadstrTempLimit();

	// TODO: Now add your UI controls here
	nana::label freqCalLbl{ *this, nana::rectangle(20, 13, 85, 20) };
	nana::textbox freqCalTb{ *this, nana::rectangle(110, 10, 105, 20) };

	nana::label UPfreqLbl{ *this, nana::rectangle(20, 43, 85, 20) };
	nana::textbox UPfreqTb{ *this, nana::rectangle(110, 40, 105, 20) };

	nana::label TXholdLbl{ *this, nana::rectangle(20, 73, 85, 20) };
	nana::textbox TXholdTb{ *this, nana::rectangle(110, 70, 105, 20) };

	nana::label FanHoldLbl{ *this, nana::rectangle(20, 103, 85, 20) };
	nana::textbox FanHoldTb{ *this, nana::rectangle(110, 100, 105, 20) };

	nana::label FanLowPWMLbl{ *this, nana::rectangle(20, 133, 85, 20) };
	nana::textbox FanLowPWMTb{ *this, nana::rectangle(110, 130, 105, 20) };

	nana::label FT8txDelayLbl{ *this, nana::rectangle(20, 163, 85, 20) };
	nana::textbox FT8txDelayTb{ *this, nana::rectangle(110, 160, 105, 20) };

	nana::label TempLimitLbl{ *this, nana::rectangle(20, 193, 85, 20) };
	nana::textbox TempLimitTb{ *this, nana::rectangle(110, 190, 105, 20) };

	nana::label TXtempLbl{ *this, nana::rectangle(20, 213, 200, 20) };

	nana::timer m_timer;

	SDRunoPlugin_TXLinkUi & m_parent;
	IUnoPluginController & m_controller;

	std::mutex m_lock;
};

