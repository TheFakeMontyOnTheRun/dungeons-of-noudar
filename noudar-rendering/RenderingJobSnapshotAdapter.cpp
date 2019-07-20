//
// Created by monty on 07/02/17.
//
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <memory>
#include <iostream>
#include <functional>
#include <utility>
#include <unordered_map>
#include <map>
#include <vector>
#include <array>
#include <Camera.h>

using std::vector;
using std::array;

#include "VBORenderingJob.h"
#include "NativeBitmap.h"
#include "ETextures.h"
#include "VBORenderingJob.h"
#include "VBORegister.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "CTile3DProperties.h"
#include "NoudarDungeonSnapshot.h"
#include "RenderingJobSnapshotAdapter.h"


namespace odb {
    const static glm::mat4 identity = glm::mat4(1.0f);
    int RenderingJobSnapshotAdapter::visibility = 8;

    const static bool kRenderSky = true;

    const static bool kOnlyRenderAreaAroundPlayer = false;

    const static bool kUseRepeatedGeometryStances = false;

    const static bool kUseSmoothAnimationCurves = true;

    glm::mat4 RenderingJobSnapshotAdapter::getSkyTransform(long animationTime) {
        long offset = animationTime;
        int integerPart = offset % ((kSkyTextureLength * 2) * 1000);
        float finalOffset = integerPart / 1000.0f;

        return glm::translate(identity, glm::vec3(finalOffset, 0.0f, 0.0f));
    }


    glm::mat4
    RenderingJobSnapshotAdapter::getCubeTransform(glm::vec3 translation, float scale = 1.0f) {
        return glm::scale(glm::translate(identity, translation), glm::vec3(1.0f, scale, 1.0f));
    }

    glm::mat4 RenderingJobSnapshotAdapter::getFloorTransform(glm::vec3 translation) {
        return glm::translate(identity, translation);
    }

    glm::vec2 RenderingJobSnapshotAdapter::easingAnimationCurveStep(glm::vec2 prevPosition,
                                                                    glm::vec2 destPosition,
                                                                    long animationTime,
                                                                    long timestamp) {
        float step = (((float) ((timestamp - animationTime))) /
                      ((float) kAnimationLength));


        float curve = 0.0f;

        if (kUseSmoothAnimationCurves) {
            if (step < 0.5f) {
                curve = ((2.0f * step) * (2.0f * step)) / 2.0f;
            } else {
                curve = (sqrt((step * 2.0f) - 1.0f) / 2.0f) + 0.5f;
            }
        } else {
            curve = step;
        }

        return {(curve * (destPosition.x - prevPosition.x)) + prevPosition.x,
                (curve * (destPosition.y - prevPosition.y)) + prevPosition.y};

    }

    glm::mat4 RenderingJobSnapshotAdapter::getBillboardTransform(const Camera &camera,
                                                                 glm::vec3 translation) {
        glm::mat4 identity = glm::mat4(1.0f);
        glm::mat4 translated = glm::translate(identity, translation);

#if defined(__ANDROID__ )
//		if (mUseStereoBillboardBehavior) {
//            return glm::rotate(translated,
//                               (mCamera.getCameraRotationXZ()) * (3.141592f / 180.0f),
//                               glm::vec3(0.0f, 1.0f, 0.0f));
//        }
#endif

        return glm::rotate(translated,
                           (360 - camera.getCameraRotationXZ()) * (3.141592f / 180.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void RenderingJobSnapshotAdapter::readSnapshot(const NoudarDungeonSnapshot &snapshot,
                                                   std::unordered_map<ETextures, vector<VBORenderingJob>> &batches,
                                                   const CTilePropertyMap &tilePropertiesRegistry,
                                                   const std::unordered_map<VBORegisterId, VBORegister> &VBORegisters,
                                                   const std::unordered_map<std::string, ETextures> &textureRegistry,
                                                   const Camera &camera,
                                                   const std::unordered_map<EActorsSnapshotElement, ETextures> &elementMap
    ) {

        glm::vec3 pos;
        const auto &floorVBO = VBORegisters.at("floor");
        const auto &billboardVBO = VBORegisters.at("billboard");
        batches.clear();

        uint8_t x0;
        uint8_t x1;
        uint8_t z0;
        uint8_t z1;

        if (kOnlyRenderAreaAroundPlayer) {
            x0 = std::max(0, snapshot.mCameraPosition.x - visibility);
            x1 = std::min(Knights::kMapSize, snapshot.mCameraPosition.x + visibility);
            z0 = std::max(0, snapshot.mCameraPosition.y - visibility);
            z1 = std::min(Knights::kMapSize, snapshot.mCameraPosition.y + visibility);
        } else {
            x0 = 0;
            x1 = Knights::kMapSize - 1;
            z0 = 0;
            z1 = Knights::kMapSize - 1;
        }

        if (kRenderSky) {
            const auto &skyVBO = VBORegisters.at("sky");
            batches[ETextures::Skybox].emplace_back(std::get<0>(skyVBO),
                                                    std::get<1>(skyVBO),
                                                    std::get<2>(skyVBO),
                                                    getSkyTransform(snapshot.mTimestamp),
                                                    1.0f, true);

            batches[ETextures::Skybox].emplace_back(std::get<0>(skyVBO),
                                                    std::get<1>(skyVBO),
                                                    std::get<2>(skyVBO),
                                                    getSkyTransform(
                                                            snapshot.mTimestamp +
                                                            kSkyTextureLength * 1000),
                                                    1.0f, true);
        }


        for (int z = z0; z <= z1; ++z) {
            for (int x = x0; x <= x1; ++x) {

                if (snapshot.mVisibilityMap[z][x] == EVisibility::kInvisible) {
                    continue;
                }

                auto tile = snapshot.map[z][x];

                Shade shade = (snapshot.mLightMap[z][x]) / 255.0f;

                if (x == snapshot.mCursorPosition.x &&
                    z == snapshot.mCursorPosition.y) {
                    shade = 1.5f;
                }


                auto mapItem = snapshot.mItemMap[z][x];
                auto actor = snapshot.snapshot[z][x];
                int splatFrame = snapshot.splat[z][x];
                auto tileProperties = tilePropertiesRegistry.at(tile);

                if (tilePropertiesRegistry.find(tile) == std::end(tilePropertiesRegistry)) {
                    continue;
                }


                if (tileProperties.mCeilingTexture != mNullTexture) {
                    pos = glm::vec3(x * 2, -5 + (2.0f * tileProperties.mCeilingHeight), z * 2);
                    batches[textureRegistry.at(tileProperties.mCeilingTexture)].emplace_back(
                            std::get<0>(floorVBO),
                            std::get<1>(floorVBO),
                            std::get<2>(floorVBO),
                            getFloorTransform(pos),
                            shade, true);
                }

                if (tileProperties.mCeilingRepetitions > 0 &&
                    tileProperties.mCeilingRepeatedWallTexture != mNullTexture) {

                    const auto &tileVBO = VBORegisters.at(tileProperties.mVBOToRender);
                    auto vboId = std::get<0>(tileVBO);
                    auto vboIndicesId = std::get<1>(tileVBO);
                    auto amount = std::get<2>(tileVBO);
                    auto &repeatedBatches = batches[textureRegistry.at(
                            tileProperties.mCeilingRepeatedWallTexture)];

                    if (kUseRepeatedGeometryStances) {
                        for (float y = 0; y < tileProperties.mCeilingRepetitions; ++y) {
                            pos = glm::vec3(x * 2,
                                            -4.0f + (2.0f * tileProperties.mCeilingHeight) +
                                            (2.0 * y),
                                            z * 2);

                            repeatedBatches.emplace_back(
                                    vboId,
                                    vboIndicesId,
                                    amount,
                                    getCubeTransform(pos),
                                    shade,
                                    tileProperties.mNeedsAlphaTest);
                        }
                    } else {
                        pos = glm::vec3(x * 2,
                                        -4 + (2.0f * tileProperties.mCeilingHeight) +
                                        (tileProperties.mCeilingRepetitions) - 1.0f,
                                        z * 2);

                        repeatedBatches.emplace_back(
                                vboId,
                                vboIndicesId,
                                amount,
                                getCubeTransform(pos, tileProperties.mCeilingRepetitions),
                                shade,
                                tileProperties.mNeedsAlphaTest);

                    }
                }

                if (tileProperties.mMainWallTexture != mNullTexture) {
                    const auto &tileVBO = VBORegisters.at(tileProperties.mVBOToRender);
                    if (kUseRepeatedGeometryStances) {
                        for (float y = tileProperties.mFloorHeight;
                             y < tileProperties.mCeilingHeight; ++y) {
                            pos = glm::vec3(x * 2, -4.0 + (y * 2), z * 2);

                            batches[textureRegistry.at(
                                    tileProperties.mMainWallTexture)].emplace_back(
                                    std::get<0>(tileVBO),
                                    std::get<1>(tileVBO),
                                    std::get<2>(tileVBO),
                                    getCubeTransform(pos), shade,
                                    tileProperties.mNeedsAlphaTest);
                        }
                    } else {
                        auto height = tileProperties.mCeilingHeight - tileProperties.mFloorHeight;
                        pos = glm::vec3(x * 2, -5.0f + 2.0f * (tileProperties.mFloorHeight +
                                                               tileProperties.mCeilingHeight) /
                                                       2.0f, z * 2);

                        batches[textureRegistry.at(tileProperties.mMainWallTexture)].emplace_back(
                                std::get<0>(tileVBO),
                                std::get<1>(tileVBO),
                                std::get<2>(tileVBO),
                                getCubeTransform(pos, height), shade,
                                tileProperties.mNeedsAlphaTest);

                    }
                }

                if (tileProperties.mFloorRepetitions > 0 &&
                    tileProperties.mFloorRepeatedWallTexture != mNullTexture) {

                    const auto &tileVBO = VBORegisters.at(tileProperties.mVBOToRender);
                    auto vboId = std::get<0>(tileVBO);
                    auto vboIndicesId = std::get<1>(tileVBO);
                    auto amount = std::get<2>(tileVBO);
                    auto &repeatedBatches = batches[textureRegistry.at(
                            tileProperties.mFloorRepeatedWallTexture)];

                    if (kUseRepeatedGeometryStances) {
                        for (float y = 0; y < tileProperties.mFloorRepetitions; ++y) {
                            //the final -1.0f in y is for accounting fore the block's length
                            pos = glm::vec3(x * 2,
                                            -5.0f + (2.0f * tileProperties.mFloorHeight) -
                                            (2.0 * y) -
                                            1.0f, z * 2);

                            repeatedBatches.emplace_back(
                                    vboId,
                                    vboIndicesId,
                                    amount, getCubeTransform(pos),
                                    shade,
                                    tileProperties.mNeedsAlphaTest);
                        }
                    } else {
                        pos = glm::vec3(x * 2,
                                        -5.0f + (2.0f * tileProperties.mFloorHeight) -
                                        (tileProperties.mFloorRepetitions), z * 2);

                        repeatedBatches.emplace_back(
                                vboId,
                                vboIndicesId,
                                amount,
                                getCubeTransform(pos, tileProperties.mFloorRepetitions),
                                shade,
                                tileProperties.mNeedsAlphaTest);
                    }
                }

                if (tileProperties.mFloorTexture != mNullTexture) {
                    pos = glm::vec3(x * 2, -5.0f + (2.0f * tileProperties.mFloorHeight), z * 2);
                    batches[textureRegistry.at(tileProperties.mFloorTexture)].emplace_back(
                            std::get<0>(floorVBO),
                            std::get<1>(floorVBO),
                            std::get<2>(floorVBO),
                            getFloorTransform(pos),
                            shade, true);
                }

                if (mapItem == 't') {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);
                    batches[ETextures::Falcata].emplace_back(
                            std::get<0>(billboardVBO),
                            std::get<1>(billboardVBO),
                            std::get<2>(billboardVBO),
                            getBillboardTransform(camera, pos), shade, true);

                }
                if (mapItem == '+') {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);
                    batches[ETextures::Cross].emplace_back(
                            std::get<0>(billboardVBO),
                            std::get<1>(billboardVBO),
                            std::get<2>(billboardVBO),
                            getBillboardTransform(camera, pos), shade, true);

                }
                if (mapItem == 'y') {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);
                    batches[ETextures::Crossbow].emplace_back(
                            std::get<0>(billboardVBO),
                            std::get<1>(billboardVBO),
                            std::get<2>(billboardVBO),
                            getBillboardTransform(camera, pos), shade, true);

                }

                if (mapItem == 'v') {
                    pos = glm::vec3(x * 2, -4.0f, z * 2);
                    batches[ETextures::Shield].emplace_back(
                            std::get<0>(billboardVBO),
                            std::get<1>(billboardVBO),
                            std::get<2>(billboardVBO),
                            getBillboardTransform(camera, pos), shade, true);

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
                    height = tileProperties.mFloorHeight;

                    if (id != 0 && snapshot.movingCharacters.count(id) > 0) {

                        auto animation = snapshot.movingCharacters.at(id);
                        auto pos = easingAnimationCurveStep(std::get<0>(animation),
                                                            std::get<1>(animation),
                                                            std::get<2>(animation),
                                                            snapshot.mTimestamp);

                        fx = pos.x;
                        fz = pos.y;

                    }

                    pos = glm::vec3(fx * 2.0f, -4.0f + 2 * height, fz * 2.0f);


                    if (id != snapshot.mCameraId) {

                        TextureId frame = elementMap.at(actor);

                        batches[static_cast<ETextures >(frame)].emplace_back(
                                std::get<0>(billboardVBO),
                                std::get<1>(billboardVBO),
                                std::get<2>(billboardVBO),
                                getBillboardTransform(camera, pos), shade, true);
                    }
                }

                if (splatFrame > -1) {
                    float height = tileProperties.mFloorHeight;
                    pos = glm::vec3(x * 2, -4.0f + 2.0f * height, z * 2);
                    batches[static_cast<ETextures >(splatFrame +
                                                    ETextures::Splat0)].emplace_back(
                            std::get<0>(billboardVBO),
                            std::get<1>(billboardVBO),
                            std::get<2>(billboardVBO),
                            getBillboardTransform(camera, pos), shade, true);
                }
            }
        }
    }
}
