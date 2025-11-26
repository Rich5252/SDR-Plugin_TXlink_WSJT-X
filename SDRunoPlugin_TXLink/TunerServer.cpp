#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <format>
#include <tuple>
#include <chrono>
#include <thread>
#include <windows.h>
#include <atomic>

#include "SDRunoPlugin_TXLink.h"
#include "TunerServer.h"
#include "SendKeys.h"
#include "UDP_Server.h"

std::atomic<bool> TunerExitRequest(false);
std::mutex m_lock3;

//started in a seperate thread from SDRunoPlugin_TXLink.cpp
void Tuner_Server_Loop(IUnoPluginController& _controller, SDRunoPlugin_TXLink& _parent)
{
    //
	int32_t EncoderSteps = 0;
	int32_t FreqIncr = 0;

	const std::string tuner_com_port = "\\\\.\\COM9";
	string tuner_COMportPrefix = "\\\\.\\";
	DWORD tuner_COM_BAUD_RATE = CBR_115200;
	SimpleSerial TunerSerial(tuner_com_port, tuner_COM_BAUD_RATE, false);

	while (!TunerExitRequest && TunerSerial.GetErrorMsg() == "")
    {
		if (!TunerSerial.Connected())
		{
			std::cout << get_millis() << "Tuner Serial not connected" << std::endl;
			//attempt to restart COM port
			string strCOM = "COM9";
			strCOM = tuner_COMportPrefix + strCOM;
			TunerSerial.ReOpenPort(strCOM, tuner_COM_BAUD_RATE, false);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		if (TunerSerial.Connected()) {			
			int32_t deltaFreq = 10;

			if (_controller.GetDemodulatorType(0) == _controller.DemodulatorCW) { deltaFreq = 1; }
			while (TunerSerial.DataAvailable() > 0)
			{
				string str = TunerSerial.ReadSerialPortChar();
				if (str == ">") {
					EncoderSteps++;
					FreqIncr = FreqIncr + deltaFreq;
				}
				if (str == "<")	{
					EncoderSteps--;
					FreqIncr = FreqIncr - deltaFreq;
				}
			}
			if (FreqIncr != 0)
			{
				if (TunerExitRequest) { break; }
				m_lock3.lock();
				SDRunoPlugin_TXLinkForm::eSplitMode tmp = _parent.m_Ui->m_form->SplitMode;			//can now get split mode from _parent.m_Ui.SplitMode directly
				if (tmp == SDRunoPlugin_TXLinkForm::SplitNone)		
				{
					//no split so change RX frequency
					long Freq = _controller.GetVfoFrequency(0);
					_controller.SetVfoFrequency(0, Freq + FreqIncr);
					EncoderSteps = 0;	//reset encoder steps
					FreqIncr = 0;	//reset frequency increment
				}
				if (tmp == SDRunoPlugin_TXLinkForm::SplitTxRx)
				{
					if (_parent.m_Ui->m_form->bRITchkbox)
					{
						//RIT so change RX frequency
						long Freq = _controller.GetVfoFrequency(0);
						_controller.SetVfoFrequency(0, Freq + FreqIncr);
					}
					else
					{
						//no RIT so change TX frequency
						long Freq = _controller.GetVfoFrequency(1);
						_controller.SetVfoFrequency(1, Freq + FreqIncr);
					}
					EncoderSteps = 0;	//reset encoder steps
					FreqIncr = 0;	//reset frequency increment
				}
				//if FT4/8 split count up 1 revolution (800 steps) and send CTRL SHFT F11 or F12 to WSTJTX to change TX frequency by 1kHz
				if (EncoderSteps > 800)
				{
					//double Freq = _controller.GetVfoFrequency(0);
					//WSJTX_UDPOutQ.push((uint64_t)(Freq + 1000));		//send new freq to wsjtx via UDP
					
					Send_CtrlShftF12();
					EncoderSteps = 0;
					FreqIncr = 0;
				}
				if (EncoderSteps < -800)
				{
					//double Freq = _controller.GetVfoFrequency(0);
					//WSJTX_UDPOutQ.push((uint64_t)(Freq - 1000));		//send new freq to wsjtx via UDP

					
					Send_CtrlShftF11();
					EncoderSteps = 0;
					FreqIncr = 0;
				}
				m_lock3.unlock();
			}

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

	/*	string tmp = TunerSerial.GetErrorMsg();			//GetErrorMsg not implimented yet
	if (tmp != "")
	{
		LogAndDisplayError(tmp);
		std::cout << get_millis() << "Tuner Serial error: " << tmp << std::endl;
	}
*/

	//end thread
	std::cout << get_millis() << "TunerSerial exit equest received, attempt to close com port" << std::endl;
	int ret = TunerSerial.CloseSerialPort();
	std::cout << get_millis() << "TunerSerial port closed " << ret << std::endl;

	std::cout << get_millis() << "Tuner Server thread exit" << std::endl;
}

