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
* BLE device's GATT database and device configuration header file.
*
******************************************************************************/

#if !defined(CYCFG_GATT_DB_H)
#define CYCFG_GATT_DB_H

#include "stdint.h"

#define __UUID_SERVICE_GENERIC_ACCESS                      0x1800
#define __UUID_CHARACTERISTIC_DEVICE_NAME                  0x2A00
#define __UUID_CHARACTERISTIC_APPEARANCE                   0x2A01
#define __UUID_SERVICE_GENERIC_ATTRIBUTE                   0x1801
#define __UUID_SERVICE_ALERT_NOTIFICATION                  0x1811
#define __UUID_CHARACTERISTIC_SUPPORTED_NEW_ALERT_CATEGORY    0x2A47
#define __UUID_CHARACTERISTIC_NEW_ALERT                    0x2A46
#define __UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION    0x2902
#define __UUID_CHARACTERISTIC_SUPPORTED_UNREAD_ALERT_CATEGORY    0x2A48
#define __UUID_CHARACTERISTIC_UNREAD_ALERT_STATUS          0x2A45
#define __UUID_CHARACTERISTIC_ALERT_NOTIFICATION_CONTROL_POINT    0x2A44

/* Service Generic Access */
#define HDLS_GAP                                           0x0001
/* Characteristic Device Name */
#define HDLC_GAP_DEVICE_NAME                               0x0002
#define HDLC_GAP_DEVICE_NAME_VALUE                         0x0003
/* Characteristic Appearance */
#define HDLC_GAP_APPEARANCE                                0x0004
#define HDLC_GAP_APPEARANCE_VALUE                          0x0005

/* Service Generic Attribute */
#define HDLS_GATT                                          0x0006

/* Service Alert Notification */
#define HDLS_ANS                                           0x0007
/* Characteristic Supported New Alert Category */
#define HDLC_ANS_SUPPORTED_NEW_ALERT_CATEGORY              0x0008
#define HDLC_ANS_SUPPORTED_NEW_ALERT_CATEGORY_VALUE        0x0009
/* Characteristic New Alert */
#define HDLC_ANS_NEW_ALERT                                 0x000A
#define HDLC_ANS_NEW_ALERT_VALUE                           0x000B
/* Descriptor Client Characteristic Configuration */
#define HDLD_ANS_NEW_ALERT_CLIENT_CHAR_CONFIG              0x000C
/* Characteristic Supported Unread Alert Category */
#define HDLC_ANS_SUPPORTED_UNREAD_ALERT_CATEGORY           0x000D
#define HDLC_ANS_SUPPORTED_UNREAD_ALERT_CATEGORY_VALUE     0x000E
/* Characteristic Unread Alert Status */
#define HDLC_ANS_UNREAD_ALERT_STATUS                       0x000F
#define HDLC_ANS_UNREAD_ALERT_STATUS_VALUE                 0x0010
/* Descriptor Client Characteristic Configuration */
#define HDLD_ANS_UNREAD_ALERT_STATUS_CLIENT_CHAR_CONFIG    0x0011
/* Characteristic Alert Notification Control Point */
#define HDLC_ANS_ALERT_NOTIFICATION_CONTROL_POINT          0x0012
#define HDLC_ANS_ALERT_NOTIFICATION_CONTROL_POINT_VALUE    0x0013

/* External Lookup Table Entry */
typedef struct
{
    uint16_t handle;
    uint16_t max_len;
    uint16_t cur_len;
    uint8_t  *p_data;
} gatt_db_lookup_table_t;

/* External definitions */
extern const uint8_t  gatt_database[];
extern const uint16_t gatt_database_len;
extern gatt_db_lookup_table_t app_gatt_db_ext_attr_tbl[];
extern const uint16_t app_gatt_db_ext_attr_tbl_size;
extern uint8_t app_gap_device_name[];
extern const uint16_t app_gap_device_name_len;
extern uint8_t app_gap_appearance[];
extern const uint16_t app_gap_appearance_len;
extern uint8_t app_ans_supported_new_alert_category[];
extern const uint16_t app_ans_supported_new_alert_category_len;
extern uint8_t app_ans_new_alert[];
extern const uint16_t app_ans_new_alert_len;
extern uint8_t app_ans_new_alert_client_char_config[];
extern const uint16_t app_ans_new_alert_client_char_config_len;
extern uint8_t app_ans_supported_unread_alert_category[];
extern const uint16_t app_ans_supported_unread_alert_category_len;
extern uint8_t app_ans_unread_alert_status[];
extern const uint16_t app_ans_unread_alert_status_len;
extern uint8_t app_ans_unread_alert_status_client_char_config[];
extern const uint16_t app_ans_unread_alert_status_client_char_config_len;
extern uint8_t app_ans_alert_notification_control_point[];
extern const uint16_t app_ans_alert_notification_control_point_len;

#endif /* CYCFG_GATT_DB_H */
