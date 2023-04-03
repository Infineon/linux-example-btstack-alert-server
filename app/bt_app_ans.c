/******************************************************************************
 * (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
 *******************************************************************************
 * This software, including source code, documentation and related materials
 * ("Software"), is owned by Cypress Semiconductor Corporation or one of its
 * subsidiaries ("Cypress") and is protected by and subject to worldwide patent
 * protection (United States and foreign), United States copyright laws and
 * international treaty provisions. Therefore, you may use this Software only
 * as provided in the license agreement accompanying the software package from
 * which you obtained this Software ("EULA").
 *
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software source
 * code solely for use in connection with Cypress's integrated circuit products.
 * Any reproduction, modification, translation, compilation, or representation
 * of this Software except as specified above is prohibited without the express
 * written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer of such
 * system or application assumes all risk of such use and in doing so agrees to
 * indemnify Cypress against all liability.
 *******************************************************************************/
/******************************************************************************
 * File Name: bt_app_ans.c
 *
 * Description:
 * This is the source code for the LE Alert Notification Server Example
 *
 * Related Document: See README.md
 *
 *******************************************************************************/

/*******************************************************************************
 *                                   INCLUDES
 *******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "wiced_memory.h"
#include "wiced_bt_stack.h"
#include "wiced_bt_dev.h"
#include "wiced_memory.h"
#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_gatt_util.h"
#include "wiced_bt_cfg.h"
#include "wiced_bt_uuid.h"
#include "wiced_bt_trace.h"
#include "wiced_hal_nvram.h"
#include "wiced_bt_stack_platform.h"
#include "app_bt_utils/app_bt_utils.h"
#include "COMPONENT_ans/wiced_bt_anp.h"
#include "COMPONENT_ans/wiced_bt_ans.h"
#include "COMPONENT_ans/wiced_bt_gatt_util.h"
#include "app_bt_config/ans_gatt_db.h"
#include "app_bt_config/ans_bt_settings.h"
#include "app_bt_config/ans_gap.h"
#include "bt_app_ans.h"

/*******************************************************************************
 *                                   MACROS
 *******************************************************************************/
#define BT_STACK_HEAP_SIZE ( 0xF000 )
#define ANS_LOCAL_KEYS_NVRAM_ID WICED_NVRAM_VSID_START
#define ANS_PAIRED_KEYS_NVRAM_ID ( WICED_NVRAM_VSID_START + 1 )
#define ANS_CLIENT_NAME "ANC"
#define MAX_KEY_SIZE ( 0x10U )

/*******************************************************************************
 *                    STRUCTURES AND ENUMERATIONS
 *******************************************************************************/
typedef struct
{
    uint16_t conn_id;
    wiced_bt_anp_alert_category_enable_t current_enabled_alert_cat;
} bt_app_ans_cb_t; /* Application control block */

/******************************************************************************
 *                                EXTERNS
 ******************************************************************************/
extern wiced_bt_heap_t *p_default_heap;

/*******************************************************************************
 *                           GLOBAL VARIABLES
 *******************************************************************************/
const char *p_ans_client_name = ANS_CLIENT_NAME;
bt_app_ans_cb_t ans_app_cb; /* Application Control block */

/*******************************************************************************
 *                           FUNCTION DECLARATIONS
 *******************************************************************************/
static wiced_result_t bt_app_ans_management_callback(wiced_bt_management_evt_t event,
                                                     wiced_bt_management_evt_data_t *p_event_data);
static void bt_app_ans_scan_result_cback(wiced_bt_ble_scan_results_t *p_scan_result,
                                         uint8_t *p_adv_data);
static void bt_app_ans_connection_up(wiced_bt_gatt_connection_status_t *p_conn_status);
static void bt_app_ans_connection_down(wiced_bt_gatt_connection_status_t *p_conn_status);
static wiced_bt_gatt_status_t bt_app_ans_gatts_req_callback(wiced_bt_gatt_attribute_request_t *p_data);
static wiced_bt_gatt_status_t bt_app_ans_gatts_req_read_handler(wiced_bt_gatt_opcode_t opcode, uint16_t conn_id,
                                                                wiced_bt_gatt_read_t *p_read_data,
                                                                uint16_t len_requested);
static wiced_bt_gatt_status_t bt_app_ans_gatts_req_write_handler(wiced_bt_gatt_opcode_t opcode, uint16_t conn_id,
                                                                 wiced_bt_gatt_write_req_t *p_data);
static wiced_bt_gatt_status_t bt_app_ans_gatts_callback(wiced_bt_gatt_evt_t event,
                                                        wiced_bt_gatt_event_data_t *p_data);
static void bt_app_ans_load_keys_to_addr_resolution_db(void);
static wiced_bool_t bt_app_ans_save_link_keys(wiced_bt_device_link_keys_t *p_keys);
static wiced_bool_t bt_app_ans_read_link_keys(wiced_bt_device_link_keys_t *p_keys);
static gatt_db_lookup_table_t *bt_app_ans_find_attr_by_handle(uint16_t handle);

/*******************************************************************************
 *                       FUNCTION DEFINITIONS
 *******************************************************************************/

/*******************************************************************************
 * Function Name: application_start
 ********************************************************************************
 * Summary:
 *  Set device configuration and start BT stack initialization. The actual
 *  application initialization will happen when stack reports that BT device
 *  is ready.
 *
 * Parameters: NONE
 *
 * Return: NONE
 *
 *******************************************************************************/
void application_start()
{
    wiced_result_t wiced_result;

    WICED_BT_TRACE("Bluetooth Alert Server Application\n");

    memset(&ans_app_cb, 0, sizeof(ans_app_cb));

    /* Register call back and configuration with stack */
    wiced_result = wiced_bt_stack_init(bt_app_ans_management_callback, &wiced_bt_cfg_settings);

    /* Check if stack initialization was successful */
    if (WICED_BT_SUCCESS == wiced_result)
    {
        WICED_BT_TRACE("Bluetooth Stack Initialization Successful \n");
        /* Create default heap */
        p_default_heap = wiced_bt_create_heap("default_heap", NULL, BT_STACK_HEAP_SIZE, NULL, WICED_TRUE);
        if (p_default_heap == NULL)
        {
            WICED_BT_TRACE("Create default heap error: size %d\n", BT_STACK_HEAP_SIZE);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        WICED_BT_TRACE("Bluetooth Stack Initialization failed!! \n");
        exit(EXIT_FAILURE);
    }
}

/*******************************************************************************
 * Function Name: ans_application_init
 ********************************************************************************
 * Summary:
 *   This function handles application level initialization tasks and is called
 *   from the BT management callback once the LE stack enabled event
 *   (BTM_ENABLED_EVT) is triggered This function is executed in the
 *   BTM_ENABLED_EVT management callback.
 *
 * Parameters:
 *   None
 *
 * Return:
 *  None
 *
 *******************************************************************************/
void ans_application_init(void)
{
    wiced_bt_gatt_status_t gatt_status;
    wiced_result_t result;
    wiced_bt_ans_gatt_handles_t gatt_handles =
        {
            .new_alert =
                {
                    .supported_category = HDLC_ANS_SUPPORTED_NEW_ALERT_CATEGORY_VALUE,
                    .value = HDLC_ANS_NEW_ALERT_VALUE,
                    .configuration = HDLD_ANS_NEW_ALERT_CLIENT_CHAR_CONFIG,
                },
            .unread_alert =
                {
                    .supported_category = HDLC_ANS_SUPPORTED_UNREAD_ALERT_CATEGORY_VALUE,
                    .value = HDLC_ANS_UNREAD_ALERT_STATUS_VALUE,
                    .configuration = HDLD_ANS_UNREAD_ALERT_STATUS_CLIENT_CHAR_CONFIG,
                },
            .notification_control = HDLC_ANS_ALERT_NOTIFICATION_CONTROL_POINT_VALUE,
        };

    /* Initialize WICED BT ANS library */
    result = wiced_bt_ans_init(&gatt_handles);
    if (result != WICED_BT_SUCCESS)
        WICED_BT_TRACE("Err: wiced_bt_ans_init failed status:%d\n", result);

    /* Register with stack to receive GATT callback */
    gatt_status = wiced_bt_gatt_register(bt_app_ans_gatts_callback);

    WICED_BT_TRACE("wiced_bt_gatt_register: %d\n", gatt_status);

    /*  Tell stack to use our GATT databse */
    gatt_status = wiced_bt_gatt_db_init(gatt_database, gatt_database_len, NULL);

    WICED_BT_TRACE("wiced_bt_gatt_db_init %d\n", gatt_status);

    /* Allow peer to pair */
    wiced_bt_set_pairable_mode(WICED_TRUE, 0);

    /* Load the address resolution DB with the keys stored in the NVRAM */
    bt_app_ans_load_keys_to_addr_resolution_db();

    /* Currently application demonstrates,
     * simple alerts, email and SMS or MMS categories*/
    ans_app_cb.current_enabled_alert_cat = ANP_ALERT_CATEGORY_ENABLE_SIMPLE_ALERT |
                                           ANP_ALERT_CATEGORY_ENABLE_EMAIL |
                                           ANP_ALERT_CATEGORY_ENABLE_SMS_OR_MMS;

    /* tell to ANS library on current supported categories */
    wiced_bt_ans_set_supported_new_alert_categories(0, ans_app_cb.current_enabled_alert_cat);
    wiced_bt_ans_set_supported_unread_alert_categories(0, ans_app_cb.current_enabled_alert_cat);
}

/*******************************************************************************
 * Function Name: bt_app_ans_management_callback
 ********************************************************************************
 * Summary:
 *   This is a Bluetooth stack event handler function to receive management
 *   events from the LE stack and process as per the application.
 *
 * Parameters:
 *   wiced_bt_management_evt_t event             : LE event code of one byte
 *                                                 length
 *   wiced_bt_management_evt_data_t *p_event_data: Pointer to LE management
 *                                                 event structures
 *
 * Return:
 *  wiced_result_t: Error code from WICED_RESULT_LIST or BT_RESULT_LIST
 *
 *******************************************************************************/
wiced_result_t bt_app_ans_management_callback(wiced_bt_management_evt_t event,
                                              wiced_bt_management_evt_data_t *p_event_data)
{
    wiced_bt_device_address_t bda = {0};
    wiced_bt_ble_advert_mode_t *p_adv_mode = NULL;
    wiced_result_t result = WICED_BT_SUCCESS;
    wiced_bt_device_link_keys_t link_keys;
    uint8_t *p_keys;

    WICED_BT_TRACE("Bluetooth Management Event: 0x%x %s\n", event, (char *)get_bt_event_name(event));
    switch (event)
    {
    case BTM_ENABLED_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        /* Bluetooth Controller and Host Stack Enabled */
        if (WICED_BT_SUCCESS == p_event_data->enabled.status)
        {
            wiced_bt_set_local_bdaddr(ans_bd_address, BLE_ADDR_PUBLIC);
            wiced_bt_dev_read_local_addr(bda);

            WICED_BT_TRACE("Bluetooth Enabled \n");
            WICED_BT_TRACE("Local Bluetooth Address: ");
            print_bd_address(bda);

            /* Perform application-specific initialization */
            ans_application_init();
        }
        else
        {
            WICED_BT_TRACE("Bluetooth Enabling Failed \n");
        }
        break;

    case BTM_DISABLED_EVT:
        WICED_BT_TRACE("Bluetooth Disabled \n");
        break;

    case BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        p_event_data->pairing_io_capabilities_ble_request.local_io_cap = BTM_IO_CAPABILITIES_NONE;
        p_event_data->pairing_io_capabilities_ble_request.oob_data = BTM_OOB_NONE;
        p_event_data->pairing_io_capabilities_ble_request.auth_req = BTM_LE_AUTH_REQ_SC_BOND;
        p_event_data->pairing_io_capabilities_ble_request.max_key_size = MAX_KEY_SIZE;
        p_event_data->pairing_io_capabilities_ble_request.init_keys =
            BTM_LE_KEY_PENC | BTM_LE_KEY_PID | BTM_LE_KEY_PCSRK | BTM_LE_KEY_LENC;
        p_event_data->pairing_io_capabilities_ble_request.resp_keys =
            BTM_LE_KEY_PENC | BTM_LE_KEY_PID | BTM_LE_KEY_PCSRK | BTM_LE_KEY_LENC;
        break;

    case BTM_PAIRING_COMPLETE_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        WICED_BT_TRACE("Pairing Complete: %d", p_event_data->pairing_complete.pairing_complete_info.ble.reason);
        break;

    case BTM_ENCRYPTION_STATUS_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        WICED_BT_TRACE("Encryption Status Event: bd (%B) res %d", p_event_data->encryption_status.bd_addr,
                       p_event_data->encryption_status.result);
        break;

    case BTM_SECURITY_REQUEST_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        wiced_bt_ble_security_grant(p_event_data->security_request.bd_addr, WICED_BT_SUCCESS);
        break;

    case BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        bt_app_ans_save_link_keys(&p_event_data->paired_device_link_keys_update);
        break;

    case BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        p_keys = (uint8_t *)&p_event_data->paired_device_link_keys_request;
        result = bt_app_ans_save_link_keys(&link_keys);
        /* Break if link key retrieval is failed or link key is not available. */
        if (result != WICED_SUCCESS)
        {
            result = WICED_BT_ERROR;
            WICED_BT_TRACE("\n Reading keys from NVRAM failed or link key not available.");
            break;
        }

        WICED_BT_TRACE("Keys read from NVRAM ");
        WICED_BT_TRACE("Result: %d \n", result);

        /* Compare the BDA */
        if (memcmp(&(link_keys.bd_addr), &(p_event_data->paired_device_link_keys_request.bd_addr),
                   sizeof(wiced_bt_device_address_t)) == 0)
        {
            memcpy(p_keys, (uint8_t *)&link_keys, sizeof(wiced_bt_device_link_keys_t));
            result = WICED_SUCCESS;
        }
        else
        {
            result = WICED_BT_ERROR;
            WICED_BT_TRACE("Key retrieval failure\n");
        }
        break;

    case BTM_LOCAL_IDENTITY_KEYS_UPDATE_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        /* save keys to NVRAM */
        p_keys = (uint8_t *)&p_event_data->local_identity_keys_update;
        wiced_hal_write_nvram(ANS_LOCAL_KEYS_NVRAM_ID, sizeof(wiced_bt_local_identity_keys_t), p_keys, &result);
        WICED_BT_TRACE("Local keys save to NVRAM result: %d \n", result);
        break;

    case BTM_LOCAL_IDENTITY_KEYS_REQUEST_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        /* read keys from NVRAM */
        p_keys = (uint8_t *)&p_event_data->local_identity_keys_request;
        wiced_hal_read_nvram(ANS_LOCAL_KEYS_NVRAM_ID, sizeof(wiced_bt_local_identity_keys_t), p_keys, &result);
        WICED_BT_TRACE("Local keys read from NVRAM result: %d \n", result);
        break;

    case BTM_BLE_SCAN_STATE_CHANGED_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        WICED_BT_TRACE("Scan State Change: %d\n", p_event_data->ble_scan_state_changed);
        break;

    case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        /* Advertisement State Changed */
        p_adv_mode = &p_event_data->ble_advert_state_changed;
        WICED_BT_TRACE("Advertisement State Change: %s\n", get_bt_advert_mode_name(*p_adv_mode));

        if (BTM_BLE_ADVERT_OFF == *p_adv_mode)
        {
            /* Advertisement Stopped */
            WICED_BT_TRACE("Advertisement stopped\n");
        }
        else
        {
            /* Advertisement Started */
            WICED_BT_TRACE("Advertisement started\n");
        }
        break;

    case BTM_BLE_CONNECTION_PARAM_UPDATE:
        if (p_event_data == NULL)
        {
            WICED_BT_TRACE("Callback data pointer p_event_data is NULL \n");
            break;
        }
        WICED_BT_TRACE("Connection parameter update status:%d, Connection Interval: %d, \
                                       Connection Latency: %d, Connection Timeout: %d\n",
                       p_event_data->ble_connection_param_update.status,
                       p_event_data->ble_connection_param_update.conn_interval,
                       p_event_data->ble_connection_param_update.conn_latency,
                       p_event_data->ble_connection_param_update.supervision_timeout);
        break;

    default:
        // WICED_BT_TRACE("Unhandled Bluetooth Management Event: 0x%x %s\n", event, get_bt_event_name(event));
        break;
    }

    return result;
}

/*******************************************************************************
 * Function Name: bt_app_ans_scan_result_cback
 ********************************************************************************
 * Summary:
 *   This function handles the scan results and attempt to connect to ANS client.
 *
 * Parameters:
 *   p_scan_result :Result with details after Scanning
 *   p_adv_data    : Pointer to Advertising data
 *
 * Return:
 *  None
 *
 *******************************************************************************/
void bt_app_ans_scan_result_cback(wiced_bt_ble_scan_results_t *p_scan_result, uint8_t *p_adv_data)
{
    wiced_result_t status;
    wiced_bool_t ret_status;
    uint8_t length;
    uint8_t *p_data;

    if ((p_scan_result) && (p_adv_data != NULL))
    {
        /* Advertisement data from clinet should have client complete name. */
        p_data = wiced_bt_ble_check_advertising_data(p_adv_data, BTM_BLE_ADVERT_TYPE_NAME_COMPLETE, &length);

        /* Check if  the client name is there in the advertisement. */
        if ((p_data == NULL) || (length != strlen(p_ans_client_name)) ||
            (memcmp(p_data, p_ans_client_name, strlen(p_ans_client_name)) != 0))
        {
            return;
        }

        WICED_BT_TRACE("Found ANS client : %B \n", p_scan_result->remote_bd_addr);

        /* Stop the scan since the desired device is found */
        status = wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_NONE, WICED_TRUE, bt_app_ans_scan_result_cback);

        WICED_BT_TRACE("Scan off status %d\n", status);

        /* Initiate the connection */
        ret_status = wiced_bt_gatt_le_connect(p_scan_result->remote_bd_addr, p_scan_result->ble_addr_type,
                                              BLE_CONN_MODE_HIGH_DUTY, TRUE);

        WICED_BT_TRACE("wiced_bt_gatt_connect status %d\n", ret_status);
    }
    else
    {
        WICED_BT_TRACE("Scan completed.\n");
    }
}

/*******************************************************************************
 * Function Name: bt_app_ans_gatts_callback
 ********************************************************************************
 * Summary:
 *   Callback for various GATT events.  As this application performs only as a
 *   GATT server, some of the events are omitted.
 *
 * Parameters:
 *   wiced_bt_gatt_evt_t event                : LE GATT event code of one
 *                                              byte length
 *   wiced_bt_gatt_event_data_t *p_event_data : Pointer to LE GATT event
 *                                              structures
 *
 * Return:
 *  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
 *  in wiced_bt_gatt.h
 *
 *******************************************************************************/
static wiced_bt_gatt_status_t bt_app_ans_gatts_callback(wiced_bt_gatt_evt_t event,
                                                        wiced_bt_gatt_event_data_t *p_data)
{
    wiced_bt_gatt_status_t result = WICED_BT_GATT_SUCCESS;

    if (p_data == NULL)
    {
        WICED_BT_TRACE("GATT Server Callback Event Data is pointing to NULL \n");
        return result;
    }
    switch (event)
    {
    case GATT_CONNECTION_STATUS_EVT:
        if (p_data->connection_status.connected)
        {
            bt_app_ans_connection_up(&p_data->connection_status);
        }
        else
        {
            bt_app_ans_connection_down(&p_data->connection_status);
        }
        break;

    case GATT_ATTRIBUTE_REQUEST_EVT:
        result = bt_app_ans_gatts_req_callback(&p_data->attribute_request);
        break;

    default:
        result = WICED_BT_GATT_SUCCESS;
        break;
    }

    return result;
}

/*******************************************************************************
* Function Name: bt_app_ans_connection_up
********************************************************************************
* Summary:
*   This function will be called when a connection is established
*
* Parameters:
*   p_conn_status  : Current status of the Connection

* Return:
*  None
*
*******************************************************************************/
void bt_app_ans_connection_up(wiced_bt_gatt_connection_status_t *p_conn_status)
{
    wiced_bt_device_link_keys_t keys;
    wiced_result_t result;
    wiced_bt_ble_sec_action_type_t sec_act = BTM_BLE_SEC_ENCRYPT;

    if (p_conn_status->connected == TRUE)
    {
        WICED_BT_TRACE("Connected to ANC \n");

        ans_app_cb.conn_id = p_conn_status->conn_id;

        /* Need to notify ANP Server library that the connection is up */
        wiced_bt_ans_connection_up(p_conn_status->conn_id);

        /* if the peer already paired with us initiate encryption instead waiting client to
        initiate*/
        if (bt_app_ans_save_link_keys(&keys))
        {
            if (!memcmp(p_conn_status->bd_addr, keys.bd_addr, LOCAL_BDA_LEN))
            {
                result = wiced_bt_dev_set_encryption(keys.bd_addr, BT_TRANSPORT_LE, &sec_act);
                WICED_BT_TRACE("Start Encryption %B %d \n", keys.bd_addr, result);
            }
        }
    }
    else
    {
        WICED_BT_TRACE("Connection to ANC failed \n");
    }
}

/*******************************************************************************
* Function Name: bt_app_ans_connection_down
********************************************************************************
* Summary:
*   This function will be called when connection goes down
*
* Parameters:
*   p_conn_status  : Current status of the Connection

* Return:
*  None
*
*******************************************************************************/
void bt_app_ans_connection_down(wiced_bt_gatt_connection_status_t *p_conn_status)
{
    WICED_BT_TRACE("Disconnected from ANC \n");

    /* tell library that connection is down */
    wiced_bt_ans_connection_down(p_conn_status->conn_id);

    ans_app_cb.conn_id = 0;
}

/*******************************************************************************
 * Function Name: bt_app_ans_gatts_req_callback
 ********************************************************************************
 * Summary:
 *   Process GATT request from the peer. Even our main goal is to be a GATT client for the
 *   ANCS service, we need to support mandatory GATT procedures.
 *
 * Parameters:
 *  p_attr_req     Pointer to LE GATT connection status
 *
 * Return:
 *  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
 *  in wiced_bt_gatt.h
 *
 *******************************************************************************/
static wiced_bt_gatt_status_t bt_app_ans_gatts_req_callback(wiced_bt_gatt_attribute_request_t *p_attr_req)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_ERROR;
    if (NULL != p_attr_req)
    {
        switch (p_attr_req->opcode)
        {
        case GATT_REQ_READ:
        case GATT_REQ_READ_BLOB:
            /* Attribute read request */
            gatt_status = bt_app_ans_gatts_req_read_handler(p_attr_req->opcode, p_attr_req->conn_id,
                                                            &(p_attr_req->data.read_req),
                                                            p_attr_req->len_requested);
            break;

        case GATT_REQ_WRITE:
        case GATT_CMD_WRITE:
            /* Attribute write request */
            gatt_status = bt_app_ans_gatts_req_write_handler(p_attr_req->opcode, p_attr_req->conn_id,
                                                             &(p_attr_req->data.write_req));
            break;

        case GATT_HANDLE_VALUE_NOTIF:
            WICED_BT_TRACE("GATT_HANDLE_VALUE_NOTIF received for Notification Sent \n");
            break;

        default:
            WICED_BT_TRACE("ERROR: Unhandled GATT GATT_ATTRIBUTE_REQUEST_EVT Opcode: 0x%x\n", p_attr_req->opcode);
            break;
        }
    }
    return gatt_status;
}

/*******************************************************************************
 * Function Name: bt_app_ans_gatts_req_read_handler
 ********************************************************************************
 * Summary:
 *   This function handles Read Requests received from the client device
 *
 * Parameters:
 *   conn_id       Connection ID
 *   opcode        LE GATT request type opcode
 *   p_read_req    Pointer to read request containing the handle to read
 *   len_req       length of data requested
 *
 * Return:
 *   wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
 *   in wiced_bt_gatt.h
 *
 ********************************************************************************/
wiced_bt_gatt_status_t bt_app_ans_gatts_req_read_handler(wiced_bt_gatt_opcode_t opcode,
                                                         uint16_t conn_id,
                                                         wiced_bt_gatt_read_t *p_data,
                                                         uint16_t len_requested)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;
    int attr_len_to_copy;
    uint8_t *from;
    int to_send;
    gatt_db_lookup_table_t *attrRec;

    WICED_BT_TRACE("Read_handler: conn_id:%d hdl:0x%x offset:%d \n ", conn_id, p_data->handle, p_data->offset);
    if (NULL == (attrRec = bt_app_ans_find_attr_by_handle(p_data->handle)))
    {
        wiced_bt_gatt_server_send_error_rsp(conn_id, opcode, p_data->handle,
                                            WICED_BT_GATT_INVALID_HANDLE);
        WICED_BT_TRACE("Invalid Handle \n");
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    /* ANP server library takes care reading of ANS service characteristics */
    if ((p_data->handle >= HDLS_ANS) && (p_data->handle <= HDLC_ANS_ALERT_NOTIFICATION_CONTROL_POINT_VALUE))
    {
        WICED_BT_TRACE("Calling profile read\n");
        gatt_status = wiced_bt_ans_process_gatt_read_req(conn_id, p_data, attrRec->p_data,
                                                         &attrRec->cur_len);
        if (gatt_status != WICED_BT_GATT_SUCCESS)
        {
            wiced_bt_gatt_server_send_error_rsp(conn_id, opcode, p_data->handle, gatt_status);
            return gatt_status;
        }
    }

    attr_len_to_copy = attrRec->cur_len;
    if (p_data->offset >= attrRec->cur_len)
    {
        wiced_bt_gatt_server_send_error_rsp(conn_id, opcode, p_data->handle,
                                            WICED_BT_GATT_INVALID_OFFSET);
        gatt_status = WICED_BT_GATT_INVALID_OFFSET;
    }
    else
    {
        to_send = attr_len_to_copy - p_data->offset;
        from = ((uint8_t *)attrRec->p_data) + p_data->offset;
        /* No need for context, as buff not allocated */
        WICED_BT_TRACE("Sending Response....\n");
        gatt_status = wiced_bt_gatt_server_send_read_handle_rsp(conn_id, opcode, to_send, from, NULL);
    }
    WICED_BT_TRACE("Returning read_handler: conn_id:%d hdl:0x%x offset:%d \n ",
                   conn_id, p_data->handle, p_data->offset);
    return gatt_status;
}

/*******************************************************************************
 * Function Name : bt_app_ans_find_attr_by_handle
 * *****************************************************************************
 * Summary :
 *    Find attribute description by handle
 *
 * Parameters:
 *    handle:    handle to look up
 *
 * Return:
 *    gatt_db_lookup_table_t:   pointer containing handle data
 ******************************************************************************/
static gatt_db_lookup_table_t *bt_app_ans_find_attr_by_handle(uint16_t handle)
{
    int i;
    for (i = 0; i < app_gatt_db_ext_attr_tbl_size; i++)
    {
        if (handle == app_gatt_db_ext_attr_tbl[i].handle)
        {
            return (&app_gatt_db_ext_attr_tbl[i]);
        }
    }
    return NULL;
}

/*******************************************************************************
 * Function Name : bt_app_ans_gatts_req_write_handler
 * *****************************************************************************
 * Summary :
 *    Process write request or write command from peer device
 *
 * Parameters:
 *  conn_id       Connection ID
 *  opcode        LE GATT request type opcode
 *  p_data        Pointer to write request containing the data to write
 *
 * Return:
 *  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
 *  in wiced_bt_gatt.h
 ******************************************************************************/
wiced_bt_gatt_status_t bt_app_ans_gatts_req_write_handler(wiced_bt_gatt_opcode_t opcode,
                                                          uint16_t conn_id,
                                                          wiced_bt_gatt_write_req_t *p_data)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;
    WICED_BT_TRACE("Write_handler: conn_id:%d hdl:0x%x offset:%d len:%d \n ", conn_id, p_data->handle,
                   p_data->offset, p_data->val_len);

    /* ANP server library takes care writing to ANS service characteristics */
    if ((p_data->handle >= HDLS_ANS) && (p_data->handle <= HDLC_ANS_ALERT_NOTIFICATION_CONTROL_POINT_VALUE))
    {
        gatt_status = wiced_bt_ans_process_gatt_write_req(conn_id, p_data);

        if (gatt_status == WICED_BT_GATT_SUCCESS)
        {
            gatt_status = wiced_bt_gatt_server_send_write_rsp(conn_id, opcode, p_data->handle);
        }
        else
        {
            gatt_status = wiced_bt_gatt_server_send_error_rsp(conn_id, opcode, p_data->handle, gatt_status);
        }
    }
    else
    {
        gatt_status = WICED_BT_GATT_INVALID_HANDLE;
    }

    return gatt_status;
}

/*******************************************************************************
 * Function Name : bt_app_ans_start_scan_connect
 * *****************************************************************************
 * Summary :
 *    Does BLE Scan and connects to the device that is advertising as ANC
 *
 * Parameters:
 *    None
 *
 * Return:
 *    uint16_t: Status of the operation. Refer to list of wiced_result_t.
 ******************************************************************************/
uint16_t bt_app_ans_start_scan_connect(void)
{
    wiced_result_t result = WICED_BT_BUSY;
    /* Start scan to find ANS Client */
    if ((wiced_bt_ble_get_current_scan_state() == BTM_BLE_SCAN_TYPE_NONE) &&
        (ans_app_cb.conn_id == 0))
    {
        result = wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_HIGH_DUTY, WICED_TRUE, bt_app_ans_scan_result_cback);
        WICED_BT_TRACE("BLE Scan Start Status: %d \n", result);
        if (WICED_BT_PENDING)
        {
            result = WICED_BT_SUCCESS;
        }
    }
    return result;
}

/*******************************************************************************
 * Function Name : bt_app_ans_load_keys_to_addr_resolution_db
 * *****************************************************************************
 * Summary :
 *    Read keys from the NVRAM and update address resolution database
 *
 * Parameters:
 *    None
 *
 * Return:
 *    None
 ******************************************************************************/
void bt_app_ans_load_keys_to_addr_resolution_db(void)
{
    uint8_t bytes_read;
    wiced_result_t result;
    wiced_bt_device_link_keys_t keys;

    bytes_read = wiced_hal_read_nvram(ANS_PAIRED_KEYS_NVRAM_ID, sizeof(keys), (uint8_t *)&keys, &result);

    WICED_BT_TRACE(" [%s] Read status %d bytes read %d \n", __FUNCTION__, result, bytes_read);

    /* if failed to read NVRAM, there is nothing saved at that location */
    if (result == WICED_SUCCESS)
    {
        result = wiced_bt_dev_add_device_to_address_resolution_db(&keys);
    }
}

/*******************************************************************************
 * Function Name : bt_app_ans_save_link_keys
 * *****************************************************************************
 * Summary :
 *    This function is called to save keys generated as a result of pairing
 *    or keys update
 *
 * Parameters:
 *    p_keys: Link keys to be saved for the bonded device
 *
 * Return:
 *    wiced_bool_t: 1 if True and 0 if false
 ******************************************************************************/
wiced_bool_t bt_app_ans_save_link_keys(wiced_bt_device_link_keys_t *p_keys)
{
    uint8_t bytes_written;
    wiced_result_t result;

    bytes_written = wiced_hal_write_nvram(ANS_PAIRED_KEYS_NVRAM_ID, sizeof(wiced_bt_device_link_keys_t),
                                          (uint8_t *)p_keys, &result);
    WICED_BT_TRACE("Saved %d bytes at id:%d \n", bytes_written, ANS_PAIRED_KEYS_NVRAM_ID);
    return (bytes_written == sizeof(wiced_bt_device_link_keys_t));
}

/*******************************************************************************
 * Function Name : bt_app_ans_read_link_keys
 * *****************************************************************************
 * Summary :
 *    This function is called to read keys for specific bdaddr
 *
 * Parameters:
 *    p_keys: Link keys to be saved for the bonded device
 *
 * Return:
 *    wiced_bool_t: 1 if True and 0 if false
 ******************************************************************************/
wiced_bool_t bt_app_ans_read_link_keys(wiced_bt_device_link_keys_t *p_keys)
{
    uint8_t bytes_read;
    wiced_result_t result;

    bytes_read = wiced_hal_read_nvram(ANS_PAIRED_KEYS_NVRAM_ID, sizeof(wiced_bt_device_link_keys_t),
                                      (uint8_t *)p_keys, &result);
    WICED_BT_TRACE("Read %d bytes at id:%d \n", bytes_read, ANS_PAIRED_KEYS_NVRAM_ID);
    return (bytes_read == sizeof(wiced_bt_device_link_keys_t));
}

/*******************************************************************************
 * Function Name : bt_app_ans_handle_set_supported_new_alert_categories
 * *****************************************************************************
 * Summary :
 *    This function is sets the selected new alert categories
 *
 * Parameters:
 *    p_data: 2 byte data containing the Bit fields pertaining to the Categories
 *    length: length of the data
 *
 * Return:
 *    uint16_t: See possible status codes in wiced_bt_gatt_status_e
 *  in wiced_bt_gatt.h
 ******************************************************************************/
uint16_t bt_app_ans_handle_set_supported_new_alert_categories(uint16_t p_data, uint8_t length)
{
    uint16_t conn_id = ans_app_cb.conn_id;
    uint16_t supported_new_alert_cat;
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (ans_app_cb.conn_id != 0)
    {
        WICED_BT_TRACE("Request not supported: ANS connected with ANC \n");
        return WICED_BT_GATT_WRONG_STATE;
    }

    if (length == 2)
    {
        WICED_BT_TRACE("Setting New Alert Categories..... \n");
        supported_new_alert_cat = p_data;
        /* Make sure user sets choice only in supported categories */

        /* supported_new_alert_cat &= ans_app_cb.current_enabled_alert_cat;*/
        wiced_bt_ans_set_supported_new_alert_categories(conn_id, supported_new_alert_cat);
    }
    else
    {
        status = WICED_BT_GATT_ILLEGAL_PARAMETER;
    }

    return status;
}

/*******************************************************************************
 * Function Name : bt_app_ans_handle_set_supported_unread_alert_categories
 * *****************************************************************************
 * Summary :
 *    This function is sets the selected unread alert categories
 *
 * Parameters:
 *    p_data: 2 byte data containing the Bit fields pertaining to the Categories
 *    length: length of the data
 *
 * Return:
 *    uint16_t: See possible status codes in wiced_bt_gatt_status_e
 *  in wiced_bt_gatt.h
 ******************************************************************************/
uint16_t bt_app_ans_handle_set_supported_unread_alert_categories(uint16_t p_data, uint8_t length)
{
    uint16_t conn_id = ans_app_cb.conn_id;
    uint16_t supported_unread_alert_cat = 0;
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;

    if (ans_app_cb.conn_id != 0)
    {
        WICED_BT_TRACE("Request not supported: ANS connected with ANC \n");
        return WICED_BT_GATT_WRONG_STATE;
    }
    if (length == 2)
    {
        WICED_BT_TRACE("Setting Unread Alert Categories..... \n");
        supported_unread_alert_cat = p_data;

        /* Make sure user sets choice only in supported categories */
        /*supported_unread_alert_cat &= ans_app_cb.current_enabled_alert_cat; */
        wiced_bt_ans_set_supported_unread_alert_categories(conn_id, supported_unread_alert_cat);
    }
    else
    {
        gatt_status = WICED_BT_GATT_ILLEGAL_PARAMETER;
    }

    return gatt_status;
}

/*******************************************************************************
 * Function Name : bt_app_ans_handle_generate_alert
 * *****************************************************************************
 * Summary :
 *    This function generates alert in the chosen category depending on the
 *    Alert Control point written by the Client
 *
 * Parameters:
 *    p_data: 1 byte data containing alert category
 *    len: length of the data
 *
 * Return:
 *    uint16_t: See possible status codes in wiced_bt_gatt_status_e
 *  in wiced_bt_gatt.h
 ******************************************************************************/
uint16_t bt_app_ans_handle_generate_alert(uint8_t p_data, uint8_t len)
{
    uint16_t conn_id = ans_app_cb.conn_id;
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;

    if (ans_app_cb.conn_id == 0)
    {
        WICED_BT_TRACE("Generate alert failed: Service not connected \n");
        gatt_status = WICED_BT_GATT_WRONG_STATE;
    }
    else if (len == 1)
    {
        gatt_status = wiced_bt_ans_process_and_send_new_alert(conn_id, p_data);
        if (gatt_status == WICED_BT_GATT_SUCCESS)
        {
            gatt_status = wiced_bt_ans_process_and_send_unread_alert(conn_id, p_data);
            if (gatt_status != WICED_BT_GATT_SUCCESS)
            {
                WICED_BT_TRACE("Unread Alert Send Error %d \n", gatt_status);
            }
        }
        else
        {
            WICED_BT_TRACE("New Alert Send Error %d \n", gatt_status);
        }
    }
    else
    {
        gatt_status = WICED_BT_GATT_ILLEGAL_PARAMETER;
    }

    return gatt_status;
}

/*******************************************************************************
 * Function Name : bt_app_ans_handle_clear_alert
 * *****************************************************************************
 * Summary :
 *    This function clears alert in the chosen category
 *
 * Parameters:
 *    p_data: 1 byte data containing alert category
 *    len: length of the data
 *
 * Return:
 *    uint16_t: See possible status codes in wiced_bt_gatt_status_e
 *  in wiced_bt_gatt.h
 ******************************************************************************/
uint16_t bt_app_ans_handle_clear_alert(uint8_t p_data, uint8_t len)
{
    uint16_t conn_id = ans_app_cb.conn_id;
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;

    if (len == 1)
    {
        if (wiced_bt_ans_clear_alerts(conn_id, p_data) != WICED_TRUE)
        {
            gatt_status = WICED_BT_GATT_ERROR;
        }
    }
    else
    {
        gatt_status = WICED_BT_GATT_ILLEGAL_PARAMETER;
    }

    return gatt_status;
}

/*******************************************************************************
 * Function Name : bt_app_ans_disconnect
 * *****************************************************************************
 * Summary :
 *    This function clears alert in the chosen category
 *
 * Parameters:
 *    None
 *
 * Return:
 *    uint16_t: See possible status codes in wiced_bt_gatt_status_e
 *  in wiced_bt_gatt.h
 ******************************************************************************/
uint16_t bt_app_ans_disconnect()
{
    return wiced_bt_gatt_disconnect(ans_app_cb.conn_id);
}

/* END OF FILE [] */
