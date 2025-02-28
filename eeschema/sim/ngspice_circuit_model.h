/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016-2022 CERN
 * Copyright (C) 2017-2022 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#ifndef NGSPICE_CIRCUIT_MODEL_H
#define NGSPICE_CIRCUIT_MODEL_H

#include <netlist_exporters/netlist_exporter_spice.h>
#include <vector>

#include "sim_types.h"
#include "spice_simulator.h"
#include "spice_value.h"


struct SPICE_DC_PARAMS
{
    wxString m_source;
    SPICE_VALUE m_vstart;
    SPICE_VALUE m_vend;
    SPICE_VALUE m_vincrement;
};

/// Special netlist exporter flavor that allows one to override simulation commands
class NGSPICE_CIRCUIT_MODEL : public NETLIST_EXPORTER_SPICE, public SIMULATION_MODEL
{
public:
    NGSPICE_CIRCUIT_MODEL( SCHEMATIC_IFACE* aSchematic ) :
            NETLIST_EXPORTER_SPICE( aSchematic ),
            m_options( 0 )
    {
    }
    virtual ~NGSPICE_CIRCUIT_MODEL() {}

    /**
     * Return name of Spice dataset for a specific plot.
     *
     * @param aVector is name of the vector produced by ngspice
     * @param [out] aSignal is output in form: V(R1), Ib(Q2), I(L8)
     * @return [SPT_VOLTAGE, SPT_CURRENT]. Otherwise SPT_UNKNOWN if vector is
     *         of different, unsupported type.
     */
    SIM_PLOT_TYPE VectorToSignal( const std::string& aVector, wxString& aSignal ) const;

    void SetOptions( int aOptions )
    {
        m_options = aOptions;
    }

    bool GetNetlist( OUTPUTFORMATTER* aFormatter, REPORTER& aReporter )
    {
        return NGSPICE_CIRCUIT_MODEL::DoWriteNetlist( *aFormatter, m_options, aReporter );
    }

    /**
     * Override the simulation command directive.
     */
    void SetSimCommandOverride( const wxString& aCmd )
    {
        if( aCmd != m_simCommand )
        {
            m_lastSheetSimCommand = GetSheetSimCommand();
            m_simCommand = aCmd;
        }
    }

    /**
     * Return the command directive that is in use (either from the sheet or from m_simCommand)
     * @return
     */
    wxString GetSimCommand()
    {
        return m_simCommand.IsEmpty() ? GetSheetSimCommand() : m_simCommand;
    }

    /**
     * Return the simulation command directive if stored separately (not as a sheet directive).
     */
    wxString GetSimCommandOverride() const { return m_simCommand; }

    /**
     * Return simulation type basing on the simulation command directives.
     *
     * Simulation directives set using SetSimCommandOverride() have priority over the ones placed in
     * schematic sheets.
     */
    SIM_TYPE GetSimType();

    /**
     * Return simulation command directives placed in schematic sheets (if any).
     */
    wxString GetSheetSimCommand();

    /**
     * Return the sim command present as a sheet directive when the sim command override was last
     * updated.
     * @return
     */
    wxString GetLastSheetSimCommand() const { return m_lastSheetSimCommand; }

    /**
     * Parse a two-source .dc command directive into its symbols.
     *
     * @param aCmd is the input command string
     * @return true if the command was parsed successfully
     */
    bool ParseDCCommand( const wxString& aCmd, SPICE_DC_PARAMS* aSource1,
                         SPICE_DC_PARAMS* aSource2 );

    /**
     * Determine if a directive is a simulation command.
     */
    static bool IsSimCommand( const wxString& aCmd )
    {
        return CommandToSimType( aCmd ) != ST_UNKNOWN;
    }

    /**
     * Return simulation type basing on a simulation command directive.
     */
    static SIM_TYPE CommandToSimType( const wxString& aCmd );

protected:
    void WriteDirectives( OUTPUTFORMATTER& aFormatter, unsigned aNetlistOptions ) const override;

private:
    ///< Custom simulation command (has priority over the schematic sheet simulation commands)
    wxString m_simCommand;

    ///< Value of schematic sheet simulation command when override was last updated
    wxString m_lastSheetSimCommand;

    int      m_options;
};

#endif /* NGSPICE_CIRCUIT_MODEL_H */
