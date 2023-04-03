/*
 * Copyright 2016-2022, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
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
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 *
 * This file Implements library interface for Alert notification profile server
 */

#include "wiced_bt_anp.h"
#include "wiced_bt_ans.h"
#include "wiced_bt_trace.h"
#include "string.h"

#define ANS_STATE_DISCONNECTED 0
#define ANS_STATE_CONNECTED 1

#ifdef WICED_BT_TRACE_ENABLE
#define ANS_TRACE_DBG(format, ...) WICED_BT_TRACE("%s: " format, __FUNCTION__, ##__VA_ARGS__)
#define ANS_TRACE_ERR(format, ...) WICED_BT_TRACE("ERR: %s: " format, __FUNCTION__, ##__VA_ARGS__)
#else
#define ANS_TRACE_DBG(...)
#define ANS_TRACE_ERR(...)
#endif

/* Alerts count */
typedef struct
{
    uint8_t num_of_new_alerts;   /* New alerts count */
    uint8_t num_of_unread_count; /* Unread alerts count */
} notify_data_cb_t;

typedef struct
{
    uint16_t conn_id; /* connection identifier */

    uint16_t supported_new_alerts; /* Server supportable new alerts. Cannot be changed during connection
                                      Each alert category represented using single bit.
                                      wiced_bt_anp_alert_category_enable_t tells the bit index for different alerts */

    uint16_t supported_unread_alerts; /* Server supportable unread alerts. Cannot be changed during connection
                                         Each alert category represented using single bit.
                                          wiced_bt_anp_alert_category_enable_t tells the bit index for different alerts */

    uint16_t client_configured_new_alerts; /* Client configured new alerts.
                                              Each alert category represented using single bit.
                                              wiced_bt_anp_alert_category_enable_t tells the bit index for different alerts */

    uint16_t client_configured_unread_alerts; /* Client configured alerts.
                                                 Each alert category represented using single bit.
                                                 wiced_bt_anp_alert_category_enable_t tells the bit index for different alerts */

    uint16_t new_alert_cccd;           /* New alerts client cfg desc */
    uint16_t unread_alert_status_cccd; /* Unread alerts client cfg desc */

    notify_data_cb_t notify_data[ANP_NOTIFY_CATEGORY_COUNT]; /* New alerts, unread alerts count */

    uint16_t new_alert_not_sent; /* bitmask to tell new alert count changed but not updated to client for the category. wiced_bt_anp_alert_category_enable_t tells the bit index for different alerts */

    uint16_t unread_alert_status_not_sent; /* bitmask to tell unread alert count changed but not updated to client for the category. wiced_bt_anp_alert_category_enable_t tells the bit index for different alerts */

    uint8_t state; /* ANS library current state */

    wiced_bt_ans_gatt_handles_t gatt_handles; /* Alert GATT handles */
} ans_lib_cb_t;

/* ANS library control block */
ans_lib_cb_t ans_lib_cb;

const char *ans_lib_new_alert_sample_text_str[ANP_NOTIFY_CATEGORY_COUNT] =
    {
        "simple_alert", /*Title ofsimple alert*/
        "john",         /* Sender of Email */
        "IOT era",      /* Title of the news feed */
        "Bob",          /* Caller name : Incoming call*/
        "Alice",        /* Caller Name :Missed call category*/
        "Dev",          /* Sender Name: SMS/MMS category */
        "George",       /*Sender Name: SMS/MMS: Voice mail category*/
        "status_meet",  /* Title of the schedule: Schedule alert category*/
        "Virus",        /* High Prioritized Aler - Title of the alert category */
        "Mary",         /*Sender name: Instant message category*/
};

wiced_bt_gatt_status_t ans_lib_send_new_alert(uint16_t conn_id, uint8_t category_id)
{
    wiced_bt_gatt_status_t status;
    uint8_t new_alert[20];

    uint16_t val_len = 2 + strlen(ans_lib_new_alert_sample_text_str[category_id]);

    if (val_len > 20)
        val_len = 20;

    new_alert[0] = category_id;
    new_alert[1] = ans_lib_cb.notify_data[category_id].num_of_new_alerts;
    memcpy(&new_alert[2], ans_lib_new_alert_sample_text_str[category_id], val_len - 2);

    status = wiced_bt_gatt_server_send_notification(conn_id, ans_lib_cb.gatt_handles.new_alert.value, val_len, new_alert, NULL);
    if (status == WICED_BT_GATT_SUCCESS)
    {
        ans_lib_cb.new_alert_not_sent &= (~(1 << category_id));
    }

    ANS_TRACE_DBG("cat:%d status:%x \n", category_id, status);

    return status;
}

wiced_bt_gatt_status_t ans_lib_send_unread_alert(uint16_t conn_id, uint8_t category_id)
{
    wiced_bt_gatt_status_t status;
    uint8_t unread_alert[2];

    unread_alert[0] = category_id;
    unread_alert[1] = ans_lib_cb.notify_data[category_id].num_of_unread_count;

    status = wiced_bt_gatt_server_send_notification(conn_id, ans_lib_cb.gatt_handles.unread_alert.value, 2, unread_alert, NULL);
    if (status == WICED_BT_GATT_SUCCESS)
    {
        ans_lib_cb.unread_alert_status_not_sent &= (~(1 << category_id));
    }

    ANS_TRACE_DBG("cat:%d status:%x \n", category_id, status);

    return status;
}

void ans_lib_handle_new_alert_immediate_notify(uint16_t conn_id, uint8_t category_id)
{
    if (ans_lib_cb.new_alert_cccd)
    {
        if ((category_id != 0xFF) &&
            (ans_lib_cb.client_configured_new_alerts & (1 << category_id)) &&
            (ans_lib_cb.new_alert_not_sent & (1 << category_id)))
        {
            ans_lib_send_new_alert(conn_id, category_id);
        }
        else if (category_id == 0xFF)
        {
            uint8_t cat;
            for (cat = 0; cat < ANP_NOTIFY_CATEGORY_COUNT; cat++)
            {
                if ((ans_lib_cb.client_configured_new_alerts & (1 << cat)) &&
                    (ans_lib_cb.new_alert_not_sent & (1 << cat)))
                {
                    ans_lib_send_new_alert(conn_id, cat);
                }
            }
        }
    }
}

void ans_lib_handle_unread_alert_immediate_notify(uint16_t conn_id, uint8_t category_id)
{
    if (ans_lib_cb.unread_alert_status_cccd)
    {
        if ((category_id != 0xFF) &&
            (ans_lib_cb.client_configured_unread_alerts & (1 << category_id)) &&
            (ans_lib_cb.unread_alert_status_not_sent & (1 << category_id)))
        {
            ans_lib_send_unread_alert(conn_id, category_id);
        }
        else if (category_id == 0xFF)
        {
            uint8_t cat;
            for (cat = 0; cat < ANP_NOTIFY_CATEGORY_COUNT; cat++)
            {
                if ((ans_lib_cb.client_configured_unread_alerts & (1 << cat)) &&
                    (ans_lib_cb.unread_alert_status_not_sent & (1 << cat)))
                {
                    ans_lib_send_unread_alert(conn_id, cat);
                }
            }
        }
    }
}

wiced_bt_gatt_status_t ans_lib_handle_client_alert_notification_control_point_write(uint16_t conn_id, wiced_bt_anp_alert_control_cmd_id_t cmd_id,
                                                                                    wiced_bt_anp_alert_category_id_t category_id)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if ((category_id != 0xFF) && (category_id >= ANP_NOTIFY_CATEGORY_COUNT))
    {
        if (cmd_id > ANP_ALERT_CONTROL_CMD_NOTIFY_UNREAD_ALERTS_IMMEDIATE)
        {
            ANS_TRACE_ERR("Wrong command ID: %d\n", cmd_id);
            return ANP_ALERT_NOTIFCATION_CONTROL_POINT_WRITE_CMD_NOT_SUPPORTED;
        }
        return WICED_BT_GATT_INVALID_CFG;
    }

    switch (cmd_id)
    {
    case ANP_ALERT_CONTROL_CMD_ENABLE_NEW_ALERTS:
        if (category_id == 0xFF)
        {
            ans_lib_cb.client_configured_new_alerts = ans_lib_cb.supported_new_alerts;
        }
        else
        {
            ans_lib_cb.client_configured_new_alerts |= (1 << category_id);
        }
        break;

    case ANP_ALERT_CONTROL_CMD_ENABLE_UNREAD_STATUS:
        if (category_id == 0xFF)
        {
            ans_lib_cb.client_configured_unread_alerts = ans_lib_cb.supported_unread_alerts;
        }
        else
        {
            ans_lib_cb.client_configured_unread_alerts |= (1 << category_id);
        }
        break;

    case ANP_ALERT_CONTROL_CMD_DISABLE_NEW_ALERTS:
        if (category_id == 0xFF)
        {
            ans_lib_cb.client_configured_new_alerts = 0;
        }
        else
        {
            ans_lib_cb.client_configured_new_alerts &= (~(1 << category_id));
        }
        break;

    case ANP_ALERT_CONTROL_CMD_DISABLE_UNREAD_ALERTS:
        if (category_id == 0xFF)
        {
            ans_lib_cb.client_configured_unread_alerts = 0;
        }
        else
        {
            ans_lib_cb.client_configured_unread_alerts &= (~(1 << category_id));
        }
        break;

    case ANP_ALERT_CONTROL_CMD_NOTIFY_NEW_ALERTS_IMMEDIATE:
        ans_lib_handle_new_alert_immediate_notify(conn_id, category_id);
        break;

    case ANP_ALERT_CONTROL_CMD_NOTIFY_UNREAD_ALERTS_IMMEDIATE:
        ans_lib_handle_unread_alert_immediate_notify(conn_id, category_id);
        break;

    default:
        status = ANP_ALERT_NOTIFCATION_CONTROL_POINT_WRITE_CMD_NOT_SUPPORTED;
        break;
    }

    if (status == WICED_BT_GATT_SUCCESS)
    {
        ANS_TRACE_DBG("control_point_write:sup na:%x, sup ua:%x, client enabled:: na:%x, ua:%x \n",
                      ans_lib_cb.supported_unread_alerts, ans_lib_cb.supported_unread_alerts,
                      ans_lib_cb.client_configured_new_alerts, ans_lib_cb.client_configured_unread_alerts);
    }

    return status;
}

/* Initialize ANS library control block */
wiced_result_t wiced_bt_ans_init(wiced_bt_ans_gatt_handles_t *p_gatt_handles)
{
    if ((p_gatt_handles == NULL) ||
        (p_gatt_handles->new_alert.supported_category == 0) ||
        (p_gatt_handles->new_alert.configuration == 0) ||
        (p_gatt_handles->new_alert.value == 0) ||
        (p_gatt_handles->unread_alert.supported_category == 0) ||
        (p_gatt_handles->unread_alert.configuration == 0) ||
        (p_gatt_handles->unread_alert.value == 0))
    {
        ANS_TRACE_ERR("Wrong GATT Handle\n");
        return WICED_BT_BADARG;
    }

    /* Clear the Alert Control Block */
    memset(&ans_lib_cb, 0, sizeof(ans_lib_cb));

    /* Save the Alert GATT Handles */
    memcpy(&ans_lib_cb.gatt_handles, p_gatt_handles, sizeof(ans_lib_cb.gatt_handles));

    return WICED_BT_SUCCESS;
}

/* Application calls this API, when ANS server establish connection with ANC */
void wiced_bt_ans_connection_up(uint16_t conn_id)
{
    ans_lib_cb.conn_id = conn_id;
    ans_lib_cb.state = ANS_STATE_CONNECTED;
}

/* Application calls this API, when ANS server disconnected from ANC */
void wiced_bt_ans_connection_down(uint16_t conn_id)
{
    ans_lib_cb.conn_id = conn_id;
    ans_lib_cb.state = ANS_STATE_DISCONNECTED;
}

/* Application calls this API, when user configure the supportable new alerts*/
void wiced_bt_ans_set_supported_new_alert_categories(uint16_t conn_id, wiced_bt_anp_alert_category_enable_t supported_new_alert_cat)
{
    /* cannot be changed during connection */
    if (ans_lib_cb.conn_id == 0)
    {
        ans_lib_cb.supported_new_alerts = supported_new_alert_cat;
    }
}

/* Application calls this API, when user configure the supportable unread alerts*/
void wiced_bt_ans_set_supported_unread_alert_categories(uint16_t conn_id, wiced_bt_anp_alert_category_enable_t supported_unread_alert_cat)
{
    /* cannot be changed during connection */
    if (ans_lib_cb.conn_id == 0)
    {
        ans_lib_cb.supported_unread_alerts = supported_unread_alert_cat;
    }
}

/* Application calls this API, when ANC read the ANS services using GATT read operation */
wiced_bt_gatt_status_t wiced_bt_ans_process_gatt_read_req(uint16_t conn_id, wiced_bt_gatt_read_t *p_read_hdr,
                                                          uint8_t *p_read, uint16_t *p_read_len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (p_read_hdr->handle == ans_lib_cb.gatt_handles.new_alert.supported_category)
    {
        *p_read_len = 2;
        memcpy(p_read, &ans_lib_cb.supported_new_alerts, 2);
    }
    else if (p_read_hdr->handle == ans_lib_cb.gatt_handles.new_alert.configuration)
    {
        *p_read_len = 2;
        memcpy(p_read, &ans_lib_cb.new_alert_cccd, 2);
    }
    else if (p_read_hdr->handle == ans_lib_cb.gatt_handles.unread_alert.supported_category)
    {
        *p_read_len = 2;
        memcpy(p_read, &ans_lib_cb.supported_unread_alerts, 2);
    }
    else if (p_read_hdr->handle == ans_lib_cb.gatt_handles.unread_alert.configuration)
    {
        *p_read_len = 2;
        memcpy(p_read, &ans_lib_cb.unread_alert_status_cccd, 2);
    }
    else
    {
        status = WICED_BT_GATT_READ_NOT_PERMIT;
    }

    if (status == WICED_BT_GATT_SUCCESS)
        ANS_TRACE_DBG("handle:%04x len:%d data:%x status:%x\n", p_read_hdr->handle,
                      *p_read_len, (p_read[0] + (p_read[1] << 8)), status);

    return status;
}

/* Application calls this API, when ANC configure the alert services using GATT write operation */
wiced_bt_gatt_status_t wiced_bt_ans_process_gatt_write_req(uint16_t conn_id, wiced_bt_gatt_write_req_t *p_write)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_INVALID_ATTR_LEN;

    if (p_write->handle == ans_lib_cb.gatt_handles.new_alert.configuration)
    {
        if (p_write->val_len == 2 && p_write->p_val)
        {
            ans_lib_cb.new_alert_cccd = p_write->p_val[0] + (p_write->p_val[1] << 8);
            status = WICED_BT_GATT_SUCCESS;
        }
    }
    else if (p_write->handle == ans_lib_cb.gatt_handles.unread_alert.configuration)
    {
        if (p_write->val_len == 2 && p_write->p_val)
        {
            ans_lib_cb.unread_alert_status_cccd = p_write->p_val[0] + (p_write->p_val[1] << 8);
            status = WICED_BT_GATT_SUCCESS;
        }
    }
    else if (p_write->handle == ans_lib_cb.gatt_handles.notification_control)
    {
        if (p_write->val_len == 2 && p_write->p_val)
        {
            status = ans_lib_handle_client_alert_notification_control_point_write(conn_id, p_write->p_val[0], p_write->p_val[1]);
        }
    }
    else
    {
        status = WICED_BT_GATT_WRITE_NOT_PERMIT;
    }

    if (status == WICED_BT_GATT_SUCCESS)
    {
        ANS_TRACE_DBG("handle:%04x len:%d data[0,1]:%x %x status:%x\n",
                      p_write->handle, p_write->val_len, p_write->p_val[0], p_write->p_val[1], status);
    }

    return status;
}

/* Application calls this API, when new alert need to send to ANC */
wiced_bt_gatt_status_t wiced_bt_ans_process_and_send_new_alert(uint16_t conn_id, wiced_bt_anp_alert_category_id_t category_id)
{
    ANS_TRACE_DBG("conn_id:%d category_id:%d\n", conn_id, category_id);

    if ((category_id == 0xff) || (category_id >= ANP_NOTIFY_CATEGORY_COUNT))
    {
        ANS_TRACE_ERR("wrong category_id:%x\n", category_id);
        return WICED_BT_GATT_INVALID_CFG;
    }

    ANS_TRACE_DBG("Server supports:%x client configured:%x CCCD:%d\n",
                  (ans_lib_cb.supported_new_alerts & (1 << category_id)),
                  (ans_lib_cb.client_configured_new_alerts & (1 << category_id)),
                  ans_lib_cb.new_alert_cccd);

    ans_lib_cb.notify_data[category_id].num_of_new_alerts++;

    if ((ans_lib_cb.conn_id) &&
        (ans_lib_cb.supported_new_alerts & (1 << category_id)) &&
        (ans_lib_cb.client_configured_new_alerts & (1 << category_id)) &&
        (ans_lib_cb.new_alert_cccd))
    {
        return ans_lib_send_new_alert(conn_id, category_id);
    }

    /* Remember the alert category to decide to notify or not on ANP_ALERT_CONTROL_CMD_NOTIFY_NEW_ALERTS_IMMEDIATE */
    ans_lib_cb.new_alert_not_sent |= (1 << category_id);

    return WICED_BT_GATT_SUCCESS;
}

/* Application calls this API, when Unread alert need to send to ANC */
wiced_bt_gatt_status_t wiced_bt_ans_process_and_send_unread_alert(uint16_t conn_id, wiced_bt_anp_alert_category_id_t category_id)
{
    if ((category_id == 0xff) || (category_id >= ANP_NOTIFY_CATEGORY_COUNT))
    {
        ANS_TRACE_ERR("category_id:%x \n", category_id);
        return WICED_BT_GATT_INVALID_CFG;
    }

    ans_lib_cb.notify_data[category_id].num_of_unread_count++;

    if ((ans_lib_cb.conn_id) &&
        (ans_lib_cb.supported_unread_alerts & (1 << category_id)) &&
        (ans_lib_cb.client_configured_unread_alerts & (1 << category_id)) &&
        (ans_lib_cb.unread_alert_status_cccd))
    {
        return ans_lib_send_unread_alert(conn_id, category_id);
    }

    /* Remember the alert category to decide to notify or not on ANP_ALERT_CONTROL_CMD_NOTIFY_NEW_ALERTS_IMMEDIATE */
    ans_lib_cb.unread_alert_status_not_sent |= (1 << category_id);

    ANS_TRACE_DBG("conn_id:%d Server supports:%x, client configured:%x, CCCD:%d \n", ans_lib_cb.conn_id,
                  (ans_lib_cb.supported_unread_alerts & (1 << category_id)),
                  (ans_lib_cb.client_configured_unread_alerts & (1 << category_id)), ans_lib_cb.unread_alert_status_cccd);

    return WICED_BT_GATT_SUCCESS;
}

/* Application calls this API, to clear the alerts which are yet to send to alert client */
wiced_bool_t wiced_bt_ans_clear_alerts(uint16_t conn_id, wiced_bt_anp_alert_category_id_t category_id)
{
    if (category_id >= ANP_NOTIFY_CATEGORY_COUNT)
    {
        ANS_TRACE_ERR("category_id:%x \n", category_id);
        return WICED_FALSE;
    }

    ans_lib_cb.notify_data[category_id].num_of_new_alerts = 0;
    ans_lib_cb.notify_data[category_id].num_of_unread_count = 0;
    ans_lib_cb.new_alert_not_sent &= (~(1 << category_id));
    ans_lib_cb.unread_alert_status_not_sent &= (~(1 << category_id));

    return WICED_TRUE;
}
