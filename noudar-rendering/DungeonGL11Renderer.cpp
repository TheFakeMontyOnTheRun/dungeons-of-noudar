//
// Created by monty on 23/11/15.
//
#ifndef __ANDROID__

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__

#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#endif




#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <map>
#include <tuple>
#include <array>

#include "Vec2i.h"
#include "IMapElement.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"

#include "NativeBitmap.h"
#include "Texture.h"
#include "Material.h"
#include "Trig.h"
#include "TrigBatch.h"
#include "MeshObject.h"
#include "Logger.h"
#include "MaterialList.h"
#include "Scene.h"
#include "Common.h"
#include "VBORenderingJob.h"
#include "DungeonGLES2Renderer.h"


namespace odb {

    struct TGeometryBatch {
        float* vertices;
        int sizeVertices;
    };

    struct TIndicesBatch {
        unsigned short* indices;
        unsigned int sizeIndices;
    };

    std::vector<TGeometryBatch> GeometryBatches; //bitches!
    std::vector<TIndicesBatch> IndicesBatches; //bitches!

    const static bool kShouldDestroyThingsManually = false;

    const static int kGeometryLineStride = 5;

    const float DungeonGLES2Renderer::billboardVertices[]{
            -1.0f, 1.0f, 0.0f, 0.0f, .0f,
            1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
    };


    const float DungeonGLES2Renderer::cornerLeftFarVertices[]{
            -1.0f, 1.0f, -1.0f, 0.0f, .0f,
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 0.0f, 1.0f,
    };

    const float DungeonGLES2Renderer::cornerLeftNearVertices[]{
            -1.0f, 1.0f, 1.0f, 0.0f, .0f,
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
    };


    const float DungeonGLES2Renderer::floorVertices[]{
            -1.0f, 0.0f, -1.0f, 0.0f, .0f,
            1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    };

    const float DungeonGLES2Renderer::skyVertices[]{
            -kSkyTextureLength - 20.0f, 10.0f, -200.0f, 0.0f, .0f,
            -20.0f, 10.0f, -200.0f, 10.0f, 0.0f,
            -20.0f, 10.0f, 200.0f, 10.0f, 10.0f,
            -kSkyTextureLength - 20.0f, 10.0f, 200.0f, 0.0f, 10.0f,
    };

    const float DungeonGLES2Renderer::cubeVertices[]{
//    4________5
//    /|       /|
//   / |      / |
// 0/__|___1_/  |
//  | 7|____|___|6
//  |  /    |  /
//  | /     | /
// 3|/______|/2
//x, y, z, r, g, b, u, v
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,    //0
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     //1
            1.0f, -1.0f, 1.0f, 1.0f, 1.0f,   //2
            -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,   //3

            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,   //4
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f,    //5
            1.0f, -1.0f, -1.0f, 1.0f, 1.0f,   //6
            -1.0f, -1.0f, -1.0f, 0.0f, 1.0f,   //7

            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,    //8 (0)
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     //9 (1)
            1.0f, -1.0f, 1.0f, 1.0f, 1.0f,   //10 (2)
            -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,   //11 (3)

            -1.0f, 1.0f, -1.0f, 1.0f, 0.0f,   //12 (4)
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f,    //13 (5)
            1.0f, -1.0f, -1.0f, 0.0f, 1.0f,   //14 (6)
            -1.0f, -1.0f, -1.0f, 1.0f, 1.0f   //15 (7)
    };

    const unsigned short DungeonGLES2Renderer::billboardIndices[]{
            0, 1, 2,
            0, 2, 3
    };

    const unsigned short DungeonGLES2Renderer::cornerLeftFarIndices[]{
            0, 1, 2,
            0, 2, 3
    };

    const unsigned short DungeonGLES2Renderer::cornerLeftNearIndices[]{
            0, 1, 2,
            0, 2, 3
    };

    const unsigned short DungeonGLES2Renderer::floorIndices[]{
            0, 1, 2,
            0, 2, 3
    };

    const unsigned short DungeonGLES2Renderer::skyIndices[]{
            0, 1, 2,
            0, 2, 3
    };


    const unsigned short DungeonGLES2Renderer::cubeIndices[]{
            0, 1, 2,
            0, 2, 3,

            5, 4, 7,
            5, 7, 6,

            9, 13, 14,
            9, 14, 10,

            12, 8, 15,
            8, 11, 15
    };

    unsigned int uploadTextureData(std::shared_ptr<NativeBitmap> bitmap) {
        // Texture object handle
        unsigned int textureId = 0;

        //Generate texture storage
        glGenTextures(1, &textureId);

        //specify what we want for that texture
        glBindTexture(GL_TEXTURE_2D, textureId);

        //upload the data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->getWidth(), bitmap->getHeight(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, bitmap->getPixelData());

        // Set the filtering mode - surprisingly, this is needed.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        odb::Logger::log("textureId:%d\n", textureId);

        return textureId;
    }


    extern void printGLString(const char *name, GLenum s) {
        const char *v = (const char *) glGetString(s);
        odb::Logger::log("GL %s = %s\n", name, v);
    }

    extern void checkGlError(const char *op) {
        for (GLint error = glGetError(); error; error = glGetError()) {
            odb::Logger::log("after %s() glError (0x%x)\n", op, error);
        }
    }

    GLuint DungeonGLES2Renderer::loadShader(EShaderType shaderType, const char *pSource) {
        return 1;
    }

    GLuint DungeonGLES2Renderer::createProgram(const char *pVertexSource,
                                               const char *pFragmentSource) {
        return 1;
    }

    void DungeonGLES2Renderer::printVerboseDriverInformation() {
    }

    DungeonGLES2Renderer::DungeonGLES2Renderer() {
        projectionMatrix = glm::mat4(1.0f);
        vertexAttributePosition = 0;
        modelMatrixAttributePosition = 0;
        projectionMatrixAttributePosition = 0;
        gProgram = 0;
    }

    DungeonGLES2Renderer::~DungeonGLES2Renderer() {
        odb::Logger::log("Destroying the renderer");

        if (kShouldDestroyThingsManually) {
            deleteVBOs();
            for (auto &texture : mTextures) {
                glDeleteTextures(1, &(texture->mTextureId));
            }
        }
    }

    bool DungeonGLES2Renderer::init(float w, float h, const std::string &vertexShader,
                                    const std::string &fragmentShader) {
        createVBOs();

        glEnable(GL_TEXTURE_2D);
        glShadeModel(GL_SMOOTH);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

        glViewport(0, 0, w, h);
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity( );

        /* Set our perspective */
        gluPerspective( 45.0f, w/h, 0.1f, 100.0f );

        printVerboseDriverInformation();

        gProgram = createProgram(vertexShader.c_str(), fragmentShader.c_str());

        if (!gProgram) {
            odb::Logger::log("Could not create program.");
            return false;
        }

        fetchShaderLocations();

        mWidth = w;
        mHeight = h;

        checkGlError("glViewport");

        projectionMatrix = glm::perspective(45.0f, w / h, 0.1f, 100.0f);


        int index = 0;
        odb::Logger::log("bitmaps size as to upload: %d", mBitmaps.size() );
        for (auto &bitmap : mBitmaps) {
            odb::Logger::log("index: %d", index);
            mTextures.push_back(std::make_shared<Texture>(uploadTextureData(bitmap), bitmap));
        }



        startFadingIn();
        return true;
    }

    glm::mat4 DungeonGLES2Renderer::getCubeTransform(glm::vec3 translation) {
        glm::mat4 identity = glm::mat4(1.0f);
        glm::mat4 translated = glm::translate(identity, translation);

        return translated;
    }

    static glm::mat4 viewMatrix;

    void DungeonGLES2Renderer::resetTransformMatrices() {

        glm::vec3 pos = mCurrentCharacterPosition;
        glm::vec4 pos_front4 = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
        glm::vec3 pos_front;
        glm::mat4 eyeMatrixOriginal =
                mEyeView != nullptr ? glm::make_mat4(mEyeView) : glm::mat4(1.0f);
        glm::mat4 eyeMatrix = glm::mat4(1.0f);

        eyeMatrix[3][0] = eyeMatrixOriginal[3][0];
        eyeMatrix[3][1] = eyeMatrixOriginal[3][1];
        eyeMatrix[3][2] = eyeMatrixOriginal[3][2];

        float angleInRadiansYZ = mAngleYZ * (3.14159f / 180.0f);
        float angleInRadiansXZ = (mAngleXZ - mCameraRotation) * (3.14159f / 180.0f);

        glm::vec3 mCameraDirection{0, 0, 0};

        mCameraDirection = glm::rotate(
                glm::rotate(glm::mat4(1.0f), angleInRadiansXZ, glm::vec3(0.0f, 1.0f, 0.0f)),
                angleInRadiansYZ, glm::vec3(1.0f, 0.0f, 0.0f)) * pos_front4;
        pos_front = mCameraDirection;

        viewMatrix = glm::lookAt(
                pos,
                pos_front + pos,
                glm::vec3(0.0f, 1.0, 0.0f)) * eyeMatrix;

    }



    void DungeonGLES2Renderer::fetchShaderLocations() {

    }

    void DungeonGLES2Renderer::drawGeometry(const int vertexVbo, const int indexVbo,
                                            int vertexCount,
                                            const glm::mat4 &transform) {

        auto geometryBatch = GeometryBatches[ vertexVbo ];
        auto indicesBatch = IndicesBatches[ indexVbo ];

        glLoadMatrixf( &(viewMatrix*transform)[0][0] );

        glBegin(GL_TRIANGLES);

        for ( int i = 0; i < indicesBatch.sizeIndices; ++i ) {
            unsigned short index = indicesBatch.indices[ i ];

            float *line = geometryBatch.vertices + ( index * kGeometryLineStride  );
            glTexCoord2f( line[ 3 ], line[ 4 ]);
            glVertex3f(line[ 0 ], line[ 1 ], line[ 2 ]);
        }

        glEnd();
    }

    void DungeonGLES2Renderer::deleteVBOs() {
    }

    void DungeonGLES2Renderer::createVBOs() {

        mCubeVBO = submitVBO((float *) cubeVertices, 16, (unsigned short *) cubeIndices, 24);
        mBillboardVBO = submitVBO((float *) billboardVertices, 4,
                                  (unsigned short *) billboardIndices, 6);
        mCornerLeftFarVBO = submitVBO((float *) cornerLeftFarVertices, 4,
                                      (unsigned short *) cornerLeftFarIndices, 6);
        mCornerLeftNearVBO = submitVBO((float *) cornerLeftNearVertices, 4,
                                       (unsigned short *) cornerLeftNearIndices, 6);
        mFloorVBO = submitVBO((float *) floorVertices, 4, (unsigned short *) floorIndices, 6);
        mSkyVBO = submitVBO((float *) skyVertices, 4, (unsigned short *) skyIndices, 6);
    }

    void DungeonGLES2Renderer::clearBuffers() {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        checkGlError("glClearColor");
        glClearDepth(1.0f);
        checkGlError("glClearDepthf");
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        checkGlError("glClear");
    }

    void DungeonGLES2Renderer::setPerspective() {
    }

    void DungeonGLES2Renderer::prepareShaderProgram() {
        checkGlError("glUseProgram");
    }

    void DungeonGLES2Renderer::render(IntGameMap map, IntGameMap actors, IntGameMap splats,
                                      LightMap lightMap, IntField ids,
                                      AnimationList movingCharacters,
                                      long animationTime) {

        if (mBitmaps.empty()) {
            return;
        }

        clearBuffers();
        prepareShaderProgram();
        setPerspective();
        resetTransformMatrices();

        invalidateCachedBatches();
        produceRenderingBatches(map, actors, splats, lightMap, ids, movingCharacters,
                                animationTime);
        consumeRenderingBatches(animationTime);
    }

    void DungeonGLES2Renderer::updateFadeState(long ms) {
        if (mFadeState == kFadingIn) {
            mFadeColour.a -= (ms / 1000.0f);
            mFadeColour.r = mFadeColour.g = mFadeColour.b = 1.0f - mFadeColour.a;
        } else if (mFadeState == kFadingOut) {
            mFadeColour.a += (ms / 1000.0f);
            mFadeColour.r = mFadeColour.g = mFadeColour.b = 1.0f - mFadeColour.a;
        } else {
            mFadeColour.a = 0.0f;
        }

        if ((mFadeState == kFadingIn) && (mFadeColour.a >= 1.0)) {
            mFadeColour.a = 0.0f;
            mFadeState = kNormal;
        }

        if ((mFadeState == kFadingOut) && (mFadeColour.a <= 0.1f)) {
            mFadeState = kNormal;
        }
    }

    void DungeonGLES2Renderer::setTexture(std::vector<std::shared_ptr<NativeBitmap>> textures) {
        mBitmaps.clear();
        mBitmaps.insert(mBitmaps.end(), textures.begin(), textures.end());
        mElementMap['.'] = ETextures::Grass;
        mElementMap['_'] = ETextures::Floor;

        mElementMap['('] = ETextures::GrassStoneFar;
        mElementMap['{'] = ETextures::GrassStoneNear;

        mElementMap[')'] = ETextures::StoneGrassNear;
        mElementMap['}'] = ETextures::StoneGrassFar;

        mElementMap['='] = ETextures::Floor;
        mElementMap['1'] = ETextures::Bricks;
        mElementMap['#'] = ETextures::Bars;
        mElementMap['~'] = ETextures::Arch;
        mElementMap['Y'] = ETextures::BricksCandles;
        mElementMap['X'] = ETextures::BricksBlood;

        mElementMap['%'] = ETextures::Bricks;
        mElementMap['|'] = ETextures::Bricks;
        mElementMap['\\'] = ETextures::Bricks;
        mElementMap['/'] = ETextures::Bricks;
        mElementMap['>'] = ETextures::Bricks;
        mElementMap['<'] = ETextures::Bricks;
        mElementMap['Z'] = ETextures::Bricks;
        mElementMap['S'] = ETextures::Bricks;

        mElementMap['9'] = ETextures::Exit;
        mElementMap['*'] = ETextures::Begin;


        mElementMap['@'] = ETextures::Cuco0;
        mElementMap['?'] = ETextures::Crusader0;

        mElementMap[' '] = ETextures::Skybox;

        mElementMap['^'] = ETextures::Crusader0;

    }

    void DungeonGLES2Renderer::shutdown() {
        odb::Logger::log("Shutdown!\n");
    }

    void DungeonGLES2Renderer::setCameraPosition(float x, float y) {
        this->mCameraTarget = glm::vec2{x, y};
    }

    void DungeonGLES2Renderer::startFadingIn() {
        if (mFadeState == kFadingIn) {
            return;
        }

        mFadeState = kFadingIn;
        mFadeColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void DungeonGLES2Renderer::startFadingOut() {
        if (mFadeState == kFadingOut) {
            return;
        }

        mFadeState = kFadingOut;
        mFadeColour = glm::vec4(0.0f, 0.0f, 0.0f, 0.1f);
    }

    void DungeonGLES2Renderer::updateCamera(long ms) {
        cameraPosition.x += ms * (mCameraTarget.x - cameraPosition.x) / 1000.0f;
        cameraPosition.y += ms * (mCameraTarget.y - cameraPosition.y) / 1000.0f;

        if (mRotationTarget > mCameraRotation) {
            mCameraRotation += 5;
        } else if (mRotationTarget < mCameraRotation) {
            mCameraRotation -= 5;
        }
    }

    void DungeonGLES2Renderer::consumeRenderingBatches(long animationTime) {
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable( GL_TEXTURE_2D );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        for (auto &batch : batches) {

            glBindTexture(GL_TEXTURE_2D, mTextures[batch.first]->mTextureId);

            for (auto &element : batch.second) {
                auto transform = element.getTransform();
                auto shade = element.getShade();
                auto amount = element.getAmount();
                auto vboId = element.getVBOId();
                auto vboIndicesId = element.getVBOIndicesId();


                drawGeometry(vboId,
                             vboIndicesId,
                             amount,
                             transform
                );
            }
        }
    }

    void DungeonGLES2Renderer::produceRenderingBatches(IntGameMap map, IntGameMap actors,
                                                       IntGameMap splats,
                                                       LightMap lightMap, IntField ids,
                                                       AnimationList movingCharacters,
                                                       long animationTime) {

        ETextures chosenTexture;
        glm::vec3 pos;

        batches.clear();


        batches[ETextures::Skybox].emplace_back(std::get<0>(mSkyVBO),
                                                std::get<1>(mSkyVBO),
                                                std::get<2>(mSkyVBO),
                                                getSkyTransform(animationTime),
                                                1.0f);

        batches[ETextures::Skybox].emplace_back(std::get<0>(mSkyVBO),
                                                std::get<1>(mSkyVBO),
                                                std::get<2>(mSkyVBO),
                                                getSkyTransform(
                                                        animationTime + kSkyTextureLength * 1000),
                                                1.0f);


        for (int z = 0; z < Knights::kMapSize; ++z) {
            for (int x = 0; x < Knights::kMapSize; ++x) {

                int tile = map[Knights::kMapSize - 1 - z][x];
                int actor = actors[Knights::kMapSize - 1 - z][x];
                int splatFrame = splats[Knights::kMapSize - 1 - z][x];

                Shade placeShade =
                        (0.25f * std::min(255, lightMap[Knights::kMapSize - 1 - z][x]) / 255.0f) +
                        0.75f;
                Shade shade = placeShade;

                if (x == mCursorPosition.x &&
                    (Knights::kMapSize - 1 - z) == static_cast<int>(mCursorPosition.y)) {
                    shade = 1.5f;
                }

                if (actor == '@') {
                    //	chosenTexture = ETextures::Shadow;
                } else {
                }

                if (tile == '.' || tile == '_' || tile == '=' || tile == '-' || tile == ')' ||
                    tile == '(' || tile == '}' || tile == '{') {
                    chosenTexture = mElementMap[tile];
                } else if (tile == '#' || tile == '~' || tile == '|' || tile == '%') {
                    chosenTexture = mElementMap['_'];
                } else if (tile == '\\') {
                    chosenTexture = mElementMap['('];
                } else if (tile == '/') {
                    chosenTexture = mElementMap[')'];
                } else if (tile == '>') {
                    chosenTexture = mElementMap['{'];
                } else if (tile == '<') {
                    chosenTexture = mElementMap['}'];

                } else {
                    chosenTexture = mElementMap['.'];
                }

                if (tile == '=' || tile == '-') {
                    shade -= 0.25f;
                }


                pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), -5.0f,
                                -(Knights::kMapSize / 2.0f) + (-z * 2));
                batches[chosenTexture].emplace_back(std::get<0>(mFloorVBO),
                                                    std::get<1>(mFloorVBO),
                                                    std::get<2>(mFloorVBO),
                                                    getFloorTransform(pos), shade);

                shade = placeShade;

                //walls
                if (tile == '\\' || tile == '<' || tile == '|' || tile == 'S') {
                    pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), -4.0f,
                                    -(Knights::kMapSize / 2.0f) + (-z * 2));

                    batches[static_cast<ETextures >(mElementMap[tile])].emplace_back(
                            std::get<0>(mCornerLeftFarVBO),
                            std::get<1>(mCornerLeftFarVBO),
                            std::get<2>(mCornerLeftFarVBO),
                            getCornerLeftFarTransform(pos),
                            shade);

                    pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), -2.0f,
                                    -(Knights::kMapSize / 2.0f) + (-z * 2));
                    batches[static_cast<ETextures >(mElementMap[tile])].emplace_back(
                            std::get<0>(mCornerLeftFarVBO),
                            std::get<1>(mCornerLeftFarVBO),
                            std::get<2>(mCornerLeftFarVBO),
                            getCornerLeftFarTransform(pos),
                            shade);

                    pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), 0.0f,
                                    -(Knights::kMapSize / 2.0f) + (-z * 2));
                    batches[static_cast<ETextures >(mElementMap[tile])].emplace_back(
                            std::get<0>(mCornerLeftFarVBO),
                            std::get<1>(mCornerLeftFarVBO),
                            std::get<2>(mCornerLeftFarVBO),
                            getCornerLeftFarTransform(pos),
                            shade);


                } else if (tile == '/' || tile == '>' || tile == '%' || tile == 'Z') {
                    pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), -4.0f,
                                    -(Knights::kMapSize / 2.0f) + (-z * 2));
                    batches[static_cast<ETextures >(mElementMap[tile])].emplace_back(
                            std::get<0>(mCornerLeftNearVBO),
                            std::get<1>(mCornerLeftNearVBO),
                            std::get<2>(mCornerLeftNearVBO),
                            getCornerLeftNearTransform(pos),
                            shade);

                    pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), -2.0f,
                                    -(Knights::kMapSize / 2.0f) + (-z * 2));
                    batches[static_cast<ETextures >(mElementMap[tile])].emplace_back(
                            std::get<0>(mCornerLeftNearVBO),
                            std::get<1>(mCornerLeftNearVBO),
                            std::get<2>(mCornerLeftNearVBO),
                            getCornerLeftNearTransform(pos),
                            shade);

                    pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), 0.0f,
                                    -(Knights::kMapSize / 2.0f) + (-z * 2));
                    batches[static_cast<ETextures >(mElementMap[tile])].emplace_back(
                            std::get<0>(mCornerLeftNearVBO),
                            std::get<1>(mCornerLeftNearVBO),
                            std::get<2>(mCornerLeftNearVBO),
                            getCornerLeftNearTransform(pos),
                            shade);

                } else if (tile != '.' && tile != '_' && tile != '=' && tile != '-' &&
                           tile != ')' && tile != '(' && tile != '}' && tile != '{') {

                    pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), -4.0f,
                                    -(Knights::kMapSize / 2.0f) + (-z * 2));

                    batches[static_cast<ETextures >(mElementMap[tile])].emplace_back(
                            std::get<0>(mCubeVBO),
                            std::get<1>(mCubeVBO),
                            std::get<2>(mCubeVBO),
                            getCubeTransform(pos),
                            shade);

                    //top of walls cube
                    ETextures textureForCeling = ETextures::Ceiling;

                    if (tile == 'B') {
                        textureForCeling = ETextures::CeilingBegin;
                    } else if (tile == 'E') {
                        textureForCeling = ETextures::CeilingEnd;
                    } else if (tile == '~') {
                        textureForCeling = ETextures::CeilingDoor;
                    } else if (tile == '#') {
                        textureForCeling = ETextures::CeilingBars;
                    } else {
                        textureForCeling = ETextures::Ceiling;
                    }

                    if (tile != 'E') {
                        pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), -2.0f,
                                        -(Knights::kMapSize / 2.0f) + (-z * 2));
                        batches[(tile == 'b') ? ETextures::CeilingEnd
                                              : mElementMap['1']].emplace_back(
                                std::get<0>(mCubeVBO),
                                std::get<1>(mCubeVBO),
                                std::get<2>(mCubeVBO),
                                getCubeTransform(pos),
                                shade);

                        pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), 0.0f,
                                        -(Knights::kMapSize / 2.0f) + (-z * 2));
                        batches[(tile == 'b') ? ETextures::CeilingEnd
                                              : mElementMap['1']].emplace_back(
                                std::get<0>(mCubeVBO),
                                std::get<1>(mCubeVBO),
                                std::get<2>(mCubeVBO),
                                getCubeTransform(pos),
                                shade);

                    }


                } else {
                    if (tile == '=' || tile == '-') {

                        pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), 0.0f,
                                        -(Knights::kMapSize / 2.0f) + (-z * 2));

                        batches[Floor].emplace_back(
                                std::get<0>(mCubeVBO),
                                std::get<1>(mCubeVBO),
                                std::get<2>(mCubeVBO),
                                getCubeTransform(pos),
                                shade);
                        shade -= 0.375f;

                        pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), -1.0f,
                                        -(Knights::kMapSize / 2.0f) + (-z * 2));

                        batches[Floor].emplace_back(
                                std::get<0>(mFloorVBO),
                                std::get<1>(mFloorVBO),
                                std::get<2>(mFloorVBO),
                                getFloorTransform(pos),
                                shade);

                        shade = placeShade;

                    }
                }

                //characters
                if (actor != 0) {

                    int id = ids[Knights::kMapSize - 1 - z][x];
                    float fx, fz;

                    fx = x;
                    fz = z;

                    float step = 0.0f;
                    float curve = 0.0f;

                    if (id != 0 && movingCharacters.count(id) > 0) {
                        auto animation = movingCharacters[id];
                        step = (((float) ((animationTime - std::get<2>(animation)))) /
                                ((float) kAnimationLength));

                        if (!mLongPressing) {
                            if (step < 0.5f) {
                                curve = ((2.0f * step) * (2.0f * step)) / 2.0f;
                            } else {
                                curve = (sqrt((step * 2.0f) - 1.0f) / 2.0f) + 0.5f;
                            }
                        }

                        auto prevPosition = std::get<0>(animation);
                        auto destPosition = std::get<1>(animation);

                        fx = (curve * (destPosition.x - prevPosition.x)) + prevPosition.x;
                        fz = Knights::kMapSize - 1 -
                             ((curve * (destPosition.y - (prevPosition.y))) + prevPosition.y);
                    }

                    pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (fx * 2), -4.0f,
                                    -(Knights::kMapSize / 2.0f) + (-fz * 2));


                    if (actor == '@' || actor == '?') {


                        batches[static_cast<ETextures >(mElementMap[actor])].emplace_back(
                                std::get<0>(mBillboardVBO),
                                std::get<1>(mBillboardVBO),
                                std::get<2>(mBillboardVBO),
                                getBillboardTransform(pos), shade);

                    } else {
                        mCurrentCharacterPosition = pos;
                    }
                }

                if (splatFrame > -1) {
                    pos = glm::vec3(-(Knights::kMapSize / 2.0f) + (x * 2), -4.0f,
                                    -(Knights::kMapSize / 2.0f) + (-z * 2));
                    batches[static_cast<ETextures >(splatFrame +
                                                    ETextures::Splat0)].emplace_back(
                            std::get<0>(mBillboardVBO),
                            std::get<1>(mBillboardVBO),
                            std::get<2>(mBillboardVBO),
                            getBillboardTransform(pos), shade);
                }
            }
        }
    }

    void DungeonGLES2Renderer::invalidateCachedBatches() {
        batches.clear();
    }

    void DungeonGLES2Renderer::rotateLeft() {
        this->mRotationTarget -= 90;
    }

    void DungeonGLES2Renderer::rotateRight() {
        this->mRotationTarget += 90;
    }

    glm::mat4 DungeonGLES2Renderer::getBillboardTransform(glm::vec3 translation) {
        glm::mat4 identity = glm::mat4(1.0f);
        glm::mat4 translated = glm::translate(identity, translation);


        return glm::rotate(translated,
                           (360 - (mCameraRotation) + mAngleXZ) * (3.141592f / 180.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 DungeonGLES2Renderer::getFloorTransform(glm::vec3 translation) {
        glm::mat4 identity = glm::mat4(1.0f);
        glm::mat4 translated = glm::translate(identity, translation);

        return translated;
    }

    bool DungeonGLES2Renderer::isAnimating() {
        return mRotationTarget != mCameraRotation;
    }

    glm::mat4 DungeonGLES2Renderer::getSkyTransform(long animationTime) {
        glm::mat4 identity = glm::mat4(1.0f);

        long offset = animationTime;
        int integerPart = offset % ((kSkyTextureLength * 2) * 1000);
        float finalOffset = integerPart / 1000.0f;

        return glm::translate(identity, glm::vec3(finalOffset, 0.0f, 0.0f));
    }

    void DungeonGLES2Renderer::onLongPressingMove() {
        this->mLongPressing = true;
    }

    void DungeonGLES2Renderer::onReleasedLongPressingMove() {
        this->mLongPressing = false;
    }

    bool DungeonGLES2Renderer::isLongPressing() {
        return mLongPressing;
    }

    glm::mat4 DungeonGLES2Renderer::getCornerLeftFarTransform(glm::vec3 translation) {
        glm::mat4 identity = glm::mat4(1.0f);
        glm::mat4 translated = glm::translate(identity, translation);

        return translated;
    }

    glm::mat4 DungeonGLES2Renderer::getCornerLeftNearTransform(glm::vec3 translation) {
        glm::mat4 identity = glm::mat4(1.0f);
        glm::mat4 translated = glm::translate(identity, translation);

        return translated;
    }

    void DungeonGLES2Renderer::setEyeView(float *eyeView) {
        mEyeView = eyeView;
    }

    void DungeonGLES2Renderer::setPerspectiveMatrix(float *perspectiveMatrix) {
        projectionMatrix = glm::make_mat4(perspectiveMatrix);
    }

    void DungeonGLES2Renderer::setAngleXZ(float xz) {
        mAngleXZ = xz;
    }

    void DungeonGLES2Renderer::setAngleYZ(float yz) {
        mAngleYZ = yz;
    }

    glm::vec3 DungeonGLES2Renderer::transformToMapPosition(const glm::vec3 &pos) {
        return glm::vec3(-(Knights::kMapSize / 2.0f) + (pos.x * 2), -5.0f + pos.y,
                         -(Knights::kMapSize / 2.0f) + (-pos.z * 2));
    }

    void DungeonGLES2Renderer::setCursorPosition(int x, int y) {
        mCursorPosition = {x, y};
    }

    VBORegister DungeonGLES2Renderer::submitVBO(float *data, int vertices,
                                                unsigned short *indexData,
                                                unsigned int indices) {

        unsigned int dataIndex = GeometryBatches.size();
        unsigned int indicesIndex = IndicesBatches.size();


        GeometryBatches.push_back( { data, vertices } );
        IndicesBatches.push_back( { indexData, indices });

        return VBORegister(dataIndex, indicesIndex, indices);
    }

    void DungeonGLES2Renderer::setPlayerHealth(float health) {
        mPlayerHealth = health;
    }
}

#endif
