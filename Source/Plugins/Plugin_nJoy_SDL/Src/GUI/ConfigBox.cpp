//////////////////////////////////////////////////////////////////////////////////////////
// Project description
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
// Name: nJoy 
// Description: A Dolphin Compatible Input Plugin
//
// Author: Falcon4ever (nJoy@falcon4ever.com)
// Site: www.multigesture.net
// Copyright (C) 2003-2008 Dolphin Project.
//
//////////////////////////////////////////////////////////////////////////////////////////
//
// Licensetype: GNU General Public License (GPL)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.
//
// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/
//
// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/
//
//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Include
// ŻŻŻŻŻŻŻŻŻ
#include "math.h" // System

#include "ConfigBox.h" // Local
#include "../nJoy.h"
#include "Images/controller.xpm"

extern CONTROLLER_INFO	*joyinfo;
//extern CONTROLLER_MAPPING joysticks[4];
extern bool emulator_running;

static const char* ControllerType[] =
{
	"Joystick (default)",
	"Joystick (no hat)",
//	"Joytstick (xbox360)",	// Shoulder buttons -> axis
//	"Keyboard"				// Not supported yet, sorry F|RES ;( ...
};
////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// The wxWidgets class
// ŻŻŻŻŻŻŻ
BEGIN_EVENT_TABLE(ConfigBox,wxDialog)
	EVT_CLOSE(ConfigBox::OnClose)
	EVT_BUTTON(ID_ABOUT, ConfigBox::AboutClick)
	EVT_BUTTON(ID_OK, ConfigBox::OKClick)
	EVT_BUTTON(ID_CANCEL, ConfigBox::CancelClick)
	EVT_COMBOBOX(IDC_JOYNAME, ConfigBox::ChangeJoystick)
	EVT_COMBOBOX(IDC_CONTROLTYPE, ConfigBox::ChangeControllertype)
	EVT_CHECKBOX(IDC_JOYATTACH, ConfigBox::EnableDisable)
	EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK, ConfigBox::NotebookPageChanged)

	EVT_CHECKBOX(IDC_SAVEBYID, ConfigBox::ChangeSettings) // Settings
	EVT_CHECKBOX(IDC_SHOWADVANCED, ConfigBox::ChangeSettings)
	EVT_COMBOBOX(IDCB_MAINSTICK_DIAGONAL, ConfigBox::ChangeSettings)	
	EVT_CHECKBOX(IDCB_MAINSTICK_S_TO_C, ConfigBox::ChangeSettings)	

	EVT_BUTTON(IDB_SHOULDER_L, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_SHOULDER_R, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_BUTTON_A, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_BUTTON_B, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_BUTTON_X, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_BUTTON_Y, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_BUTTON_Z, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_BUTTONSTART, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_BUTTONHALFPRESS, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_DPAD_UP, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_DPAD_DOWN, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_DPAD_LEFT, ConfigBox::GetButtons)
	EVT_BUTTON(IDB_DPAD_RIGHT, ConfigBox::GetButtons)

	EVT_BUTTON(IDB_ANALOG_MAIN_X, ConfigBox::GetAxis)
	EVT_BUTTON(IDB_ANALOG_MAIN_Y, ConfigBox::GetAxis)
	EVT_BUTTON(IDB_ANALOG_SUB_X, ConfigBox::GetAxis)
	EVT_BUTTON(IDB_ANALOG_SUB_Y, ConfigBox::GetAxis)

	#if wxUSE_TIMER
		EVT_TIMER(wxID_ANY, ConfigBox::OnTimer)
	#endif
END_EVENT_TABLE()

ConfigBox::ConfigBox(wxWindow *parent, wxWindowID id, const wxString &title,
					 const wxPoint &position, const wxSize& size, long style)
	: wxDialog(parent, id, title, position, size, style)
	#if wxUSE_TIMER
	, m_timer(this)
	#endif
{

	notebookpage = 0;
	CreateGUIControls();

	#if wxUSE_TIMER
		m_timer.Start( floor((double)(1000 / 30)) );
	#endif


}

ConfigBox::~ConfigBox()
{
	// empty
}

// Close window
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ConfigBox::OnClose(wxCloseEvent& /*event*/)
{
	EndModal(0);
	if(!emulator_running) PAD_Shutdown(); // Close pads, unless we are running a game
}

// Call about dialog
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ConfigBox::AboutClick(wxCommandEvent& event)
{
	#ifdef _WIN32
		wxWindow win;
		win.SetHWND((WXHWND)this->GetHWND());
		win.Enable(false);  

		AboutBox frame(&win);
		frame.ShowModal();

		win.Enable(true);
		win.SetHWND(0); 
	#else
		AboutBox frame(NULL);
		frame.ShowModal();
	#endif	
}

// Click OK
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ConfigBox::OKClick(wxCommandEvent& event)
{
	if (event.GetId() == ID_OK)
	{
		for(int i=0; i<4 ;i++) GetControllerAll(i); // Update joysticks array
		g_Config.Save();	// Save settings
		g_Config.Load();	// Reload settings
		Close(); // Call OnClose()
	}
}

// Click Cancel
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ConfigBox::CancelClick(wxCommandEvent& event)
{
	if (event.GetId() == ID_CANCEL)
	{
		g_Config.Load();	// Reload settings
		Close(); // Call OnClose()
	}
}
//////////////////////////////////


// Change settings
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ConfigBox::ChangeSettings( wxCommandEvent& event )
{
	switch(event.GetId())
	{
		case IDC_SAVEBYID:
			g_Config.bSaveByID = m_CBSaveByID[notebookpage]->IsChecked();
			break;

		case IDC_SHOWADVANCED: 
			g_Config.bShowAdvanced = m_CBShowAdvanced[notebookpage]->IsChecked();			
			for(int i = 0; i < 4; i++)
			{
				m_CBShowAdvanced[i]->SetValue(g_Config.bShowAdvanced);
				m_sMainRight[i]->Show(g_Config.bShowAdvanced);
			}
			SizeWindow();
			break;

		case IDCB_MAINSTICK_DIAGONAL:
			g_Config.SDiagonal = m_CoBDiagonal[notebookpage]->GetLabel().mb_str();

		case IDCB_MAINSTICK_S_TO_C:
			g_Config.bSquareToCircle = m_CBS_to_C[notebookpage]->IsChecked();
	}
}


// Enable or disable joystick and update the GUI
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ConfigBox::EnableDisable(wxCommandEvent& event)
{
	// Update the enable / disable status
	DoEnableDisable(notebookpage);

}
void ConfigBox::DoEnableDisable(int _notebookpage)
{
#ifdef _WIN32 // There is no FindItem in linux so this doesn't work
	// Update the enable / disable status
	joysticks[_notebookpage].enabled = m_Joyattach[_notebookpage]->GetValue();

	// Enable or disable all buttons
	for(int i = IDB_SHOULDER_L; i < (IDB_SHOULDER_L + 13 + 4); i++)
	{
		m_Controller[_notebookpage]->FindItem(i)->Enable(joysticks[_notebookpage].enabled);
	}

	// Enable or disable settings controls
	m_Controller[_notebookpage]->FindItem(IDC_DEADZONE)->Enable(joysticks[_notebookpage].enabled);
	m_Controller[_notebookpage]->FindItem(IDC_CONTROLTYPE)->Enable(joysticks[_notebookpage].enabled);

	// General settings
	m_CBSaveByID[_notebookpage]->SetValue(g_Config.bSaveByID);
	m_CBShowAdvanced[_notebookpage]->SetValue(g_Config.bShowAdvanced);

	// Advanced settings
	m_CoBDiagonal[notebookpage]->SetValue(wxString::FromAscii(g_Config.SDiagonal.c_str()));
	m_CBS_to_C[notebookpage]->SetValue(g_Config.bSquareToCircle);

	m_Controller[_notebookpage]->Refresh(); // Repaint the background
#endif
}


// Notebook page changed
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ConfigBox::NotebookPageChanged(wxNotebookEvent& event)
{	
	notebookpage = event.GetSelection();
}


// Change Joystick. Load saved settings.
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ConfigBox::ChangeJoystick(wxCommandEvent& event)
{
	 // Save potential changes
	if(g_Config.bSaveByID)
	{
		int Tmp = joysticks[notebookpage].ID; // Don't update the ID
		GetControllerAll(notebookpage);
		joysticks[notebookpage].ID = Tmp;
		g_Config.Save();
	}

	//PanicAlert("%i", m_Joyname[notebookpage]->GetSelection());

	// Update the ID for the virtual device to a new physical device
	joysticks[notebookpage].ID = m_Joyname[notebookpage]->GetSelection();

	//PanicAlert("%i  %i", joysticks[notebookpage].ID, notebookpage);

	// Load device settings
	if(g_Config.bSaveByID)
	{
		g_Config.Load(true); // Then load the current
		SetControllerAll(notebookpage);
	}

	// Remap the controller to
	if (joysticks[notebookpage].enabled)
	{
		if (SDL_JoystickOpened(notebookpage)) SDL_JoystickClose(joystate[notebookpage].joy);
		joystate[notebookpage].joy = SDL_JoystickOpen(joysticks[notebookpage].ID);
	}

}


// Populate the config window
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ConfigBox::OnPaint( wxPaintEvent &event )
{
	event.Skip();

	wxPaintDC dcWin(m_pKeys[notebookpage]);
	PrepareDC( dcWin );
	if(joysticks[notebookpage].enabled)
		dcWin.DrawBitmap( WxStaticBitmap1_BITMAP, 94, 0, true );
	else
		dcWin.DrawBitmap( WxStaticBitmap1_BITMAPGray, 94, 0, true );
}

// Populate the config window
// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ
void ConfigBox::CreateGUIControls()
{
	#ifndef _DEBUG		
		SetTitle(wxT("Configure: nJoy v"INPUT_VERSION" Input Plugin"));
	#else			
		SetTitle(wxT("Configure: nJoy v"INPUT_VERSION" (Debug) Input Plugin"));
	#endif
	
	SetIcon(wxNullIcon);

	//WxStaticBitmap1_BITMAP(ConfigBox_WxStaticBitmap1_XPM);
	//WxStaticBitmap1_BITMAP = new WxStaticBitmap1_BITMAP(ConfigBox_WxStaticBitmap1_XPM);

#ifndef _WIN32
	// Force a 8pt font so that it looks more or less "correct" regardless of the default font setting
	wxFont f(8,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	SetFont(f);
#endif

	// Buttons
	m_About = new wxButton(this, ID_ABOUT, wxT("About"), wxDefaultPosition, wxSize(75, 25), 0, wxDefaultValidator, wxT("About"));
	m_OK = new wxButton(this, ID_OK, wxT("OK"), wxDefaultPosition, wxSize(75, 25), 0, wxDefaultValidator, wxT("OK"));
	m_Cancel = new wxButton(this, ID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxSize(75, 25), 0, wxDefaultValidator, wxT("Cancel"));

	// Notebook
	m_Notebook = new wxNotebook(this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
	
	// Controller pages
	m_Controller[0] = new wxPanel(m_Notebook, ID_CONTROLLERPAGE1, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_Controller[0], wxT("Controller 1"));
	m_Controller[1] = new wxPanel(m_Notebook, ID_CONTROLLERPAGE2, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_Controller[1], wxT("Controller 2"));
	m_Controller[2] = new wxPanel(m_Notebook, ID_CONTROLLERPAGE3, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_Controller[2], wxT("Controller 3"));
	m_Controller[3] = new wxPanel(m_Notebook, ID_CONTROLLERPAGE4, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_Controller[3], wxT("Controller 4"));


	// Define bitmap for EVT_PAINT
	WxStaticBitmap1_BITMAP = wxBitmap(ConfigBox_WxStaticBitmap1_XPM);

	// Gray version
	wxImage WxImageGray = WxStaticBitmap1_BITMAP.ConvertToImage();
	WxImageGray = WxImageGray.ConvertToGreyscale();
	WxStaticBitmap1_BITMAPGray = wxBitmap(WxImageGray);

	// --------------------------------------------------------------------
	// Search for devices and add them to the device list	
	// -----------------------------
	wxArrayString arrayStringFor_Joyname; // The string array
	if(SDL_NumJoysticks() > 0)
	{
		for(int x = 0; x < SDL_NumJoysticks(); x++)
		{
			arrayStringFor_Joyname.Add(wxString::FromAscii(joyinfo[x].Name));
		}
	}
	else
	{
		arrayStringFor_Joyname.Add(wxString::FromAscii("No Joystick detected!"));
	}

	// --------------------------------------------------------------------
	// Populate the controller type list
	// -----------------------------
	wxArrayString arrayStringFor_Controltype;
	arrayStringFor_Controltype.Add(wxString::FromAscii(ControllerType[CTL_TYPE_JOYSTICK]));
	arrayStringFor_Controltype.Add(wxString::FromAscii(ControllerType[CTL_TYPE_JOYSTICK_NO_HAT]));
	// arrayStringFor_Controltype.Add(wxString::FromAscii(ControllerType[CTL_TYPE_JOYSTICK_XBOX360]));
	// arrayStringFor_Controltype.Add(wxString::FromAscii(ControllerType[CTL_TYPE_KEYBOARD]));

	
	// --------------------------------------------------------------------
	// Populate the deadzone list
	// -----------------------------
	char buffer [8];
	wxArrayString arrayStringFor_Deadzone;
	for(int x = 1; x <= 100; x++)
	{		
		sprintf (buffer, "%d %%", x);
		arrayStringFor_Deadzone.Add(wxString::FromAscii(buffer));
	}


	// Populate all four pages
	for(int i=0; i<4 ;i++)
	{	
		// --------------------------------------------------------------------
		// Populate keys sizer
		// -----------------------------
		// Set relative values for the keys
		int t = -75; // Top
		int l = -4; // Left
		m_sKeys[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("Keys"));
		m_pKeys[i] = new wxPanel(m_Controller[i], ID_KEYSPANEL1 + i, wxDefaultPosition, wxSize(600, 400));
		//m_sKeys[i] = new wxStaticBox (m_Controller[i], IDG_JOYSTICK, wxT("Keys"), wxDefaultPosition, wxSize(608, 500));
		m_sKeys[i]->Add(m_pKeys[i], 0, (wxALL), 0); // margin = 0

		// --------------------------------------------------------------------
		// GameCube controller picture
		// -----------------------------
		// TODO: Controller image
		// Placeholder instead of bitmap
		// m_PlaceholderBMP[i] = new wxTextCtrl(m_Controller[i], ID_CONTROLLERPICTURE, wxT("BITMAP HERE PLZ KTHX!"), wxPoint(98, 75), wxSize(423, 306), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("BITMAP HERE PLZ KTHX!"));
		// m_PlaceholderBMP[i]->Enable(false);
		
		/* You can enable the bitmap here. But it loads überslow on init... (only in windows, linux
		seems to load it fast!) AAaaand the XPM file (256 colours) looks crappier than the real bitmap...
		so maybe we can find a way to use a bitmap?	*/
		//m_controllerimage[i] = new wxStaticBitmap(m_pKeys[i], ID_CONTROLLERPICTURE, WxStaticBitmap1_BITMAP, wxPoint(l + 98, t + 75), wxSize(421,304));		
		//m_controllerimage[i] = new wxBitmap( WxStaticBitmap1_BITMAP );		

		 // Paint background. This allows objects to be visible on top of the picture
		m_pKeys[i]->Connect(wxID_ANY, wxEVT_PAINT,
			wxPaintEventHandler(ConfigBox::OnPaint),
			(wxObject*)0, this);


		// --------------------------------------------------------------------
		// Keys objects
		// -----------------------------
		// Left shoulder
		m_JoyShoulderL[i] = new wxTextCtrl(m_pKeys[i], ID_SHOULDER_L, wxT("0"), wxPoint(l + 6, t + 80), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyShoulderL[i]->Enable(false);
		m_bJoyShoulderL[i] = new wxButton(m_pKeys[i], IDB_SHOULDER_L, wxEmptyString, wxPoint(l + 70, t + 82), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);

		// Left analog
		int ALt = 170; int ALw = ALt + 14; int ALb = ALw + 2; // Set offset
		m_JoyAnalogMainX[i] = new wxTextCtrl(m_pKeys[i], ID_ANALOG_MAIN_X, wxT("0"), wxPoint(l + 6, t + ALw), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyAnalogMainX[i]->Enable(false);
		m_JoyAnalogMainY[i] = new wxTextCtrl(m_pKeys[i], ID_ANALOG_MAIN_Y, wxT("0"), wxPoint(l + 6, t + ALw + 36), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyAnalogMainY[i]->Enable(false);		
		m_bJoyAnalogMainX[i] = new wxButton(m_pKeys[i], IDB_ANALOG_MAIN_X, wxEmptyString, wxPoint(l + 70, t + ALb), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyAnalogMainY[i] = new wxButton(m_pKeys[i], IDB_ANALOG_MAIN_Y, wxEmptyString, wxPoint(l + 70, t + ALb + 36), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_textMainX[i] = new wxStaticText(m_pKeys[i], IDT_ANALOG_MAIN_X, wxT("X-axis"), wxPoint(l + 6, t + ALt), wxDefaultSize, 0, wxT("X-axis"));
		m_textMainY[i] = new wxStaticText(m_pKeys[i], IDT_ANALOG_MAIN_Y, wxT("Y-axis"), wxPoint(l + 6, t + ALt + 36), wxDefaultSize, 0, wxT("Y-axis"));
		
		// D-Pad
		int DPt = 255; int DPw = DPt + 14; int DPb = DPw + 2; // Set offset
		m_JoyDpadUp[i] = new wxTextCtrl(m_pKeys[i], ID_DPAD_UP, wxT("0"), wxPoint(l + 6, t + DPw), wxSize(59, t + 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyDpadUp[i]->Enable(false);
		m_JoyDpadDown[i] = new wxTextCtrl(m_pKeys[i], ID_DPAD_DOWN, wxT("0"), wxPoint(l + 6, t + DPw + 36*1), wxSize(59, t + 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyDpadDown[i]->Enable(false);
		m_JoyDpadLeft[i] = new wxTextCtrl(m_pKeys[i], ID_DPAD_LEFT, wxT("0"), wxPoint(l + 6, t + DPw + 36*2), wxSize(59, t + 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyDpadLeft[i]->Enable(false);
		m_JoyDpadRight[i] = new wxTextCtrl(m_pKeys[i], ID_DPAD_RIGHT, wxT("0"), wxPoint(l + 6, t + DPw + 36*3), wxSize(59, t + 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyDpadRight[i]->Enable(false);
		m_bJoyDpadUp[i] = new wxButton(m_pKeys[i], IDB_DPAD_UP, wxEmptyString, wxPoint(l + 70, t + DPb + 36*0), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyDpadDown[i] = new wxButton(m_pKeys[i], IDB_DPAD_DOWN, wxEmptyString, wxPoint(l + 70, t + DPb + 36*1), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyDpadLeft[i] = new wxButton(m_pKeys[i], IDB_DPAD_LEFT, wxEmptyString, wxPoint(l + 70, t + DPb + 36*2), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyDpadRight[i] = new wxButton(m_pKeys[i], IDB_DPAD_RIGHT, wxEmptyString, wxPoint(l + 70, t + DPb + 36*3), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_textDpadUp[i] = new wxStaticText(m_pKeys[i], IDT_DPAD_UP, wxT("Up"), wxPoint(l + 6, t + DPt + 36*0), wxDefaultSize, 0, wxT("Up"));
		m_textDpadDown[i] = new wxStaticText(m_pKeys[i], IDT_DPAD_DOWN, wxT("Down"), wxPoint(l + 6, t + DPt + 36*1), wxDefaultSize, 0, wxT("Down"));
		m_textDpadLeft[i] = new wxStaticText(m_pKeys[i], IDT_DPAD_LEFT, wxT("Left"), wxPoint(l + 6, t + DPt + 36*2), wxDefaultSize, 0, wxT("Left"));
		m_textDpadRight[i] = new wxStaticText(m_pKeys[i], IDT_DPAD_RIGHT, wxT("Right"), wxPoint(l + 6, t + DPt + 36*3), wxDefaultSize, 0, wxT("Right"));
		
		// Right side buttons
		m_JoyShoulderR[i] = new wxTextCtrl(m_pKeys[i], ID_SHOULDER_R, wxT("0"), wxPoint(l + 552, t + 106), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyShoulderR[i]->Enable(false);
		m_JoyButtonA[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTON_A, wxT("0"), wxPoint(l + 552, t + 280), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonA[i]->Enable(false);
		m_JoyButtonB[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTON_B, wxT("0"), wxPoint(l + 552, t + 80), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonB[i]->Enable(false);
		m_JoyButtonX[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTON_X, wxT("0"), wxPoint(l + 552, t + 242), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonX[i]->Enable(false);
		m_JoyButtonY[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTON_Y, wxT("0"), wxPoint(l + 552, t + 171), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonY[i]->Enable(false);
		m_JoyButtonZ[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTON_Z, wxT("0"), wxPoint(l + 552, t + 145), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonZ[i]->Enable(false);
		m_bJoyShoulderR[i] = new wxButton(m_pKeys[i], IDB_SHOULDER_R, wxEmptyString, wxPoint(l + 526, t + 108), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyButtonA[i] = new wxButton(m_pKeys[i], IDB_BUTTON_A, wxEmptyString, wxPoint(l + 526, t + 282), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyButtonB[i] = new wxButton(m_pKeys[i], IDB_BUTTON_B, wxEmptyString, wxPoint(l + 526, t + 82), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyButtonX[i] = new wxButton(m_pKeys[i], IDB_BUTTON_X, wxEmptyString, wxPoint(l + 526, t + 244), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyButtonY[i] = new wxButton(m_pKeys[i], IDB_BUTTON_Y, wxEmptyString, wxPoint(l + 526, t + 173), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyButtonZ[i] = new wxButton(m_pKeys[i], IDB_BUTTON_Z, wxEmptyString, wxPoint(l + 526, t + 147), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);

		// C-buttons
		m_JoyAnalogSubX[i] = new wxTextCtrl(m_pKeys[i], ID_ANALOG_SUB_X, wxT("0"), wxPoint(l + 552, t + 336), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyAnalogSubX[i]->Enable(false);
		m_JoyAnalogSubY[i] = new wxTextCtrl(m_pKeys[i], ID_ANALOG_SUB_Y, wxT("0"), wxPoint(l + 552, t + 373), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyAnalogSubY[i]->Enable(false);
		m_bJoyAnalogSubX[i] = new wxButton(m_pKeys[i], IDB_ANALOG_SUB_X, wxEmptyString, wxPoint(l + 526, t + 338), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyAnalogSubY[i] = new wxButton(m_pKeys[i], IDB_ANALOG_SUB_Y, wxEmptyString, wxPoint(l + 526, t + 375), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_textSubX[i] = new wxStaticText(m_pKeys[i], IDT_ANALOG_SUB_X, wxT("X-axis"), wxPoint(l + 552, t + 321), wxDefaultSize, 0, wxT("X-axis"));
		m_textSubY[i] = new wxStaticText(m_pKeys[i], IDT_ANALOG_SUB_Y, wxT("Y-axis"), wxPoint(l + 552, t + 358), wxDefaultSize, 0, wxT("Y-axis"));
		
		// Start button
		m_bJoyButtonStart[i] = new wxButton(m_pKeys[i], IDB_BUTTONSTART, wxEmptyString, wxPoint(l + 284, t + 365), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_JoyButtonStart[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTONSTART, wxT("0"), wxPoint(l + 220, t + 363), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonStart[i]->Enable(false);
		
		// Website text
		#ifdef _WIN32
		m_textWebsite[i] = new wxStaticText(m_pKeys[i], IDT_WEBSITE, wxT("www.multigesture.net"), wxPoint(l + 400, t + 380), wxDefaultSize, 0, wxT("www.multigesture.net"));
		#else
		m_textWebsite[i] = new wxStaticText(m_Controller[i], IDT_WEBSITE, wxT("www.multigesture.net"), wxPoint(l + 480, t + 418), wxDefaultSize, 0, wxT("www.multigesture.net"));
		#endif


		// --------------------------------------------------------------------
		// Populate Controller sizer
		// -----------------------------
		// Groups
		#ifdef _WIN32
		m_Joyname[i] = new wxComboBox(m_Controller[i], IDC_JOYNAME, arrayStringFor_Joyname[0], wxDefaultPosition, wxSize(476, 21), arrayStringFor_Joyname, wxCB_READONLY);
		m_Joyattach[i] = new wxCheckBox(m_Controller[i], IDC_JOYATTACH, wxT("Controller attached"), wxDefaultPosition, wxSize(109, 25));
		#else
		m_Joyname[i] = new wxComboBox(m_Controller[i], IDC_JOYNAME, arrayStringFor_Joyname[0], wxDefaultPosition, wxSize(450, 25), arrayStringFor_Joyname, 0, wxDefaultValidator, wxT("m_Joyname"));
		m_Joyattach[i] = new wxCheckBox(m_Controller[i], IDC_JOYATTACH, wxT("Controller attached"), wxDefaultPosition, wxSize(140, 25), 0, wxDefaultValidator, wxT("Controller attached"));
		#endif
		m_Joyattach[i]->SetToolTip(wxString::Format(wxT("Decide if Controller %i shall be detected by the game."), i + 1));

		m_gJoyname[i] = new wxStaticBoxSizer (wxHORIZONTAL, m_Controller[i], wxT("Controller:"));
		m_gJoyname[i]->Add(m_Joyname[i], 0, (wxLEFT | wxRIGHT), 5);
		m_gJoyname[i]->Add(m_Joyattach[i], 0, (wxRIGHT | wxLEFT | wxBOTTOM), 1);

		// --------------------------------------------------------------------
		// Populate settings sizer
		// -----------------------------

		// Extra settings members
		m_gGBExtrasettings[i] = new wxGridBagSizer(0, 0);
		m_JoyButtonHalfpress[i] = new wxTextCtrl(m_Controller[i], ID_BUTTONHALFPRESS, wxT("0"), wxDefaultPosition, wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonHalfpress[i]->Enable(false);
		m_bJoyButtonHalfpress[i] = new wxButton(m_Controller[i], IDB_BUTTONHALFPRESS, wxEmptyString, wxPoint(231, 426), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		#ifdef _WIN32
		m_Deadzone[i] = new wxComboBox(m_Controller[i], IDC_DEADZONE, wxEmptyString, wxDefaultPosition, wxSize(59, 21), arrayStringFor_Deadzone, 0, wxDefaultValidator, wxT("m_Deadzone"));
		m_textDeadzone[i] = new wxStaticText(m_Controller[i], IDT_DEADZONE, wxT("Deadzone"), wxDefaultPosition, wxDefaultSize, 0, wxT("Deadzone"));		
		m_textHalfpress[i] = new wxStaticText(m_Controller[i], IDT_BUTTONHALFPRESS, wxT("Half press"), wxDefaultPosition, wxDefaultSize, 0, wxT("Half press"));
		#else
		m_Deadzone[i] = new wxComboBox(m_Controller[i], IDC_DEADZONE, wxEmptyString, wxPoint(167, 398), wxSize(80, 25), arrayStringFor_Deadzone, 0, wxDefaultValidator, wxT("m_Deadzone"));
		m_textDeadzone[i] = new wxStaticText(m_Controller[i], IDT_DEADZONE, wxT("Deadzone"), wxPoint(105, 404), wxDefaultSize, 0, wxT("Deadzone"));		
		m_textHalfpress[i] = new wxStaticText(m_Controller[i], IDT_BUTTONHALFPRESS, wxT("Half press"), wxPoint(105, 428), wxDefaultSize, 0, wxT("Half press"));
		#endif

		// Populate extra settings
		m_gExtrasettings[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("Extra settings"));
		m_gGBExtrasettings[i]->Add(m_textDeadzone[i], wxGBPosition(0, 0), wxGBSpan(1, 1), (wxRIGHT | wxTOP), 2);
		m_gGBExtrasettings[i]->Add(m_Deadzone[i], wxGBPosition(0, 1), wxGBSpan(1, 1), (wxBOTTOM), 2);
		m_gGBExtrasettings[i]->Add(m_textHalfpress[i], wxGBPosition(1, 0), wxGBSpan(1, 1), (wxRIGHT | wxTOP), 2);
		m_gGBExtrasettings[i]->Add(m_JoyButtonHalfpress[i], wxGBPosition(1, 1), wxGBSpan(1, 1), wxALL, 0);
		m_gGBExtrasettings[i]->Add(m_bJoyButtonHalfpress[i], wxGBPosition(1, 2), wxGBSpan(1, 1), (wxLEFT | wxTOP), 2);
		m_gExtrasettings[i]->Add(m_gGBExtrasettings[i], 0, wxEXPAND | wxALL, 3);

		// Populate controller typ
		m_gControllertype[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("Controller type"));
		m_Controltype[i] = new wxComboBox(m_Controller[i], IDC_CONTROLTYPE, arrayStringFor_Controltype[0], wxDefaultPosition, wxDefaultSize, arrayStringFor_Controltype, wxCB_READONLY);
		m_gControllertype[i]->Add(m_Controltype[i], 0, wxEXPAND | wxALL, 3);

		// Populate general settings
		m_gGenSettings[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("Settings") );
		m_CBSaveByID[i] = new wxCheckBox(m_Controller[i], IDC_SAVEBYID, wxT("Save by ID"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
		m_CBShowAdvanced[i] = new wxCheckBox(m_Controller[i], IDC_SHOWADVANCED, wxT("Show advanced settings"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
		m_gGenSettings[i]->Add(m_CBSaveByID[i], 0, wxEXPAND | wxALL, 3);
		m_gGenSettings[i]->Add(m_CBShowAdvanced[i], 0, wxEXPAND | wxALL, 3);
		m_CBSaveByID[i]->SetToolTip(wxString::Format(wxT(
			"Map these settings to the selected controller device instead of to the"
			"\nselected controller number (%i). This may be a more convenient way"
			"\nto save your settings if you have multiple controllers.")
			, i+1
			));

		// Populate settings
		m_sSettings[i] = new wxBoxSizer ( wxHORIZONTAL );
		m_sSettings[i]->Add(m_gExtrasettings[i], 0, wxEXPAND | wxALL, 0);
		m_sSettings[i]->Add(m_gControllertype[i], 0, wxEXPAND | wxLEFT, 5);
		m_sSettings[i]->Add(m_gGenSettings[i], 0, wxEXPAND | wxLEFT, 5);	


		//////////////////////////////////////
		// Populate advanced settings
		// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ

		// Populate input status
		/**/
		
		// Input status text
		m_TStatusIn[i] = new wxStaticText(m_Controller[i], IDT_STATUS_IN, wxT("In"));
		m_TStatusOut[i] = new wxStaticText(m_Controller[i], IDT_STATUS_OUT, wxT("Out"));

		m_gStatusIn[i] = new wxStaticBoxSizer( wxHORIZONTAL, m_Controller[i], wxT("Main-stick (In) (Out)"));
		CreateAdvancedControls(i);
		m_GBAdvancedMainStick[i] = new wxGridBagSizer(0, 0);

		m_GBAdvancedMainStick[i]->Add(m_pInStatus[i], wxGBPosition(0, 0), wxGBSpan(1, 1), wxALL, 0);
		m_GBAdvancedMainStick[i]->Add(m_pOutStatus[i], wxGBPosition(0, 1), wxGBSpan(1, 1), wxLEFT, 5);
		m_GBAdvancedMainStick[i]->Add(m_TStatusIn[i], wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL, 0);
		m_GBAdvancedMainStick[i]->Add(m_TStatusOut[i], wxGBPosition(1, 1), wxGBSpan(1, 1), wxLEFT, 5);

		m_gStatusIn[i]->Add(m_GBAdvancedMainStick[i], 0, wxLEFT, 5);

		// Populate input status settings

		// The label
		m_STDiagonal[i] = new wxStaticText(m_Controller[i], IDT_MAINSTICK_DIAGONAL, wxT("Diagonal"));
		m_STDiagonal[i]->SetToolTip(wxT(
			"To produce a smooth circle in the 'Out' window you have to manually set"
			"\nyour diagonal values here from the 'In' window."
			));

		// The drop down menu
		m_gStatusInSettings[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("Main-stick settings"));
		m_gStatusInSettingsH[i] = new wxBoxSizer(wxHORIZONTAL);
		wxArrayString asStatusInSet;
			asStatusInSet.Add(wxT("100%"));
			asStatusInSet.Add(wxT("95%"));
			asStatusInSet.Add(wxT("90%"));
			asStatusInSet.Add(wxT("85%"));
		m_CoBDiagonal[i] = new wxComboBox(m_Controller[i], IDCB_MAINSTICK_DIAGONAL, asStatusInSet[0], wxDefaultPosition, wxDefaultSize, asStatusInSet, wxCB_READONLY);

		// The checkbox
		m_CBS_to_C[i] = new wxCheckBox(m_Controller[i], IDCB_MAINSTICK_S_TO_C, wxT("Square-to-circle"));

		m_gStatusInSettings[i]->Add(m_CBS_to_C[i], 0, (wxALL), 4);
		m_gStatusInSettings[i]->Add(m_gStatusInSettingsH[i], 0, (wxLEFT | wxRIGHT | wxBOTTOM), 4);		

		m_gStatusInSettingsH[i]->Add(m_STDiagonal[i], 0, wxTOP, 3);
		m_gStatusInSettingsH[i]->Add(m_CoBDiagonal[i], 0, wxLEFT, 3);
		

		

		//////////////////////////////////////
		// Populate sizers
		// ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ

		// --------------------------------------------------------------------
		// Populate main left sizer
		// -----------------------------
		m_sMainLeft[i] = new wxBoxSizer(wxVERTICAL);
		m_sMainLeft[i]->Add(m_gJoyname[i], 0, wxEXPAND | (wxALL), 5);
		m_sMainLeft[i]->Add(m_sKeys[i], 1, wxEXPAND | (wxLEFT | wxRIGHT), 5);
		m_sMainLeft[i]->Add(m_sSettings[i], 0, wxEXPAND | (wxALL), 5);

		// --------------------------------------------------------------------
		// Populate main right sizer
		// -----------------------------
		m_sMainRight[i] = new wxBoxSizer(wxVERTICAL);
		m_sMainRight[i]->Add(m_gStatusIn[i], 0, wxEXPAND | (wxLEFT), 2);
		m_sMainRight[i]->Add(m_gStatusInSettings[i], 0, wxEXPAND | (wxLEFT | wxTOP), 2);

		// --------------------------------------------------------------------
		// Populate main sizer
		// -----------------------------
		m_sMain[i] = new wxBoxSizer(wxHORIZONTAL);
		m_sMain[i]->Add(m_sMainLeft[i], 0, wxEXPAND | (wxALL), 0);
		m_sMain[i]->Add(m_sMainRight[i], 0, wxEXPAND | (wxRIGHT | wxTOP), 5);
		m_Controller[i]->SetSizer(m_sMain[i]); // Set the main sizer

		// Show or hide it. We have to do this after we add it to its sizer
		m_sMainRight[i]->Show(g_Config.bShowAdvanced);

		// Don't allow these changes when running
		if(emulator_running)
		{
			m_Joyname[i]->Enable(false);
			m_Joyattach[i]->Enable(false);
			m_Controltype[i]->Enable(false);			
		}
		
		// Set dialog items from saved values
		SetControllerAll(i);

		// Update GUI
		DoEnableDisable(i);
	} // end of loop

	
	// --------------------------------------------------------------------
	// Populate buttons sizer.
	// -----------------------------
	wxBoxSizer * m_sButtons = new wxBoxSizer(wxHORIZONTAL);
	m_sButtons->Add(m_About, 0, (wxBOTTOM), 0);
	m_sButtons->AddStretchSpacer(1);
	m_sButtons->Add(m_OK, 0, wxALIGN_RIGHT | (wxBOTTOM), 0);
	m_sButtons->Add(m_Cancel, 0, wxALIGN_RIGHT | (wxLEFT), 5);


	// --------------------------------------------------------------------
	// Populate master sizer.
	// -----------------------------
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
	m_MainSizer->Add(m_Notebook, 0, wxEXPAND | wxALL, 5);
	m_MainSizer->Add(m_sButtons, 1, wxEXPAND | ( wxLEFT | wxRIGHT | wxBOTTOM), 5);
	this->SetSizer(m_MainSizer);

	// --------------------------------------------------------------------
	// Debugging
	// -----------------------------
	//m_pStatusBar = new wxStaticText(this, IDT_DEBUGGING, wxT("Debugging"), wxPoint(100, 510), wxDefaultSize);	
	//m_pStatusBar2 = new wxStaticText(this, IDT_DEBUGGING2, wxT("Debugging2"), wxPoint(100, 530), wxDefaultSize);	
	//m_pStatusBar->SetLabel(wxString::Format("Debugging text"));

	// --------------------------------------------------------------------
	// Set window size
	// -----------------------------
	SizeWindow();
	Center();
}

void ConfigBox::SizeWindow()
{
	SetClientSize(m_MainSizer->GetMinSize().GetWidth(), m_MainSizer->GetMinSize().GetHeight());
}
