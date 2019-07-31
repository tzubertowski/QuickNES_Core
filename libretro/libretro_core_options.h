#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include <retro_inline.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
*/

/* RETRO_LANGUAGE_ENGLISH */

/* Default language:
 * - All other languages must include the same keys and values
 * - Will be used as a fallback in the event that frontend language
 *   is not available
 * - Will be used as a fallback for any missing entries in
 *   frontend language definition */

#define MAX_CORE_OPTIONS 32

struct retro_core_option_definition option_defs_us[] = {
   {
      "quicknes_up_down_allowed",
      "Allow Opposing Directions",
      "Enabling this will allow pressing / quickly alternating / holding both left and right (or up and down in some games) directions at the same time. This may cause movement based glitches to occur in certain games. It's best to keep this core option disabled.",
      {
         { "disabled",  NULL },
         { "enabled",  NULL },
         { NULL, NULL},
      },
      "disabled",
   },
   {
      "quicknes_aspect_ratio_par",
      "Aspect Ratio",
      "Configure QuickNES's core provided aspect ratio.",
      {
         { "PAR", NULL },
         { "4:3",     NULL },
         { NULL, NULL},
      },
      "PAR",
   },
#ifndef PSP
   {
      "quicknes_use_overscan_h",
      "Show horizontal overscan",
      "Set this to disabled to crop out (horizontally) the potentially random glitchy video output that would have been hidden by the bezel around the edge of a standard-definition television screen.",
      {
         { "enabled", NULL },
         { "disabled",     NULL },
         { NULL, NULL},
      },
      "enabled",
   },
   {
      "quicknes_use_overscan_v",
      "Show vertical overscan",
      "Set this to disabled to crop out (vertically) the potentially random glitchy video output that would have been hidden by the bezel around the edge of a standard-definition television screen.",
      {
         { "disabled",     NULL },
         { "enabled", NULL },
         { NULL, NULL},
      },
      "enabled",
   },
#endif
   {
      "quicknes_no_sprite_limit",
      "No Sprite Limit",
      "Removes the 8-per-scanline hardware limit. This reduces sprite flickering but can cause some games to glitch since some use this for effects.",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL},
      },
      "disabled",
   },
   {
      "quicknes_audio_nonlinear",
      "Audio mode",
      "Configure audio mode. Stereo panning simulates stereo by using a panning method and some reverb effects to add some depth.",
      {
         { "nonlinear",  NULL },
         { "linear",  NULL },
         { "stereo spanning",  NULL },
         { NULL, NULL },
      },
      "nonlinear",
   },
   {
      "quicknes_audio_eq",
      "Audio equalizer preset",
      "Applies a preset to the audio equalize",
      {
         { "default",      "Default" },
         { "famicom",      "Famicom" },
         { "tv",           "TV" },
         { "flat",         "Flat" },
         { "crisp",        "Crisp" },
         { "tinny",        "Tinny" },
         { NULL, NULL},
      },
      "default",
   },
   {
      "quicknes_palette",
      "Color Palette",
      "Specifies which color palette to use when decoding the NTSC video signal output by the NES.",
      {
         { "default",              NULL },
         { "asqrealc",             NULL },
         { "nintendo-vc",          NULL },
         { "rgb",                  NULL },
         { "yuv-v3",               NULL },
         { "unsaturated-final",    NULL },
         { "sony-cxa2025as-us",    NULL },
         { "pal",                  NULL },
         { "bmf-final2",           NULL },
         { "bmf-final3",           NULL },
         { "smooth-fbx",           NULL },
         { "composite-direct-fbx", NULL },
         { "pvm-style-d93-fbx",    NULL },
         { "ntsc-hardware-fbx",    NULL },
         { "nes-classic-fbx-fs",   NULL },
         { "nescap",               NULL },
         { "wavebeam",             NULL },
         { NULL, NULL},
      },
      "default",
   },
   {
      "quicknes_turbo_enable",
      "Turbo Enable",
      "Enables the use of the Turbo A and Turbo B buttons.",
      {
         { "none", NULL },
         { "player 1",  NULL },
         { "player 2",  NULL },
         { "both",  NULL },
         { NULL, NULL},
      },
      "none",
   },
   {
      "quicknes_turbo_pulse_width",
      "Turbo pulse width (in frames)",
      "Specifies both the width and spacing (in frames) of input 'pulses' when the Turbo A and Turbo B buttons are held down. For example, the default setting of '3' corresponds to a (60/(3+3)) = 10 Hz turbo frequency (10 presses per second).",
      {
         { "1",     NULL },
         { "2",     NULL },
         { "3",     NULL },
         { "5",     NULL },
         { "10",     NULL },
         { "15",     NULL },
         { "30",     NULL },
         { "60",     NULL },
         { NULL, NULL},
      },
      "3",
   },
   { NULL, NULL, NULL, { NULL, NULL }, NULL },
};

/* RETRO_LANGUAGE_JAPANESE */

/* RETRO_LANGUAGE_FRENCH */

/* RETRO_LANGUAGE_SPANISH */

/* RETRO_LANGUAGE_GERMAN */

/* RETRO_LANGUAGE_ITALIAN */

/* RETRO_LANGUAGE_DUTCH */

/* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */

/* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */

/* RETRO_LANGUAGE_RUSSIAN */

/* RETRO_LANGUAGE_KOREAN */

/* RETRO_LANGUAGE_CHINESE_TRADITIONAL */

/* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */

/* RETRO_LANGUAGE_ESPERANTO */

/* RETRO_LANGUAGE_POLISH */

/* RETRO_LANGUAGE_VIETNAMESE */

/* RETRO_LANGUAGE_ARABIC */

/* RETRO_LANGUAGE_GREEK */

/* RETRO_LANGUAGE_TURKISH */

/*
 ********************************
 * Language Mapping
 ********************************
*/

struct retro_core_option_definition *option_defs_intl[RETRO_LANGUAGE_LAST] = {
   option_defs_us, /* RETRO_LANGUAGE_ENGLISH */
   NULL,           /* RETRO_LANGUAGE_JAPANESE */
   NULL,           /* RETRO_LANGUAGE_FRENCH */
   NULL,           /* RETRO_LANGUAGE_SPANISH */
   NULL,           /* RETRO_LANGUAGE_GERMAN */
   NULL,           /* RETRO_LANGUAGE_ITALIAN */
   NULL,           /* RETRO_LANGUAGE_DUTCH */
   NULL,           /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
   NULL,           /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
   NULL,           /* RETRO_LANGUAGE_RUSSIAN */
   NULL,           /* RETRO_LANGUAGE_KOREAN */
   NULL,           /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
   NULL,           /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
   NULL,           /* RETRO_LANGUAGE_ESPERANTO */
   NULL,           /* RETRO_LANGUAGE_POLISH */
   NULL,           /* RETRO_LANGUAGE_VIETNAMESE */
   NULL,           /* RETRO_LANGUAGE_ARABIC */
   NULL,           /* RETRO_LANGUAGE_GREEK */
   NULL,           /* RETRO_LANGUAGE_TURKISH */
};

/*
 ********************************
 * Functions
 ********************************
*/

/* Handles configuration/setting of core options.
 * Should only be called inside retro_set_environment().
 * > We place the function body in the header to avoid the
 *   necessity of adding more .c files (i.e. want this to
 *   be as painless as possible for core devs)
 */

static INLINE void libretro_set_core_options(retro_environment_t environ_cb)
{
   unsigned version = 0;

   if (!environ_cb)
      return;

   if (environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version) && (version == 1))
   {
      struct retro_core_options_intl core_options_intl;
      unsigned language = 0;

      core_options_intl.us    = option_defs_us;
      core_options_intl.local = NULL;

      if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
          (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
         core_options_intl.local = option_defs_intl[language];

      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_intl);
   }
   else
   {
      size_t i;
      size_t option_index              = 0;
      size_t num_options               = 0;
      struct retro_variable *variables = NULL;
      char **values_buf                = NULL;

      /* Determine number of options
       * > Note: We are going to skip a number of irrelevant
       *   core options when building the retro_variable array,
       *   but we'll allocate space for all of them. The difference
       *   in resource usage is negligible, and this allows us to
       *   keep the code 'cleaner' */
      while (true)
      {
         if (option_defs_us[num_options].key)
            num_options++;
         else
            break;
      }

      /* Allocate arrays */
      variables  = (struct retro_variable *)calloc(num_options + 1, sizeof(struct retro_variable));
      values_buf = (char **)calloc(num_options, sizeof(char *));

      if (!variables || !values_buf)
         goto error;

      /* Copy parameters from option_defs_us array */
      for (i = 0; i < num_options; i++)
      {
         const char *key                        = option_defs_us[i].key;
         const char *desc                       = option_defs_us[i].desc;
         const char *default_value              = option_defs_us[i].default_value;
         struct retro_core_option_value *values = option_defs_us[i].values;
         size_t buf_len                         = 3;
         size_t default_index                   = 0;

         values_buf[i] = NULL;

         /* Skip options that are irrelevant when using the
          * old style core options interface */
         if ((strcmp(key, "fceumm_advance_sound_options") == 0))
            continue;

         if (desc)
         {
            size_t num_values = 0;

            /* Determine number of values */
            while (true)
            {
               if (values[num_values].value)
               {
                  /* Check if this is the default value */
                  if (default_value)
                     if (strcmp(values[num_values].value, default_value) == 0)
                        default_index = num_values;

                  buf_len += strlen(values[num_values].value);
                  num_values++;
               }
               else
                  break;
            }

            /* Build values string */
            if (num_values > 1)
            {
               size_t j;

               buf_len += num_values - 1;
               buf_len += strlen(desc);

               values_buf[i] = (char *)calloc(buf_len, sizeof(char));
               if (!values_buf[i])
                  goto error;

               strcpy(values_buf[i], desc);
               strcat(values_buf[i], "; ");

               /* Default value goes first */
               strcat(values_buf[i], values[default_index].value);

               /* Add remaining values */
               for (j = 0; j < num_values; j++)
               {
                  if (j != default_index)
                  {
                     strcat(values_buf[i], "|");
                     strcat(values_buf[i], values[j].value);
                  }
               }
            }
         }

         variables[option_index].key   = key;
         variables[option_index].value = values_buf[i];
         option_index++;
      }

      /* Set variables */
      environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);

error:

      /* Clean up */
      if (values_buf)
      {
         for (i = 0; i < num_options; i++)
         {
            if (values_buf[i])
            {
               free(values_buf[i]);
               values_buf[i] = NULL;
            }
         }

         free(values_buf);
         values_buf = NULL;
      }

      if (variables)
      {
         free(variables);
         variables = NULL;
      }
   }
}

#ifdef __cplusplus
}
#endif

#endif
