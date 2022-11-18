#pragma once

#include <wx/string.h>
#include <pcb_edit_frame.h>
namespace HEROEDA
{
class heroEDAcmdHandle
{
public:
    heroEDAcmdHandle( PCB_EDIT_FRAME* frame );
    ~heroEDAcmdHandle();
    void handle( std::string cmdOp,std::string cmd );
    void updateNetlist( std::string cmd );
    void sendProjectPath( std::string cmd );

private: 
    PCB_EDIT_FRAME* m_pcbFrame = nullptr;
};

}