#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#include <winuser.h>
#endif

#include "SDRunoPlugin_TXLinkForm.h"
#include "SDRunoPlugin_TXLinkSettingsDialog.h"
#include "SDRunoPlugin_TXLinkUi.h"
#include "SDRunoPlugin_TXLink.h"
#include "UDP_Server.h"
#include "resource.h"
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


#define VERSION "V1.3"

char com_port[] = "\\\\.\\COM5";
string COMportPrefix = "\\\\.\\";
DWORD COM_BAUD_RATE = CBR_38400;
SimpleSerial Serial(com_port, COM_BAUD_RATE);		//needed here to create object but gets reset again later to saved port setting


HWND UNOhwnd = NULL;


// Form constructor with handles to parent and uno controller - launches form Setup
SDRunoPlugin_TXLinkForm::SDRunoPlugin_TXLinkForm(SDRunoPlugin_TXLinkUi& parent, IUnoPluginController& controller) :
	nana::form(nana::API::make_center(formWidth, formHeight), nana::appearance(false, true, false, false, true, false, false)),
	m_parent(parent),
	m_timerCount(0),
	m_controller(controller)
{
	Setup();
}

// Form deconstructor
SDRunoPlugin_TXLinkForm::~SDRunoPlugin_TXLinkForm()
{
	UDPExitRequest = true;			//UDP thread stop
	m_timer.stop();
}

// Start Form and start Nana UI processing
void SDRunoPlugin_TXLinkForm::Run()
{
	show();
	nana::exec();
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

	// TODO: Form code starts here
	COMportTb.multi_lines(false);
	COMportTb.events().text_changed([&] {
		string strCOM = COMportTb.caption();
		strCOM = COMportPrefix + strCOM;
		char* cstr = &strCOM[0];
		Serial.ReOpenPort(cstr, COM_BAUD_RATE);
		});

	string strCOM = m_parent.LoadCOM();		//get stored ini file
	if (strCOM == "")
	{
		strCOM = "COM6";			//default if not in ini file
	}
	// and reset serial
	COMportTb.caption(strCOM);		//this triggers event and opens/resets port on startup


	FixFreqBtn.caption("Split RX TX");
	FixFreqBtn.edge_effects(true);
	FixFreqBtn.events().click([&]
		{
			FixFreqBtnClicked();
		});

	FixTXupBtn.caption("Split TX UP");
	FixTXupBtn.edge_effects(true);
	FixTXupBtn.events().click([&]
		{
			FixTXup();
		});

	ResetRXFreqBtn.caption("Reset RX<-TX");
	ResetRXFreqBtn.edge_effects(true);
	ResetRXFreqBtn.events().click([&]
		{
			SetRXFreq();
		});

	StandbyBtn.caption("Set TX on");
	StandbyBtn.edge_effects(true);
	StandbyBtn.events().click([&]
		{
			if (!m_parent.StandbyMode)
			{
				StandbyBtn.caption("Set TX on");
				m_parent.StandbyMode = true;
			}
			else
			{
				StandbyBtn.caption("Set TX off");
				m_parent.StandbyMode = false;
				SendRXfreq();							//Update freq and levels to make be sure TX is sync'd
				SendTXlevel(TXlevelTb.caption());
				SendPAlevel(PAlevelTb.caption());
				SendTXhold(GetValidTXhold());
			}
			SetStandbyMode(m_parent.StandbyMode);
		});
	SetStandbyMode(m_parent.StandbyMode);

	BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam); 
	EnumWindows(EnumWindowsProc, NULL);							//find UNOhwnd for main SDRuno window
	m_timer.interval(std::chrono::milliseconds(100));
	// this next call sets the code to be executed after every interval
	m_timer.elapse([&] {
		m_timerCount++;
		WINDOWPLACEMENT wd;
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

		// look for incoming serial on timer event, also handles UDP requests
		ProcessIncomingSerial();
		});
	m_timer.start();

	// Load X and Y location for the form from the ini file (if exists)
	int posX = m_parent.LoadX();
	int posY = m_parent.LoadY();
	if (posX >= 0 && posY >= 0)
	{
		move(posX, posY);
	}
	


	// This code sets the plugin size, title and what to do when the X is pressed
	size(nana::size(formWidth, formHeight));
	caption("SDRuno Plugin TXlink");
	events().destroy([&] { m_parent.FormClosed(); });
	events().unload([&] { m_parent.FormUnload(); });

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

	FT8Lbl.caption("FT8");
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

	TXlevelTb.multi_lines(false);
	TXlevelTb.events().text_changed([&] {
		string strTXlevel = TXlevelTb.caption();
		SendTXlevel(strTXlevel);
		});

	string strTXlevel = m_parent.LoadTXlevel();		//get stored ini file
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
		});
	PAchkbox.events().click([&] {
		SendPAstate(PAchkbox.checked());
		});

	MuteTXchkbox.fgcolor(nana::color_rgb(forecolor));
	MuteTXchkbox.transparent(true);
	MuteTXchkbox.caption("Mute TX");
	MuteTXchkbox.check(true);
	MuteTXchkbox.events().click([&] {
		SetMuteTX(MuteTXchkbox.checked());
		});

	string strPAlevel = m_parent.LoadPAlevel();		//get stored ini file
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

	// set text boxes to last saved defaults from SDRuno ini file
	TXlevelTb.caption(strTXlevel);		//this triggers event and opens/resets port on startup
	PAlevelTb.caption(strPAlevel);		//this triggers event and opens/resets port on startup

	// and send to Arduino (although not sure this does anything here??????????
	SendTXlevel(strTXlevel);
	SendPAlevel(strPAlevel);
	SendPAstate(false);					//always start with ext PA off for safety
	SendFreqCalToTX();
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

void SDRunoPlugin_TXLinkForm::SetStandbyMode(bool Mode)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{U}";
		if (Mode)
		{
			str = "{D}";
		}
		char* to_send = &str[0];
		is_sent = Serial.WriteSerialPort(to_send);
	}
}

string NewMsg = "";

void SDRunoPlugin_TXLinkForm::ProcessIncomingSerial()
{
	// called from timer event. Read any data while available, uses {} as begin/end message

	string NextChar;
	if (Serial.ClearComm() > 0) { NextChar = Serial.ReadSerialPortChar(); }
	while (NextChar.length() > 0)
	{
		//valid read
		if (NextChar == "{")
		{
			NewMsg = "";
		}
		else if (NextChar == "}")
		{
			//execute compiled message
			if (NewMsg.substr(0,1) == "M")
			{
				m_controller.SetAudioMute(0, true);
			}
			else if (NewMsg.substr(0,1) == "N")
			{
				m_controller.SetAudioMute(0, false);
			}
			else if (NewMsg.substr(0,1) == "F")
			{
				SWRFWDtxt.caption(NewMsg.substr(1));
				//SWRFWDtxt.caption("123");
				//SWRFWDlbl.caption("123");
			}
			else if (NewMsg.substr(0,1) == "R")
			{
				SWRREFtxt.caption(NewMsg.substr(1));
			}
			else if (NewMsg.substr(0,1) == "S")
			{
				SWRSWRtxt.caption(NewMsg.substr(1));
			}
		}
		else
		{
			NewMsg.append(NextChar);
		}
		NextChar = "";
		if (Serial.ClearComm() > 0) { NextChar = Serial.ReadSerialPortChar(); }
	}

	//WSJT_X UDP request?
	if (UDPevent)
	{
		bool is_sent = false;
		if (Serial.Connected())
		{
			string str = "{X0}";
			char* to_send = &str[0];
			if (TXflg)
			{
				//new TX so send freq and message so Arduino updates
				str = "{Y" + to_string(TXDF) + "}";
				to_send = &str[0];
				is_sent = Serial.WriteSerialPort(to_send);

				str = "{Z" + TXsymbols + "}";
				to_send = &str[0];
				is_sent = Serial.WriteSerialPort(to_send);

				// TX key on message
				str = "{X1}";
				to_send = &str[0];
			}
			if (str == "{X0}")
			{
				FT8txOnLbl.bgcolor(nana::colors::light_grey);
			}
			else
			{
				FT8txOnLbl.bgcolor(nana::colors::red);
			}

			//send TX last so freq & message updated first in Arduino
			is_sent = Serial.WriteSerialPort(to_send);
		}
		if (is_sent) { UDPevent = false; }
		ShowIsSent(is_sent);
		FT8msgLbl.caption(strDecodeMsg);
		UDPevent = false;
	}
}

void SDRunoPlugin_TXLinkForm::SendRXfreq()
{

	if (m_parent.TrackFreq || freqTb.caption() == "")
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

void SDRunoPlugin_TXLinkForm::SendTXfreq()
{
	bool is_sent = false;
	string strTXfreq = "";

	strTXfreq = std::to_string(CurrentTXfreq);

	//only need upto decimal . (ie Hz)
	strTXfreq = strTXfreq.substr(0, strTXfreq.find(".", 0));

	// send current TX freq to TX
	if (!Serial.Connected())
	{
		//attempt to restart COM port
		string strCOM = COMportTb.caption();
		strCOM = COMportPrefix + strCOM;
		char* cstr = &strCOM[0];
		Serial.ReOpenPort(cstr, COM_BAUD_RATE);
	}
	if (Serial.Connected()) {
		string str = "{F" + strTXfreq + "}";
		char* to_send = &str[0];
		is_sent = Serial.WriteSerialPort(to_send);
	}

	ShowIsSent(is_sent);

	//update Plugin label to show TXfreq
	//add dots to show 1000's in Plugin label
	for (int i = strTXfreq.length(); i > 3; i = i - 3)
	{
		strTXfreq = strTXfreq.substr(0, i - 3) + "." + strTXfreq.substr(i - 3);
	}
	freqTb.format(true);
	freqTb.caption("<bold size=12>" + strTXfreq + "</>");				//TXfreq + "< bold size = 14 color = 0x000000 font = \"Verdana\"></>");

	m_parent._updateTX_AnnoEntry("TX", "", (int64_t) CurrentTXfreq);
	m_parent._updateRX_AnnoEntry("RX", "", (int64_t) CurrentRXfreq);

	double vtxFreq = m_controller.GetVfoFrequency(1);
	if (vtxFreq != CurrentTXfreq && vtxFreq > 0)
	{
		m_controller.SetVfoFrequency(1, CurrentTXfreq);					// force vrx1 to track TX freq
	}
}

void SDRunoPlugin_TXLinkForm::ShowIsSent(bool is_sent)
{
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
	}
}

void SDRunoPlugin_TXLinkForm::SendTXlevel(string strTXlevel)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{L" + strTXlevel + "}";
		char* to_send = &str[0];
		is_sent = Serial.WriteSerialPort(to_send);
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
		char* to_send = &str[0];
		is_sent = Serial.WriteSerialPort(to_send);
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
		char* to_send = &str[0];
		is_sent = Serial.WriteSerialPort(to_send);
	}
}

void SDRunoPlugin_TXLinkForm::SendTXhold(string TXhold)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{H" + TXhold + "}";
		char* to_send = &str[0];
		is_sent = Serial.WriteSerialPort(to_send);
	}
}

void SDRunoPlugin_TXLinkForm::SendFanHold(string FanHold)
{
	bool is_sent = false;
	if (Serial.Connected())
	{
		string str = "{B" + FanHold + "}";
		char* to_send = &str[0];
		is_sent = Serial.WriteSerialPort(to_send);
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
		tmp = "-16900";								//default
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
			char* to_send = &str[0];
			is_sent = Serial.WriteSerialPort(to_send);
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
	Serial.CloseSerialPort();
}

void SDRunoPlugin_TXLinkForm::FixFreqBtnClicked()
{
	if (!m_parent.TrackFreq)
	{
		FixFreqBtn.caption("Split RX TX");
		m_parent.TrackFreq = true;
		freqTb.bgcolor(nana::colors::white);
		freqTb.fgcolor(nana::colors::black);
	}
	else
	{
		FixFreqBtn.caption("Track RX->TX");
		m_parent.TrackFreq = false;
		freqTb.bgcolor(nana::colors::red);
		freqTb.fgcolor(nana::colors::white);
	}
	m_parent.FixFreqButtonClicked();

}

void SDRunoPlugin_TXLinkForm::FixTXup()
{
	TXupHz = GetValidTXupHz();
	CurrentTXfreq = CurrentTXfreq + TXupHz;			//increment every click
	if (m_parent.TrackFreq)
	{	//take it out of tracking
		FixFreqBtn.caption("Track RX->TX");
		m_parent.TrackFreq = false;
		freqTb.bgcolor(nana::colors::red);
		freqTb.fgcolor(nana::colors::white);
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

void SDRunoPlugin_TXLinkForm::SetRXFreq()
{
	m_controller.SetVfoFrequency(0, CurrentTXfreq);
	m_controller.SetVfoFrequency(1, CurrentTXfreq);
}
	

void SDRunoPlugin_TXLinkForm::SettingsButton_Click()
{
	//Create a new settings dialog object
	SDRunoPlugin_TemplateSettingsDialog settingsDialog{ m_parent,m_controller };

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
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char class_name[79];
	char title[79];
	GetClassNameA(hwnd, class_name, sizeof(class_name));
	GetWindowTextA(hwnd, title, sizeof(title));
	if (strcmp(title, "SDRuno Main 0") == 0)
	{
		UNOhwnd = hwnd;
		return FALSE;				//halts Enum iteration
	}
	else
	{
		return TRUE;
	}
}

