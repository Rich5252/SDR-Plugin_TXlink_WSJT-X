#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/checkbox.hpp>
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
#include <queue>
#include <mutex>
#include <condition_variable>

#include <iunoplugincontroller.h>

#include "SDRunoPlugin_TXLinkATUDialog.h"
#include "SimpleSerial.h"
#include "TunerServer.h"
#include "DXspiderClient.h"
#include "MessageBox.h"
#include "NamedPipeServer.h"
#include "MessageQueue.h"


// Shouldn't need to change these
#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

// TODO: Change these numbers to the height and width of your form
#define formWidth (297)
#define formHeight (240)

#define FilterBandwidthCW 500
#define FilterBandwidthFT4 3000
#define FilterBandwidthFT8 3000

class SDRunoPlugin_TXLinkUi;
class SDRunoPlugin_TXLink;


class SDRunoPlugin_TXLinkForm : public nana::form
{

public:

	SDRunoPlugin_TXLinkForm(SDRunoPlugin_TXLinkUi& parent, IUnoPluginController& controller);
	~SDRunoPlugin_TXLinkForm();
	
	void Run();

	double	CurrentTXfreq = 10120000;
	double	CurrentRXfreq = 10120000;
	double	CurrentWSJTXfreq = 18100000;
	double	TXupHz = 500;
	long	FT8txDelayMillis = 250;
	string	strTXtemp = "";
	bool	bRITchkbox = false;

	enum eSplitMode {
		SplitNone,			//same RxTx, both vrx0 & vrx1 track same freq, only vrx0 changes freq
		SplitTxRx,			//split Rx Tx, vrx0 is RX freq and changes Rx, vrx1 is TX and can change TX
		SplitFT8			//TX is fixed base freq, both vrx0 and vrx1 set to Rx and both only change Rx
							//    wsjtx sets TX offset that is added to tx in Arduino.
							//	  This mode set on first FT8 TX/Tune enable.
	};

	bool TrackFreq = true;
	eSplitMode SplitMode = SplitNone;
	bool StandbyMode = true;

	string DigiTXmodeStr = "FT8";
	
	bool SendSerialMsg(const std::string& msg);			//thread safe way of sending messages from other processes or threads
	

private:
	const std::string com_port = "\\\\.\\COM5";
	string COMportPrefix = "\\\\.\\";
	DWORD COM_BAUD_RATE = CBR_115200;
	SimpleSerial Serial;

	std::queue<std::string> m_messageQueue;
	std::mutex m_queueMutex;
	bool _SendSerialMsg(string Msg); //local function to send serial messages frm msg queue

	void OnTimer();
	void ProcessMessageQueue();
	void CheckWindowState();
	void ProcessIncomingSerial();
	void ProcessUDPEvent();
	std::string ProcessRTTYcommand(std::string rttyCmdText);

	void StartTimer()
	{
		m_timerCount = 0.0;
		m_timer.start();
	}

	void StopTimer()
	{
		m_timer.stop();
	}

	void SetupDataMode();
	void SendRXfreq();
	void SendTXfreq();
	void SetRXFreq();
	void SwapTxRxFreq();
	bool GetRITchkbox();
	void FixFreqBtnClicked();
	void AtuBtnClicked();
	void DXspotBtnClicked();
	void AtuDialog_Closed();
	void FixTXup();
	int GetValidTXupHz();
	string GetValidTXhold();
	string GetValidFanHold();
	string GetValidFanLowPWM();
	string GetValidFT8txDelay();
	string GetValidTempLimit();
	string GetSetConfigurationKey(std::string strItem, std::string strDefault);
	void CloseCOMPort();
	void SetStandbyMode(bool Mode);
	void SetMuteTX(bool chk);
	void SendTXlevel(string strTXlevel);
	void SendPAlevel(string strPAlevel);
	void SendTXhold(string TXhold);
	void SendFanHold(string FanHold);
	void SendFanLowPWM(string FanLowPWM);
	void SendFT8txDelay(string FT8txDelay);
	void SendTempLimit(string TempLimit);
	void SendPAstate(bool PAstate);
	void SendFreqCalToTX();
	double strToDouble(string str);				//with error check
	void ShowIsSent(bool is_sent);

	string	GetCOM();
	string	GetTXlevel();
	string	GetPAlevel();
	string	GetRIT();
	void	SetRIT(string strRIT);
	void	SaveSplitMode();
	long	millisToEnd();					//calc FT8 millis to end of tx period

	void FixFreqButtonClicked();
	int LoadX();
	int LoadY();
	void SaveLocation();
	void SaveIniData();
	string LoadCOM();
	string LoadTXlevel();
	string LoadPAlevel();
	void SaveCOM(string strCOM);
	void SaveTXlevel(string strTXlevel);
	void SavePAlevel(string strPAlevel);
	string LoadRIT();
	void LoadSplitMode();
	void SaveRIT(string strRIT);
	void SetStandbyModeClicked();

	void SetTXtemp(string strTemp);
	std::string GetTXtemp();

	void InitiateFormUnload();

	void Setup();
	bool UnloadInProgress = false;
	bool BackFromExitRequest = false;

	// The following is to set up the panel graphic to look like a standard SDRuno panel
	nana::picture bg_border{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::picture bg_inner{ bg_border, nana::rectangle(sideBorderWidth, topBarHeight, formWidth - (2 * sideBorderWidth), formHeight - topBarHeight - bottomBarHeight) };
	nana::picture header_bar{ *this, true };
	nana::label title_bar_label{ *this, true };
	nana::dragger form_dragger;
	nana::label form_drag_label{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::paint::image img_min_normal;
	nana::paint::image img_min_down;
	nana::paint::image img_close_normal;
	nana::paint::image img_close_down;
	nana::paint::image img_header;
	nana::picture close_button{ *this, nana::rectangle(0, 0, 20, 15) };
	nana::picture min_button{ *this, nana::rectangle(0, 0, 20, 15) };
	nana::label versionLbl{ *this, nana::rectangle(formWidth - 40, 38, 30, 20) };

	// Uncomment the following 5 lines if you want a SETT button and panel
	nana::paint::image img_sett_normal;
	nana::paint::image img_sett_down;
	nana::picture sett_button{ *this, nana::rectangle(0, 0, 40, 15) };
	void SettingsButton_Click();
	void SettingsDialog_Closed();

	nana::timer m_timer;

	// TODO: Now add your UI controls here
	nana::label freqTbLbl{ *this, nana::rectangle(20, 38, 65, 20) };
	nana::label freqTb{ *this, nana::rectangle(98, 35, 105, 22)};
	nana::button FixFreqBtn{ *this, nana::rectangle(20, 65, 50, 20) };
	nana::button FixTXupBtn{ *this, nana::rectangle(75, 65,50,20) };
	nana::button ResetRXFreqBtn{ *this, nana::rectangle(130, 65, 50, 20) };
	nana::button SwapTxRxBtn{ *this, nana::rectangle(185, 65, 50, 20) };
	nana::checkbox RITchkbox{ *this, nana::rectangle(240, 68, 50, 20) };
	nana::textbox COMportTb{ *this, nana::rectangle(20, 95, 50, 20) };
	nana::button StandbyBtn{ *this, nana::rectangle(110, 95, 80, 20) };
	nana::checkbox MuteTXchkbox{ *this, nana::rectangle(205, 98, 70, 20) };
	nana::label FT8Lbl{ *this, nana::rectangle(20, 126, 25, 20) }; 
	nana::label FT8txOnLbl{ *this, nana::rectangle(50, 128, 10, 10) };
	nana::label FT8msgLbl{ *this, nana::rectangle(70, 126, 210, 20) };
	nana::button DXspotBtn{ *this, nana::rectangle(259, 126, 20, 20) };
	nana::textbox TXlevelTb{ *this, nana::rectangle(110, 149, 40, 20) };
	nana::label TXlevelLbl{ *this, nana::rectangle(18, 152, 100, 20) };
	nana::label TXleveldBLbl{ *this, nana::rectangle(160, 152, 40, 20) };
	nana::label TXtempLbl{ *this, nana::rectangle(205, 152, 80, 20) };
	nana::textbox PAlevelTb{ *this, nana::rectangle(110, 176, 40, 20) };
	nana::label PAlevelLbl{ *this, nana::rectangle(18, 179, 100, 20) };
	nana::label PAleveldBLbl{ *this, nana::rectangle(160, 179, 40, 20) };
	nana::checkbox PAchkbox{ *this, nana::rectangle(205, 179, 50, 20) };
	nana::label SWRFWDlbl{ *this, nana::rectangle(19, 205, 30, 20) };
	nana::label SWRFWDtxt{ *this, nana::rectangle(59, 202, 30, 20) };
	nana::label SWRREFlbl{ *this, nana::rectangle(99, 205, 30, 20) };
	nana::label SWRREFtxt{ *this, nana::rectangle(139, 202, 30, 20) };
	nana::label SWRSWRlbl{ *this, nana::rectangle(179, 205, 30, 20) };
	nana::label SWRSWRtxt{ *this, nana::rectangle(219, 202, 30, 20) };
	nana::button AtuBtn{ *this, nana::rectangle(259, 202, 20, 20) };
	double m_timerCount;

	std::unique_ptr<SDRunoPlugin_TemplateATUDialog> m_atuDialog;
	bool bATUformOpen;

	bool bLastTenCalls(string DXcall);				// chk last ten dxcalls from wsjtx
	#define nCalls 10
	string strLastTenCalls[nCalls];
	int nLastCall;									//last call index


	SDRunoPlugin_TXLinkUi & m_parent;
	IUnoPluginController & m_controller;

	std::mutex m_lock;
};
