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
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#endif

#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <map>
#include <array>

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
#include "DungeonGLES2Renderer.h"


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

	const float DungeonGLES2Renderer::XZWallVertices[]{
			-1.0f, 0.0f, -1.0f, 0.0f, .0f,
			1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
			-1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	};

	const float DungeonGLES2Renderer::XYWallVertices[]{
			-1.0f, 1.0f, -1.0f, 0.0f, .0f,
			1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 0.0f, -1.0f, 1.0f, 1.0f,
			-1.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	};

	const float DungeonGLES2Renderer::YZWallVertices[]{
			-1.0f, 1.0f, -1.0f, 0.0f, .0f,
			-1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
			-1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
			-1.0f, 0.0f, -1.0f, 0.0f, 1.0f,
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

	const unsigned short DungeonGLES2Renderer::XZWallIndices[]{
			0, 1, 2,
			0, 2, 3
	};


	const unsigned short DungeonGLES2Renderer::XYWallIndices[]{
			0, 1, 2,
			0, 2, 3
	};

	const unsigned short DungeonGLES2Renderer::YZWallIndices[]{
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
		odb::Logger::log("Destroying the renderer");

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
			odb::Logger::log("Could not create program.");
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

		mCameraDirection = glm::rotate(
				glm::rotate(glm::mat4(1.0f), angleInRadiansXZ, glm::vec3(0.0f, 1.0f, 0.0f)),
				angleInRadiansYZ, glm::vec3(1.0f, 0.0f, 0.0f)) * pos_front4;
		pos_front = mCameraDirection;

		viewMatrix = glm::lookAt(
				pos,
				pos_front + pos,
				glm::vec3(0.0f, 1.0, 0.0f)) * eyeMatrix;


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

	void DungeonGLES2Renderer::drawGeometry(const int vertexVbo, const int indexVbo,
	                                        int vertexCount,
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


		glDeleteBuffers(1, &vboXZWallVertexDataIndex);
		glDeleteBuffers(1, &vboXZWallVertexIndicesIndex);

		glDeleteBuffers(1, &vboXYWallVertexDataIndex);
		glDeleteBuffers(1, &vboXYWallVertexIndicesIndex);

		glDeleteBuffers(1, &vboYZWallVertexDataIndex);
		glDeleteBuffers(1, &vboYZWallVertexIndicesIndex);

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
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, cornerLeftNearVertices,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboCornerLeftNearVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboCornerLeftNearVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), cornerLeftNearIndices,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);





		//XY Wall
		glGenBuffers(1, &vboXYWallVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboXYWallVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, XYWallVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboXYWallVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboXYWallVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), XYWallIndices,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//XZ Wall
		glGenBuffers(1, &vboXZWallVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboXZWallVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, XZWallVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboXZWallVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboXZWallVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), XZWallIndices,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//YZ Wall
		glGenBuffers(1, &vboYZWallVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboYZWallVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, YZWallVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboYZWallVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboYZWallVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), YZWallIndices,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

	void DungeonGLES2Renderer::render(IntGameMap map, IntGameMap actors, IntGameMap splats,
	                                  LightMap lightMap, IntField ids,
	                                  AnimationList movingCharacters,
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

		mElementMap[ '.' ] = ETextures::Grass;
		mElementMap[ '_' ] = ETextures::Floor;

		mElementMap[ '(' ] = ETextures::GrassStoneFar;
		mElementMap[ '{' ] = ETextures::GrassStoneNear;

		mElementMap[ ')' ] = ETextures::StoneGrassNear;
		mElementMap[ '}' ] = ETextures::StoneGrassFar;

		mElementMap[ '=' ] = ETextures::Floor;
		mElementMap[ '1' ] = ETextures::Bricks;
		mElementMap[ '#' ] = ETextures::Bars;
		mElementMap[ '~' ] = ETextures::Arch;
		mElementMap[ 'Y' ] = ETextures::BricksCandles;
		mElementMap[ 'X' ] = ETextures::BricksBlood;

		mElementMap[ '%' ] = ETextures::Bricks;
		mElementMap[ '|' ] = ETextures::Bricks;
		mElementMap[ '\\' ] = ETextures::Bricks;
		mElementMap[ '/' ] = ETextures::Bricks;
		mElementMap[ '>' ] = ETextures::Bricks;
		mElementMap[ '<' ] = ETextures::Bricks;
		mElementMap[ 'Z' ] = ETextures::Bricks;
		mElementMap[ 'S' ] = ETextures::Bricks;

		mElementMap[ '9' ] = ETextures::Exit;
		mElementMap[ '*' ] = ETextures::Begin;


		mElementMap[ '@' ] = ETextures::Cuco0;

		mElementMap[ ' ' ] = ETextures::Skybox;

		mElementMap[ '^' ] = ETextures::Crusader;

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
		glm::mat4 transform;
		Shade shade;

		for (auto &batch : batches) {

			glBindTexture(GL_TEXTURE_2D, mTextures[batch.first]->mTextureId);

			for (auto &element : batch.second) {
				transform = std::get<0>(element);
				shade = std::get<2>(element);
				EGeometryType type = std::get<1>(element);

				glUniform4f(uMod, shade, shade, shade, 1.0f);

				if (EGeometryType::kFloor == type) {
					drawGeometry(vboFloorVertexDataIndex,
					             vboFloorVertexIndicesIndex,
					             6, transform
					);
				} else if (EGeometryType::kWalls == type) {
					drawGeometry(vboCubeVertexDataIndex,
					             vboCubeVertexIndicesIndex,
					             24,
					             transform
					);
				} else if (EGeometryType::kBillboard == type) {

					drawGeometry(vboBillboardVertexDataIndex,
					             vboBillboardVertexIndicesIndex,
					             6,
					             transform
					);

				} else if (EGeometryType::kLeftNearCorner == type) {

					drawGeometry(vboCornerLeftNearVertexDataIndex,
					             vboCornerLeftNearVertexIndicesIndex,
					             6,
					             transform
					);

				} else if (EGeometryType::kLeftFarCorner == type) {

					drawGeometry(vboCornerLeftFarVertexDataIndex,
					             vboCornerLeftFarVertexIndicesIndex,
					             6,
					             transform
					);

				} else if (EGeometryType::kXZWall == type) {

					drawGeometry(vboXZWallVertexDataIndex,
					             vboXZWallVertexIndicesIndex,
					             6,
					             transform
					);

				} else if (EGeometryType::kXYWall == type) {

					drawGeometry(vboXYWallVertexDataIndex,
					             vboXYWallVertexIndicesIndex,
					             6,
					             transform
					);

				} else if (EGeometryType::kYZWall == type) {

					drawGeometry(vboYZWallVertexDataIndex,
					             vboYZWallVertexIndicesIndex,
					             6,
					             transform
					);

				} else if (EGeometryType::kSkyBox == type) {
					drawGeometry(vboSkyVertexDataIndex,
					             vboSkyVertexIndicesIndex,
					             6,
					             getSkyTransform(animationTime)
					);

					drawGeometry(vboSkyVertexDataIndex,
					             vboSkyVertexIndicesIndex,
					             6,
					             getSkyTransform(animationTime + kSkyTextureLength * 1000)
					);

				}
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
		Shade shade;

		batches.clear();

		if (mFloorNumber == 0) {
			batches[ETextures::Skybox].emplace_back( getSkyTransform(0),
			                                        EGeometryType::kSkyBox,
			                                        1.0f);
		}


		for (int z = 0; z < 20; ++z) {
			for (int x = 0; x < 20; ++x) {

				int tile = map[19 - z][x];
				int actor = actors[19 - z][x];
				int splatFrame = splats[19 - z][x];

				Shade placeShade = (0.25f * std::min(255, lightMap[19 - z][x]) / 255.0f) + 0.75f;
				Shade shade = placeShade;

				if ( x == mCursorPosition.x && ( 19 - z ) == static_cast<int>(mCursorPosition.y) ) {
					shade = 1.5f

							;
				}

				if ( actor == '@' ) {
				//	chosenTexture = ETextures::Shadow;
				} else {
				}

				if (tile == '.' || tile == '_' || tile == '=' || tile == '-' || tile == ')' || tile == '(' || tile == '}' || tile == '{' ) {
					chosenTexture = mElementMap[ tile ];
				} else if (tile == '#' || tile == '~' || tile == '|'  || tile == '%' ) {
					chosenTexture = mElementMap[ '_' ];
				} else if (tile == '\\' ) {
					chosenTexture = mElementMap[ '(' ];
				} else if (tile == '/' ) {
					chosenTexture = mElementMap[ ')' ];
				} else if (tile == '>') {
					chosenTexture = mElementMap[ '{' ];
				} else if (tile == '<') {
					chosenTexture = mElementMap[ '}' ];

				} else {
					chosenTexture = mElementMap[ '.' ];
				}

				if (tile == '=' || tile == '-' ) {
					shade -= 0.25f;
				}


				pos = glm::vec3(-10 + (x * 2), -5.0f, -10 + (-z * 2));
				batches[chosenTexture].emplace_back(getFloorTransform(pos), EGeometryType::kFloor, shade);

				shade = placeShade;

				//walls
				if (tile == '\\' || tile == '<' || tile == '|' || tile == 'S' ) {
					pos = glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(mElementMap[ tile ])].emplace_back(getCornerLeftFarTransform(pos),
					                                                    EGeometryType::kLeftFarCorner,
					                                                    shade);

					pos = glm::vec3(-10 + (x * 2), -2.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(mElementMap[ tile ])].emplace_back(getCornerLeftFarTransform(pos),
					                                                    EGeometryType::kLeftFarCorner,
					                                                    shade);

					pos = glm::vec3(-10 + (x * 2), 0.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(mElementMap[ tile ])].emplace_back(getCornerLeftFarTransform(pos),
					                                                                   EGeometryType::kLeftFarCorner,
					                                                                   shade);


				} else if (tile == '/'  || tile == '>' || tile == '%' || tile == 'Z') {
					pos = glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(mElementMap[ tile ])].emplace_back(getCornerLeftNearTransform(pos),
					                                                    EGeometryType::kLeftNearCorner,
					                                                    shade);


					pos = glm::vec3(-10 + (x * 2), -2.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(mElementMap[ tile ])].emplace_back(getCornerLeftNearTransform(pos),
					                                                    EGeometryType::kLeftNearCorner,
					                                                    shade);

					pos = glm::vec3(-10 + (x * 2), 0.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(mElementMap[ tile ])].emplace_back(getCornerLeftNearTransform(pos),
					                                                                   EGeometryType::kLeftNearCorner,
					                                                                   shade);

				} else if (tile != '.' && tile != '_' &&  tile != '=' && tile != '-' && tile != ')' && tile != '(' && tile != '}' && tile != '{') {

					pos = glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(mElementMap[ tile ])].emplace_back(getCubeTransform(pos), EGeometryType::kWalls,
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
						pos = glm::vec3(-10 + (x * 2), -2.0f, -10 + (-z * 2));
						batches[(tile == 'b') ? ETextures::CeilingEnd
						                                   : mElementMap[ '1' ]].emplace_back(getCubeTransform(pos),
						                                                                     EGeometryType::kWalls,
						                                                                     shade);

						pos = glm::vec3(-10 + (x * 2), 0.0f, -10 + (-z * 2));
						batches[(tile == 'b') ? ETextures::CeilingEnd
						                      : mElementMap[ '1' ]].emplace_back(getCubeTransform(pos),
						                                                         EGeometryType::kWalls,
						                                                         shade);

					}


				} else {
					if (tile == '=' || tile == '-') {



						pos = glm::vec3(-10 + (x * 2), 0.0f, -10 + (-z * 2));
						batches[Floor].emplace_back(getCubeTransform(pos), EGeometryType::kWalls,
						                                                                   shade);
						shade -= 0.375f;
						pos = glm::vec3(-10 + (x * 2), -1.0f, -10 + (-z * 2));
						batches[Floor].emplace_back(getFloorTransform(pos), EGeometryType::kFloor, shade);

						shade = placeShade;

					}
				}

				//characters
				if (actor != 0) {

					int id = ids[19 - z][x];
					float fx, fz;

					fx = x;
					fz = z;

					if (id != 0 && movingCharacters.count(id) > 0) {
						auto animation = movingCharacters[id];
						float step = (((float) ((animationTime - std::get<2>(animation)))) /
						              ((float) kAnimationLength));

						if (!mLongPressing) {
							if (step < 0.5f) {
								step = ((2.0f * step) * (2.0f * step)) / 2.0f;
							} else {
								step = (sqrt((step * 2.0f) - 1.0f) / 2.0f) + 0.5f;
							}
						}

						auto prevPosition = std::get<0>(animation);
						auto destPosition = std::get<1>(animation);

						fx = (step * (destPosition.x - prevPosition.x)) + prevPosition.x;
						fz = 19.0f -
						     ((step * (destPosition.y - (prevPosition.y))) + prevPosition.y);
					}

					pos = glm::vec3(-10 + (fx * 2), -4.0f, -10 + (-fz * 2));


					if (actor == '@' ) {
						batches[static_cast<ETextures >(mElementMap[ actor ])].emplace_back(getBillboardTransform(pos),
						                                                     EGeometryType::kBillboard,
						                                                     shade);
					} else {
						mCurrentCharacterPosition = pos;
					}
				}

				if (splatFrame > -1) {
					pos = glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2));
					batches[static_cast<ETextures >(splatFrame +
					                                ETextures::Splat0)].emplace_back(getBillboardTransform(pos),
					                                                                 EGeometryType::kBillboard, shade);
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

	void DungeonGLES2Renderer::setFloorNumber(long floor) {
		mFloorNumber = floor;
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

	void DungeonGLES2Renderer::drawTrigBatch(odb::TrigBatch &batch) {

		glm::vec3 pos = glm::vec3(-10 + (2 * 2), -4.0f, -10 + (-5 * 2));
		glm::mat4 trans = getCubeTransform(pos);

		glBindTexture(GL_TEXTURE_2D, mTextures[ETextures::Floor]->mTextureId);

		glUniformMatrix4fv(modelMatrixAttributePosition, 1, false, &trans[0][0]);
		checkGlError("before drawing");

		glEnableVertexAttribArray(vertexAttributePosition);
		glEnableVertexAttribArray(textureCoordinatesAttributePosition);

		batch.draw(vertexAttributePosition, textureCoordinatesAttributePosition);

		glDisableVertexAttribArray(vertexAttributePosition);
		glDisableVertexAttribArray(textureCoordinatesAttributePosition);
	}

	glm::vec3 DungeonGLES2Renderer::transformToMapPosition(const glm::vec3 &pos) {
		return glm::vec3(-10 + (pos.x * 2), -5.0f + pos.y, -10 + (-pos.z * 2));
	}

	glm::mat4 DungeonGLES2Renderer::getXYWallTransform(glm::vec3 translation, float y0, float y1) {
		glm::mat4 translated = glm::translate(glm::mat4(1.0f), translation);
		glm::mat4 scaled = glm::scale( translated, glm::vec3( 1.0f, (y1-y0)/4.0f, 1.0f ) );


		return scaled;

	}

	glm::mat4 DungeonGLES2Renderer::getYZWallTransform(glm::vec3 translation, float y0, float y1) {
		glm::mat4 translated = glm::translate(glm::mat4(1.0f), translation);
		glm::mat4 scaled = glm::scale( translated, glm::vec3( 1.0f, (y1-y0)/4.0f, 1.0f ) );


		return scaled;
	}

	void DungeonGLES2Renderer::setCursorPosition(int x, int y) {
		mCursorPosition = {x , y };
	}
}
