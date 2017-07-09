//
// Created by monty on 06/10/16.
//

#ifndef NOUDAR_CORE_NOUDARGLES2BRIDGE_H
#define NOUDAR_CORE_NOUDARGLES2BRIDGE_H

namespace odb {
    class NoudarGLES2Bridge : public Knights::IRenderer {
        char mNextCmd = '.';
        bool mPerformVisibilityCheck = true;
    public:
        void drawMap( Knights::CMap &map, std::shared_ptr<Knights::CActor> current ) override;
        char getInput() override;
        void setNextCommand( char cmd );
        void reset();
        void setVisibilityChecks( bool visibilityCheck );
    };
}



#endif //NOUDAR_CORE_NOUDARGLES2BRIDGE_H
