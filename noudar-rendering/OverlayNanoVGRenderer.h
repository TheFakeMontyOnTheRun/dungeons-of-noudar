//
// Created by monty on 23/11/15.
//

#ifndef NOUDARRENDERING_OVERLAYNANOVGRENDERER_H
#define NOUDARRENDERING_OVERLAYNANOVGRENDERER_H


#include <IFileLoaderDelegate.h>

namespace odb {
    class OverlayNanoVGRenderer {

        float mWidth;
        float mHeight;
        std::vector<char> mFontData;
    public:
        explicit OverlayNanoVGRenderer();
        void setFrame(float width, float height);
        void loadFonts(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoaderDelegate );
        ~OverlayNanoVGRenderer();
        void render( const odb::NoudarDungeonSnapshot& snapshot );
    };
}
#endif //NOUDARRENDERING_OVERLAYNANOVGRENDERER_H
