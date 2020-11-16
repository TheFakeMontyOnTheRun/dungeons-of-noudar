#ifndef IRENDERER_H
#define IRENDERER_H

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
public:
        bool mCached = false;
        bool mHudDrawn = false;
        bool mStaticPartsOfHudDrawn = false;
        bool mNeedsToRedraw = true;

        array<array<Knights::ElementView , Knights::kMapSize>, Knights::kMapSize> mElementsMap;
        std::shared_ptr<NativeBitmap > mFont;
        array< uint8_t, 320 * 200 > mBuffer;
        EActorsSnapshotElement mActors[40][40];
        EItemsSnapshotElement mItems[ 40 ][ 40 ];
        Knights::EDirection mCameraDirection;
        char mLogBuffer[8][64];
        uint8_t mLineColour[5];
        Knights::CommandType mLastCommand = Knights::kNullCommand;
        int16_t mHealth;
        EItemsSnapshotElement mCurrentItem = EItemsSnapshotElement::kSword;
        std::string mItemName;
        int mItemCapacity;
        int mUsageCost = 0;
        Vec3 mCamera{ FixP{1}, FixP{1}, FixP{1}};

#ifdef PROFILEBUILD
        uint16_t mFrame = 0;
        int32_t mUsefulFrames = 0;
        int32_t mAccMs = 0;
        int32_t mProcVisTime = 0;
#endif

#ifdef SDLSW
        bool mSlow = false;
      
#endif
	  
        int mSplats[ 40 ][ 40 ];

        CTilePropertyMap mTileProperties;
        Knights::Vec2i mCameraPosition;

        int mDamageHighlight = 0;
        int mHealHighlight = 0;
        int mHighlightTime = 0;
        int mSplatFrameTime = 0;
    public:

        void drawMap( Knights::CMap& map, std::shared_ptr<Knights::CActor> current ) override;
        Knights::CommandType getInput() override;
        void flip();
    private:

     public:
	array< uint32_t , 256 > mPalette;
        Knights::CommandType mBufferedCommand = Knights::kNullCommand;

        const static uint8_t mTransparency;
        vector<TexturePair> mNativeTextures;

        void addSplatAt( const Knights::Vec2i& position );

        void addDeathAt( const Knights::Vec2i& position );

        Knights::CommandType peekInput() override;
        void appendToLog(const char* message, uint8_t colour );
        CRenderer(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader);
        virtual ~CRenderer() override;
        uint8_t * getBufferData();

        void fillSidebar();

        void fillUnderbar();

        void startHealHighlight();

        void startDamageHighlight();

        void drawCubeAt(const Vec3 &center, TexturePair texture );

        void drawTextAt( uint16_t x, uint16_t y, const char* text, uint8_t colour = 15 );

        void drawFloorAt(const Vec3 &center, TexturePair texture );

        void drawCeilingAt(const Vec3 &center, TexturePair texture );

        void drawLeftNear(const Vec3 &center, const FixP &scale, std::shared_ptr<odb::NativeTexture> texture, bool mask[4]);

        void drawRightNear(const Vec3 &center, const FixP &scale, std::shared_ptr<odb::NativeTexture> texture, bool mask[4]);

        void drawColumnAt(const Vec3 &center, const FixP &scale, TexturePair texture, bool mask[4], bool enableAlpha = false );

        void drawBillboardAt(const Vec3 &center, std::shared_ptr<odb::NativeTexture> texture );

        void drawSprite( uint16_t x, uint16_t y, std::shared_ptr<odb::NativeTexture > tile );

        void drawBitmap( uint16_t x, uint16_t y, std::shared_ptr<odb::NativeBitmap> tile );

        void loadTextures( vector<std::shared_ptr<odb::NativeBitmap>> textureList, CTilePropertyMap& tileProperties );

        void render(long ms);

        void handleSystemEvents();

        void sleep(long ms);

        void putRaw(int16_t x, int16_t y, uint32_t pixel);

        void fill( uint16_t x, uint16_t y, uint16_t dx, uint16_t dy, uint8_t pixel );

        void drawWall(FixP x0, FixP x1, FixP x0y0, FixP x0y1, FixP x1y0, FixP x1y1, std::shared_ptr<odb::NativeTexture> texture, FixP textureScaleY );

        void drawFloor(FixP y0, FixP y1, FixP x0y0, FixP x1y0, FixP x0y1, FixP x1y1, std::shared_ptr<NativeTexture > texture);

        void drawFrontWall( FixP x0, FixP y0, FixP x1, FixP y1, std::shared_ptr<odb::NativeTexture> texture, FixP textureScaleY, bool enableAlpha = false );

        void drawMask( FixP x0, FixP y0, FixP x1, FixP y1 );

        void clear();

        void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

        void drawLine(const Vec2 &p0, const Vec2 &p1);

        void projectAllVertices(uint8_t count);

        static uint8_t getPaletteEntry(uint32_t origin);

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
    };

    bool peekKeyboard(std::shared_ptr<CRenderer> renderer);

    int readKeyboard(std::shared_ptr<CRenderer> renderer);

    std::shared_ptr<odb::NativeTexture> makeTexture(const std::string& path, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader );

    TexturePair makeTexturePair( std::shared_ptr<odb::NativeBitmap> bitmap );

    vector<std::shared_ptr<odb::NativeBitmap>>
    loadTexturesForLevel(uint8_t levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader);
    odb::CTilePropertyMap loadTileProperties( uint8_t levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader );

    extern std::shared_ptr<CRenderer> renderer;
}
#endif
