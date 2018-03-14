#include "libretro.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "Nes_Emu.h"
#include "Data_Reader.h"
#include "abstract_file.h"

#ifdef PSP
#include "pspkernel.h"
#include "pspgu.h"
#endif

#define CORE_VERSION "1.0-WIP"

#define NES_4_3 (4.0 / 3.0)
#define NES_PAR (width * (8.0 / 7.0) / height)

static Nes_Emu *emu;

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static bool use_overscan;
static bool aspect_ratio_par;

const int videoBufferWidth = Nes_Emu::image_width + 16;
const int videoBufferHeight = Nes_Emu::image_height + 2;

void retro_init(void)
{
}

void retro_deinit(void)
{
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

double_t get_aspect_ratio(unsigned width, unsigned height)
{
   return (aspect_ratio_par ? NES_PAR : NES_4_3);
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   unsigned width  = Nes_Emu::image_width  - (use_overscan? 0 : 16);
   unsigned height = Nes_Emu::image_height - (use_overscan? 0 : 16);

   const retro_system_timing timing = { Nes_Emu::frame_rate, 44100.0 };
   info->timing = timing;

   info->geometry.base_width  = width;
   info->geometry.base_height = height;
   info->geometry.max_width   = width;
   info->geometry.max_height  = height;
   info->geometry.aspect_ratio = get_aspect_ratio(width, height);
}

void retro_set_environment(retro_environment_t cb)
{
   static const struct retro_variable vars[] = {
      { "quicknes_aspect_ratio_par", "Aspect Ratio; 4:3|PAR" },
#ifndef PSP
      { "quicknes_use_overscan", "Use overscan; enabled|disabled" },
#endif
      { NULL, NULL },
   };

   environ_cb = cb;
   cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
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
      emu->reset();
}

static void check_variables(void)
{
   struct retro_variable var = {0};
   bool video_changed = false;

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

#ifndef PSP
   var.key = "quicknes_use_overscan";

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      bool newval = (!strcmp(var.value, "enabled"));
      use_overscan = newval;
      video_changed = true;
   }
#endif

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

static void update_input(int pads[2])
{
   pads[0] = pads[1] = 0;
   input_poll_cb();

   for (unsigned p = 0; p < 2; p++)
      for (unsigned bind = 0; bind < sizeof(bindmap) / sizeof(bindmap[0]); bind++)
         pads[p] |= input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, bindmap[bind].retro) ? bindmap[bind].nes : 0;
}

void retro_run(void)
{
   bool updated = false;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      check_variables();

   int pads[2] = {0};
   update_input(pads);

   emu->emulate_frame(pads[0], pads[1]);
   const Nes_Emu::frame_t &frame = emu->frame();

#ifdef PSP
   static uint16_t     __attribute__((aligned(16))) retro_palette[256];
   static unsigned int __attribute__((aligned(16))) d_list[128];
   void* const texture_vram_p =
         (void*) (0x44200000 - (Nes_Emu::image_width * Nes_Emu::image_height)); // max VRAM address - frame size


   sceGuSync(0, 0);

   for (unsigned i = 0; i < 256; i++)
   {
      const Nes_Emu::rgb_t& rgb = emu->nes_colors[frame.palette[i]];
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
                  (use_overscan? 0 : 4) + ((u32)frame.pixels & 0xF) / 2,
                  (use_overscan? 0 : 4),
                  Nes_Emu::image_width / 2 - (use_overscan? 0 : 8),
                  Nes_Emu::image_height    - (use_overscan? 0 : 16),
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
            Nes_Emu::image_width  - (use_overscan? 0 : 16),
            Nes_Emu::image_height - (use_overscan? 0 : 16),
            256);
#else

   static uint16_t video_buffer[Nes_Emu::image_width * Nes_Emu::image_height];
   static uint16_t retro_palette[256];

   for (unsigned i = 0; i < 256; i++)
   {
      const Nes_Emu::rgb_t& rgb = emu->nes_colors[frame.palette[i]];
      retro_palette[i] = ((rgb.red & 0xf8) << 8) | ((rgb.green & 0xfc) << 3) | ((rgb.blue & 0xf8) >> 3);
   }

   for (int y = 0; y < Nes_Emu::image_height; y++)
   {
       uint16_t *out_scanline = video_buffer + Nes_Emu::image_width * y;
       uint8_t *in_scanline = frame.pixels + videoBufferWidth * y;
       for (int x = 0; x < Nes_Emu::image_width; x++)
           out_scanline[x] = retro_palette[in_scanline[x]];
   }

   video_cb(video_buffer    + (use_overscan? 0 : Nes_Emu::image_width * 8 + 8),
      Nes_Emu::image_width  - (use_overscan? 0 : 16),
      Nes_Emu::image_height - (use_overscan? 0 : 16),
      Nes_Emu::image_width  * sizeof(uint16_t));
#endif
   // Mono -> Stereo.
   int16_t samples[2048];
   long read_samples = emu->read_samples(samples, 2048);
   int16_t out_samples[4096];
   for (long i = 0; i < read_samples; i++)
      out_samples[(i << 1)] = out_samples[(i << 1) + 1] = samples[i];

   audio_batch_cb(out_samples, read_samples);
}

bool retro_load_game(const struct retro_game_info *info)
{
   struct retro_input_descriptor desc[] = {
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

      { 0 },
   };

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      fprintf(stderr, "RGB565 is not supported.\n");
      return false;
   }

   emu = new Nes_Emu;
   register_optional_mappers();

   check_variables();

#ifdef PSP
   if (!environ_cb(RETRO_ENVIRONMENT_GET_OVERSCAN, &use_overscan))
      use_overscan = true;
#endif

   emu->set_sample_rate(44100);
   emu->set_equalizer(Nes_Emu::nes_eq);
   emu->set_palette_range(0);

   static uint8_t video_buffer[videoBufferWidth * videoBufferHeight];
   emu->set_pixels(video_buffer, videoBufferWidth);

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

bool retro_serialize(void *data, size_t size)
{
   Mem_Writer writer(data, size);
   return !emu->save_state(writer);
}

bool retro_unserialize(const void *data, size_t size)
{
   Mem_File_Reader reader(data, size);
   return !emu->load_state(reader);
}

void *retro_get_memory_data(unsigned id)
{
   switch (id)
   {
      case RETRO_MEMORY_SAVE_RAM:
         return emu->high_mem();
      case RETRO_MEMORY_SYSTEM_RAM:
         return emu->low_mem();
      default:
         break;
   }

   return 0;
}

size_t retro_get_memory_size(unsigned id)
{
   switch (id)
   {
      case RETRO_MEMORY_SAVE_RAM:
         return Nes_Emu::high_mem_size;
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
