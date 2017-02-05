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
#else

#ifndef OSMESA
#include <GL/gl.h>
#else
# include <osmesa.h>     // For everything OpenGL, but done all in software.
#endif

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
    const static int kGeometryLineStride = 5;
    const static bool kShouldDestroyThingsManually = false;
	const static bool kFogEnabled = false;

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
            2, 1, 0,
		    3, 2, 0
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

        unsigned int dataIndex = GeometryBatches.size();
        unsigned int indicesIndex = IndicesBatches.size();


        GeometryBatches.push_back( { data, vertices } );
        IndicesBatches.push_back( { indexData, indices });

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

        odb::Logger::log("textureId:%d\n", textureId);

        return textureId;
    }


    extern void printGLString(const char *name, GLenum s) {
        const char *v = (const char *) glGetString(s);
        odb::Logger::log("GL %s = %s\n", name, v);
    }

    extern void checkGlError(const char *op) {
#ifndef OSMESA
        for (GLint error = glGetError(); error; error = glGetError()) {
            odb::Logger::log("after %s() glError (0x%x)\n", op, error);
        }
#endif
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

    bool DungeonGLES2Renderer::init(float w, float h, const std::string &vertexShader,
                                    const std::string &fragmentShader) {

        createVBOs();

        glEnable(GL_TEXTURE_2D);
        glShadeModel(GL_FLAT);
        glDisable(GL_DITHER);
        glDisable(GL_MULTISAMPLE);
#ifdef OSMESA
        glViewport(w/4, h/2, w/2, h/2);
#else
        glViewport(0, 0, w, h);
#endif
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity( );

        printVerboseDriverInformation();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable( GL_TEXTURE_2D );

	    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
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
            deleteVBOs();
            for (auto &texture : mTextures) {
                glDeleteTextures(1, &(texture->mTextureId));
            }
        }
    }

    void DungeonGLES2Renderer::fetchShaderLocations() {
    }

    void DungeonGLES2Renderer::drawGeometry(const unsigned int textureId, const int vertexVbo, const int indexVbo,
                                            int vertexCount,
                                            const glm::mat4 &transform, float shade) {


        auto geometryBatch = GeometryBatches[ vertexVbo ];
        auto indicesBatch = IndicesBatches[ indexVbo ];

        glLoadMatrixf( &(mViewMatrix*transform)[0][0] );

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

        mVBORegisters[ "cube" ] = mCubeVBO;
        mVBORegisters[ "leftfar" ] = mCornerLeftFarVBO;
        mVBORegisters[ "leftnear" ] = mCornerLeftNearVBO;
        mVBORegisters[ "floor" ] = mFloorVBO;

        initTileProperties();
    }

    void DungeonGLES2Renderer::clearBuffers() {

        checkGlError("glClearColor");
        glClearDepth(1.0f);
        checkGlError("glClearDepthf");
#ifndef OSMESA
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
#else
        glClear(GL_DEPTH_BUFFER_BIT );
#endif
        checkGlError("glClear");
    }

    void DungeonGLES2Renderer::setPerspective() {
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity( );
        glLoadMatrixf( &projectionMatrix[0][0]);
        glMatrixMode( GL_MODELVIEW );
    }

    void DungeonGLES2Renderer::prepareShaderProgram() {
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

#ifndef OSMESA
        if (mRotationTarget > mCameraRotation) {
            mCameraRotation += 5;
        } else if (mRotationTarget < mCameraRotation) {
            mCameraRotation -= 5;
        }
#else
	    if (mRotationTarget > mCameraRotation) {
            mCameraRotation += 45;
        } else if (mRotationTarget < mCameraRotation) {
            mCameraRotation -= 45;
        }

#endif
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

	    auto result = (glm::rotate(
			    glm::rotate(glm::mat4(1.0f), angleInRadiansXZ, glm::vec3(0.0f, 1.0f, 0.0f)),
			    angleInRadiansYZ, glm::vec3(1.0f, 0.0f, 0.0f)) * pos_front4 );

        mCameraDirection = glm::vec3( result.x, result.y, result.z );

        pos_front = mCameraDirection;

        mViewMatrix = glm::lookAt(
                pos,
                pos_front + pos,
                glm::vec3(0.0f, 1.0, 0.0f)) * eyeMatrix;
    }

    void
    DungeonGLES2Renderer::produceRenderingBatches(const IntMap& map,const CharMap& actors,const IntMap& splats,const IntMap& lightMap,const IntMap& ids,
                                                      const AnimationList& movingCharacters, long animationTime, const VisMap& visibilityMap) {

        glm::vec3 pos;

        batches[ETextures::Skybox].emplace_back(std::get<0>(mSkyVBO),
                                                std::get<1>(mSkyVBO),
                                                std::get<2>(mSkyVBO),
                                                getSkyTransform(animationTime),
                                                1.0f, false);

        batches[ETextures::Skybox].emplace_back(std::get<0>(mSkyVBO),
                                                std::get<1>(mSkyVBO),
                                                std::get<2>(mSkyVBO),
                                                getSkyTransform(
                                                        animationTime + kSkyTextureLength * 1000),
                                                1.0f, false);

		int lowerX = 0;
	    int lowerZ = 0;
	    int higherX = Knights::kMapSize;
	    int higherZ = Knights::kMapSize;

	    if (kFogEnabled ) {
		    lowerX = std::min<int>(std::max<int>(cameraPosition.x - 7, 0), Knights::kMapSize);
		    lowerZ = std::min<int>(std::max<int>(cameraPosition.y - 7, 0), Knights::kMapSize);
		    higherX = std::min<int>(std::max<int>(cameraPosition.x + 7, 0), Knights::kMapSize);
		    higherZ = std::min<int>(std::max<int>(cameraPosition.y + 7, 0), Knights::kMapSize);
	    }

        for (int z = lowerZ; z < higherZ; ++z) {
            for (int x = lowerX; x < higherX; ++x) {

//				if ( visibilityMap[z][x] == EVisibility::kInvisible) {
//					continue;
//				}

                auto tile = map[z][x];
                auto actor = actors[z][x];
                int splatFrame = splats[z][x];
	            Shade shade = 0;
#ifndef OSMESA
                shade = (0.25f * std::min(255, lightMap[z][x]) / 255.0f) + 0.75f;

                if (x == static_cast<int>(mCursorPosition.x) &&
                    z == static_cast<int>(mCursorPosition.y)) {
                    shade = 1.5f;
                }
#endif
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
                                                                         shade,
																		tileProperties.mNeedsAlphaTest);
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
                                shade,
                                tileProperties.mNeedsAlphaTest);
                    }
                }

                if (tileProperties.mMainWallTexture != mNullTexture) {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);

                    batches[textureIndexFrom(tileProperties.mMainWallTexture)].emplace_back(
                            std::get<0>(tileVBO),
                            std::get<1>(tileVBO),
                            std::get<2>(tileVBO),
                            getCubeTransform(pos),
                            shade,
                            tileProperties.mNeedsAlphaTest);

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
                                shade,
                                tileProperties.mNeedsAlphaTest);

                    }
                }

                if (tileProperties.mFloorTexture != mNullTexture) {
                    pos = glm::vec3(x * 2, -5.0f + (2.0f * tileProperties.mFloorHeight), z * 2);
                    batches[textureIndexFrom(tileProperties.mFloorTexture)].emplace_back(std::get<0>(mFloorVBO),
                                                                       std::get<1>(mFloorVBO),
                                                                       std::get<2>(mFloorVBO),
                                                                       getFloorTransform(pos),
                                                                       shade,
                                                                       tileProperties.mNeedsAlphaTest);

                }

	            //characters
	            if (actor != EActorsSnapshotElement::kNothing) {
		            const int id = ids[z][x];

                    float fx, fz;

		            fx = x;
		            fz = z;

		            float step = 0.0f;
		            float curve = 0.0f;

		            if (id != 0 && movingCharacters.count(id) > 0) {

			            auto animation = movingCharacters.at(id);

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



		            if (id == mCameraId) {
			            mCurrentCharacterPosition = pos;
		            } else {


			            TextureId frame = mElementMap[actor];

			            if (splatFrame > -1) {
				            frame = ETextures::Foe2a;
			            }

			            batches[static_cast<ETextures >(frame) ].emplace_back(
					            std::get<0>(mBillboardVBO),
					            std::get<1>(mBillboardVBO),
					            std::get<2>(mBillboardVBO),
					            getBillboardTransform(pos), shade, true);
		            }
	            }

                if (splatFrame > -1) {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);
                    batches[static_cast<ETextures >(splatFrame +
                                                    ETextures::Splat0)].emplace_back(
                            std::get<0>(mBillboardVBO),
                            std::get<1>(mBillboardVBO),
                            std::get<2>(mBillboardVBO),
                            getBillboardTransform(pos), shade, true);
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

    void DungeonGLES2Renderer::render(const IntMap& map, const CharMap& actors, const IntMap& splats,
                                      const IntMap& lightMap, const IntMap& ids,
                                      const AnimationList& movingCharacters,
                                      long animationTime, const VisMap& visibilityMap) {

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
                                    animationTime, visibilityMap);
        }
        consumeRenderingBatches(animationTime);
    }

    void DungeonGLES2Renderer::consumeRenderingBatches(long animationTime) {
	    glMatrixMode(GL_MODELVIEW);


        for (auto &batch : batches) {

            auto textureId = mTextures[batch.first]->mTextureId;
	        glBindTexture(GL_TEXTURE_2D, textureId);

            for (auto &element : batch.second) {
                auto transform = element.getTransform();
                auto shade = element.getShade();
                auto amount = element.getAmount();
                auto vboId = element.getVBOId();
                auto vboIndicesId = element.getVBOIndicesId();
#ifdef OSMESA
	            if ( element.mNeedsAlphaTest ) {
					glEnable( GL_ALPHA_TEST );
		            glAlphaFunc(GL_GREATER,0.5f);
	            } else {
		            glDisable( GL_ALPHA_TEST );
	            }

	            if ( vboId == std::get<0>(mCubeVBO) && !element.mNeedsAlphaTest ) {
		            glEnable( GL_CULL_FACE );
	            } else {
		            glDisable( GL_CULL_FACE );
	            }
#else
	            glEnable( GL_ALPHA_TEST );
	            glAlphaFunc(GL_GREATER,0.5f);
#endif
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

    void DungeonGLES2Renderer::setMesh(std::shared_ptr<odb::Scene> mesh) {

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

    void DungeonGLES2Renderer::setCameraId(int id) {
        mCameraId = id;
    }
}

#endif
