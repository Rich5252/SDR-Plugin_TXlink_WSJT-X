#pragma once

#include <thread>
#include <functional>
#include <mutex>
#include <atomic>
#include <iunoplugincontroller.h>
#include <iunoplugin.h>
#include <iunostreamobserver.h>
#include <iunoaudioobserver.h>
#include <iunoaudioprocessor.h>
#include <iunostreamobserver.h>
#include <iunoannotator.h>

#include "SDRunoPlugin_TXLink.h"
#include "SDRunoPlugin_TXLinkUi.h"
#include "UDP_Server.h"
#include "RTTY_Server.h"

//TXlink plugin for SDRuno app. The plugin code is based on the SDRuno plugin template example.
// It talks to an Arduino transmitter project via USB serial link.
// Includes a UDP server connected to wsjtx app that provides FT8 digital support.
// The FT8 encoding is done using the wsjtx utility (ft8code.exe) that is included in their installation.
// 73's G4AHN


class SDRunoPlugin_TXLink : public IUnoPlugin,
							public IUnoAnnotator
{

public:
	
	SDRunoPlugin_TXLink(IUnoPluginController& controller);
	virtual ~SDRunoPlugin_TXLink();

	// TODO: change the plugin title here
	virtual const char* GetPluginName() const override { return "TX Link"; }

	// IUnoPlugin
	virtual void HandleEvent(const UnoEvent& ev) override;

	//annotation
	virtual void AnnotatorProcess(std::vector<IUnoAnnotatorItem>& items) override;

	void StartAnnotator();
	void StopAnnotator();

	void UpdateSampleRate();

	int ANNOCount() const { return AnnoCount; }

	struct AnnoEntry {
		std::string callsign;
		long long freq;
		std::string timeUTC;
		IUnoAnnotatorStyle style;
		AnnoEntry* next;
	};

	AnnoEntry* head;
	AnnoEntry* tail;
	AnnoEntry* current;
	AnnoEntry* prev;
	AnnoEntry* annocurr;
	AnnoEntry* freqcurr;

	void add_AnnoEntry(std::string tag, std::string tim, long long freq);
	void updateTX_AnnoEntry(std::string tag, std::string tim, long long freq);
	void updateRX_AnnoEntry(std::string tag, std::string tim, long long freq);
	//end annotation

	SDRunoPlugin_TXLinkForm::eSplitMode GetSplitMode();

	std::unique_ptr<std::thread> UDPthread;
	std::unique_ptr<std::thread> TunerThread;
	std::unique_ptr<std::thread> RTTYThread;
	std::unique_ptr<std::thread> DXspiderThread;
	std::unique_ptr<SDRunoPlugin_TXLinkUi> m_Ui;
	void StartTXLinkUi();			// New public method to start m_Ui
	void StopTXLinkUi();			// New public method to shutdown m_Ui


private:
	void WorkerFunction();
	std::thread* m_worker;

	std::mutex m_lock;

	int AnnoCount;
	int64_t sampleRate;

	bool bFormOpen;

};