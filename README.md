# AIROC™ BTSTACK: Bluetooth® Alert Server(ANS) for Linux host

This code example demonstrates Bluetooth® Alert Server using AIROC™ Wi-Fi & Bluetooth® combo chip.

## Requirements

- Programming language: C
- Embedded Linux platforms (Arm® Cortex®-A Class processors) for host communications. Supported Embedded Linux host platforms:
   1. RPI CM4-lite IFX custom HW board from Infineon
   2. [IMX8 nano uCOM board](https://www.embeddedartists.com/products/imx8m-nano-developers-kit-v3/) from Embedded Artists
- AIROC™ BTSTACK library and Linux porting layer source code
- AIROC™ Wi-Fi & Bluetooth® combo chip Bluetooth® Firmware file (.hcd).
- Linux Host PC with Ubuntu 20.04

## Supported toolchains (make variable 'TOOLCHAIN')

- GNU Arm® GCC AArch64 v9.3.0 (`GCC_ARM`)

## Supported AIROC™ Wi-Fi & Bluetooth® combo chip

- [AIROC™ CYW5557x Wi-Fi & Bluetooth® combo chip](https://www.infineon.com/cms/en/product/wireless-connectivity/airoc-wi-fi-plus-bluetooth-combos/wi-fi-6-6e-802.11ax/)

## Hardware setup

Set up the hardware required for this code example as follows:

**Figure 1. Hardware setup block diagram**

![](images/blockdiagram.png)

## Software setup

Set up a cross compiler according to the target platform requirement along with CMake on the Linux host PC based on Ubuntu 20.04.

**Note:** These steps are for Arm&reg; 64-based target platform. Review these steps before adding support for any other target.

1. Open a terminal on the Linux host PC.

2. Use the following command to install the cross compiler, build tools, and dependencies:
   ```
   sudo apt-get install git cmake gcc-aarch64-linux-gnu build-essential -y
   ```

## Using the code example

This section describes the steps to be executed on the Linux host PC for compiling the code example.

1. Create a directory under `$HOME` on the Linux host PC and switch to the created directory. Use the following commands for reference:

   ```bash
   mkdir $HOME/Linux_CE
   cd $HOME/Linux_CE
   ```

2. Fetch the code example source code using the following command:

   ```bash
    git clone https://github.com/Infineon/linux-example-btstack-alert-server
   ```

3. Clone the code example dependencies (BTSTACK library and Linux porting layer source code) using the following commands:

   ```bash
   git clone https://github.com/Infineon/btstack --branch release-v3.6.0
   git clone https://github.com/Infineon/bluetooth-linux.git --branch release-v1.0.0
   ```

      Creates three different directories after cloning the code example and its dependencies with the directory structure as follows:

   **Figure 3. Code example directory structure**

   ![](images/directory_structure.png)

4. Clone the Bluetooth® firmware using following command.
   ```bash
   git clone https://github.com/Infineon/combo-bluetooth-firmware.git
   ```
   User can choose appropriate Bluetooth® firmware for particular AIROC™ Wi-Fi & Bluetooth® combo chip from cloned "combo-bluetooth-firmware" directory.

5. Create the build folder under the code example source folder and build the code example using the following commands:

   ```
   cd $HOME/Linux_CE/linux-example-btstack-alert-server
   mkdir build && cd build
   cmake -DCMAKE_C_COMPILER:PATH=<GCC_CROSS_COMPILER> ../ && make
   ```
   Where,
   - `GCC_CROSS_COMPILER` is the target cross compiler for GCC (generally */usr/bin/aarch64-linux-gnu-gcc* for Arm&reg; 64-based targets)

   The code example executable is generated under the *build* folder with the same name of code example.

   For example, in this project, the "linux-example-btstack-alert-server" executable is generated at */home/$USER/Linux_CE/linux-example-btstack-alert-server/build*.

## Operation

### Using two hardware platforms (Linux host platform and AIROC™ Wi-Fi & Bluetooth® combo chip)

**Devices used:**

- **Device Under Test (DUT):** Linux host platform with AIROC™ Wi-Fi & Bluetooth® combo chip (target platform).
Implements: "Alert Notification Server", GATT Server.

- **Testing Device:** Linux host platform with AIROC™ Wi-Fi & Bluetooth® combo chip (target platform).
Used as: "Alert Notification CLient", GATT Client. User can get ANC application from [here](https://github.com/Infineon/linux-example-btstack-alert-client
)

1. Copy the code example executable and AIROC™ BTSTACK library from the Linux host PC to the target platform using [SCP](https://help.ubuntu.com/community/SSH/TransferFiles). For example, use the following commands:

   ```
   cd $HOME/Linux_CE/linux-example-btstack-alert-server/build
   scp linux-example-btstack-alert-server <TARGET_USER>@<TARGET_IP>:<TARGET_PATH>/.
   cd $HOME/Linux_CE/btstack/stack/COMPONENT_WICED_DUALMODE/COMPONENT_ARMv8_LINUX/COMPONENT_GCC
   scp libbtstack.so <TARGET_USER>@<TARGET_IP>:<TARGET_PATH>/.
   scp <FW_FILE.hcd> <TARGET1_USER>@<TARGET1_IP>:<TARGET1_PATH>/.
   ```
   Where,
   - `TARGET_USER` is the user name of the target platform
   - `TARGET_IP` is the IP address of the target platform
   - `TARGET_PATH` is the path of target platform
   - `FW_FILE.hcd` file is Bluetooth® Firmware file cloned in step-4 of [Using the code example section](#using-the-code-example).

2. Take SSH console of target platform.
   ```bash
   ssh <TARGET_DEVICE_USER_NAME>@<TARGET_DEVICE_IP_ADDRESS>
   ```

3. Add the udev rule in the target host board for HCI UART and GPIO to bypass root access. Use the following steps to create and set up a udev rule.

   **Note:** If you have root access, the following udev rules are not required; you can execute the code example with `sudo` permissions or by switching to the root user.

   1. Create a new *.rules* (for example, *combo-chip-uart-port.rules*) file under */etc/udev/rules.d/* directory for HCI UART. Use the following commands:

      **IMX8Nano:**

      ```bash
      echo "KERNEL==\"ttymxc0\"d,SYMLINK+=\"combo_chip_uart\",MODE=\"0666\"" | sudo tee /etc/udev/rules.d/combo-chip-uart-port.rules
      ```

      **RPICM4:**

      ```bash
      echo "KERNEL==\"ttyAMA0\",SYMLINK+=\"combo_chip_uart\",MODE=\"0666\"" | sudo tee /etc/udev/rules.d/combo-chip-uart-port.rules
      ```

   2. Create new *.rules* (for example, *combo-chip-gpio-port.rules*) for BT_REG_ON GPIO under */etc/udev/rules.d/*. Use the following commands:

      **IMX8Nano & RPICM4:**

      1. Create a rule file using the following command.

         ```bash
         sudo vim /etc/udev/rules.d/combo-chip-gpio-port.rules.rules
         ```

      2. Add the following rules in created files:

         ```bash
         SUBSYSTEM=="gpio*", PROGRAM="/bin/sh -c 'chown -R $user:$group /sys/class/gpio/export /sys/class/gpio/unexport;'"
         SUBSYSTEM=="gpio*", PROGRAM="/bin/sh -c 'chown -R $user:$group /sys%p/direction /sys%p/value; chmod 660 /sys%p/direction /sys%p/value;'"
         ```

   3. Reboot the target device:

      ```bash
      sudo reboot
      ```

       Where,

      - `ttymxc0` and `ttyAMA0` are HCI UART ports for IMX8Nano and RPICM4 respectively
      - `combo_chip_uart` is a friendly name for the HCI UART port
      - `0666` is the permission mask to bypass the root access for HCI UART


4. Execute the application by setting the paths of the AIROC™ BTSTACK library using the following command on the target platform:
   ```
   cd <TARGET_PATH>
   chmod +x <APP_NAME>
   LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<BTSTACK_LIB_PATH>
   ./<APP_NAME> -c <COM_PORT> -b 3000000 -f 921600 -r <GPIOCHIPx> <REGONPIN> -n -p <FW_FILE_NAME>.hcd -d 112233221133
   ```
   Where,
   - `TARGET_PATH` is the path of the target platform where the code example application copied
   - `BTSTACK_LIB_PATH` is path of the AIROC™ BTSTACK library. Skip this if the AIROC™ BTSTACK library and code example application executable are in the same folder
   - `/dev/ttymxc0` is the COM port for IMX8-NANO
   - `/dev/ttyAMA0` is the COM port for RPI CM4
   - `3000000` is HCI baud rate
   - `112233221133` is a device BD address
   - `-r <GPIOCHIPx> <REGONPIN> -n`  is setting the GPIO control to enable autobaud for Hatchet2
		- `-r gpiochip5 0 -n`  For IMX8-NANo	
		- `-r gpiochip0 3 -n`  For RPI CM4
   - `921600` is FW download baudrate
   - `.hcd` is FW file to download (make sure to validate this firmware file file path)

     **Note 1:** Currently, random BD addresses are used for testing.
     **Note 2:** The CE supports maximum of 1 paired device. If the user is connecting to a new device, nvramxxx.bin should be deleted on the DUT.

5. To test this code example "Alert Notifications Server" (ANS) with other testing device runs "Alert Notification Client" (ANC) code example, do the following:

   1. Start the "Alert Client" application on the testing device.

   2. Start the "Alert Server" application on the DUT as mentioned in Step 4 of the [Operation](#operation) section.

   3. Set the required new alert and unread alert options for required services from the ANS menu according to the instructions available on the menu.

      **Figure 4. Configuring the new alert and unread alert using the ANS application menu**
     
      ![](images/ans_alert_config.png)

   4. On the ANC testing device, start advertising. Once the advertising starts at ANC, the ANS device can scan and connect. 

   5. Choose 'Scan and Connect' option on DUT to connect to available ANC automatically. ANS performs a Scan and connects to the Alert Notification Client (ANC), if the ANC device is advertising within the range. If an ANC device is not found with in 90 seconds, the Scan is stopped automatically. The user has to choose the option again to restart the Scan and connect to the ANC device whenever it is ready for a connection. 

      **Note:**  If the remote device bonds with the ANS device, then the Link keys will be saved for further retrieval. If user is connecting a fresh device that is not previously paired, then delete the nvramxxx.bin from the current directory.

   6. On ANC testing device, user can choose to enable the Alert Notification for New Alerts and/ or Unread alerts by selecting the corresponding option in the ANC application menu.

   7. Additionally, the categories of the Alert can be configured through the Control Point commands on ANC testing device.
   
   8. On ANS, The user can use the 'Generate Alert' option to generate alerts and 'Clear alert' option to clear the alerts.

   9. The application allows the user to set all the available alert categories. Note that the setting of New alert and Unread alert can only be done when the ANS is not connected to ANC.
  
   10. After connection to the ANC device, the ANS device sends new alerts and unread alerts to the client based on ANC configuration. New alerts and unread alerts get generated as follows:
    - User generates the alert when an ANS device has a connection with an ANC device.
    - When the user generates an Alert using the Menu, the ANC device receives the new alert and unread alert.

    **Figure 5. Receiving a new alert from ANC application**

    ![](images/ans_email_alert.png)
    
   11. User can choose 'Disconnect' option to disconnect the ANC device.

## Debugging

You can debug the example using a generic Linux debugging mechanism such as the following:

1. **Debugging by logging:** Add required prints in the application and check them during execution.

2. **Debugging using GDB:** See the [GDB man page](https://linux.die.net/man/1/gdb) for more details.

## Design and implementation

**Roles implemented:**

- **Code example (implementation under test):** Alert Notification Server, GATT Server, and GAP Central role

- **Testing device running Alert Notification Client Code Example:** Alert Notification Client, GATT Client, and GAP Peripheral role

**Alert Notification Server (ANS) working:**

   The ‘Alert Notification Server’ (the Bluetooth&reg; LE Central device) is a Bluetooth&reg; LE GATT Server. 
  
  ![](images/flowchart.png)

   The ANS connects to the ANC and exposes the Alert notification Service with various categories. When ANC device does a service discovery using the "GATT Discover All Primary Services" procedure, these services are available to the ANC to configure and use.
   The Bluetooth&reg; LE Service Characteristic discovery is done by the "Discover All Characteristics of a Service" procedure. When the ANC wants to configure the any alert category on the ANS, it writes to the Alert Notification Control Point in the Alert Level characteristic of the ANS. When the user triggers an Alert (New/ Unread), the ANC receives the alert and required info as per the configuration.

This code example does the following:

1. Parses the command-line arguments and initializes the AIROC™ BTSTACK library for the CYW5557x device

2. On starting the ANS Code example, the application registers alert notification service with the GATT Server.

3. Provides options as follows:

   ----------------------------------
      Alert Notification Server Menu
   ----------------------------------
      0.  Exit
      1.  Set Supported New Alert Categories
      2.  Set Supported Unread Alert Categories
      3.  Generate Alert
      4.  Clear Alert
      5.  Scan and Connect
      6.  Disconnect
   ----------------------------------
      
      Choose option (0-6):
5. Application follows the sequence as shown in the flowchart above.

6. On choosing 0, the application exits.

**Note:** Run the application without any arguments to get the details of the command-line arguments.

## Source files

 Files   | Description of files
 ------- | ---------------------
 *app/main.c*  | Implements the main function which takes the user command-line inputs.
 *app_bt_utils/app_bt_utils.c*  |  Contains utility functions like functions to print error codes, status, etc in a user-understandable format.
 *app_bt_utils/app_bt_utils.h*  | Header file corresponding to *app_bt_utils.c*
 *app/bt_app_ans.c*  | Functions for all the Alert Notification Server functionalities.
 *include/bt_app_ans.h*  | Header file corresponding to *bt_app_ans.c*.
 *app_bt_config/ans_bt_settings.c*  | Contains Bluetooth&reg; stack configuration parameters.
 *app_bt_config/ans_gap.c*  | Contains Bluetooth&reg; GAP parameters.
 *app_bt_config/ans_gatt_db.c*  | Contains Bluetooth&reg; GATT database.

## Resources and settings

**Table 1. Application resources**

 Resource  |  Alias/object     |    Purpose
 :-------- | :-------------    | :------------
 UART |HCI| UART is used for HCI communication with host system

<br>

## Related resources

Resources  | Links
-----------|----------------------------------
Device documentation | [CYW5557x](https://www.infineon.com/cms/en/product/wireless-connectivity/airoc-wi-fi-plus-bluetooth-combos/wi-fi-6-6e-802.11ax/)
AIROC™ BTSTACK library | [AIROC™ BTSTACK library](https://github.com/Infineon/btstack/tree/release-v3.6.0)
Linux porting layer source code | [Linux porting layer source code](https://github.com/Infineon/bluetooth-linux/tree/release-v1.0.0)

## Other resources

Infineon provides a wealth of data at [www.infineon.com](https://www.infineon.com/) to help you select the right device, and quickly and effectively integrate it into your design.

## Document history

Document title: *CE237176* - *AIROC&trade; BTSTACK: Bluetooth&reg; Alert Server(ANS) for Linux host*

 Version | Description of change
 ------- | ---------------------
 1.0.0   | New code example

<br>

---------------------------------------------------------

© Cypress Semiconductor Corporation, 2023. This document is the property of Cypress Semiconductor Corporation, an Infineon Technologies company, and its affiliates ("Cypress").  This document, including any software or firmware included or referenced in this document ("Software"), is owned by Cypress under the intellectual property laws and treaties of the United States and other countries worldwide.  Cypress reserves all rights under such laws and treaties and does not, except as specifically stated in this paragraph, grant any license under its patents, copyrights, trademarks, or other intellectual property rights.  If the Software is not accompanied by a license agreement and you do not otherwise have a written agreement with Cypress governing the use of the Software, then Cypress hereby grants you a personal, non-exclusive, nontransferable license (without the right to sublicense) (1) under its copyright rights in the Software (a) for Software provided in source code form, to modify and reproduce the Software solely for use with Cypress hardware products, only internally within your organization, and (b) to distribute the Software in binary code form externally to end users (either directly or indirectly through resellers and distributors), solely for use on Cypress hardware product units, and (2) under those claims of Cypress’s patents that are infringed by the Software (as provided by Cypress, unmodified) to make, use, distribute, and import the Software solely for use with Cypress hardware products.  Any other use, reproduction, modification, translation, or compilation of the Software is prohibited.
<br>
TO THE EXTENT PERMITTED BY APPLICABLE LAW, CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD TO THIS DOCUMENT OR ANY SOFTWARE OR ACCOMPANYING HARDWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  No computing device can be absolutely secure.  Therefore, despite security measures implemented in Cypress hardware or software products, Cypress shall have no liability arising out of any security breach, such as unauthorized access to or use of a Cypress product. CYPRESS DOES NOT REPRESENT, WARRANT, OR GUARANTEE THAT CYPRESS PRODUCTS, OR SYSTEMS CREATED USING CYPRESS PRODUCTS, WILL BE FREE FROM CORRUPTION, ATTACK, VIRUSES, INTERFERENCE, HACKING, DATA LOSS OR THEFT, OR OTHER SECURITY INTRUSION (collectively, "Security Breach").  Cypress disclaims any liability relating to any Security Breach, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any Security Breach.  In addition, the products described in these materials may contain design defects or errors known as errata which may cause the product to deviate from published specifications. To the extent permitted by applicable law, Cypress reserves the right to make changes to this document without further notice. Cypress does not assume any liability arising out of the application or use of any product or circuit described in this document. Any information provided in this document, including any sample design information or programming code, is provided only for reference purposes.  It is the responsibility of the user of this document to properly design, program, and test the functionality and safety of any application made of this information and any resulting product.  "High-Risk Device" means any device or system whose failure could cause personal injury, death, or property damage.  Examples of High-Risk Devices are weapons, nuclear installations, surgical implants, and other medical devices.  "Critical Component" means any component of a High-Risk Device whose failure to perform can be reasonably expected to cause, directly or indirectly, the failure of the High-Risk Device, or to affect its safety or effectiveness.  Cypress is not liable, in whole or in part, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any use of a Cypress product as a Critical Component in a High-Risk Device. You shall indemnify and hold Cypress, including its affiliates, and its directors, officers, employees, agents, distributors, and assigns harmless from and against all claims, costs, damages, and expenses, arising out of any claim, including claims for product liability, personal injury or death, or property damage arising from any use of a Cypress product as a Critical Component in a High-Risk Device. Cypress products are not intended or authorized for use as a Critical Component in any High-Risk Device except to the limited extent that (i) Cypress’s published data sheet for the product explicitly states Cypress has qualified the product for use in a specific High-Risk Device, or (ii) Cypress has given you advance written authorization to use the product as a Critical Component in the specific High-Risk Device and you have signed a separate indemnification agreement.
<br>
Cypress, the Cypress logo, and combinations thereof, WICED, ModusToolbox, PSoC, CapSense, EZ-USB, F-RAM, and Traveo are trademarks or registered trademarks of Cypress or a subsidiary of Cypress in the United States or in other countries. For a more complete list of Cypress trademarks, visit www.infineon.com. Other names and brands may be claimed as property of their respective owners.
