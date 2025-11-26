#include <sstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/detail/bedrock.hpp>
#include <unoevent.h>

#include "SDRunoPlugin_TXLink.h"
#include "SDRunoPlugin_TXLinkUi.h"
#include "SDRunoPlugin_TXLinkForm.h"
#include "UDP_Server.h"
#include "RTTY_Server.h"
//#include "nana_cleanup.h"




// Ui constructor - load the Ui control into a thread
SDRunoPlugin_TXLinkUi::SDRunoPlugin_TXLinkUi(SDRunoPlugin_TXLink& parent, IUnoPluginController& controller) :
	m_parent(parent),
	m_form(nullptr),
	m_started(false),
	m_controller(controller)
{
	//std::string strCOM = "COM5";
	//bool ret = m_controller.GetConfigurationKey("TXLink.COM", strCOM);

	//m_thread = std::thread(&SDRunoPlugin_TXLinkUi::RunForm, this);
	std::cout << get_millis() << " SDRunoPlugin_TXLinkUi Constructor : m_Ui body" << std::endl;
}

// Ui destructor (the nana::API::exit_all();) is required if using Nana UI library
SDRunoPlugin_TXLinkUi::~SDRunoPlugin_TXLinkUi()
{
	std::cout << get_millis() << " SDRunoPlugin_TXLinkUi Destructor called, proceed to ShutdownFormThread" << std::endl;
	ShutdownFormThread();
}

void SDRunoPlugin_TXLinkUi::StartFormThread()
{
	if (!m_started) {
		// Now it's safe to launch the thread, as the host has explicitly
		// told the plugin to start via the StartRequest event.
		m_thread = std::thread(&SDRunoPlugin_TXLinkUi::RunForm, this);
		m_started = true;
	}
}


void SDRunoPlugin_TXLinkUi::ShutdownFormThread()
{	
	std::cout << get_millis() << " SDRunoPlugin_TXLinkUi destructor called - nana exit_all Request" << std::endl;

	//Cant be sure what thread called this and need to ensure nana::API::exit_all() is called from the form thread 
	FormExitAllRequest = true;		//set flag to indicate form exit_all requested, executed in the form thread by nana.timer elapse event

	int sleeps = 0;
	int sleepTime = 100;
	while (FormExitAllRequest && sleeps < 50) //FormExitAllRequest&& 
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));         //wait for request to be cleared by nana form timer event
		sleeps++;
	}
	std::cout << get_millis() << " SDRunoPlugin_TXLinkUi Form ExitAll complete, joining form thread. FormExitAllRequest=" << std::boolalpha << !FormExitAllRequest << ", after " << sleeps * sleepTime << "ms" << std::endl;

#ifdef xxx
	std::string tmp;
	std::cout << get_millis() << " Press Enter to continue..." << std::endl;
	std::cin >> tmp; // Wait for user input to see the message before exiting
	std::cout << get_millis() << " SDRunoPlugin_TXLinkUi destructor: Form thread join requested" << std::endl;
#endif

	// The call to .wait() will block until the promise is set on the Form thread.
	//m_exitFuture.wait();
	if (m_thread.joinable())
	{
		m_thread.join();			//ensure nana::exit_all() has completed before continuing
		std::cout << get_millis() << " SDRunoPlugin_TXLinkUi Form thread joined and returned" << std::endl;
	}
	else
	{
		std::cout << get_millis() << " SDRunoPlugin_TXLinkUi Form thread not joinable" << std::endl;
	}
}

// Show and execute the form on its own thread
void SDRunoPlugin_TXLinkUi::RunForm()
{
	std::cout << get_millis() << " SDRunoPlugin_TXLinkUi::RunForm thread started" << std::endl;
	//get defaults for wsjtx UDP_Server and enable that to run
	std::string wsjtxPath;
	m_controller.GetConfigurationKey("TXLink.wsjtxPath", wsjtxPath);
	if (wsjtxPath.empty())
	{
		wsjtxPath = strWSJTXpath;											//coded default
		m_controller.SetConfigurationKey("TXLink.wsjtxPath", wsjtxPath);
	}
	strWSJTXpath = wsjtxPath;
	
	// This is the thread-safe way to update the shared m_form pointer
	std::shared_ptr<SDRunoPlugin_TXLinkForm> newForm;
	{
		std::lock_guard<std::mutex> lock(m_lock);
		newForm = std::make_shared<SDRunoPlugin_TXLinkForm>(*this, m_controller);
		m_form = newForm; // Safely update the shared pointer
		m_started = true;
	}

	// Run the form outside the lock, as this is a potentially long-running operation
	// and doesn't directly touch other shared data
	if (newForm) {
		newForm->Run();					// start the form and run the nana UI, only returns when the form is closed
	}
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
	if (ev.GetType() != UnoEvent::SavingWorkspace && ev.GetType() != UnoEvent::FrequencyChanged && ev.GetType() != UnoEvent::ClosingDown) { return; } //ignore other events
	std::cout << get_millis() << "SDRunoPlugin_TXLinkUi::HandleEvent called " << UnoEventTypeToString(ev.GetType()) << std::endl;
	
	//std::lock_guard<std::mutex> lock(m_lock);

	double freq0 = 0;
	double freq1 = 0;
	bool bF0changed = false;
	bool bF1changed = false;
	static bool bBusy = 0;
	static double lastF0 = 0;
	static double lastF1 = 0;

	switch (ev.GetType())
	{
	case UnoEvent::SavingWorkspace:
		m_form->SendSerialMsg("$SaveIniData$");			//special Msg to execute SaveIniData()
		break;

	case UnoEvent::FrequencyChanged:
		//m_lock.lock();						//avoid iterative changes caused by RX/TX swaps etc (caused exceptions at startup)

		if (bBusy) { std::cout << "FrequencyChanged re-entry while processing previous event **************************"; }
		//while (bBusy) {};
		bBusy = true;

		freq0 = m_controller.GetVfoFrequency(0);
		bF0changed = false;
		if ((uint32_t)freq0 != (uint32_t)lastF0) { bF0changed = true; lastF0 = freq0; }

		freq1 = m_controller.GetVfoFrequency(1);
		bF1changed = false;
		if ((uint32_t)freq1 != (uint32_t)lastF1) { bF1changed = true; lastF1 = freq1; }

		std::cout << "F Event " << (uint32_t)freq0 << " " << (uint32_t)freq1 << " "  << m_form->SplitMode << std::endl;

		if (!bF0changed && !bF1changed)
		{
			std::cout << "F Dupe" << (uint32_t)freq0 << " " << (uint32_t)freq1 << std::endl;
			//ignore it
			bBusy = false;
			break;
		}

		if (freq0 != 0 && freq0 != m_form->CurrentRXfreq && m_form->SplitMode != m_form->SplitFT8)
		{
			m_form->CurrentRXfreq = freq0;
			_updateRX_AnnoEntry("RX", "", (int64_t)m_form->CurrentRXfreq);		// needed here in case of split TX/RX when new TX not sent
		}
		if (m_form->SplitMode == m_form->SplitNone && bF1changed) //F1 should = F0 so force it back
		{
			std::cout << "Revert vfo1 " << (int64_t)freq0 << std::endl;
			m_controller.SetVfoFrequency(1, freq0);						//because thread locked this will not occur until scope exit
		}
		else if (m_form->SplitMode == m_form->SplitNone && bF0changed)
		{
			m_form->SendSerialMsg("$SendRXfreq$");			// special Msg to execute SendRXfrq()
											// and it sends TX
		}
		else if (m_form->SplitMode == m_form->SplitFT8)
		{
			//in SplitFT8 the tx freq base is fixed so force VRX0 back to it (this forces a change back to SplitNone by user
			// before tx freq can be changed and hence avoids trying to TX on wrong freq)
			if (freq0 != m_form->CurrentTXfreq)
			{
				m_controller.SetVfoFrequency(0, m_form->CurrentTXfreq);
			}
		}
		else
		{
			//split TX freq so update TX with VRX1 if changed (unless in FT8 mode when no change)
			//double freq = m_controller.GetVfoFrequency(1);
			if (freq1 != m_form->CurrentTXfreq && freq1 > 0 && m_form->SplitMode != m_form->SplitFT8)			// freq is 0 if vrx1 not installed
			{
				m_form->CurrentTXfreq = freq1;						//TX is set by vrx1
				m_form->SendSerialMsg("$SendTXfreq$");			// special Msg to execute SendTXfrq(). only update TX frq in this case
			}
		}
		bBusy = false;

		//m_lock.unlock();
		break;

	case UnoEvent::ClosingDown:
		std::cout << get_millis() << "SDRunoPlugin_TXLinkUi::HandleEvent ClosingDown called" << std::endl;
		RequestDllUnload();							//request unload of plugin from controller
													//   this will cause TXLink top level to be destroyed
													//   which generates a consitent shutdown sequence
		break;

	default:
		break;
	}
}

// Required to make sure the
//  plugin is correctly unloaded when closed
void SDRunoPlugin_TXLinkUi::RequestDllUnload()
{
	std::cout << get_millis() << " SDRunoPlugin_TXLinkUi:: m_controller.RequestDllUnload called" << std::endl;
	m_controller.RequestUnload(&m_parent);
}

std::string SDRunoPlugin_TXLinkUi::UnoEventTypeToString(UnoEvent::UnoEventType eventType)
{
	switch (eventType)
	{
	case UnoEvent::UndefinedEvent:                return "UndefinedEvent";
	case UnoEvent::DemodulatorChanged:            return "DemodulatorChanged";
	case UnoEvent::BandwidthChanged:              return "BandwidthChanged";
	case UnoEvent::FrequencyChanged:              return "FrequencyChanged";
	case UnoEvent::CenterFrequencyChanged:        return "CenterFrequencyChanged";
	case UnoEvent::SampleRateChanged:             return "SampleRateChanged";
	case UnoEvent::StreamingStarted:              return "StreamingStarted";
	case UnoEvent::StreamingStopped:              return "StreamingStopped";
	case UnoEvent::SquelchEnableChanged:          return "SquelchEnableChanged";
	case UnoEvent::SquelchThresholdChanged:       return "SquelchThresholdChanged";
	case UnoEvent::AgcThresholdChanged:           return "AgcThresholdChanged";
	case UnoEvent::AgcModeChanged:                return "AgcModeChanged";
	case UnoEvent::NoiseBlankerLevelChanged:      return "NoiseBlankerLevelChanged";
	case UnoEvent::NoiseReductionLevelChanged:    return "NoiseReductionLevelChanged";
	case UnoEvent::CwPeakFilterThresholdChanged:  return "CwPeakFilterThresholdChanged";
	case UnoEvent::FmNoiseReductionEnabledChanged:return "FmNoiseReductionEnabledChanged";
	case UnoEvent::FmNoiseReductionThresholdChanged:return "FmNoiseReductionThresholdChanged";
	case UnoEvent::WfmDeemphasisModeChanged:      return "WfmDeemphasisModeChanged";
	case UnoEvent::AudioVolumeChanged:            return "AudioVolumeChanged";
	case UnoEvent::AudioMuteChanged:              return "AudioMuteChanged";
	case UnoEvent::IFGainChanged:                 return "IFGainChanged";
	case UnoEvent::SavingWorkspace:               return "SavingWorkspace";
	case UnoEvent::VRXCountChanged:               return "VRXCountChanged";
	case UnoEvent::VRXStateChanged:               return "VRXStateChanged";
	case UnoEvent::StepSizeChanged:               return "StepSizeChanged";
	case UnoEvent::VFOChanged:                    return "VFOChanged";
	case UnoEvent::ClosingDown:                   return "ClosingDown";
	case UnoEvent::SP1MinFreqChanged:             return "SP1MinFreqChanged";
	case UnoEvent::SP1MaxFreqChanged:             return "SP1MaxFreqChanged";
	case UnoEvent::BiasTEnableChanged:            return "BiasTEnableChanged";
	case UnoEvent::SP1MinPowerChanged:            return "SP1MinPowerChanged";
	case UnoEvent::SP1MaxPowerChanged:            return "SP1MaxPowerChanged";
	case UnoEvent::StartRequest:                  return "StartRequest";
	case UnoEvent::StopRequest:                   return "StopRequest";
	case UnoEvent::VFOSignalPresent:              return "VFOSignalPresent";
	case UnoEvent::VFOSignalNotPresent:           return "VFOSignalNotPresent";
	default:                            return "UnknownEventType";
	}
}
