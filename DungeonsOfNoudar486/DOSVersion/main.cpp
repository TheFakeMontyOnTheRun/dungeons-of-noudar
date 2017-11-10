#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <functional>
#include <memory>
#include <iostream>
#include <utility>
#include <string>
#include <memory>
#include <fstream>
#include <map>
#include <algorithm>
#include <memory>
#include <chrono>
#include <iostream>
#include <sg14/fixed_point>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;
using namespace std::chrono;
using sg14::fixed_point;

#include "NativeBitmap.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "IFileLoaderDelegate.h"
#include "CGame.h"
#include "CPlainFileLoader.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "commands/IGameCommand.h"
#include "RasterizerCommon.h"
#include "CRenderer.h"
#include "DOSHacks.h"

std::shared_ptr<odb::CRenderer> renderer;

std::function< std::string(std::string)> kDosLongFileNameTransformer = [](const std::string& filename ) {
    char c = 219;
    c = 176;
    c = 177;
    c = 178;
    c = '.';
    std::cout << c;
    std::cout.flush();

    auto dotPosition = std::find( std::begin(filename), std::end( filename), '.');
    auto indexDot =  std::distance( std::begin( filename ), dotPosition );
    auto extension = filename.substr( indexDot + 1, 3 );

    if ( indexDot >  8 ) {
        return filename.substr( 0, 6 ) + "~1." + extension;
    }

    if ( filename.length() - indexDot > 4 ) {
        return filename.substr( 0, indexDot ) + "~1." + extension;
    }

    return filename;
};

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags,
                     const char* file, int line) {
    return malloc( size );
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName,
                     int flags, unsigned debugFlags, const char* file, int line) {
    return malloc( size );
}

void gameLoopTick() {
    renderer->render( 33 );
    renderer->handleSystemEvents();
}


int main() {

    const auto LEVEL_LIMIT = 2;
    renderer = std::make_shared<odb::CRenderer>();

    auto delegate = std::make_shared<Knights::CGameDelegate>();

    auto fileLoader =  std::make_shared<Knights::CPlainFileLoader>("res\\");

    fileLoader->setFilenameTransformation( kDosLongFileNameTransformer );

    auto tileProperties = odb::CTile3DProperties::parsePropertyList(fileLoader->loadFileFromPath("tiles0.prp"));

    renderer->loadTextures( odb::loadTexturesForLevel(0, fileLoader), tileProperties);

    auto game = std::make_shared<Knights::CGame>( fileLoader, renderer, delegate );

    auto onLevelLoaded = [&]() {
        if ( game->getLevelNumber() >= LEVEL_LIMIT ) {
            game->setIsPlaying( false );
        }
    };

    delegate->setOnLevelLoadedCallback(onLevelLoaded );

#ifdef __EMSCRIPTEN__
    //  emscripten_request_fullscreen(0, 1);
  emscripten_set_main_loop( gameLoopTick, 30, 1 );
#else

    game->endOfTurn(game->getMap());

    auto item = game->getMap()->makeItemWithSymbol('y');
    auto avatar = game->getMap()->getAvatar();
    avatar->giveItem(item);
    avatar->suggestCurrentItem('y');

    while ( game->isPlaying() ) {
        game->tick();
        renderer->sleep( 33 );
        gameLoopTick();

    }
#endif
    return 0;
}
