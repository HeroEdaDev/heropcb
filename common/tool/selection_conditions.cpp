/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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


#include <tool/selection.h>
#include <tool/selection_conditions.h>

#include <functional>
#include <eda_item.h>

using namespace std::placeholders;


bool SELECTION_CONDITIONS::NotEmpty( const SELECTION& aSelection )
{
    return !aSelection.Empty();
}


bool SELECTION_CONDITIONS::Empty( const SELECTION& aSelection )
{
    return aSelection.Empty();
}


bool SELECTION_CONDITIONS::Idle( const SELECTION& aSelection )
{
    constexpr int busyMask = ( IS_NEW | IS_PASTED | IS_MOVING | IS_RESIZING | IS_DRAGGING );

    return !aSelection.Front() || ( aSelection.Front()->GetEditFlags() & busyMask ) == 0;
}


bool SELECTION_CONDITIONS::IdleSelection( const SELECTION& aSelection )
{
    return ( aSelection.Front() && aSelection.Front()->GetEditFlags() == 0 );
}


SELECTION_CONDITION SELECTION_CONDITIONS::HasType( KICAD_T aType )
{
    return std::bind( &SELECTION_CONDITIONS::hasTypeFunc, _1, aType );
}


SELECTION_CONDITION SELECTION_CONDITIONS::HasTypes( std::vector<KICAD_T> aTypes )
{
    return std::bind( &SELECTION_CONDITIONS::hasTypesFunc, _1, aTypes );
}


SELECTION_CONDITION SELECTION_CONDITIONS::OnlyTypes( std::vector<KICAD_T> aTypes )
{
    return std::bind( &SELECTION_CONDITIONS::onlyTypesFunc, _1, aTypes );
}


SELECTION_CONDITION SELECTION_CONDITIONS::Count( int aNumber )
{
    return std::bind( &SELECTION_CONDITIONS::countFunc, _1, aNumber );
}


SELECTION_CONDITION SELECTION_CONDITIONS::MoreThan( int aNumber )
{
    return std::bind( &SELECTION_CONDITIONS::moreThanFunc, _1, aNumber );
}


SELECTION_CONDITION SELECTION_CONDITIONS::LessThan( int aNumber )
{
    return std::bind( &SELECTION_CONDITIONS::lessThanFunc, _1, aNumber );
}


bool SELECTION_CONDITIONS::hasTypeFunc( const SELECTION& aSelection, KICAD_T aType )
{
    if( aSelection.Empty() )
        return false;

    for( const EDA_ITEM* item : aSelection )
    {
        if( item->Type() == aType )
            return true;
    }

    return false;
}


bool SELECTION_CONDITIONS::hasTypesFunc( const SELECTION& aSelection, std::vector<KICAD_T> aTypes )
{
    if( aSelection.Empty() )
        return false;

    for( const EDA_ITEM* item : aSelection )
    {
        if( item->IsType( aTypes ) )
            return true;
    }

    return false;
}


bool SELECTION_CONDITIONS::onlyTypesFunc( const SELECTION& aSelection, std::vector<KICAD_T> aTypes )
{
    if( aSelection.Empty() )
        return false;

    for( const EDA_ITEM* item : aSelection )
    {
        if( !item->IsType( aTypes ) )
            return false;
    }

    return true;
}


bool SELECTION_CONDITIONS::countFunc( const SELECTION& aSelection, int aNumber )
{
    return aSelection.Size() == aNumber;
}


bool SELECTION_CONDITIONS::moreThanFunc( const SELECTION& aSelection, int aNumber )
{
    return aSelection.Size() > aNumber;
}


bool SELECTION_CONDITIONS::lessThanFunc( const SELECTION& aSelection, int aNumber )
{
    return aSelection.Size() < aNumber;
}


SELECTION_CONDITION operator||( const SELECTION_CONDITION& aConditionA,
                                const SELECTION_CONDITION& aConditionB )
{
    return std::bind( &SELECTION_CONDITIONS::orFunc, aConditionA, aConditionB, _1 );
}


SELECTION_CONDITION operator&&( const SELECTION_CONDITION& aConditionA,
                                const SELECTION_CONDITION& aConditionB )
{
    return std::bind( &SELECTION_CONDITIONS::andFunc, aConditionA, aConditionB, _1 );
}


SELECTION_CONDITION operator!( const SELECTION_CONDITION& aCondition )
{
    return std::bind( &SELECTION_CONDITIONS::notFunc, aCondition, _1 );
}


SELECTION_CONDITION operator||( const SELECTION_CONDITION& aConditionA,
                                SELECTION_BOOL aConditionB )
{
    return std::bind( &SELECTION_CONDITIONS::orBoolFunc, aConditionA, std::ref( aConditionB ), _1 );
}

SELECTION_CONDITION operator||( SELECTION_BOOL aConditionA,
                                const SELECTION_CONDITION& aConditionB )
{
    return aConditionB || aConditionA;
}


SELECTION_CONDITION operator&&( const SELECTION_CONDITION& aConditionA,
                                SELECTION_BOOL aConditionB )
{
    return std::bind( &SELECTION_CONDITIONS::andBoolFunc, aConditionA, std::ref( aConditionB ), _1 );
}

SELECTION_CONDITION operator&&( SELECTION_BOOL aConditionA,
                                const SELECTION_CONDITION& aConditionB )
{
    return aConditionB && aConditionA;
}
