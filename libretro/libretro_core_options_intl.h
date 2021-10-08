#ifndef LIBRETRO_CORE_OPTIONS_INTL_H__
#define LIBRETRO_CORE_OPTIONS_INTL_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1500 && _MSC_VER < 1900)
/* https://support.microsoft.com/en-us/kb/980263 */
#pragma execution_character_set("utf-8")
#pragma warning(disable:4566)
#endif

#include <libretro.h>

/*
 ********************************
 * VERSION: 2.0
 ********************************
 *
 * - 2.0: Add support for core options v2 interface
 * - 1.3: Move translations to libretro_core_options_intl.h
 *        - libretro_core_options_intl.h includes BOM and utf-8
 *          fix for MSVC 2010-2013
 *        - Added HAVE_NO_LANGEXTRA flag to disable translations
 *          on platforms/compilers without BOM support
 * - 1.2: Use core options v1 interface when
 *        RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION is >= 1
 *        (previously required RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION == 1)
 * - 1.1: Support generation of core options v0 retro_core_option_value
 *        arrays containing options with a single value
 * - 1.0: First commit
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
*/

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

struct retro_core_option_v2_category option_cats_tr[] = {
   { NULL, NULL, NULL },
};

struct retro_core_option_v2_definition option_defs_tr[] = {
   {
      "quicknes_aspect_ratio_par",
      "En Boy Oranı",
      NULL,
      "QuickNES Core'un sağlanan en boy oranını yapılandırın.",
      NULL,
      NULL,
      {
         { NULL, NULL },
      },
      NULL,
   },
#ifndef PSP
   {
      "quicknes_use_overscan_h",
      "Yatay ekran taşmasını göster",
      NULL,
      "Standart bir televizyon ekranının kenarına çerçeve tarafından gizlenmiş potansiyel olarak rastgele rastlanan video çıkışını kesmek (yatay olarak) için bunu devre dışı olarak ayarlayın.",
      NULL,
      NULL,
      {
         { NULL, NULL },
      },
      NULL,
   },
   {
      "quicknes_use_overscan_v",
      "Yatay ekran taşmasını göster",
      NULL,
      "Standart bir televizyon ekranının kenarına çerçeve tarafından gizlenmiş potansiyel olarak rastgele rastlanan video çıkışını kesmek (dikey olarak) için bunu devre dışı olarak ayarlayın.",
      NULL,
      NULL,
      {
         { NULL, NULL },
      },
      NULL,
   },
#endif
   {
      "quicknes_palette",
      "Renk paleti",
      NULL,
      "NTS tarafından NTSC video sinyali çıkışının kodunu çözerken hangi renk paletinin kullanılacağını belirtir.",
      NULL,
      NULL,
      {
         { "default", "Varsayılan" },
         { NULL, NULL },
      },
      NULL,
   },
   {
      "quicknes_no_sprite_limit",
      "Sprite Sınırı Yok",
      NULL,
      "Scanline başına 8 donanım sınırını kaldırır. Bu, sprite titremesini azaltır ancak bazı efektler için bunu kullandığında bazı oyunların hata yapmasına neden olabilir.",
      NULL,
      NULL,
      {
         { NULL, NULL },
      },
      NULL,
   },
   {
      "quicknes_audio_nonlinear",
      "Ses Modu",
      NULL,
      "Ses modunu yapılandırın. Stereo kaydırma, derinlik yöntemi eklemek için kaydırma yöntemi ve bazı yankı efektleri kullanarak stereoyu simüle eder.",
      NULL,
      NULL,
      {
         { NULL, NULL },
      },
      NULL,
   },
   {
      "quicknes_audio_eq",
      "Ses ekolayzer ön ayarı",
      NULL,
      "Sesi eşitlemeye bir ön ayar uygular",
      NULL,
      NULL,
      {
         { "default", "Varsayılan" },
         { NULL, NULL },
      },
      NULL,
   },
   {
      "quicknes_turbo_enable",
      "Turbo'yu Etkinleştir",
      NULL,
      "Turbo A ve Turbo B düğmelerinin kullanılmasını sağlar.",
      NULL,
      NULL,
      {
         { "none",     "Hiçbiri" },
         { "player 1", "1. Oyuncu" },
         { "player 2", "2. Oyuncu" },
         { "both",     "ikisi içinde" },
         { NULL, NULL },
      },
      NULL,
   },
   {
      "quicknes_turbo_pulse_width",
      "Turbo darbe genişliği (çerçevelerde)",
      NULL,
      "Turbo A ve Turbo B düğmeleri basılı tutulduğunda 'darbelerin' girişinin hem genişliğini hem de aralığını (çerçevelerde) belirtir. Örneğin, varsayılan '3' ayarı bir (60 / (3 + 3)) = 10 Hz turbo frekansına (saniyede 10 basma) karşılık gelir.",
      NULL,
      NULL,
      {
         { NULL, NULL },
      },
      NULL,
   },
   {
      "quicknes_up_down_allowed",
      "Karşı Yönlere İzin Ver",
      NULL,
      "Bunu etkinleştirmek aynı anda hem sola hem de sağa (veya bazı oyunlarda yukarı ve aşağı) yönlere basma / hızlı değiştirme / tutma olanağı sağlar. Bu, bazı oyunlarda harekete dayalı hataların oluşmasına neden olabilir. Bu core seçeneğinin devre dışı bırakılması en iyisidir.",
      NULL,
      NULL,
      {
         { NULL, NULL },
      },
      NULL,
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};

struct retro_core_options_v2 options_tr = {
   option_cats_tr,
   option_defs_tr
};

#ifdef __cplusplus
}
#endif

#endif
