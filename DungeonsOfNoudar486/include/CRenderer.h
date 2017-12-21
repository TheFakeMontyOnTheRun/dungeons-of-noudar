#ifndef IRENDERER_H
#define IRENDERER_H

#include <CTile3DProperties.h>

namespace odb {
    constexpr const static auto NATIVE_TEXTURE_SIZE = 32;
    constexpr const static auto XRES = 256;
    constexpr const static auto YRES = 128;

    constexpr const static auto HALF_XRES = XRES / 2;
    constexpr const static auto HALF_YRES = YRES / 2;

    enum class EActorsSnapshotElement : uint8_t {
        kNothing,
        kHeroStanding0,
        kHeroStanding1,
        kHeroAttacking0,
        kHeroAttacking1,
        kWeakenedDemonAttacking0,
        kWeakenedDemonAttacking1,
        kWeakenedDemonStanding0,
        kWeakenedDemonStanding1,
        kStrongDemonAttacking0,
        kStrongDemonAttacking1,
        kStrongDemonStanding0,
        kStrongDemonStanding1,
        kCocoonStanding0,
        kCocoonStanding1,
        kEvilSpiritAttacking0,
        kEvilSpiritAttacking1,
        kEvilSpiritStanding0,
        kEvilSpiritStanding1,
        kWarthogAttacking0,
        kWarthogAttacking1,
        kWarthogStanding0,
        kWarthogStanding1,
        kMonkAttacking0,
        kMonkAttacking1,
        kMonkStanding0,
        kMonkStanding1,
        kFallenAttacking0,
        kFallenAttacking1,
        kFallenStanding0,
        kFallenStanding1,
        kRope,
        kMagicSeal
    };

    enum class EItemsSnapshotElement : uint8_t {
        kNothing,
        kSword,
        kCrossbow,
        kCross,
        kShield
    };

    using NativeTexture = array<uint8_t , NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE >;
    using TexturePair = std::pair<std::shared_ptr<NativeTexture >, std::shared_ptr<NativeTexture >>;

    class CRenderer  : public Knights::IRenderer {
        bool mCached = false;
        bool mHudDrawn = false;
        array<array<Knights::ElementView , Knights::kMapSize>, Knights::kMapSize> mElementsMap;
        Knights::CommandType mBufferedCommand = '.';
        bool mNeedsToRedraw = true;
        array< uint8_t, 320 * 200 > mBuffer;
        array< uint32_t , 256 > mPalette;
        EActorsSnapshotElement mActors[40][40];
        EItemsSnapshotElement mItems[ 40 ][ 40 ];
        Knights::EDirection mCameraDirection;
        int16_t mHealth;
        uint16_t mFrame = 0;
        int32_t mUsefulFrames = 0;
        int32_t mAccMs = 0;
        int32_t mProcVisTime = 0;
        std::string mItemName;

#ifdef SDLSW
        bool mSlow = false;
#endif
    public:
        void drawMap( Knights::CMap& map, std::shared_ptr<Knights::CActor> current ) override;
        Knights::CommandType getInput() override;
        void flip();
    private:

     public:
        CRenderer();
        ~CRenderer();
        uint8_t * getBufferData();

        void fillSidebar();

        void fillUnderbar();

        void drawCubeAt(const Vec3 &center, TexturePair texture );

        void drawTextAt( int x, int y, const char* text );

        void drawFloorAt(const Vec3 &center, TexturePair texture );

        void drawCeilingAt(const Vec3 &center, TexturePair texture );

        void drawLeftNear(const Vec3 &center, const FixP &scale, std::shared_ptr<odb::NativeTexture> texture);

        void drawRightNear(const Vec3 &center, const FixP &scale, std::shared_ptr<odb::NativeTexture> texture);

        void drawColumnAt(const Vec3 &center, const FixP &scale, TexturePair texture, bool mask[3], bool enableAlpha = false );

        void drawBillboardAt(const Vec3 &center, std::shared_ptr<odb::NativeTexture> texture );

        void drawSprite( int x, int y, std::shared_ptr<odb::NativeTexture > tile );

        void drawBitmap( int x, int y, std::shared_ptr<odb::NativeBitmap> tile );

        void loadTextures( vector<vector<std::shared_ptr<odb::NativeBitmap>>> textureList, CTilePropertyMap& tileProperties );

        void render(long ms);

        void handleSystemEvents();

        void sleep(long ms);

        void putRaw(int16_t x, int16_t y, uint32_t pixel);

        void fill( int x, int y, int dx, int dy, uint8_t pixel );

        void drawWall(FixP x0, FixP x1, FixP x0y0, FixP x0y1, FixP x1y0, FixP x1y1, std::shared_ptr<odb::NativeTexture> texture, FixP textureScaleY );

        void drawFloor(FixP y0, FixP y1, FixP x0y0, FixP x1y0, FixP x0y1, FixP x1y1, std::shared_ptr<NativeTexture > texture);

        void drawFrontWall( FixP x0, FixP y0, FixP x1, FixP y1, std::shared_ptr<odb::NativeTexture> texture, FixP textureScaleY, bool enableAlpha = false );

        void clear();

        Vec3 mCamera{ FixP{1}, FixP{1}, FixP{1}};

        void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

        void drawLine(const Vec2 &p0, const Vec2 &p1);

        void projectAllVertices(int count);

        static uint8_t getPaletteEntry(uint32_t origin);

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

    std::shared_ptr<odb::NativeTexture> makeTexture(const std::string& path, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader );

    TexturePair makeTexturePair( std::shared_ptr<odb::NativeBitmap> bitmap );

    vector<vector<std::shared_ptr<odb::NativeBitmap>>>
    loadTexturesForLevel(int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader);
    odb::CTilePropertyMap loadTileProperties( int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader );
}
#endif