/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2015  Cirilo Bernardo
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <pcb_edit_frame.h>
#include <board.h>
#include <widgets/text_ctrl_eval.h>
#include <dialog_export_idf.h>
#include <pcbnew_settings.h>
#include <project/project_file.h> // LAST_PATH_TYPE
#include <kidialog.h>


DIALOG_EXPORT_IDF3::DIALOG_EXPORT_IDF3( PCB_EDIT_FRAME* aEditFrame ) :
        DIALOG_EXPORT_IDF3_BASE( aEditFrame ),
        m_editFrame( aEditFrame )
{
    SetFocus();

    auto cfg = m_editFrame->GetPcbNewSettings();

    m_idfThouOpt = cfg->m_ExportIdf.units_mils;
    m_rbUnitSelection->SetSelection( m_idfThouOpt ? 1 : 0 );
    m_AutoAdjust = cfg->m_ExportIdf.auto_adjust;
    m_RefUnits = cfg->m_ExportIdf.ref_units;
    m_XRef = cfg->m_ExportIdf.ref_x;
    m_YRef = cfg->m_ExportIdf.ref_y;

    m_cbRemoveUnspecified->SetValue( cfg->m_ExportIdf.no_unspecified );
    m_cbRemoveDNP->SetValue( cfg->m_ExportIdf.no_dnp );

    m_cbAutoAdjustOffset->SetValue( m_AutoAdjust );
    m_cbAutoAdjustOffset->Bind( wxEVT_CHECKBOX, &DIALOG_EXPORT_IDF3::OnAutoAdjustOffset, this );

    m_IDF_RefUnitChoice->SetSelection( m_RefUnits );
    wxString tmpStr;
    tmpStr << m_XRef;
    m_IDF_Xref->SetValue( tmpStr );
    tmpStr = wxT( "" );
    tmpStr << m_YRef;
    m_IDF_Yref->SetValue( tmpStr );

    if( m_AutoAdjust )
    {
        m_IDF_RefUnitChoice->Enable( false );
        m_IDF_Xref->Enable( false );
        m_IDF_Yref->Enable( false );
    }
    else
    {
        m_IDF_RefUnitChoice->Enable( true );
        m_IDF_Xref->Enable( true );
        m_IDF_Yref->Enable( true );
    }

    SetupStandardButtons();

    // Now all widgets have the size fixed, call FinishDialogSettings
    finishDialogSettings();
}


DIALOG_EXPORT_IDF3::~DIALOG_EXPORT_IDF3()
{
    m_idfThouOpt = m_rbUnitSelection->GetSelection() == 1;

    PCBNEW_SETTINGS* cfg = nullptr;

    try
    {
        cfg = m_editFrame->GetPcbNewSettings();
    }
    catch( const std::runtime_error& e )
    {
        wxFAIL_MSG( e.what() );
    }

    if( cfg )
    {
        cfg->m_ExportIdf.units_mils = m_idfThouOpt;
        cfg->m_ExportIdf.auto_adjust = m_AutoAdjust;
        cfg->m_ExportIdf.ref_units = m_RefUnits;
        cfg->m_ExportIdf.ref_x = m_XRef;
        cfg->m_ExportIdf.ref_y = m_YRef;

        cfg->m_ExportIdf.no_unspecified = m_cbRemoveUnspecified->GetValue();
        cfg->m_ExportIdf.no_dnp = m_cbRemoveDNP->GetValue();
    }
}


void DIALOG_EXPORT_IDF3::OnAutoAdjustOffset( wxCommandEvent& event )
{
    if( GetAutoAdjustOffset() )
    {
        m_IDF_RefUnitChoice->Enable( false );
        m_IDF_Xref->Enable( false );
        m_IDF_Yref->Enable( false );
    }
    else
    {
        m_IDF_RefUnitChoice->Enable( true );
        m_IDF_Xref->Enable( true );
        m_IDF_Yref->Enable( true );
    }

    event.Skip();
}


bool DIALOG_EXPORT_IDF3::TransferDataFromWindow()
{
    wxFileName fn = m_filePickerIDF->GetPath();

    if( fn.FileExists() )
    {
        wxString msg = wxString::Format( _( "File %s already exists." ), fn.GetPath() );
        KIDIALOG dlg( this, msg, _( "Confirmation" ), wxOK | wxCANCEL | wxICON_WARNING );
        dlg.SetOKLabel( _( "Overwrite" ) );
        dlg.DoNotShowCheckbox( __FILE__, __LINE__ );

        return ( dlg.ShowModal() == wxID_OK );
    }

    return true;
}


void PCB_EDIT_FRAME::OnExportIDF3( wxCommandEvent& event )
{
    // Build default output file name
    wxString path = GetLastPath( LAST_PATH_IDF );

    if( path.IsEmpty() )
    {
        wxFileName brdFile = GetBoard()->GetFileName();
        brdFile.SetExt( wxT( "emn" ) );
        path = brdFile.GetFullPath();
    }

    DIALOG_EXPORT_IDF3 dlg( this );
    dlg.FilePicker()->SetPath( path );

    if ( dlg.ShowModal() != wxID_OK )
        return;

    bool thou = dlg.GetThouOption();
    double aXRef;
    double aYRef;

    if( dlg.GetAutoAdjustOffset() )
    {
        BOX2I bbox = GetBoard()->GetBoardEdgesBoundingBox();

        aXRef = bbox.Centre().x * pcbIUScale.MM_PER_IU;
        aYRef = bbox.Centre().y * pcbIUScale.MM_PER_IU;
    }
    else
    {
        aXRef = dlg.GetXRef();
        aYRef = dlg.GetYRef();

        if( dlg.GetRefUnitsChoice() == 1 )
        {
            // selected reference unit is in inches
            aXRef *= 25.4;
            aYRef *= 25.4;
        }

    }

    wxBusyCursor dummy;

    wxString fullFilename = dlg.FilePicker()->GetPath();
    SetLastPath( LAST_PATH_IDF, fullFilename );

    if( !Export_IDF3( GetBoard(), fullFilename, thou, aXRef, aYRef, !dlg.GetNoUnspecifiedOption(),
                      !dlg.GetNoDNPOption() ) )
    {
        wxString msg = wxString::Format( _( "Failed to create file '%s'." ), fullFilename );
        wxMessageBox( msg );
        return;
    }
}

