#pragma once

#include <wx/string.h>
#include <pcb_edit_frame.h>
namespace HEROEDA
{
class heroEDAcmdHandle
{
public:
    static heroEDAcmdHandle* instance();
    ~heroEDAcmdHandle();
    void handle( std::string cmdOp,std::string cmd );
    void updateNetlist( std::string cmd );
    void sendProjectPath( std::string cmd );

    bool getSingleApp() { return m_bSingleApp; }
    void setSingleApp( bool _b ) { m_bSingleApp = _b; }
    void setPcbFrame( PCB_EDIT_FRAME* _fram ) { m_pcbFrame = _fram; }

private:
    heroEDAcmdHandle();
private: 
    static heroEDAcmdHandle* m_instance;
    PCB_EDIT_FRAME* m_pcbFrame = nullptr;
    bool m_bSingleApp = true;
};

}