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

#include "junction_helpers.h"

#include <sch_line.h>

using namespace JUNCTION_HELPERS;

POINT_INFO JUNCTION_HELPERS::AnalyzePoint( const EE_RTREE& aItems, const VECTOR2I& aPosition,
                                           bool aBreakCrossings )
{
    enum layers
    {
        WIRES = 0,
        BUSES
    };

    POINT_INFO info{};
    info.hasBusEntry = false;
    info.hasExplicitJunctionDot = false;
    info.isJunction = false;
    info.hasBusEntryToMultipleWires = false;

    bool                         breakLines[2] = { false };
    std::unordered_set<int>      exitAngles[2];
    std::vector<const SCH_LINE*> midPointLines[2];

    // A pin at 90° still shouldn't match a line at 90° so just give pins unique numbers
    int uniqueAngle = 10000;

    for( const SCH_ITEM* item : aItems.Overlapping( aPosition ) )
    {
        if( item->GetEditFlags() & STRUCT_DELETED )
            continue;

        switch( item->Type() )
        {
        case SCH_JUNCTION_T:
            if( item->HitTest( aPosition, -1 ) )
                info.hasExplicitJunctionDot = true;

            break;

        case SCH_LINE_T:
        {
            const SCH_LINE* line = static_cast<const SCH_LINE*>( item );
            int             layer;

            if( line->GetStartPoint() == line->GetEndPoint() )
                break;
            else if( line->GetLayer() == LAYER_WIRE )
                layer = WIRES;
            else if( line->GetLayer() == LAYER_BUS )
                layer = BUSES;
            else
                break;

            if( line->IsConnected( aPosition ) )
            {
                breakLines[layer] = true;
                exitAngles[layer].insert( line->GetAngleFrom( aPosition ) );
            }
            else if( line->HitTest( aPosition, -1 ) )
            {
                if( aBreakCrossings )
                    breakLines[layer] = true;

                // Defer any line midpoints until we know whether or not we're breaking them
                midPointLines[layer].push_back( line );
            }
        }
        break;

        case SCH_BUS_WIRE_ENTRY_T:
            if( item->IsConnected( aPosition ) )
            {
                breakLines[BUSES] = true;
                exitAngles[BUSES].insert( uniqueAngle++ );
                breakLines[WIRES] = true;
                exitAngles[WIRES].insert( uniqueAngle++ );
                info.hasBusEntry = true;
            }

            break;

        case SCH_SYMBOL_T:
        case SCH_SHEET_T:
            if( item->IsConnected( aPosition ) )
            {
                breakLines[WIRES] = true;
                exitAngles[WIRES].insert( uniqueAngle++ );
            }

            break;

        default: break;
        }
    }

    for( int layer : { WIRES, BUSES } )
    {
        if( breakLines[layer] )
        {
            for( const SCH_LINE* line : midPointLines[layer] )
            {
                exitAngles[layer].insert( line->GetAngleFrom( aPosition ) );
                exitAngles[layer].insert( line->GetReverseAngleFrom( aPosition ) );
            }
        }
    }

    if( info.hasBusEntry )
    {
        // The bus entry and one wire is 2 wires, and the one entry is exactly one bus
        // Any more wires must be multiple wires, but any more buses means a wire
        // crossing at the bus entry root.
        info.hasBusEntryToMultipleWires =
                exitAngles[WIRES].size() > 2 && exitAngles[BUSES].size() == 1;
    }

    // Any three things of the same type is a junction of some sort
    info.isJunction = exitAngles[WIRES].size() >= 3 || exitAngles[BUSES].size() >= 3;

    return info;
}