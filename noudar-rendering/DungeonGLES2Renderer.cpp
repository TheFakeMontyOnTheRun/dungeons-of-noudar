//
// Created by monty on 23/11/15.
//
#include <unistd.h>
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

#include <unordered_map>
#include <memory>
#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <map>
#include <vector>
#include <array>

using std::vector;
using std::array;


#include "CLerp.h"

#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "NativeBitmap.h"
#include "Logger.h"
#include "VBORenderingJob.h"
#include "IRenderer.h"
#include "NoudarDungeonSnapshot.h"
#include "ETextures.h"
#include "VBORegister.h"
#include "CTile3DProperties.h"
#include "Camera.h"
#include "RenderingJobSnapshotAdapter.h"
#include "DungeonGLES2Renderer.h"


namespace odb {
    const static bool kShouldDestroyThingsManually = true;

    const static glm::mat4 identity = glm::mat4(1.0f);
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        return textureId;
    }


    extern void printGLString(const char *name, GLenum s) {
        const char *v = (const char *) glGetString(s);
        odb::Logger::log("GL %s = %s", name, v);
    }

    extern void checkGlError(const char *op) {
        for (GLint error = glGetError(); error; error = glGetError()) {
            odb::Logger::log("after %s() glError (0x%x)", op, error);
        }
    }

    int DungeonGLES2Renderer::loadShader(EShaderType shaderType, const char *pSource) {

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
                        odb::Logger::log("Could not compile shader %d:\n%s", shaderType, buf);
                        free(buf);
                    }
                    glDeleteShader(shader);
                    shader = 0;
                }
            }
        }
        return shader;
    }

    int DungeonGLES2Renderer::createProgram(const char *pVertexSource,
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
                        odb::Logger::log("Could not link program:\n%s", buf);
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

    void DungeonGLES2Renderer::reloadTextures() {
#if !defined(__ANDROID__ )
        unloadTextures();
        mTextures.clear();
#endif


        for (auto &bitmapList : mBitmaps) {
            vector<unsigned int> tex;
            for (auto &bitmap : bitmapList) {
                tex.push_back(uploadTextureData(bitmap));
            }
            mTextures.push_back(tex);
        }

        mBitmaps.clear();
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

        projectionMatrix = glm::perspective(45.0f, w / h, 0.1f, 100.0f);

        glViewport(0, 0, w, h);
        checkGlError("glViewport");

        mTextureRegistry["sky"] = ETextures::Skybox;
        mTextureRegistry["grass"] = ETextures::Grass;
        mTextureRegistry["lava"] = ETextures::Lava;
        mTextureRegistry["floor"] = ETextures::Floor;
        mTextureRegistry["bricks"] = ETextures::Bricks;
        mTextureRegistry["arch"] = ETextures::Arch;
        mTextureRegistry["bars"] = ETextures::Bars;
        mTextureRegistry["bricksblood"] = ETextures::BricksBlood;
        mTextureRegistry["brickscandles"] = ETextures::BricksCandles;
        mTextureRegistry["ceiling"] = ETextures::Ceiling;
        mTextureRegistry["ceilingdoor"] = ETextures::CeilingDoor;
        mTextureRegistry["ceilingbegin"] = ETextures::CeilingBegin;
        mTextureRegistry["ceilingend"] = ETextures::CeilingEnd;
        mTextureRegistry["ceilingbars"] = ETextures::CeilingBars;
        mTextureRegistry["rope"] = ETextures::Rope;
        mTextureRegistry["slot"] = ETextures::Slot;
        mTextureRegistry["magicseal"] = ETextures::MagicSeal;
        mTextureRegistry["cobblestone"] = ETextures::Cobblestone;
        mTextureRegistry["fence"] = ETextures::Fence;
        createVBOs();
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glFrontFace(GL_CW);
        glDepthMask(true);
        startFadingIn();
        return true;
    }

    void DungeonGLES2Renderer::unloadTextures() {
        for (auto &texture : mTextures) {
            for (auto &textureId : texture) {
                glDeleteTextures(1, &textureId);
            }
        }
    }

    DungeonGLES2Renderer::~DungeonGLES2Renderer() {
        odb::Logger::log("Destroying the renderer");

        if (kShouldDestroyThingsManually) {
            unloadTextures();
            deleteVBOs();
            glDeleteShader(gProgram);
        }
    }

    void DungeonGLES2Renderer::fetchShaderLocations() {

        vertexAttributePosition = glGetAttribLocation(gProgram, "aPosition");
        projectionViewMatrixAttributePosition = glGetUniformLocation(gProgram, "uProjectionView");
        samplerUniformPosition = glGetUniformLocation(gProgram, "sTexture");
        textureCoordinatesAttributePosition = glGetAttribLocation(gProgram, "aTexCoord");

        uMod = glGetUniformLocation(gProgram, "uMod");
        fadeUniform = glGetUniformLocation(gProgram, "uFade");
        uScale = glGetUniformLocation(gProgram, "uScale");
    }

    void DungeonGLES2Renderer::deleteVBOs() {

        mTextures.clear();

        for (auto &vbo : mVBORegisters) {
            unsigned int get0 = (GLuint) std::get<0>(vbo.second);
            unsigned int get1 = (GLuint) std::get<1>(vbo.second);

            glDeleteBuffers(1, &get0);
            glDeleteBuffers(1, &get1);
        }
    }

    void DungeonGLES2Renderer::createVBOs() {

        mVBORegisters["cube"] = submitVBO((float *) cubeVertices, 16,
                                          (unsigned short *) cubeIndices, 24);

        mVBORegisters["billboard"] = submitVBO((float *) billboardVertices, 4,
                                               (unsigned short *) billboardIndices, 6);
        mVBORegisters["leftfar"] = submitVBO((float *) cornerLeftFarVertices, 4,
                                             (unsigned short *) cornerLeftFarIndices, 6);
        mVBORegisters["leftnear"] = submitVBO((float *) cornerLeftNearVertices, 4,
                                              (unsigned short *) cornerLeftNearIndices, 6);
        mVBORegisters["floor"] = submitVBO((float *) floorVertices, 4,
                                           (unsigned short *) floorIndices, 6);

        mVBORegisters["sky"] = submitVBO((float *) skyVertices, 4, (unsigned short *) skyIndices,
                                         6);


        mBillboardVBOVertexId = std::get<0>(mVBORegisters["billboard"]);
        mSkyboxVBOVertexId = std::get<0>(mVBORegisters["sky"]);

        initTileProperties();
    }

    void DungeonGLES2Renderer::clearBuffers() {
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        checkGlError("glClearColor");
        glClearDepthf(1.0f);
        checkGlError("glClearDepthf");
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        checkGlError("glClear");
    }

    void DungeonGLES2Renderer::setPerspective() {

    }

    void DungeonGLES2Renderer::prepareShaderProgram() {
        glUseProgram(gProgram);
        checkGlError("glUseProgram");
    }

    //independent code
    DungeonGLES2Renderer::DungeonGLES2Renderer() {
        projectionMatrix = glm::mat4(1.0f);
        vertexAttributePosition = 0;
        projectionViewMatrixAttributePosition = 0;
        gProgram = 0;
    }

    void DungeonGLES2Renderer::updateFadeState(long ms) {
        mFadeLerp.update(ms);
    }

    void DungeonGLES2Renderer::setTexture(vector<vector<std::shared_ptr<NativeBitmap>>> textures) {
        mBitmaps.clear();
        mBitmaps.insert(mBitmaps.end(), std::begin(textures), std::end(textures));
    }

    void DungeonGLES2Renderer::shutdown() {
        odb::Logger::log("Shutdown!");
    }

    void DungeonGLES2Renderer::startFadingIn() {
        mFadeLerp = CLerp(0, 1000, 1000);
    }

    void DungeonGLES2Renderer::startFadingOut() {
        mFadeLerp = CLerp(1000, 0, 1000);
    }

    void DungeonGLES2Renderer::updateCamera(long ms) {
        mCamera.update(ms);
    }

    void DungeonGLES2Renderer::resetTransformMatrices() {
        mViewMatrix = mCamera.getViewMatrix(mCurrentCharacterPosition);
    }

    void
    DungeonGLES2Renderer::produceRenderingBatches(const NoudarDungeonSnapshot &snapshot) {

        glm::vec3 pos;

        batches.clear();

        mSnapshotAdapter.readSnapshot(snapshot, batches, mTileProperties, mVBORegisters,
                                      mTextureRegistry, mCamera, mElementMap);

        for (int z = 0; z < Knights::kMapSize; ++z) {
            for (int x = 0; x < Knights::kMapSize; ++x) {

                int id = snapshot.ids[z][x];
                auto mapItem = snapshot.map[z][x];
                float fx, fz, height;

                fx = x;
                fz = z;
                height = mTileProperties[mapItem].mFloorHeight;
                if (id != 0 && snapshot.movingCharacters.count(id) > 0) {

                    auto animation = snapshot.movingCharacters.at(id);
                    auto pos = mSnapshotAdapter.easingAnimationCurveStep(std::get<0>(animation),
                                                                         std::get<1>(animation),
                                                                         std::get<2>(animation),
                                                                         snapshot.mTimestamp);

                    fx = pos.x;
                    fz = pos.y;
                }

                pos = glm::vec3(fx * 2.0f, -4.0f + 2.0f * height, fz * 2.0f);

                if (id != 0 && id == snapshot.mCameraId) {
                    mCurrentCharacterPosition = pos;
                    return;
                }
            }
        }
    }

    void DungeonGLES2Renderer::initTileProperties() {
        mElementMap[EActorsSnapshotElement::kRope] = ETextures::Rope;
        mElementMap[EActorsSnapshotElement::kMagicSeal] = ETextures::MagicSeal;
        mElementMap[EActorsSnapshotElement::kStrongDemonAttacking0] = ETextures::StrongDemonAttack0;
        mElementMap[EActorsSnapshotElement::kStrongDemonAttacking1] =
                ETextures::StrongDemonAttack0;
        mElementMap[EActorsSnapshotElement::kWeakenedDemonAttacking0] = ETextures::WeakDemonAttack0;
        mElementMap[EActorsSnapshotElement::kWeakenedDemonAttacking1] =
                ETextures::WeakDemonAttack0;

        mElementMap[EActorsSnapshotElement::kCocoonStanding0] = ETextures::CocoonStanding0;
        mElementMap[EActorsSnapshotElement::kCocoonStanding1] =
                ETextures::CocoonStanding0;
        mElementMap[EActorsSnapshotElement::kEvilSpiritAttacking0] = ETextures::EvilSpiritAttack0;
        mElementMap[EActorsSnapshotElement::kEvilSpiritAttacking1] =
                ETextures::EvilSpiritAttack0;
        mElementMap[EActorsSnapshotElement::kWarthogAttacking0] = ETextures::WarthogAttack0;
        mElementMap[EActorsSnapshotElement::kWarthogAttacking1] =
                ETextures::WarthogAttack0;
        mElementMap[EActorsSnapshotElement::kMonkAttacking0] = ETextures::MonkAttack0;
        mElementMap[EActorsSnapshotElement::kMonkAttacking1] =
                ETextures::MonkAttack0;
        mElementMap[EActorsSnapshotElement::kFallenAttacking0] = ETextures::FallenAttack0;
        mElementMap[EActorsSnapshotElement::kFallenAttacking1] =
                ETextures::FallenAttack0;

        mElementMap[EActorsSnapshotElement::kStrongDemonStanding0] =
                ETextures::StrongDemonAttack0;
        mElementMap[EActorsSnapshotElement::kStrongDemonStanding1] =
                ETextures::StrongDemonAttack0;
        mElementMap[EActorsSnapshotElement::kWeakenedDemonStanding0] = ETextures::WeakDemonAttack0;
        mElementMap[EActorsSnapshotElement::kWeakenedDemonStanding1] =
                ETextures::WeakDemonAttack0;

        mElementMap[EActorsSnapshotElement::kEvilSpiritStanding0] = ETextures::EvilSpiritAttack0;
        mElementMap[EActorsSnapshotElement::kEvilSpiritStanding1] =
                ETextures::EvilSpiritAttack0;
        mElementMap[EActorsSnapshotElement::kWarthogStanding0] = ETextures::WarthogAttack0;
        mElementMap[EActorsSnapshotElement::kWarthogStanding1] =
                ETextures::WarthogAttack0;
        mElementMap[EActorsSnapshotElement::kMonkStanding0] = ETextures::MonkAttack0;
        mElementMap[EActorsSnapshotElement::kMonkStanding1] =
                ETextures::MonkAttack0;
        mElementMap[EActorsSnapshotElement::kFallenStanding0] = ETextures::FallenAttack0;
        mElementMap[EActorsSnapshotElement::kFallenStanding1] =
                ETextures::FallenAttack0;
    }

    void DungeonGLES2Renderer::invalidateCachedBatches() {
        batches.clear();
    }

    void DungeonGLES2Renderer::render(const NoudarDungeonSnapshot &snapshot) {

        if (mTextures.empty()) {
            return;
        }

        ++frame;

        clearBuffers();
        prepareShaderProgram();
        setPerspective();
        resetTransformMatrices();

        invalidateCachedBatches();

        if (batches.empty()) {
            produceRenderingBatches(snapshot);
        }

        consumeRenderingBatches(snapshot.mTimestamp);
    }

    void DungeonGLES2Renderer::consumeRenderingBatches(long animationTime) {
        glm::vec4 mFadeColour = glm::vec4(0.0f, 0.0f, 0.0f, mFadeLerp.getCurrentValue() / 1000.0f);

        glUniform4fv(fadeUniform, 1, &mFadeColour[0]);



        glDepthFunc(GL_LEQUAL);
        glEnableVertexAttribArray(vertexAttributePosition);
        glEnableVertexAttribArray(textureCoordinatesAttributePosition);

        glUniformMatrix4fv(uScale, 1, false, &identity[0][0]);

        //0 is for texturing unit 0 (since we never changed it)
        glUniform1i(samplerUniformPosition, 0);

        for (const auto &batch : batches) {

            auto textureId = mTextures[batch.first];
            glBindTexture(GL_TEXTURE_2D, textureId[(frame / 4) % textureId.size()]);

            for (const auto &element : batch.second) {

                if (!element.mNeedsZBuffer) {
                    continue;
                }

                const auto &transform = element.getTransform();
                const auto &shade = element.getShade();
                const auto &amount = element.getAmount();
                const auto &vboId = element.getVBOId();
                const auto &vboIndicesId = element.getVBOIndicesId();

                drawGeometry(vboId,
                             vboIndicesId,
                             amount,
                             transform,
                             shade
                );
            }
        }

        glDepthFunc(GL_ALWAYS);

        for (const auto &batch : batches) {

            auto textureId = mTextures[batch.first];
            glBindTexture(GL_TEXTURE_2D, textureId[(frame / 4) % textureId.size()]);

            for (const auto &element : batch.second) {

                if (element.mNeedsZBuffer) {
                    continue;
                }

                const auto &transform = element.getTransform();
                const auto &shade = element.getShade();
                const auto &amount = element.getAmount();
                const auto &vboId = element.getVBOId();
                const auto &vboIndicesId = element.getVBOIndicesId();

                drawGeometry(vboId,
                             vboIndicesId,
                             amount,
                             transform,
                             shade
                );
            }
        }
        glDisableVertexAttribArray(vertexAttributePosition);
        glDisableVertexAttribArray(textureCoordinatesAttributePosition);
    }

    void DungeonGLES2Renderer::drawGeometry(const int vertexVbo,
                                            const int indexVbo,
                                            int vertexCount,
                                            const glm::mat4 &transform, float shade) {

        glUniform4f(uMod, shade, shade, shade, 1.0f);

        if (vertexVbo != mSkyboxVBOVertexId && vertexVbo != mBillboardVBOVertexId) {
            glm::mat2 scale(1.0f);
            scale[0][0] = transform[0][0];
            scale[1][1] = transform[1][1];
            glUniformMatrix2fv(uScale, 1, false, &scale[0][0]);
        }

        glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);

        glm::mat4 projectionView =  projectionMatrix * mViewMatrix * transform;

        glUniformMatrix4fv(projectionViewMatrixAttributePosition, 1, false, &projectionView[0][0]);

        glVertexAttribPointer(vertexAttributePosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
        glVertexAttribPointer(textureCoordinatesAttributePosition, 2, GL_FLOAT, GL_TRUE,
                              sizeof(float) * 5, (void *) (sizeof(float) * 3));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo);

        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_SHORT, 0);

        if (vertexVbo != mSkyboxVBOVertexId && vertexVbo != mBillboardVBOVertexId) {
            glm::mat2 scale(1.0f);
            glUniformMatrix2fv(uScale, 1, false, &scale[0][0]);
        }
    }

    void DungeonGLES2Renderer::rotateLeft() {
        mCamera.incrementRotateTarget(-90);
    }

    void DungeonGLES2Renderer::rotateRight() {
        mCamera.incrementRotateTarget(90);
    }

    bool DungeonGLES2Renderer::isAnimating() {
        return mCamera.isAnimating();
    }

    void DungeonGLES2Renderer::setEyeView(float *eyeView) {
        mCamera.setEyeView(eyeView);
    }

    void DungeonGLES2Renderer::setPerspectiveMatrix(float *perspectiveMatrix) {
        projectionMatrix = glm::make_mat4(perspectiveMatrix);
    }

    void DungeonGLES2Renderer::setAngleXZ(float xz) {
        mCamera.setRotationXZ(xz);
    }

    void DungeonGLES2Renderer::setAngleYZ(float yz) {
        mCamera.setRotationYZ(yz);
    }

    void DungeonGLES2Renderer::resetCamera() {
        mCamera.reset();
    }

    void DungeonGLES2Renderer::setTileProperties(CTilePropertyMap map) {

        mTileProperties.clear();

        auto it = std::begin(map);
        auto mapEnd = std::end(map);

        while (it != mapEnd) {
            mTileProperties[it->first] = it->second;
            it = std::next(it);
        }
    }
}
