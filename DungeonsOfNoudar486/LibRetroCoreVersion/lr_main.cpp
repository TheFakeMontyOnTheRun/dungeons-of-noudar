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

#include "libretro.h"

void *operator new[](size_t size, const char *pName, int flags, unsigned debugFlags,
                     const char *file, int line) {
    return new uint8_t[size];
}

void *operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char *pName,
                     int flags, unsigned debugFlags, const char *file, int line) {
    return new uint8_t[size];;
}

std::shared_ptr<odb::CPackedFileReader> fileLoader = nullptr;
std::shared_ptr<odb::CRenderer> odb::renderer = nullptr;
std::shared_ptr<Knights::CGame> game = nullptr;

static uint32_t *frame_buf;
static struct retro_log_callback logging;
static retro_log_printf_t log_cb;

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
//    (void)level;
//    va_list va;
//    va_start(va, fmt);
//    vfprintf(stderr, fmt, va);
//    va_end(va);
}

void retro_init(void)
{
    log_cb(RETRO_LOG_INFO, "retro_init.\n");
    frame_buf = calloc(320 * 240, sizeof(uint32_t));

    fileLoader = std::make_shared<odb::CPackedFileReader>("data.pfs");
    odb::renderer = std::make_shared<odb::CRenderer>(fileLoader);
    game = std::make_shared<Knights::CGame>(fileLoader, odb::renderer, std::make_shared<Knights::CGameDelegate>());
}

void retro_deinit(void)
{
    free(frame_buf);
    frame_buf = NULL;
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name     = "Dungeons of Noudar 3D";
    info->library_version  = "v1";
    info->need_fullpath    = false;
    info->valid_extensions = NULL; // Anything is fine, we don't care.
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    float aspect = 1.6f;
    float sampling_rate = 30000.0f;

    info->timing = (struct retro_system_timing) {
            .fps = 60.0,
            .sample_rate = sampling_rate,
    };

    info->geometry = (struct retro_game_geometry) {
            .base_width   = 320,
            .base_height  = 200,
            .max_width    = 320,
            .max_height   = 200,
            .aspect_ratio = aspect,
    };
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    bool no_content = true;
    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    else
        log_cb = fallback_log;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

static unsigned x_coord;
static unsigned y_coord;
static int mouse_rel_x;
static int mouse_rel_y;

void retro_reset(void)
{
    x_coord = 0;
    y_coord = 0;
}

bool up = false;
bool down = false;
bool left = false;
bool right = false;

bool strafeLeft = false;
bool strafeRight = false;

bool use = false;
bool pick = false;
bool cycle = false;


static void update_input(void)
{
    input_poll_cb();
    if (!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))
    {
        if ( up ) {
            odb::renderer->mBufferedCommand = Knights::kMovePlayerForwardCommand;
        }

        up = false;
    } else {
        up = true;
    }

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))
    {
        if (down) {
            odb::renderer->mBufferedCommand = Knights::kMovePlayerBackwardCommand;
        }
        down = false;
    } else {
        down = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))
    {
        if ( left ) {
            odb::renderer->mBufferedCommand = Knights::kTurnPlayerLeftCommand;
        }
        left = false;
    } else {
        left = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
    {
        if ( right ) {
            odb::renderer->mBufferedCommand = Knights::kTurnPlayerRightCommand;
        }
        right = false;
    } else {
        right = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L))
    {
        if ( strafeLeft ) {
            odb::renderer->mBufferedCommand = Knights::kStrafeLeftCommand;
        }
        strafeLeft = false;
    } else {
        strafeLeft = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R))
    {
        if (strafeRight ) {
            odb::renderer->mBufferedCommand = Knights::kStrafeRightCommand;
        }
        strafeRight = false;
    } else {
        strafeRight = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A))
    {
        if (use) {
            odb::renderer->mBufferedCommand = Knights::kUseCurrentItemInInventoryCommand;
        }
        use = false;
    } else {
        use = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B))
    {
        if (pick) {
            odb::renderer->mBufferedCommand = Knights::kPickItemCommand;
        }
        pick = false;
    } else {
        pick = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X))
    {
        if (cycle){
            odb::renderer->mBufferedCommand = Knights::kCycleRightInventoryCommand;
        }
        cycle = false;
    } else {
        cycle = true;
    }



}

static void render_checkered(void)
{
    uint32_t *buf    = frame_buf;
    log_cb(RETRO_LOG_INFO, "render_checkered.\n");

    odb::renderer->render(1);

    auto imgBuffer = odb::renderer->getBufferData();

    unsigned stride  = 320;
    uint32_t *line   = buf;
    auto srcLine = imgBuffer;
    for (unsigned y = 0; y < 200; y++, line += stride)
    {
        for (unsigned x = 0; x < 320; x++)
        {

            auto pixel = odb::renderer->mPalette[ *srcLine ];
            auto finalPixel = 0;
            finalPixel += (((pixel & 0xFF0000) >> 16) - 0x10 ) << 0;
            finalPixel += (((pixel & 0x00FF00) >> 8 ) - 0x18 ) << 8;
            finalPixel += (((pixel & 0x0000FF) >> 0 ) - 0x38 ) << 16;
            line[x] = finalPixel;
            ++srcLine;
        }
    }



    video_cb(buf, 320, 200, stride << 2);
}

static void check_variables(void)
{
}

static void audio_callback(void)
{
    audio_cb(0, 0);
}

void retro_run(void)
{
    log_cb(RETRO_LOG_INFO, "retro_run.\n");


    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
        check_variables();


    update_input();
    odb::renderer->handleSystemEvents();
    game->tick();
    render_checkered();
    audio_callback();

}

bool retro_load_game(const struct retro_game_info *info)
{
    log_cb(RETRO_LOG_INFO, "retro_load_game.\n");
    auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
    odb::renderer->loadTextures(odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader), tileProperties);


    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
        return false;
    }

    check_variables();

    (void)info;
    return true;
}

void retro_unload_game(void)
{
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
    if (type != 0x200)
        return false;
    if (num != 2)
        return false;
    return retro_load_game(NULL);
}

size_t retro_serialize_size(void)
{
    return 2;
}

bool retro_serialize(void *data_, size_t size)
{
    if (size < 2)
        return false;

    uint8_t *data = data_;
    data[0] = x_coord;
    data[1] = y_coord;
    return true;
}

bool retro_unserialize(const void *data_, size_t size)
{
    if (size < 2)
        return false;

    const uint8_t *data = data_;
    x_coord = data[0] & 31;
    y_coord = data[1] & 31;
    return true;
}

void *retro_get_memory_data(unsigned id)
{
    (void)id;
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    (void)id;
    return 0;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    (void)index;
    (void)enabled;
    (void)code;
}