//general stuff below
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
#include <vector>
#include <array>

using std::vector;
using std::array;
using namespace std::chrono;
using sg14::fixed_point;

#include "NativeBitmap.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "MapWithCharKey.h"
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
#include "MapWithCharKey.h"
#include "CMap.h"
#include "IRenderer.h"
#include "commands/IGameCommand.h"
#include "RasterizerCommon.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"
#include "LoadPNG.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

#include "libretro.h"

#define BUFSIZE 128
#define AMP_MUL 64

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;


struct WAVhead
{
    char ChunkID[4];
    uint32_t ChunkSize;
    char Format[4];

    char Subchunk1ID[4];
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;

    char Subchunk2ID[4];
    uint32_t Subchunk2Size;
} head;

void* rawsamples                 = NULL;
unsigned int sample_pos          = 0;
unsigned int samples_tot         = 0;

unsigned int g_samples_to_play   = 0;
unsigned int sample_rate         = 0;
unsigned int bytes_per_sample    = 0;


uint8_t* i114t1o8f;
uint8_t* t200i101o3afo1a;
uint8_t* t200i101o8ao4ao2ao1a;
uint8_t* t200i52o4defg;
uint8_t* t200i53o3fo1f;
uint8_t* t200i98a;
uint8_t* t200i9o1fa;

uint32_t si114t1o8f;
uint32_t st200i101o3afo1a;
uint32_t st200i101o8ao4ao2ao1a;
uint32_t st200i52o4defg;
uint32_t st200i53o3fo1f;
uint32_t st200i98a;
uint32_t st200i9o1fa;


enum
{
    ST_OFF = 0,
    ST_ON,
    ST_AUTO
} state;

static void emit_audio(void)
{
    unsigned int samples_to_play;
    unsigned int samples_played  = 0;

    if (state == ST_OFF)
        return;

    if (state == ST_ON)
        samples_to_play=BUFSIZE;
    if (state == ST_AUTO)
        samples_to_play=g_samples_to_play;

    /* no locking here, despite threading;
     * if we touch this variable, threading is off. */

    while (samples_to_play >= BUFSIZE)
    {
        unsigned int played;
        int16_t samples[2*BUFSIZE];
        unsigned int samples_to_read = samples_to_play;

        if (samples_to_read > BUFSIZE)
            samples_to_read = BUFSIZE;
        if (sample_pos > samples_tot)
            sample_pos = samples_tot;
        if (sample_pos + samples_to_read > samples_tot)
            samples_to_read = samples_tot-sample_pos;

        if (samples_to_read != 0)
        {
            unsigned i;
            uint8_t* rawsamples8  = (uint8_t*)rawsamples + bytes_per_sample * sample_pos;
            int16_t* rawsamples16 = (int16_t*)rawsamples8;

            for (i = 0; i < samples_to_read; i++)
            {
                int16_t left  = 0;
                int16_t right = 0;

                if (head.NumChannels == 1 && head.BitsPerSample==8)
                {
                    left  = rawsamples8[i] * AMP_MUL;
                    right = rawsamples8[i] * AMP_MUL;
                }
                if (head.NumChannels == 2 && head.BitsPerSample==8)
                {
                    left  = rawsamples8[i*2] * AMP_MUL;
                    right = rawsamples8[i*2+1]*AMP_MUL;
                }
                if (head.NumChannels==1 && head.BitsPerSample==16)
                {
                    left  = rawsamples16[i];
                    right = rawsamples16[i];
                }
                if (head.NumChannels==2 && head.BitsPerSample==16)
                {
                    left  = rawsamples16[i*2];
                    right = rawsamples16[i*2+1];
                }

                samples[i*2+0] = left;
                samples[i*2+1] = right;
            }
        }

        if (samples_to_read!=BUFSIZE)
            memset(samples + samples_to_read * 2,
                   0,
                   sizeof(int16_t) * 2 * (BUFSIZE-samples_to_read));

        played               = audio_batch_cb(samples, BUFSIZE);
        sample_pos          += played;
        samples_played      += played;

        if (samples_to_play < played)
            break;

        samples_to_play -= played;

        if (played != BUFSIZE)
            break;
    }

    if (state == ST_AUTO)
        g_samples_to_play-=samples_played;
}


enum class ESoundDriver {
    kNone, kPcSpeaker, kOpl2Lpt, kTandy, kCovox
};

ESoundDriver soundDriver = ESoundDriver::kNone;
void initOPL2(int port);
void playTune(const std::string &);

///game interface
void init();
void loopTick(long ms);
void shutdown();
bool isPlaying();
void pushCommand(char cmd );
uint8_t *getFramebuffer();
uint32_t *getPalette();
void renderTick(long ms);
///

long uclock() {
    return 0;
}

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
    frame_buf = calloc(320 * 200, sizeof(uint32_t));
    init();
}

void retro_deinit(void)
{
    shutdown();
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
    info->need_fullpath    = true;
    info->valid_extensions = NULL; // Anything is fine, we don't care.
}


void retro_get_system_av_info(struct retro_system_av_info *info)
{
    float aspect = 1.6f;
    float sampling_rate = 44100;

    info->timing = (struct retro_system_timing) {
            .fps = 20.0,
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
    sample_pos=0; g_samples_to_play=0;
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
bool start = false;


static void update_input(void)
{
    input_poll_cb();
    if (!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))
    {
        if ( up ) {
            pushCommand( Knights::kMovePlayerForwardCommand );
        }

        up = false;
    } else {
        up = true;
    }

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))
    {
        if (down) {
            pushCommand( Knights::kMovePlayerBackwardCommand );
        }
        down = false;
    } else {
        down = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))
    {
        if ( left ) {
            pushCommand( Knights::kTurnPlayerLeftCommand );
        }
        left = false;
    } else {
        left = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
    {
        if ( right ) {
            pushCommand( Knights::kTurnPlayerRightCommand );
        }
        right = false;
    } else {
        right = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L))
    {
        if ( strafeLeft ) {
            pushCommand( Knights::kStrafeLeftCommand );
        }
        strafeLeft = false;
    } else {
        strafeLeft = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R))
    {
        if (strafeRight ) {
            pushCommand( Knights::kStrafeRightCommand );
        }
        strafeRight = false;
    } else {
        strafeRight = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A))
    {
        if (use) {
            pushCommand( Knights::kUseCurrentItemInInventoryCommand );
        }
        use = false;
    } else {
        use = true;
    }


    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B))
    {
        if (pick) {
            pushCommand( Knights::kPickItemCommand );
        }
        pick = false;
    } else {
        pick = true;
    }

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
    {
        if (start){
            pushCommand( Knights::kStartCommand );
        }
        start = false;
    } else {
        start = true;
    }

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X))
    {
        if (cycle){
            pushCommand( Knights::kCycleRightInventoryCommand );
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

    auto imgBuffer = getFramebuffer();
    auto palette = getPalette();

    unsigned stride  = 320;
    uint32_t *line   = buf;
    auto srcLine = imgBuffer;
    for (unsigned y = 0; y < 200; y++, line += stride)
    {
        for (unsigned x = 0; x < 320; x++)
        {

            auto pixel = palette[ *srcLine ];
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
    if (state == ST_AUTO)
    {
        g_samples_to_play += head.SampleRate / 20.0;
        emit_audio();
    }
}

void retro_run(void)
{
    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
        check_variables();

    update_input();
    odb::renderer->mNeedsToRedraw = true;
    loopTick(12);



    render_checkered();
    audio_callback();

}

void loadAudio( std::shared_ptr<odb::CPackedFileReader> fileLoader, const char* filename, uint8_t** buffer, uint32_t& size ) {

}

void playMusic(const std::string &music) {
    auto musicName = music.c_str();

    uint8_t *sampleData;
    size_t sampleSize;

    if ( !strcmp(musicName, "i114t1o8f" ) ) {
        sampleData = i114t1o8f;
        sampleSize = si114t1o8f;
    } else if ( !strcmp(musicName, "t200i101o3afo1a" ) ) {
        sampleData = t200i101o3afo1a;
        sampleSize = st200i101o3afo1a;

    } else if ( !strcmp(musicName, "t200i101o8ao4ao2ao1a" ) ) {
        sampleData = t200i101o8ao4ao2ao1a;
        sampleSize = st200i101o8ao4ao2ao1a;

    } else if ( !strcmp(musicName, "t200i52o4defg" ) ) {
        sampleData = t200i52o4defg;
        sampleSize = st200i52o4defg;

    } else if ( !strcmp(musicName, "t200i53o3fo1f" ) ) {
        sampleData = t200i53o3fo1f;
        sampleSize = st200i53o3fo1f;

    } else if ( !strcmp(musicName, "t200i98a" ) ) {
        sampleData = t200i98a;
        sampleSize = st200i98a;

    } else if ( !strcmp(musicName, "t200i9o1fa" ) ) {
        sampleData = t200i9o1fa;
        sampleSize = st200i9o1fa;
    }

    sample_pos=0;
    g_samples_to_play=0;

    if (sampleSize < 44)
        return false;

    memcpy(&head, sampleData, 44);

    if (sampleSize != 44 + head.Subchunk2Size)
        return false;
    if (head.NumChannels   != 1 && head.NumChannels   != 2)
        return false;
    if (head.BitsPerSample != 8 && head.BitsPerSample != 16)
        return false;

    bytes_per_sample       = head.NumChannels   * head.BitsPerSample / 8;
    samples_tot            = head.Subchunk2Size / bytes_per_sample;

    rawsamples = sampleData + 44;

    state      = ST_AUTO;
}

void playTune(const std::string &music) {
    playMusic(music);
}

bool retro_load_game(const struct retro_game_info *info)
{

    auto fileLoader = std::make_shared<odb::CPackedFileReader>("audio.pfs");


    i114t1o8f = fileLoader->loadBinaryFileFromPath("i114t1o8f");
    t200i101o3afo1a = fileLoader->loadBinaryFileFromPath("t200i101o3afo1a");
    t200i101o8ao4ao2ao1a = fileLoader->loadBinaryFileFromPath("t200i101o8ao4ao2ao1a");
    t200i52o4defg = fileLoader->loadBinaryFileFromPath("t200i52o4defg");
    t200i53o3fo1f = fileLoader->loadBinaryFileFromPath("t200i53o3fo1f");
    t200i98a = fileLoader->loadBinaryFileFromPath("t200i98a");
    t200i9o1fa = fileLoader->loadBinaryFileFromPath("t200i9o1fa");

    si114t1o8f = fileLoader->sizeOfFile("i114t1o8f");
    st200i101o3afo1a = fileLoader->sizeOfFile("t200i101o3afo1a");
    st200i101o8ao4ao2ao1a = fileLoader->sizeOfFile("t200i101o8ao4ao2ao1a");
    st200i52o4defg = fileLoader->sizeOfFile("t200i52o4defg");
    st200i53o3fo1f = fileLoader->sizeOfFile("t200i53o3fo1f");
    st200i98a = fileLoader->sizeOfFile("t200i98a");
    st200i9o1fa = fileLoader->sizeOfFile("t200i9o1fa");

    state = ST_OFF;

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
    delete[] i114t1o8f; i114t1o8f = nullptr;
    delete[] t200i101o3afo1a; t200i101o3afo1a = nullptr;
    delete[] t200i101o8ao4ao2ao1a; t200i101o8ao4ao2ao1a = nullptr;
    delete[] t200i52o4defg; t200i52o4defg = nullptr;
    delete[] t200i53o3fo1f; t200i53o3fo1f = nullptr;
    delete[] t200i98a; t200i98a = nullptr;
    delete[] t200i9o1fa; t200i9o1fa = nullptr;
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


void setupOPL2(int port) {
}

void stopSounds() {
}

void soundTick() {

}

void muteSound() {

}