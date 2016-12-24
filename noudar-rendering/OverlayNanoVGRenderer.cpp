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
#include <algorithm>
#include <tuple>
#include <array>
#include <fstream>

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
#include "NoudarDungeonSnapshot.h"


#include "OverlayNanoVGRenderer.h"

#include "nanovg.h"

#define NANOVG_GLES2_IMPLEMENTATION

#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

NVGcontext* mContext;

namespace odb {


    void OverlayNanoVGRenderer::setFrame(float width, float height){
        mWidth = width;
        mHeight = height;
    }

    void OverlayNanoVGRenderer::loadFonts(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoaderDelegate ) {

        mFontData = fileLoaderDelegate->loadBinaryFileFromPath( fileLoaderDelegate->getFilePathPrefix() + "fonts/MedievalSharp.ttf" );
    }


    OverlayNanoVGRenderer::OverlayNanoVGRenderer() {

    }

    void OverlayNanoVGRenderer::render(const odb::NoudarDungeonSnapshot &snapshot) {

        if ( mFontData.size() > 0 ) {
            mContext = 	nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES );
            unsigned char* data = (unsigned char*)malloc(mFontData.size());
            std::copy(mFontData.begin(), mFontData.end(), data);
            nvgCreateFontMem( mContext, "font", data, mFontData.size(),  1 );
            mFontData.clear();
        }

        glViewport(0, 0, mWidth, mHeight);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        nvgBeginFrame(mContext, mWidth, mHeight, mWidth / mHeight);

        nvgBeginPath(mContext);
        nvgFillColor(mContext, nvgRGBA(255, 255, 255, 128));
        nvgStrokeColor(mContext, nvgRGBA(255,0,0, 255));
        nvgRoundedRect(mContext, 5, 5, 200, 100, 2.0f );
        nvgCircle( mContext, 30, 30 , 50 );

        nvgFill(mContext);
        nvgStroke(mContext);


        nvgFontSize(mContext, 18.0f);
        nvgFontFace(mContext, "font");
        nvgFillColor(mContext, nvgRGBA(255,255,255,128));

        nvgTextAlign(mContext,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
        nvgText(mContext, 50, 370, "Monty rendering text in Dungeons of Noudar", nullptr);


        nvgEndFrame(mContext);

        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }

    OverlayNanoVGRenderer::~OverlayNanoVGRenderer() {
        nvgDeleteGLES2(mContext);
    }
}
