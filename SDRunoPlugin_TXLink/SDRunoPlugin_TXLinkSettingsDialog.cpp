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
	m_controller.GetConfigurationKey("TXLink.X", tmp);
	if (tmp.empty())
	{
		return -1;
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
// TODO: Change Template to plugin name
int SDRunoPlugin_TemplateSettingsDialog::LoadY()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.Y", tmp);
	if (tmp.empty())
	{
		return -1;
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


	LoadFreqCal();
	LoadUPfreq();
	LoadTXhold();
	LoadFanHold();
}