//
// Created by monty on 23/11/15.
//

#ifndef NOUDARRENDERING_DUNGEONRENDERER_H
#define NOUDARRENDERING_DUNGEONRENDERER_H


namespace odb {

    enum EShaderType {
        kVertexShader,
        kFragmentShader,
    };

    const CLerp kNoFade = CLerp(1000, 1000, 0);;

    class DungeonGLES2Renderer {

    private:
        void fetchShaderLocations();

        void setPerspective();

        void prepareShaderProgram();

        void clearBuffers();

        void resetTransformMatrices();

        void printVerboseDriverInformation();

        void createVBOs();

        void deleteVBOs();

        void drawGeometry(const int vertexVbo, const int indexVbo,
                          int vertexCount,
                          const glm::mat4 &transform, float shade);

        int createProgram(const char *pVertexSource, const char *pFragmentSource);

        int loadShader(EShaderType shaderType, const char *pSource);

        void consumeRenderingBatches(long animationTime);

        void produceRenderingBatches(const NoudarDungeonSnapshot &snapshot);

        void initTileProperties();

    private:
        int frame = 0;
        int vertexAttributePosition;
        int samplerUniformPosition;
        int textureCoordinatesAttributePosition;
        int projectionViewMatrixAttributePosition;
        int gProgram;
        int uMod;
        int uScale;
        int fadeUniform;
        int mSkyboxVBOVertexId = -1;
        int mBillboardVBOVertexId = -1;
        glm::mat4 projectionMatrix;
        glm::mat4 mViewMatrix = glm::mat4(1.0f);
        std::unordered_map<ETextures, vector<odb::VBORenderingJob>> batches;
        std::unordered_map<EActorsSnapshotElement, ETextures> mElementMap;
        vector<vector<std::shared_ptr<NativeBitmap>>> mBitmaps;
        vector<vector<unsigned int>> mTextures;
        std::unordered_map<VBORegisterId, VBORegister> mVBORegisters;
        std::unordered_map<std::string, ETextures> mTextureRegistry;
        CTilePropertyMap mTileProperties;

        //interaction
        glm::vec3 mCurrentCharacterPosition;

        //VBOs
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

        RenderingJobSnapshotAdapter mSnapshotAdapter;
        Camera mCamera;
        CLerp mFadeLerp = kNoFade;
    public:
        //basic bookeeping
        DungeonGLES2Renderer();

        void reloadTextures();

        void unloadTextures();

        ~DungeonGLES2Renderer();

        bool init(float w, float h, const std::string &vertexShader,
                  const std::string &fragmentShader);

        void setTexture(vector<vector<std::shared_ptr<NativeBitmap>>> textures);

        void render(const NoudarDungeonSnapshot &snapshot);

        void shutdown();

        void invalidateCachedBatches();

        void setTileProperties(CTilePropertyMap propertyMap);

        void setPerspectiveMatrix(float *perspectiveMatrix);

        VBORegister
        submitVBO(float *data, int vertices, unsigned short *indexData, unsigned int indices);

        //camera controls

        void updateCamera(long ms);

        void rotateLeft();

        void rotateRight();

        bool isAnimating();

        void setEyeView(float *eyeView);

        void setAngleXZ(float xz);

        void setAngleYZ(float yz);

        //fade
        void startFadingIn();

        void startFadingOut();

        void updateFadeState(long ms);

        //interactions
        void resetCamera();
    };
}
#endif //NOUDARRENDERING_DUNGEONRENDERER_H
