#include "heroEDAcmdHandle.h"

#include <tools/pcb_actions.h>
#include <dialog_netlist.h>
#include <../include/project/project_file.h>
#include <eda_dde.h>
#include <../include/tool/tool_manager.h>

using namespace HEROEDA;
using namespace std::placeholders;
heroEDAcmdHandle::heroEDAcmdHandle( PCB_EDIT_FRAME* frame ) : m_pcbFrame(frame)
{
}

heroEDAcmdHandle::~heroEDAcmdHandle()
{}

void heroEDAcmdHandle::handle( std::string cmdOp, std::string cmd )
{
    if( cmdOp.compare( "$UpdateNetlist:" ) == 0 )
        updateNetlist( cmd );
    else if( cmdOp.compare( "$GetProjectPath:" ) == 0 )
        sendProjectPath( cmd );
}

void heroEDAcmdHandle::updateNetlist( std::string cmd )
{
    if( !m_pcbFrame )
        return;
    wxString              path = cmd;
    DIALOG_NETLIST_IMPORT dlg( m_pcbFrame,path );
    dlg.ShowModal();
    m_pcbFrame->SetLastPath( LAST_PATH_NETLIST, path );
}

void heroEDAcmdHandle::sendProjectPath( std::string cmd )
{
    if( !m_pcbFrame )
        return;
    auto path=m_pcbFrame->GetLastPath( LAST_PATH_NETLIST );
    std::string packet =
            wxString::Format( _( "{\"command\":\"Cmd_GetProjectPath\",\"params\":\"%s\"}" ), path );
    SendCommand( MSG_TO_SCH, packet );
}
