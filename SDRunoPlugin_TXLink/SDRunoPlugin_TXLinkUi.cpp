#include <sstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <unoevent.h>

#include "SDRunoPlugin_TXLink.h"
#include "SDRunoPlugin_TXLinkUi.h"
#include "SDRunoPlugin_TXLinkForm.h"
#include "UDP_Server.h"


// Ui constructor - load the Ui control into a thread
SDRunoPlugin_TXLinkUi::SDRunoPlugin_TXLinkUi(SDRunoPlugin_TXLink& parent, IUnoPluginController& controller) :
	m_parent(parent),
	m_form(nullptr),
	m_controller(controller)
{
	m_thread = std::thread(&SDRunoPlugin_TXLinkUi::ShowUi, this);
}

// Ui destructor (the nana::API::exit_all();) is required if using Nana UI library
SDRunoPlugin_TXLinkUi::~SDRunoPlugin_TXLinkUi()
{	
	nana::API::exit_all();
	m_thread.join();	
}

// Show and execute the form
void SDRunoPlugin_TXLinkUi::ShowUi()
{	
	m_lock.lock();
	m_form = std::make_shared<SDRunoPlugin_TXLinkForm>(*this, m_controller);
	m_lock.unlock();

	m_form->Run();
	//m_form->StartTimer();
	m_started = true;

	//get defaults for wsjtx UDP_Server and enable that to run
	std::string wsjtxPath;
	m_controller.GetConfigurationKey("TXLink.wsjtxPath", wsjtxPath);
	if (wsjtxPath.empty())
	{
		wsjtxPath = strWSJTXpath;											//coded default
		m_controller.SetConfigurationKey("TXLink.wsjtxPath", wsjtxPath);
	}
	strWSJTXpath = wsjtxPath;
	EnableServer = true;			//allow UDP_Server to startup

}

// Load X from the ini file (if exists)
// TODO: Change Template to plugin name
int SDRunoPlugin_TXLinkUi::LoadX()
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
int SDRunoPlugin_TXLinkUi::LoadY()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.Y", tmp);
	if (tmp.empty())
	{
		return -1;
	}
	return stoi(tmp);
}

void SDRunoPlugin_TXLinkUi::SaveLocation()
{
	nana::point position = m_form->pos();
	if (position.x >= 0 && position.y >= 0)
	{
		m_controller.SetConfigurationKey("TXLink.X", std::to_string(position.x));
		m_controller.SetConfigurationKey("TXLink.Y", std::to_string(position.y));
	}

}

//Load COM port
string SDRunoPlugin_TXLinkUi::LoadCOM()
{
	std::string tmp = "";
	m_controller.GetConfigurationKey("TXLink.COM", tmp);
	if (tmp.empty())
	{
		return "";
	}
	return tmp;
}

void SDRunoPlugin_TXLinkUi::SaveCOM(string strCOM)
{
	m_controller.SetConfigurationKey("TXLink.COM", strCOM);
}

//Load TXlevel
string SDRunoPlugin_TXLinkUi::LoadTXlevel()
{
	std::string tmp = "";
	m_controller.GetConfigurationKey("TXLink.TXlevel", tmp);
	if (tmp.empty())
	{
		return "0";
	}
	return tmp;
}

//Load PAlevel
string SDRunoPlugin_TXLinkUi::LoadPAlevel()
{
	std::string tmp = "";
	m_controller.GetConfigurationKey("TXLink.PAlevel", tmp);
	if (tmp.empty())
	{
		return "0";
	}
	return tmp;
}

void SDRunoPlugin_TXLinkUi::SaveTXlevel(string strTXlevel)
{
	m_controller.SetConfigurationKey("TXLink.TXlevel", strTXlevel);
}

void SDRunoPlugin_TXLinkUi::SavePAlevel(string strPAlevel)
{
	m_controller.SetConfigurationKey("TXLink.PAlevel", strPAlevel);
}

void SDRunoPlugin_TXLinkUi::FixFreqButtonClicked()
{
	if (TrackFreq)
	{
		m_form->SendRXfreq();
	}
}

void SDRunoPlugin_TXLinkUi::SetStandbyModeClicked()
{
		m_form->SetStandbyMode(StandbyMode);
}

void SDRunoPlugin_TXLinkUi::_updateTX_AnnoEntry(std::string tag, std::string tim, long long freq)
{
	m_parent.updateTX_AnnoEntry(tag, tim, freq);
}

void SDRunoPlugin_TXLinkUi::_updateRX_AnnoEntry(std::string tag, std::string tim, long long freq)
{
	m_parent.updateRX_AnnoEntry(tag, tim, freq);
}

// Handle events from SDRuno
// TODO: code what to do when receiving relevant events
void SDRunoPlugin_TXLinkUi::HandleEvent(const UnoEvent& ev)
{
	double freq0 = 0;
	switch (ev.GetType())
	{
	case UnoEvent::StreamingStarted:
		break;

	case UnoEvent::StreamingStopped:
		break;

	case UnoEvent::SavingWorkspace:
		SaveLocation();
		SaveCOM(m_form->GetCOM());
		SaveTXlevel(m_form->GetTXlevel());
		SavePAlevel(m_form->GetPAlevel());
		break;

	case UnoEvent::FrequencyChanged:
		m_lock.lock();						//avoid iterative changes caused by RX/TX swaps etc (caused exceptions at startup)
		freq0 = m_controller.GetVfoFrequency(0);
		if (freq0 != 0 && freq0 != m_form->CurrentRXfreq)
		{
			m_form->CurrentRXfreq = freq0;
			_updateRX_AnnoEntry("RX", "", (int64_t)m_form->CurrentRXfreq);		// needed here in case of split TX/RX when new TX not sent
		}
		if (TrackFreq)
		{
			m_form->SendRXfreq();			// this function reads and sets freq from VRX0
											// and its send TX
		}
		else
		{
			//split TX freq so update TX with VRX1 if changed
			double freq = m_controller.GetVfoFrequency(1);
			if (freq != m_form->CurrentTXfreq && freq > 0)			// freq is 0 if vrx1 not installed
			{
				m_form->CurrentTXfreq = freq;						//TX is set by vrx1
				m_form->SendTXfreq();						// only update TX frq in this case
			}
		}
		m_lock.unlock();
		break;

	case UnoEvent::ClosingDown:
		SaveLocation();
		SaveCOM(m_form->GetCOM());
		SaveTXlevel(m_form->GetTXlevel());
		SavePAlevel(m_form->GetPAlevel());
		m_form->SetStandbyMode(true);				//shutdown TX if app closed
		FormClosed();
		break;

	default:
		break;
	}
}

void SDRunoPlugin_TXLinkUi::FormUnload()
{
	SaveLocation();
	SaveCOM(m_form->GetCOM());
	SaveTXlevel(m_form->GetTXlevel());
	SavePAlevel(m_form->GetPAlevel());
	m_form->SetStandbyMode(true);				//shutdown TX if app closed
	m_form->StopTimer();
	m_form->CloseCOMPort();
}


// Required to make sure the
//  plugin is correctly unloaded when closed
void SDRunoPlugin_TXLinkUi::FormClosed()
{
	m_form->StopTimer();
	m_form->CloseCOMPort();
	m_controller.RequestUnload(&m_parent);
}
