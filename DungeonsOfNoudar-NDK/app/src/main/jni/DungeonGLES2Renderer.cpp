//
// Created by monty on 23/11/15.
//


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLES2/gl2.h>
#include <android/log.h>
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <map>
#include <array>

#include "gles2-renderer/NativeBitmap.h"
#include "gles2-renderer/Texture.h"
#include "gles2-renderer/Material.h"
#include "gles2-renderer/Trig.h"
#include "gles2-renderer/TrigBatch.h"
#include "gles2-renderer/MeshObject.h"
#include "gles2-renderer/MaterialList.h"
#include "gles2-renderer/Scene.h"
#include "DungeonGLES2Renderer.h"
#include "NdkGlue.h"

#include <glm/gtc/type_ptr.hpp>

namespace odb {
	const static bool kShouldDestroyThingsManually = false;

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
			-kSkyTextureLength -20.0f, 10.0f, -200.0f, 0.0f, .0f,
			-20.0f, 10.0f, -200.0f, 10.0f, 0.0f,
			-20.0f, 10.0f, 200.0f, 10.0f, 10.0f,
			-kSkyTextureLength-20.0f, 10.0f, 200.0f, 0.0f, 10.0f,
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

		return textureId;
	}


	extern void printGLString(const char *name, GLenum s) {
		const char *v = (const char *) glGetString(s);
		LOGI("GL %s = %s\n", name, v);
	}

	extern void checkGlError(const char *op) {
		for (GLint error = glGetError(); error; error = glGetError()) {
			LOGI("after %s() glError (0x%x)\n", op, error);
		}
	}

	GLuint DungeonGLES2Renderer::loadShader(GLenum shaderType, const char *pSource) {
		auto shader = glCreateShader(shaderType);
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
						LOGE("Could not compile shader %d:\n%s\n", shaderType, buf);
						free(buf);
					}
					glDeleteShader(shader);
					shader = 0;
				}
			}
		}
		return shader;
	}

	GLuint DungeonGLES2Renderer::createProgram(const char *pVertexSource, const char *pFragmentSource) {
		auto vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
		if (!vertexShader) {
			return 0;
		}

		auto pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
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
						LOGE("Could not link program:\n%s\n", buf);
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
		printGLString("Version", GL_VERSION);
		printGLString("Vendor", GL_VENDOR);
		printGLString("Renderer", GL_RENDERER);
		printGLString("Extensions", GL_EXTENSIONS);
	}

	DungeonGLES2Renderer::DungeonGLES2Renderer() {
//start off as identity - late we will init it with proper values.
		cubeTransformMatrix = glm::mat4(1.0f);
		projectionMatrix = glm::mat4(1.0f);
		vertexAttributePosition = 0;
		modelMatrixAttributePosition = 0;
		projectionMatrixAttributePosition = 0;
		gProgram = 0;
	}

	DungeonGLES2Renderer::~DungeonGLES2Renderer() {
		LOGI("Destroying the renderer");

		if (kShouldDestroyThingsManually) {
			for (auto &texture : mTextures) {
				glDeleteTextures(1, &(texture->mTextureId));
			}
			deleteVBOs();
		}

	}

	bool DungeonGLES2Renderer::init(float w, float h, const std::string &vertexShader,
	                         const std::string &fragmentShader) {

		printVerboseDriverInformation();

		gProgram = createProgram(vertexShader.c_str(), fragmentShader.c_str());

		if (!gProgram) {
			LOGE("Could not create program.");
			return false;
		}

		fetchShaderLocations();

		glViewport(0, 0, w, h);
		checkGlError("glViewport");

		projectionMatrix = glm::perspective(45.0f, w / h, 0.1f, 100.0f);

		createVBOs();

		for (auto &bitmap : mBitmaps) {
			mTextures.push_back(std::make_shared<Texture>(uploadTextureData(bitmap), bitmap));
		}

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glFrontFace(GL_CW);
		glDepthMask(true);
		startFadingIn();
		return true;
	}

	glm::mat4 DungeonGLES2Renderer::getCubeTransform(glm::vec3 translation) {
		glm::mat4 identity = glm::mat4(1.0f);
		glm::mat4 translated = glm::translate(identity, translation);

		return translated;
	}

	void DungeonGLES2Renderer::resetTransformMatrices() {
		glm::mat4 viewMatrix;

		switch (mCameraMode ) {
			case kFirstPerson: {
				glm::vec3 pos = mCurrentCharacterPosition;
				glm::vec4 pos_front4 = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
				glm::vec3 pos_front;
                glm::mat4 eyeMatrixOriginal = mEyeView != nullptr? glm::make_mat4( mEyeView) : glm::mat4(1.0f);
                glm::mat4 eyeMatrix = glm::mat4(1.0f);

                eyeMatrix[ 3 ][ 0 ] = eyeMatrixOriginal[ 3 ][ 0 ];
                eyeMatrix[ 3 ][ 1 ] = eyeMatrixOriginal[ 3 ][ 1 ];
                eyeMatrix[ 3 ][ 2 ] = eyeMatrixOriginal[ 3 ][ 2 ];

                float angleInRadiansYZ = mAngleYZ * (3.14159f / 180.0f);
                float angleInRadiansXZ = (mAngleXZ - mCameraRotation) * (3.14159f / 180.0f);

                mCameraDirection = glm::rotate( glm::rotate( glm::mat4(1.0f), angleInRadiansXZ, glm::vec3( 0.0f, 1.0f, 0.0f) ), angleInRadiansYZ, glm::vec3( 1.0f, 0.0f, 0.0f ) ) * pos_front4;
                pos_front = mCameraDirection;

				viewMatrix = glm::lookAt(
						pos,
						pos_front + pos,
						glm::vec3(0.0f, 1.0, 0.0f)) * eyeMatrix;
            }
				break;
		}

		glUniformMatrix4fv(uView, 1, false, &viewMatrix[0][0]);
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

	void DungeonGLES2Renderer::drawGeometry(const int vertexVbo, const int indexVbo, int vertexCount,
	                                 const glm::mat4 &transform) {

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
		glDeleteBuffers(1, &vboCubeVertexDataIndex);
		glDeleteBuffers(1, &vboCubeVertexIndicesIndex);

		glDeleteBuffers(1, &vboFloorVertexDataIndex);
		glDeleteBuffers(1, &vboFloorVertexIndicesIndex);

		glDeleteBuffers(1, &vboBillboardVertexDataIndex);
		glDeleteBuffers(1, &vboBillboardVertexIndicesIndex);

		glDeleteBuffers(1, &vboSkyVertexIndicesIndex);
		glDeleteBuffers(1, &vboSkyVertexDataIndex);

		glDeleteBuffers(1, &vboCornerLeftFarVertexIndicesIndex);
		glDeleteBuffers(1, &vboCornerLeftFarVertexDataIndex);

		glDeleteBuffers(1, &vboCornerLeftNearVertexDataIndex);
		glDeleteBuffers(1, &vboCornerLeftNearVertexIndicesIndex);
	}

	void DungeonGLES2Renderer::createVBOs() {
		//walls
		glGenBuffers(1, &vboCubeVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboCubeVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float) * 5, cubeVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboCubeVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboCubeVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLushort), cubeIndices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//characters
		glGenBuffers(1, &vboBillboardVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboBillboardVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, billboardVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glGenBuffers(1, &vboBillboardVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboBillboardVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), billboardIndices,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//floor
		glGenBuffers(1, &vboFloorVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboFloorVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, floorVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboFloorVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFloorVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), floorIndices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


		//sky
		glGenBuffers(1, &vboSkyVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboSkyVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, skyVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboSkyVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSkyVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), skyIndices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


		//corner left far
		glGenBuffers(1, &vboCornerLeftFarVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboCornerLeftFarVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, cornerLeftFarVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboCornerLeftFarVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboCornerLeftFarVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), cornerLeftFarIndices,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//corner left near
		glGenBuffers(1, &vboCornerLeftNearVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboCornerLeftNearVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, cornerLeftNearVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboCornerLeftNearVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboCornerLeftNearVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), cornerLeftNearIndices,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void DungeonGLES2Renderer::clearBuffers() {
		if ( mCameraMode == kFirstPerson ) {
			glClearColor( 0.5f, 0.5f, 0.5f, 1.0f);
		} else {
			glClearColor(mClearColour.r, mClearColour.g, mClearColour.b, 1.0f);
		}
		glClearDepthf(1.0f);
		checkGlError("glClearColor");
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

	void DungeonGLES2Renderer::render(IntGameMap map, IntGameMap actors, IntGameMap splats,
	                           LightMap lightMap, IntField ids, AnimationList movingCharacters,
	                           long animationTime) {
		clearBuffers();
		prepareShaderProgram();
		setPerspective();
		resetTransformMatrices();

		glUniform4fv(fadeUniform, 1, &mFadeColour[0]);
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
	}

	void DungeonGLES2Renderer::shutdown() {
		LOGI("Shutdown!\n");
	}

	void DungeonGLES2Renderer::setCameraPosition(float x, float y) {
		this->mCameraTarget = glm::vec2{x, y};
	}

	void DungeonGLES2Renderer::setCursorAt(float x, float y) {
		this->cursorPosition = glm::vec2{x, y};
	}

	void DungeonGLES2Renderer::toggleCloseUpCamera() {
		mCameraMode = static_cast<ECameraMode>(( static_cast<int>(mCameraMode) + 1 ) % ECameraMode::kTotal);
	}

	void DungeonGLES2Renderer::setClearColour(float r, float g, float b) {
		this->mClearColour = glm::vec3(r, g, b);
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


		if (  mRotationTarget > mCameraRotation  ) {
			mCameraRotation+=5;
		} else if (  mRotationTarget < mCameraRotation  ) {
			mCameraRotation-=5;
		}
	}

	void DungeonGLES2Renderer::consumeRenderingBatches(long animationTime) {
		glm::vec3 pos;
		Shade shade;

		for (auto &batch : batches) {

			glBindTexture(GL_TEXTURE_2D, mTextures[batch.first]->mTextureId);

			for (auto &element : batch.second) {
				pos = std::get<0>(element);
				shade = std::get<2>(element);
				EGeometryType type = std::get<1>(element);

				glUniform4f(uMod, shade, shade, shade, 1.0f);

				if (EGeometryType::kFloor == type) {
					drawGeometry(vboFloorVertexDataIndex,
					             vboFloorVertexIndicesIndex,
					             6,
					             getFloorTransform(pos)
					);
				} else if (EGeometryType::kWalls == type) {
					drawGeometry(vboCubeVertexDataIndex,
					             vboCubeVertexIndicesIndex,
					             24,
					             getCubeTransform(pos)
					);
				} else if (EGeometryType::kBillboard == type) {

					drawGeometry(vboBillboardVertexDataIndex,
					             vboBillboardVertexIndicesIndex,
					             6,
					             getBillboardTransform(pos)
					);

				} else if (EGeometryType::kLeftNearCorner == type) {

					drawGeometry(vboCornerLeftNearVertexDataIndex,
					             vboCornerLeftNearVertexIndicesIndex,
					             6,
					             getCornerLeftNearTransform(pos)
					);

				} else if (EGeometryType::kLeftFarCorner == type) {

					drawGeometry(vboCornerLeftFarVertexDataIndex,
					             vboCornerLeftFarVertexIndicesIndex,
					             6,
					             getCornerLeftFarTransform(pos)
					);


				} else if (EGeometryType::kSkyBox == type) {
					drawGeometry(vboSkyVertexDataIndex,
					             vboSkyVertexIndicesIndex,
					             6,
					             getSkyTransform( animationTime)
					);

					drawGeometry(vboSkyVertexDataIndex,
					             vboSkyVertexIndicesIndex,
					             6,
					             getSkyTransform( animationTime + kSkyTextureLength * 1000)
					);

				}
			}
		}
	}

	void DungeonGLES2Renderer::produceRenderingBatches(IntGameMap map, IntGameMap actors, IntGameMap splats,
	                                            LightMap lightMap, IntField ids,
	                                            AnimationList movingCharacters, long animationTime) {

		ETextures chosenTexture;
		glm::vec3 pos;
		Shade shade;

		batches.clear();

		if ( mCameraMode == ECameraMode::kFirstPerson && mFloorNumber == 0 ) {
			batches[ETextures::Skybox].emplace_back(glm::vec3(0.0f, 0.0f, 0.0f), EGeometryType::kSkyBox,
			                                        1.0f);
		}


		for (int z = 0; z < 20; ++z) {
			for (int x = 0; x < 20; ++x) {

				int tile = map[19 - z][x];
				int actor = actors[19 - z][x];
				int splatFrame = splats[19 - z][x];
				bool isCursorPoint = ((x == static_cast<int>(this->cursorPosition.x)) &&
				                      ((19 - z) == static_cast<int>(this->cursorPosition.y)));

				Shade shade = (0.25f * std::min(255, lightMap[19 - z][x]) / 255.0f) + 0.75f;

				if (isCursorPoint) {
					if ( mCameraMode == ECameraMode::kFirstPerson ) {
						chosenTexture = ETextures::Grass;
					} else {
						chosenTexture = ETextures::CursorGood0;
					}
				} else {
					if (ETextures::Boss0 <= actor && actor < ETextures::Bull0) {
						if ( mCameraMode == ECameraMode::kFirstPerson ) {
							chosenTexture = ETextures::Shadow;
						} else {
							chosenTexture = ETextures::CursorBad0;
						}

					} else if (ETextures::Bull0 <= actor && actor < ETextures::Shadow) {
						chosenTexture = ETextures::Shadow;
					} else {
						chosenTexture = ETextures::Grass;
					};
				};

				pos = glm::vec3(-10 + (x * 2), -5.0f, -10 + (-z * 2));
				batches[chosenTexture].emplace_back(pos, EGeometryType::kFloor, shade);


				//walls
				if (tile == ETextures::CornerLeftFar) {
					pos = glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(tile)].emplace_back(pos, EGeometryType::kLeftFarCorner,
					                                                    shade);

					if ( mCameraMode == ECameraMode::kFirstPerson ) {
						pos = glm::vec3(-10 + (x * 2), -2.0f, -10 + (-z * 2));
						batches[static_cast<ETextures >(tile)].emplace_back(pos, EGeometryType::kLeftFarCorner,
						                                                    shade);
					}
				}

				if (tile == ETextures::CornerLeftNear) {
					pos = glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(tile)].emplace_back(pos, EGeometryType::kLeftNearCorner,
					                                                    shade);

					if ( mCameraMode == ECameraMode::kFirstPerson ) {
						pos = glm::vec3(-10 + (x * 2), -2.0f, -10 + (-z * 2));
						batches[static_cast<ETextures >(tile)].emplace_back(pos, EGeometryType::kLeftNearCorner,
						                                                    shade);
					}
				}

				if (ETextures::Bricks <= tile && tile <= ETextures::BricksCandles) {

					pos = glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(tile)].emplace_back(pos, EGeometryType::kWalls,
					                                                    shade);

					//top of walls cube
					ETextures textureForCeling = ETextures::Ceiling;

					if (tile == ETextures::Begin) {
						textureForCeling = ETextures::CeilingBegin;
					} else if (tile == ETextures::Exit) {
						textureForCeling = ETextures::CeilingEnd;
					} else if (tile == ETextures::Arch) {
						textureForCeling = ETextures::CeilingDoor;
					} else if (tile == ETextures::Bars) {
						textureForCeling = ETextures::CeilingBars;
					} else {
						textureForCeling = ETextures::Ceiling;
					}

					if ( mCameraMode != ECameraMode::kFirstPerson || textureForCeling != ETextures::Ceiling ) {
						pos = glm::vec3(-10 + (x * 2), -3.0f, -10 + (-z * 2));
						batches[textureForCeling].emplace_back(pos, EGeometryType::kFloor, shade);
					}

					if ( mCameraMode == ECameraMode::kFirstPerson && (tile != ETextures::Exit) ) {
						pos = glm::vec3(-10 + (x * 2), -2.0f, -10 + (-z * 2));
						batches[ (tile == ETextures::Begin) ? ETextures::CeilingEnd : ETextures::Bricks ].emplace_back(pos, EGeometryType::kWalls,
						                                        shade);
					}
				} else {
					if ( mCameraMode == ECameraMode::kFirstPerson && mFloorNumber > 0 ) {
						pos = glm::vec3(-10 + (x * 2), -1.0f, -10 + (-z * 2));
						batches[Grass].emplace_back(pos, EGeometryType::kFloor, shade);
					}
				}

				//characters
				if (ETextures::Boss0 <= actor && actor < ETextures::Shadow) {

					int id = ids[19 - z][x];
					float fx, fz;

					fx = x;
					fz = z;

					if (id != 0 && movingCharacters.count(id) > 0) {
						auto animation = movingCharacters[id];
						float step = (((float) ((animationTime - std::get<2>(animation)))) /
						              ((float) kAnimationLength));

						if( !mLongPressing ) {
							if ( step < 0.5f ) {
								step = ((2.0f*step)*(2.0f*step))/2.0f;
							} else {
								step = (sqrt( (step* 2.0f) - 1.0f )  / 2.0f) + 0.5f;
							}
						}

						auto prevPosition = std::get<0>(animation);
						auto destPosition = std::get<1>(animation);

						fx = (step * (destPosition.x - prevPosition.x)) + prevPosition.x;
						fz = 19.0f -
						     ((step * (destPosition.y - (prevPosition.y))) + prevPosition.y);
					}

					pos = glm::vec3(-10 + (fx * 2), -4.0f, -10 + (-fz * 2));

					if (mCameraMode == ECameraMode::kFirstPerson) {

						if ( !isCursorPoint) {
							batches[static_cast<ETextures >(actor)].emplace_back(pos,
							                                                     EGeometryType::kBillboard,
							                                                     shade);
						} else {
							mCurrentCharacterPosition = pos;
						}
					} else {
						batches[static_cast<ETextures >(actor)].emplace_back(pos,
						                                                     EGeometryType::kBillboard,
						                                                     shade);
					}
				}


				if ( mCameraMode != ECameraMode::kFirstPerson || !isCursorPoint ) {
					if (splatFrame > -1) {
						pos = glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2));
						batches[static_cast<ETextures >(splatFrame +
						                                ETextures::Splat0)].emplace_back(pos,
						                                                                 EGeometryType::kBillboard,
						                                                                 shade);
					}
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

		if ( mCameraMode == ECameraMode::kFirstPerson ) {
			return glm::rotate( translated, (360 - (mCameraRotation) + mAngleXZ ) * (3.141592f / 180.0f), glm::vec3(0.0f, 1.0f, 0.0f) );
		} else {
			return translated;
		}
	}

	glm::mat4 DungeonGLES2Renderer::getFloorTransform(glm::vec3 translation) {
		glm::mat4 identity = glm::mat4(1.0f);
		glm::mat4 translated = glm::translate(identity, translation);

		return translated;
	}

	bool DungeonGLES2Renderer::isAnimating() {
		return mRotationTarget != mCameraRotation;
	}

	void DungeonGLES2Renderer::setFloorNumber(long floor) {
		mFloorNumber = floor;
	}

	glm::mat4 DungeonGLES2Renderer::getSkyTransform(long animationTime ) {
		glm::mat4 identity = glm::mat4(1.0f);

		long offset = animationTime;
		int integerPart = offset % ( (kSkyTextureLength * 2) * 1000);
		float finalOffset = integerPart / 1000.0f;

		return glm::translate( identity, glm::vec3(finalOffset, 0.0f, 0.0f  ));
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
		projectionMatrix = glm::make_mat4( perspectiveMatrix );
	}

    void DungeonGLES2Renderer::setAngleXZ(float xz) {
        mAngleXZ = xz;
    }

    void DungeonGLES2Renderer::setAngleYZ(float yz) {
        mAngleYZ = yz;
    }

	void DungeonGLES2Renderer::drawTrigBatch( odb::TrigBatch &batch ) {

		glm::vec3 pos = glm::vec3(-10 + (2 * 2), -4.0f, -10 + (-5 * 2));
		glm::mat4 trans = getCubeTransform( pos );

		glBindTexture(GL_TEXTURE_2D, mTextures[ETextures::CubeColours]->mTextureId);

		glUniformMatrix4fv(modelMatrixAttributePosition, 1, false, &trans[0][0]);
		checkGlError("before drawing");

		glEnableVertexAttribArray(vertexAttributePosition);
		glEnableVertexAttribArray(textureCoordinatesAttributePosition);

		batch.draw(vertexAttributePosition, textureCoordinatesAttributePosition);

		glDisableVertexAttribArray(vertexAttributePosition);
		glDisableVertexAttribArray(textureCoordinatesAttributePosition);
	}
}