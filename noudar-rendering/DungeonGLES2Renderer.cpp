//
// Created by monty on 23/11/15.
//

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#if TARGET_IOS
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#else
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>
#endif
#else

#include <GLES2/gl2.h>

#endif

#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <map>
#include <array>

#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "NativeBitmap.h"
#include "Texture.h"
#include "Logger.h"
#include "VBORenderingJob.h"
#include "IRenderer.h"
#include "NoudarDungeonSnapshot.h"
#include "ETextures.h"
#include "VBORegister.h"
#include "CTile3DProperties.h"
#include "Material.h"
#include "Trig.h"
#include "TrigBatch.h"
#include "MeshObject.h"
#include "MaterialList.h"
#include "Scene.h"
#include "DungeonGLES2Renderer.h"


namespace odb {
    const static bool kShouldDestroyThingsManually = false;

    //OpenGL specific stuff

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

    VBORegister DungeonGLES2Renderer::submitVBO(float *data, int vertices,
                                                unsigned short *indexData,
                                                unsigned int indices) {

        unsigned int dataIndex;
        unsigned int indicesIndex;

        glGenBuffers(1, &dataIndex);
        glBindBuffer(GL_ARRAY_BUFFER, dataIndex);
        glBufferData(GL_ARRAY_BUFFER, vertices * sizeof(float) * 5, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &indicesIndex);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesIndex);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices * sizeof(GLushort), indexData,
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        return VBORegister(dataIndex, indicesIndex, indices);
    }

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

        GLenum shaderSourceType =
                shaderType == EShaderType::kFragmentShader ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER;

        auto shader = glCreateShader(shaderSourceType);
        if (shader) {
            glShaderSource(shader, 1, &pSource, NULL);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled) {
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen) {
                    char *buf = (char *) malloc(infoLen);
                    if (buf) {
                        glGetShaderInfoLog(shader, infoLen, NULL, buf);
                        odb::Logger::log("Could not compile shader %d:\n%s\n", shaderType, buf);
                        free(buf);
                    }
                    glDeleteShader(shader);
                    shader = 0;
                }
            }
        }
        return shader;
    }

    GLuint DungeonGLES2Renderer::createProgram(const char *pVertexSource,
                                               const char *pFragmentSource) {
        auto vertexShader = loadShader(EShaderType::kVertexShader, pVertexSource);
        if (!vertexShader) {
            return 0;
        }

        auto pixelShader = loadShader(EShaderType::kFragmentShader, pFragmentSource);
        if (!pixelShader) {
            return 0;
        }

        auto program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShader);
            checkGlError("glAttachShader");
            glAttachShader(program, pixelShader);
            checkGlError("glAttachShader");
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char *buf = (char *) malloc(bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, NULL, buf);
                        odb::Logger::log("Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }

    void DungeonGLES2Renderer::printVerboseDriverInformation() {
    }

    bool DungeonGLES2Renderer::init(float w, float h, const std::string &vertexShader,
                                    const std::string &fragmentShader) {

        printVerboseDriverInformation();

        gProgram = createProgram(vertexShader.c_str(), fragmentShader.c_str());

        if (!gProgram) {
            odb::Logger::log("Could not create program.");
            return false;
        }
        fetchShaderLocations();

        mWidth = w;
        mHeight = h;
        projectionMatrix = glm::perspective(45.0f, w / h, 0.1f, 100.0f);

        glViewport(0, 0, w, h);
        checkGlError("glViewport");

        for (auto &bitmap : mBitmaps) {
            mTextures.push_back(std::make_shared<Texture>(uploadTextureData(bitmap), bitmap));
        }

        mTextureRegistry[ "sky" ] = ETextures::Skybox;
        mTextureRegistry[ "grass" ] = ETextures::Grass;
        mTextureRegistry[ "floor" ] = ETextures::Floor;
        mTextureRegistry[ "bricks" ] = ETextures::Bricks;
        mTextureRegistry[ "arch" ] = ETextures::Arch;
        mTextureRegistry[ "bars" ] = ETextures::Bars;
        mTextureRegistry[ "begin" ] = ETextures::Begin;
        mTextureRegistry[ "exit" ] = ETextures::Exit;
        mTextureRegistry[ "bricksblood" ] = ETextures::BricksBlood;
        mTextureRegistry[ "brickscandles" ] = ETextures::BricksCandles;
        mTextureRegistry[ "stonegrassfar" ] = ETextures::StoneGrassFar;
        mTextureRegistry[ "grassstonefar" ] = ETextures::GrassStoneFar;
        mTextureRegistry[ "stonegrassnear" ] = ETextures::StoneGrassNear;
        mTextureRegistry[ "grassstonenear" ] = ETextures::GrassStoneNear;
        mTextureRegistry[ "ceiling" ] = ETextures::Ceiling;
        mTextureRegistry[ "ceilingdoor" ] = ETextures::CeilingDoor;
        mTextureRegistry[ "ceilingbegin" ] = ETextures::CeilingBegin;
        mTextureRegistry[ "ceilingend" ] = ETextures::CeilingEnd;
        mTextureRegistry[ "ceilingbars" ] = ETextures::CeilingBars;

        createVBOs();
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glFrontFace(GL_CW);
        glDepthMask(true);
        startFadingIn();
        return true;
    }

    DungeonGLES2Renderer::~DungeonGLES2Renderer() {
        odb::Logger::log("Destroying the renderer");

        if (kShouldDestroyThingsManually) {
            for (auto &texture : mTextures) {
                glDeleteTextures(1, &(texture->mTextureId));
            }
            deleteVBOs();
        }
    }

    void DungeonGLES2Renderer::fetchShaderLocations() {

        vertexAttributePosition = glGetAttribLocation(gProgram, "aPosition");
        modelMatrixAttributePosition = glGetUniformLocation(gProgram, "uModel");
        projectionMatrixAttributePosition = glGetUniformLocation(gProgram, "uProjection");
        samplerUniformPosition = glGetUniformLocation(gProgram, "sTexture");
        textureCoordinatesAttributePosition = glGetAttribLocation(gProgram, "aTexCoord");
        uView = glGetUniformLocation(gProgram, "uView");
        uMod = glGetUniformLocation(gProgram, "uMod");
        fadeUniform = glGetUniformLocation(gProgram, "uFade");
    }

    void DungeonGLES2Renderer::drawGeometry(const unsigned int textureId, const int vertexVbo, const int indexVbo,
                                            int vertexCount,
                                            const glm::mat4 &transform, float shade) {

        glBindTexture(GL_TEXTURE_2D, textureId);
        glEnable(GL_DEPTH_TEST);
        glUniform4fv(fadeUniform, 1, &mFadeColour[0]);
        glUniform4f(uMod, shade, shade, shade, 1.0f);
        glUniformMatrix4fv(uView, 1, false, &mViewMatrix[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
        glEnableVertexAttribArray(vertexAttributePosition);
        glEnableVertexAttribArray(textureCoordinatesAttributePosition);

        //0 is for texturing unit 0 (since we never changed it)
        glUniform1i(samplerUniformPosition, 0);

        glUniformMatrix4fv(modelMatrixAttributePosition, 1, false, &transform[0][0]);
        glVertexAttribPointer(vertexAttributePosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
        glVertexAttribPointer(textureCoordinatesAttributePosition, 2, GL_FLOAT, GL_TRUE,
                              sizeof(float) * 5, (void *) (sizeof(float) * 3));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_SHORT, 0);

        glDisableVertexAttribArray(vertexAttributePosition);
        glDisableVertexAttribArray(textureCoordinatesAttributePosition);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void DungeonGLES2Renderer::deleteVBOs() {

        mVBORegisters.clear();
        mTextures.clear();

        glDeleteBuffers(1, &std::get<0>(mCubeVBO));
        glDeleteBuffers(1, &std::get<1>(mCubeVBO));

        glDeleteBuffers(1, &std::get<0>(mBillboardVBO));
        glDeleteBuffers(1, &std::get<1>(mBillboardVBO));

        glDeleteBuffers(1, &std::get<0>(mCornerLeftFarVBO));
        glDeleteBuffers(1, &std::get<1>(mCornerLeftFarVBO));

        glDeleteBuffers(1, &std::get<0>(mCornerLeftNearVBO));
        glDeleteBuffers(1, &std::get<1>(mCornerLeftNearVBO));

        glDeleteBuffers(1, &std::get<0>(mFloorVBO));
        glDeleteBuffers(1, &std::get<1>(mFloorVBO));

        glDeleteBuffers(1, &std::get<0>(mSkyVBO));
        glDeleteBuffers(1, &std::get<1>(mSkyVBO));
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

        mVBORegisters[ "cube" ] = mCubeVBO;
        mVBORegisters[ "leftfar" ] = mCornerLeftFarVBO;
        mVBORegisters[ "leftnear" ] = mCornerLeftNearVBO;
        mVBORegisters[ "floor" ] = mFloorVBO;

        initTileProperties();
    }

    void DungeonGLES2Renderer::clearBuffers() {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        checkGlError("glClearColor");
        glClearDepthf(1.0f);
        checkGlError("glClearDepthf");
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        checkGlError("glClear");
    }

    void DungeonGLES2Renderer::setPerspective() {
        glUniformMatrix4fv(projectionMatrixAttributePosition, 1, false, &projectionMatrix[0][0]);
    }

    void DungeonGLES2Renderer::prepareShaderProgram() {
        glUseProgram(gProgram);
        checkGlError("glUseProgram");
    }

    //independent code
    DungeonGLES2Renderer::DungeonGLES2Renderer() {
        projectionMatrix = glm::mat4(1.0f);
        vertexAttributePosition = 0;
        modelMatrixAttributePosition = 0;
        projectionMatrixAttributePosition = 0;
        gProgram = 0;
    }


    glm::mat4 DungeonGLES2Renderer::getCubeTransform(glm::vec3 translation) {
        glm::mat4 identity = glm::mat4(1.0f);
        glm::mat4 translated = glm::translate(identity, translation);

        return translated;
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
        mBitmaps.insert(mBitmaps.end(), begin(textures), end(textures));
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

        mViewMatrix = glm::lookAt(
                pos,
                pos_front + pos,
                glm::vec3(0.0f, 1.0, 0.0f)) * eyeMatrix;
    }

    void DungeonGLES2Renderer::produceRenderingBatches(IntMap map, CharMap actors,
                                                       IntMap splats,
                                                       IntMap lightMap, IntMap ids,
                                                       AnimationList movingCharacters,
                                                       long animationTime) {

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

                auto tile = map[z][x];
                auto actor = actors[z][x];
                int splatFrame = splats[z][x];

                Shade shade = (0.25f * std::min(255, lightMap[z][x]) / 255.0f) + 0.75f;

                if (x == static_cast<int>(mCursorPosition.x) &&
                    z == static_cast<int>(mCursorPosition.y)) {
                    shade = 1.5f;
                }

                if (mTileProperties.count(tile) <= 0) {
                    continue;
                }

                auto tileProperties = mTileProperties[tile];
                auto tileVBO = VBORegisterFrom( tileProperties.mVBOToRender );

                if (tileProperties.mCeilingTexture != mNullTexture) {
                    pos = glm::vec3(x * 2, -5.0f + (2.0 * tileProperties.mCeilingHeight), z * 2);
                    batches[textureIndexFrom(tileProperties.mCeilingTexture)].emplace_back(std::get<0>(mFloorVBO),
                                                                         std::get<1>(mFloorVBO),
                                                                         std::get<2>(mFloorVBO),
                                                                         getFloorTransform(pos),
                                                                         shade);
                }

                if (tileProperties.mCeilingRepeatedWallTexture != mNullTexture) {

                    for (float y = 0; y < tileProperties.mCeilingRepetitions; ++y) {

                        pos = glm::vec3(x * 2,
                                        -4.0f + (2.0f * tileProperties.mCeilingHeight) + (2.0 * y),
                                        z * 2);

                        batches[textureIndexFrom(tileProperties.mCeilingRepeatedWallTexture)].emplace_back(
                                std::get<0>(tileVBO),
                                std::get<1>(tileVBO),
                                std::get<2>(tileVBO),
                                getCubeTransform(pos),
                                shade);
                    }
                }

                if (tileProperties.mMainWallTexture != mNullTexture) {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);

                    batches[textureIndexFrom(tileProperties.mMainWallTexture)].emplace_back(
                            std::get<0>(tileVBO),
                            std::get<1>(tileVBO),
                            std::get<2>(tileVBO),
                            getCubeTransform(pos), shade);
                }

                if (tileProperties.mFloorRepeatedWallTexture != mNullTexture) {

                    for (float y = 0; y < tileProperties.mFloorRepetitions; ++y) {

                        //the final -1.0f in y is for accounting fore the block's length
                        pos = glm::vec3(x * 2,
                                        -5.0f + (2.0f * tileProperties.mFloorHeight) - (2.0 * y) -
                                        1.0f, z * 2);

                        batches[textureIndexFrom(tileProperties.mFloorRepeatedWallTexture)].emplace_back(
                                std::get<0>(tileVBO),
                                std::get<1>(tileVBO),
                                std::get<2>(tileVBO),
                                getCubeTransform(pos),
                                shade);
                    }
                }

                if (tileProperties.mFloorTexture != mNullTexture) {
                    pos = glm::vec3(x * 2, -5.0f + (2.0f * tileProperties.mFloorHeight), z * 2);
                    batches[textureIndexFrom(tileProperties.mFloorTexture)].emplace_back(std::get<0>(mFloorVBO),
                                                                       std::get<1>(mFloorVBO),
                                                                       std::get<2>(mFloorVBO),
                                                                       getFloorTransform(pos),
                                                                       shade);
                }

                //characters
                if (actor != EActorsSnapshotElement::kNothing) {

                    int id = ids[z][x];
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
                        fz = (curve * (destPosition.y - prevPosition.y)) + prevPosition.y;
                    }

                    pos = glm::vec3(fx * 2.0f, -4.0f, fz * 2.0f);


                    if (actor == EActorsSnapshotElement::kDemonAttacking0  ||
                            actor == EActorsSnapshotElement::kDemonAttacking1 ||
                            actor == EActorsSnapshotElement::kDemonStanding0 ||
                            actor == EActorsSnapshotElement::kDemonStanding1) {

                        TextureId frame = mElementMap[actor];

                        if (splatFrame > -1) {
                            frame = ETextures::Foe2a;
                        }

                        batches[static_cast<ETextures >(frame) ].emplace_back(
                                std::get<0>(mBillboardVBO),
                                std::get<1>(mBillboardVBO),
                                std::get<2>(mBillboardVBO),
                                getBillboardTransform(pos), shade);

                    } else {
                        mCurrentCharacterPosition = pos;
                    }
                }

                if (splatFrame > -1) {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);
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

    void DungeonGLES2Renderer::initTileProperties() {
        mElementMap[EActorsSnapshotElement::kDemonAttacking0] = ETextures::Foe1a;
        mElementMap[EActorsSnapshotElement::kDemonAttacking1] = ETextures::Foe1b;
        mElementMap[EActorsSnapshotElement::kDemonStanding0] = ETextures::Foe0a;
        mElementMap[EActorsSnapshotElement::kDemonStanding1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kHeroStanding0] = ETextures::Crusader0;
        mElementMap[EActorsSnapshotElement::kHeroStanding1] = ETextures::Crusader0;
        mElementMap[EActorsSnapshotElement::kHeroAttacking0] = ETextures::Crusader1;
        mElementMap[EActorsSnapshotElement::kHeroAttacking1] = ETextures::Crusader1;
    }

    void DungeonGLES2Renderer::invalidateCachedBatches() {
        batches.clear();
    }

    void DungeonGLES2Renderer::render(IntMap map, CharMap actors, IntMap splats,
                                      IntMap lightMap, IntMap ids,
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

        if ( batches.size() == 0 ) {
            produceRenderingBatches(map, actors, splats, lightMap, ids, movingCharacters,
                                    animationTime);
        }
        consumeRenderingBatches(animationTime);
    }

    void DungeonGLES2Renderer::consumeRenderingBatches(long animationTime) {

        for (auto &batch : batches) {

            auto textureId = mTextures[batch.first]->mTextureId;

            for (auto &element : batch.second) {
                auto transform = element.getTransform();
                auto shade = element.getShade();
                auto amount = element.getAmount();
                auto vboId = element.getVBOId();
                auto vboIndicesId = element.getVBOIndicesId();

                drawGeometry(textureId,
                             vboId,
                             vboIndicesId,
                             amount,
                             transform,
                             shade
                );
            }
        }
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

    void DungeonGLES2Renderer::setPlayerHealth(float health) {
        mPlayerHealth = health;
    }

    void DungeonGLES2Renderer::resetCamera() {
        mAngleXZ = 0;
        mAngleYZ = 0;
        mCameraRotation = 0;
        mRotationTarget = 0;
    }

    void DungeonGLES2Renderer::setTurn(int turn) {
        mTurn = turn;
    }

    VBORegister DungeonGLES2Renderer::VBORegisterFrom(VBORegisterId id) {
        return mVBORegisters[ id ];
    }

    ETextures DungeonGLES2Renderer::textureIndexFrom( TextureName name ) {
        return mTextureRegistry[ name ];
    }

    void DungeonGLES2Renderer::setTileProperties(CTilePropertyMap map ) {

        auto it = std::begin( map );
        auto mapEnd = std::end( map );

        while ( it != mapEnd ) {
            mTileProperties[ it->first ] = it->second;
            it = std::next( it );
        }
    }

    void DungeonGLES2Renderer::setMesh(std::string id, std::shared_ptr<odb::Scene> mesh) {

        auto m = std::begin( mesh->meshObjects );
        auto mEnd = std::end( mesh->meshObjects );

        while ( m != mEnd ) {
            auto& meshData = m->second->trigBatches[0];

            auto floatData = meshData.getVertexData();
            auto vertexCount = meshData.getVertexCount();
            auto indexData = meshData.getIndices();
            auto indicesCount = meshData.getIndicesCount();

            mVBORegisters[ m->first ] = submitVBO( floatData,
                                                    vertexCount,
                                                    indexData,
                                                    indicesCount );

            m = std::next( m );
        }
    }
}
