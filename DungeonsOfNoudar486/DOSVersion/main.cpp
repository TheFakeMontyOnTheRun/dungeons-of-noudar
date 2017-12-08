#include <conio.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <memory>
#include <utility>
#include <string>
#include <map>
#include <algorithm>
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
#include "CPackedFileReader.h"
#include "LoadPNG.h"

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

std::shared_ptr<Knights::CGame> game;

int main(int argc, char **argv) {
    printf("Dungeons of Noudar 486 tech demo startup. Gonna load some stuff...");
    const auto LEVEL_LIMIT = 2;
    auto delegate = std::make_shared<Knights::CGameDelegate>();
    auto fileLoader = std::make_shared<odb::CPackedFileReader>("data.pfs");
    auto intro = loadPNG( "intro.png", fileLoader );
    renderer = std::make_shared<odb::CRenderer>();
    renderer->drawBitmap(0, 0, intro );
    renderer->flip();
    game = std::make_shared<Knights::CGame>( fileLoader, renderer, delegate );

    if ( argc > 1 ) {
        game->playLevel(atoi( argv[1]));
    }


    auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
    renderer->loadTextures( odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader), tileProperties);

    auto onLevelLoaded = [&]() {
        if ( game->getLevelNumber() >= LEVEL_LIMIT ) {
            game->setIsPlaying( false );
        } else {
            auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
            renderer->loadTextures( odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader), tileProperties);
        }
    };

    delegate->setOnLevelLoadedCallback(onLevelLoaded );

#ifdef __EMSCRIPTEN__
    //  emscripten_request_fullscreen(0, 1);
  emscripten_set_main_loop( gameLoopTick, 30, 1 );
#else

    game->endOfTurn(game->getMap());

    auto ready = loadPNG( "enter.png", fileLoader );
    renderer->drawBitmap(0, 0, intro );
    renderer->drawBitmap(0, 180, ready );
    renderer->flip();


    getch();

    while ( game->isPlaying() ) {
        game->tick();
        renderer->sleep( 33 );
        gameLoopTick();

    }
#endif
    return 0;
}
