///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.0.0-0-g0efcecf)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/filepicker.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/radiobox.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_EXPORT_3DFILE_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_EXPORT_3DFILE_BASE : public DIALOG_SHIM
{
	private:

	protected:
		wxStaticText* m_staticText1;
		wxFilePickerCtrl* m_filePicker;
		wxStaticText* m_staticText3;
		wxTextCtrl* m_SubdirNameCtrl;
		wxRadioBox* m_rbCoordOrigin;
		wxStaticText* m_staticText6;
		wxStaticText* m_staticText61;
		wxChoice* m_VRML_RefUnitChoice;
		wxStaticText* m_staticText4;
		wxTextCtrl* m_VRML_Xref;
		wxStaticText* m_staticText5;
		wxTextCtrl* m_VRML_Yref;
		wxRadioBox* m_rbSelectUnits;
		wxCheckBox* m_cbCopyFiles;
		wxCheckBox* m_cbUseRelativePaths;
		wxStdDialogButtonSizer* m_sdbSizer;
		wxButton* m_sdbSizerOK;
		wxButton* m_sdbSizerCancel;

		// Virtual event handlers, override them in your derived class
		virtual void OnUpdateUseRelativePath( wxUpdateUIEvent& event ) { event.Skip(); }


	public:

		DIALOG_EXPORT_3DFILE_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("VRML Export Options"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );

		~DIALOG_EXPORT_3DFILE_BASE();

};

