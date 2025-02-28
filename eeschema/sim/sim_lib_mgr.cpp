/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2022 Mikolaj Wielgus
 * Copyright (C) 2022 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * https://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <pgm_base.h>
#include <string>
#include <string_utils.h>
#include <common.h>
#include <functional>
#include <sch_symbol.h>

// Include simulator headers after wxWidgets headers to avoid conflicts with Windows headers
// (especially on msys2 + wxWidgets 3.0.x)
#include <sim/sim_lib_mgr.h>
#include <sim/sim_library.h>
#include <sim/sim_model.h>
#include <sim/sim_model_ideal.h>

SIM_LIB_MGR::SIM_LIB_MGR( const PROJECT* aPrj ) :
        m_project( aPrj )
{
}


void SIM_LIB_MGR::Clear()
{
    m_libraries.clear();
    m_models.clear();
}


wxString SIM_LIB_MGR::ResolveLibraryPath( const wxString& aLibraryPath, const PROJECT* aProject )
{
    wxString   expandedPath = ExpandEnvVarSubstitutions( aLibraryPath, aProject );
    wxFileName fn( expandedPath );

    if( fn.IsAbsolute() )
        return fn.GetFullPath();

    wxFileName projectFn( aProject ? aProject->AbsolutePath( expandedPath ) : expandedPath );

    if( projectFn.Exists() )
        return projectFn.GetFullPath();

    wxFileName spiceLibFn( expandedPath );
    wxString   spiceLibDir;

    wxGetEnv( wxT( "SPICE_LIB_DIR" ), &spiceLibDir );

    if( !spiceLibDir.IsEmpty() && spiceLibFn.MakeAbsolute( spiceLibDir ) && spiceLibFn.Exists() )
        return spiceLibFn.GetFullPath();

    THROW_IO_ERROR( wxString::Format( _( "Simulation model library not found at '%s' or '%s'" ),
                                      projectFn.GetFullPath(),
                                      spiceLibFn.GetFullPath() ) );
}


std::string SIM_LIB_MGR::ResolveEmbeddedLibraryPath( const std::string& aLibPath, const std::string& aRelativeLib )
{
    wxFileName testPath( aLibPath );
    wxString fullPath( aLibPath );

    if( !testPath.IsAbsolute() && !aRelativeLib.empty() )
    {
        wxString relLib( aRelativeLib );

        try
        {
            relLib = ResolveLibraryPath( relLib, m_project );
        }
        catch( ... )
        {}

        wxFileName fn( relLib );

        testPath.MakeAbsolute( fn.GetPath( true ) );
        fullPath = testPath.GetFullPath();
    }

    try
    {
        wxFileName fn( fullPath );

        if( !fn.Exists() )
            fullPath = aLibPath;

        fullPath = ResolveLibraryPath( fullPath, m_project );
    }
    catch( ... )
    {}

    return fullPath.ToStdString();
}


SIM_LIBRARY& SIM_LIB_MGR::AddLibrary( const wxString& aLibraryPath, REPORTER* aReporter )
{
    // May throw an exception.
    wxString path = ResolveLibraryPath( aLibraryPath, m_project );

    std::function<std::string(const std::string&, const std::string&)> f2 =
            std::bind( &SIM_LIB_MGR::ResolveEmbeddedLibraryPath, this, std::placeholders::_1, std::placeholders::_2 );

    // May throw an exception.
    auto it = m_libraries.try_emplace( path, SIM_LIBRARY::Create( path, aReporter, &f2 ) ).first;
    return *it->second;
}


SIM_LIBRARY& SIM_LIB_MGR::SetLibrary( const wxString& aLibraryPath, REPORTER* aReporter  )
{
    // May throw an exception.
    wxString path = ResolveLibraryPath( aLibraryPath, m_project );

    std::function<std::string(const std::string&, const std::string&)> f2 =
            std::bind( &SIM_LIB_MGR::ResolveEmbeddedLibraryPath, this, std::placeholders::_1, std::placeholders::_2 );

    std::unique_ptr<SIM_LIBRARY> library = SIM_LIBRARY::Create( path, aReporter, &f2 );
    
    Clear();
    m_libraries[path] = std::move( library );
    return *m_libraries.at( path );
}


SIM_MODEL& SIM_LIB_MGR::CreateModel( SIM_MODEL::TYPE aType, const std::vector<LIB_PIN*>& aPins )
{
    m_models.push_back( SIM_MODEL::Create( aType, aPins ) );
    return *m_models.back();
}


SIM_MODEL& SIM_LIB_MGR::CreateModel( const SIM_MODEL& aBaseModel,
                                     const std::vector<LIB_PIN*>& aPins )
{
    m_models.push_back( SIM_MODEL::Create( aBaseModel, aPins ) );
    return *m_models.back();
}


template <typename T>
SIM_MODEL& SIM_LIB_MGR::CreateModel( const SIM_MODEL& aBaseModel,
                                     const std::vector<LIB_PIN*>& aPins,
                                     const std::vector<T>& aFields )
{
    m_models.push_back( SIM_MODEL::Create( aBaseModel, aPins, aFields ) );
    return *m_models.back();
}

template SIM_MODEL& SIM_LIB_MGR::CreateModel( const SIM_MODEL& aBaseModel,
                                              const std::vector<LIB_PIN*>& aPins,
                                              const std::vector<SCH_FIELD>& aFields );
template SIM_MODEL& SIM_LIB_MGR::CreateModel( const SIM_MODEL& aBaseModel,
                                              const std::vector<LIB_PIN*>& aPins,
                                              const std::vector<LIB_FIELD>& aFields );


SIM_LIBRARY::MODEL SIM_LIB_MGR::CreateModel( const SCH_SHEET_PATH* aSheetPath, SCH_SYMBOL& aSymbol )
{
    // Note: currently this creates a resolved model (all Kicad variables references are resolved
    // before building the model).
    //
    // That's not what we want if this is ever called from the Simulation Model Editor (or other
    // editors, but it is what we want if called to generate a netlist or other exported items.


    std::vector<SCH_FIELD> fields;

    for( int i = 0; i < aSymbol.GetFieldCount(); ++i )
    {
        fields.emplace_back( VECTOR2I(), i, &aSymbol, aSymbol.GetFields()[ i ].GetName() );

        if( i == REFERENCE_FIELD )
            fields.back().SetText( aSymbol.GetRef( aSheetPath ) );
        else
            fields.back().SetText( aSymbol.GetFields()[ i ].GetShownText( 0, false ) );
    }

    wxString deviceType;
    wxString modelType;
    wxString modelParams;
    wxString pinMap;
    bool     storeInValue = false;

    // Infer RLC and VI models if they aren't specified
    if( SIM_MODEL::InferSimModel( aSymbol, &fields, true, SIM_VALUE_GRAMMAR::NOTATION::SI,
                                  &deviceType, &modelType, &modelParams, &pinMap ) )
    {
        fields.emplace_back( &aSymbol, -1, SIM_MODEL::DEVICE_TYPE_FIELD );
        fields.back().SetText( deviceType );

        if( !modelType.IsEmpty() )
        {
            fields.emplace_back( &aSymbol, -1, SIM_MODEL::TYPE_FIELD );
            fields.back().SetText( modelType );
        }

        fields.emplace_back( &aSymbol, -1, SIM_MODEL::PARAMS_FIELD );
        fields.back().SetText( modelParams );

        fields.emplace_back( &aSymbol, -1, SIM_MODEL::PINS_FIELD );
        fields.back().SetText( pinMap );

        storeInValue = true;
    }

    std::vector<LIB_PIN*> sourcePins = aSymbol.GetAllLibPins();

    std::sort( sourcePins.begin(), sourcePins.end(),
               []( const LIB_PIN* lhs, const LIB_PIN* rhs )
               {
                   return StrNumCmp( lhs->GetNumber(), rhs->GetNumber(), true ) < 0;
               } );

    SIM_LIBRARY::MODEL model = CreateModel( fields, sourcePins );

    model.model.SetIsStoredInValue( storeInValue );

    return model;
}


template <typename T>
SIM_LIBRARY::MODEL SIM_LIB_MGR::CreateModel( const std::vector<T>& aFields,
                                             const std::vector<LIB_PIN*>& aPins )
{
    std::string libraryPath = SIM_MODEL::GetFieldValue( &aFields, SIM_LIBRARY::LIBRARY_FIELD );
    std::string baseModelName = SIM_MODEL::GetFieldValue( &aFields, SIM_LIBRARY::NAME_FIELD );

    if( libraryPath != "" )
    {
        return CreateModel( libraryPath, baseModelName, aFields, aPins );
    }
    else
    {
        m_models.push_back( SIM_MODEL::Create( aFields, aPins ) );
        return { baseModelName, *m_models.back() };
    }
}

template SIM_LIBRARY::MODEL SIM_LIB_MGR::CreateModel( const std::vector<SCH_FIELD>& aFields,
                                                      const std::vector<LIB_PIN*>& aPins );
template SIM_LIBRARY::MODEL SIM_LIB_MGR::CreateModel( const std::vector<LIB_FIELD>& aFields,
                                                      const std::vector<LIB_PIN*>& aPins );


template <typename T>
SIM_LIBRARY::MODEL SIM_LIB_MGR::CreateModel( const wxString& aLibraryPath,
                                             const std::string& aBaseModelName,
                                             const std::vector<T>& aFields,
                                             const std::vector<LIB_PIN*>& aPins )
{
    wxString     path = ResolveLibraryPath( aLibraryPath, m_project );
    SIM_LIBRARY* library = nullptr;

    std::function<std::string(const std::string&, const std::string&)> f2 =
            std::bind( &SIM_LIB_MGR::ResolveEmbeddedLibraryPath, this, std::placeholders::_1, std::placeholders::_2 );

    try
    {
        auto it = m_libraries.try_emplace( path, SIM_LIBRARY::Create( path, nullptr, &f2 ) ).first;
        library = &*it->second;
    }
    catch( const IO_ERROR& e )
    {
        THROW_IO_ERROR( wxString::Format( _( "Error loading simulation model library '%s': %s" ),
                                          path,
                                          e.What() ) );
    }

    if( aBaseModelName == "" )
    {
        THROW_IO_ERROR( wxString::Format( _( "Error loading simulation model: no '%s' field" ),
                                          SIM_LIBRARY::NAME_FIELD ) );
    }

    SIM_MODEL* baseModel = library->FindModel( aBaseModelName );

    if( !baseModel )
    {
        THROW_IO_ERROR( wxString::Format( _( "Error loading simulation model: could not find "
                                             "base model '%s' in library '%s'" ),
                                          aBaseModelName,
                                          path ) );
    }

    m_models.push_back( SIM_MODEL::Create( *baseModel, aPins, aFields ) );

    return { aBaseModelName, *m_models.back() };
}


void SIM_LIB_MGR::SetModel( int aIndex, std::unique_ptr<SIM_MODEL> aModel )
{
    m_models.at( aIndex ) = std::move( aModel );
}


std::map<wxString, std::reference_wrapper<const SIM_LIBRARY>> SIM_LIB_MGR::GetLibraries() const
{
    std::map<wxString, std::reference_wrapper<const SIM_LIBRARY>> libraries;

    for( auto& [path, library] : m_libraries )
        libraries.try_emplace( path, *library );

    return libraries;
}


std::vector<std::reference_wrapper<SIM_MODEL>> SIM_LIB_MGR::GetModels() const
{
    std::vector<std::reference_wrapper<SIM_MODEL>> models;

    for( const std::unique_ptr<SIM_MODEL>& model : m_models )
        models.emplace_back( *model );

    return models;
}
