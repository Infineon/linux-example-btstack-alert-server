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
 * File Name: main.c
 *
 * Description: This is the source code for Linux CE ANS project.
 *
 * Related Document: See README.md
 *
 *******************************************************************************/
/*******************************************************************************
 *                               INCLUDES
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "wiced_memory.h"
#include "wiced_bt_trace.h"
#include "wiced_bt_cfg.h"
#include "wiced_bt_stack.h"
#include "platform_linux.h"
#include "app_bt_utils/app_bt_utils.h"
#include "bt_app_ans.h"
#include "utils_arg_parser.h"

/*******************************************************************************
 *                               MACROS
 *******************************************************************************/
#define MAX_PATH ( 256U )
#define IP_ADDR_LEN ( 16U )
#define IP_ADDR "000.000.000.000"
#define INVALID_IP_CMD ( 15 )
#define EXP_IP_RET_VAL ( 1 )
#define INVALID_SCAN   ( 0 )

/*******************************************************************************
 *                    STRUCTURES AND ENUMERATIONS
 *******************************************************************************/
static const char bt_app_ans_app_menu[] = "\n\
================================== \n\
  Alert Notification Server Menu \n\
----------------------------------\n\
    0.  Exit \n\
    1.  Set Supported New Alert Categories \n\
    2.  Set Supported Unread Alert Categories \n\
    3.  Generate Alert \n\
    4.  Clear Alert \n\
    5.  Scan and Connect \n\
    6.  Disconnect \n\
 =================================\n\
 Choose option (0-6): ";

static const char alert_ids[] = "\
    ----------------------------- \n\
    SIMPLE_ALERT          0 \n\
    EMAIL                 1 \n\
    NEWS                  2 \n\
    CALL                  3 \n\
    MISSED_CALL           4 \n\
    SMS_OR_MMS            5 \n\
    VOICE_MAIL            6 \n\
    SCHEDULE_ALERT        7 \n\
    HIGH_PRIORITY_ALERT   8 \n\
    INSTANT_MESSAGE       9 \n\
    ----------------------------- \n";

static const char alert_categ[] = "\
-------------------------------- \n\
INSTANT_MESSAGE|HIGH_PRIORITY_ALERT|SCHEDULE_ALERT|VOICE_MAIL|SMS_OR_MMS|\
MISSED_CALL|CALL|NEWS|EMAIL|SIMPLE_ALERT\n\
     10               9                  8              7         6      \
      5       4    3    2        1 \n";

/*******************************************************************************
 *                           GLOBAL VARIABLES
 *******************************************************************************/
wiced_bt_heap_t *p_default_heap = NULL;
uint8_t ans_bd_address[LOCAL_BDA_LEN] = {0x11, 0x12, 0x13, 0x41, 0x42, 0x43};

/*******************************************************************************
 *                       FUNCTION DECLARATIONS
 *******************************************************************************/
uint32_t hci_control_proc_rx_cmd(uint8_t *p_buffer, uint32_t length);
void APPLICATION_START(void);

/*******************************************************************************
 *                       FUNCTION DEFINITIONS
 *******************************************************************************/

/*******************************************************************************
 * Function Name: hci_control_proc_rx_cmd()
 ********************************************************************************
 * Summary:
 *   Function to handle HCI receive called from platform code
 *
 * Parameters:
 *   uint8_t* p_buffer   : rx buffer
 *   uint32_t length     : rx buffer length
 *
 * Return:
 *  status code
 *
 *******************************************************************************/
uint32_t hci_control_proc_rx_cmd(uint8_t *p_buffer, uint32_t length)
{
    return 0;
}

/*******************************************************************************
 * Function Name: APPLICATION_START()
 ********************************************************************************
 * Summary:
 *   BT stack initialization function wrapper called from platform initialization
 *
 * Parameters:
 *   None
 *
 * Return:
 *   None
 *
 *******************************************************************************/
void APPLICATION_START(void)
{
    application_start();
}

/*******************************************************************************
 * Function Name: main()
 ********************************************************************************
 * Summary:
 *   Application entry function
 *
 * Parameters:
 *   int argc            : argument count
 *   char *argv[]        : list of arguments
 *
 * Return:
 *   None
 *
 *******************************************************************************/
int main(int argc, char *argv[])
{
    wiced_result_t status = WICED_BT_SUCCESS;
    int ip = 0;
    unsigned int alert_id = 0;
    unsigned int alert_category = 0;
    int len = 0;
    int filename_len = 0;
    char fw_patch_file[MAX_PATH];
    char hci_port[MAX_PATH];
    char peer_ip_addr[IP_ADDR_LEN] = "000.000.000.000";
    uint32_t hci_baudrate = 0;
    uint32_t patch_baudrate = 0;
    int btspy_inst = 0;
    uint8_t btspy_is_tcp_socket = 0;		 /* Throughput calculation thread handler */
    pthread_t throughput_calc_thread_handle; /* Audobaud configuration GPIO bank and pin */
    cybt_controller_autobaud_config_t autobaud;
    int ret = 0;
    memset(fw_patch_file, 0, MAX_PATH);
    memset(hci_port, 0, MAX_PATH);
    if (PARSE_ERROR ==
        arg_parser_get_args(argc, argv, hci_port, ans_bd_address, &hci_baudrate,
                            &btspy_inst, peer_ip_addr, &btspy_is_tcp_socket,
                            fw_patch_file, &patch_baudrate, &autobaud))
    {
        return EXIT_FAILURE;
    }
    filename_len = strlen(argv[0]);
    if (filename_len >= MAX_PATH)
    {
        filename_len = MAX_PATH - 1;
    }

    cy_platform_bluetooth_init(fw_patch_file, hci_port, hci_baudrate,
                               patch_baudrate, &autobaud);

    do
    {
        fprintf(stdout, "%s", bt_app_ans_app_menu);
        fflush(stdin);
		ret = fscanf(stdin, "%d", &ip);
        if( EXP_IP_RET_VAL != ret )
        {
            ip = INVALID_IP_CMD;
            while(getchar() != '\n');
        }

        switch (ip)
        {
        case 0: /* Exiting application */
            status = WICED_BT_SUCCESS;
            break;

        case 1: /* Set Supported New Alert Categories */
            fprintf(stdout, "\n Alert Category and bit position \n");
            fprintf(stdout, "%s", alert_categ);
            fprintf(stdout, "Set Supported New Alert Categories.");
            fprintf(stdout,
                    "2 Byte Value with bit representation for each category \n");
            fprintf(stdout,
                    "Example 26 (0001 1010) will SET the Alert for Categories: Missed call, Call, Email: ");
            if ( INVALID_SCAN == fscanf(stdin, "%u", &alert_category) )
            {
                fprintf(stdout, "Unknown input for new alert categories\n");
                continue;
            }
            len = LEN_2_BYTE; /* Number of bytes */
            status =
                bt_app_ans_handle_set_supported_new_alert_categories((uint16_t)
                                                                         alert_category,
                                                                     len);
            if (status == WICED_BT_GATT_SUCCESS)
            {
                fprintf(stdout, "New Alerts Setting successful \n");
            }
            break;

        case 2: /* Set Supported Unread Alert Categories */
            fprintf(stdout, "\n Alert Category and bit position \n");
            fprintf(stdout, "%s", alert_categ);
            fprintf(stdout, "Set Supported Unread Alert Categories.");
            fprintf(stdout,
                    "2 Byte Value with bit representation for each category \n");
            fprintf(stdout,
                    "Example 26 (0001 1010) will SET the Alert for Categories: Missed call, Call, Email: ");
            if ( INVALID_SCAN == fscanf(stdin, "%u", &alert_category) )
            {
                fprintf(stdout, "Unknown input for unread alert categories\n");
                continue;
            }
            len = LEN_2_BYTE; /* Number of bytes */
            status =
                bt_app_ans_handle_set_supported_unread_alert_categories((uint16_t)alert_category, len);
            if (status == WICED_BT_GATT_SUCCESS)
            {
                fprintf(stdout, "Unread Alerts Setting successful \n");
            }
            break;

        case 3: /* Generate Alert */
            fprintf(stdout, "\n    Alert Categories \n");
            fprintf(stdout, "%s", alert_ids);
            fprintf(stdout,
                    "Generate Alert for a particular Category ID (0-9): ");
            if ( INVALID_SCAN == fscanf(stdin, "%u", &alert_id) )
            {
                fprintf(stdout, "Unknown input for generate alert categories\n");
                continue;
            }
            len = LEN_1_BYTE; /* Number of bytes */
            status = bt_app_ans_handle_generate_alert((uint8_t)alert_id, len);
            if (status == WICED_BT_GATT_SUCCESS)
            {
                fprintf(stdout, "Generate Alert initiated \n");
            }
            break;

        case 4: /* Clear Alert */
            fprintf(stdout, "\n    Alert Categories \n");
            fprintf(stdout, "%s", alert_ids);
            fprintf(stdout,
                    "Clear Alert for a particular Category ID (0-9): ");
            if ( INVALID_SCAN == fscanf(stdin, "%u", &alert_id) )
            {
                fprintf(stdout, "Unknown input for clear alert categories\n");
                continue;
            }
            len = LEN_1_BYTE; /* Number of bytes */
            status = bt_app_ans_handle_clear_alert((uint8_t)alert_id, len);
            if (status == WICED_BT_GATT_SUCCESS)
            {
                fprintf(stdout, "Alert Category %d cleared \n", alert_id);
            }
            break;

        case 5: /* Scan and Connect */
            status = bt_app_ans_start_scan_connect();
            if (status == WICED_BT_SUCCESS)
            {
                fprintf(stdout, "Scan and Connect Initiated. \n");
            }
            break;

        case 6: /* Disconnect */
            status = bt_app_ans_disconnect();
            if (status == WICED_BT_GATT_SUCCESS)
            {
                fprintf(stdout, "Disconnect initiated. \n");
            }
            break;

        default:
            fprintf(stdout,
                    "Unknown ANS Command. Choose option from the Menu \n");
            break;
        }
        if (status != WICED_BT_SUCCESS)
        {
            fprintf(stderr, "\n Command Failed. Status: 0x%x \n", status);
            status = WICED_BT_SUCCESS;
        }
        fflush(stdin);
    } while (ip != 0);

    fprintf(stdout, "Exiting...\n");
    wiced_bt_delete_heap(p_default_heap);
    wiced_bt_stack_deinit();

    return EXIT_SUCCESS;
}
