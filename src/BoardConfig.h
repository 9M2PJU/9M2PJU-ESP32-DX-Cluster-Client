#pragma once

/*
 * BoardConfig — selects the LovyanGFX panel/bus/pin configuration for the
 * board being built. The active board is chosen at compile time by a
 * -D BOARD_<NAME> build flag set in platformio.ini.
 *
 * Each board defines a macro BOARD_DISPLAY_CLASS which is the concrete
 * LovyanGFX subclass to instantiate, plus BOARD_DISPLAY_WIDTH /
 * BOARD_DISPLAY_HEIGHT (in the default rotation) and BOARD_HAS_BACKLIGHT.
 *
 * To add a new board:
 *   1. Add a `#elif defined(BOARD_<NAME>)` block below that defines
 *      BOARD_DISPLAY_CLASS, BOARD_DISPLAY_WIDTH, BOARD_DISPLAY_HEIGHT,
 *      and BOARD_DISPLAY_ROTATION.
 *   2. Implement the LovyanGFX subclass in src/boards/<name>.h.
 *   3. Add an [env:<name>] section in platformio.ini with the matching
 *      -D BOARD_<NAME> flag.
 */

#include <LovyanGFX.hpp>

// ============================================================================
// Default fallback (used if no BOARD_* flag is set — picks T-Display-S3)
// ============================================================================
#if !defined(BOARD_LILYGO_TDISPLAY_S3) && !defined(BOARD_LILYGO_TDISPLAY) && \
    !defined(BOARD_LILYGO_TQT) && !defined(BOARD_M5STICKC_PLUS) && \
    !defined(BOARD_M5STACK_CORE) && !defined(BOARD_M5STACK_CORE2) && \
    !defined(BOARD_SUNTON_2432S028) && !defined(BOARD_WAVESHARE_S3_ROUND) && \
    !defined(BOARD_LILYGO_T_HMI) && !defined(BOARD_LILYGO_TDISPLAY_S3_AMOLED) && \
    !defined(BOARD_LILYGO_TWATCH_2020) && !defined(BOARD_LILYGO_TWATCH_S3) && \
    !defined(BOARD_LILYGO_TDECK)
#define BOARD_LILYGO_TDISPLAY_S3 1
#endif

// ============================================================================
// Board selection — pulls in the matching board header and defines the
// display class + geometry used by Display.cpp.
// ============================================================================
#if defined(BOARD_LILYGO_TDISPLAY_S3)
#include "boards/lilygo_tdisplay_s3.h"
#define BOARD_DISPLAY_CLASS   LGFX_TDISPLAY_S3
#define BOARD_DISPLAY_WIDTH   320
#define BOARD_DISPLAY_HEIGHT  170
#define BOARD_DISPLAY_ROTATION 1
#define BOARD_HAS_BACKLIGHT   1

#elif defined(BOARD_LILYGO_TDISPLAY_S3_AMOLED)
#include "boards/lilygo_tdisplay_s3_amoled.h"
#define BOARD_DISPLAY_CLASS   LGFX_TDISPLAY_S3_AMOLED
#define BOARD_DISPLAY_WIDTH   240
#define BOARD_DISPLAY_HEIGHT  536
#define BOARD_DISPLAY_ROTATION 0
#define BOARD_HAS_BACKLIGHT   0

#elif defined(BOARD_LILYGO_TDISPLAY)
#include "boards/lilygo_tdisplay.h"
#define BOARD_DISPLAY_CLASS   LGFX_TDISPLAY
#define BOARD_DISPLAY_WIDTH   240
#define BOARD_DISPLAY_HEIGHT  135
#define BOARD_DISPLAY_ROTATION 0
#define BOARD_HAS_BACKLIGHT   1

#elif defined(BOARD_LILYGO_TQT)
#include "boards/lilygo_tqt.h"
#define BOARD_DISPLAY_CLASS   LGFX_TQT
#define BOARD_DISPLAY_WIDTH   128
#define BOARD_DISPLAY_HEIGHT  128
#define BOARD_DISPLAY_ROTATION 0
#define BOARD_HAS_BACKLIGHT   1

#elif defined(BOARD_LILYGO_T_HMI)
#include "boards/lilygo_t_hmi.h"
#define BOARD_DISPLAY_CLASS   LGFX_T_HMI
#define BOARD_DISPLAY_WIDTH   320
#define BOARD_DISPLAY_HEIGHT  240
#define BOARD_DISPLAY_ROTATION 0
#define BOARD_HAS_BACKLIGHT   1

#elif defined(BOARD_M5STICKC_PLUS)
#include "boards/m5stickc_plus.h"
#define BOARD_DISPLAY_CLASS   LGFX_M5STICKC_PLUS
#define BOARD_DISPLAY_WIDTH   240
#define BOARD_DISPLAY_HEIGHT  135
#define BOARD_DISPLAY_ROTATION 1
#define BOARD_HAS_BACKLIGHT   1

#elif defined(BOARD_M5STACK_CORE)
#include "boards/m5stack_core.h"
#define BOARD_DISPLAY_CLASS   LGFX_M5STACK_CORE
#define BOARD_DISPLAY_WIDTH   320
#define BOARD_DISPLAY_HEIGHT  240
#define BOARD_DISPLAY_ROTATION 1
#define BOARD_HAS_BACKLIGHT   1

#elif defined(BOARD_M5STACK_CORE2)
#include "boards/m5stack_core2.h"
#define BOARD_DISPLAY_CLASS   LGFX_M5STACK_CORE2
#define BOARD_DISPLAY_WIDTH   320
#define BOARD_DISPLAY_HEIGHT  240
#define BOARD_DISPLAY_ROTATION 1
#define BOARD_HAS_BACKLIGHT   1

#elif defined(BOARD_SUNTON_2432S028)
#include "boards/sunton_2432s028.h"
#define BOARD_DISPLAY_CLASS   LGFX_SUNTON_2432S028
#define BOARD_DISPLAY_WIDTH   320
#define BOARD_DISPLAY_HEIGHT  240
#define BOARD_DISPLAY_ROTATION 1
#define BOARD_HAS_BACKLIGHT   1

#elif defined(BOARD_WAVESHARE_S3_ROUND)
#include "boards/waveshare_s3_round.h"
#define BOARD_DISPLAY_CLASS   LGFX_WAVESHARE_S3_ROUND
#define BOARD_DISPLAY_WIDTH   240
#define BOARD_DISPLAY_HEIGHT  240
#define BOARD_DISPLAY_ROTATION 0
#define BOARD_HAS_BACKLIGHT   1

#elif defined(BOARD_LILYGO_TWATCH_2020)
#include "boards/lilygo_twatch_2020.h"
#define BOARD_DISPLAY_CLASS   LGFX_TWATCH_2020
#define BOARD_DISPLAY_WIDTH   240
#define BOARD_DISPLAY_HEIGHT  240
#define BOARD_DISPLAY_ROTATION 0
#define BOARD_HAS_BACKLIGHT   1
#define BOARD_NEEDS_PMU_INIT  1

#elif defined(BOARD_LILYGO_TWATCH_S3)
#include "boards/lilygo_twatch_s3.h"
#define BOARD_DISPLAY_CLASS   LGFX_TWATCH_S3
#define BOARD_DISPLAY_WIDTH   240
#define BOARD_DISPLAY_HEIGHT  240
#define BOARD_DISPLAY_ROTATION 0
#define BOARD_HAS_BACKLIGHT   1

#elif defined(BOARD_LILYGO_TDECK)
#include "boards/lilygo_tdeck.h"
#define BOARD_DISPLAY_CLASS   LGFX_TDECK
#define BOARD_DISPLAY_WIDTH   320
#define BOARD_DISPLAY_HEIGHT  240
#define BOARD_DISPLAY_ROTATION 0
#define BOARD_HAS_BACKLIGHT   1
#define BOARD_NEEDS_POWER_INIT 1

#else
#error "No BOARD_* flag defined. Add one in platformio.ini."
#endif

// A friendly board name string for the splash / about screen.
#if defined(BOARD_LILYGO_TDISPLAY_S3)
#define BOARD_NAME "LilyGO T-Display-S3"
#elif defined(BOARD_LILYGO_TDISPLAY_S3_AMOLED)
#define BOARD_NAME "LilyGO T-Display-S3 AMOLED"
#elif defined(BOARD_LILYGO_TDISPLAY)
#define BOARD_NAME "LilyGO T-Display"
#elif defined(BOARD_LILYGO_TQT)
#define BOARD_NAME "LilyGO T-QT"
#elif defined(BOARD_LILYGO_T_HMI)
#define BOARD_NAME "LilyGO T-HMI"
#elif defined(BOARD_M5STICKC_PLUS)
#define BOARD_NAME "M5StickC Plus"
#elif defined(BOARD_M5STACK_CORE)
#define BOARD_NAME "M5Stack Core/Basic"
#elif defined(BOARD_M5STACK_CORE2)
#define BOARD_NAME "M5Stack Core2"
#elif defined(BOARD_SUNTON_2432S028)
#define BOARD_NAME "Sunton ESP32-2432S028"
#elif defined(BOARD_WAVESHARE_S3_ROUND)
#define BOARD_NAME "Waveshare S3 Round LCD"
#elif defined(BOARD_LILYGO_TWATCH_2020)
#define BOARD_NAME "LilyGO T-Watch 2020"
#elif defined(BOARD_LILYGO_TWATCH_S3)
#define BOARD_NAME "LilyGO T-Watch S3"
#elif defined(BOARD_LILYGO_TDECK)
#define BOARD_NAME "LilyGO T-Deck"
#endif
