/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
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

#include <dialogs/dialog_assign_netclass.h>
#include <widgets/wx_html_report_box.h>
#include <project.h>
#include <project/project_file.h>
#include <project/net_settings.h>
#include <eda_base_frame.h>


DIALOG_ASSIGN_NETCLASS::DIALOG_ASSIGN_NETCLASS( EDA_BASE_FRAME* aParent, const wxString aNetName,
                                                const std::set<wxString> aCandidateNetNames,
                                                const std::function<void( const std::vector<wxString>& )>& aPreviewer ) :
        DIALOG_ASSIGN_NETCLASS_BASE( aParent ),
        m_frame( aParent ),
        m_netCandidates( aCandidateNetNames ),
        m_previewer( aPreviewer )
{
    std::shared_ptr<NET_SETTINGS>& netSettings = m_frame->Prj().GetProjectFile().m_NetSettings;

    m_netclassCtrl->Append( NETCLASS::Default );

    for( const auto& [name, netclass] : netSettings->GetNetclasses() )
        m_netclassCtrl->Append( name );

    if( m_netclassCtrl->GetCount() > 1 )
        m_netclassCtrl->SetSelection( 1 );  // First non-Default netclass
    else
        m_netclassCtrl->SetSelection( 0 );  // Default netclass

    m_patternCtrl->SetValue( aNetName );
    m_matchingNets->SetFont( KIUI::GetInfoFont( this ) );
    m_info->SetFont( KIUI::GetInfoFont( this ).Italic() );

    // @translate the string below.
    if( aParent->GetFrameType() == FRAME_PCB_EDITOR )
        m_info->SetLabel( wxT( "Note: complete netclass assignments can be edited in Board "
                               "Setup > Project." ) );

    SetupStandardButtons();

    finishDialogSettings();
}


bool DIALOG_ASSIGN_NETCLASS::TransferDataFromWindow()
{
    std::shared_ptr<NET_SETTINGS>& netSettings = m_frame->Prj().GetProjectFile().m_NetSettings;

    if( m_patternCtrl->GetValue().IsEmpty() )
        return true;

    netSettings->SetNetclassPatternAssignment( m_patternCtrl->GetValue(),
                                               m_netclassCtrl->GetStringSelection() );

    return true;
}


void DIALOG_ASSIGN_NETCLASS::onPatternText( wxCommandEvent& aEvent )
{
    wxString pattern = m_patternCtrl->GetValue();

    if( pattern != m_lastPattern )
    {
        m_matchingNets->Clear();

        std::vector<wxString> matchingNetNames;

        if( !pattern.IsEmpty() )
        {
            EDA_COMBINED_MATCHER matcher( pattern, CTX_NETCLASS );

            m_matchingNets->Report( _( "<b>Currently matching nets:</b>" ) );

            for( const wxString& net : m_netCandidates )
            {
                if( matcher.StartsWith( net ) )
                {
                    m_matchingNets->Report( net );
                    matchingNetNames.push_back( net );
                }
            }
        }

        m_matchingNets->Flush();

        m_previewer( matchingNetNames );
        m_lastPattern = pattern;
    }
}


