#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

#include "Vec2i.h"
#include "CCharacterArchetype.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CCharacter.h"

class MockCharacter : public Knights::CCharacter {
 public:
  MockCharacter( std::shared_ptr<Knights::CCharacterArchetype> aArchetype, std::shared_ptr<Knights::CTeam> aTeam, int aId) : CCharacter( aArchetype, aTeam, aId ) {
  }

  MOCK_METHOD0( hasEnoughAP, bool() );
};

TEST(TestCCharacter, TestArchetypeInheritance ) {
  auto team1 = std::make_shared<Knights::CTeam>("Test");
  auto archetype1 = std::make_shared<Knights::CCharacterArchetype>( 1, 2, 3, 4, '@', "TestCharacter" );
  int id1 = 0;
  auto actor1 = std::make_shared<Knights::CCharacter>( archetype1, team1, id1 );
  
  EXPECT_EQ( 1, actor1->getAttack() );
}
