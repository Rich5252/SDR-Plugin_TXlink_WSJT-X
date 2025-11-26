#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "SDRunoPlugin_TXLinkATUDialog.h"
#include "SDRunoPlugin_TXLinkForm.h"
#include "resource.h"
#include <io.h>
#include <shlobj.h>

// Constructor implementation
SDRunoPlugin_TemplateATUDialog::SDRunoPlugin_TemplateATUDialog(SDRunoPlugin_TXLinkForm& parent, IUnoPluginController& controller) :
	//nana::form(parent, nana::API::make_center(parent, dialogFormWidth, dialogFormHeight), nana::appearance(true, false, true, false, false, false, false)),
	nana::form(nana::API::make_center(dialogFormWidth, dialogFormHeight), nana::appearance(true, false, true, false, false, false, false)),
	m_parent(parent),
	m_controller(controller)
{
	Setup();
}


// Form deconstructor
SDRunoPlugin_TemplateATUDialog::~SDRunoPlugin_TemplateATUDialog()
{
	// **This Should not be necessary, but just in case - we are going to remove all event handlers
	// previously assigned to the "destroy" event to avoid memory leaks;
	this->events().destroy.clear();
}

// Start Form and start Nana UI processing
void SDRunoPlugin_TemplateATUDialog::Run()
{	
	show();
	nana::exec();
}

int SDRunoPlugin_TemplateATUDialog::LoadX()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.Xsettings", tmp);
	if (tmp.empty())
	{
		tmp = "0";
		//?std::lock_guard<std::mutex> l(m_lock); 
		m_controller.SetConfigurationKey("TXLink.Xsettings", tmp);		//default
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
// TODO: Change Template to plugin name
int SDRunoPlugin_TemplateATUDialog::LoadY()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TXLink.Ysettings", tmp);
	if (tmp.empty())
	{
		tmp = "0";
		//?std::lock_guard<std::mutex> l(m_lock); 
		m_controller.SetConfigurationKey("TXLink.Ysettings", tmp);		//default
	}
	return stoi(tmp);
}

// Create the settings dialog form
void SDRunoPlugin_TemplateATUDialog::Setup()
{
	// TODO: Form code starts here

	// Load X and Y locations for the dialog from the ini file (if exists)
	int posX = LoadX();
	int posY = LoadY();
	move(posX, posY);

	// This code sets the plugin size and title
	size(nana::size(dialogFormWidth, dialogFormHeight));
	caption("ATU Tune");
	events().unload([&] { SaveLocation(); });

	// Set the forms back color to black to match SDRuno's settings dialogs
	this->bgcolor(nana::colors::black);

	// TODO: Extra form code goes here
	lIncBtn.caption("Incr l");
	lIncBtn.edge_effects(true);
	lIncBtn.events().click([&]
		{
			lIncBtn_clicked();
		});
	lIncLbl.caption("0");
	lIncLbl.bgcolor(nana::colors::black);
	lIncLbl.fgcolor(nana::colors::white);

	lDecBtn.caption("Decr l");
	lDecBtn.edge_effects(true);
	lDecBtn.events().click([&]
		{
			lDecBtn_clicked();
		});

	cIncBtn.caption("Incr c");
	cIncBtn.edge_effects(true);
	cIncBtn.events().click([&]
		{
			cIncBtn_clicked();
		});
	cIncLbl.caption("0");
	cIncLbl.bgcolor(nana::colors::black);
	cIncLbl.fgcolor(nana::colors::white);

	cDecBtn.caption("Decr c");
	cDecBtn.edge_effects(true);
	cDecBtn.events().click([&]
		{
			cDecBtn_clicked();
		});

	BankBtn.caption("Bank");
	BankBtn.edge_effects(true);
	BankBtn.events().click([&]
		{
			BankBtn_clicked();
		});

	IOtogBtn.caption("IO Tog");
	IOtogBtn.edge_effects(true);
	IOtogBtn.events().click([&]
		{
			IOtogBtn_clicked();
		});

	IOtogLbl.caption("o");
	IOtogLbl.bgcolor(nana::colors::black);
	IOtogLbl.fgcolor(nana::colors::white);

	SaveBtn.caption("Save");
	SaveBtn.edge_effects(true);
	SaveBtn.events().click([&]
		{
			SaveBtn_clicked();
		});

	CancelBtn.caption("Cancel");
	CancelBtn.edge_effects(true);
	CancelBtn.events().click([&]
		{
			CancelBtn_clicked();
		});

	QueryBtn.caption("Query");
	QueryBtn.edge_effects(true);
	QueryBtn.events().click([&]
		{
			QueryBtn_clicked();
		});


		m_timer.interval(std::chrono::milliseconds(100));
	// this next call sets the code to be executed after every interval
	m_timer.elapse([&] {

		});
	m_timer.start();

}

void SDRunoPlugin_TemplateATUDialog::lIncBtn_clicked()
{
	if (m_parent.SendSerialMsg("{+l}"))
	{
		lIncLbl.caption(std::to_string(stoi(lIncLbl.caption()) + 1));
	}
}

void SDRunoPlugin_TemplateATUDialog::lDecBtn_clicked()
{
	if (m_parent.SendSerialMsg("{-l}"))
	{
		lIncLbl.caption(std::to_string(stoi(lIncLbl.caption()) - 1));
	}
}

void SDRunoPlugin_TemplateATUDialog::cIncBtn_clicked()
{
	if (m_parent.SendSerialMsg("{+c}"))
	{
		cIncLbl.caption(std::to_string(stoi(cIncLbl.caption()) + 1));
	}
}

void SDRunoPlugin_TemplateATUDialog::cDecBtn_clicked()
{
	if (m_parent.SendSerialMsg("{-c}"))
	{
		cIncLbl.caption(std::to_string(stoi(cIncLbl.caption()) - 1));
	}
}

void SDRunoPlugin_TemplateATUDialog::BankBtn_clicked()
{
	std::string strBank = "{b}";
	m_parent.SendSerialMsg(strBank);			//toggle TuneBank
}

void SDRunoPlugin_TemplateATUDialog::IOtogBtn_clicked()
{
	string strIO = "i";
	if (IOtogLbl.caption() != "o")
	{
		strIO = "o";
	}
	if (m_parent.SendSerialMsg("{" + strIO + "}"))
	{
		IOtogLbl.caption(strIO);
	}
}

void SDRunoPlugin_TemplateATUDialog::SaveBtn_clicked()
{
	m_parent.SendSerialMsg("{s}");
	lIncLbl.caption("0");
	cIncLbl.caption("0");
}

void SDRunoPlugin_TemplateATUDialog::CancelBtn_clicked()
{
	m_parent.SendSerialMsg("{f}");
	lIncLbl.caption("0");
	cIncLbl.caption("0");
	IOtogLbl.caption("o");
}

void SDRunoPlugin_TemplateATUDialog::QueryBtn_clicked()
{
	m_parent.SendSerialMsg("{?}");
}

void SDRunoPlugin_TemplateATUDialog::SaveLocation()
{
	nana::point position = this->pos();
	if (position.x >= 0 && position.y >= 0)
	{
		m_controller.SetConfigurationKey("TXLink.Xsettings", std::to_string(position.x));
		m_controller.SetConfigurationKey("TXLink.Ysettings", std::to_string(position.y));
	}

}

