//
// Created by monty on 23/11/15.
//

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "nanovg.h"

#ifdef __APPLE__
#if TARGET_IOS

#define NANOVG_GLES2_IMPLEMENTATION

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#else

#define NANOVG_GL2_IMPLEMENTATION
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>
#endif

#else

#if defined(__ANDROID__ ) || defined(__EMSCRIPTEN__) || defined(MESA_GLES2)

#define NANOVG_GLES2_IMPLEMENTATION

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#else

#define NANOVG_GL2_IMPLEMENTATION

#include <GL/gl.h>

#endif
#endif

#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <tuple>
#include <array>
#include <fstream>

#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "IFileLoaderDelegate.h"
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

#include "Vec2i.h"
#include "NativeBitmap.h"
#include "IMapElement.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"

#include "NoudarDungeonSnapshot.h"

#include "OverlayNanoVGRenderer.h"


NVGcontext* mContext;

namespace odb {


    void OverlayNanoVGRenderer::setFrame(float width, float height){
        mWidth = width;
        mHeight = height;
    }

    void OverlayNanoVGRenderer::loadFonts(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoaderDelegate ) {

        mFontData = fileLoaderDelegate->loadBinaryFileFromPath( fileLoaderDelegate->getFilePathPrefix() + "MedievalSharp.ttf" );
    }


    OverlayNanoVGRenderer::OverlayNanoVGRenderer() {

    }

    void OverlayNanoVGRenderer::render(const odb::NoudarDungeonSnapshot &snapshot) {

        if ( mFontData.size() > 0 ) {

#ifdef NANOVG_GLES2_IMPLEMENTATION
            mContext = 	nvgCreateGLES2(NVG_STENCIL_STROKES );
#else
            mContext = 	nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES );
#endif

            if ( mContext ) {
                std::cout << "NVG context is faulty as salty" << std::endl;
            }

            unsigned char* data = (unsigned char*)malloc(mFontData.size());
            std::copy(mFontData.begin(), mFontData.end(), data);
            nvgCreateFontMem( mContext, "font", data, mFontData.size(),  1 );
            mFontData.clear();
        }

        glViewport(0, 0, mWidth, mHeight);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        nvgBeginFrame(mContext, mWidth, mHeight, mWidth / mHeight);

        nvgFontSize(mContext, 18.0f);
        nvgFontFace(mContext, "font");
        nvgFillColor(mContext, nvgRGBA(255,255,255,255));
        nvgStrokeColor(mContext, nvgRGBA(255,0,0, 255));
        nvgTextAlign(mContext,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
        nvgText(mContext, 10, 10, "Map", nullptr);

        for ( int y = 0; y < Knights::kMapSize; ++y ) {
            for ( int x = 0; x < Knights::kMapSize; ++x ) {

                if ( snapshot.ids[ y ][ x ] > 0 ) {
                    nvgRoundedRect(mContext, 10 + (10 * x), 10 + ( 10 * y ), 10, 10, 2.0f );
                }
            }
        }

        nvgFill(mContext);
        nvgStroke(mContext);

        nvgEndFrame(mContext);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    OverlayNanoVGRenderer::~OverlayNanoVGRenderer() {
        nvgDeleteGLES2(mContext);
    }
}
