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


#pragma once

#include <board.h>
#include <widgets/unit_binder.h>
#include <widgets/paged_dialog.h>

#include <panel_setup_dimensions_base.h>

class BOARD_DESIGN_SETTINGS;


class PANEL_SETUP_DIMENSIONS : public PANEL_SETUP_DIMENSIONS_BASE
{
public:
    PANEL_SETUP_DIMENSIONS( wxWindow* aParentWindow, UNITS_PROVIDER& aUnitsProvider,
                            BOARD_DESIGN_SETTINGS& aBrdSettings );
    ~PANEL_SETUP_DIMENSIONS( ) override;

    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    void LoadFromSettings( const BOARD_DESIGN_SETTINGS& aBrdSettings );

private:
    void onUnitsChanged( wxCommandEvent& aEvent );

private:
    wxWindow*               m_parentWindow;
    UNITS_PROVIDER&         m_unitsProvider;
    BOARD_DESIGN_SETTINGS*  m_brdSettings;
    UNIT_BINDER             m_arrowLength;
    UNIT_BINDER             m_extensionOffset;
};
