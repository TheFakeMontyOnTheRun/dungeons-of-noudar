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
#include "CItem.h"
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

#include "GraphicNode.h"
#include "OverlayNanoVGRenderer.h"
#include "AnimationStep.h"
#include "Animation.h"


NVGcontext *mContext;
std::shared_ptr<odb::Animation> currentAnimation = nullptr;
std::map<std::string, std::shared_ptr<odb::Animation>> animations;

long timeUntilNextFrame = 0;
int frame = 0;

std::vector<NVGpaint> paints;

namespace odb {


    void OverlayNanoVGRenderer::setFrame(float width, float height) {
        mWidth = width;
        mHeight = height;
    }

    void OverlayNanoVGRenderer::loadFonts(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoaderDelegate) {

        mFontData = fileLoaderDelegate->loadBinaryFileFromPath("MedievalSharp.ttf");
    }


    OverlayNanoVGRenderer::OverlayNanoVGRenderer(std::vector<std::shared_ptr<odb::NativeBitmap>> bitmaps) {
        for (const auto &bitmap : bitmaps) {
            auto id = bitmap->getId();
            mBitmaps[id] = bitmap;
        }


        animations[ "crossbow-still" ] = std::make_shared<odb::Animation>(
                std::vector<odb::AnimationStep>{
                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow0.png", glm::vec2{0.5125f, 0.85f}, glm::vec2{0.512f, 0.8f}
                                 ),
                         },
                                20000
                        },

                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow0.png", glm::vec2{0.512f, 0.8f}, glm::vec2{0.5125f, 0.85f}
                                 )
                         },
                                20000
                        },

                },
                true,
                ""
        );

        animations[ "crossbow-fire" ] = std::make_shared<odb::Animation>(
                std::vector<odb::AnimationStep>{
                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow0.png", glm::vec2{0.512f, 0.8f}
                                 ),
                         },
                                500
                        },

                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow1.png", glm::vec2{0.512f, 0.8f}, glm::vec2{0.5125f, 0.85f}
                                 ),
                         },
                                500
                        },
                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow1.png", glm::vec2{0.5125f, 0.85f}
                                 )
                         },
                                500
                        },

                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow1.png", glm::vec2{0.5125f, 0.85f}, glm::vec2( 0.4f, 1.0f)
                                 )
                         },
                                500
                        },

                        {{},
                                200
                        },


                },
                false,
                "crossbow-reload"
        );

        animations[ "crossbow-reload" ] = std::make_shared<odb::Animation>(
                //arco-mão-esquerda-diff: 0.1, 0.65
                std::vector<odb::AnimationStep>{
                        {{
                                 std::make_shared<odb::GraphicNode>( "bow2.png", glm::vec2(0.15f, 1.0f),  glm::vec2(0.15f, 0.2f) ),
                                 std::make_shared<odb::GraphicNode>("hand1.png", glm::vec2(0.25f, 1.65f),  glm::vec2(0.25f, 0.85f) )
                         },
                                500
                        },
                        {{
                                 std::make_shared<odb::GraphicNode>( "bow2.png", glm::vec2(0.15f, 0.2f) ),
                                 std::make_shared<odb::GraphicNode>("hand1.png", glm::vec2(0.25f, 0.85f) )
                         },
                                500
                        },

                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow2.png", glm::vec2{0.2f, 0.15f}
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                         "hand1.png", glm::vec2{0.3f, 0.8f}
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                         "hand0.png", glm::vec2(0.55f, 1.0f) , glm::vec2{0.55f, 0.65f}
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                 "dart0.png", glm::vec2(0.55f, 1.0f), glm::vec2{0.55f, 0.65f}
                                 )

                         },
                                500
                        },
                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow2.png", glm::vec2{0.2f, 0.15f}
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                         "hand1.png", glm::vec2{0.3f, 0.8f}
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                         "hand0.png", glm::vec2{0.45f, 0.65f}
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                         "dart0.png", glm::vec2{0.45f, 0.65f}
                                 )

                         },
                                2000
                        },
                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow2.png", glm::vec2{0.1f, 0.3f}
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                         "hand1.png", glm::vec2{0.2f, 0.95f}
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                         "dart0.png", glm::vec2{0.35f, 0.8f}
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                         "hand0.png", glm::vec2(0.55f, 0.8f), glm::vec2( 0.5f, 1.0f )
                                 ),

                         },
                                1000
                        },
                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow2.png", glm::vec2{0.1f, 0.3f}, glm::vec2( 0.1f, 1.0f )
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                         "hand1.png", glm::vec2{0.2f, 0.95f}, glm::vec2( 0.2f, 1.65f )
                                 ),
                                 std::make_shared<odb::GraphicNode>(
                                         "dart0.png", glm::vec2{0.35f, 0.8f}, glm::vec2( 0.35f, 1.5f )
                                 )


                         },
                                1000
                        },
                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow0.png", glm::vec2{0.35f, 1.0f}, glm::vec2{0.45f, 0.8f}
                                 ),
                         },
                                500
                        },
                        {{
                                 std::make_shared<odb::GraphicNode>(
                                         "bow0.png", glm::vec2{0.45f, 0.8f}, glm::vec2{0.5125f, 0.85f}
                                 ),
                         },
                                500
                        },



                },
                false,
                "crossbow-still"
        );


        currentAnimation = nullptr;
    }

    void OverlayNanoVGRenderer::render(const odb::NoudarDungeonSnapshot &snapshot) {

        if (mFontData.size() > 0) {

#ifdef NANOVG_GLES2_IMPLEMENTATION
            mContext = nvgCreateGLES2(NVG_STENCIL_STROKES);
#else
            mContext = 	nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES );
#endif

            if (!mContext) {
                std::cout << "NVG context is faulty as salty" << std::endl;
            }

            unsigned char *data = (unsigned char *) malloc(mFontData.size());
            std::copy(mFontData.begin(), mFontData.end(), data);
            nvgCreateFontMem(mContext, "font", data, mFontData.size(), 1);
            mFontData.clear();
        }

        if (mFrames.empty()) {

            for (const auto &bitmapPair : mBitmaps) {
                auto bitmap = bitmapPair.second;
                int imgW = bitmap->getWidth();
                int imgH = bitmap->getHeight();

                mFrames[bitmapPair.first] = nvgCreateImageRGBA(mContext, imgW, imgH, 0,
                                                               (const unsigned char *) bitmap->getPixelData());
            }
        }

        glViewport(0, 0, mWidth, mHeight);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        nvgBeginFrame(mContext, mWidth, mHeight, mWidth / mHeight);
        nvgFontSize(mContext, 18.0f);
        nvgFontFace(mContext, "font");
        nvgFillColor(mContext, nvgRGBA(255, 255, 255, 255));
        nvgStrokeColor(mContext, nvgRGBA(255, 0, 0, 255));
        nvgTextAlign(mContext, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgText(mContext, 10, 10, snapshot.mCurrentItem.c_str(), nullptr);

        std::stringstream ss;
        ss << snapshot.mHP;
        nvgText(mContext, 10, mHeight - 18, ss.str().c_str(), nullptr);

        timeUntilNextFrame -= (snapshot.mTimestamp - mLastTimestamp);
        mLastTimestamp = snapshot.mTimestamp;

        if ( currentAnimation != nullptr ) {


            auto animationSize = currentAnimation->mStepList.size();
            if (timeUntilNextFrame <= 0 && frame < animationSize) {

                if (currentAnimation->mRepeatedPlayback) {
                    frame = (frame + 1) % currentAnimation->mStepList.size();
                } else if (frame < animationSize - 1) {
                    ++frame;
                } else {
                    playAnimation( snapshot.mTimestamp, currentAnimation->mNextAnimation );
                }

                timeUntilNextFrame = currentAnimation->mStepList[frame].mDelay;
            }

            for (const auto &node : currentAnimation->mStepList[frame].mNodes) {

                auto nodeId = node->mFrameId;
                auto registeredTexture = mFrames[node->mFrameId];
                auto bitmap = mBitmaps[node->mFrameId];
                int imgW = bitmap->getWidth();
                int imgH = bitmap->getHeight();
                auto position = node->getPositionForTime( 1.0f - ((float) timeUntilNextFrame ) / ( (float) currentAnimation->mStepList[frame].mDelay ) );
                float offsetX = position.x * mWidth;
                float offsetY = position.y * mHeight;
                auto imgPaint = nvgImagePattern(mContext, offsetX, offsetY, imgW, imgH, 0, registeredTexture, 1.0f);
                nvgBeginPath(mContext);
                nvgRect(mContext, offsetX, offsetY, imgW, imgH);
                nvgFillPaint(mContext, imgPaint);
                nvgFill(mContext);
            }
        }

        nvgEndFrame(mContext);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    OverlayNanoVGRenderer::~OverlayNanoVGRenderer() {
        nvgDeleteGLES2(mContext);
    }

    void OverlayNanoVGRenderer::playAnimation( long currentTimestamp, std::string animationName ) {
        currentAnimation = animations[ animationName ];

        if ( currentAnimation != nullptr ) {
            mLastTimestamp = currentTimestamp;
            timeUntilNextFrame = currentAnimation->mStepList[frame].mDelay;
            frame = 0;
        }
    }
}
