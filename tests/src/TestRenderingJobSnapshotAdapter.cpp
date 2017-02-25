#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "glm/glm.hpp"

#include <memory>
#include <iostream>
#include <array>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <VBORenderingJob.h>
#include <vector>
#include <utility>
#include <map>
#include <vector>
#include "NativeBitmap.h"
#include "Texture.h"
#include "Material.h"
#include "Trig.h"
#include "TrigBatch.h"
#include "MeshObject.h"
#include "MaterialList.h"
#include "Scene.h"
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

using ::testing::Return;
using ::testing::_;


TEST(TestRenderingJobSnapshotAdapter, WillDoNothingWithAInvalidTimestampInSnapshot) {
	odb::RenderingJobSnapshotAdapter adapter;

	auto pos = adapter.easingAnimationCurveStep( { 1, 1}, {3, 3}, 0, 250 );

	ASSERT_FLOAT_EQ( 2, pos.x );
	ASSERT_FLOAT_EQ( 2, pos.y );
}