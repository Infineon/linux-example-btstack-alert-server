/******************************************************************************
 * (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
 ******************************************************************************
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
 *****************************************************************************/
/******************************************************************************
* File Name: ans_gatt_db.c
*
* Description:
* BLE device's GATT database and device configuration.
*
******************************************************************************/

#include "ans_gatt_db.h"
#include "wiced_bt_uuid.h"
#include "wiced_bt_gatt.h"

/*************************************************************************************
* GATT server definitions
*************************************************************************************/

const uint8_t gatt_database[] = 
{
    /* Primary Service: Generic Access */
    PRIMARY_SERVICE_UUID16 (HDLS_GAP, __UUID_SERVICE_GENERIC_ACCESS),
        /* Characteristic: Device Name */
        CHARACTERISTIC_UUID16 (HDLC_GAP_DEVICE_NAME, HDLC_GAP_DEVICE_NAME_VALUE, __UUID_CHARACTERISTIC_DEVICE_NAME, GATTDB_CHAR_PROP_READ, GATTDB_PERM_READABLE),
        /* Characteristic: Appearance */
        CHARACTERISTIC_UUID16 (HDLC_GAP_APPEARANCE, HDLC_GAP_APPEARANCE_VALUE, __UUID_CHARACTERISTIC_APPEARANCE, GATTDB_CHAR_PROP_READ, GATTDB_PERM_READABLE),

    /* Primary Service: Generic Attribute */
    PRIMARY_SERVICE_UUID16 (HDLS_GATT, __UUID_SERVICE_GENERIC_ATTRIBUTE),

    /* Primary Service: Alert Notification */
    PRIMARY_SERVICE_UUID16 (HDLS_ANS, __UUID_SERVICE_ALERT_NOTIFICATION),
        /* Characteristic: Supported New Alert Category */
        CHARACTERISTIC_UUID16 (HDLC_ANS_SUPPORTED_NEW_ALERT_CATEGORY, HDLC_ANS_SUPPORTED_NEW_ALERT_CATEGORY_VALUE, __UUID_CHARACTERISTIC_SUPPORTED_NEW_ALERT_CATEGORY, GATTDB_CHAR_PROP_READ, GATTDB_PERM_READABLE),
        /* Characteristic: New Alert */
        CHARACTERISTIC_UUID16 (HDLC_ANS_NEW_ALERT, HDLC_ANS_NEW_ALERT_VALUE, __UUID_CHARACTERISTIC_NEW_ALERT, GATTDB_CHAR_PROP_NOTIFY, GATTDB_PERM_NONE),
            /* Descriptor: Client Characteristic Configuration */
            CHAR_DESCRIPTOR_UUID16_WRITABLE (HDLD_ANS_NEW_ALERT_CLIENT_CHAR_CONFIG, __UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION, GATTDB_PERM_READABLE | GATTDB_PERM_WRITE_REQ),
        /* Characteristic: Supported Unread Alert Category */
        CHARACTERISTIC_UUID16 (HDLC_ANS_SUPPORTED_UNREAD_ALERT_CATEGORY, HDLC_ANS_SUPPORTED_UNREAD_ALERT_CATEGORY_VALUE, __UUID_CHARACTERISTIC_SUPPORTED_UNREAD_ALERT_CATEGORY, GATTDB_CHAR_PROP_READ, GATTDB_PERM_READABLE),
        /* Characteristic: Unread Alert Status */
        CHARACTERISTIC_UUID16 (HDLC_ANS_UNREAD_ALERT_STATUS, HDLC_ANS_UNREAD_ALERT_STATUS_VALUE, __UUID_CHARACTERISTIC_UNREAD_ALERT_STATUS, GATTDB_CHAR_PROP_NOTIFY, GATTDB_PERM_NONE),
            /* Descriptor: Client Characteristic Configuration */
            CHAR_DESCRIPTOR_UUID16_WRITABLE (HDLD_ANS_UNREAD_ALERT_STATUS_CLIENT_CHAR_CONFIG, __UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION, GATTDB_PERM_READABLE | GATTDB_PERM_WRITE_REQ),
        /* Characteristic: Alert Notification Control Point */
        CHARACTERISTIC_UUID16_WRITABLE (HDLC_ANS_ALERT_NOTIFICATION_CONTROL_POINT, HDLC_ANS_ALERT_NOTIFICATION_CONTROL_POINT_VALUE, __UUID_CHARACTERISTIC_ALERT_NOTIFICATION_CONTROL_POINT, GATTDB_CHAR_PROP_WRITE, GATTDB_PERM_WRITE_REQ),
};

/* Length of the GATT database */
const uint16_t gatt_database_len = sizeof(gatt_database);

/*************************************************************************************
 * GATT Initial Value Arrays
 ************************************************************************************/
 
uint8_t app_gap_device_name[]                            = {'A', 'N', 'S', '\0', };
uint8_t app_gap_appearance[]                             = {0x00, 0x00, };
uint8_t app_ans_supported_new_alert_category[]           = {0x00, 0x00, };
uint8_t app_ans_new_alert[]                              = {0x00, 0x00, };
uint8_t app_ans_new_alert_client_char_config[]           = {0x00, 0x00, };
uint8_t app_ans_supported_unread_alert_category[]        = {0x00, 0x00, };
uint8_t app_ans_unread_alert_status[]                    = {0x00, 0x00, };
uint8_t app_ans_unread_alert_status_client_char_config[] = {0x00, 0x00, };
uint8_t app_ans_alert_notification_control_point[]       = {0x00, 0x00, };
 
 /************************************************************************************
 * GATT Lookup Table
 ************************************************************************************/
 
gatt_db_lookup_table_t app_gatt_db_ext_attr_tbl[] =
{
    /* { attribute handle,                             maxlen, curlen, attribute data } */
    { HDLC_GAP_DEVICE_NAME_VALUE,                      3,      3,      app_gap_device_name },
    { HDLC_GAP_APPEARANCE_VALUE,                       2,      2,      app_gap_appearance },
    { HDLC_ANS_SUPPORTED_NEW_ALERT_CATEGORY_VALUE,     2,      2,      app_ans_supported_new_alert_category },
    { HDLC_ANS_NEW_ALERT_VALUE,                        2,      2,      app_ans_new_alert },
    { HDLD_ANS_NEW_ALERT_CLIENT_CHAR_CONFIG,           2,      2,      app_ans_new_alert_client_char_config },
    { HDLC_ANS_SUPPORTED_UNREAD_ALERT_CATEGORY_VALUE,  2,      2,      app_ans_supported_unread_alert_category },
    { HDLC_ANS_UNREAD_ALERT_STATUS_VALUE,              2,      2,      app_ans_unread_alert_status },
    { HDLD_ANS_UNREAD_ALERT_STATUS_CLIENT_CHAR_CONFIG, 2,      2,      app_ans_unread_alert_status_client_char_config },
    { HDLC_ANS_ALERT_NOTIFICATION_CONTROL_POINT_VALUE, 2,      2,      app_ans_alert_notification_control_point },
};

/* Number of Lookup Table entries */
const uint16_t app_gatt_db_ext_attr_tbl_size = (sizeof(app_gatt_db_ext_attr_tbl) / sizeof(gatt_db_lookup_table_t));

/* Number of GATT initial value arrays entries */
const uint16_t app_gap_device_name_len = 3;
const uint16_t app_gap_appearance_len = (sizeof(app_gap_appearance));
const uint16_t app_ans_supported_new_alert_category_len = (sizeof(app_ans_supported_new_alert_category));
const uint16_t app_ans_new_alert_len = (sizeof(app_ans_new_alert));
const uint16_t app_ans_new_alert_client_char_config_len = (sizeof(app_ans_new_alert_client_char_config));
const uint16_t app_ans_supported_unread_alert_category_len = (sizeof(app_ans_supported_unread_alert_category));
const uint16_t app_ans_unread_alert_status_len = (sizeof(app_ans_unread_alert_status));
const uint16_t app_ans_unread_alert_status_client_char_config_len = (sizeof(app_ans_unread_alert_status_client_char_config));
const uint16_t app_ans_alert_notification_control_point_len = (sizeof(app_ans_alert_notification_control_point));
