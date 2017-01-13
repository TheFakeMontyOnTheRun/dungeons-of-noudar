//
// Created by monty on 23/11/15.
//

#ifndef NOUDARRENDERING_DUNGEONRENDERER_H
#define NOUDARRENDERING_DUNGEONRENDERER_H


namespace odb {

    enum EFadeState {
        kNormal,
        kFadingIn,
        kFadingOut
    };

    enum EShaderType {
        kVertexShader,
        kFragmentShader,
    };

    enum ETextures {
        Grass,
        Floor,
        Bricks,
        Arch,
        Bars,
        Begin,
        Exit,
        BricksBlood,
        BricksCandles,

        Foe0a,
        Foe0b,

        Foe1a,
        Foe1b,

        Foe2a,
        Foe2b,

        Crusader0,
        Crusader1,
        Crusader2,
        Shadow,
        Ceiling,
        CeilingDoor,
        CeilingBegin,
        CeilingEnd,
        Splat0,
        Splat1,
        Splat2,
        CeilingBars,
        Skybox,
        StoneGrassFar,
        GrassStoneFar,
        StoneGrassNear,
        GrassStoneNear,
        Cross
    };

    using Shade = float;
    using AnimationList = std::map<int, std::tuple<glm::vec2, glm::vec2, long> >;
    using TextureId = int;
    using VBORegister = std::tuple<unsigned int, unsigned int, unsigned int>;

    static const long kAnimationLength = 500;

    class CTile3DProperties {
    public:
        ETextures mCeilingTexture;
        ETextures mFloorTexture;
        ETextures mMainWallTexture;
        VBORegister mVBOToRender;
        ETextures mCeilingRepeatedWallTexture;
        ETextures mFloorRepeatedWallTexture;
        int mCeilingRepetitions;
        int mFloorRepetitions;
        float mCeilingHeight;
        float mFloorHeight;
    };

    class DungeonGLES2Renderer {

    private:
        std::map< int, CTile3DProperties > mTileProperties;

        void fetchShaderLocations();

        void setPerspective();

        void prepareShaderProgram();

        void clearBuffers();

        void resetTransformMatrices();

        void printVerboseDriverInformation();

        void createVBOs();

        void deleteVBOs();

        void drawGeometry(const unsigned int textureId, const int vertexVbo, const int indexVbo, int vertexCount,
                          const glm::mat4 &transform, float shade);

        unsigned int createProgram(const char *pVertexSource, const char *pFragmentSource);

        unsigned int loadShader(EShaderType shaderType, const char *pSource);

        glm::mat4 getSkyTransform(long offset);

        glm::mat4 getFloorTransform(glm::vec3 translation);

        glm::mat4 getBillboardTransform(glm::vec3 translation);

        glm::mat4 getCornerLeftFarTransform(glm::vec3 translation);

        glm::mat4 getCornerLeftNearTransform(glm::vec3 translation);

        glm::mat4 getCubeTransform(glm::vec3 translation);

        void consumeRenderingBatches(long animationTime);

        void produceRenderingBatches(IntMap map, CharMap actors, IntMap splats,
                                     IntMap lightmap, IntMap ids,
                                     AnimationList movingCharacters, long animationTime);

        glm::vec3 transformToMapPosition(const glm::vec3 &pos);

        void initTileProperties();

    private:

        const static int kSkyTextureLength = 400;

        int vertexAttributePosition;
        int modelMatrixAttributePosition;
        int samplerUniformPosition;
        int textureCoordinatesAttributePosition;
        int projectionMatrixAttributePosition;
        unsigned int gProgram;
        unsigned int uView;
        unsigned int uMod;
        int mTurn = 0;
        glm::mat4 projectionMatrix;
        glm::mat4 mViewMatrix = glm::mat4( 1.0f );
        std::map<ETextures, std::vector<odb::VBORenderingJob>> batches;
        std::map<EActorsSnapshotElement, ETextures> mElementMap;
        std::vector<std::shared_ptr<NativeBitmap>> mBitmaps;
        std::vector<std::shared_ptr<Texture>> mTextures;

        //fade state
        glm::vec4 mFadeColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        int fadeUniform;
        EFadeState mFadeState = EFadeState::kNormal;

        //interaction
        bool mLongPressing = false;

        glm::vec2 mCursorPosition{0, 0};

        glm::vec3 mCurrentCharacterPosition;

        //camera
        glm::vec2 mCameraTarget;
        glm::vec2 cameraPosition;
        float *mEyeView = nullptr;

        float mAngleXZ = 0;
        float mAngleYZ = 0;

        int mCameraRotation = 0;
        int mRotationTarget = 0;

        //VBOs

        VBORegister mCubeVBO;
        VBORegister mBillboardVBO;
        VBORegister mCornerLeftFarVBO;
        VBORegister mCornerLeftNearVBO;
        VBORegister mFloorVBO;
        VBORegister mSkyVBO;

        const static float cubeVertices[16 * 5];
        const static unsigned short cubeIndices[6 * 4];

        static const float billboardVertices[20];
        static const unsigned short billboardIndices[6];

        static const float cornerLeftFarVertices[20];
        static const unsigned short cornerLeftFarIndices[6];

        static const float cornerLeftNearVertices[20];
        static const unsigned short cornerLeftNearIndices[6];

        static const float floorVertices[20];
        static const unsigned short floorIndices[6];

        static const float skyVertices[20];
        static const unsigned short skyIndices[6];


        float mPlayerHealth = 0.0f;

    public:
        //basic bookeeping
        DungeonGLES2Renderer();

        ~DungeonGLES2Renderer();

        void setTurn( int turn );

        bool init(float w, float h, const std::string &vertexShader,
                  const std::string &fragmentShader);

        void setTexture(std::vector<std::shared_ptr<NativeBitmap>> textures);

        void render(IntMap map, CharMap actors, IntMap splats, IntMap lightmap,
                    IntMap ids, AnimationList movingCharacters, long animationTime);

        void shutdown();

        void invalidateCachedBatches();

        void setPerspectiveMatrix(float *perspectiveMatrix);

        void setClearColour(float r, float g, float b);

        VBORegister submitVBO( float* data, int vertices, unsigned short* indexData, unsigned int indices );

        //camera controls

        void updateCamera(long ms);

        void rotateLeft();

        void rotateRight();

        bool isAnimating();

        void setEyeView(float *eyeView);

        void setCameraPosition(float x, float y);

        void setAngleXZ(float xz);

        void setAngleYZ(float yz);

        //fade
        void startFadingIn();

        void startFadingOut();

        void updateFadeState(long ms);

        //interactions
        void onLongPressingMove();

        bool isLongPressing();

        void setCursorPosition(int x, int y);

        void onReleasedLongPressingMove();

        void setPlayerHealth( float health );

        void resetCamera();

        float mWidth{ 0.0f };
        float mHeight{ 0.0f };
    };
}
#endif //NOUDARRENDERING_DUNGEONRENDERER_H
