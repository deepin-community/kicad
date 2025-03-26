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

#include <utility>

#include <component_class_manager.h>


void COMPONENT_CLASS::AddConstituentClass( COMPONENT_CLASS* componentClass )
{
    m_constituentClasses.push_back( componentClass );
}


bool COMPONENT_CLASS::ContainsClassName( const wxString& className ) const
{
    if( m_constituentClasses.size() == 0 )
        return false;

    if( m_constituentClasses.size() == 1 )
        return m_name == className;

    return std::any_of( m_constituentClasses.begin(), m_constituentClasses.end(),
                        [&className]( const COMPONENT_CLASS* testClass )
                        {
                            return testClass->GetFullName() == className;
                        } );
}


wxString COMPONENT_CLASS::GetName() const
{
    if( m_constituentClasses.size() == 0 )
        return wxT( "<None>" );

    if( m_constituentClasses.size() == 1 )
        return m_name;

    wxASSERT( m_constituentClasses.size() >= 2 );

    wxString name;

    if( m_constituentClasses.size() == 2 )
    {
        name.Printf( _( "%s and %s" ), m_constituentClasses[0]->GetName(),
                     m_constituentClasses[1]->GetName() );
    }
    else if( m_constituentClasses.size() == 3 )
    {
        name.Printf( _( "%s, %s and %s" ), m_constituentClasses[0]->GetName(),
                     m_constituentClasses[1]->GetName(), m_constituentClasses[2]->GetName() );
    }
    else if( m_constituentClasses.size() > 3 )
    {
        name.Printf( _( "%s, %s and %d more" ), m_constituentClasses[0]->GetName(),
                     m_constituentClasses[1]->GetName(),
                     static_cast<int>( m_constituentClasses.size() - 2 ) );
    }

    return name;
}


bool COMPONENT_CLASS::IsEmpty() const
{
    return m_constituentClasses.size() == 0;
}


COMPONENT_CLASS_MANAGER::COMPONENT_CLASS_MANAGER()
{
    m_noneClass = std::make_unique<COMPONENT_CLASS>( wxEmptyString );
}


COMPONENT_CLASS* COMPONENT_CLASS_MANAGER::GetEffectiveComponentClass(
        const std::unordered_set<wxString>& classNames )
{
    if( classNames.size() == 0 )
        return m_noneClass.get();

    // Lambda to handle finding constituent component classes. This first checks the cache,
    // and if found moves the class to the primary classes map. If not found, it either returns
    // an existing class in the primary list or creates a new class.
    auto getOrCreateClass = [this]( const wxString& className )
    {
        if( m_classesCache.count( className ) )
        {
            auto existingClass = m_classesCache.extract( className );
            m_classes.insert( std::move( existingClass ) );
        }
        else if( !m_classes.count( className ) )
        {
            std::unique_ptr<COMPONENT_CLASS> newClass =
                    std::make_unique<COMPONENT_CLASS>( className );
            newClass->AddConstituentClass( newClass.get() );
            m_classes[className] = std::move( newClass );
        }

        return m_classes[className].get();
    };

    // Handle single-assignment component classes
    if( classNames.size() == 1 )
        return getOrCreateClass( *classNames.begin() );

    // Handle composite component classes
    std::vector<wxString> sortedClassNames( classNames.begin(), classNames.end() );

    std::sort( sortedClassNames.begin(), sortedClassNames.end(),
               []( const wxString& str1, const wxString& str2 )
               {
                   return str1.Cmp( str2 ) < 0;
               } );

    wxString fullName = GetFullClassNameForConstituents( sortedClassNames );

    if( m_effectiveClassesCache.count( fullName ) )
    {
        // The effective class was previously constructed - copy it across to the new live map
        auto             existingClass = m_effectiveClassesCache.extract( fullName );
        COMPONENT_CLASS* effClass = existingClass.mapped().get();
        m_effectiveClasses.insert( std::move( existingClass ) );

        // Ensure that all constituent component classes are copied to the live map
        for( COMPONENT_CLASS* constClass : effClass->GetConstituentClasses() )
        {
            if( m_classesCache.count( constClass->GetFullName() ) )
            {
                auto constClassNode = m_classesCache.extract( constClass->GetFullName() );
                m_classes.insert( std::move( constClassNode ) );
            }
        }
    }
    else if( !m_effectiveClasses.count( fullName ) )
    {
        // The effective class was not previously constructed
        std::unique_ptr<COMPONENT_CLASS> effClass = std::make_unique<COMPONENT_CLASS>( fullName );

        for( const wxString& className : sortedClassNames )
            effClass->AddConstituentClass( getOrCreateClass( className ) );

        m_effectiveClasses[fullName] = std::move( effClass );
    }

    return m_effectiveClasses[fullName].get();
}


void COMPONENT_CLASS_MANAGER::InitNetlistUpdate()
{
    m_classesCache = std::move( m_classes );
    m_effectiveClassesCache = std::move( m_effectiveClasses );
}


void COMPONENT_CLASS_MANAGER::FinishNetlistUpdate()
{
    m_classesCache.clear();
    m_effectiveClassesCache.clear();
}


wxString COMPONENT_CLASS_MANAGER::GetFullClassNameForConstituents(
        const std::unordered_set<wxString>& classNames )
{
    std::vector<wxString> sortedClassNames( classNames.begin(), classNames.end() );

    std::sort( sortedClassNames.begin(), sortedClassNames.end(),
               []( const wxString& str1, const wxString& str2 )
               {
                   return str1.Cmp( str2 ) < 0;
               } );

    return GetFullClassNameForConstituents( sortedClassNames );
}


wxString
COMPONENT_CLASS_MANAGER::GetFullClassNameForConstituents( const std::vector<wxString>& classNames )
{
    if( classNames.size() == 0 )
        return wxEmptyString;

    wxString fullName = classNames[0];

    for( std::size_t i = 1; i < classNames.size(); ++i )
    {
        fullName += ",";
        fullName += classNames[i];
    }

    return fullName;
}
