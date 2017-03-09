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

#include "VisibilityStrategy.h"

auto getTestSnapshotWithCrossOfSize(int size, Knights::Vec2i characterPosition) -> odb::NoudarDungeonSnapshot {
    odb::NoudarDungeonSnapshot snapshot;
    int middleOfTheMap = Knights::kMapSize / 2;

    for ( auto& line : snapshot.map ) {
        std::fill( begin( line ), end( line ), '_' );
    }

    for ( auto& line : snapshot.mVisibilityMap ) {
        std::fill( begin( line ), end( line ), odb::EVisibility::kInvisible );
    }

    for ( auto& line : snapshot.snapshot ) {
        std::fill( begin( line ), end( line ), odb::EActorsSnapshotElement::kNothing );
    }


    snapshot.snapshot[ characterPosition.y ][ characterPosition.x ] = odb::EActorsSnapshotElement::kHeroStanding0;

    for ( int c = -size/2; c <= size / 2; ++c ) {
        snapshot.map[ middleOfTheMap + c ][ middleOfTheMap ] = 'X';
        snapshot.map[ middleOfTheMap ][ middleOfTheMap + c ] = 'Y';
    }

    return snapshot;
}

TEST(TestVisibilityStrategy, VisibilityWillPropagateToCoverTheFullFieldOfView) {
    odb::VisibilityStrategy strategy;
    int middleOfTheMap = Knights::kMapSize / 2;
    Knights::Vec2i cameraPosition = { middleOfTheMap, Knights::kMapSize - 2 };
    auto snapshot = getTestSnapshotWithCrossOfSize(2, cameraPosition );


    strategy.castVisibility(snapshot.mVisibilityMap, snapshot.map, cameraPosition, Knights::EDirection::kNorth, true );
    std::cout << "map: " << std::endl << snapshot << std::endl;
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y ][ cameraPosition.x ], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y ][ cameraPosition.x - 10], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y ][ cameraPosition.x + 10], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y - 5][ cameraPosition.x - 10], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y - 5][ cameraPosition.x + 10], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y + 1][ cameraPosition.x ], odb::EVisibility::kInvisible);
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y + 1][ cameraPosition.x - 1], odb::EVisibility::kInvisible);
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y + 1][ cameraPosition.x + 1], odb::EVisibility::kInvisible);
}

TEST(TestVisibilityStrategy, VisibilityWillWorkRegardlessOfDirection) {
    odb::VisibilityStrategy strategy;
    int middleOfTheMap = Knights::kMapSize / 2;
    Knights::Vec2i cameraPosition = { 2, middleOfTheMap };
    auto snapshot = getTestSnapshotWithCrossOfSize(2, cameraPosition);


    strategy.castVisibility(snapshot.mVisibilityMap, snapshot.map, cameraPosition, Knights::EDirection::kEast, true );
    std::cout << "map: " << std::endl << snapshot << std::endl;

    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y - 1][ middleOfTheMap - 2 ], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y    ][ middleOfTheMap - 2 ], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y + 1][ middleOfTheMap - 2 ], odb::EVisibility::kVisible );

    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y - 1][ middleOfTheMap + 1], odb::EVisibility::kInvisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y    ][ middleOfTheMap + 1], odb::EVisibility::kInvisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y + 1][ middleOfTheMap + 1], odb::EVisibility::kInvisible );

    ASSERT_EQ( snapshot.mVisibilityMap[ 0 ][ Knights::kMapSize - 1], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ Knights::kMapSize - 1][ Knights::kMapSize - 1], odb::EVisibility::kVisible );

    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y + 10][ cameraPosition.x + 5], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y - 10][ cameraPosition.x + 5], odb::EVisibility::kVisible );

    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y - 1][ cameraPosition.x - 1], odb::EVisibility::kInvisible);
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y    ][ cameraPosition.x - 1], odb::EVisibility::kInvisible);
    ASSERT_EQ( snapshot.mVisibilityMap[ cameraPosition.y + 1][ cameraPosition.x - 1], odb::EVisibility::kInvisible);
}

TEST(TestVisibilityStrategy, WillNotFlowInOpositeDirection) {
    odb::VisibilityStrategy strategy;
    int middleOfTheMap = Knights::kMapSize / 2;
    Knights::Vec2i cameraPosition = { ( Knights::kMapSize) / 3, Knights::kMapSize - 1 };
    auto snapshot = getTestSnapshotWithCrossOfSize(2, cameraPosition);

    strategy.castVisibility(snapshot.mVisibilityMap, snapshot.map, cameraPosition, Knights::EDirection::kNorth, true );
    std::cout << "map: " << std::endl << snapshot << std::endl;
    ASSERT_EQ( snapshot.mVisibilityMap[ middleOfTheMap - 1][ middleOfTheMap + 1 ], odb::EVisibility::kInvisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ middleOfTheMap - 1][ middleOfTheMap - 1 ], odb::EVisibility::kVisible );

}

TEST(TestVisibilityStrategy, WillOmitTheBacksideOfWall) {
    int middleOfTheMap = Knights::kMapSize / 2;
    Knights::Vec2i cameraPosition = { middleOfTheMap, Knights::kMapSize - 1 };
    auto snapshot = getTestSnapshotWithCrossOfSize(2, cameraPosition);
    odb::VisibilityStrategy strategy;


    strategy.castVisibility(snapshot.mVisibilityMap, snapshot.map, cameraPosition, Knights::EDirection::kNorth, true );
    std::cout << "map: " << std::endl << snapshot << std::endl;
    ASSERT_EQ( snapshot.mVisibilityMap[ middleOfTheMap ][ middleOfTheMap ], odb::EVisibility::kInvisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ middleOfTheMap - 2 ][ middleOfTheMap ], odb::EVisibility::kInvisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ middleOfTheMap + 2][ middleOfTheMap ], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ middleOfTheMap ][ middleOfTheMap - 2], odb::EVisibility::kVisible );
    ASSERT_EQ( snapshot.mVisibilityMap[ middleOfTheMap ][ middleOfTheMap + 2], odb::EVisibility::kVisible );
}