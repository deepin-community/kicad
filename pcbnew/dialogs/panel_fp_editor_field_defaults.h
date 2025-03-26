/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <memory>

#include <panel_fp_editor_field_defaults_base.h>

class BOARD_DESIGN_SETTINGS;
class PAGED_DIALOG;
class FOOTPRINT_EDITOR_SETTINGS;
class UNITS_PROVIDER;


class PANEL_FP_EDITOR_FIELD_DEFAULTS : public PANEL_FP_EDITOR_FIELD_DEFAULTS_BASE
{
public:
    PANEL_FP_EDITOR_FIELD_DEFAULTS( wxWindow* aParent, UNITS_PROVIDER* aUnitsProvider );
    ~PANEL_FP_EDITOR_FIELD_DEFAULTS() override;

    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    void ResetPanel() override;

private:
    virtual void OnAddTextItem( wxCommandEvent& event ) override;
    virtual void OnDeleteTextItem( wxCommandEvent& event ) override;
    virtual void OnAddLayerItem( wxCommandEvent& event ) override;
    virtual void OnDeleteLayerItem( wxCommandEvent& event ) override;
    virtual void onLayerChange( wxGridEvent& event ) override;

    bool Show( bool aShow ) override;

    void loadFPSettings( const FOOTPRINT_EDITOR_SETTINGS* aCfg );

    bool isLayerAvailable( int aLayerId ) const;
    int getNextAvailableLayer() const;


private:
    UNITS_PROVIDER*        m_unitProvider;
    BOARD_DESIGN_SETTINGS& m_designSettings;
};
