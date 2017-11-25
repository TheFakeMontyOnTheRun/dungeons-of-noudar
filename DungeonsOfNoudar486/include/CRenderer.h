#ifndef IRENDERER_H
#define IRENDERER_H

#include <CTile3DProperties.h>

namespace odb {
    constexpr const static auto NATIVE_TEXTURE_SIZE = 16;
    constexpr const static auto XRES = 256;
    constexpr const static auto YRES = 128;

    constexpr const static auto HALF_XRES = XRES / 2;
    constexpr const static auto HALF_YRES = YRES / 2;


    using NativeTexture = array<uint8_t , NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE >;
    using TexturePair = std::pair<std::shared_ptr<NativeTexture >, std::shared_ptr<NativeTexture >>;

    class CRenderer  : public Knights::IRenderer {
        bool mCached = false;
        bool mHudDrawn = false;
        char mElementsMap[40][40];
        Knights::CommandType mBufferedCommand = '.';
        bool mNeedsToRedraw = true;
        array< uint8_t, 320 * 200 > mBuffer;
        array< uint32_t , 256 > mPalette;
        EActorsSnapshotElement mActors[40][40];
        Knights::EDirection mCameraDirection;
    public:
        void drawMap( Knights::CMap& map, std::shared_ptr<Knights::CActor> current ) override;
        Knights::CommandType getInput() override;
    private:
        void flip();
     public:
        CRenderer();

        uint8_t * getBufferData();

        void fillSidebar();

        void fillUnderbar();

        void drawCubeAt(const Vec3 &center, TexturePair texture );

        void drawFloorAt(const Vec3 &center, TexturePair texture );

        void drawCeilingAt(const Vec3 &center, TexturePair texture );

        void drawLeftNear(const Vec3 &center, const FixP &scale, TexturePair texture);

        void drawRightNear(const Vec3 &center, const FixP &scale, TexturePair texture);

        void drawColumnAt(const Vec3 &center, const FixP &scale, TexturePair texture, bool mask[3], bool enableAlpha = false );

        void drawSprite( int x, int y, std::shared_ptr<odb::NativeBitmap> tile );

        Vec2 project(const Vec3 &p);

        void loadTextures( vector<vector<std::shared_ptr<odb::NativeBitmap>>> textureList, CTilePropertyMap& tileProperties );

        void render(long ms);

        void handleSystemEvents();

        void sleep(long ms);

        void putRaw(int16_t x, int16_t y, uint32_t pixel);

        void fill( int x, int y, int dx, int dy, uint8_t pixel );

        void drawWall(FixP x0, FixP x1, FixP x0y0, FixP x0y1, FixP x1y0, FixP x1y1, TexturePair texture, FixP textureScaleY );

        void drawFloor(FixP y0, FixP y1, FixP x0y0, FixP x1y0, FixP x0y1, FixP x1y1, TexturePair texture);

        void drawFrontWall( FixP x0, FixP y0, FixP x1, FixP y1, TexturePair texture, FixP textureScaleY, bool enableAlpha = false );

        void clear();

        Vec3 mCamera{ FixP{1}, FixP{1}, FixP{1}};
        Vec3 mSpeed{0, 0, 0};

        void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

        void drawLine(const Vec2 &p0, const Vec2 &p1);

        void projectAllVertices();

        static unsigned char getPaletteEntry(int origin);

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
        static vector<TexturePair> mNativeTextures;

        const static uint8_t mTransparency;
    };

    vector<vector<std::shared_ptr<odb::NativeBitmap>>>
    loadTexturesForLevel(int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader);
    odb::CTilePropertyMap loadTileProperties( int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader );
}
#endif