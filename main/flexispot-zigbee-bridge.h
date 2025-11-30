#pragma once

#include "esp_zigbee_core.h"


#define TAG "FLEXISPOT_ZB_BRIDGE"

/* Zigbee configuration */
#define INSTALLCODE_POLICY_ENABLE       false    /* enable the install code policy for security */
#define ED_AGING_TIMEOUT                ESP_ZB_ED_AGING_TIMEOUT_64MIN
#define ED_KEEP_ALIVE                   3000    /* 3000 millisecond */

#define ZB_CHANNEL_MASK(ch) (1UL << (ch))
#define ESP_ZB_PRIMARY_CHANNEL_MASK_ALL   ( \
    ZB_CHANNEL_MASK(11) | ZB_CHANNEL_MASK(12) | ZB_CHANNEL_MASK(13) | ZB_CHANNEL_MASK(14) | \
    ZB_CHANNEL_MASK(15) | ZB_CHANNEL_MASK(16) | ZB_CHANNEL_MASK(17) | ZB_CHANNEL_MASK(18) | \
    ZB_CHANNEL_MASK(19) | ZB_CHANNEL_MASK(20) | ZB_CHANNEL_MASK(21) | ZB_CHANNEL_MASK(22) | \
    ZB_CHANNEL_MASK(23) | ZB_CHANNEL_MASK(24) | ZB_CHANNEL_MASK(25) | ZB_CHANNEL_MASK(26)   \
)

#define ESP_ZB_PRIMARY_CHANNEL_MASK     ZB_CHANNEL_MASK(25)  /* Zigbee primary channel derived from home assistant config */
#define ESP_ZB_SECONDARY_CHANNEL_MASK   ESP_ZB_PRIMARY_CHANNEL_MASK_ALL  /* Zigbee secondary channel find all */

#define ENDPOINT_ID 1

#define ENDPOINT_CONFIG()                                           \
    {                                                               \
        .endpoint = ENDPOINT_ID,                                    \
        .app_device_id = ESP_ZB_ZCL_CLUSTER_ID_ANALOG_VALUE,        \
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,                  \
        .app_device_version = 0                                     \
    }                                                               \

/* Basic manufacturer information */
#define ESP_MANUFACTURER_NAME "\x09" "MAXBAS"      /* Customized manufacturer name */
#define ESP_MODEL_IDENTIFIER "\x07" "FLX_BRDG" /* Customized model identifier */

#define ESP_ZB_ZED_CONFIG()                                         \
    {                                                               \
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED,                       \
        .install_code_policy = INSTALLCODE_POLICY_ENABLE,           \
        .nwk_cfg.zed_cfg = {                                        \
            .ed_timeout = ED_AGING_TIMEOUT,                         \
            .keep_alive = ED_KEEP_ALIVE,                            \
        },                                                          \
    }

#define ESP_ZB_DEFAULT_RADIO_CONFIG()                               \
    {                                                               \
        .radio_mode = ZB_RADIO_MODE_NATIVE,                         \
        .radio_uart_config = {}                                     \
    }

#define ESP_ZB_DEFAULT_HOST_CONFIG()                                \
    {                                                               \
        .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE,       \
        .host_uart_config = {}                                      \
    }
