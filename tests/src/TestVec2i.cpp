#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Vec2i.h"

TEST(TestVec2i, DisplacingDirectionWithOffsetWillWrap ) {

    ASSERT_EQ( Knights::EDirection::kWest,  Knights::wrapDirection(  Knights::EDirection::kNorth, -1 ) );
    ASSERT_EQ( Knights::EDirection::kNorth, Knights::wrapDirection(  Knights::EDirection::kEast, -1 ) );
    ASSERT_EQ( Knights::EDirection::kEast,  Knights::wrapDirection(  Knights::EDirection::kSouth, -1 ) );
    ASSERT_EQ( Knights::EDirection::kSouth,  Knights::wrapDirection(  Knights::EDirection::kWest, -1 ) );

    ASSERT_EQ( Knights::EDirection::kNorth,  Knights::wrapDirection(  Knights::EDirection::kNorth, -4 ) );
    ASSERT_EQ( Knights::EDirection::kEast, Knights::wrapDirection(  Knights::EDirection::kEast, -4 ) );
    ASSERT_EQ( Knights::EDirection::kSouth,  Knights::wrapDirection(  Knights::EDirection::kSouth, -4 ) );
    ASSERT_EQ( Knights::EDirection::kWest,  Knights::wrapDirection(  Knights::EDirection::kWest, -4 ) );

    ASSERT_EQ( Knights::EDirection::kEast,  Knights::wrapDirection(  Knights::EDirection::kNorth, +1 ) );
    ASSERT_EQ( Knights::EDirection::kSouth, Knights::wrapDirection(  Knights::EDirection::kEast, +1 ) );
    ASSERT_EQ( Knights::EDirection::kWest,  Knights::wrapDirection(  Knights::EDirection::kSouth, +1 ) );
    ASSERT_EQ( Knights::EDirection::kNorth,  Knights::wrapDirection(  Knights::EDirection::kWest, +1 ) );

    ASSERT_EQ( Knights::EDirection::kNorth,  Knights::wrapDirection(  Knights::EDirection::kNorth, +4 ) );
    ASSERT_EQ( Knights::EDirection::kEast, Knights::wrapDirection(  Knights::EDirection::kEast, +4 ) );
    ASSERT_EQ( Knights::EDirection::kSouth,  Knights::wrapDirection(  Knights::EDirection::kSouth, +4 ) );
    ASSERT_EQ( Knights::EDirection::kWest,  Knights::wrapDirection(  Knights::EDirection::kWest, +4 ) );
}