//
// Created by monty on 23/11/15.
//

#ifndef NOUDARRENDERING_OVERLAYNANOVGRENDERER_H
#define NOUDARRENDERING_OVERLAYNANOVGRENDERER_H

namespace odb {
    class OverlayNanoVGRenderer {

        float mWidth;
        float mHeight;
        std::vector<char> mFontData;
        std::vector<std::string> mQueuedAnimations;
        std::map<std::string, std::shared_ptr<odb::NativeBitmap>> mBitmaps;
        std::map<std::string, int> mFrames;
        long mLastTimestamp;

    public:
        explicit OverlayNanoVGRenderer( std::vector<std::shared_ptr<odb::NativeBitmap>> bitmaps);
        void setFrame(float width, float height);
        void loadFonts(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoaderDelegate );
        ~OverlayNanoVGRenderer();
        void render( const odb::NoudarDungeonSnapshot& snapshot );
        void playAnimation( long currentTimestamp, std::string animationName );
        void enqueueAnimation( long currentTimestamp, std::string animationName );
    };
}
#endif //NOUDARRENDERING_OVERLAYNANOVGRENDERER_H
