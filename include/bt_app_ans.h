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
 ******************************************************************************/
/******************************************************************************
 * File Name: bt_app_ans.h
 *
 * Description: Header file for bt_app_ans.c
 *
 * Related Document: See README.md
 *
 *******************************************************************************/

#ifndef _BT_APP_ANS_H_
#define _BT_APP_ANS_H_

/*******************************************************************************
 *                                   INCLUDES
 *******************************************************************************/
#include <stdint.h>
#include "wiced_bt_trace.h"
#include "wiced_bt_cfg.h"
#include "platform_linux.h"

/*******************************************************************************
 *                                   MACROS
 *******************************************************************************/
#define LEN_1_BYTE (1U)
#define LEN_2_BYTE (2U)
#define LOCAL_BDA_LEN (6U)

/******************************************************************************
 *                                EXTERNS
 *****************************************************************************/
extern uint8_t ans_bd_address[];

/******************************************************************************
 *                           FUNCTION PROTOTYPES
 ******************************************************************************/
void application_start(void);
uint16_t bt_app_ans_handle_set_supported_new_alert_categories(uint16_t p_data, uint8_t length);
uint16_t bt_app_ans_handle_set_supported_unread_alert_categories(uint16_t p_data, uint8_t length);
uint16_t bt_app_ans_handle_generate_alert(uint8_t p_data, uint8_t len);
uint16_t bt_app_ans_handle_clear_alert(uint8_t p_data, uint8_t len);
uint16_t bt_app_ans_start_scan_connect(void);
uint16_t bt_app_ans_disconnect(void);

#endif /* _BT_APP_ANS_H_ */
