#ifndef IRENDERER_H
#define IRENDERER_H

#include <CTile3DProperties.h>

namespace odb {

    class CRenderer  : public Knights::IRenderer {
        bool mCached = false;
        char mElementsMap[40][40];
        Knights::CommandType mBufferedCommand = '.';
        bool mNeedsToRedraw = true;
        std::shared_ptr<NativeBitmap> mFrameBuffer;
    public:
        void drawMap( Knights::CMap& map, std::shared_ptr<Knights::CActor> current ) override;
        Knights::CommandType getInput() override;
    private:
        void flip();
     public:
        CRenderer();

        int* getBufferData();

        void fillSidebar();

        void fillUnderbar();

        void drawCubeAt(const Vec3 &center, std::shared_ptr<odb::NativeBitmap> texture );

        void drawFloorAt(const Vec3 &center, std::shared_ptr<odb::NativeBitmap> texture );

        void drawCeilingAt(const Vec3 &center, std::shared_ptr<odb::NativeBitmap> texture );

        void drawLeftNear(const Vec3 &center, const Vec3 &scale, std::shared_ptr<odb::NativeBitmap> texture );

        void drawRightNear(const Vec3 &center, const Vec3 &scale, std::shared_ptr<odb::NativeBitmap> texture );

        void drawColumnAt(const Vec3 &center, const Vec3 &scale, std::shared_ptr<odb::NativeBitmap> texture );

        Vec2 project(const Vec3 &p);

        void loadTextures( vector<vector<std::shared_ptr<odb::NativeBitmap>>> textureList, CTilePropertyMap& tileProperties );

        void render(long ms);

        void handleSystemEvents();

        void sleep(long ms);

        void putRaw(int16_t x, int16_t y, uint32_t pixel);

        void drawWall(FixP x0, FixP x1, FixP x0y0, FixP x0y1, FixP x1y0, FixP x1y1, std::shared_ptr<odb::NativeBitmap> texture );

        void drawFloor(FixP y0, FixP y1, FixP x0y0, FixP x1y0, FixP x0y1, FixP x1y1, std::shared_ptr<odb::NativeBitmap> texture);

        void drawFrontWall( FixP x0, FixP y0, FixP x1, FixP y1, std::shared_ptr<odb::NativeBitmap> texture );

        void clear();

        Vec3 mCamera{ FixP{1}, FixP{1}, FixP{1}};
        Vec3 mSpeed{0, 0, 0};
        bool clearScr = true;

        void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

        void drawLine(const Vec2 &p0, const Vec2 &p1);

        void projectAllVertices();

        vector<vector<std::shared_ptr<odb::NativeBitmap>>> mTextures;

        vector<std::pair<Vec3, Vec2>> mVertices {
                {Vec3{0,0,0},Vec2{0,0}},
                {Vec3{0,0,0},Vec2{0,0}},
                {Vec3{0,0,0},Vec2{0,0}},
                {Vec3{0,0,0},Vec2{0,0}},

                {Vec3{0,0,0},Vec2{0,0}},
                {Vec3{0,0,0},Vec2{0,0}},
                {Vec3{0,0,0},Vec2{0,0}},
                {Vec3{0,0,0},Vec2{0,0}},
        };

        CTilePropertyMap mTileProperties;
        Knights::Vec2i mCameraPosition;
    };

    vector<vector<std::shared_ptr<odb::NativeBitmap>>>
    loadTexturesForLevel(int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader);
    odb::CTilePropertyMap loadTileProperties( int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader );
}
#endif