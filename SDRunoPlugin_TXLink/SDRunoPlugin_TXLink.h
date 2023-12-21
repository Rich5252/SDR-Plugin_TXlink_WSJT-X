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

#include "SDRunoPlugin_TXLinkUi.h"
#include "UDP_Server.h"


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

	int ANNOCount() { return AnnoCount; }

	struct AnnoEntry {
		std::string callsign;
		long long freq;
		std::string timeUTC;
		IUnoAnnotatorStyle style;
		AnnoEntry* next;
	};

	AnnoEntry* head, * tail, * current, * prev, * annocurr, * freqcurr;

	void add_AnnoEntry(std::string tag, std::string tim, long long freq);
	void updateTX_AnnoEntry(std::string tag, std::string tim, long long freq);
	void updateRX_AnnoEntry(std::string tag, std::string tim, long long freq);
	//end annotation

	std::thread UDPthread;


private:
	
	void WorkerFunction();
	std::thread* m_worker;

	std::mutex m_lock;
	SDRunoPlugin_TXLinkUi m_form;


	int AnnoCount;
	int64_t sampleRate;

//	std::thread threadObject;
//	std::reference_wrapper<std::thread> UDPthread;
};