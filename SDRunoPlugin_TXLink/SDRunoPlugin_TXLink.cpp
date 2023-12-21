#include <sstream>
#include <unoevent.h>
#include <iunoplugincontroller.h>
#include <vector>
#include <sstream>
#include <chrono>

#include "SDRunoPlugin_TXLink.h"
#include "SDRunoPlugin_TXLinkUi.h"
#include "SDRunoPlugin_TXLinkForm.h"

//TXlink plugin for SDRuno app. The plugin code is based on the SDRuno plugin template example.
// It talks to an Arduino transmitter project via USB serial link.
// Includes a UDP server connected to wsjtx app that provides FT8 digital support.
// The FT8 encoding is done using the wsjtx utility (ft8code.exe) that is included in their installation.
// 73's G4AHN


SDRunoPlugin_TXLink::SDRunoPlugin_TXLink(IUnoPluginController& controller) :
	IUnoPlugin(controller),
	m_form(*this, controller),
	m_worker(nullptr),
	head(nullptr),
	tail(nullptr),
	current(nullptr),
	prev(nullptr),
	freqcurr(nullptr),
	annocurr(nullptr),
	AnnoCount(0),
	sampleRate(62500),
	UDPthread (& UDP_Server)
{
	StartAnnotator();

}

SDRunoPlugin_TXLink::~SDRunoPlugin_TXLink()
{	
	StopAnnotator();

	// stop UDP server thread
	UDPExitRequest = true;
	if (UDPthread.joinable()) {
		UDPthread.join(); // Wait for the thread to finish
	}
}

void SDRunoPlugin_TXLink::HandleEvent(const UnoEvent& ev)
{
	m_form.HandleEvent(ev);	
}

void SDRunoPlugin_TXLink::AnnotatorProcess(std::vector<IUnoAnnotatorItem>& items)
{
	const uint32_t colors[] = {
		0x00ff0000, // red
		0x00ffff00, // yellow
		0x00ff00ff, // purple
		0x00aaff00, // green
		0x0030a0ff, // light blue
		0x00ffffff  // white
	};

	annocurr = head;
	while (annocurr != nullptr)
	{
		IUnoAnnotatorItem im;
		im.frequency = annocurr->freq;
		im.text = annocurr->callsign;
		im.power = -85;
		if (im.text == "RX")
		{
			im.power = -90;
		}
		im.rgb = colors[3];						// m_form.GetColourIndex()];
		im.style = IUnoAnnotatorStyle::AnnotatorStyleMarkerAndLine;

		items.push_back(im);
		if (annocurr->next != nullptr)
		{
			annocurr = annocurr->next;
		}
		else
		{
			break;
		}
	}
}

void SDRunoPlugin_TXLink::StartAnnotator()
{
	m_controller.RegisterAnnotator(this);
}

void SDRunoPlugin_TXLink::StopAnnotator()
{
	m_controller.UnregisterAnnotator(this);
}

void SDRunoPlugin_TXLink::add_AnnoEntry(std::string tag, std::string tim, long long freq)
{
	AnnoEntry* tmp = new AnnoEntry;
	tmp->callsign = tag;
	tmp->freq = freq;
	tmp->timeUTC = tim;
	tmp->next = nullptr;

	if (head == nullptr)
	{
		head = tmp;
		tail = tmp;
	}
	else
	{
		tail->next = tmp;
		tail = tail->next;
	}
}

void SDRunoPlugin_TXLink::updateTX_AnnoEntry(std::string tag, std::string tim, long long freq)
{	// First entry is TX freq
	if (head == NULL)
	{
		AnnoEntry* tmp = new AnnoEntry;
		tmp->callsign = tag;
		tmp->freq = freq;
		tmp->timeUTC = tim;
		tmp->next = NULL;

		head = tmp;
		tail = tmp;
	}
	else
	{
		AnnoEntry* tmp = head;
		tmp->callsign = tag;
		tmp->freq = freq;
		tmp->timeUTC = tim;
	}
}

void SDRunoPlugin_TXLink::updateRX_AnnoEntry(std::string tag, std::string tim, long long freq)
{	// Second entry is RX freq
	if (head == NULL)
	{	//add both TX and RX if still empty
		AnnoEntry* tmp = new AnnoEntry;
		tmp->callsign = tag;
		tmp->freq = freq;
		tmp->timeUTC = tim;
		tmp->next = NULL;

		head = tmp;
		tail = tmp;
	}

	if (head->next == NULL)
	{	//add second (RX) entry
		AnnoEntry* tmp = new AnnoEntry;
		tmp->callsign = tag;
		tmp->freq = freq;
		tmp->timeUTC = tim;
		tmp->next = NULL;

		head->next = tmp;
		tail = tmp;
	}
	else
	{	//second entry
		AnnoEntry* tmp = head->next;
		tmp->callsign = tag;
		tmp->freq = freq;
		tmp->timeUTC = tim;
	}
}

void SDRunoPlugin_TXLink::UpdateSampleRate()
{
	sampleRate = (int64_t)m_controller.GetSampleRate(0);
}

void SDRunoPlugin_TXLink::WorkerFunction()
{
	// Worker Function Code Goes Here

}