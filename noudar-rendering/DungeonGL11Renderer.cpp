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

#include <functional>
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <map>
#include <array>
#include <unordered_map>
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
#include "Material.h"
#include "Trig.h"
#include "TrigBatch.h"
#include "MeshObject.h"
#include "MaterialList.h"
#include "Scene.h"
#include "RenderingJobSnapshotAdapter.h"
#include "CLerp.h"
#include "Camera.h"
#include "DungeonGLES2Renderer.h"

namespace odb {

	struct TGeometryBatch {
		float *vertices;
		int sizeVertices;
	};

	struct TIndicesBatch {
		unsigned short *indices;
		unsigned int sizeIndices;
	};

	std::vector<TGeometryBatch> GeometryBatches; //bitches!
	std::vector<TIndicesBatch> IndicesBatches; //bitches!
	const static int kGeometryLineStride = 5;
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
#ifndef OSMESA
	const float DungeonGLES2Renderer::skyVertices[]{
			-kSkyTextureLength - 20.0f, 10.0f, -200.0f, 0.0f, .0f,
			-20.0f, 10.0f, -200.0f, 10.0f, 0.0f,
			-20.0f, 10.0f, 200.0f, 10.0f, 10.0f,
			-kSkyTextureLength - 20.0f, 10.0f, 200.0f, 0.0f, 10.0f,
	};
#endif
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

#ifdef OSMESA
    static int DungeonGLES2Renderer::visibility = 8;
#endif

	VBORegister DungeonGLES2Renderer::submitVBO(float *data, int vertices,
	                                            unsigned short *indexData,
	                                            unsigned int indices) {

		unsigned int dataIndex = GeometryBatches.size();
		unsigned int indicesIndex = IndicesBatches.size();


		GeometryBatches.push_back({data, vertices});
		IndicesBatches.push_back({indexData, indices});

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

#ifndef OSMESA
		odb::Logger::log("textureId:%d\n", textureId);
#endif
		return textureId;
	}


	extern void printGLString(const char *name, GLenum s) {
#ifndef OSMESA
		const char *v = (const char *) glGetString(s);
		odb::Logger::log("GL %s = %s\n", name, v);
#endif
	}

	extern void checkGlError(const char *op) {
#ifndef OSMESA
		for (GLint error = glGetError(); error; error = glGetError()) {
			odb::Logger::log("after %s() glError (0x%x)\n", op, error);
		}
#endif
	}

	int DungeonGLES2Renderer::loadShader(EShaderType shaderType, const char *pSource) {
		return 1;
	}

	int DungeonGLES2Renderer::createProgram(const char *pVertexSource,
	                                           const char *pFragmentSource) {
		return 1;
	}

	void DungeonGLES2Renderer::printVerboseDriverInformation() {
	}

	void DungeonGLES2Renderer::reloadTextures() {
        unloadTextures();
        mTextures.clear();

		for (auto &bitmap : mBitmaps) {
#ifndef OSMESA
			odb::Logger::log("index: %d", index);
#endif
			mTextures.push_back(uploadTextureData(bitmap));
		}

		mBitmaps.clear();
	}

	bool DungeonGLES2Renderer::init(float w, float h, const std::string &vertexShader,
	                                const std::string &fragmentShader) {

		createVBOs();

		glEnable(GL_TEXTURE_2D);
		glShadeModel(GL_FLAT);
		glDisable(GL_DITHER);
//		glDisable(GL_MULTISAMPLE);
		glViewport(0, 0, w, h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		printVerboseDriverInformation();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		gProgram = createProgram(vertexShader.c_str(), fragmentShader.c_str());

		if (!gProgram) {
			odb::Logger::log("Could not create program.");
			return false;
		}

		fetchShaderLocations();

		checkGlError("glViewport");

		projectionMatrix = glm::perspective(45.0f, w / h, 0.1f, 100.0f);


		int index = 0;
#ifndef OSMESA
		odb::Logger::log("bitmaps size as to upload: %d", mBitmaps.size());
#endif

		mTextureRegistry["sky"] = ETextures::Skybox;
        mTextureRegistry["grass"] = ETextures::Grass;
        mTextureRegistry["grass2"] = ETextures::Grass2;
        mTextureRegistry["floor"] = ETextures::Floor;
        mTextureRegistry["bricks"] = ETextures::Bricks;
        mTextureRegistry["arch"] = ETextures::Arch;
        mTextureRegistry["bars"] = ETextures::Bars;
        mTextureRegistry["begin"] = ETextures::Begin;
        mTextureRegistry["exit"] = ETextures::Exit;
        mTextureRegistry["bricksblood"] = ETextures::BricksBlood;
        mTextureRegistry["brickscandles"] = ETextures::BricksCandles;
        mTextureRegistry["stonegrassfar"] = ETextures::StoneGrassFar;
        mTextureRegistry["grassstonefar"] = ETextures::GrassStoneFar;
        mTextureRegistry["stonegrassnear"] = ETextures::StoneGrassNear;
        mTextureRegistry["grassstonenear"] = ETextures::GrassStoneNear;
        mTextureRegistry["ceiling"] = ETextures::Ceiling;
        mTextureRegistry["ceilingdoor"] = ETextures::CeilingDoor;
        mTextureRegistry["ceilingbegin"] = ETextures::CeilingBegin;
        mTextureRegistry["ceilingend"] = ETextures::CeilingEnd;
        mTextureRegistry["ceilingbars"] = ETextures::CeilingBars;


		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glFrontFace(GL_CW);
		glDepthMask(true);
		startFadingIn();
		return true;
	}

	void DungeonGLES2Renderer::unloadTextures() {
		for (auto &texture : mTextures) {
			glDeleteTextures(1, &texture);
		}
	}

	DungeonGLES2Renderer::~DungeonGLES2Renderer() {
		odb::Logger::log("Destroying the renderer");

		if (kShouldDestroyThingsManually) {
			deleteVBOs();
			unloadTextures();
		}
	}

	void DungeonGLES2Renderer::fetchShaderLocations() {
	}

	void DungeonGLES2Renderer::drawGeometry(const unsigned int textureId, const int vertexVbo, const int indexVbo,
	                                        int vertexCount,
	                                        const glm::mat4 &transform, float shade) {


		auto geometryBatch = GeometryBatches[vertexVbo];
		auto indicesBatch = IndicesBatches[indexVbo];

		glLoadMatrixf(&(mViewMatrix * transform)[0][0]);

		glBegin(GL_TRIANGLES);

		for (int i = 0; i < indicesBatch.sizeIndices; ++i) {
			unsigned short index = indicesBatch.indices[i];

			float *line = geometryBatch.vertices + (index * kGeometryLineStride);
			glTexCoord2f(line[3], line[4]);
#ifndef OSMESA
            glColor3f( shade, shade, shade );
#endif
			glVertex3f(line[0], line[1], line[2]);
		}

		glEnd();
	}

	void DungeonGLES2Renderer::deleteVBOs() {
	}

	void DungeonGLES2Renderer::createVBOs() {

		mVBORegisters["cube"] = submitVBO((float *) cubeVertices, 16, (unsigned short *) cubeIndices, 24);

		mVBORegisters["billboard"] = submitVBO((float *) billboardVertices, 4,
		                                       (unsigned short *) billboardIndices, 6);
		mVBORegisters["leftfar"] = submitVBO((float *) cornerLeftFarVertices, 4,
		                                     (unsigned short *) cornerLeftFarIndices, 6);
		mVBORegisters["leftnear"] = submitVBO((float *) cornerLeftNearVertices, 4,
		                                      (unsigned short *) cornerLeftNearIndices, 6);
		mVBORegisters["floor"] = submitVBO((float *) floorVertices, 4, (unsigned short *) floorIndices, 6);
#ifndef OSMESA
		mVBORegisters["sky"] = submitVBO((float *) skyVertices, 4, (unsigned short *) skyIndices, 6);
#endif
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
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
#endif
		checkGlError("glClear");
	}

	void DungeonGLES2Renderer::setPerspective() {
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&projectionMatrix[0][0]);
		glMatrixMode(GL_MODELVIEW);
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

	void DungeonGLES2Renderer::updateFadeState(long ms) {
		mFadeLerp.update( ms );
	}

	void DungeonGLES2Renderer::setTexture(std::vector<std::shared_ptr<NativeBitmap>> textures) {
		mBitmaps.clear();
		mBitmaps.insert(mBitmaps.end(), begin(textures), end(textures));
	}

	void DungeonGLES2Renderer::shutdown() {
		odb::Logger::log("Shutdown!\n");
	}

	void DungeonGLES2Renderer::startFadingIn() {
		mFadeLerp = CLerp( 0, 1000, 1000 );
	}

	void DungeonGLES2Renderer::startFadingOut() {
		mFadeLerp = CLerp( 1000, 0, 1000 );
	}

	void DungeonGLES2Renderer::updateCamera(long ms) {
		mCamera.update(ms);
	}

	void DungeonGLES2Renderer::resetTransformMatrices() {

		mViewMatrix = mCamera.getViewMatrix( mCurrentCharacterPosition );
	}

	void
	DungeonGLES2Renderer::produceRenderingBatches(const NoudarDungeonSnapshot &snapshot) {
        glm::vec3 pos;
        const auto &billboardVBO = mVBORegisters["billboard"];

#ifdef OSMESA
        mSnapshotAdapter.visibility = visibility;
#endif
        mSnapshotAdapter.readSnapshot(snapshot, batches, mTileProperties, mVBORegisters, mTextureRegistry);

#ifdef OSMESA
        auto x0 = std::max( 0, snapshot.mCameraPosition.x - visibility );
        auto x1 = std::min( Knights::kMapSize, snapshot.mCameraPosition.x + visibility );
        auto z0 = std::max( 0, snapshot.mCameraPosition.y - visibility );
        auto z1 = std::min( Knights::kMapSize, snapshot.mCameraPosition.y + visibility );
#else
        auto x0 = 0;
        auto x1 = Knights::kMapSize - 1;
        auto z0 = 0;
        auto z1 = Knights::kMapSize - 1;
#endif

        for (int z = z0; z <= z1; ++z) {
            for (int x = x0; x <= x1; ++x) {

                if (snapshot.mVisibilityMap[z][x] == EVisibility::kInvisible) {
                    continue;
                }

                auto mapItem = snapshot.mItemMap[ z ][ x ];
                auto actor = snapshot.snapshot[z][x];
                int splatFrame = snapshot.splat[z][x];
#ifndef OSMESA
				Shade shade = ( snapshot.mLightMap[z][x] ) / 255.0f;
#else
                Shade shade = 1.0f;
#endif
				if ( z == snapshot.mCursorPosition.y && x == snapshot.mCursorPosition.y ) {
					shade = 1.5 * shade;
				}


				if ( mapItem == 't') {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);
                    batches[ETextures::Falcata].emplace_back(
                            std::get<0>(billboardVBO),
                            std::get<1>(billboardVBO),
                            std::get<2>(billboardVBO),
                            getBillboardTransform(pos), shade, true);

                }
                if ( mapItem == '+') {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);
                    batches[ETextures::Cross].emplace_back(
                            std::get<0>(billboardVBO),
                            std::get<1>(billboardVBO),
                            std::get<2>(billboardVBO),
                            getBillboardTransform(pos), shade, true);

                }
                if ( mapItem == 'y') {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);
                    batches[ETextures::Crossbow].emplace_back(
                            std::get<0>(billboardVBO),
                            std::get<1>(billboardVBO),
                            std::get<2>(billboardVBO),
                            getBillboardTransform(pos), shade, true);

                }

                if ( mapItem == 'u') {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);
                    batches[ETextures::Quiver].emplace_back(
                            std::get<0>(billboardVBO),
                            std::get<1>(billboardVBO),
                            std::get<2>(billboardVBO),
                            getBillboardTransform(pos), shade, true);

                }



                if (x == static_cast<int>(snapshot.mCursorPosition.x) &&
                    z == static_cast<int>(snapshot.mCursorPosition.y)) {
                    shade = 1.5f;
                }

                //characters
                if (actor != EActorsSnapshotElement::kNothing) {

                    int id = snapshot.ids[z][x];
                    float fx, fz, height;

                    fx = x;
                    fz = z;
                    height = mTileProperties[ mapItem ].mFloorHeight;

                    if (id != 0 && snapshot.movingCharacters.count(id) > 0) {

                        auto animation = snapshot.movingCharacters.at(id);
                        auto pos = mSnapshotAdapter.easingAnimationCurveStep(std::get<0>(animation),
                                                                             std::get<1>(animation),
                                                                             std::get<2>(animation),
                                                                             snapshot.mTimestamp);

                        fx = pos.x;
                        fz = pos.y;

                    }

                    pos = glm::vec3(fx * 2.0f, -4.0f + 2 * height, fz * 2.0f);


                    if (id != snapshot.mCameraId) {

                        TextureId frame = mElementMap[actor];

                        if (splatFrame > -1) {
                            frame = ETextures::Foe2a;
                        }

                        batches[static_cast<ETextures >(frame)].emplace_back(
                                std::get<0>(billboardVBO),
                                std::get<1>(billboardVBO),
                                std::get<2>(billboardVBO),
                                getBillboardTransform(pos), shade, true);
                    }
                }

                if (splatFrame > -1) {
                    float height = mTileProperties[ mapItem ].mFloorHeight;
                    pos = glm::vec3(x * 2, -4.0f + 2.0f * height, z * 2);
                    batches[static_cast<ETextures >(splatFrame +
                                                    ETextures::Splat0)].emplace_back(
                            std::get<0>(billboardVBO),
                            std::get<1>(billboardVBO),
                            std::get<2>(billboardVBO),
                            getBillboardTransform(pos), shade, true);
                }
            }
        }

        for (int z = z0; z <= z1; ++z) {
            for (int x = x0; x <= x1; ++x) {

                int id = snapshot.ids[z][x];
                auto mapItem = snapshot.map[ z ][ x ];
                float fx, fz, height;

                fx = x;
                fz = z;
                height = mTileProperties[ mapItem ].mFloorHeight;
                if (id != 0 && snapshot.movingCharacters.count(id) > 0) {

                    auto animation = snapshot.movingCharacters.at(id);
                    auto pos = mSnapshotAdapter.easingAnimationCurveStep(std::get<0>(animation), std::get<1>(animation),
                                                                         std::get<2>(animation), snapshot.mTimestamp);

                    fx = pos.x;
                    fz = pos.y;
                }

                pos = glm::vec3(fx * 2.0f, -4.0f + 2.0f * height, fz * 2.0f);

                if (id == snapshot.mCameraId) {
                    mCurrentCharacterPosition = pos;
                    return;
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
        mElementMap[EActorsSnapshotElement::kWeakenedDemonSpiritAttacking0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kWeakenedDemonAttacking1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kWeakenedDemonStanding0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kWeakenedDemonStanding1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kCocoonStanding0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kCocoonStanding1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kEvilSpiritAttacking0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kEvilSpiritAttacking1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kEvilSpiritStanding0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kEvilSpiritStanding1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kWarthogAttacking0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kWarthogAttacking1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kWarthogStanding0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kWarthogStanding1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kMonkAttacking0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kMonkAttacking1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kMonkStanding0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kMonkStanding1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kFallenAttacking0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kFallenAttacking1] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kFallenStanding0] = ETextures::Foe0b;
        mElementMap[EActorsSnapshotElement::kFallenStanding1] = ETextures::Foe0b;
	}

	void DungeonGLES2Renderer::invalidateCachedBatches() {
		batches.clear();
	}

	void DungeonGLES2Renderer::render(const NoudarDungeonSnapshot &snapshot) {

		if (mTextures.empty()) {
			return;
		}

		clearBuffers();
		prepareShaderProgram();
		setPerspective();
		resetTransformMatrices();

//	    bool containsCamera = false;
//
//	    for ( const auto& movement : movingCharacters ) {
//		    if ( std::get<0>(movement) == mCameraId ) {
//			    containsCamera = true;
//		    }
//	    }

//	    if ( isAnimating() || containsCamera ) {

		invalidateCachedBatches();

		if (batches.size() == 0) {
			produceRenderingBatches(snapshot);
		}
//	    }
		consumeRenderingBatches(snapshot.mTimestamp);
	}

	void DungeonGLES2Renderer::consumeRenderingBatches(long animationTime) {
		glMatrixMode(GL_MODELVIEW);

#ifdef OSMESA
	auto cubeVBO = mVBORegisters["cube"];
#endif

		for (const auto &batch : batches) {

			auto textureId = mTextures[batch.first];
			glBindTexture(GL_TEXTURE_2D, textureId);

			for (const auto &element : batch.second) {
				const auto &transform = element.getTransform();
				const auto &shade = element.getShade();
				const auto &amount = element.getAmount();
				const auto &vboId = element.getVBOId();
				const auto &vboIndicesId = element.getVBOIndicesId();
#ifdef OSMESA
				if ( element.mNeedsAlphaTest ) {
					glEnable( GL_ALPHA_TEST );
					glAlphaFunc(GL_GREATER,0.5f);
				} else {
					glDisable( GL_ALPHA_TEST );
				}

				if ( vboId == std::get<0>(cubeVBO) && !element.mNeedsAlphaTest ) {
					glEnable( GL_CULL_FACE );
				} else {
					glDisable( GL_CULL_FACE );
				}
#else
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.5f);
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
		mCamera.incrementRotateTarget( -90 );
	}

	void DungeonGLES2Renderer::rotateRight() {
		mCamera.incrementRotateTarget( 90 );
	}

	glm::mat4 DungeonGLES2Renderer::getBillboardTransform(glm::vec3 translation) {
		glm::mat4 identity = glm::mat4(1.0f);
		glm::mat4 translated = glm::translate(identity, translation);


		return glm::rotate(translated,
		                   (360 - mCamera.getCameraRotationXZ()) * (3.141592f / 180.0f),
		                   glm::vec3(0.0f, 1.0f, 0.0f));
	}

	bool DungeonGLES2Renderer::isAnimating() {
		return mCamera.isAnimating();
	}

	void DungeonGLES2Renderer::setEyeView(float *eyeView) {
		mCamera.setEyeView( eyeView );
	}

	void DungeonGLES2Renderer::setPerspectiveMatrix(float *perspectiveMatrix) {
		projectionMatrix = glm::make_mat4(perspectiveMatrix);
	}

	void DungeonGLES2Renderer::setAngleXZ(float xz) {
		mCamera.setRotationXZ( xz );
	}

	void DungeonGLES2Renderer::setAngleYZ(float yz) {
		mCamera.setRotationYZ( yz );
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

	void DungeonGLES2Renderer::setMesh(std::shared_ptr<odb::Scene> mesh) {

		auto m = std::begin(mesh->meshObjects);
		auto mEnd = std::end(mesh->meshObjects);

		while (m != mEnd) {
			auto &meshData = m->second->trigBatches[0];

			auto floatData = meshData.getVertexData();
			auto vertexCount = meshData.getVertexCount();
			auto indexData = meshData.getIndices();
			auto indicesCount = meshData.getIndicesCount();

			mVBORegisters[m->first] = submitVBO(floatData,
			                                    vertexCount,
			                                    indexData,
			                                    indicesCount);

			m = std::next(m);
		}
	}
}

#endif
