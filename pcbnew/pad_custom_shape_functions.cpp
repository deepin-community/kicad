/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 1992-2023 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <board.h>
#include <board_design_settings.h>
#include <board_item.h>
#include <pcb_shape.h>
#include <pad.h>
#include <convert_basic_shapes_to_polygon.h>
#include <geometry/shape_rect.h>


/*
 * Has meaning only for free shape pads.
 * add a free shape to the shape list.
 * the shape is a polygon (can be with thick outline), segment, circle or arc
 */

void PAD::AddPrimitivePoly( const SHAPE_POLY_SET& aPoly, int aThickness, bool aFilled )
{
    // If aPoly has holes, convert it to a polygon with no holes.
    SHAPE_POLY_SET poly_no_hole;
    poly_no_hole.Append( aPoly );

    if( poly_no_hole.HasHoles() )
        poly_no_hole.Fracture( SHAPE_POLY_SET::PM_STRICTLY_SIMPLE );

    // There should never be multiple shapes, but if there are, we split them into
    // primitives so that we can edit them both.
    for( int ii = 0; ii < poly_no_hole.OutlineCount(); ++ii )
    {
        SHAPE_POLY_SET poly_outline( poly_no_hole.COutline( ii ) );
        PCB_SHAPE* item = new PCB_SHAPE();
        item->SetShape( SHAPE_T::POLY );
        item->SetFilled( aFilled );
        item->SetPolyShape( poly_outline );
        item->SetStroke( STROKE_PARAMS( aThickness, LINE_STYLE::SOLID ) );
        item->SetParent( this );
        m_editPrimitives.emplace_back( item );
    }

    SetDirty();
}


void PAD::AddPrimitivePoly( const std::vector<VECTOR2I>& aPoly, int aThickness, bool aFilled )
{
    PCB_SHAPE* item = new PCB_SHAPE( nullptr, SHAPE_T::POLY );
    item->SetFilled( aFilled );
    item->SetPolyPoints( aPoly );
    item->SetStroke( STROKE_PARAMS( aThickness, LINE_STYLE::SOLID ) );
    item->SetParent( this );
    m_editPrimitives.emplace_back( item );
    SetDirty();
}


void PAD::ReplacePrimitives( const std::vector<std::shared_ptr<PCB_SHAPE>>& aPrimitivesList )
{
    // clear old list
    DeletePrimitivesList();

    // Import to the given shape list
    if( aPrimitivesList.size() )
        AppendPrimitives( aPrimitivesList );

    SetDirty();
}


void PAD::AppendPrimitives( const std::vector<std::shared_ptr<PCB_SHAPE>>& aPrimitivesList )
{
    // Add duplicates of aPrimitivesList to the pad primitives list:
    for( const std::shared_ptr<PCB_SHAPE>& prim : aPrimitivesList )
        AddPrimitive( new PCB_SHAPE( *prim ) );

    SetDirty();
}


void PAD::AddPrimitive( PCB_SHAPE* aPrimitive )
{
    aPrimitive->SetParent( this );
    m_editPrimitives.emplace_back( aPrimitive );

    SetDirty();
}


// clear the basic shapes list and associated data
void PAD::DeletePrimitivesList()
{
    m_editPrimitives.clear();

    SetDirty();
}


void PAD::addPadPrimitivesToPolygon( SHAPE_POLY_SET* aMergedPolygon, int aError,
                                     ERROR_LOC aErrorLoc ) const
{
    SHAPE_POLY_SET polyset;

    for( const std::shared_ptr<PCB_SHAPE>& primitive : m_editPrimitives )
    {
        if( !primitive->IsProxyItem() )
            primitive->TransformShapeToPolygon( polyset, UNDEFINED_LAYER, 0, aError, aErrorLoc );
    }

    polyset.Simplify( SHAPE_POLY_SET::PM_FAST );

    // Merge all polygons with the initial pad anchor shape
    if( polyset.OutlineCount() )
    {
        aMergedPolygon->BooleanAdd( polyset, SHAPE_POLY_SET::PM_STRICTLY_SIMPLE );
        aMergedPolygon->Fracture( SHAPE_POLY_SET::PM_STRICTLY_SIMPLE );
    }
}

void PAD::MergePrimitivesAsPolygon( SHAPE_POLY_SET* aMergedPolygon, ERROR_LOC aErrorLoc ) const
{
    const BOARD* board = GetBoard();
    int          maxError = board ? board->GetDesignSettings().m_MaxError : ARC_HIGH_DEF;

    aMergedPolygon->RemoveAllContours();

    // Add the anchor pad shape in aMergedPolygon, others in aux_polyset:
    // The anchor pad is always at 0,0
    switch( GetAnchorPadShape() )
    {
    case PAD_SHAPE::RECTANGLE:
    {
        SHAPE_RECT rect( -GetSize().x / 2, -GetSize().y / 2, GetSize().x, GetSize().y );
        aMergedPolygon->AddOutline( rect.Outline() );
    }
        break;

    default:
    case PAD_SHAPE::CIRCLE:
        TransformCircleToPolygon( *aMergedPolygon, VECTOR2I( 0, 0 ), GetSize().x / 2, maxError,
                                  aErrorLoc );
        break;
    }

    addPadPrimitivesToPolygon( aMergedPolygon, maxError, aErrorLoc );
}
