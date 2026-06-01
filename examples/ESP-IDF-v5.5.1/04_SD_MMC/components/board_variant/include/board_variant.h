#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BOARD_VARIANT_UNKNOWN = 0,
    BOARD_VARIANT_SH8601_FT3168,
    BOARD_VARIANT_CO5300_CST816,
} board_variant_t;

board_variant_t board_variant_detect(void);
const char *board_variant_to_name(board_variant_t variant);

#ifdef __cplusplus
}
#endif
