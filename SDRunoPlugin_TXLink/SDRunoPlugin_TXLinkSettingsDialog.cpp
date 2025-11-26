#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "SDRunoPlugin_TXLinkSettingsDialog.h"
#include "SDRunoPlugin_TXLinkUi.h"
#include "resource.h"
#include <io.h>
#include <shlobj.h>

// Form constructor with handles to parent and uno controller - launches form TemplateForm
SDRunoPlugin_TemplateSettingsDialog::SDRunoPlugin_TemplateSettingsDialog(SDRunoPlugin_TXLinkUi& parent, IUnoPluginController& controller) :
	nana::form(nana::API::make_center(dialogFormWidth, dialogFormHeight), nana::appearance(true, false, true, false, false, false, false)),
	m_parent(parent),
	m_controller(controller)
{
	Setup();	
}

// Form deconstructor
SDRunoPlugin_TemplateSettingsDialog::~SDRunoPlugin_TemplateSettingsDialog()
{
	// **This Should not be necessary, but just in case - we are going to remove all event handlers
	// previously assigned to the "destroy" event to avoid memory leaks;
	this->events().destroy.clear();
}

// Start Form and start Nana UI processing
void SDRunoPlugin_TemplateSettingsDialog::Run()
{	
	show();
	nana::exec();
}

int SDRunoPlugin_TemplateSettingsDialog::LoadX()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.Xsettings", tmp);
	if (tmp.empty())
	{
		tmp = "0";
		//?std::lock_guard<std::mutex> l(m_lock); 
		m_controller.SetConfigurationKey("TXLink.Xsettings", tmp);		//default
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
// TODO: Change Template to plugin name
int SDRunoPlugin_TemplateSettingsDialog::LoadY()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.Ysettings", tmp);
	if (tmp.empty())
	{
		tmp = "0";
		//?std::lock_guard<std::mutex> l(m_lock); 
		m_controller.SetConfigurationKey("TXLink.Ysettings", tmp);		//default
	}
	return stoi(tmp);
}

// Load freq cal value from the ini file (if exists)
int SDRunoPlugin_TemplateSettingsDialog::LoadFreqCal()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.freqCal", tmp);
	if (tmp.empty())
	{
		tmp = "-16900";
		//?std::lock_guard<std::mutex> l(m_lock); 
		m_controller.SetConfigurationKey("TXLink.freqCal", tmp);		//default
	}
	freqCalTb.caption(tmp);
	return stoi(tmp);
}

// Load freq cal value from the ini file (if exists)
int SDRunoPlugin_TemplateSettingsDialog::LoadUPfreq()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.UPfreq", tmp);
	if (tmp.empty())
	{
		tmp = "500";
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.UPfreq", tmp);		//default
	}
	UPfreqTb.caption(tmp);
	return stoi(tmp);
}

// Load TXhold time (ms) value from the ini file (if exists)
int SDRunoPlugin_TemplateSettingsDialog::LoadTXhold()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.TXhold", tmp);
	if (tmp.empty())
	{
		tmp = "450";
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.TXhold", tmp);		//default
	}
	TXholdTb.caption(tmp);
	return stoi(tmp);
}

// Load FanHold time (Mins) value from the ini file (if exists)
int SDRunoPlugin_TemplateSettingsDialog::LoadFanHold()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.FanHold", tmp);
	if (tmp.empty())
	{
		tmp = "3";
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.FanHold", tmp);		//default
	}
	FanHoldTb.caption(tmp);
	return stoi(tmp);
}

// Load FanLowPWM value from the ini file (if exists)
int SDRunoPlugin_TemplateSettingsDialog::LoadFanLowPWM()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.FanLowPWM", tmp);
	if (tmp.empty())
	{
		tmp = "175";
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.FanLowPWM", tmp);		//default
	}
	FanLowPWMTb.caption(tmp);
	return stoi(tmp);
}

// Load FT8txDelay value from the ini file (if exists)
int SDRunoPlugin_TemplateSettingsDialog::LoadstrFT8txDelay()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.FT8txDelay", tmp);
	if (tmp.empty())
	{
		tmp = "250";
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.FT8txDelay", tmp);		//default
	}
	FT8txDelayTb.caption(tmp);
	return stoi(tmp);
}

// Load TempLimit value from the ini file (if exists)
int SDRunoPlugin_TemplateSettingsDialog::LoadstrTempLimit()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.TempLimit", tmp);
	if (tmp.empty())
	{
		tmp = "54";
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.TempLimit", tmp);		//default
	}
	TempLimitTb.caption(tmp);
	return stoi(tmp);
}

// Create the settings dialog form
void SDRunoPlugin_TemplateSettingsDialog::Setup()
{
	// TODO: Form code starts here

	// Load X and Y locations for the dialog from the ini file (if exists)
	int posX = LoadX();
	int posY = LoadY();
	move(posX, posY);

	// This code sets the plugin size and title
	size(nana::size(dialogFormWidth, dialogFormHeight));
	caption("TXLink - Settings");
	events().unload([&] { SaveLocation(); });

	// Set the forms back color to black to match SDRuno's settings dialogs
	this->bgcolor(nana::colors::black);

	// TODO: Extra form code goes here
	freqCalLbl.caption("Freq Cal Offset");
	freqCalLbl.bgcolor(nana::colors::black);
	freqCalLbl.fgcolor(nana::colors::white);
	freqCalTb.multi_lines(false);
	freqCalTb.events().text_changed([&] {
		string strCal = freqCalTb.caption();
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.freqCal", strCal);
		});

	UPfreqLbl.caption("TX UP Offset");
	UPfreqLbl.bgcolor(nana::colors::black);
	UPfreqLbl.fgcolor(nana::colors::white);
	UPfreqTb.multi_lines(false);
	UPfreqTb.events().text_changed([&] {
		string strFreq = UPfreqTb.caption();
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.UPfreq", strFreq);
		});

	TXholdLbl.caption("TX HoldOn (ms)");
	TXholdLbl.bgcolor(nana::colors::black);
	TXholdLbl.fgcolor(nana::colors::white);
	TXholdTb.multi_lines(false);
	TXholdTb.events().text_changed([&] {
		string strTXhold = TXholdTb.caption();
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.TXhold", strTXhold);
		});

	FanHoldLbl.caption("Fan Hold Mins");
	FanHoldLbl.bgcolor(nana::colors::black);
	FanHoldLbl.fgcolor(nana::colors::white);
	FanHoldTb.multi_lines(false);
	FanHoldTb.events().text_changed([&] {
		string strFanHold = FanHoldTb.caption();
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.FanHold", strFanHold);
		});

	FanLowPWMLbl.caption("FanLow (0-255)");
	FanLowPWMLbl.bgcolor(nana::colors::black);
	FanLowPWMLbl.fgcolor(nana::colors::white);
	FanLowPWMTb.multi_lines(false);
	FanLowPWMTb.events().text_changed([&] {
		string strFanLowPWM = FanLowPWMTb.caption();
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.FanLowPWM", strFanLowPWM);
		});

	FT8txDelayLbl.caption("FT8 TX delay ms");
	FT8txDelayLbl.bgcolor(nana::colors::black);
	FT8txDelayLbl.fgcolor(nana::colors::white);
	FT8txDelayTb.multi_lines(false);
	FT8txDelayTb.events().text_changed([&] {
		string strFT8txDelay = FT8txDelayTb.caption();
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.FT8txDelay", strFT8txDelay);
		});

	TempLimitLbl.caption("Temp Limit");
	TempLimitLbl.bgcolor(nana::colors::black);
	TempLimitLbl.fgcolor(nana::colors::white);
	TempLimitTb.multi_lines(false);
	TempLimitTb.events().text_changed([&] {
		string strTempLimit = TempLimitTb.caption();
		//?std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.TempLimit", strTempLimit);
		});

	TXtempLbl.caption("Temp =");
	TXtempLbl.bgcolor(nana::colors::black);
	TXtempLbl.fgcolor(nana::colors::white);
	m_timer.interval(std::chrono::milliseconds(100));
	// this next call sets the code to be executed after every interval
	m_timer.elapse([&] {
		string tmp = "Temp = " + m_parent.m_form->strTXtemp + " degC";
		TXtempLbl.caption(tmp);
		});
	m_timer.start();



	LoadFreqCal();
	LoadUPfreq();
	LoadTXhold();
	LoadFanHold();
	LoadFanLowPWM();
	LoadstrFT8txDelay();
	LoadstrTempLimit();
}

void SDRunoPlugin_TemplateSettingsDialog::SaveLocation()
{
	nana::point position = this->pos();
	if (position.x >= 0 && position.y >= 0)
	{
		m_controller.SetConfigurationKey("TXLink.Xsettings", std::to_string(position.x));
		m_controller.SetConfigurationKey("TXLink.Ysettings", std::to_string(position.y));
	}

}