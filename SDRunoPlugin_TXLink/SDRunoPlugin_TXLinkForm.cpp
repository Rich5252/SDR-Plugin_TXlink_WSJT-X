#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#include <winuser.h>
#endif

#include "SDRunoPlugin_TXLinkForm.h"
#include "SDRunoPlugin_TXLinkSettingsDialog.h"
#include "SDRunoPlugin_TXLinkATUDialog.h"
#include "SDRunoPlugin_TXLinkUi.h"
#include "SDRunoPlugin_TXLink.h"
#include "UDP_Server.h"
#include "resource.h"
#include "RTTY_Server.h"
#include "DXspiderClient.h"
#include "FindWindowTitleRoot.h"

#include <io.h>
#include <shlobj.h>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/dragger.hpp>


#define VERSION "V2.0"

// Serial commands from PC TXlink sent to Arduino
// {Bn}     Ext PA fan hold time minutes
// {Cn}     DDS Frequency calibration  
// {D}      Power Down (TX off)
// {Fn}     TX frequency
// {Hn}     TX key up hold time ms
// {L%}     Local PA level (QRP)
// {P%)     External PA power level
// {Q0/1}   External PA Off/On
// {U}      Power Up (TX On)
// {V%}		FT8 TX start delay ms (default 250ms)
// {W%}     TX FT8 watchdog - millisecs to end of this tx period
// {X0/1)   FT8 TX on/off
// {Yn}     FT8 TXDF (delta freq)
// {Zmsg}   FT8 message


HWND UNOhwnd = NULL;


// Form constructor with handles to parent and uno controller - launches form Setup
SDRunoPlugin_TXLinkForm::SDRunoPlugin_TXLinkForm(SDRunoPlugin_TXLinkUi& parent, IUnoPluginController& controller) :
	nana::form(nana::API::make_center(formWidth, formHeight), nana::appearance(false, true, false, false, true, false, false)),
	m_parent(parent),
	m_timerCount(0),
	m_controller(controller),
	Serial(com_port, COM_BAUD_RATE, false)
{
	Setup();
}

// Form deconstructor
SDRunoPlugin_TXLinkForm::~SDRunoPlugin_TXLinkForm()
{
	std::cout << get_millis() << " SDRunoPlugin_TXLinkForm destructor called" << std::endl;
}

// Start Form and start Nana UI processing
void SDRunoPlugin_TXLinkForm::Run()
{
	show();
	nana::exec();			//returns here after nana::API::exit_all() called

	std::cout << get_millis() << " SDRunoPlugin_TXLinkForm Run() EXIT" << std::endl;
	//$m_parent.FormExitAllRequest = false;		//tell parent closedown function that nana forms have exited
}



// Create the initial plugin form
void SDRunoPlugin_TXLinkForm::Setup()
{
	// This first section is all related to the background and border
	// it shouldn't need to be changed
	nana::paint::image img_border;
	nana::paint::image img_inner;
	HMODULE hModule = NULL;
	HRSRC rc_border = NULL;
	HRSRC rc_inner = NULL;
	HRSRC rc_close = NULL;
	HRSRC rc_close_over = NULL;
	HRSRC rc_min = NULL;
	HRSRC rc_min_over = NULL;
	HRSRC rc_bar = NULL;
	HRSRC rc_sett = NULL;
	HRSRC rc_sett_over = NULL;
	HBITMAP bm_border = NULL;
	HBITMAP bm_inner = NULL;
	HBITMAP bm_close = NULL;
	HBITMAP bm_close_over = NULL;
	HBITMAP bm_min = NULL;
	HBITMAP bm_min_over = NULL;
	HBITMAP bm_bar = NULL;
	HBITMAP bm_sett = NULL;
	HBITMAP bm_sett_over = NULL;
	BITMAPINFO bmInfo_border = { 0 };
	BITMAPINFO bmInfo_inner = { 0 };
	BITMAPINFO bmInfo_close = { 0 };
	BITMAPINFO bmInfo_close_over = { 0 };
	BITMAPINFO bmInfo_min = { 0 };
	BITMAPINFO bmInfo_min_over = { 0 };
	BITMAPINFO bmInfo_bar = { 0 };
	BITMAPINFO bmInfo_sett = { 0 };
	BITMAPINFO bmInfo_sett_over = { 0 };
	BITMAPFILEHEADER borderHeader = { 0 };
	BITMAPFILEHEADER innerHeader = { 0 };
	BITMAPFILEHEADER closeHeader = { 0 };
	BITMAPFILEHEADER closeoverHeader = { 0 };
	BITMAPFILEHEADER minHeader = { 0 };
	BITMAPFILEHEADER minoverHeader = { 0 };
	BITMAPFILEHEADER barHeader = { 0 };
	BITMAPFILEHEADER settHeader = { 0 };
	BITMAPFILEHEADER settoverHeader = { 0 };
	HDC hdc = NULL;
	BYTE* borderPixels = NULL;
	BYTE* innerPixels = NULL;
	BYTE* closePixels = NULL;
	BYTE* closeoverPixels = NULL;
	BYTE* minPixels = NULL;
	BYTE* minoverPixels = NULL;
	BYTE* barPixels = NULL;
	BYTE* barfocusedPixels = NULL;
	BYTE* settPixels = NULL;
	BYTE* settoverPixels = NULL;
	const unsigned int rawDataOffset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	hModule = GetModuleHandle(L"SDRunoPlugin_TXLink");
	hdc = GetDC(NULL);
	rc_border = FindResource(hModule, MAKEINTRESOURCE(IDB_BG_BORDER), RT_BITMAP);
	rc_inner = FindResource(hModule, MAKEINTRESOURCE(IDB_BACKGROUND), RT_BITMAP);
	rc_close = FindResource(hModule, MAKEINTRESOURCE(IDB_CLOSE), RT_BITMAP);
	rc_close_over = FindResource(hModule, MAKEINTRESOURCE(IDB_CLOSE_DOWN), RT_BITMAP);
	rc_min = FindResource(hModule, MAKEINTRESOURCE(IDB_MIN), RT_BITMAP);
	rc_min_over = FindResource(hModule, MAKEINTRESOURCE(IDB_MIN_DOWN), RT_BITMAP);
	rc_bar = FindResource(hModule, MAKEINTRESOURCE(IDB_HEADER), RT_BITMAP);
	rc_sett = FindResource(hModule, MAKEINTRESOURCE(IDB_SETT), RT_BITMAP);
	rc_sett_over = FindResource(hModule, MAKEINTRESOURCE(IDB_SETT_DOWN), RT_BITMAP);
	bm_border = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_BG_BORDER), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_inner = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_BACKGROUND), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_close = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_CLOSE), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_close_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_CLOSE_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_min = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_MIN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_min_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_MIN_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_bar = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_HEADER), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_sett = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_SETT), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_sett_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_SETT_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bmInfo_border.bmiHeader.biSize = sizeof(bmInfo_border.bmiHeader);
	bmInfo_inner.bmiHeader.biSize = sizeof(bmInfo_inner.bmiHeader);
	bmInfo_close.bmiHeader.biSize = sizeof(bmInfo_close.bmiHeader);
	bmInfo_close_over.bmiHeader.biSize = sizeof(bmInfo_close_over.bmiHeader);
	bmInfo_min.bmiHeader.biSize = sizeof(bmInfo_min.bmiHeader);
	bmInfo_min_over.bmiHeader.biSize = sizeof(bmInfo_min_over.bmiHeader);
	bmInfo_bar.bmiHeader.biSize = sizeof(bmInfo_bar.bmiHeader);
	bmInfo_sett.bmiHeader.biSize = sizeof(bmInfo_sett.bmiHeader);
	bmInfo_sett_over.bmiHeader.biSize = sizeof(bmInfo_sett_over.bmiHeader);
	GetDIBits(hdc, bm_border, 0, 0, NULL, &bmInfo_border, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_inner, 0, 0, NULL, &bmInfo_inner, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close, 0, 0, NULL, &bmInfo_close, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close_over, 0, 0, NULL, &bmInfo_close_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min, 0, 0, NULL, &bmInfo_min, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min_over, 0, 0, NULL, &bmInfo_min_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_bar, 0, 0, NULL, &bmInfo_bar, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett, 0, 0, NULL, &bmInfo_sett, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett_over, 0, 0, NULL, &bmInfo_sett_over, DIB_RGB_COLORS);
	bmInfo_border.bmiHeader.biCompression = BI_RGB;
	bmInfo_inner.bmiHeader.biCompression = BI_RGB;
	bmInfo_close.bmiHeader.biCompression = BI_RGB;
	bmInfo_close_over.bmiHeader.biCompression = BI_RGB;
	bmInfo_min.bmiHeader.biCompression = BI_RGB;
	bmInfo_min_over.bmiHeader.biCompression = BI_RGB;
	bmInfo_bar.bmiHeader.biCompression = BI_RGB;
	bmInfo_sett.bmiHeader.biCompression = BI_RGB;
	bmInfo_sett_over.bmiHeader.biCompression = BI_RGB;
	borderHeader.bfOffBits = rawDataOffset;
	borderHeader.bfSize = bmInfo_border.bmiHeader.biSizeImage;
	borderHeader.bfType = 0x4D42;
	innerHeader.bfOffBits = rawDataOffset;
	innerHeader.bfSize = bmInfo_inner.bmiHeader.biSizeImage;
	innerHeader.bfType = 0x4D42;
	closeHeader.bfOffBits = rawDataOffset;
	closeHeader.bfSize = bmInfo_close.bmiHeader.biSizeImage;
	closeHeader.bfType = 0x4D42;
	closeoverHeader.bfOffBits = rawDataOffset;
	closeoverHeader.bfSize = bmInfo_close_over.bmiHeader.biSizeImage;
	closeoverHeader.bfType = 0x4D42;
	minHeader.bfOffBits = rawDataOffset;
	minHeader.bfSize = bmInfo_min.bmiHeader.biSizeImage;
	minHeader.bfType = 0x4D42;
	minoverHeader.bfOffBits = rawDataOffset;
	minoverHeader.bfSize = bmInfo_min_over.bmiHeader.biSizeImage;
	minoverHeader.bfType = 0x4D42;
	barHeader.bfOffBits = rawDataOffset;
	barHeader.bfSize = bmInfo_bar.bmiHeader.biSizeImage;
	barHeader.bfType = 0x4D42;
	settHeader.bfOffBits = rawDataOffset;
	settHeader.bfSize = bmInfo_sett.bmiHeader.biSizeImage;
	settHeader.bfType = 0x4D42;
	settoverHeader.bfOffBits = rawDataOffset;
	settoverHeader.bfSize = bmInfo_sett_over.bmiHeader.biSizeImage;
	settoverHeader.bfType = 0x4D42;
	borderPixels = new BYTE[bmInfo_border.bmiHeader.biSizeImage + rawDataOffset];
	innerPixels = new BYTE[bmInfo_inner.bmiHeader.biSizeImage + rawDataOffset];
	closePixels = new BYTE[bmInfo_close.bmiHeader.biSizeImage + rawDataOffset];
	closeoverPixels = new BYTE[bmInfo_close_over.bmiHeader.biSizeImage + rawDataOffset];
	minPixels = new BYTE[bmInfo_min.bmiHeader.biSizeImage + rawDataOffset];
	minoverPixels = new BYTE[bmInfo_min_over.bmiHeader.biSizeImage + rawDataOffset];
	barPixels = new BYTE[bmInfo_bar.bmiHeader.biSizeImage + rawDataOffset];
	settPixels = new BYTE[bmInfo_sett.bmiHeader.biSizeImage + rawDataOffset];
	settoverPixels = new BYTE[bmInfo_sett_over.bmiHeader.biSizeImage + rawDataOffset];
	*(BITMAPFILEHEADER*)borderPixels = borderHeader;
	*(BITMAPINFO*)(borderPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_border;
	*(BITMAPFILEHEADER*)innerPixels = innerHeader;
	*(BITMAPINFO*)(innerPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_inner;
	*(BITMAPFILEHEADER*)closePixels = closeHeader;
	*(BITMAPINFO*)(closePixels + sizeof(BITMAPFILEHEADER)) = bmInfo_close;
	*(BITMAPFILEHEADER*)closeoverPixels = closeoverHeader;
	*(BITMAPINFO*)(closeoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_close_over;
	*(BITMAPFILEHEADER*)minPixels = minHeader;
	*(BITMAPINFO*)(minPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_min;
	*(BITMAPFILEHEADER*)minoverPixels = minoverHeader;
	*(BITMAPINFO*)(minoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_min_over;
	*(BITMAPFILEHEADER*)barPixels = barHeader;
	*(BITMAPINFO*)(barPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_bar;
	*(BITMAPFILEHEADER*)settPixels = settHeader;
	*(BITMAPINFO*)(settPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_sett;
	*(BITMAPFILEHEADER*)settoverPixels = settoverHeader;
	*(BITMAPINFO*)(settoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_sett_over;
	GetDIBits(hdc, bm_border, 0, bmInfo_border.bmiHeader.biHeight, (LPVOID)(borderPixels + rawDataOffset), &bmInfo_border, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_inner, 0, bmInfo_inner.bmiHeader.biHeight, (LPVOID)(innerPixels + rawDataOffset), &bmInfo_inner, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close, 0, bmInfo_close.bmiHeader.biHeight, (LPVOID)(closePixels + rawDataOffset), &bmInfo_close, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close_over, 0, bmInfo_close_over.bmiHeader.biHeight, (LPVOID)(closeoverPixels + rawDataOffset), &bmInfo_close_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min, 0, bmInfo_min.bmiHeader.biHeight, (LPVOID)(minPixels + rawDataOffset), &bmInfo_min, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min_over, 0, bmInfo_min_over.bmiHeader.biHeight, (LPVOID)(minoverPixels + rawDataOffset), &bmInfo_min_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_bar, 0, bmInfo_bar.bmiHeader.biHeight, (LPVOID)(barPixels + rawDataOffset), &bmInfo_bar, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett, 0, bmInfo_sett.bmiHeader.biHeight, (LPVOID)(settPixels + rawDataOffset), &bmInfo_sett, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett_over, 0, bmInfo_sett_over.bmiHeader.biHeight, (LPVOID)(settoverPixels + rawDataOffset), &bmInfo_sett_over, DIB_RGB_COLORS);
	img_border.open(borderPixels, bmInfo_border.bmiHeader.biSizeImage);
	img_inner.open(innerPixels, bmInfo_inner.bmiHeader.biSizeImage);
	img_close_normal.open(closePixels, bmInfo_close.bmiHeader.biSizeImage);
	img_close_down.open(closeoverPixels, bmInfo_close_over.bmiHeader.biSizeImage);
	img_min_normal.open(minPixels, bmInfo_min.bmiHeader.biSizeImage);
	img_min_down.open(minoverPixels, bmInfo_min_over.bmiHeader.biSizeImage);
	img_header.open(barPixels, bmInfo_bar.bmiHeader.biSizeImage);
	img_sett_normal.open(settPixels, bmInfo_sett.bmiHeader.biSizeImage);
	img_sett_down.open(settoverPixels, bmInfo_sett_over.bmiHeader.biSizeImage);
	ReleaseDC(NULL, hdc);
	bg_border.load(img_border, nana::rectangle(0, 0, 590, 340));
	bg_border.stretchable(0, 0, 0, 0);
	bg_border.transparent(true);
	bg_inner.load(img_inner, nana::rectangle(0, 0, 582, 299));
	bg_inner.stretchable(sideBorderWidth, 0, sideBorderWidth, bottomBarHeight);
	bg_inner.transparent(false);
	const uint32_t forecolor = 0xffffff;


	// This code sets the plugin size, title and what to do when the X is pressed
	size(nana::size(formWidth, formHeight));
	caption("SDRuno Plugin TXlink");
	//events().destroy([&] { m_parent.FormClosed(); });
	//events().unload([&] { m_parent.FormUnload(); });
	events().unload([&](const nana::arg_unload& arg)					//called when X is pressed, request unload from controller
	{
		std::cout << std::endl << get_millis() << " SDRunoPlugin_TXLinkForm:: nana form exit clicked. UnloadInProgress = " << std::boolalpha << UnloadInProgress << std::endl;
		if (!UnloadInProgress) { m_parent.RequestDllUnload(); }								//ask SDRUno to unload this plugin if not already in progress
		arg.cancel = true;							// Now, cancel the unload. This prevents the form from closing so saves and clean up happens from the top.
	});				

	//Initialize header bar
	header_bar.size(nana::size(122, 20));
	header_bar.load(img_header, nana::rectangle(0, 0, 122, 20));
	header_bar.stretchable(0, 0, 0, 0);
	header_bar.move(nana::point((formWidth / 2) - 61, 5));
	header_bar.transparent(true);

	//Initial header text 
	title_bar_label.size(nana::size(65, 12));
	title_bar_label.move(nana::point((formWidth / 2) - 5, 9));
	title_bar_label.format(true);
	title_bar_label.caption("< bold size = 6 color = 0x000000 font = \"Verdana\">TXlink</>");
	title_bar_label.text_align(nana::align::center, nana::align_v::center);
	title_bar_label.fgcolor(nana::color_rgb(0x000000));
	title_bar_label.transparent(true);

	//Iniitialize drag_label
	form_drag_label.move(nana::point(0, 0));
	form_drag_label.transparent(true);

	//Initialize dragger and set target to form, and trigger to drag_label 
	form_dragger.target(*this);
	form_dragger.trigger(form_drag_label);

	//Initialise the "Minimize button"
	min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15));
	min_button.bgcolor(nana::color_rgb(0x000000));
	min_button.move(nana::point(formWidth - 51, 9));
	min_button.transparent(true);
	min_button.events().mouse_down([&] { min_button.load(img_min_down, nana::rectangle(0, 0, 20, 15)); });
	min_button.events().mouse_up([&] { min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15)); nana::API::zoom_window(this->handle(), false); });
	min_button.events().mouse_leave([&] { min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15)); });

	//Initialise the "Close button"
	close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15));
	close_button.bgcolor(nana::color_rgb(0x000000));
	close_button.move(nana::point(formWidth - 26, 9));
	close_button.transparent(true);
	close_button.events().mouse_down([&] { close_button.load(img_close_down, nana::rectangle(0, 0, 20, 15)); });
	close_button.events().mouse_up([&] { close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15)); close(); });
	close_button.events().mouse_leave([&] { close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15)); });

	//Uncomment the following block of code to Initialise the "Setting button"
	sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15));
	sett_button.bgcolor(nana::color_rgb(0x000000));
	sett_button.move(nana::point(10, 9));
	sett_button.events().mouse_down([&] { sett_button.load(img_sett_down, nana::rectangle(0, 0, 40, 15)); });
	sett_button.events().mouse_up([&] { sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15)); SettingsButton_Click(); });
	sett_button.events().mouse_leave([&] { sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15)); });
	sett_button.tooltip("Show settings window");
	sett_button.transparent(true);

	versionLbl.fgcolor(nana::colors::white);
	versionLbl.caption(VERSION);
	versionLbl.transparent(true);


	// TODO: Form code starts here
	COMportTb.multi_lines(false);
	COMportTb.events().text_changed([&] {
		string strCOM = COMportTb.caption();
		strCOM = COMportPrefix + strCOM;
		Serial.ReOpenPort(strCOM, COM_BAUD_RATE, false);
		});

	string strCOM = LoadCOM();
	if (strCOM == "")
	{
		strCOM = "COM5";			//default if not in ini file
	}
	// and reset serial
	COMportTb.caption(strCOM);		//this triggers event and opens/resets port on startup

	LoadSplitMode();		//load split mode from ini file, used to know if first UDP call changes to data mode.
	SplitMode == SplitNone ? FixFreqBtn.caption("Split") : FixFreqBtn.caption("UnSplit");
	FixFreqBtn.edge_effects(true);
	FixFreqBtn.events().click([&]
		{
			FixFreqBtnClicked();
		});


	FixTXupBtn.caption("TX UP");
	FixTXupBtn.edge_effects(true);
	FixTXupBtn.events().click([&]
		{
			FixTXup();
		});

	ResetRXFreqBtn.caption("RX<-TX");
	ResetRXFreqBtn.edge_effects(true);
	ResetRXFreqBtn.events().click([&]
		{
			SetRXFreq();
		});

	SwapTxRxBtn.caption("Swap");
	SwapTxRxBtn.edge_effects(true);
	SwapTxRxBtn.events().click([&]
		{
			SwapTxRxFreq();
		});

	RITchkbox.fgcolor(nana::color_rgb(forecolor));
	RITchkbox.transparent(true);
	RITchkbox.caption("RIT");
	RITchkbox.check(LoadRIT() == "true");

	StandbyBtn.caption("Set TX on");
	StandbyBtn.edge_effects(true);
	StandbyBtn.events().click([&]
		{
			if (!StandbyMode)
			{
				StandbyBtn.caption("Set TX on");
				StandbyMode = true;
			}
			else
			{
				StandbyBtn.caption("Set TX off");
				StandbyMode = false;
				TXtempLbl.fgcolor(nana::colors::white);
			}
			SetStandbyMode(StandbyMode);
		});
	SetStandbyMode(StandbyMode);

	UNOhwnd = FindTargetWindow(L"SDRuno Main 0", true);							//find UNOhwnd for main SDRuno window

	// Load X and Y location for the form from the ini file (if exists)
	int posX = LoadX();
	int posY = LoadY();
	if (posX >= 0 && posY >= 0)
	{
		move(posX, posY);
	}
	


	// TODO: Extra Form code goes here
	freqTb.bgcolor(nana::colors::white);
	freqTb.borderless(false);
	freqTb.text_align(nana::align::center);

	if (Serial.Connected()) {
		COMportTb.bgcolor(nana::colors::green);
		COMportTb.fgcolor(nana::colors::white);
	}
	SendRXfreq();

	freqTbLbl.caption("TX freq Hz");
	freqTbLbl.fgcolor(nana::color_rgb(forecolor));
	freqTbLbl.transparent(true);

	FT8Lbl.format(true);
	FT8Lbl.caption("<bold=true size=10>" + DigiTXmodeStr + "</>");		//"<bold size = 12>" + strTXfreq + "< / >"
	FT8Lbl.fgcolor(nana::color_rgb(forecolor));
	FT8Lbl.transparent(true);
	FT8txOnLbl.caption("");
	FT8txOnLbl.bgcolor(nana::colors::light_grey);
	FT8msgLbl.caption("");
	FT8msgLbl.fgcolor(nana::color_rgb(forecolor));
	FT8msgLbl.transparent(true);

	TXlevelLbl.caption("TX Level %");
	TXlevelLbl.fgcolor(nana::color_rgb(forecolor));
	TXlevelLbl.transparent(true);
	TXleveldBLbl.caption("-99dB");
	TXleveldBLbl.fgcolor(nana::color_rgb(forecolor));
	TXleveldBLbl.transparent(true);

	TXtempLbl.caption("? degC");
	TXtempLbl.fgcolor(nana::color_rgb(forecolor));
	TXtempLbl.transparent(true);

	TXlevelTb.multi_lines(false);
	TXlevelTb.events().text_changed([&] {
		string strTXlevel = TXlevelTb.caption();
		SendTXlevel(strTXlevel);
		});

	string strTXlevel = LoadTXlevel();		//get stored ini file
	if (strTXlevel == "")
	{
		strTXlevel = "0";			//default if not in ini file
	}

	PAlevelLbl.caption("Ext PA Drive %");
	PAlevelLbl.fgcolor(nana::color_rgb(forecolor));
	PAlevelLbl.transparent(true);
	PAleveldBLbl.caption("-99dB");
	PAleveldBLbl.fgcolor(nana::color_rgb(forecolor));
	PAleveldBLbl.transparent(true);
	PAchkbox.fgcolor(nana::color_rgb(forecolor));
	PAchkbox.transparent(true);
	PAchkbox.caption("QRO");

	PAlevelTb.multi_lines(false);
	PAlevelTb.events().text_changed([&] {
		string strPAlevel = PAlevelTb.caption();
		SendPAlevel(strPAlevel);
		SendPAstate(PAchkbox.checked());
		});
	PAchkbox.events().click([&] {
		SendPAstate(PAchkbox.checked());
		string strPAlevel = PAlevelTb.caption();
		SendPAlevel(strPAlevel);
		});

	MuteTXchkbox.fgcolor(nana::color_rgb(forecolor));
	MuteTXchkbox.transparent(true);
	MuteTXchkbox.caption("Mute TX");
	MuteTXchkbox.check(true);
	MuteTXchkbox.events().click([&] {
		SetMuteTX(MuteTXchkbox.checked());
		});

	string strPAlevel = LoadPAlevel();		//get stored ini file
	if (strPAlevel == "")
	{
		strPAlevel = "0";			//default if not in ini file
	}

	//SWR row
	SWRFWDlbl.caption("FWD");
	SWRFWDlbl.fgcolor(nana::color_rgb(forecolor));
	SWRFWDlbl.transparent(true);
	SWRFWDlbl.text_align(nana::align::right);
	SWRFWDtxt.bgcolor(nana::colors::white);
	SWRFWDtxt.fgcolor(nana::colors::black);
	SWRFWDtxt.borderless(false);
	SWRFWDtxt.text_align(nana::align::right);

	SWRREFlbl.caption("REF");
	SWRREFlbl.fgcolor(nana::color_rgb(forecolor));
	SWRREFlbl.transparent(true);
	SWRREFlbl.text_align(nana::align::right);
	SWRREFtxt.bgcolor(nana::colors::white);
	SWRREFtxt.fgcolor(nana::colors::black);
	SWRREFtxt.borderless(false);
	SWRREFtxt.text_align(nana::align::right);

	SWRSWRlbl.caption("SWR");
	SWRSWRlbl.fgcolor(nana::color_rgb(forecolor));
	SWRSWRlbl.transparent(true);
	SWRSWRlbl.text_align(nana::align::right);
	SWRSWRtxt.bgcolor(nana::colors::white);
	SWRSWRtxt.fgcolor(nana::colors::black);
	SWRSWRtxt.borderless(false);
	SWRSWRtxt.text_align(nana::align::right);

	AtuBtn.caption("T");
	AtuBtn.edge_effects(true);
	bATUformOpen = false;
	AtuBtn.events().click([&]
		{
			AtuBtnClicked();
		});

	DXspotBtn.caption("DX");
	DXspotBtn.edge_effects(true);
	DXspotBtn.events().click([&]
		{
			DXspotBtnClicked();
		});

	nLastCall = -1;
	for (int i = 0; i < nCalls; i++)
	{
		strLastTenCalls[i] = "";
	}


	// set text boxes to last saved defaults from SDRuno ini file
	TXlevelTb.caption(strTXlevel);		//this triggers event and opens/resets port on startup
	PAlevelTb.caption(strPAlevel);		//this triggers event and opens/resets port on startup

	m_timer.interval(std::chrono::milliseconds(50));
	// this next call sets the code to be executed after every interval
	m_timer.elapse([&]
		{
			m_timer.stop();		//stop timer while processing to prevent re-entrancy
			OnTimer();
			m_timer.start();	//restart timer
		});

	m_timer.start();


	//Note initial values sent to TXlink Arduino in ProcessIncomingSerial() which detects startup message from it
}

string SDRunoPlugin_TXLinkForm::GetCOM()
{
	return COMportTb.caption();
}


string SDRunoPlugin_TXLinkForm::GetTXlevel()
{
	return TXlevelTb.caption();
}

string SDRunoPlugin_TXLinkForm::GetPAlevel()
{
	return PAlevelTb.caption();
}

string SDRunoPlugin_TXLinkForm::GetRIT()
{
	string ret = RITchkbox.checked() ? "true" : "false";
	return ret;
}

void SDRunoPlugin_TXLinkForm::SetRIT(string RIT)
{
	RITchkbox.check(RIT == "true");
}

void SDRunoPlugin_TXLinkForm::SetStandbyMode(bool Mode)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{D}";
		if (!Mode)
		{
			//make sure TX levels and QRO mode are set correctly
			string strTXlevel = TXlevelTb.caption();
			SendTXlevel(strTXlevel);
			string strPAlevel = PAlevelTb.caption();
			SendPAlevel(strPAlevel);
			SendPAstate(PAchkbox.checked());
			str = "{U}";
		}
		is_sent = Serial.WriteSerialPort(str);
	}
}

void SDRunoPlugin_TXLinkForm::OnTimer()
{
	m_timerCount++;
	static bool busy = false;
	if (busy) {
		std::cout << get_millis() << "Form::OnTimer Event already BUSY - IGNORED CALL" << endl;
			return;	//should never happen as elapse is not re-entrant
	}
	busy = true;

	if (m_parent.FormPrepareUnloadRequest)				//request from TXLink destructor - THIS NEEDS TO HAPPEN FIRST
	{
		std::cout << get_millis() << " SDRunoPlugin_TXLinkForm::m_timer.elapse - FormPrepareUnloadRequest" << std::endl;
		UnloadInProgress = true;						//set unload in progress flag
		SaveIniData();								//save ini data before closing
		InitiateFormUnload();						//request form unload
		CloseCOMPort();									//COM to TX no longer needed
		m_parent.FormPrepareUnloadRequest = false;		//clear the request flag to signal complete to Ui
		busy = false;									// allow another tick to complete close down
		return;											//skip the rest of the code in this elapse function
	}
	if (UnloadInProgress)								//prep already done for final unload			
	{
		if (m_parent.FormExitAllRequest && !BackFromExitRequest)				////request from TXLinkUI destructor
		{
			std::cout << get_millis() << " SDRunoPlugin_TXLinkForm::m_timer.elapse - FormExitAllRequest" << std::endl;
			nana::API::exit_all();						//close the nana UI
			std::cout << get_millis() << " SDRunoPlugin_TXLinkForm::m_timer.elapse - back from nana::API::exit_all()" << std::endl;
			BackFromExitRequest = true;				//set flag to signal back from exit request
			m_parent.FormExitAllRequest = false;		//clear the request flag to signal complete to Ui
			busy = false;								// allow another tick to complete close down
			return;									//another tick never happens after nana::API::exit_all();
		}
		if (BackFromExitRequest)						//shouldnt get here as exit_all() turns off timer too
		{
			std::cout << get_millis() << " SDRunoPlugin_TXLinkForm::m_timer.elapse - BackFromExitRequest - SHOULD NOT GET HERE" << std::endl;
			return;									//skip the rest of the code in this elapse function if UnloadInProgress
		}
		//std::cout << get_millis() << " SDRunoPlugin_TXLinkForm::m_timer.elapse - UnloadInProgress - waiting for ExitAllRequest" << std::endl;
		busy = false;								// allow another tick to complete close down
		return;										//skip the rest of the code in this elapse function if UnloadInProgress
	}

	CheckWindowState();					//check if main SDRuno window minimized or restored

	//update TX Freq control
	string strTXfreq = std::to_string((int64_t)CurrentTXfreq);
	//update Plugin label to show TXfreq
	//add dots to show 1000's in Plugin label
	for (int i = strTXfreq.length(); i > 3; i = i - 3)
	{
		strTXfreq = strTXfreq.substr(0, i - 3) + "." + strTXfreq.substr(i - 3);
	}
	freqTb.format(true);
	freqTb.caption("<bold size=12>" + strTXfreq + "</>");
	//freqTb.caption(strTXfreq);

	bRITchkbox = RITchkbox.checked();		//get RIT checkbox state

	ProcessMessageQueue();					//process any external messages in the queue

	// look for incoming serial and UDP on timer event
	ProcessIncomingSerial();
	ProcessUDPEvent();

	busy = false;
}

void SDRunoPlugin_TXLinkForm::CheckWindowState()
{
	WINDOWPLACEMENT wd{};
	wd.length = sizeof(WINDOWPLACEMENT);
	wd.flags = 0;
	UINT nCurShow;
	bool UNOvisible = true;
	if (GetWindowPlacement(UNOhwnd, &wd))
	{
		nCurShow = wd.showCmd;
		if (nCurShow == SW_SHOWMINIMIZED) { UNOvisible = false; }
	}
	if (UNOvisible)
	{
		//normal
		show();
	}
	else
	{
		hide();
	}
}

void SDRunoPlugin_TXLinkForm::ProcessMessageQueue()
{
	// Process msgs to be sent to hardware

	std::string message = "";
	bool bMsgDone = false;
	while (!bMsgDone)
	{
		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			if (m_messageQueue.empty()) {
				bMsgDone = true;
			}
			else
			{
				message = m_messageQueue.front();
				m_messageQueue.pop();
				bMsgDone = false;
			}
		}

		if (bMsgDone) { return; }

		// special cases
		if (message == "$SendRXfreq$")
		{
			SendRXfreq();
			return;
		}
		else if (message == "$SendTXfreq$")
		{
			SendTXfreq();
			return;
		}
		else if (message == "$SaveIniData$")
		{
			SaveIniData();
			return;
		}
		else
		{
			if (message.rfind("RTTY:", 0) == 0) { // Command starts with "RTTY:"
				std::string rttyCmdText = message.substr(5); // Extract text after "RTTY:"
				message = ProcessRTTYcommand(rttyCmdText);
			}

			// Send serial message string to Arduino
			_SendSerialMsg(message);
		}
	}
}


void SDRunoPlugin_TXLinkForm::ProcessIncomingSerial()
{
	// called from timer event. Read any data while available, uses {} as begin/end message
	// also send FT8 event data

	string NextChar = "";
	string NewMsg = "";
	static bool VRX0mute = false;
	static bool VRX1mute = true;

	while (Serial.DataAvailable() > 0)
	{
		NextChar = Serial.ReadSerialPortChar();
		//valid read
		if (NextChar == "{")
		{
			NewMsg = "";
		}
		else if (NextChar == "}")
		{
			//execute compiled message
			if (NewMsg == "")					//null command or an error
			{
				//do nothing for now
			}
			else if (NewMsg == "~TX")					//init message from TXlink Arduino
			{
				//so send all the initial data to TXlink Arduino
				if (Serial.Connected()) {		
					//force a freq update here as it can be 20-30seconds after DLL load before TXlink Arduino resets and sends ~TX
					string str = "{F" + std::to_string((int64_t)CurrentTXfreq) + "}";
					std::cout << get_millis() << " SDRunoPlugin_TXLinkForm::ProcessIncomingSerial() - Arduino ~TX received - init TXfreq = " << str << std::endl;
					Serial.WriteSerialPort(str);
					RTTYmsgOutQ.push(str);
					//TODO send to RTTYpipe
				}
				SendFreqCalToTX();
				SendTXlevel(TXlevelTb.caption());
				SendPAlevel(PAlevelTb.caption());
				SendPAstate(false);					//always start with ext PA off for safety
				SendFanHold(GetValidFanHold());
				SendFanLowPWM(GetValidFanLowPWM());
				SendFT8txDelay(GetValidFT8txDelay());
				SendTempLimit(GetValidTempLimit());
				SetMuteTX(true);					//enable vrx1 mute as default

			}
			else if (NewMsg.substr(0, 1) == "M" && DigiTXmodeStr != "TTY")
			{
				VRX0mute = m_controller.GetAudioMute(0);
				VRX1mute = m_controller.GetAudioMute(1);
				m_controller.SetAudioMute(0, true);
				m_controller.SetAudioMute(1, true);
			}
			else if (NewMsg.substr(0, 1) == "N")
			{
				m_controller.SetAudioMute(0, VRX0mute);
				m_controller.SetAudioMute(1, VRX1mute);
			}
			else if (NewMsg.substr(0, 1) == "F")
			{
				SWRFWDtxt.caption(NewMsg.substr(1));
				//SWRFWDtxt.caption("123");
				//SWRFWDlbl.caption("123");
			}
			else if (NewMsg.substr(0, 1) == "R")
			{
				SWRREFtxt.caption(NewMsg.substr(1));
			}
			else if (NewMsg.substr(0, 1) == "S")
			{
				SWRSWRtxt.caption(NewMsg.substr(1));
			}
			else if (NewMsg.substr(0, 1) == "T")
			{
				SetTXtemp(NewMsg.substr(1));
				TXtempLbl.caption(NewMsg.substr(1) + " degC");
			}
			else if (NewMsg.substr(0, 1) == "D")				//force TX off due to TXtempLimit exceeded
			{
				StandbyBtn.caption("Set TX on");
				StandbyMode = true;
				TXtempLbl.fgcolor(nana::colors::red);
			}
			NewMsg = "";		//message completed
		}
		else
		{
			NewMsg.append(NextChar);
		}
		NextChar = "";
	}
}

std::string SDRunoPlugin_TXLinkForm::ProcessRTTYcommand(std::string rttyCmdText)
{
	// Process command message string from RTTY server.
	// eg {M1}{Zrtty message to transmit}{X1}
	// use the {Mx} to set mode, {Z} to set message, {X1} to start TX, {X0} to stop TX

	if (rttyCmdText.rfind("{M1}", 0) >= 0) {				//set DigiTXmodeStr and UI mode Lbl to RTTY
		DigiTXmodeStr = "TTY";							//only space for 3 chars in Lbl
		FT8Lbl.format(true);
		FT8Lbl.caption("<bold=true size=10>" + DigiTXmodeStr + "</>");    // FT8Lbl.caption(WSJTX_TXmodeStr);		//FT8Lbl.caption(< bold size = 12>" + "FT8" + " < / > ");
		FT8txOnLbl.bgcolor(nana::colors::red);
	}

	return rttyCmdText;
}


void SDRunoPlugin_TXLinkForm::ProcessUDPEvent()
{													//called from Form timer event to process wsjtx UDP	
	//WSJT_X FT8 UDP request?

	//std::lock_guard<std::mutex> lock(UDPEvent_mtx);                     // Locks the UDPEvent mutex
	if (UDPevent)									//set from UDP thread when wsjtx message received
	{
		//following logic to handle correct mode restore on startup
		static bool FirstTime = true;				//first time in this function so maybe ignore wsjtx for now?
		if (FirstTime && SplitMode != SplitFT8)     //only reset datamode when SplitModeFT8 has been restored on startup
		{
			if (newFrequency > 0) { newFrequency = 0; FirstTime = false; }			//wait until wsjtx first changes freq to set data mode if datamode was not restored on startup
			UDPevent = false;
			return; 
		}		

		FirstTime = false;		//setup data mode on first UDP event only when SplitFT8 restored in data mode

		//std::cout << "SDRunoPlugin_TXLinkForm::ProcessIncomingSerial() UDPevent = true" << std::endl;
		bool is_sent = false;
		long TXtimeToEnd = 0;
		if (Serial.Connected())
		{
			//always update freq when changed in wsjtx
			if (newFrequency > 0)			//user changed band / mode in wsjtx
			{
				CurrentWSJTXfreq = static_cast<double>(newFrequency);					//local form master setting
				newFrequency = 0;									//handshake for server; 
				SetupDataMode();
			}

			std::string str = "{X0}";
			if (TXflg)
			{
				//refresh TX data and enable TX as required
				str = "{M8}";
				if (WSJTX_TXmodeStr == "FT4") { str = "{M4}"; }
				is_sent = Serial.WriteSerialPort(str);

				str = "{Y" + to_string(TXDF) + "}";
				is_sent = Serial.WriteSerialPort(str);

				str = "{Z" + TXsymbols + "}";
				is_sent = Serial.WriteSerialPort(str);

				if (strDecodeMsg != "TUNE")
				{
					TXtimeToEnd = millisToEnd();
				}
				else
				{
					TXtimeToEnd = 30000;		//TUNE for 30 seconds from now
				}
				str = "{W" + to_string(TXtimeToEnd) + "}";
				is_sent = Serial.WriteSerialPort(str);

				// TX key on message
				str = "{X1}";							//if (TXtimeToEnd >= 12000) { str = "{X1}"; }		//only enable if enough time left
			}
			//indicate FT8 tx on UI form
			if (str == "{X0}")
			{
				FT8txOnLbl.bgcolor(nana::colors::light_grey);
			}
			else           //FT8 TX
			{
				DigiTXmodeStr = WSJTX_TXmodeStr;		//save current TX mode
				FT8Lbl.format(true);
				FT8Lbl.caption("<bold=true size=10>" + DigiTXmodeStr + "</>");    // FT8Lbl.caption(WSJTX_TXmodeStr);		//FT8Lbl.caption(< bold size = 12>" + "FT8" + " < / > ");
				FT8txOnLbl.bgcolor(nana::colors::red);
				
				SetupDataMode();
			}

			if (UDPHaltTXevent) {
				TXtimeToEnd = 0;
				str = "{X0}{W0}";					//force TX to stop
				UDPHaltTXevent = false;
			}

			//send TX last so freq & message updated first in Arduino
			is_sent = true;
			if (SplitMode == SplitFT8 && (DigiTXmodeStr == "FT4" || DigiTXmodeStr == "FT8")) { is_sent = Serial.WriteSerialPort(str); }

		}
		//if (is_sent) { UDPevent = false; }
		ShowIsSent(is_sent);
		FT8msgLbl.caption(strDecodeMsg + " - " + to_string(TXtimeToEnd));

		if (newDXcall != "" && !bLastTenCalls(newDXcall))
		{
			std::string strURL = "https://www.qrz.com/db/" + newDXcall;
			//std::string command = "start /b chrome \"" + strURL + "\"";
			//system(command.c_str());

			ShellExecute(
				NULL,                    // No parent window
				ConvertToLPWSTR("open"),                  // Operation to perform
				ConvertToLPWSTR("chrome.exe"),            // Application to open (assuming chrome.exe is in PATH)
				ConvertToLPWSTR(strURL.c_str()),             // Parameters (the URL)
				NULL,                    // Default directory
				SW_SHOWNORMAL            // Show command (can be SW_HIDE, SW_MINIMIZE, etc.)
			);
		}
		newDXcall = "";
		UDPevent = false;
		//std::cout << "SDRunoPlugin_TXLinkForm::ProcessIncomingSerial() UDPevent = false" << std::endl;
	}
}

void SDRunoPlugin_TXLinkForm::SetupDataMode()
{
	freqTb.bgcolor(nana::colors::light_salmon);
	FT8Lbl.fgcolor(nana::colors::black);
	FT8Lbl.bgcolor(nana::colors::light_salmon);
	FT8Lbl.transparent(false);
	SplitMode = SplitFT8;
	// and set FT8 RX in vrx0 & vrx1
	if (CurrentTXfreq != CurrentWSJTXfreq)				//make sure freq is set
	{
		CurrentTXfreq = CurrentWSJTXfreq;
		SetRXFreq();									//sets both vfos to CurrentTXfreq
		SendTXfreq();									//update TX hardware
	}
	CurrentRXfreq = CurrentTXfreq + TXDF;
	/*
	if (CurrentTXfreq > 10000000 && CurrentTXfreq < 11000000)
	{
		m_controller.SetCenterFrequency(0,  CurrentTXfreq);
		m_controller.SetSampleRate(0, (double) 62500.0);
		m_controller.SetCenterFrequency(0, CurrentTXfreq);
		m_controller.SetSampleRate(1, (double) 62500.0);
	}
	else
	{
		m_controller.SetSampleRate(0, (double) 111111.0);
		m_controller.SetSampleRate(1, (double) 111111.0);
	}
	*/
	m_controller.SetVfoFrequency(0, CurrentTXfreq);
	m_controller.SetVfoFrequency(1, CurrentRXfreq);
	m_parent._updateTX_AnnoEntry("FT8Base", "", (int64_t)CurrentTXfreq);
	m_parent._updateRX_AnnoEntry("FTX", "", (int64_t)CurrentRXfreq);
	m_controller.SetDemodulatorType(0, m_controller.DemodulatorUSB);
	FixFreqBtn.caption("UnSplit");
	if (TXmodeChange)
	{
		int bndw = FilterBandwidthFT8;
		if (DigiTXmodeStr == "FT4") { bndw = FilterBandwidthFT4; }
		m_controller.SetFilterBandwidth(0, bndw);
		TXmodeChange = false;
	}
}

bool SDRunoPlugin_TXLinkForm::bLastTenCalls(string DXcall)
{
	// look for recent DXcalls
	bool ret = false;			//not found

	if (DXcall != "")
	{
		for (int i = 0; i < nCalls; i++)
		{
			if (strLastTenCalls[i] == DXcall)
			{
				ret = true;
				break;
			}
		}
		if (!ret)
		{
			//call not found so insert new one at next location
			if (++nLastCall >= nCalls)			//nLastCall == -1 means empty
			{
				nLastCall = 0;
			}
			strLastTenCalls[nLastCall] = DXcall;
		}
	}
	return false;		// return ret;
}

void SDRunoPlugin_TXLinkForm::SendRXfreq()
{

	if (TrackFreq || freqTb.caption() == "")
	{
		//update TX freq fron VRX0 freq change
		double tmp = m_controller.GetVfoFrequency(0);
		if (tmp > 0)
		{
			CurrentTXfreq = tmp;
			CurrentRXfreq = tmp;
		}
	}
	SendTXfreq();
}

std::mutex mtx2;
void SDRunoPlugin_TXLinkForm::SendTXfreq()
{
	// called from timer event msg queue, sends current TX freq to TXlink Arduino
	bool is_sent = false;
	string strTXfreq = "";
	std::lock_guard<std::mutex> lock(mtx2);

	// following needed to avoid sending same freq multiple times
	// by the time it gets here the freq may have stabalised in tuner thread, so only send if different.
	static double LastTXfreq = -1.0;		//static so only set once per call
	if (CurrentTXfreq == LastTXfreq) { return; }		//no change so skip sending
	LastTXfreq = CurrentTXfreq;				//set last TX freq to current

	strTXfreq = std::to_string((int64_t)CurrentTXfreq);

	// send current TX freq to TX
	if (!Serial.Connected())
	{
		//attempt to restart COM port
		string strCOM = COMportTb.caption();
		strCOM = COMportPrefix + strCOM;
		Serial.ReOpenPort(strCOM, COM_BAUD_RATE, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(250));		//wait for Arduino to restart
	}
	if (Serial.Connected()) {

		std::string str = "{F" + strTXfreq + "}";
		is_sent = Serial.WriteSerialPort(str);
		RTTYmsgOutQ.push(str);
		std::cout << str << std::endl;
	}

	ShowIsSent(is_sent);				//animate form button to show if sent ok

	//update Plugin label to show TXfreq
	//add dots to show 1000's in Plugin label
	for (int i = strTXfreq.length(); i > 3; i = i - 3)
	{
		strTXfreq = strTXfreq.substr(0, i - 3) + "." + strTXfreq.substr(i - 3);
	}
	//freqTb.format(true);
	//freqTb.caption("<bold size=12>" + strTXfreq + "</>");				//TXfreq + "< bold size = 14 color = 0x000000 font = \"Verdana\"></>");
	//freqTb.caption(strTXfreq);

	if (SplitMode != SplitFT8)
	{
		m_parent._updateTX_AnnoEntry("TX", "", (int64_t)CurrentTXfreq);
		m_parent._updateRX_AnnoEntry("RX", "", (int64_t)CurrentRXfreq);
	}
	else
	{
		m_parent._updateTX_AnnoEntry("FT8base", "", (int64_t)CurrentTXfreq);
		m_parent._updateRX_AnnoEntry("FTX", "", (int64_t)CurrentRXfreq);
	}

	double vtxFreq = m_controller.GetVfoFrequency(1);
	if (vtxFreq != CurrentTXfreq && vtxFreq > 0 && SplitMode != SplitFT8)
	{
		m_controller.SetVfoFrequency(1, CurrentTXfreq);					// force vrx1 to track TX freq
	}
}



void SDRunoPlugin_TXLinkForm::ShowIsSent(bool is_sent)
{
	//return;
	if (is_sent) {
		if (COMportTb.bgcolor() == nana::colors::green)
		{
			COMportTb.bgcolor(nana::colors::blue);
			COMportTb.fgcolor(nana::colors::white);
		}
		else
		{
			COMportTb.bgcolor(nana::colors::green);
			COMportTb.fgcolor(nana::colors::white);
		}
	}
	else
	{
		//Port not open or write failed
		COMportTb.bgcolor(nana::colors::red);
		COMportTb.fgcolor(nana::colors::white);
		std::cout << "SDRunoPlugin_TXLinkForm::ShowIsSent() - Serial port not connected or write failed" << std::endl;
	}
}

void SDRunoPlugin_TXLinkForm::SendTXlevel(string strTXlevel)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{L" + strTXlevel + "}";
		is_sent = Serial.WriteSerialPort(str);
	}
	if (is_sent) {
		TXlevelTb.bgcolor(nana::colors::white);
	}
	else
	{
		TXlevelTb.bgcolor(nana::colors::red);
	}

	//display dB rel full output
	string strDB = "";
	double dB = strToDouble(strTXlevel);
	dB = abs(dB);
	if (dB == 0)
	{
		dB = -99;
	}
	else
	{
		if (dB > 100)
		{
			dB = 100;
		}
		dB = 20 * log10(dB / 100);
		dB = int(dB);
	}
	strDB = to_string(dB);

	//only need upto decimal
	strDB = strDB.substr(0, strDB.find(".", 0));
	TXleveldBLbl.caption(strDB + "dB");
}

void SDRunoPlugin_TXLinkForm::SendPAlevel(string strPAlevel)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{P" + strPAlevel + "}";
		is_sent = Serial.WriteSerialPort(str);
	}
	if (is_sent) {
		PAlevelTb.bgcolor(nana::colors::white);
	}
	else
	{
		PAlevelTb.bgcolor(nana::colors::red);
	}

	//display dB rel full output
	string strDB = "";
	double dB = strToDouble(strPAlevel);
	dB = abs(dB);
	if (dB == 0)
	{
		dB = -99;
	}
	else
	{
		if (dB > 100)
		{
			dB = 100;
		}
		dB = 20 * log10(dB / 100);
		dB = int(dB);
	}
	strDB = to_string(dB);

	//only need upto decimal
	strDB = strDB.substr(0, strDB.find(".", 0));
	PAleveldBLbl.caption(strDB + "dB");
}

void SDRunoPlugin_TXLinkForm::SendPAstate(bool PAstate)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string strState = "0";				//PA off
		if (PAstate) { strState = "1"; }	//PA on
		string str = "{Q" + strState + "}";
		is_sent = Serial.WriteSerialPort(str);
	}
}

void SDRunoPlugin_TXLinkForm::SendTXhold(string TXhold)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{H" + TXhold + "}";
		is_sent = Serial.WriteSerialPort(str);
	}
}

void SDRunoPlugin_TXLinkForm::SendFanHold(string FanHold)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{B" + FanHold + "}";
		is_sent = Serial.WriteSerialPort(str);
	}
}

void SDRunoPlugin_TXLinkForm::SendFanLowPWM(string FanLowPWM)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{R" + FanLowPWM + "}";
		is_sent = Serial.WriteSerialPort(str);
	}
}

void SDRunoPlugin_TXLinkForm::SendFT8txDelay(string FT8txDelay)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{V" + FT8txDelay + "}";
		is_sent = Serial.WriteSerialPort(str);
	}
}

void SDRunoPlugin_TXLinkForm::SendTempLimit(string TempLimit)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{T" + TempLimit + "}";
		is_sent = Serial.WriteSerialPort(str);
	}
}

void SDRunoPlugin_TXLinkForm::SetMuteTX(bool chk)
{
	m_controller.SetAudioMute(1, chk);
}

void SDRunoPlugin_TXLinkForm::SendFreqCalToTX()
{
	string tmp = "";
	m_controller.GetConfigurationKey("TXLink.freqCal", tmp);
	if (tmp.empty()) {
		tmp = "0";								//default
		std::lock_guard<std::mutex> l(m_lock);
		m_controller.SetConfigurationKey("TXLink.freqCal", tmp);
	}
	try {
		int CalVal = stoi(tmp);
		//only send if it can be converted to int
		bool is_sent = false;
		if (Serial.Connected())
		{
			string str = "{C" + tmp + "}";
			is_sent = Serial.WriteSerialPort(str);
		}
		SendRXfreq();				//to reset it????
	}
	catch (const std::invalid_argument&) {
		std::cerr << "Argument is invalid\n";
		//throw;
	}
	catch (const std::out_of_range&) {
		std::cerr << "Argument is out of range for a double\n";
		//throw;
	}
}

double SDRunoPlugin_TXLinkForm::strToDouble(string str)
{
	double d = 0;

	try {
		d = std::stod(str);
	}
	catch (const std::invalid_argument&) {
		std::cerr << "Argument is invalid\n";
		//throw;
	}
	catch (const std::out_of_range&) {
		std::cerr << "Argument is out of range for a double\n";
		//throw;
	}
	return d;
}

void SDRunoPlugin_TXLinkForm::CloseCOMPort()
{
	std::cout << get_millis() << "Request Close TxLink COM port" << std::endl;
	Serial.CloseSerialPort();
	std::cout << get_millis() << "TxLink COM port closed" << std::endl;
}

void SDRunoPlugin_TXLinkForm::FixFreqBtnClicked()
{
	// sets split mode back to normal from FT8 and toggles normal Split
	if (SplitMode != SplitNone)
	{
		FixFreqBtn.caption("Split");
		TrackFreq = true;
		SplitMode = SplitNone;
		freqTb.bgcolor(nana::colors::white);
		
		FT8Lbl.fgcolor(nana::colors::white);
		FT8Lbl.transparent(true);
		freqTb.fgcolor(nana::colors::black);
		m_controller.SetDemodulatorType(0, m_controller.DemodulatorCW);
		m_controller.SetFilterBandwidth(0, FilterBandwidthCW);
	}
	else
	{
		FixFreqBtn.caption("UnSplit");
		TrackFreq = false;
		SplitMode = SplitTxRx;
		freqTb.bgcolor(nana::colors::red);
		freqTb.fgcolor(nana::colors::white);
		FT8Lbl.fgcolor(nana::colors::white);
		FT8Lbl.transparent(true);
	}
	FixFreqButtonClicked();
}

void SDRunoPlugin_TXLinkForm::AtuBtnClicked()
{
	// Open Atu dialog - one only
	
	if (!bATUformOpen)
	{
		//Create a new settings dialog object
		//SDRunoPlugin_TemplateATUDialog AtuDialog{ this, m_controller };
		m_atuDialog = std::make_unique<SDRunoPlugin_TemplateATUDialog>(*this, m_controller);

		//disable this form so settings dialog retains top level focus
		//this->enabled(false);

		//Attach a handler to the settings dialog close event
		m_atuDialog->events().unload([&] { AtuDialog_Closed(); });
		bATUformOpen = true;
		
		//Show the setttings dialog
		m_atuDialog->Run();
	}
}

void SDRunoPlugin_TXLinkForm::AtuDialog_Closed()
{
	//DO NOT REMOVE THE FLLOWING CODE it is required for the proper operation of the settings dialog form

	this->enabled(true);
	this->focus();
	bATUformOpen = false;

	//TODO: Extra code goes here to be preformed when settings dialog form closes
}

void SDRunoPlugin_TXLinkForm::DXspotBtnClicked()
{
	// Derive a DXspot message and send to DXspider
	// spot message format: "DX <frequency> <dx_call> <notes>"
	std::cout << get_millis() << "DXspotBtnClicked: Request DXspot" << std::endl;

	std::string strDXspot = "";
	if (CurrentWSJTXfreq > 0)
	{
		strDXspot += "dx " + std::to_string((int64_t)CurrentWSJTXfreq / 1000) + ".0";
	}
	else
	{
		std::cout << get_millis() << "DXspotBtnClicked: No TXfreq" << "\n";
		return;
	}

	if (DXcall != "")
	{
		strDXspot += " " + DXcall;

		//remove "<" and ">" from DXcall
		char charToRemove = '<';
		strDXspot.erase(std::remove(strDXspot.begin(), strDXspot.end(), charToRemove), strDXspot.end());
		charToRemove = '>';
		strDXspot.erase(std::remove(strDXspot.begin(), strDXspot.end(), charToRemove), strDXspot.end());
	}
	else
	{
		std::cout << get_millis() << "DXspotBtnClicked: No DXcall" << std::endl;
		return;
	}

	if (DigiTXmodeStr != "")
	{
		strDXspot += " " + DigiTXmodeStr;
	}
	else
	{
		std::cout << get_millis() << "DXspotBtnClicked: No DigiTXmodeStr" << "\n";
		return;
	}

	if (strReport != "")
	{
		strDXspot += " " + strReport + "dB";
	}
	else
	{
		std::cout << get_millis() << "DXspotBtnClicked: No Report" << "\n";
		//return;
	}

	if (RxDF != 0)
	{
		strDXspot += " " + std::to_string(RxDF) + "Hz";
	}
	else
	{
		std::cout << get_millis() << "DXspotBtnClicked: No RxDF" << "\n";
		return;
	}

	if (DXgrid != "")
	{
		strDXspot += " Loc: " + DXgrid;
	}
	else
	{
		std::cout << get_millis() << "DXspotBtnClicked: No DXgrid" << "\n";
	}

	strDXspot += " 73GL";

	std::cout << get_millis() << "DXspotBtnClicked: DXspot created: " << strDXspot << "\n";
	//DXspiderSpot("G4AHN-2", strDXspot);

	DXspot_t spot;
	spot.dxSpiderPort = GetSetConfigurationKey("TXLink.dxSpiderPort", "dxspider.co.uk:7300");			//"dxspider.co.uk:7300";
	spot.myCall = GetSetConfigurationKey("TXLink.myDXspotCall", "G4AHN-1");			//
	spot.DXspot = strDXspot;
	DXspiderSpotOutQ.push(spot);			//put spot on DXspider queue to send
}


//get setting from ini file and create a default if not already set
//
std::string SDRunoPlugin_TXLinkForm::GetSetConfigurationKey(std::string strItem, std::string strDefault)
{
	std::string tmp = "";
	m_controller.GetConfigurationKey(strItem, tmp);
	if (tmp == "") {
		tmp = strDefault;
		m_controller.SetConfigurationKey(strItem, tmp);
	}
	return tmp;
}


void SDRunoPlugin_TXLinkForm::FixTXup()
{
	TXupHz = GetValidTXupHz();
	CurrentTXfreq = CurrentTXfreq + TXupHz;			//increment every click
	if (TrackFreq)
	{	//take it out of tracking
		FixFreqBtn.caption("UnSplit");
		TrackFreq = false;
		SplitMode = SplitTxRx;
		freqTb.bgcolor(nana::colors::red);
		freqTb.fgcolor(nana::colors::white);
		FT8Lbl.fgcolor(nana::colors::white);
		FT8Lbl.transparent(true);
	}
	SendTXfreq();					//and update TX module
}

int SDRunoPlugin_TXLinkForm::GetValidTXupHz()
{
	string tmp = "";
	int Val = 500;
	m_controller.GetConfigurationKey("TXLink.UPfreq", tmp);			//stored by settings panel
	if (tmp.empty()) {
		tmp = "500";								//default
		Val = 500;
	}
	try {
		Val = stoi(tmp);
	}
	catch (const std::invalid_argument&) {
		Val = 500;
		tmp = "500";
		//std::cerr << "Argument is invalid\n";
		//throw;
	}
	catch (const std::out_of_range&) {
		Val = 500;
		tmp = "500";
		//std::cerr << "Argument is out of range for a double\n";
		//throw;
	}
	std::lock_guard<std::mutex> l(m_lock);
	m_controller.SetConfigurationKey("TXLink.UPfreq", tmp);
	return Val;
}

string SDRunoPlugin_TXLinkForm::GetValidTXhold()
{
	string tmp = "";
	int Val = 450;
	m_controller.GetConfigurationKey("TXLink.TXhold", tmp);			//stored by settings panel
	if (tmp.empty()) {
		tmp = "450";								//default
		Val = 450;
	}
	try {
		Val = stoi(tmp);
	}
	catch (const std::invalid_argument&) {
		Val = 450;
		tmp = "450";
		//std::cerr << "Argument is invalid\n";
		//throw;
	}
	catch (const std::out_of_range&) {
		Val = 450;
		tmp = "450";
		//std::cerr << "Argument is out of range for a double\n";
		//throw;
	}
	std::lock_guard<std::mutex> l(m_lock);
	m_controller.SetConfigurationKey("TXLink.TXhold", tmp);
	return tmp;
}

string SDRunoPlugin_TXLinkForm::GetValidFanHold()
{
	string tmp = "";
	int Val = 450;
	m_controller.GetConfigurationKey("TXLink.FanHold", tmp);			//stored by settings panel
	if (tmp.empty()) {
		tmp = "3";								//default
		Val = 3;
	}
	try {
		Val = stoi(tmp);
	}
	catch (const std::invalid_argument&) {
		Val = 3;
		tmp = "3";
		//std::cerr << "Argument is invalid\n";
		//throw;
	}
	catch (const std::out_of_range&) {
		Val = 3;
		tmp = "3";
		//std::cerr << "Argument is out of range for a double\n";
		//throw;
	}
	std::lock_guard<std::mutex> l(m_lock);
	m_controller.SetConfigurationKey("TXLink.FanHold", tmp);
	return tmp;
}

string SDRunoPlugin_TXLinkForm::GetValidFanLowPWM()
{
	string tmp = "";
	int Val = 175;
	m_controller.GetConfigurationKey("TXLink.FanLowPWM", tmp);			//stored by settings panel
	if (tmp.empty()) {
		tmp = "175";								//default
		Val = 175;
	}
	try {
		Val = stoi(tmp);
	}
	catch (const std::invalid_argument&) {
		Val = 175;
		tmp = "175";
		//std::cerr << "Argument is invalid\n";
		//throw;
	}
	catch (const std::out_of_range&) {
		Val = 175;
		tmp = "175";
		//std::cerr << "Argument is out of range for a double\n";
		//throw;
	}
	std::lock_guard<std::mutex> l(m_lock);
	m_controller.SetConfigurationKey("TXLink.FanLowPWM", tmp);
	return tmp;
}

string SDRunoPlugin_TXLinkForm::GetValidFT8txDelay()
{
	string tmp = "";
	int Val = 250;
	m_controller.GetConfigurationKey("TXLink.FT8txDelay", tmp);			//stored by settings panel
	if (tmp.empty()) {
		tmp = "250";								//default
		Val = 250;
	}
	try {
		Val = stoi(tmp);
		if (Val < 0 || Val >= 3000)
		{
			Val = 250;
			tmp = "250";
		}
	}
	catch (int errNum) {
		if (errNum) {};
		Val = 250;
		tmp = "250";
	}
	std::lock_guard<std::mutex> l(m_lock);
	m_controller.SetConfigurationKey("TXLink.FT8txDelay", tmp);
	return tmp;
}

string SDRunoPlugin_TXLinkForm::GetValidTempLimit()
{
	string tmp = "";
	int Val = 54;
	m_controller.GetConfigurationKey("TXLink.TempLimit", tmp);			//stored by settings panel
	if (tmp.empty()) {
		tmp = "54";								//default
		Val = 54;
	}
	try {
		Val = stoi(tmp);
		if (Val < 0 || Val >= 75)
		{
			Val = 54;
			tmp = "54";
		}
	}
	catch (int errNum) {
		if (errNum) {};
		Val = 54;
		tmp = "54";
	}
	std::lock_guard<std::mutex> l(m_lock);
	m_controller.SetConfigurationKey("TXLink.TempLimit", tmp);
	return tmp;
}

void SDRunoPlugin_TXLinkForm::SetRXFreq()
{
	m_controller.SetVfoFrequency(0, CurrentTXfreq);
	m_controller.SetVfoFrequency(1, CurrentTXfreq);
}

void SDRunoPlugin_TXLinkForm::SwapTxRxFreq()
{
	if (SplitMode == SplitTxRx)
	{
		//swap TX and RX freqs
		double VFO0 = m_controller.GetVfoFrequency(0);
		double VFO1 = m_controller.GetVfoFrequency(1);
		m_controller.SetVfoFrequency(0, VFO1);
		m_controller.SetVfoFrequency(1, VFO0);
	}
}

bool SDRunoPlugin_TXLinkForm::GetRITchkbox()
{
	return RITchkbox.checked();
}

void SDRunoPlugin_TXLinkForm::SettingsButton_Click()
{
	//Create a new settings dialog object
	SDRunoPlugin_TemplateSettingsDialog settingsDialog{ m_parent,m_controller};

	//disable this form so settings dialog retains top level focus
	this->enabled(false);

	//Attach a handler to the settings dialog close event
	settingsDialog.events().unload([&] { SettingsDialog_Closed(); });

	//Show the setttings dialog
	settingsDialog.Run();


}

void SDRunoPlugin_TXLinkForm::SettingsDialog_Closed()
{
	//DO NOT REMOVE THE FLLOWING CODE it is required for the proper operation of the settings dialog form

	this->enabled(true);
	this->focus();

	//TODO: Extra code goes here to be preformed when settings dialog form closes
	SendFreqCalToTX();
	SendTXhold(GetValidTXhold());
	SendFanHold(GetValidFanHold());
	SendFanLowPWM(GetValidFanLowPWM());
	SendFT8txDelay(GetValidFT8txDelay());
	SendTempLimit(GetValidTempLimit());
}


long SDRunoPlugin_TXLinkForm::millisToEnd() {
	//compute remaining milliseconds to end of FT8             TX period
	// Get the current time point
	auto currentTime = std::chrono::system_clock::now();

	// Extract seconds and fractional seconds since last minute
	auto seconds = std::chrono::time_point_cast<std::chrono::minutes>(currentTime);
	auto fraction = currentTime - seconds;

	// Calculate the milliseconds since the start of the current minute
	auto millisecondsInMinute = std::chrono::duration_cast<std::chrono::milliseconds>(fraction);

	// Print the milliseconds since the start of the current minute
	//std::cout << "Milliseconds since the start of the current minute: " << millisecondsInMinute.count() << std::endl;

	long millisToEnd = (59999 - millisecondsInMinute.count()) % 15000 - 170;			//remaining millisecs in 15 sec slot
	if (WSJTX_TXmodeStr == "FT4") { millisToEnd = (59999 - millisecondsInMinute.count()) % 7500 - 50; }   //7.5sec slot

	//TXtempLbl.caption(to_string(millisToEnd));

	return millisToEnd;
}

// used in ATU tune dialog to change settings
bool SDRunoPlugin_TXLinkForm::_SendSerialMsg(string Msg)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		is_sent = Serial.WriteSerialPort(Msg);
	}
	return is_sent;
}


// Load X from the ini file (if exists)
// TODO: Change Template to plugin name
int SDRunoPlugin_TXLinkForm::LoadX()
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
int SDRunoPlugin_TXLinkForm::LoadY()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.Y", tmp);
	if (tmp.empty())
	{
		return -1;
	}
	return stoi(tmp);
}

void SDRunoPlugin_TXLinkForm::SaveLocation()
{
	nana::point position = pos();
	if (position.x >= 0 && position.y >= 0)
	{
		m_controller.SetConfigurationKey("TXLink.X", std::to_string(position.x));
		m_controller.SetConfigurationKey("TXLink.Y", std::to_string(position.y));
	}

}

//Load COM port
string SDRunoPlugin_TXLinkForm::LoadCOM()
{
	std::string tmp = "COM5";
	m_controller.GetConfigurationKey("TXLink.COM", tmp);

	if (tmp.empty())
	{
		return "";
	}
	return tmp;
}

void SDRunoPlugin_TXLinkForm::SaveCOM(string strCOM)
{
	m_controller.SetConfigurationKey("TXLink.COM", strCOM);
}

//Load TXlevel
string SDRunoPlugin_TXLinkForm::LoadTXlevel()
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
string SDRunoPlugin_TXLinkForm::LoadPAlevel()
{
	std::string tmp = "";
	m_controller.GetConfigurationKey("TXLink.PAlevel", tmp);
	if (tmp.empty())
	{
		return "0";
	}
	return tmp;
}

void SDRunoPlugin_TXLinkForm::SaveTXlevel(string strTXlevel)
{
	m_controller.SetConfigurationKey("TXLink.TXlevel", strTXlevel);
}

void SDRunoPlugin_TXLinkForm::SavePAlevel(string strPAlevel)
{
	m_controller.SetConfigurationKey("TXLink.PAlevel", strPAlevel);
}

string SDRunoPlugin_TXLinkForm::LoadRIT()
{
	std::string tmp = "";
	m_controller.GetConfigurationKey("TXLink.RIT", tmp);
	if (tmp.empty())
	{
		return "false";
	}
	return tmp;
}

void SDRunoPlugin_TXLinkForm::SaveRIT(string strRIT)
{
	m_controller.SetConfigurationKey("TXLink.RIT", strRIT);
}

void SDRunoPlugin_TXLinkForm::LoadSplitMode()
{
	std::string tmp = "";
	m_controller.GetConfigurationKey("TXLink.SplitMode", tmp);
	if (tmp.empty())
	{
		tmp = "0";
	}
	if (tmp == "0")
	{
		SplitMode = SplitNone;				//no split
	}
	else if (tmp == "1")
	{
		SplitMode = SplitTxRx;				//TX/RX split
	}
	else if (tmp == "2")
	{
		SplitMode = SplitFT8;				//FT8 split
	}
}

void SDRunoPlugin_TXLinkForm::SaveSplitMode()
{
	m_controller.SetConfigurationKey("TXLink.SplitMode", std::to_string(SplitMode));
}


void SDRunoPlugin_TXLinkForm::SetTXtemp(string strTemp)
{
	strTXtemp = strTemp;
}

string SDRunoPlugin_TXLinkForm::GetTXtemp()
{
	return strTXtemp;
}

void SDRunoPlugin_TXLinkForm::SetStandbyModeClicked()
{
	SetStandbyMode(StandbyMode);
}

void SDRunoPlugin_TXLinkForm::FixFreqButtonClicked()
{
	if (TrackFreq)
	{
		SendRXfreq();
	}
}

bool SDRunoPlugin_TXLinkForm::SendSerialMsg(const std::string& msg)
{
	// Lock the mutex to safely push the message
	std::lock_guard<std::mutex> lock(m_queueMutex);
	m_messageQueue.push(msg);
	return true;
}

void SDRunoPlugin_TXLinkForm::SaveIniData()
{
	std::cout << get_millis() << " SDRunoPlugin_TXLinkUi::SaveIniData called" << std::endl;
	//*
	SaveLocation();
	SaveCOM(GetCOM());
	SaveTXlevel(GetTXlevel());
	SavePAlevel(GetPAlevel());
	SaveRIT(GetRIT());
	SaveSplitMode();
	//*/
	std::cout << get_millis() << " SDRunoPlugin_TXLinkUi::SaveIniData complete" << std::endl;
}

void SDRunoPlugin_TXLinkForm::InitiateFormUnload()		//called from top level TXLink class
{
	//std::cout << "SDRunoPlugin_TXLinkUi::InitiateFormUnload - stopTimer" << std::endl;
	//StopTimer();		//stop further processing in form

	std::cout << get_millis() << " SDRunoPlugin_TXLinkUi::InitiateFormUnload - SetStandbyMode" << std::endl;
	SetStandbyMode(true);			//send TXto standby mode while COM port is still open.

	//shutdowm other Form resources
	//std::cout << get_millis() << "SDRunoPlugin_TXLinkUi::InitiateFormUnload - DXspiderDisconnect" << std::endl;
	//DXspiderDisconnect();

	//std::cout << "SDRunoPlugin_TXLinkUi::InitiateFormUnload - CloseCOMPort" << std::endl;
	//CloseCOMPort();		

	std::cout << get_millis() << "SDRunoPlugin_TXLinkUi::InitiateFormUnload - InitiateFormUnload complete" << std::endl;
}
