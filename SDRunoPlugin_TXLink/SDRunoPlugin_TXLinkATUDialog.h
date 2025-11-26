#pragma once

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
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>


// TODO: Change these numbers to the height and width of your form
#define dialogFormWidth (297)
#define dialogFormHeight (240)

class SDRunoPlugin_TXLinkForm;

class SDRunoPlugin_TemplateATUDialog : public nana::form
{

public:

	SDRunoPlugin_TemplateATUDialog(SDRunoPlugin_TXLinkForm& parent, IUnoPluginController& controller);
	~SDRunoPlugin_TemplateATUDialog();

	void Run();

private:

	void Setup();
	void SaveLocation();
	int LoadX();
	int LoadY();
	void lIncBtn_clicked();
	void lDecBtn_clicked();
	void cIncBtn_clicked();
	void cDecBtn_clicked();
	void BankBtn_clicked();
	void IOtogBtn_clicked();
	void SaveBtn_clicked();
	void CancelBtn_clicked();
	void QueryBtn_clicked();

	// TODO: Now add your UI controls here
	nana::button lIncBtn{ *this, nana::rectangle(20, 65, 40, 20) };
	nana::button lDecBtn{ *this, nana::rectangle(80, 65, 40, 20) };
	nana::label lIncLbl{ *this, nana::rectangle(140, 65, 40, 20) };
	nana::button cIncBtn{ *this, nana::rectangle(20, 95, 40, 20) };
	nana::button cDecBtn{ *this, nana::rectangle(80, 95, 40, 20) };
	nana::label cIncLbl{ *this, nana::rectangle(140, 95, 40, 20) };
	nana::button BankBtn{ *this, nana::rectangle(20, 125, 40, 20) };
	nana::button IOtogBtn{ *this, nana::rectangle(80, 125, 40, 20) };
	nana::label IOtogLbl{ *this, nana::rectangle(140, 125, 40, 20) };
	nana::button SaveBtn{ *this, nana::rectangle(20, 155, 40, 20) };
	nana::button CancelBtn{ *this, nana::rectangle(140, 155, 40, 20) };
	nana::button QueryBtn{ *this, nana::rectangle(80, 155, 40, 20) };
	nana::timer m_timer;

	SDRunoPlugin_TXLinkForm & m_parent;
	IUnoPluginController & m_controller;

	std::mutex m_lock;
};

