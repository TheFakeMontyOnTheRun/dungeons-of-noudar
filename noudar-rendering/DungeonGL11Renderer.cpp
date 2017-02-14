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

#include "RenderingJobSnapshotAdapter.h"

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
		odb::Logger::log("bitmaps size as to upload: %d", mBitmaps.size());
		for (auto &bitmap : mBitmaps) {
			odb::Logger::log("index: %d", index);
			mTextures.push_back(std::make_shared<Texture>(uploadTextureData(bitmap), bitmap));
		}

		mTextureRegistry["sky"] = ETextures::Skybox;
		mTextureRegistry["grass"] = ETextures::Grass;
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


		auto geometryBatch = GeometryBatches[vertexVbo];
		auto indicesBatch = IndicesBatches[indexVbo];

		glLoadMatrixf(&(mViewMatrix * transform)[0][0]);

		glBegin(GL_TRIANGLES);

		for (int i = 0; i < indicesBatch.sizeIndices; ++i) {
			unsigned short index = indicesBatch.indices[i];

			float *line = geometryBatch.vertices + (index * kGeometryLineStride);
			glTexCoord2f(line[3], line[4]);
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

		mVBORegisters["sky"] = submitVBO((float *) skyVertices, 4, (unsigned short *) skyIndices, 6);

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
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
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
				angleInRadiansYZ, glm::vec3(1.0f, 0.0f, 0.0f)) * pos_front4);

		mCameraDirection = glm::vec3(result.x, result.y, result.z);

		pos_front = mCameraDirection;

		mViewMatrix = glm::lookAt(
				pos,
				pos_front + pos,
				glm::vec3(0.0f, 1.0, 0.0f)) * eyeMatrix;
	}

	void
	DungeonGLES2Renderer::produceRenderingBatches(const NoudarDungeonSnapshot &snapshot) {

		glm::vec3 pos;
		const auto &billboardVBO = mVBORegisters["billboard"];

		batches.clear();

		mSnapshotAdapter.readSnapshot(snapshot, batches, mTileProperties, mVBORegisters, mTextureRegistry);

		for (int z = 0; z < Knights::kMapSize; ++z) {
			for (int x = 0; x < Knights::kMapSize; ++x) {

				if (snapshot.mVisibilityMap[z][x] == EVisibility::kInvisible) {
					continue;
				}

				auto actor = snapshot.snapshot[z][x];
				int splatFrame = snapshot.splat[z][x];
				Shade shade = (0.25f * std::min(255, snapshot.mLightMap[z][x]) / 255.0f) + 0.75f;

				auto tile = snapshot.map[z][x];

				if (x == static_cast<int>(snapshot.mCursorPosition.x) &&
				    z == static_cast<int>(snapshot.mCursorPosition.y)) {
					shade = 1.5f;
				}

				//characters
				if (actor != EActorsSnapshotElement::kNothing) {

					int id = snapshot.ids[z][x];
					float fx, fz;

					fx = x;
					fz = z;

					float step = 0.0f;
					float curve = 0.0f;

					if (id != 0 && snapshot.movingCharacters.count(id) > 0) {

						auto animation = snapshot.movingCharacters.at(id);
						auto pos = mSnapshotAdapter.easingAnimationCurveStep(std::get<0>(animation),
						                                                     std::get<1>(animation),
						                                                     std::get<2>(animation),
						                                                     snapshot.mTimestamp);

						fx = pos.x;
						fz = pos.y;
					}

					pos = glm::vec3(fx * 2.0f, -4.0f, fz * 2.0f);


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
					pos = glm::vec3(x * 2, -4.0f, z * 2);
					batches[static_cast<ETextures >(splatFrame +
					                                ETextures::Splat0)].emplace_back(
							std::get<0>(billboardVBO),
							std::get<1>(billboardVBO),
							std::get<2>(billboardVBO),
							getBillboardTransform(pos), shade, true);
				}
			}
		}


		for (int z = 0; z < Knights::kMapSize; ++z) {
			for (int x = 0; x < Knights::kMapSize; ++x) {

				int id = snapshot.ids[z][x];

				float fx, fz;

				fx = x;
				fz = z;

				float step = 0.0f;
				float curve = 0.0f;

				if (id != 0 && snapshot.movingCharacters.count(id) > 0) {

					auto animation = snapshot.movingCharacters.at(id);
					auto pos = mSnapshotAdapter.easingAnimationCurveStep(std::get<0>(animation), std::get<1>(animation),
					                                                     std::get<2>(animation), snapshot.mTimestamp);
					fx = pos.x;
					fz = pos.y;
				}

				pos = glm::vec3(fx * 2.0f, -4.0f, fz * 2.0f);

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
	}

	void DungeonGLES2Renderer::invalidateCachedBatches() {
		batches.clear();
	}

	void DungeonGLES2Renderer::render(const NoudarDungeonSnapshot &snapshot) {

		if (mBitmaps.empty()) {
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

			auto textureId = mTextures[batch.first]->mTextureId;
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

	bool DungeonGLES2Renderer::isAnimating() {
		return mRotationTarget != mCameraRotation;
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

	void DungeonGLES2Renderer::resetCamera() {
		mAngleXZ = 0;
		mAngleYZ = 0;
		mCameraRotation = 0;
		mRotationTarget = 0;
	}

	void DungeonGLES2Renderer::setTileProperties(CTilePropertyMap map) {

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
