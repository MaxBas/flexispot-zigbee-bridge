#include "flexispot-zigbee-bridge.h"

#include "esp_err.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Add storage for the Analog Value PresentValue so the stack has a place to write
static float esp_zb_analog_value_present_value = 0;


static esp_err_t zb_set_attr_handler(const esp_zb_zcl_set_attr_value_message_t *msg)
{
//    ESP_RETURN_ON_FALSE(msg, ESP_FAIL, TAG, "Set attribute message is null");
//    ESP_RETURN_ON_FALSE(msg->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_FAIL, TAG, "Set attribute failed, status: 0x%x", msg->info.status);

    if (msg->info.dst_endpoint != ZB_EP_ID_HEIGHT)
    {
//        ESP_LOGW(TAG, "Invalid Endpoint ID: 0x%x", msg->info.dst_endpoint);
        return ESP_FAIL;
    }
    switch (msg->info.cluster)
    {
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_VALUE:
        switch (msg->attribute.id)
        {
            case ESP_ZB_ZCL_ATTR_ANALOG_VALUE_PRESENT_VALUE_ID:
                if (msg->attribute.data.size == sizeof(float) && msg->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_SINGLE)
                {
                    float new_height = 0;
                    memcpy(&new_height, msg->attribute.data.value, sizeof(float));
//                    ESP_LOGI(TAG, "Height present value set to: %u", new_height);

                    /*
                    esp_zb_zcl_report_attr_cmd_t report = {
                    .zcl_basic_cmd.dst_addr_u.addr_short = 0x0000, // Coordinator short address
                    .zcl_basic_cmd.src_endpoint = ZB_EP_ID_HEIGHT,
                    .zcl_basic_cmd.dst_endpoint = ZB_EP_ID_HEIGHT,
                    .clusterID = ESP_ZB_ZCL_CLUSTER_ID_ANALOG_VALUE,
                    .address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                    .direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV
                    };

                    esp_zb_lock_acquire(portMAX_DELAY);
                    esp_zb_zcl_report_attr_cmd_req(&report);
                    esp_zb_lock_release();
                    */

                    // TODO: Add code to handle the new height value
                } else
                {
                    ESP_LOGW(TAG, "Invalid size for Present Value attribute: %u", msg->attribute.data.size);
                }
                break;
            default:
                ESP_LOGW(TAG, "Unhandled Analog Value attribute ID: 0x%x", msg->attribute.id);
                break;
        }
        break;

    default:
        ESP_LOGW(TAG, "Invalid Cluster ID: 0x%x", msg->info.cluster);
        break;
    }

    return ESP_OK;
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;

    switch (callback_id)
    {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
        const esp_zb_zcl_set_attr_value_message_t *set_attr_value_msg = (esp_zb_zcl_set_attr_value_message_t *)message;
        ret = zb_set_attr_handler(set_attr_value_msg);
        break;

    default:
        ESP_LOGW(TAG, "Unhandled Zigbee action ID: 0x%x", callback_id);
        break;
    }

    return ret;
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_s)
{
    const esp_err_t err_status = signal_s->esp_err_status;
    const uint32_t *p_app_signal = signal_s->p_app_signal;

    const esp_zb_app_signal_type_t sig_type = *p_app_signal;

    switch (sig_type)
    {
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status != ESP_OK)
        {

        } else
        {
            // Personal Area Network ID (PAN ID)
            esp_zb_ieee_addr_t extended_pan_address;
            esp_zb_get_extended_pan_id(extended_pan_address);

            ESP_LOGI(TAG, "Zigbee network started, Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                     extended_pan_address[7], extended_pan_address[6], extended_pan_address[5], extended_pan_address[4],
                     extended_pan_address[3], extended_pan_address[2], extended_pan_address[1], extended_pan_address[0]);


        }
        break;

    default:
        ESP_LOGI(TAG, "Unhandled ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type, esp_err_to_name(err_status));
        break;
    }
}

static void esp_zb_task(void *pvParameters)
{
    // init Zigbee stack
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    esp_zb_init(&zb_nwk_cfg);

    // basic cluster - obligatory, contains basic information about the device
    uint16_t esp_zb_zcl_basic_power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE;
    uint8_t esp_zb_zcl_basic_zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE;

    esp_zb_attribute_list_t *esp_zb_basic_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_BASIC);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, ESP_MANUFACTURER_NAME);     // doesn't need update since value is a compile time char array pointer
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, ESP_MODEL_IDENTIFIER);       // doesn't need update since value is a compile time char array pointer

    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, &esp_zb_zcl_basic_zcl_version);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID, &esp_zb_zcl_basic_power_source);

    esp_zb_cluster_update_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, &esp_zb_zcl_basic_zcl_version);
    esp_zb_cluster_update_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID, &esp_zb_zcl_basic_power_source);

    // identify cluster - obligatory
    esp_zb_attribute_list_t *esp_zb_identify_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY);
    esp_zb_identify_cluster_add_attr(esp_zb_identify_cluster, ESP_ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID, ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE);

    // client roles
    esp_zb_attribute_list_t *esp_zb_identify_client_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY);

    // server roles
    esp_zb_attribute_list_t *esp_zb_analog_value_server_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_ANALOG_VALUE);

    // Register the PresentValue attribute so write (set attribute) requests are handled by the stack
    esp_zb_analog_value_cluster_add_attr(esp_zb_analog_value_server_cluster, ESP_ZB_ZCL_ATTR_ANALOG_VALUE_PRESENT_VALUE_ID, &esp_zb_analog_value_present_value);
    esp_zb_cluster_update_attr(esp_zb_analog_value_server_cluster, ESP_ZB_ZCL_ATTR_ANALOG_VALUE_PRESENT_VALUE_ID, &esp_zb_analog_value_present_value);

    // endpoint cluster list
    esp_zb_cluster_list_t *esp_zb_cluster_list = esp_zb_zcl_cluster_list_create();
    esp_zb_cluster_list_add_basic_cluster(esp_zb_cluster_list, esp_zb_basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_identify_cluster(esp_zb_cluster_list, esp_zb_identify_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_identify_cluster(esp_zb_cluster_list, esp_zb_identify_client_cluster, ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);
    esp_zb_cluster_list_add_analog_value_cluster(esp_zb_cluster_list, esp_zb_analog_value_server_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    esp_zb_ep_list_t *esp_zb_ep_list = esp_zb_ep_list_create();
    const esp_zb_endpoint_config_t endpoint_config = ENDPOINT_CONFIG();

    esp_zb_ep_list_add_ep(esp_zb_ep_list, esp_zb_cluster_list, endpoint_config);
    esp_zb_device_register(esp_zb_ep_list);
    esp_zb_core_action_handler_register(zb_action_handler);

    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    esp_zb_set_secondary_network_channel_set(ESP_ZB_SECONDARY_CHANNEL_MASK);

    ESP_ERROR_CHECK(esp_zb_start(true));
    esp_zb_stack_main_loop();
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting");
    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG()
    };
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));
    xTaskCreate(esp_zb_task, "Zigbee_main", 2048, NULL, 5, nullptr);
}
