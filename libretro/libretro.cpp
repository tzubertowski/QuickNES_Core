#include <libretro.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "Nes_Emu.h"
#include "Data_Reader.h"
#include "abstract_file.h"
#include "Nes_Effects_Buffer.h"
#include "nes_ntsc.h"

#ifdef PSP
#include "pspkernel.h"
#include "pspgu.h"
#endif

#if defined(RENDER_GSKIT_PS2)
#include <libretro_gskit_ps2.h>
#endif

#include "libretro_core_options.h"

#define CORE_VERSION "1.0-WIP"

// Splashscreen font and rendering implementation (adapted from Frodo core)
static const unsigned char font_8x8[][8] = {
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
   {0x3C, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00}, // A
   {0x7C, 0x42, 0x42, 0x7C, 0x42, 0x42, 0x7C, 0x00}, // B
   {0x3C, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3C, 0x00}, // C
   {0x7C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7C, 0x00}, // D
   {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x7E, 0x00}, // E
   {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x00}, // F
   {0x3C, 0x42, 0x40, 0x4E, 0x42, 0x42, 0x3C, 0x00}, // G
   {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00}, // H
   {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}, // I
   {0x0E, 0x04, 0x04, 0x04, 0x04, 0x44, 0x38, 0x00}, // J
   {0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x00}, // K
   {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00}, // L
   {0x42, 0x66, 0x5A, 0x42, 0x42, 0x42, 0x42, 0x00}, // M
   {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x00}, // N
   {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00}, // O
   {0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40, 0x00}, // P
   {0x38, 0x44, 0x44, 0x44, 0x4C, 0x44, 0x3A, 0x00}, // Q
   {0x7C, 0x42, 0x42, 0x7C, 0x48, 0x44, 0x42, 0x00}, // R
   {0x3C, 0x42, 0x40, 0x3C, 0x02, 0x42, 0x3C, 0x00}, // S
   {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // T
   {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00}, // U
   {0x42, 0x42, 0x42, 0x42, 0x24, 0x24, 0x18, 0x00}, // V
   {0x42, 0x42, 0x42, 0x42, 0x5A, 0x66, 0x42, 0x00}, // W
   {0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x42, 0x00}, // X
   {0x42, 0x42, 0x24, 0x18, 0x18, 0x18, 0x18, 0x00}, // Y
   {0x7E, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7E, 0x00}, // Z
   {0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00}, // 0
   {0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00}, // 1
   {0x3C, 0x42, 0x02, 0x0C, 0x30, 0x40, 0x7E, 0x00}, // 2
   {0x3C, 0x42, 0x02, 0x1C, 0x02, 0x42, 0x3C, 0x00}, // 3
   {0x04, 0x0C, 0x14, 0x24, 0x7E, 0x04, 0x04, 0x00}, // 4
   {0x7E, 0x40, 0x7C, 0x02, 0x02, 0x42, 0x3C, 0x00}, // 5
   {0x1C, 0x20, 0x40, 0x7C, 0x42, 0x42, 0x3C, 0x00}, // 6
   {0x7E, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00}, // 7
   {0x3C, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x3C, 0x00}, // 8
   {0x3C, 0x42, 0x42, 0x3E, 0x02, 0x04, 0x38, 0x00}, // 9
   {0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00}, // .
   {0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00}, // /
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // :
   {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00}, // -
   // Lowercase letters a-z
   {0x00, 0x00, 0x3C, 0x02, 0x3E, 0x42, 0x3E, 0x00}, // a
   {0x40, 0x40, 0x7C, 0x42, 0x42, 0x42, 0x7C, 0x00}, // b
   {0x00, 0x00, 0x3C, 0x42, 0x40, 0x42, 0x3C, 0x00}, // c
   {0x02, 0x02, 0x3E, 0x42, 0x42, 0x42, 0x3E, 0x00}, // d
   {0x00, 0x00, 0x3C, 0x42, 0x7E, 0x40, 0x3C, 0x00}, // e
   {0x1C, 0x22, 0x20, 0x7C, 0x20, 0x20, 0x20, 0x00}, // f
   {0x00, 0x00, 0x3E, 0x42, 0x42, 0x3E, 0x02, 0x3C}, // g
   {0x40, 0x40, 0x7C, 0x42, 0x42, 0x42, 0x42, 0x00}, // h
   {0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x1C, 0x00}, // i
   {0x04, 0x00, 0x0C, 0x04, 0x04, 0x44, 0x44, 0x38}, // j
   {0x40, 0x40, 0x42, 0x44, 0x78, 0x44, 0x42, 0x00}, // k
   {0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00}, // l
   {0x00, 0x00, 0x76, 0x4A, 0x4A, 0x4A, 0x4A, 0x00}, // m
   {0x00, 0x00, 0x7C, 0x42, 0x42, 0x42, 0x42, 0x00}, // n
   {0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00}, // o
   {0x00, 0x00, 0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40}, // p
   {0x00, 0x00, 0x3E, 0x42, 0x42, 0x3E, 0x02, 0x02}, // q
   {0x00, 0x00, 0x5C, 0x62, 0x40, 0x40, 0x40, 0x00}, // r
   {0x00, 0x00, 0x3E, 0x40, 0x3C, 0x02, 0x7C, 0x00}, // s
   {0x20, 0x20, 0x7C, 0x20, 0x20, 0x22, 0x1C, 0x00}, // t
   {0x00, 0x00, 0x42, 0x42, 0x42, 0x46, 0x3A, 0x00}, // u
   {0x00, 0x00, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00}, // v
   {0x00, 0x00, 0x42, 0x42, 0x4A, 0x4A, 0x36, 0x00}, // w
   {0x00, 0x00, 0x42, 0x24, 0x18, 0x24, 0x42, 0x00}, // x
   {0x00, 0x00, 0x42, 0x42, 0x42, 0x3E, 0x02, 0x3C}, // y
   {0x00, 0x00, 0x7E, 0x04, 0x18, 0x20, 0x7E, 0x00}, // z
   // Additional special characters
   {0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00}, // +
   {0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00}, // =
};

// Character to font index mapping
static int char_to_font_index(char c) {
   if (c >= 'A' && c <= 'Z') return c - 'A' + 1;          // Uppercase: indices 1-26
   if (c >= '0' && c <= '9') return c - '0' + 27;         // Digits: indices 27-36
   if (c == '.') return 37;                               // Period: index 37
   if (c == '/') return 38;                               // Slash: index 38
   if (c == ':') return 39;                               // Colon: index 39
   if (c == '-') return 40;                               // Dash: index 40
   if (c >= 'a' && c <= 'z') return c - 'a' + 41;        // Lowercase: indices 41-66
   if (c == '+') return 67;                               // Plus: index 67
   if (c == '=') return 68;                               // Equals: index 68
   return 0; // Space for any unsupported character
}

// Draw a character at x, y
static void draw_char(int x, int y, char c, uint16_t *framebuffer, int fb_width, int fb_height, uint16_t color) {
   int font_idx = char_to_font_index(c);
   
   // Bounds check for font array (0-68)
   if (font_idx < 0 || font_idx >= 69) {
      font_idx = 0; // Default to space
   }
   
   const unsigned char *font_data = font_8x8[font_idx];
   
   for (int row = 0; row < 8; row++) {
      unsigned char byte = font_data[row];
      for (int col = 0; col < 8; col++) {
         if (byte & (0x80 >> col)) {
            int px = x + col;
            int py = y + row;
            if (px >= 0 && px < fb_width && py >= 0 && py < fb_height) {
               framebuffer[py * fb_width + px] = color;
            }
         }
      }
   }
}

// Draw a string at x, y
static void draw_string(int x, int y, const char *str, uint16_t *framebuffer, int fb_width, int fb_height, uint16_t color) {
   int curr_x = x;
   while (*str) {
      if (*str == ' ') {
         curr_x += 8;
      } else {
         draw_char(curr_x, y, *str, framebuffer, fb_width, fb_height, color);
         curr_x += 8;
      }
      str++;
   }
}

// Splash screen function for QuickNES
static void draw_splash_screen(uint16_t *framebuffer, int fb_width, int fb_height)
{
   // Background color: Pink #e791bf (RGB 231, 145, 191)
   // Convert to RGB565: R=231>>3=28, G=145>>2=36, B=191>>3=23
   uint16_t bg_color = ((231 >> 3) << 11) | ((145 >> 2) << 5) | (191 >> 3); // RGB565: Pink #e791bf
   uint16_t text_color = 0xFFFF; // White

   // Fill entire screen buffer with pink background
   for (int i = 0; i < fb_width * fb_height; i++) {
      framebuffer[i] = bg_color;
   }

   // Calculate center position for text
   int center_x = fb_width / 2;
   int center_y = fb_height / 2;
   
   // Main title: "QuickNES DASH V" - 60 pixels above center
   char title[] = "QuickNES DASH V";
   int title_width = strlen(title) * 8;
   draw_string(center_x - title_width / 2, center_y - 60, title, framebuffer, fb_width, fb_height, text_color);
   
   // Credits: "MOD BY PROSTY" - 40 pixels above center
   char credits[] = "MOD BY PROSTY";
   int credits_width = strlen(credits) * 8;
   draw_string(center_x - credits_width / 2, center_y - 40, credits, framebuffer, fb_width, fb_height, text_color);
   
   // Rewind instructions: "SEL + L = REWIND" - 10 pixels above center
   char rewind_text[] = "SEL + L = REWIND";
   int rewind_width = strlen(rewind_text) * 8;
   draw_string(center_x - rewind_width / 2, center_y - 10, rewind_text, framebuffer, fb_width, fb_height, text_color);
   
   // Fast forward instructions: "SEL + R = FF" - 10 pixels below center
   char ff_text[] = "SEL + R = FF";
   int ff_width = strlen(ff_text) * 8;
   draw_string(center_x - ff_width / 2, center_y + 10, ff_text, framebuffer, fb_width, fb_height, text_color);
   
   // Slow motion instructions: "SEL + LEFT = SLOW" - 30 pixels below center
   char slow_text[] = "SEL + LEFT = SLOW";
   int slow_width = strlen(slow_text) * 8;
   draw_string(center_x - slow_width / 2, center_y + 30, slow_text, framebuffer, fb_width, fb_height, text_color);
   
   // Discord link: "discord.gg/bvfKkHvsXK" - 50 pixels below center
   char discord[] = "discord.gg/bvfKkHvsXK";
   int discord_width = strlen(discord) * 8;
   draw_string(center_x - discord_width / 2, center_y + 50, discord, framebuffer, fb_width, fb_height, text_color);
   
   // Version date: dynamic compilation date - 50 pixels below center
   char version[] = "compiled " __DATE__;
   int version_width = strlen(version) * 8;
   draw_string(center_x - version_width / 2, center_y + 50, version, framebuffer, fb_width, fb_height, text_color);
}

// SF2000 MIPS optimization: Fast video output
#ifdef SF2000
	#define FAST_VIDEO_OUTPUT __attribute__((always_inline))
#else
	#define FAST_VIDEO_OUTPUT
#endif

#define NES_4_3 (4.0 / 3.0)
#define NES_PAR (width * (8.0 / 7.0) / height)

static Nes_Emu *emu;

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static bool aspect_ratio_par;
#ifdef PSP
static bool use_overscan;
#else
static bool use_overscan_v;
static bool use_overscan_h;
#endif
static bool up_down_allowed = false;
static bool libretro_supports_bitmasks = false;

const int videoBufferWidth = Nes_Emu::image_width + 16;
const int videoBufferHeight = Nes_Emu::image_height + 2;

// SF2000 Rewind Implementation
#define REWIND_BUFFER_SIZE 600  // 10 seconds at 60fps
#define REWIND_GRANULARITY 6    // Save state every 6 frames (10fps)

struct rewind_state {
    uint8_t* data;
    size_t size;
    bool valid;
};

static struct {
    struct rewind_state buffer[REWIND_BUFFER_SIZE];
    int current_index;
    int frame_count;
    bool enabled;
    bool rewinding;
    uint8_t* temp_buffer;
    size_t temp_buffer_size;
} rewind_data = {0};

static bool rewind_button_pressed = false;
static bool rewind_button_held = false;

// SF2000 Fast Forward Variables
static int fast_forward_speed = 1;  // 1x, 2x, 3x speed multiplier
static bool fast_forward_button_pressed = false;
static bool fast_forward_button_held = false;

// SF2000 Slow Motion Variables
static int slow_motion_mode = 0;    // 0=normal, 1=0.7x, 2=0.5x
static bool slow_motion_button_pressed = false;
static bool slow_motion_button_held = false;
static int slow_motion_frame_counter = 0;

// Splashscreen timing variables
static int splashscreen_frame_count = 0;
static bool splashscreen_active = true;

Mono_Buffer mono_buffer;
Nes_Buffer nes_buffer;
Nes_Effects_Buffer effects_buffer;
Silent_Buffer silent_buffer;
Multi_Buffer *current_buffer = NULL;
bool use_silent_buffer = false;

bool is_fast_savestate();

// SF2000 Rewind Functions
static void rewind_init();
static void rewind_deinit();
static void rewind_save_state();
static void rewind_load_state();
static void rewind_check_buttons();
static void fast_forward_check_buttons();
static void slow_motion_check_buttons();

/* ========================================
 * Palette additions START
 * (lifted from libretro-fceumm)
 * ======================================== */

struct st_palettes {
   char name[32];
   char desc[32];
   unsigned data[64];
};

static const struct st_palettes extra_palettes[] = {
   { "asqrealc", "AspiringSquire's Real palette",
      { 0x6c6c6c, 0x00268e, 0x0000a8, 0x400094,
       0x700070, 0x780040, 0x700000, 0x621600,
       0x442400, 0x343400, 0x005000, 0x004444,
       0x004060, 0x000000, 0x101010, 0x101010,
       0xbababa, 0x205cdc, 0x3838ff, 0x8020f0,
       0xc000c0, 0xd01474, 0xd02020, 0xac4014,
       0x7c5400, 0x586400, 0x008800, 0x007468,
       0x00749c, 0x202020, 0x101010, 0x101010,
       0xffffff, 0x4ca0ff, 0x8888ff, 0xc06cff,
       0xff50ff, 0xff64b8, 0xff7878, 0xff9638,
       0xdbab00, 0xa2ca20, 0x4adc4a, 0x2ccca4,
       0x1cc2ea, 0x585858, 0x101010, 0x101010,
       0xffffff, 0xb0d4ff, 0xc4c4ff, 0xe8b8ff,
       0xffb0ff, 0xffb8e8, 0xffc4c4, 0xffd4a8,
       0xffe890, 0xf0f4a4, 0xc0ffc0, 0xacf4f0,
       0xa0e8ff, 0xc2c2c2, 0x202020, 0x101010 }
   },
   { "nintendo-vc", "Virtual Console palette",
      { 0x494949, 0x00006a, 0x090063, 0x290059,
        0x42004a, 0x490000, 0x420000, 0x291100,
        0x182700, 0x003010, 0x003000, 0x002910,
        0x012043, 0x000000, 0x000000, 0x000000,
        0x747174, 0x003084, 0x3101ac, 0x4b0194,
        0x64007b, 0x6b0039, 0x6b2101, 0x5a2f00,
        0x424900, 0x185901, 0x105901, 0x015932,
        0x01495a, 0x101010, 0x000000, 0x000000,
        0xadadad, 0x4a71b6, 0x6458d5, 0x8450e6,
        0xa451ad, 0xad4984, 0xb5624a, 0x947132,
        0x7b722a, 0x5a8601, 0x388e31, 0x318e5a,
        0x398e8d, 0x383838, 0x000000, 0x000000,
        0xb6b6b6, 0x8c9db5, 0x8d8eae, 0x9c8ebc,
        0xa687bc, 0xad8d9d, 0xae968c, 0x9c8f7c,
        0x9c9e72, 0x94a67c, 0x84a77b, 0x7c9d84,
        0x73968d, 0xdedede, 0x000000, 0x000000 }
   },
   { "rgb", "Nintendo RGB PPU palette",
      { 0x6D6D6D, 0x002492, 0x0000DB, 0x6D49DB,
        0x92006D, 0xB6006D, 0xB62400, 0x924900,
        0x6D4900, 0x244900, 0x006D24, 0x009200,
        0x004949, 0x000000, 0x000000, 0x000000,
        0xB6B6B6, 0x006DDB, 0x0049FF, 0x9200FF,
        0xB600FF, 0xFF0092, 0xFF0000, 0xDB6D00,
        0x926D00, 0x249200, 0x009200, 0x00B66D,
        0x009292, 0x242424, 0x000000, 0x000000,
        0xFFFFFF, 0x6DB6FF, 0x9292FF, 0xDB6DFF,
        0xFF00FF, 0xFF6DFF, 0xFF9200, 0xFFB600,
        0xDBDB00, 0x6DDB00, 0x00FF00, 0x49FFDB,
        0x00FFFF, 0x494949, 0x000000, 0x000000,
        0xFFFFFF, 0xB6DBFF, 0xDBB6FF, 0xFFB6FF,
        0xFF92FF, 0xFFB6B6, 0xFFDB92, 0xFFFF49,
        0xFFFF6D, 0xB6FF49, 0x92FF6D, 0x49FFDB,
        0x92DBFF, 0x929292, 0x000000, 0x000000 }
   },
   { "yuv-v3", "FBX's YUV-V3 palette",
      { 0x666666, 0x002A88, 0x1412A7, 0x3B00A4,
        0x5C007E, 0x6E0040, 0x6C0700, 0x561D00,
        0x333500, 0x0C4800, 0x005200, 0x004C18,
        0x003E5B, 0x000000, 0x000000, 0x000000,
        0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE,
        0xA01ACC, 0xB71E7B, 0xB53120, 0x994E00,
        0x6B6D00, 0x388700, 0x0D9300, 0x008C47,
        0x007AA0, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x64B0FF, 0x9290FF, 0xC676FF,
        0xF26AFF, 0xFF6ECC, 0xFF8170, 0xEA9E22,
        0xBCBE00, 0x88D800, 0x5CE430, 0x45E082,
        0x48CDDE, 0x4F4F4F, 0x000000, 0x000000,
        0xFFFFFF, 0xC0DFFF, 0xD3D2FF, 0xE8C8FF,
        0xFAC2FF, 0xFFC4EA, 0xFFCCC5, 0xF7D8A5,
        0xE4E594, 0xCFEF96, 0xBDF4AB, 0xB3F3CC,
        0xB5EBF2, 0xB8B8B8, 0x000000, 0x000000 }
   },
   { "unsaturated-final", "FBX's Unsaturated-Final palette",
      { 0x676767, 0x001F8E, 0x23069E, 0x40008E,
        0x600067, 0x67001C, 0x5B1000, 0x432500,
        0x313400, 0x074800, 0x004F00, 0x004622,
        0x003A61, 0x000000, 0x000000, 0x000000,
        0xB3B3B3, 0x205ADF, 0x5138FB, 0x7A27EE,
        0xA520C2, 0xB0226B, 0xAD3702, 0x8D5600,
        0x6E7000, 0x2E8A00, 0x069200, 0x008A47,
        0x037B9B, 0x101010, 0x000000, 0x000000,
        0xFFFFFF, 0x62AEFF, 0x918BFF, 0xBC78FF,
        0xE96EFF, 0xFC6CCD, 0xFA8267, 0xE29B26,
        0xC0B901, 0x84D200, 0x58DE38, 0x46D97D,
        0x49CED2, 0x494949, 0x000000, 0x000000,
        0xFFFFFF, 0xC1E3FF, 0xD5D4FF, 0xE7CCFF,
        0xFBC9FF, 0xFFC7F0, 0xFFD0C5, 0xF8DAAA,
        0xEBE69A, 0xD1F19A, 0xBEF7AF, 0xB6F4CD,
        0xB7F0EF, 0xB2B2B2, 0x000000, 0x000000 }
   },
   { "sony-cxa2025as-us", "Sony CXA2025AS US palette",
      { 0x585858, 0x00238C, 0x00139B, 0x2D0585,
        0x5D0052, 0x7A0017, 0x7A0800, 0x5F1800,
        0x352A00, 0x093900, 0x003F00, 0x003C22,
        0x00325D, 0x000000, 0x000000, 0x000000,
        0xA1A1A1, 0x0053EE, 0x153CFE, 0x6028E4,
        0xA91D98, 0xD41E41, 0xD22C00, 0xAA4400,
        0x6C5E00, 0x2D7300, 0x007D06, 0x007852,
        0x0069A9, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x1FA5FE, 0x5E89FE, 0xB572FE,
        0xFE65F6, 0xFE6790, 0xFE773C, 0xFE9308,
        0xC4B200, 0x79CA10, 0x3AD54A, 0x11D1A4,
        0x06BFFE, 0x424242, 0x000000, 0x000000,
        0xFFFFFF, 0xA0D9FE, 0xBDCCFE, 0xE1C2FE,
        0xFEBCFB, 0xFEBDD0, 0xFEC5A9, 0xFED18E,
        0xE9DE86, 0xC7E992, 0xA8EEB0, 0x95ECD9,
        0x91E4FE, 0xACACAC, 0x000000, 0x000000 }
   },
   { "pal", "PAL palette",
      { 0x808080, 0x0000BA, 0x3700BF, 0x8400A6,
        0xBB006A, 0xB7001E, 0xB30000, 0x912600,
        0x7B2B00, 0x003E00, 0x00480D, 0x003C22,
        0x002F66, 0x000000, 0x050505, 0x050505,
        0xC8C8C8, 0x0059FF, 0x443CFF, 0xB733CC,
        0xFE33AA, 0xFE375E, 0xFE371A, 0xD54B00,
        0xC46200, 0x3C7B00, 0x1D8415, 0x009566,
        0x0084C4, 0x111111, 0x090909, 0x090909,
        0xFEFEFE, 0x0095FF, 0x6F84FF, 0xD56FFF,
        0xFE77CC, 0xFE6F99, 0xFE7B59, 0xFE915F,
        0xFEA233, 0xA6BF00, 0x51D96A, 0x4DD5AE,
        0x00D9FF, 0x666666, 0x0D0D0D, 0x0D0D0D,
        0xFEFEFE, 0x84BFFF, 0xBBBBFF, 0xD0BBFF,
        0xFEBFEA, 0xFEBFCC, 0xFEC4B7, 0xFECCAE,
        0xFED9A2, 0xCCE199, 0xAEEEB7, 0xAAF8EE,
        0xB3EEFF, 0xDDDDDD, 0x111111, 0x111111 }
   },
   { "bmf-final2", "BMF's Final 2 palette",
      { 0x525252, 0x000080, 0x08008A, 0x2C007E,
        0x4A004E, 0x500006, 0x440000, 0x260800,
        0x0A2000, 0x002E00, 0x003200, 0x00260A,
        0x001C48, 0x000000, 0x000000, 0x000000,
        0xA4A4A4, 0x0038CE, 0x3416EC, 0x5E04DC,
        0x8C00B0, 0x9A004C, 0x901800, 0x703600,
        0x4C5400, 0x0E6C00, 0x007400, 0x006C2C,
        0x005E84, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x4C9CFF, 0x7C78FF, 0xA664FF,
        0xDA5AFF, 0xF054C0, 0xF06A56, 0xD68610,
        0xBAA400, 0x76C000, 0x46CC1A, 0x2EC866,
        0x34C2BE, 0x3A3A3A, 0x000000, 0x000000,
        0xFFFFFF, 0xB6DAFF, 0xC8CAFF, 0xDAC2FF,
        0xF0BEFF, 0xFCBCEE, 0xFAC2C0, 0xF2CCA2,
        0xE6DA92, 0xCCE68E, 0xB8EEA2, 0xAEEABE,
        0xAEE8E2, 0xB0B0B0, 0x000000, 0x000000 }
   },
   { "bmf-final3", "BMF's Final 3 palette",
      { 0x686868, 0x001299, 0x1A08AA, 0x51029A,
        0x7E0069, 0x8E001C, 0x7E0301, 0x511800,
        0x1F3700, 0x014E00, 0x005A00, 0x00501C,
        0x004061, 0x000000, 0x000000, 0x000000,
        0xB9B9B9, 0x0C5CD7, 0x5035F0, 0x8919E0,
        0xBB0CB3, 0xCE0C61, 0xC02B0E, 0x954D01,
        0x616F00, 0x1F8B00, 0x01980C, 0x00934B,
        0x00819B, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x63B4FF, 0x9B91FF, 0xD377FF,
        0xEF6AFF, 0xF968C0, 0xF97D6C, 0xED9B2D,
        0xBDBD16, 0x7CDA1C, 0x4BE847, 0x35E591,
        0x3FD9DD, 0x606060, 0x000000, 0x000000,
        0xFFFFFF, 0xACE7FF, 0xD5CDFF, 0xEDBAFF,
        0xF8B0FF, 0xFEB0EC, 0xFDBDB5, 0xF9D28E,
        0xE8EB7C, 0xBBF382, 0x99F7A2, 0x8AF5D0,
        0x92F4F1, 0xBEBEBE, 0x000000, 0x000000 }
   },
   { "smooth-fbx", "FBX's Smooth palette",
      { 0x6A6D6A, 0x001380, 0x1E008A, 0x39007A,
        0x550056, 0x5A0018, 0x4F1000, 0x3D1C00,
        0x253200, 0x003D00, 0x004000, 0x003924,
        0x002E55, 0x000000, 0x000000, 0x000000,
        0xB9BCB9, 0x1850C7, 0x4B30E3, 0x7322D6,
        0x951FA9, 0x9D285C, 0x983700, 0x7F4C00,
        0x5E6400, 0x227700, 0x027E02, 0x007645,
        0x006E8A, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x68A6FF, 0x8C9CFF, 0xB586FF,
        0xD975FD, 0xE377B9, 0xE58D68, 0xD49D29,
        0xB3AF0C, 0x7BC211, 0x55CA47, 0x46CB81,
        0x47C1C5, 0x4A4D4A, 0x000000, 0x000000,
        0xFFFFFF, 0xCCEAFF, 0xDDDEFF, 0xECDAFF,
        0xF8D7FE, 0xFCD6F5, 0xFDDBCF, 0xF9E7B5,
        0xF1F0AA, 0xDAFAA9, 0xC9FFBC, 0xC3FBD7,
        0xC4F6F6, 0xBEC1BE, 0x000000, 0x000000 }
   },
   { "composite-direct-fbx", "FBX's Composite Direct palette",
      { 0x656565, 0x00127D, 0x18008E, 0x360082,
        0x56005D, 0x5A0018, 0x4F0500, 0x381900,
        0x1D3100, 0x003D00, 0x004100, 0x003B17,
        0x002E55, 0x000000, 0x000000, 0x000000,
        0xAFAFAF, 0x194EC8, 0x472FE3, 0x6B1FD7,
        0x931BAE, 0x9E1A5E, 0x993200, 0x7B4B00,
        0x5B6700, 0x267A00, 0x008200, 0x007A3E,
        0x006E8A, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x64A9FF, 0x8E89FF, 0xB676FF,
        0xE06FFF, 0xEF6CC4, 0xF0806A, 0xD8982C,
        0xB9B40A, 0x83CB0C, 0x5BD63F, 0x4AD17E,
        0x4DC7CB, 0x4C4C4C, 0x000000, 0x000000,
        0xFFFFFF, 0xC7E5FF, 0xD9D9FF, 0xE9D1FF,
        0xF9CEFF, 0xFFCCF1, 0xFFD4CB, 0xF8DFB1,
        0xEDEAA4, 0xD6F4A4, 0xC5F8B8, 0xBEF6D3,
        0xBFF1F1, 0xB9B9B9, 0x000000, 0x000000 }
   },
   { "pvm-style-d93-fbx", "FBX's PVM Style D93 palette",
      { 0x696B63, 0x001774, 0x1E0087, 0x340073,
        0x560057, 0x5E0013, 0x531A00, 0x3B2400,
        0x243000, 0x063A00, 0x003F00, 0x003B1E,
        0x00334E, 0x000000, 0x000000, 0x000000,
        0xB9BBB3, 0x1453B9, 0x4D2CDA, 0x671EDE,
        0x98189C, 0x9D2344, 0xA03E00, 0x8D5500,
        0x656D00, 0x2C7900, 0x008100, 0x007D42,
        0x00788A, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x69A8FF, 0x9691FF, 0xB28AFA,
        0xEA7DFA, 0xF37BC7, 0xF28E59, 0xE6AD27,
        0xD7C805, 0x90DF07, 0x64E53C, 0x45E27D,
        0x48D5D9, 0x4E5048, 0x000000, 0x000000,
        0xFFFFFF, 0xD2EAFF, 0xE2E2FF, 0xE9D8FF,
        0xF5D2FF, 0xF8D9EA, 0xFADEB9, 0xF9E89B,
        0xF3F28C, 0xD3FA91, 0xB8FCA8, 0xAEFACA,
        0xCAF3F3, 0xBEC0B8, 0x000000, 0x000000 }
   },
   { "ntsc-hardware-fbx", "FBX's NTSC Hardware palette",
      { 0x6A6D6A, 0x001380, 0x1E008A, 0x39007A,
        0x550056, 0x5A0018, 0x4F1000, 0x382100,
        0x213300, 0x003D00, 0x004000, 0x003924,
        0x002E55, 0x000000, 0x000000, 0x000000,
        0xB9BCB9, 0x1850C7, 0x4B30E3, 0x7322D6,
        0x951FA9, 0x9D285C, 0x963C00, 0x7A5100,
        0x5B6700, 0x227700, 0x027E02, 0x007645,
        0x006E8A, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x68A6FF, 0x9299FF, 0xB085FF,
        0xD975FD, 0xE377B9, 0xE58D68, 0xCFA22C,
        0xB3AF0C, 0x7BC211, 0x55CA47, 0x46CB81,
        0x47C1C5, 0x4A4D4A, 0x000000, 0x000000,
        0xFFFFFF, 0xCCEAFF, 0xDDDEFF, 0xECDAFF,
        0xF8D7FE, 0xFCD6F5, 0xFDDBCF, 0xF9E7B5,
        0xF1F0AA, 0xDAFAA9, 0xC9FFBC, 0xC3FBD7,
        0xC4F6F6, 0xBEC1BE, 0x000000, 0x000000 }
   },
   { "nes-classic-fbx-fs", "FBX's NES-Classic FS palette",
      { 0x60615F, 0x000083, 0x1D0195, 0x340875,
        0x51055E, 0x56000F, 0x4C0700, 0x372308,
        0x203A0B, 0x0F4B0E, 0x194C16, 0x02421E,
        0x023154, 0x000000, 0x000000, 0x000000,
        0xA9AAA8, 0x104BBF, 0x4712D8, 0x6300CA,
        0x8800A9, 0x930B46, 0x8A2D04, 0x6F5206,
        0x5C7114, 0x1B8D12, 0x199509, 0x178448,
        0x206B8E, 0x000000, 0x000000, 0x000000,
        0xFBFBFB, 0x6699F8, 0x8974F9, 0xAB58F8,
        0xD557EF, 0xDE5FA9, 0xDC7F59, 0xC7A224,
        0xA7BE03, 0x75D703, 0x60E34F, 0x3CD68D,
        0x56C9CC, 0x414240, 0x000000, 0x000000,
        0xFBFBFB, 0xBED4FA, 0xC9C7F9, 0xD7BEFA,
        0xE8B8F9, 0xF5BAE5, 0xF3CAC2, 0xDFCDA7,
        0xD9E09C, 0xC9EB9E, 0xC0EDB8, 0xB5F4C7,
        0xB9EAE9, 0xABABAB, 0x000000, 0x000000 }
   },
   { "nescap", "RGBSource's NESCAP palette",
      { 0x646365, 0x001580, 0x1D0090, 0x380082,
        0x56005D, 0x5A001A, 0x4F0900, 0x381B00,
        0x1E3100, 0x003D00, 0x004100, 0x003A1B,
        0x002F55, 0x000000, 0x000000, 0x000000,
        0xAFADAF, 0x164BCA, 0x472AE7, 0x6B1BDB,
        0x9617B0, 0x9F185B, 0x963001, 0x7B4800,
        0x5A6600, 0x237800, 0x017F00, 0x00783D,
        0x006C8C, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x60A6FF, 0x8F84FF, 0xB473FF,
        0xE26CFF, 0xF268C3, 0xEF7E61, 0xD89527,
        0xBAB307, 0x81C807, 0x57D43D, 0x47CF7E,
        0x4BC5CD, 0x4C4B4D, 0x000000, 0x000000,
        0xFFFFFF, 0xC2E0FF, 0xD5D2FF, 0xE3CBFF,
        0xF7C8FF, 0xFEC6EE, 0xFECEC6, 0xF6D7AE,
        0xE9E49F, 0xD3ED9D, 0xC0F2B2, 0xB9F1CC,
        0xBAEDED, 0xBAB9BB, 0x000000, 0x000000 }
   },
   { "wavebeam", "nakedarthur's Wavebeam palette",
      { 0X6B6B6B, 0X001B88, 0X21009A, 0X40008C,
        0X600067, 0X64001E, 0X590800, 0X481600,
        0X283600, 0X004500, 0X004908, 0X00421D,
        0X003659, 0X000000, 0X000000, 0X000000,
        0XB4B4B4, 0X1555D3, 0X4337EF, 0X7425DF,
        0X9C19B9, 0XAC0F64, 0XAA2C00, 0X8A4B00,
        0X666B00, 0X218300, 0X008A00, 0X008144,
        0X007691, 0X000000, 0X000000, 0X000000,
        0XFFFFFF, 0X63B2FF, 0X7C9CFF, 0XC07DFE,
        0XE977FF, 0XF572CD, 0XF4886B, 0XDDA029,
        0XBDBD0A, 0X89D20E, 0X5CDE3E, 0X4BD886,
        0X4DCFD2, 0X525252, 0X000000, 0X000000,
        0XFFFFFF, 0XBCDFFF, 0XD2D2FF, 0XE1C8FF,
        0XEFC7FF, 0XFFC3E1, 0XFFCAC6, 0XF2DAAD,
        0XEBE3A0, 0XD2EDA2, 0XBCF4B4, 0XB5F1CE,
        0XB6ECF1, 0XBFBFBF, 0X000000, 0X000000 }
   }
};

#define NUM_EXTRA_PALETTES 16
#define NUM_PALETTES_TOTAL (NUM_EXTRA_PALETTES + 1)

// 0: default palette (nes_colors)
// 1->(NUM_EXTRA_PALETTES+1): extra palettes
static int palette_index = 0;
static int last_palette_index = -1;

// Required by nes_ntsc
static unsigned char base_palette[64 * 3];
static unsigned char palette_out[nes_ntsc_palette_size * 3];
static nes_ntsc_setup_t nes_ntsc_setup = nes_ntsc_rgb; // (initial preset doesn't matter)

static Nes_Emu::rgb_t current_nes_colors[Nes_Emu::color_table_size];

/* Palette switching */
/*********************/

/* Period in frames between palette switches
 * when holding RetroPad L/R */
#define PALETTE_SWITCH_PERIOD 30

static bool libretro_supports_set_variable            = false;
static unsigned libretro_msg_interface_version        = 0;
static unsigned palette_switch_counter                = 0;
struct retro_core_option_value *palette_opt_values    = NULL;
static const char *palette_labels[NUM_PALETTES_TOTAL] = {0};

static void palette_switch_init(void)
{
   size_t i;
   struct retro_core_option_v2_definition *opt_defs      = option_defs_us;
   struct retro_core_option_v2_definition *opt_def       = NULL;
#ifndef HAVE_NO_LANGEXTRA
   struct retro_core_option_v2_definition *opt_defs_intl = NULL;
   struct retro_core_option_v2_definition *opt_def_intl  = NULL;
   unsigned language                                     = 0;
#endif

   libretro_supports_set_variable = false;
   if (environ_cb(RETRO_ENVIRONMENT_SET_VARIABLE, NULL))
      libretro_supports_set_variable = true;

   libretro_msg_interface_version = 0;
   environ_cb(RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION,
         &libretro_msg_interface_version);

   palette_switch_counter  = 0;

#ifndef HAVE_NO_LANGEXTRA
   if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
       (language < RETRO_LANGUAGE_LAST) &&
       (language != RETRO_LANGUAGE_ENGLISH) &&
       options_intl[language])
      opt_defs_intl = options_intl[language]->definitions;
#endif

   /* Find option corresponding to palettes key */
   for (opt_def = opt_defs; opt_def->key; opt_def++)
      if (!strcmp(opt_def->key, "quicknes_palette"))
         break;

   /* Cache option values array for fast access
    * when setting palette index */
   palette_opt_values = opt_def->values;

   /* Loop over all palette values and fetch
    * palette labels for notification purposes */
   for (i = 0; i < NUM_PALETTES_TOTAL; i++)
   {
      const char *value       = opt_def->values[i].value;
      const char *value_label = NULL;

      /* Check if we have a localised palette label */
#ifndef HAVE_NO_LANGEXTRA
      if (opt_defs_intl)
      {
         /* Find localised option corresponding to key */
         for (opt_def_intl = opt_defs_intl;
              opt_def_intl->key;
              opt_def_intl++)
         {
            if (!strcmp(opt_def_intl->key, "quicknes_palette"))
            {
               size_t j = 0;

               /* Search for current option value */
               for (;;)
               {
                  const char *value_intl = opt_def_intl->values[j].value;

                  if (!value_intl)
                     break;

                  if (!strcmp(value, value_intl))
                  {
                     /* We have a match; fetch localised label */
                     value_label = opt_def_intl->values[j].label;
                     break;
                  }

                  j++;
               }

               break;
            }
         }
      }
#endif
      /* If localised palette label is unset,
       * use label from option_defs_us or fallback
       * to value itself */
      if (!value_label)
         value_label = opt_def->values[i].label;
      if (!value_label)
         value_label = value;

      palette_labels[i] = value_label;
   }
}

static void palette_switch_deinit(void)
{
   libretro_supports_set_variable = false;
   libretro_msg_interface_version = 0;
   palette_switch_counter         = 0;
   palette_opt_values             = NULL;
}

static void palette_switch_set_index(int palette_index)
{
   struct retro_variable var = {0};

   if (palette_index >= NUM_PALETTES_TOTAL)
      palette_index = NUM_PALETTES_TOTAL - 1;
   if (palette_index < 0)
      palette_index = 0;

   /* Notify frontend of option value changes */
   var.key   = "quicknes_palette";
   var.value = palette_opt_values[palette_index].value;
   environ_cb(RETRO_ENVIRONMENT_SET_VARIABLE, &var);

   /* Display notification message */
   if (libretro_msg_interface_version >= 1)
   {
      struct retro_message_ext msg = {
         palette_labels[palette_index],
         2000,
         1,
         RETRO_LOG_INFO,
         RETRO_MESSAGE_TARGET_OSD,
         RETRO_MESSAGE_TYPE_NOTIFICATION_ALT,
         -1
      };
      environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE_EXT, &msg);
   }
   else
   {
      struct retro_message msg = {
         palette_labels[palette_index],
         120
      };
      environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &msg);
   }
}

/* ========================================
 * Palette additions END
 * ======================================== */

#define MAX_PLAYERS 2
#define NUM_TURBO_BUTTONS 2

static unsigned char turbo_enabled[MAX_PLAYERS] = {0};
static unsigned turbo_pulse_width = 0;
static unsigned char turbo_counter[MAX_PLAYERS][NUM_TURBO_BUTTONS] = { {0} };
static unsigned char turbo_state[MAX_PLAYERS][NUM_TURBO_BUTTONS] = { {1} };

void retro_init(void)
{
   // Assign nes_ntsc setup pointers
   // (Seems the best place to do it...)
   nes_ntsc_setup.base_palette = base_palette;
   nes_ntsc_setup.palette_out = palette_out;

   palette_switch_init();

   if (environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL))
      libretro_supports_bitmasks = true;

   // SF2000 Rewind: Initialize rewind system
   rewind_init();
}

void retro_deinit(void)
{
   libretro_supports_bitmasks = false;

   palette_switch_deinit();
   
   // SF2000 Rewind: Cleanup rewind system
   rewind_deinit();
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned, unsigned)
{
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "QuickNES";
#ifdef GIT_VERSION
   info->library_version  = CORE_VERSION GIT_VERSION;
#else
   info->library_version  = CORE_VERSION;
#endif
   info->need_fullpath    = false;
   info->valid_extensions = "nes"; // Anything is fine, we don't care.
}

float get_aspect_ratio(unsigned width, unsigned height)
{
   return (aspect_ratio_par ? NES_PAR : NES_4_3);
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
#ifdef PSP
   unsigned width  = Nes_Emu::image_width  - (use_overscan   ? 0 : 16);
   unsigned height = Nes_Emu::image_height - (use_overscan   ? 0 : 16);
#else
   unsigned width  = Nes_Emu::image_width  - (use_overscan_h ? 0 : 16);
   unsigned height = Nes_Emu::image_height - (use_overscan_v ? 0 : 16);
#endif

   const retro_system_timing timing = { Nes_Emu::frame_rate, 44100.0 };
   info->timing = timing;

   info->geometry.base_width   = width;
   info->geometry.base_height  = height;
   info->geometry.max_width    = Nes_Emu::image_width;
   info->geometry.max_height   = Nes_Emu::image_height;
   info->geometry.aspect_ratio = get_aspect_ratio(width, height);
}

void retro_set_environment(retro_environment_t cb)
{
   bool categories_supported;

   environ_cb = cb;
   libretro_set_core_options(environ_cb,
      &categories_supported);
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

void retro_reset(void)
{
   if (emu)
      emu->reset( false, false );
}

static void set_current_palette(void)
{
   if (palette_index <= 0 || palette_index > NUM_EXTRA_PALETTES)
   {
      // Copy default internal palette into current_nes_colors
      for (unsigned i = 0; i < Nes_Emu::color_table_size; i++)
      {
         const Nes_Emu::rgb_t &rgb_in = emu->nes_colors[i];
         Nes_Emu::rgb_t &rgb_out = current_nes_colors[i];
         rgb_out.red = rgb_in.red;
         rgb_out.green = rgb_in.green;
         rgb_out.blue = rgb_in.blue;
      }
   }
   else
   {
      // Copy external palette into base_palette buffer
      const unsigned *palette_data = extra_palettes[palette_index - 1].data;
      unsigned counter = 0;
      for (unsigned i = 0; i < 64; i++)
      {
         unsigned rgb = palette_data[i];
         base_palette[counter] = (char)((rgb >> 16) & 0xFF);
         base_palette[counter + 1] = (char)((rgb >> 8) & 0xFF);
         base_palette[counter + 2] = (char)(rgb & 0xFF);
         counter += 3;
      }
      // Generate complete palette data set (including emphasis modes)
      nes_ntsc_init(0, &nes_ntsc_setup);
      // Copy generated palette into current_nes_colors
      counter = 0;
      for (unsigned i = 0; i < Nes_Emu::color_table_size; i++)
      {
         Nes_Emu::rgb_t &rgb = current_nes_colors[i];
         rgb.red = (unsigned char)palette_out[counter];
         rgb.green = (unsigned char)palette_out[counter + 1];
         rgb.blue = (unsigned char)palette_out[counter + 2];
         counter += 3;
      }
   }
   
   last_palette_index = palette_index;
}

static void update_audio_mode(void)
{
	if (use_silent_buffer)
	{
		emu->set_sample_rate(44100, &silent_buffer);
		current_buffer = &silent_buffer;
		return;
	}
	struct retro_variable var = { 0 };

	var.key = "quicknes_audio_nonlinear";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	{
		if (0 == strcmp(var.value, "nonlinear"))
		{
			if (current_buffer != &nes_buffer)
			{
				emu->set_sample_rate(44100, &nes_buffer);
				current_buffer = &nes_buffer;
			}
		}
		else if (0 == strcmp(var.value, "stereo panning"))
		{
			if (current_buffer != &effects_buffer)
			{
				emu->set_sample_rate(44100, &effects_buffer);
				current_buffer = &effects_buffer;
			}

			Effects_Buffer::config_t c;
			c.pan_1             = -0.6f; // no full panning
			c.pan_2             =  0.6f;
			c.delay_variance    =  18.0f;
			c.reverb_delay      =  88.0f;
			c.echo_delay        =  61.0;
			c.reverb_level      =  0.2f; // adds a bit of "depth" instead of just being dry for each channel
			c.echo_level        =  0.2f;
			c.effects_enabled   =  1;
			effects_buffer.config( c );
		}
		else
		{
			if (current_buffer != &mono_buffer)
			{
				emu->set_sample_rate(44100, &mono_buffer);
				current_buffer = &mono_buffer;
			}
		}
	}
	else
	{
		//if the environment callback failed (won't happen), just set the nonlinear buffer
		if (current_buffer != &nes_buffer)
		{
			emu->set_sample_rate(44100, &nes_buffer);
			current_buffer = &nes_buffer;
		}
	}

	var.key   = "quicknes_audio_eq";
   var.value = NULL;

	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	{
		if (0 == strcmp(var.value, "default"))
			emu->set_equalizer(Nes_Emu::nes_eq);
		else if (0 == strcmp(var.value, "nes"))
			emu->set_equalizer(Nes_Emu::nes_eq);
		else if (0 == strcmp(var.value, "famicom"))
			emu->set_equalizer(Nes_Emu::famicom_eq);
		else if (0 == strcmp(var.value, "tv"))
			emu->set_equalizer(Nes_Emu::tv_eq);
		else if (0 == strcmp(var.value, "flat"))
			emu->set_equalizer(Nes_Emu::flat_eq);
		else if (0 == strcmp(var.value, "crisp"))
			emu->set_equalizer(Nes_Emu::crisp_eq);
		else if (0 == strcmp(var.value, "tinny"))
			emu->set_equalizer(Nes_Emu::tinny_eq);
		else
			emu->set_equalizer(Nes_Emu::nes_eq);
	}
	else
	{
		//if the environment callback failed (won't happen), just set the default NES equalizer
		emu->set_equalizer(Nes_Emu::nes_eq);
	}
}

static void check_variables(void)
{
   struct retro_variable var = {0};
   bool video_changed = false;

   var.key = "quicknes_no_sprite_limit";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         emu->set_sprite_mode( Nes_Emu::sprites_enhanced);
      else
         emu->set_sprite_mode( Nes_Emu::sprites_visible);
   }

   var.key = "quicknes_aspect_ratio_par";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      bool newval = (!strcmp(var.value, "PAR"));
      if (newval != aspect_ratio_par)
      {
         aspect_ratio_par = newval;
         video_changed = true;
      }
   }

   var.key = "quicknes_up_down_allowed";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      up_down_allowed = (!strcmp(var.value, "enabled")) ? true : false;
   }
   else
      up_down_allowed = false;


#ifndef PSP
   var.key = "quicknes_use_overscan_h";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      bool newval = (!strcmp(var.value, "enabled"));
      if (newval != use_overscan_h)
      {
         use_overscan_h = newval;
         video_changed = true;
      }
   }

   var.key = "quicknes_use_overscan_v";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      bool newval = (!strcmp(var.value, "enabled"));
      if (newval != use_overscan_v)
      {
         use_overscan_v = newval;
         video_changed = true;
      }
   }
#endif

   update_audio_mode();

   var.key = "quicknes_palette";
   palette_index = 0;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "default")) {
         palette_index = 0;
      } else if (!strcmp(var.value, "asqrealc")) {
         palette_index = 1;
      } else if (!strcmp(var.value, "nintendo-vc")) {
         palette_index = 2;
      } else if (!strcmp(var.value, "rgb")) {
         palette_index = 3;
      } else if (!strcmp(var.value, "yuv-v3")) {
         palette_index = 4;
      } else if (!strcmp(var.value, "unsaturated-final")) {
         palette_index = 5;
      } else if (!strcmp(var.value, "sony-cxa2025as-us")) {
         palette_index = 6;
      } else if (!strcmp(var.value, "pal")) {
         palette_index = 7;
      } else if (!strcmp(var.value, "bmf-final2")) {
         palette_index = 8;
      } else if (!strcmp(var.value, "bmf-final3")) {
         palette_index = 9;
      } else if (!strcmp(var.value, "smooth-fbx")) {
         palette_index = 10;
      } else if (!strcmp(var.value, "composite-direct-fbx")) {
         palette_index = 11;
      } else if (!strcmp(var.value, "pvm-style-d93-fbx")) {
         palette_index = 12;
      } else if (!strcmp(var.value, "ntsc-hardware-fbx")) {
         palette_index = 13;
      } else if (!strcmp(var.value, "nes-classic-fbx-fs")) {
         palette_index = 14;
      } else if (!strcmp(var.value, "nescap")) {
         palette_index = 15;
      } else if (!strcmp(var.value, "wavebeam")) {
         palette_index = 16;
      }
   }
   if (palette_index != last_palette_index) {
      set_current_palette();
   }

   var.key = "quicknes_turbo_enable";
   for (unsigned i = 0; i < MAX_PLAYERS; i++) { turbo_enabled[i] = 0; }

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "player 1")) {
         turbo_enabled[0] = 1;
      } else if (!strcmp(var.value, "player 2")) {
         turbo_enabled[1] = 1;
      } else if (!strcmp(var.value, "both")) {
         for (unsigned i = 0; i < MAX_PLAYERS; i++) { turbo_enabled[i] = 1; }
      }
   }

   var.key = "quicknes_turbo_pulse_width";
   turbo_pulse_width = 0;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      turbo_pulse_width = atoi(var.value);
   }

   // This is also a good place to reset the turbo
   // counter/state arrays...
   for (unsigned i = 0; i < MAX_PLAYERS; i++) {
      for (unsigned j = 0; j < NUM_TURBO_BUTTONS; j++) {
         turbo_counter[i][j] = 0;
         turbo_state[i][j] = 1;
         // 'state' starts at 1 -> turbo engaged as soon as button is pressed
      }
   }

   if (video_changed)
   {
      struct retro_system_av_info info;
      retro_get_system_av_info(&info);
      environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &info);
   }
}

#define JOY_A           1
#define JOY_B           2
#define JOY_SELECT      4
#define JOY_START       8
#define JOY_UP       0x10
#define JOY_DOWN     0x20
#define JOY_LEFT     0x40
#define JOY_RIGHT    0x80

typedef struct
{
   unsigned retro;
   unsigned nes;
} keymap;

static const keymap bindmap[] = {
   { RETRO_DEVICE_ID_JOYPAD_A, JOY_A },
   { RETRO_DEVICE_ID_JOYPAD_B, JOY_B },
   { RETRO_DEVICE_ID_JOYPAD_SELECT, JOY_SELECT },
   { RETRO_DEVICE_ID_JOYPAD_START, JOY_START },
   { RETRO_DEVICE_ID_JOYPAD_UP, JOY_UP },
   { RETRO_DEVICE_ID_JOYPAD_DOWN, JOY_DOWN },
   { RETRO_DEVICE_ID_JOYPAD_LEFT, JOY_LEFT },
   { RETRO_DEVICE_ID_JOYPAD_RIGHT, JOY_RIGHT },
};

static const keymap turbomap[] = {
   { RETRO_DEVICE_ID_JOYPAD_X, JOY_A },
   { RETRO_DEVICE_ID_JOYPAD_Y, JOY_B },
};

static void update_input(int pads[MAX_PLAYERS])
{
   unsigned p;
   bool palette_prev = false;
   bool palette_next = false;

   pads[0] = pads[1] = 0;
   input_poll_cb();

   for (p = 0; p < MAX_PLAYERS; p++)
   {
      unsigned bind;
      bool turbo_btn[2];

      if (libretro_supports_bitmasks)
      {
         int16_t ret = input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
         for (bind = 0; bind < sizeof(bindmap) / sizeof(bindmap[0]); bind++)
            if (ret & (1 << bindmap[bind].retro))
               pads[p] |= bindmap[bind].nes;
         turbo_btn[0] = (ret & (1 << RETRO_DEVICE_ID_JOYPAD_X));
         turbo_btn[1] = (ret & (1 << RETRO_DEVICE_ID_JOYPAD_Y));

         if (libretro_supports_set_variable && (p == 0))
         {
            palette_prev = (bool)(ret & (1 << RETRO_DEVICE_ID_JOYPAD_L));
            palette_next = (bool)(ret & (1 << RETRO_DEVICE_ID_JOYPAD_R));
         }

         // SF2000 Rewind: Check for SELECT + L button combination
         if (p == 0) {
            bool select_pressed = (ret & (1 << RETRO_DEVICE_ID_JOYPAD_SELECT));
            bool l_pressed = (ret & (1 << RETRO_DEVICE_ID_JOYPAD_L));
            bool r_pressed = (ret & (1 << RETRO_DEVICE_ID_JOYPAD_R));
            rewind_button_pressed = select_pressed && l_pressed;
            fast_forward_button_pressed = select_pressed && r_pressed;
         }
      }
      else
      {
         for (bind = 0; bind < sizeof(bindmap) / sizeof(bindmap[0]); bind++)
            pads[p] |= input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, bindmap[bind].retro) ? bindmap[bind].nes : 0;
         if (turbo_enabled[p])
         {
            turbo_btn[0] = input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X);
            turbo_btn[1] = input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
         }

         if (libretro_supports_set_variable && (p == 0))
         {
            palette_prev = input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
            palette_next = input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
         }

         // SF2000 Rewind: Check for SELECT + L button combination
         // SF2000 Fast Forward: Check for SELECT + R button combination
         // SF2000 Slow Motion: Check for SELECT + LEFT button combination
         if (p == 0) {
            bool select_pressed = input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT);
            bool l_pressed = input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
            bool r_pressed = input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
            bool left_pressed = input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
            rewind_button_pressed = select_pressed && l_pressed;
            fast_forward_button_pressed = select_pressed && r_pressed;
            slow_motion_button_pressed = select_pressed && left_pressed;
         }
      }

      /* Handle turbo buttons */
      if (turbo_enabled[p])
      {
         for (unsigned i = 0; i < NUM_TURBO_BUTTONS; i++)
         {
            if (turbo_btn[i])
            {
               if (turbo_state[p][i] == 1)
                  pads[p] |= turbomap[i].nes;

               turbo_counter[p][i] += 1;
               if (turbo_counter[p][i] >= turbo_pulse_width)
               {
                  turbo_state[p][i] = !turbo_state[p][i];
                  turbo_counter[p][i] = 0;
               }
            }
            else
            {
               turbo_state[p][i]   = 1;
               turbo_counter[p][i] = 0;
            }
         }
      }

      /* Prevent opposing directions, if required */
      if (!up_down_allowed)
      {
         if ((pads[p] & JOY_UP) && (pads[p] & JOY_DOWN))
            pads[p] &= ~(JOY_UP | JOY_DOWN);

         if ((pads[p] & JOY_LEFT) && (pads[p] & JOY_RIGHT))
            pads[p] &= ~(JOY_LEFT | JOY_RIGHT);
      }
   }

   /* Handle internal palette switching */
   if (palette_prev || palette_next)
   {
      if (palette_switch_counter == 0)
      {
         int new_palette_index = palette_index;

         if (palette_prev)
         {
            if (new_palette_index > 0)
               new_palette_index--;
            else
               new_palette_index = NUM_PALETTES_TOTAL - 1;
         }
         else /* palette_next */
         {
            if (new_palette_index < NUM_PALETTES_TOTAL - 1)
               new_palette_index++;
            else
               new_palette_index = 0;
         }

         palette_switch_set_index(new_palette_index);
      }

      palette_switch_counter++;
      if (palette_switch_counter >= PALETTE_SWITCH_PERIOD)
         palette_switch_counter = 0;
   }
   else
      palette_switch_counter = 0;
}

void retro_run(void)
{
   bool updated = false;
   int  pads[MAX_PLAYERS] = {0};

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      check_variables();
   
   // Handle splashscreen display for first 180 frames (3 seconds at 60fps)
   if (splashscreen_active) {
      splashscreen_frame_count++;
      
      if (splashscreen_frame_count <= 180) {
         // Create a temporary video buffer for splashscreen
         static uint16_t splash_buffer[Nes_Emu::image_width * Nes_Emu::image_height];
         
         // Draw splashscreen
         draw_splash_screen(splash_buffer, Nes_Emu::image_width, Nes_Emu::image_height);
         
         // Send splashscreen to video callback
#ifdef PSP
         unsigned width  = Nes_Emu::image_width  - (use_overscan   ? 0 : 16);
         unsigned height = Nes_Emu::image_height - (use_overscan   ? 0 : 16);
#else
         unsigned width  = Nes_Emu::image_width  - (use_overscan_h ? 0 : 16);
         unsigned height = Nes_Emu::image_height - (use_overscan_v ? 0 : 16);
#endif
         video_cb(splash_buffer + (use_overscan_v ? (use_overscan_h ? 0 : 8) : ((use_overscan_h ? 0 : 8) + 256 * 8)),
                  width, height, Nes_Emu::image_width * sizeof(uint16_t));
         
         // Provide silent audio during splashscreen
         int16_t silence[2048] = {0};
         audio_batch_cb(silence, 1024);
         
         return; // Don't run emulation during splashscreen
      } else {
         // Splashscreen timer expired, disable it
         splashscreen_active = false;
      }
   }
   
   bool audioDisabledForThisFrame = false;
   bool videoDisabledForThisFrame = false;
   bool hardDisableAudio = false;
   int flags;
   if (environ_cb(RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE, &flags))
   {
	   videoDisabledForThisFrame = !(flags & 1);
	   audioDisabledForThisFrame = !(flags & 2);
	   hardDisableAudio = !!(flags & 8);
   }

   if (hardDisableAudio != use_silent_buffer)
   {
      use_silent_buffer = hardDisableAudio;
      update_audio_mode();
   }

   update_input(pads);

   if (!videoDisabledForThisFrame)
   {
      // SF2000 Slow Motion: Check if we should skip this frame
      bool should_run_frame = true;
      if (slow_motion_mode == 1) {  // 0.7x speed
         slow_motion_frame_counter++;
         if (slow_motion_frame_counter % 10 >= 7) {
            should_run_frame = false;
         }
      } else if (slow_motion_mode == 2) {  // 0.5x speed
         slow_motion_frame_counter++;
         if (slow_motion_frame_counter % 2 == 1) {
            should_run_frame = false;
         }
      }
      
      if (should_run_frame) {
         // SF2000 Fast Forward: Run multiple frames based on speed setting
         for (int ff_frame = 0; ff_frame < fast_forward_speed; ff_frame++) {
            emu->emulate_frame(pads[0], pads[1]);
         }
      }
	   const Nes_Emu::frame_t &frame = emu->frame();
#ifdef PSP
	   static uint16_t     __attribute__((aligned(16))) retro_palette[256];
	   static unsigned int __attribute__((aligned(16))) d_list[128];
	   void* const texture_vram_p =
		   (void*)(0x44200000 - (Nes_Emu::image_width * Nes_Emu::image_height)); // max VRAM address - frame size


	   sceGuSync(0, 0);

	   for (unsigned i = 0; i < 256; i++)
	   {
		   const Nes_Emu::rgb_t& rgb = current_nes_colors[frame.palette[i]];
		   retro_palette[i] = ((rgb.blue & 0xf8) << 8) | ((rgb.green & 0xfc) << 3) | ((rgb.red & 0xf8) >> 3);
	   }

	   sceKernelDcacheWritebackRange(retro_palette, sizeof(retro_palette));
	   sceKernelDcacheWritebackRange(frame.pixels, Nes_Emu::image_width * Nes_Emu::image_height);

	   sceGuStart(GU_DIRECT, d_list);

	   /* sceGuCopyImage doesnt seem to work correctly with GU_PSM_T8
		* so we use GU_PSM_4444 ( 2 Bytes per pixel ) instead
		* with half the values for pitch / width / x offset
		*/

	   sceGuCopyImage(GU_PSM_4444,
		   (use_overscan ? 0 : 4) + ((u32)frame.pixels & 0xF) / 2,
		   (use_overscan ? 0 : 4),
		   Nes_Emu::image_width / 2 - (use_overscan ? 0 : 8),
		   Nes_Emu::image_height - (use_overscan ? 0 : 16),
		   Nes_Emu::image_width / 2, (void*)((u32)frame.pixels & ~0xF), 0, 0,
		   Nes_Emu::image_width / 2, texture_vram_p);

	   sceGuTexSync();
	   sceGuTexImage(0, 256, 256, 256, texture_vram_p);
	   sceGuTexMode(GU_PSM_T8, 0, 0, GU_FALSE);
	   sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
	   sceGuDisable(GU_BLEND);
	   sceGuClutMode(GU_PSM_5650, 0, 0xFF, 0);
	   sceGuClutLoad(32, retro_palette);

	   sceGuFinish();

	   video_cb(texture_vram_p,
		   Nes_Emu::image_width - (use_overscan ? 0 : 16),
		   Nes_Emu::image_height - (use_overscan ? 0 : 16),
		   256);
#elif defined(RENDER_GSKIT_PS2)

   RETRO_HW_RENDER_INTEFACE_GSKIT_PS2 *ps2 = NULL;
   uint32_t *buf = (uint32_t *)RETRO_HW_FRAME_BUFFER_VALID;

   if (!ps2) {
      static __attribute__((aligned(16))) uint16_t retro_palette[256];
      
      if (!environ_cb(RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE, (void **)&ps2) || !ps2) {
         printf("Failed to get HW rendering interface!\n");
         return;
	   }

      if (ps2->interface_version != RETRO_HW_RENDER_INTERFACE_GSKIT_PS2_VERSION) {
         printf("HW render interface mismatch, expected %u, got %u!\n", 
                  RETRO_HW_RENDER_INTERFACE_GSKIT_PS2_VERSION, ps2->interface_version);
         return;
      }

      for (unsigned i = 0; i < 256; i++) {
         const Nes_Emu::rgb_t& rgb = current_nes_colors[frame.palette[i]];
         int dest_index = i;
         int modi = i & 63;
         if ((modi >= 8 && modi < 16) || (modi >= 40 && modi < 48)) {
            dest_index += 8;
         } else if ((modi >= 16 && modi < 24) || (modi >= 48 && modi < 56)) {
            dest_index -= 8;
         }

         retro_palette[dest_index] = (rgb.blue << 16) | (rgb.green << 8) | rgb.red;
      }
      
      ps2->coreTexture->Width = videoBufferWidth;
      ps2->coreTexture->Height = videoBufferHeight;
      ps2->coreTexture->PSM = GS_PSM_T8;
      ps2->coreTexture->ClutPSM = GS_PSM_CT32;
      ps2->coreTexture->Filter = GS_FILTER_LINEAR;
      ps2->coreTexture->Clut = (u32*)retro_palette;
      ps2->padding = (retro_hw_ps2_insets){8.0f, 8.0f, 8.0f, 8.0f};
   }

   ps2->coreTexture->Mem = (u32*)frame.pixels;

   video_cb(buf,
		   videoBufferWidth,
		   videoBufferHeight,
		   videoBufferWidth * sizeof(uint16_t));
#else

	   static uint16_t video_buffer[Nes_Emu::image_width * Nes_Emu::image_height];
	   static uint16_t retro_palette[256];

	   for (unsigned i = 0; i < 256; i++)
	   {
		   const Nes_Emu::rgb_t& rgb = current_nes_colors[frame.palette[i]];
#if defined(ABGR1555)
         retro_palette[i] = ((rgb.blue & 0xf8) << 7) | ((rgb.green & 0xf8) << 2) | ((rgb.red & 0xf8) >> 3);
#else
         retro_palette[i] = ((rgb.red & 0xf8) << 8) | ((rgb.green & 0xfc) << 3) | ((rgb.blue & 0xf8) >> 3);
#endif
	   }

	   for (int y = 0; y < Nes_Emu::image_height; y++)
	   {
		   uint16_t *out_scanline = video_buffer + Nes_Emu::image_width * y;
		   uint8_t *in_scanline = frame.pixels + videoBufferWidth * y;
		   for (int x = 0; x < Nes_Emu::image_width; x++)
			   out_scanline[x] = retro_palette[in_scanline[x]];
	   }

	   video_cb(video_buffer + (use_overscan_v ? (use_overscan_h ? 0 : 8) : ((use_overscan_h ? 0 : 8) + 256 * 8)),
		   Nes_Emu::image_width - (use_overscan_h ? 0 : 16),
		   Nes_Emu::image_height - (use_overscan_v ? 0 : 16),
		   Nes_Emu::image_width * sizeof(uint16_t));
#endif
   }
   else
   {
      // SF2000 Slow Motion: Check if we should skip this frame
      bool should_run_frame = true;
      if (slow_motion_mode == 1) {  // 0.7x speed
         slow_motion_frame_counter++;
         if (slow_motion_frame_counter % 10 >= 7) {
            should_run_frame = false;
         }
      } else if (slow_motion_mode == 2) {  // 0.5x speed
         slow_motion_frame_counter++;
         if (slow_motion_frame_counter % 2 == 1) {
            should_run_frame = false;
         }
      }
      
      if (should_run_frame) {
         // SF2000 Fast Forward: Run multiple frames even when video disabled
         for (int ff_frame = 0; ff_frame < fast_forward_speed; ff_frame++) {
            emu->emulate_skip_frame(pads[0], pads[1]);
         }
      }
   }

   if (!audioDisabledForThisFrame)
   {
	   // Mono -> Stereo.
	   int16_t samples[2048];
	   long read_samples = emu->read_samples(samples, 2048);
	   int16_t out_samples[4096];

	   if ( current_buffer != &effects_buffer)
	   {
			for (long i = 0; i < read_samples; i++)
				out_samples[(i << 1)] = out_samples[(i << 1) + 1] = samples[i];
			audio_batch_cb(out_samples, read_samples);
	   }
	   else
			audio_batch_cb(samples, read_samples >> 1);
   }
   else
   {
	   emu->read_samples(NULL, 2048);
   }

   // SF2000 Rewind: Handle rewind functionality
   rewind_check_buttons();
   
   // SF2000 Fast Forward: Handle fast forward functionality
   fast_forward_check_buttons();
   
   // SF2000 Slow Motion: Handle slow motion functionality
   slow_motion_check_buttons();
   
   // SF2000 Rewind: Save state periodically (not while rewinding)
   if (!rewind_data.rewinding) {
      rewind_data.frame_count++;
      if (rewind_data.frame_count >= REWIND_GRANULARITY) {
         rewind_save_state();
         rewind_data.frame_count = 0;
      }
   }
}

bool retro_load_game(const struct retro_game_info *info)
{
   struct retro_input_descriptor desc[] = {
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "D-Pad Left" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "D-Pad Up" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "D-Pad Down" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "D-Pad Right" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "B" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "A" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,      "Turbo B" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,      "Turbo A" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },

      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "D-Pad Left" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "D-Pad Up" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "D-Pad Down" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "D-Pad Right" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "B" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "A" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,      "Turbo B" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,      "Turbo A" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },

      { 0 },
   };

   struct retro_input_descriptor desc_ps[] = { /* ps: palette switch */
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "D-Pad Left" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "D-Pad Up" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "D-Pad Down" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "D-Pad Right" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "B" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "A" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,      "Turbo B" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,      "Turbo A" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,      "Prev. Color Palette" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,      "Next Color Palette" },

      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "D-Pad Left" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "D-Pad Up" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "D-Pad Down" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "D-Pad Right" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "B" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "A" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,      "Turbo B" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,      "Turbo A" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },

      { 0 },
   };

   if (libretro_supports_set_variable)
       environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc_ps);
   else
      environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
      return false;

   emu = new Nes_Emu;
   check_variables();

   update_audio_mode(); //calls set_sample_rate and set_equalizer
   emu->set_palette_range(0);

#ifdef PSP
   // TODO: fix overscan setting for PSP
   use_overscan = false;
   static uint8_t video_buffer[Nes_Emu::image_width * (Nes_Emu::image_height + 16)];
   emu->set_pixels(video_buffer + (8 * Nes_Emu::image_width), Nes_Emu::image_width);
#else
   static uint8_t video_buffer[videoBufferWidth * videoBufferHeight];
   emu->set_pixels(video_buffer, videoBufferWidth);
#endif

   struct retro_memory_descriptor descs[2];
   struct retro_memory_map retromap;

   memset(descs, 0, sizeof(descs));

   descs[0].ptr    = emu->low_mem();         // System RAM
   descs[0].start  = 0x00000000;
   descs[0].len    = Nes_Emu::low_mem_size;
   descs[0].select = 0;

   descs[1].ptr    = emu->high_mem();        // WRAM
   descs[1].start  = 0x00006000;
   descs[1].len    = Nes_Emu::high_mem_size;
   descs[1].select = 0;

   retromap.descriptors       = descs;
   retromap.num_descriptors   = sizeof(descs) / sizeof(*descs);

   environ_cb(RETRO_ENVIRONMENT_SET_MEMORY_MAPS, &retromap);

   Mem_File_Reader reader(info->data, info->size);
   return !emu->load_ines(reader);
}

void retro_unload_game(void)
{
   if (emu)
      emu->close();
   delete emu;
   emu = 0;
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned, const struct retro_game_info *, size_t)
{
   return false;
}

size_t retro_serialize_size(void)
{
   Mem_Writer writer;
   if (emu->save_state(writer))
      return 0;

   return writer.size();
}

bool is_fast_savestate()
{
	int value;
	bool okay = environ_cb(RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE, &value);
	if (okay)
	{
		if (value & 4)
		{
			return true;
		}
	}
	return false;
}

bool retro_serialize(void *data, size_t size)
{
   bool isFastSavestate = is_fast_savestate();
   Mem_Writer writer(data, size);
   bool okay = !emu->save_state(writer);
   if (isFastSavestate)
   {
      emu->SaveAudioBufferState();
   }
   return okay;
}

bool retro_unserialize(const void *data, size_t size)
{
   bool isFastSavestate = is_fast_savestate();
   Mem_File_Reader reader(data, size);
   bool okay = !emu->load_state(reader);
   if (isFastSavestate)
   {
      emu->RestoreAudioBufferState();
   }
   return okay;
}

void *retro_get_memory_data(unsigned id)
{
   switch (id)
   {
      case RETRO_MEMORY_SAVE_RAM:
         if (emu->has_battery_ram())
             return emu->high_mem();
         break;
      case RETRO_MEMORY_SYSTEM_RAM:
         return emu->low_mem();
      default:
         break;
   }

   return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
   switch (id)
   {
      case RETRO_MEMORY_SAVE_RAM:
         if (emu->has_battery_ram())
            return Nes_Emu::high_mem_size;
         break;
      case RETRO_MEMORY_SYSTEM_RAM:
         return Nes_Emu::low_mem_size;
      default:
         break;
   }

   return 0;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned, bool, const char *)
{}

// SF2000 Rewind Implementation
static void rewind_init()
{
   // Initialize rewind system
   rewind_data.enabled = true;
   rewind_data.rewinding = false;
   rewind_data.current_index = 0;
   rewind_data.frame_count = 0;
   rewind_data.temp_buffer_size = 65536; // 64KB initial buffer
   rewind_data.temp_buffer = (uint8_t*)malloc(rewind_data.temp_buffer_size);
   
   // Initialize all buffer slots
   for (int i = 0; i < REWIND_BUFFER_SIZE; i++) {
      rewind_data.buffer[i].data = NULL;
      rewind_data.buffer[i].size = 0;
      rewind_data.buffer[i].valid = false;
   }
}

static void rewind_deinit()
{
   // Free all allocated memory
   if (rewind_data.temp_buffer) {
      free(rewind_data.temp_buffer);
      rewind_data.temp_buffer = NULL;
   }
   
   for (int i = 0; i < REWIND_BUFFER_SIZE; i++) {
      if (rewind_data.buffer[i].data) {
         free(rewind_data.buffer[i].data);
         rewind_data.buffer[i].data = NULL;
      }
      rewind_data.buffer[i].valid = false;
   }
   
   rewind_data.enabled = false;
}

static void rewind_save_state()
{
   if (!rewind_data.enabled || !emu) return;
   
   // Get current state size
   size_t state_size = retro_serialize_size();
   if (state_size == 0) return;
   
   // Resize temp buffer if needed
   if (state_size > rewind_data.temp_buffer_size) {
      rewind_data.temp_buffer_size = state_size + 8192; // Add some padding
      rewind_data.temp_buffer = (uint8_t*)realloc(rewind_data.temp_buffer, rewind_data.temp_buffer_size);
      if (!rewind_data.temp_buffer) return;
   }
   
   // Save current state to temp buffer
   if (!retro_serialize(rewind_data.temp_buffer, state_size)) return;
   
   // Move to next buffer slot
   rewind_data.current_index = (rewind_data.current_index + 1) % REWIND_BUFFER_SIZE;
   
   // Free old data in this slot
   if (rewind_data.buffer[rewind_data.current_index].data) {
      free(rewind_data.buffer[rewind_data.current_index].data);
   }
   
   // Allocate new data for this slot
   rewind_data.buffer[rewind_data.current_index].data = (uint8_t*)malloc(state_size);
   if (!rewind_data.buffer[rewind_data.current_index].data) return;
   
   // Copy state data
   memcpy(rewind_data.buffer[rewind_data.current_index].data, rewind_data.temp_buffer, state_size);
   rewind_data.buffer[rewind_data.current_index].size = state_size;
   rewind_data.buffer[rewind_data.current_index].valid = true;
}

static void rewind_load_state()
{
   if (!rewind_data.enabled || !emu) return;
   
   // Find the most recent valid state
   int load_index = rewind_data.current_index;
   int attempts = 0;
   
   while (attempts < REWIND_BUFFER_SIZE) {
      load_index = (load_index - 1 + REWIND_BUFFER_SIZE) % REWIND_BUFFER_SIZE;
      
      if (rewind_data.buffer[load_index].valid) {
         // Load this state
         if (retro_unserialize(rewind_data.buffer[load_index].data, rewind_data.buffer[load_index].size)) {
            // Successfully loaded, invalidate this state so we don't load it again
            rewind_data.buffer[load_index].valid = false;
            rewind_data.current_index = load_index;
            return;
         }
      }
      attempts++;
   }
}

static void rewind_check_buttons()
{
   if (!rewind_data.enabled) return;
   
   // Handle button state changes
   if (rewind_button_pressed && !rewind_button_held) {
      // Button just pressed - start rewinding
      rewind_data.rewinding = true;
      rewind_button_held = true;
      rewind_load_state();
   } else if (!rewind_button_pressed && rewind_button_held) {
      // Button just released - stop rewinding
      rewind_data.rewinding = false;
      rewind_button_held = false;
   } else if (rewind_button_pressed && rewind_button_held) {
      // Button held - continue rewinding
      rewind_load_state();
   }
}

// Fast forward functionality for SF2000

static void fast_forward_toggle()
{
   // Cycle through speeds: 1x -> 2x -> 3x -> 1x
   fast_forward_speed++;
   if (fast_forward_speed > 3) {
      fast_forward_speed = 1;
   }
}

static void fast_forward_check_buttons()
{
   // Handle button state changes for SELECT + R
   if (fast_forward_button_pressed && !fast_forward_button_held) {
      // Button just pressed - toggle speed
      fast_forward_toggle();
      fast_forward_button_held = true;
   } else if (!fast_forward_button_pressed && fast_forward_button_held) {
      // Button just released
      fast_forward_button_held = false;
   }
}

// Slow motion functionality for SF2000

static void slow_motion_toggle()
{
   // Cycle through speeds: 1x -> 0.7x -> 0.5x -> 1x
   slow_motion_mode++;
   if (slow_motion_mode > 2) {
      slow_motion_mode = 0;
   }
   slow_motion_frame_counter = 0;  // Reset frame counter
}

static void slow_motion_check_buttons()
{
   // Handle button state changes for SELECT + LEFT
   if (slow_motion_button_pressed && !slow_motion_button_held) {
      // Button just pressed - toggle slow motion mode
      slow_motion_toggle();
      slow_motion_button_held = true;
   } else if (!slow_motion_button_pressed && slow_motion_button_held) {
      // Button just released
      slow_motion_button_held = false;
   }
}
