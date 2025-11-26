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
//
//DLL is entry point for SDRuno app. TXLinkProject.cpp contains CreatePlugin function which returns the new TXLink object.
// Classes: TXLink calls TXLinkUi calls TXLinkForm which inherits Nano::form
//			TXLink calls UDP_Server & TunerServer both in separate threads

SDRunoPlugin_TXLink::SDRunoPlugin_TXLink(IUnoPluginController& controller) :
	IUnoPlugin(controller),
	// Initialize all unique_ptr to nullptr
	m_Ui(nullptr),
	TunerThread(nullptr),
	RTTYThread(nullptr),
	// Assuming UDPthread is also a unique_ptr<std::thread>
	UDPthread(nullptr),

	// Initialize all simple types
	m_worker(nullptr),
	head(nullptr),
	tail(nullptr),
	current(nullptr),
	prev(nullptr),
	freqcurr(nullptr),
	annocurr(nullptr),
	AnnoCount(0),
	sampleRate(62500),
	bFormOpen(true)
{
	std::cout << get_millis() << " SDRunoPlugin_TXLink Constructor : m_Ui created next" << std::endl;
	// The m_Ui object constructor must be light-weight (no threads/Nana::exec() calls).
	m_Ui = std::make_unique<SDRunoPlugin_TXLinkUi>(*this, controller);
	StartTXLinkUi();

}

SDRunoPlugin_TXLink::~SDRunoPlugin_TXLink()
{	
	//comes here when SDRuno is closing the plugin
	// For nana shutdown we need to be careful to ensure nana Form thread is used for all Form operations
	std::cout << std::endl << std::endl << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl 
				<< get_millis() << " SDRunoPlugin_TXLink Destructor called: request Form unload prep" << std::endl;

	m_Ui->FormPrepareUnloadRequest = true;		//set flag to indicate form unload requested

	int sleeps = 0;
	while (m_Ui->FormPrepareUnloadRequest && sleeps < 50)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));         //wait for request to be cleared bt nana form timer event
		sleeps++;
	}
	std::cout << get_millis() << " SDRunoPlugin_TXLink Form unload prep complete " << std::boolalpha << !m_Ui->FormPrepareUnloadRequest  << " after " << sleeps * 10 << "ms" << std::endl;
	//m_Ui->m_thread.join(); // Wait for the UI thread to completely finish.
	//std::cout << "SDRunoPlugin_TXLink Destructor: Form thread joined and exited" << std::endl;

	StopAnnotator();

	// stop UDP server thread	
	if (UDPthread->joinable()) {
		UDPExitRequest = true;
		std::cout << get_millis() << " UDP Exit Requested" << std::endl;
		UDPthread->join(); // Wait for the thread to finish
		std::cout << get_millis() << " UDP Thread Joined and exit" << std::endl;
	}

	// stop Tuner server thread	
	if (TunerThread->joinable()) {
		TunerExitRequest = true;
		std::cout << get_millis() << " Tuner Exit Requested" << std::endl;
		TunerThread->join(); // Wait for the thread to finish
		std::cout << get_millis() << " Tuner Thread Joined and exit" << std::endl;
	}

	// stop RTTY server thread
	if (RTTYThread->joinable()) {
		RTTYExitRequest = true;
		std::cout << get_millis() << " RTTY Exit Requested" << std::endl;
		RTTYThread->join(); // Wait for the thread to finish
		std::cout << get_millis() << " RTTY Thread Joined and exit" << std::endl;
	}

	// stop DXspider server thread
	if (DXspiderThread->joinable()) {
		DXspider_force_close();
		DXspiderExitRequest = true;
		std::cout << get_millis() << " DXspider Exit Requested" << std::endl;
		DXspiderThread->join(); // Wait for the thread to finish
		std::cout << get_millis() << " DXspider Thread Joined and exit" << std::endl;
	}
}

void SDRunoPlugin_TXLink::StartTXLinkUi()
{
	std::cout << get_millis() << "SDRunoPlugin_TXLink::StartTXLinkUi: Starting UI and threads." << std::endl;
	// Check if the UI has already been created (in case StartTXLinkUi is called multiple times)
	if (!m_Ui) {
		// FIX: UI Initialization ONLY HAPPENS HERE (Safe for permanent instance)
		//m_Ui = std::make_unique<SDRunoPlugin_TXLinkUi>(*this, SDRunoPlugin_TXLink::m_controller);
	}

	// You MUST have a method in SDRunoPlugin_TXLinkUi to explicitly create/show the Nana form.
	m_Ui->StartFormThread();

	// Now, start your threads, as they rely on the UI object or the main plugin being stable.
	if (!TunerThread) {
		// Use reset() to explicitly initialize the unique_ptr with a new thread
		TunerThread.reset(
			new std::thread([this, &controller = SDRunoPlugin_TXLink::m_controller]() {
				Tuner_Server_Loop(controller, *this);
				})
		);
	}

	if (!RTTYThread) {
		RTTYThread.reset(
			// Capture 'this' (for member access) and '&controller' (which is local/external)
			new std::thread([this, &controller = SDRunoPlugin_TXLink::m_controller]() {

				// Pass the queue member (by reference) directly to the server loop function
				// NOTE: 'this->outgoing_RTTY_queue_' provides the queue instance.
				RTTY_Server_Loop(controller, *this);
				})
		);
	}

	// Initialize DXspiderThread here if needed, using the same reset() pattern:
	if (!DXspiderThread) {
		DXspiderThread.reset(new std::thread (DXspiderLoop));
	}

	// Initialize UDPthread here if needed, using the same reset() pattern:
	if (!UDPthread) {
	    UDPthread.reset(new std::thread(UDP_Server));
	}

	StartAnnotator();
}

// All thread joins and unique_ptr cleanup are left to the destructor.
void SDRunoPlugin_TXLink::StopTXLinkUi()
{
	std::cout << get_millis() << "\n\nSDRunoPlugin_TXLink::StopTXLinkUi: Signaling UI shutdown only." << std::endl;

	// 1. Signal UI shutdown
	if (m_Ui) {
		m_Ui->FormPrepareUnloadRequest = true;
	}

	// NOTE: No thread joining or m_Ui.reset() here to avoid crashing your destructor.
}


void SDRunoPlugin_TXLink::HandleEvent(const UnoEvent& ev)
{
	//std::cout << get_millis() << " SDRunoPlugin_TXLink::HandleEvent: Event received: " << ev.ToString() << std::endl;

	static bool bBusy = false;		//static so only one thread can process at a time
	if (bBusy) {
		std::cout << get_millis() << "SDRunoPlugin_TXLink::HandleEvent already busy *********************" << std::endl;
		return; // Exit immediately if already busy
	}

	bBusy = true;		//set busy flag so no other thread can process an event

	// --- LIFECYCLE MANAGEMENT ---

	static bool bStarted = false;
	if (!bStarted) {
		// This is the first event received after plugin creation.
		// We can do any one-time initialization here if needed.
		//std::cout << get_millis() << " SDRunoPlugin_TXLink::HandleEvent: First event after creation." << std::endl;
		//StartTXLinkUi();
		bStarted = true;
	}

	// 1. Intercept START Request: Create the UI object and threads.
	if (ev.GetType() == UnoEvent::StartRequest) { // StartRequest (32)
		//StartTXLinkUi();
	}

	// 2. Intercept STOP Request: Signal soft shutdown for the UI.
	// The full thread cleanup (your destructor code) will happen when DestroyPlugin is called.
	else if (ev.GetType() == UnoEvent::ClosingDown) {
		StopTXLinkUi();
	}

	// NOTE: We do NOT call StopTXLinkUi for ClosingDown (26) here.
	// We rely on the core SDRUno path (DestroyPlugin -> Destructor) for that hard cleanup.

	// 3. Forward Event: Only forward events if the UI object exists.
	if (m_Ui) {
		//std::cout << get_millis() << " SDRunoPlugin_TXLink::HandleEvent: Forwarding event to UI.  ev = "  << ev.ToString() << std::endl;
		m_Ui->HandleEvent(ev);
	}

	bBusy = false;
}


SDRunoPlugin_TXLinkForm::eSplitMode SDRunoPlugin_TXLink::GetSplitMode()
{
	return m_Ui->m_form->SplitMode;
}

void SDRunoPlugin_TXLink::AnnotatorProcess(std::vector<IUnoAnnotatorItem>& items)
{
	std::lock_guard<std::mutex> lock(m_lock);

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
		if (im.text == "RX" || im.text == "FTX")
		{
			im.power = -90;
		}
		im.rgb = colors[3];						// m_Ui->GetColourIndex()];
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
	std::cout << get_millis() << "TXLink::StopAnnotator: Unregistering Annotator" << std::endl;
	std::lock_guard<std::mutex> lock(m_lock);
	m_controller.UnregisterAnnotator(this);
}

void SDRunoPlugin_TXLink::add_AnnoEntry(std::string tag, std::string tim, long long freq)
{
	std::lock_guard<std::mutex> lock(m_lock);

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
	std::lock_guard<std::mutex> lock(m_lock);
	
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
	std::lock_guard<std::mutex> lock(m_lock);

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