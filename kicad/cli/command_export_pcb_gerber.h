/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2022 Mark Roszko <mark.roszko@gmail.com>
 * Copyright (C) 1992-2022 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef COMMAND_EXPORT_PCB_GERBER_H
#define COMMAND_EXPORT_PCB_GERBER_H

#include "command_export_pcb_base.h"

class JOB_EXPORT_PCB_GERBER;

namespace CLI
{
#define ARG_NO_X2 "--no-x2"
#define ARG_NO_NETLIST "--no-netlist"
#define ARG_SUBTRACT_SOLDERMASK "--subtract-soldermask"
#define ARG_DISABLE_APERTURE_MACROS "--disable-aperture-macros"
#define ARG_PRECISION "--precision"

class EXPORT_PCB_GERBER_COMMAND : public EXPORT_PCB_BASE_COMMAND
{
public:
    EXPORT_PCB_GERBER_COMMAND( const std::string& aName );
    EXPORT_PCB_GERBER_COMMAND();

    int Perform( KIWAY& aKiway ) override;

protected:
    int populateJob( JOB_EXPORT_PCB_GERBER* aJob );
};
} // namespace CLI

#endif