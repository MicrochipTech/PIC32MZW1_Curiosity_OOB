/*******************************************************************************
  WiFi MAC interface functions

  File Name:
    wdrv_pic32mzw_cfg.h

  Summary:
   Microchip WLAN functions.

  Description:
    This File provide the read/write functionality for configuring WLAN MAC.
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc. All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*******************************************************************************/
// DOM-IGNORE-END

#ifndef _WDRV_PIC32MZW_CFG_H
#define _WDRV_PIC32MZW_CFG_H

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
    extern "C" {
#endif
// DOM-IGNORE-END

// *****************************************************************************
/*
                    Character Type WIDs
*/
// *****************************************************************************
/*
    Summary:
        WID for Wi-Fi BSS Type.
    Description:
        DRV_WIFI_WID_BSS_TYPE contains the BSS type. The possible values are shown below,

        BSS Type    Value
        BSS STA     0
        IBSS STA    1
        AP          2

*/
#define DRV_WIFI_WID_BSS_TYPE                       0x0000

// *****************************************************************************
/*
    Summary:
        WID for  TX legacy data rates.
    Description:
        DRV_WIFI_WID_CURRENT_TX_RATE contains the legacy transmission rate. The possible values are shown below,

        Data Rate   Value
        Autorate    0
        1 Mbps      1
        2 Mbps      2
        5.5 Mbps    5
        11 Mbps     11
        6 Mbps      6
        9 Mbps      9
        12 Mbps     12
        18 Mbps     18
        24 Mbps     24
        36 Mbps     36
        48 Mbps     48
        54 Mbps     54

        In case when 11g Operating Mode (DRV_WIFI_WID_11G_OPERATING_MODE) is set to 11g High performance mode,
        the allowed values are only 6 to 54 Mbps (OFDM rates).
        In Autorate mode, the transmit rate is chosen automatically by the AP/STA depending upon the channel conditions.
*/
#define DRV_WIFI_WID_CURRENT_TX_RATE                 0x0001

#define DRV_WIFI_BOOTMODE                           0x0058
// *****************************************************************************
/*
    Summary:
        WID for  primary channel.
    Description:
        DRV_WIFI_WID_PRIMARY_CHANNEL contains the primary channel number.
        The possible values range from 1 to 14 in 2.4 GHz and 36 to 144 in 5GHz.
*/
#define DRV_WIFI_WID_PRIMARY_CHANNEL                    0x0002

// *****************************************************************************
/*
    Summary:
        WID for short preamble capabilities
    Description:
        DRV_WIFI_WID_PREAMBLE is used for short preamble capabilities.
        The supported values are shown below,
                Description                     Value
                Short preamble supported        0
                Short preamble unsupported      1
                Short preamble auto selection   2
*/
#define DRV_WIFI_WID_PREAMBLE                             0x0003

// *****************************************************************************
/*
    Summary:
        WID for  operating mode values.
    Description:
        DRV_WIFI_WID_11G_OPERATING_MODE valid only when 11g or 11n is enabled.
        It indicates the 11g operating mode (and thus the basic and extended rate set) to be supported by the AP/STA.
        The possible values are shown in below,

            Operating Modes             Value       Basic Rate                  Set Extended Rate Set
            11b Only mode               0           1,2,5.5, 11 Mbps            None
            11g High performance mode   1           6,12, 24 Mbps               9,18,36,48, 54 Mbps
            11g compatibility mode 1    2           1,2,5.5, 11 Mbps            6, 9, 12, 18, 24, 36, 48, 54 Mbps
            11g compatibility mode 2    3           1,2,5.5,11,6,12, 24 Mbps    9,18,36,48, 54 Mbps
*/
#define DRV_WIFI_WID_11G_OPERATING_MODE              0x0004

// *****************************************************************************
/*
    Summary:
        WID for requested wid status.
    Description:
        DRV_WIFI_WID_STATUS contains the WID write response.
        MAC SW uses this to specify whether the WID request was successfully processed or not.
        The possible values are shown below,

                Status      Value
                FAILURE     0
                SUCCESS     1
*/
#define DRV_WIFI_WID_STATUS                             0x0005

// *****************************************************************************
/*
    Summary:
        WID for  scan type.
    Description:
        DRV_WIFI_WID_SCAN_TYPE field specifies the type of channel scanning to be used in STA mode.
        The possible values are shown in below,
                Scan Type           Value
                Passive Scanning    0
                Active Scanning     1
*/
#define DRV_WIFI_WID_SCAN_TYPE                      0x0007

// *****************************************************************************
/*
    Summary:
        WID for  privacy invoked field.
    Description:
        DRV_WIFI_WID_PRIVACY_INVOKED field indicates if WEP mechanism is being used for transmission of data frames.
        The possible values are shown below,
        WEP Encryption Indication Value
        Disabled        0
        Enabled         1

*/
#define DRV_WIFI_WID_PRIVACY_INVOKED                0x0008

// *****************************************************************************
/*
    Summary:
        WID for WEP KEY ID.
    Description:
        DRV_WIFI_WID_KEY_ID indicates the index of the WEP encryption key in use, in the key table.
        The possible values are 0, 1, 2 and 3.
*/

#define DRV_WIFI_WID_KEY_ID                          0x0009

// *****************************************************************************
/*
    Summary:
        WID for QOS.
    Description:
        DRV_WIFI_WID_QOS_ENABLE is used to enable/disable WMM Quality of Service.
        The values are 1 for Enable and 0 for Disable.
*/
#define DRV_WIFI_WID_QOS_ENABLE                      0x000A

// *****************************************************************************
/*
    Summary:
        WID for site survey.
    Description:
        DRV_WIFI_WID_SITE_SURVEY indicates the type of site survey required to be done by the STA.
        The possible values are shown below,

        Site Survey                                 Value
        Survey only the currently set channel       0
        Survey all channels                         1
        Disable site survey                         2
*/
#define DRV_WIFI_WID_SITE_SURVEY                    0x000E

// *****************************************************************************
/*
    Summary:
        WID for listen interval.
    Description:
        This field contains the Listen Interval value to be used by a BSS-STA.
        The range is 1 to 255.
*/
#define DRV_WIFI_WID_LISTEN_INTERVAL                    0x000F

// *****************************************************************************
/*
    Summary:
        WID for DTIM period.
    Description:
        DRV_WIFI_WID_DTIM_PERIOD contains the DTIM Period to be used in Access Point mode.
        The range is 1 to 255.
*/
#define DRV_WIFI_WID_DTIM_PERIOD                        0x0010

// *****************************************************************************
/*
    Summary:
        WID for ACK policy.
    Description:
    DRV_WIFI_WID_ACK_POLICY contains the Acknowledgement Policy to be used.
    The values used are shown below,
    In AP/STA mode the ACK policy is used for the transmission of Unicast data packets and only
    the first 2 values are valid for this (Normal ACK and No ACK).

        ACK Policy                          Value
        Normal ACK                          0
        No ACK                              1
        No Explicit ACK                     2
        Block ACK                           3
        Broadcast No ACK                    4
        Compressed Block ACK                5
        Compressed Block ACK Request        6

*/
#define DRV_WIFI_WID_ACK_POLICY                      0x0011

// *****************************************************************************
/*
    Summary:
        WID for reset request.
    Description:
        DRV_WIFI_WID_RESET indicates the reset action to be taken on processing the configuration packet
        containing this WID (along with possibly some other WIDs).
        The possible values are shown below,

        Reset Request                                                       Value
        Not Reset: MAC will not do a reset
        after processing the WIDs in the
        configuration packet even if some
        of the WIDs require a reset.                                        0

        Reset: MAC will do a reset after
        processing the WIDs in the configuration
        packet even if none of the WIDs require a reset.                    1

        No Request: MAC will do a reset after
        processing the WIDs in the configuration
        packet if any of the WIDs require a reset.                          2

*/
#define DRV_WIFI_WID_RESET                          0x0012

// *****************************************************************************
/*
    Summary:
        WID for broadcast SSID.
    Description:
        DRV_WIFI_WID_BCAST_SSID indicates if Broadcast SSID should be used. The possible values are shown below,

        Broadcast SSID                      Value
        Disable SSID broadcast              0
        Enable SSID broadcast               1

        In case of AP, if this option is disabled, the SSID element in Beacon frames will be set to NULL.
        In case of STA if this option is disabled it will not compare the SSID element during scan procedure.
*/
#define DRV_WIFI_WID_BCAST_SSID                         0x0015

// *****************************************************************************
/*
    Summary:
        WID for disconnect state in AP and STA mode.
    Description:
        DRV_WIFI_WID_DISCONNECT has different meaning in STA and AP modes.
        In STA Mode, this WID when set to any value causes the STA to disconnect from the BSS/IBSS network with which it is currently associated.

        In AP Mode, the AP disassociates the Station which has the Association-ID mentioned as the value of this WID.
        The mapping of the Association-ID to the station can be obtained by using DRV_WIFI_WID_CONNECTED_STA_LIST.
        Setting the value as 0, disassociates all the stations currently associated with the AP.
*/
#define DRV_WIFI_WID_DISCONNECT                         0x0016

// *****************************************************************************
/*
    Summary:
        WID for Start Scan request.
    Description:
        DRV_WIFI_WID_START_SCAN_REQ is used to initiate scan procedure in STA mode.
        When the value of this field is set to 1, the STA initiates the scan procedure if site survey option (DRV_WIFI_WID_SITE_SURVEY) is already enabled.
*/
#define DRV_WIFI_WID_START_SCAN_REQ                     0x001E

// *****************************************************************************
/*
    Summary:
        WID for RSSI level.
    Description:
        DRV_WIFI_WID_RSSI indicates the received signal strength value in dB. The range of valid values is -128 to 0.
*/
#define DRV_WIFI_WID_RSSI                               0x001F

// *****************************************************************************
/*
    Summary:
        WID for association status.
    Description:
        DRV_WIFI_WID_ASSOC_STAT provide the association status.
*/
#define DRV_WIFI_WID_ASSOC_STAT                         0x0022


// *****************************************************************************
/*
    Summary:
        WID for SCAN Filter.
    Description:
        DRV_WIFI_WID_SCAN_FILTER is used to restrict the device find only some particular types of networks while scanning and sending results to host.
        Bit Number              Value Set                   Meaning
        BIT1, BIT0              00                          Scan All networks
                                01                          Do not scan Ad-Hoc networks
                                10                          Do not scan Infrastructure networks
                                11                          Do not scan both

        BIT3, BIT2              00                          Scan and report the networks with the best signal strength.
                                                            This filter is applicable only when the number of available networks exceeds the maximum capacity of the station.
                                01                          Scan and report the networks with the worst signal strength.
                                                            This filter is applicable only when the number of available networks exceeds the maximum capacity of the station.
                                10                          Do not consider signal strength while scanning
                                11                          Reserved
        BIT4                    0                           Scan for the networks in all channels
                                1                           Scan for the networks in only the current channel
        BIT5 to BIT7            Any                         Reserved
*/
#define DRV_WIFI_WID_SCAN_FILTER                    0x0036

// *****************************************************************************
/*
    Summary:
        WID for operation mode.
    Description:
        DRV_WIFI_WID_SWITCH_MODE used for switching operationg mode.
        For setting operation mode as Station mode use value 0 and for Access point mode use value 1.
*/
#define DRV_WIFI_WID_SWITCH_MODE                    0x004a


// *****************************************************************************
/*
    Summary:
        WID for enable / disable 11N.
    Description:
        DRV_WIFI_WID_11N_ENABLE is used to enable/disable the High Throughput (HT) feature (802.11n) in the AP/STA. The values are 1 for Enable, 0 for Disable.
*/
#define DRV_WIFI_WID_11N_ENABLE                     0x0082

// *****************************************************************************
/*
    Summary:
        WID for operation type.
    Description:
        DRV_WIFI_WID_11N_OPERATING_TYPE contains the 802.11n operating type. It is valid only if HT option is enabled. The valid values are shown below,
        11n Operating Type                                                                      Value
        HT Mixed: Both HT and legacy STAs are allowed to join the network.                      0

        HT Only (20MHz): Only HT STAs are allowed to join the network and
        only 20MHz operation is permitted in the BSS.                                           1

        HT Only (20/40MHz): Only HT STAs are allowed to join the network
        and 20/40 MHz operation is permitted in the BSS.                                        2
*/
#define DRV_WIFI_WID_11N_OPERATING_TYPE                 0x0083

// *****************************************************************************
/*
    Summary:
        WID for for 11n detection.
    Description:
        DRV_WIFI_WID_11N_OBSS_NONHT_DETECTION contains the 802.11n overlapping BSS Non-HT STA detection setting.
        It is valid only if HT option is enabled. The valid values are shown below,

        11n OBSS non-HT STA Detection                           Value
        Do not detect: The AP does not detect
        the presence of non-HT STAs in OBSS networks.           0

        Detect, do not protect or report: The AP only
        detects the presence of non-HT STAs in OBSS networks.   1

        Detect, protect and do not report: The AP detects
        the presence of non-HT STAs in OBSS networks.
        It also protects the transmissions in its
        own BSS but does not report this to other APs.          2

        Detect, protect and report to other BSS:
        The AP detects the presence of non-HT STAs
        in OBSS networks and reports it to other APs.
        It also protects the transmissions in its own BSS.      3
*/
#define DRV_WIFI_WID_11N_OBSS_NONHT_DETECTION       0x0084

// *****************************************************************************
/*
    Summary:
        WID for for HT protection.
    Description:
        DRV_WIFI_WID_11N_HT_PROT_TYPE contains the type of frame exchange to be used for HT Protection in 802.11n mode. The possible values are shown below,
        11n HT Protection Type                          Value
        RTS-CTS                                         0
        First Frame Exchange at non-HT-rate             1
        LSIG TXOP                                       2
        First Frame Exchange of Mixed frame format      3
*/

#define DRV_WIFI_WID_11N_HT_PROT_TYPE               0x0085

// *****************************************************************************
/*
    Summary:
        WID for enable/disable protection for RIFS transmission.
    Description:
        DRV_WIFI_WID_11N_RIFS_PROT_ENABLE is used to enable/disable protection for RIFS transmission. It is valid only if HT option is enabled.
        The values are 1 for Enable, 0 for Disable.
*/
#define DRV_WIFI_WID_11N_RIFS_PROT_ENABLE               0x0086


// *****************************************************************************
/*
    Summary:
        WID for 11n current transmit MCS index value.
    Description:
        DRV_WIFI_WID_11N_CURRENT_TX_MCS contains the current transmit MCS index value.
        It is valid only if HT option is enabled.
        The valid values depend upon the transmit antenna configuration of the system.
        For a single transmit antenna system, the valid values are from 0 to 7.
        A value of 255 indicates the use of non-HT (or Legacy) rates for HT stations also.
*/

#define DRV_WIFI_WID_11N_CURRENT_TX_MCS              0x0088


// *****************************************************************************
/*
    Summary:
        WID for 11n tx bandwidth.
    Description:
        DRV_WIFI_WID_11N_CURRENT_TX_BW contains the current transmission bandwidth.
        The possible values are 0 for Auto 20 or 40 MHz bandwidth and 1 for 20 MHz only bandwidth.
*/
#define DRV_WIFI_WID_11N_CURRENT_TX_BW                  0x00C8

// *****************************************************************************
/*
                    Short Type WIDs
*/
// *****************************************************************************
/*
    Summary:
        WID for RTS threshold.
    Description:
        DRV_WIFI_WID_RTS_THRESHOLD is valid in both AP/STA modes.
        This field contains the RTS threshold value.
        The possible value range is 256 to 65535.
*/
#define DRV_WIFI_WID_RTS_THRESHOLD                  0x1000

// *****************************************************************************
/*
    Summary:
        WID for Fragmentation threshold value.
    Description:
        DRV_WIFI_WID_FRAG_THRESHOLD is valid in both AP/STA modes.
        This field contains the Fragmentation threshold value.
        The possible value range is 256 to 7936.
*/
#define DRV_WIFI_WID_FRAG_THRESHOLD                  0x1001

// *****************************************************************************
/*
    Summary:
        WID for Beacon interval value.
    Description:
        DRV_WIFI_WID_BEACON_INTERVAL is valid in both AP/STA modes.
        This field contains the Beacon interval value in time units.
        The possible value range is 1 to 65535.
*/
#define DRV_WIFI_WID_BEACON_INTERVAL                0x1004

// *****************************************************************************
/*
    Summary:
        WID for active scan time.
    Description:
        DRV_WIFI_WID_ACTIVE_SCAN_TIME is valid only in STA mode.
        This field contains the time in milliseconds for scanning each channel while doing an active scan (scan by transmitting probe request frames).
        The possible value range is 1 to 65535.
*/
#define DRV_WIFI_WID_ACTIVE_SCAN_TIME                   0x100C

// *****************************************************************************
/*
    Summary:
        WID for passive scan time.
    Description:
        DRV_WIFI_WID_PASSIVE_SCAN_TIME is valid only in STA mode.
        This field contains the time in milliseconds for scanning each channel while doing a passive scan (scan by only listening to the channel).
        The possible value range is 1 to 65535.
*/
#define DRV_WIFI_WID_PASSIVE_SCAN_TIME                  0x100D

// *****************************************************************************
/*
    Summary:
        WID for join start timeout.
    Description:
        DRV_WIFI_WID_JOIN_START_TIMEOUT is valid in AP/STA mode.
        This field contains the timeout value in milliseconds for the Start procedure for an AP or IBSS STA and the Join procedure for a STA.
        The possible value range is 1 to 65535.
*/
#define DRV_WIFI_WID_JOIN_START_TIMEOUT             0x100F

// *****************************************************************************
/*
    Summary:
        WID for Authentication timeout.
    Description:
        DRV_WIFI_WID_AUTH_TIMEOUT is valid in AP/STA mode.
        This field contains the timeout value in milliseconds for the Authentication procedure for an AP or a STA.
        The possible value range is 1 to 65535.
*/
#define DRV_WIFI_WID_AUTH_TIMEOUT                   0x1010

// *****************************************************************************
/*
    Summary:
        WID for association timeout.
    Description:
        DRV_WIFI_WID_ASOC_TIMEOUT is valid in AP/STA mode.
        This field contains the timeout value in milliseconds for the Association procedure for an AP or a STA.
        The possible value range is 1 to 65535.
*/
#define DRV_WIFI_WID_ASOC_TIMEOUT                       0x1011

// *****************************************************************************
/*
    Summary:
        WID for WPA/WPA2 handshake timeout.
    Description:
        DRV_WIFI_WID_11I_PROTOCOL_TIMEOUT is valid in AP/STA mode.
        This field contains the timeout value in milliseconds for the WPA/WPA2 handshake procedure for an AP or a BSS STA.
        The possible value range is 1 to 65535.
*/
#define DRV_WIFI_WID_11I_PROTOCOL_TIMEOUT                       0x1012

// *****************************************************************************
/*
    Summary:
        WID for EAPOL response timeout.
    Description:
        DRV_WIFI_WID_EAPOL_RESPONSE_TIMEOUT is valid in AP/STA mode.
        This field contains the timeout value in milliseconds for an EAPOL response reception for an AP or STA.
        The possible value range is 1 to 65535.
*/
#define DRV_WIFI_WID_EAPOL_RESPONSE_TIMEOUT                     0x1013

// *****************************************************************************
/*
    Summary:
        WID for user preference channel.
    Description:
        DRV_WIFI_WID_USER_PREF_CHANNEL contains the encoded format for programming the user preference for band of operation, primary channel and secondary channel offset.
        The bit encoded format for this WID is shown below. A value of 0 for this WID is used to indicate Auto channel selection.

            Bits                                Description
            Bit 7 - 0                           Channel Number
            Bit 9 - 8                           Secondary channel offset

                                                Value       Description
                                                0           SCN (Secondary Channel None)
                                                1           SCA (Secondary Channel Above)
                                                3           SCB (Secondary Channel Below)

            Bit 15                              Frequency Band

                                                Value       Description
                                                0           2.4 GHz
                                                1           5 GHz

*/
#define DRV_WIFI_WID_USER_PREF_CHANNEL                          0x1020

// *****************************************************************************
/*
    Summary:
        WID for current operating channel.
    Description:
        DRV_WIFI_WID_CURR_OPER_CHANNEL indicates the current operating channel in AP/STA mode.
        The bit encoding of this is the same as that for DRV_WIFI_WID_USER_PREF_CHANNEL.
*/
#define DRV_WIFI_WID_CURR_OPER_CHANNEL                          0x1021

// *****************************************************************************
/*
    Summary:
        WID for user scan specified channel.
    Description:
        DRV_WIFI_WID_USER_SCAN_CHANNEL sets the channel to scan.
        The bit encoding of this is the same as that for DRV_WIFI_WID_USER_PREF_CHANNEL.
*/
#define DRV_WIFI_WID_USER_SCAN_CHANNEL                          0x1022

// *****************************************************************************
/*
                    Integer Type WIDs
*/
// *****************************************************************************
/*
    Summary:
        WID for retry count.
    Description:
        DRV_WIFI_WID_RETRY_COUNT is valid in both AP/STA  modes.
        This field specifies the value of the counter which shall increment when an MSDU is successfully transmitted after one or more retransmissions.
        Any value set to this WID results in resetting the counter in the WLAN Library.
*/
#define DRV_WIFI_WID_RETRY_COUNT                                0x2001

// *****************************************************************************
/*
    Summary:
        WID for multiple retry count.
    Description:
        DRV_WIFI_WID_MULTIPLE_RETRY_COUNT is valid in both AP/STA modes.
        This field specifies the value of the counter which shall increment when an MSDU is successfully transmitted after more than one retransmission.
        Any value set to this WID results in resetting the counter in the WLAN Library.
*/
#define DRV_WIFI_WID_MULTIPLE_RETRY_COUNT                   0x2002

// *****************************************************************************
/*
    Summary:
        WID for frame duplicate count.
    Description:
        DRV_WIFI_WID_FRAME_DUPLICATE_COUNT is valid in both AP/STA  modes.
        This field specifies the value of the counter which shall increment when a duplicate frame is received.
        Any value set to this WID results in resetting the counter in the WLAN Library.
*/
#define DRV_WIFI_WID_FRAME_DUPLICATE_COUNT                  0x2003

// *****************************************************************************
/*
    Summary:
        WID for acknowledgement failure count.
    Description:
        DRV_WIFI_WID_ACK_FAILURE_COUNT is valid in both AP/STA and HUT modes.
        This field specifies the value of the counter which shall increment if a valid acknowledgement is not received when expected.
        Any value set to this WID results in resetting the counter in the WLAN Library.
*/
#define DRV_WIFI_WID_ACK_FAILURE_COUNT                  0x2004

// *****************************************************************************
/*
    Summary:
        WID for receive fragment count.
    Description:
        DRV_WIFI_WID_RECEIVED_FRAGMENT_COUNT is valid in both AP/STA  modes.
        This field specifies the value of the counter which shall increment when an MPDU of type Data or Management is received with no FCS error.
        Any value set to this WID results in resetting the counter in the WLAN Library.
*/
#define DRV_WIFI_WID_RECEIVED_FRAGMENT_COUNT                0x2005

// *****************************************************************************
/*
    Summary:
        WID for multicast received frame count.
    Description:
        DRV_WIFI_WID_MCAST_RECEIVED_FRAME_COUNT is valid in both AP/STA modes.
        This field specifies the value of the counter which shall increment when a MSDU is received with the multicast bit set in the destination MAC address.
        Any value set to this WID results in resetting the counter in the WLAN Library.
*/
#define DRV_WIFI_WID_MCAST_RECEIVED_FRAME_COUNT         0x2006

// *****************************************************************************
/*
    Summary:
        WID for FCS error count.
    Description:
        DRV_WIFI_WID_FCS_ERROR_COUNT is valid in both AP/STA modes.
        This field specifies the value of the counter which shall increment when an FCS error is detected in a received MPDU.
        Any value set to this WID results in resetting the counter in the WLAN Library.
*/
#define DRV_WIFI_WID_FCS_ERROR_COUNT                        0x2007

// *****************************************************************************
/*
    Summary:
        WID for success frame count.
    Description:
        DRV_WIFI_WID_SUCCESS_FRAME_COUNT is valid in both AP/STA modes.
        This field specifies the value of the counter which shall increment when an MSDU is successfully transmitted.
        Any value set to this WID results in resetting the counter in the WLAN Library.
*/
#define DRV_WIFI_WID_SUCCESS_FRAME_COUNT                    0x2008

// *****************************************************************************
/*
    Summary:
        WID for Tx fragment count.
    Description:
        DRV_WIFI_WID_TX_FRAGMENT_COUNT is valid in both AP/STA modes.
        This field specifies the value of the counter which shall be incremented when an MPDU with an individual address in the address 1 field is acknowledged or an MPDU with a multicast address in the address 1 field is transmitted.
*/
#define DRV_WIFI_WID_TX_FRAGMENT_COUNT                      0x200B

// *****************************************************************************
/*
    Summary:
        WID for Tx multicast frame count.
    Description:
        DRV_WIFI_WID_TX_MULTICAST_FRAME_COUNT is valid in both AP/STA  modes.
        This field specifies the value of the counter which shall increment when the multicast bit is set in the destination MAC address of a transmitted MSDU.
        When operating as a STA in an ESS, where these frames are directed to the AP, this implies having received an acknowledgment to all associated MPDUs.
*/
#define DRV_WIFI_WID_TX_MULTICAST_FRAME_COUNT             0x200C

// *****************************************************************************
/*
    Summary:
        WID for RTS success count.
    Description:
        DRV_WIFI_WID_RTS_SUCCESS_COUNT is valid in both AP/STA modes.
        This field specifies the value of the counter which shall increment when a CTS is received in response to an RTS.
*/
#define DRV_WIFI_WID_RTS_SUCCESS_COUNT                   0x200D

// *****************************************************************************
/*
    Summary:
        WID for RTS failure count.
    Description:
        DRV_WIFI_WID_RTS_FAILURE_COUNT is valid in both AP/STA  modes.
        This field specifies the value of the counter which shall increment when a CTS is not received in response to an RTS.
*/
#define DRV_WIFI_WID_RTS_FAILURE_COUNT                   0x200E

// *****************************************************************************
/*
    Summary:
        WID for WEP undecryptable count.
    Description:
        DRV_WIFI_WID_WEP_UNDECRYPTABLE_COUNT is valid in both AP/STA  modes.
        This field specifies the value of the counter which shall increment when a frame is received with WEP subfield of the Frame
        Control field set to one and the WEP On value for the key mapped to the transmit station's MAC address indicates that the frame
        should not have been encrypted or that frame is discarded due to receiving station not implementing the privacy option.
*/
#define DRV_WIFI_WID_WEP_UNDECRYPTABLE_COUNT                 0x200F

// *****************************************************************************
/*
    Summary:
        WID for hardware Rx count.
    Description:
        DRV_WIFI_WID_HW_RX_COUNT is valid in both AP/STA modes.
        This field indicates the total number of frames received without PHY level errors.
*/
#define DRV_WIFI_WID_HW_RX_COUNT                             0x2015

// *****************************************************************************
/*
    Summary:
        WID for DGC RSSI threshold high enable/disable.
    Description:
        DRV_WIFI_WID_DGC_RSSI_TH_HIGH is used for enabling/disabling DGC RSSI threshold high.
*/
#define DRV_WIFI_WID_DGC_RSSI_TH_HIGH                        0x2032

// *****************************************************************************
/*
    Summary:
        WID for DGC RSSI threshold low enable/disable.
    Description:
        DRV_WIFI_WID_DGC_RSSI_TH_LOW is used for enabling/disabling DGC RSSI threshold low.
*/
#define DRV_WIFI_WID_DGC_RSSI_TH_LOW                         0x2033

// *****************************************************************************
/*
    Summary:
        WID for setting Blocker detection thresholds.
    Description:
        This WID is used for setting Blocker detection thresholds.
*/
#define DRV_WIFI_WID_BD_TH_SET                               0x2035

// *****************************************************************************
/*
    Summary:
        WID for local 11i configuration.
    Description:
        DRV_WIFI_WID_11I_SETTINGS indicates the local 11i configuration.
        Refer to DRV_PIC32MZW_11I_MASK.
*/
#define DRV_WIFI_WID_11I_SETTINGS                            0x2036

// *****************************************************************************
/*
    Summary:
        WID for Tx power Level.
    Description:
        DRV_WIFI_WID_TX_POWER_LEVELS is valid in both AP/STA modes.
        This field contains the current Tx power levels in dBm in hexadecimal format.

        Description     Power in dBm
        Byte 0          11a
        Byte 1          11b
        Byte 2          11n
        Byte 3          11n40
*/
#define DRV_WIFI_WID_TX_POWER_LEVELS                         0x207f

// *****************************************************************************
/*
    Summary:
        WID for channel bitmap 2.4GHZ.
    Description:
        This wid is used for changing channel bit map of 2.4GHZ.
 */
#define DRV_WIFI_WID_CH_BITMAP_2GHZ                       0x2085

// *****************************************************************************
/*
    Summary:
        WID for channel bitmap 5GHZ.
    Description:
        This wid is used for changing channel bit map of 5GHZ.
 */
#define DRV_WIFI_WID_CH_BITMAP_5GHZ                       0x2086

// *****************************************************************************
/*
    Summary:
        WID for CCA threshold.
    Description:
        This wid is used for changing the MAC CCA Threshold in the PHY Register value table.
        0 to 15 bit  for busy threshold
        16 to 31 bit for clear threshold
 */

#define DRV_WIFI_WID_CCA_THRESHOLD                       0x2087

// *****************************************************************************
/*
                    String Type WIDs
*/
// *****************************************************************************

/*
    Summary:
        Wid for SSID.
    Description:
        DRV_WIFI_WID_SSID is valid in both AP/STA modes. This field contains the SSID of the network.
        The maximum length of this field is 32 bytes.
*/
#define DRV_WIFI_WID_SSID                               0x3000

// *****************************************************************************
/*
    Summary:
        WID for operational rate set.
    Description:
        DRV_WIFI_WID_OPERATIONAL_RATE_SET is valid in both AP/STA modes. This field consists of a string of all the supported rates (in Mbps) separated by commas.
        For example, in case of an 11g system, this field would contain the following value: "1,2,5.5,6,9,11,12,18,24,36,48,54"
*/
#define DRV_WIFI_WID_OPERATIONAL_RATE_SET               0x3001

// *****************************************************************************
/*
    Summary:
        WID for BSSID.
    Description:
        DRV_WIFI_WID_BSSID is valid in both AP/STA modes.
        This field gives the BSSID of the network.
        The length of this field is 6 bytes. Note that in all the modes a GET access is allowed for this parameter.
        However a SET access is allowed only in STA modes. in STA mode the user can configure the preferred BSSID of a network it wishes to join.
*/
#define DRV_WIFI_WID_BSSID                              0x3003

// *****************************************************************************
/*
    Summary:
        WID for WEP Key value.
    Description:
        DRV_WIFI_WID_WEP_KEY_VALUE is valid in AP/STA mode.
        This field contains the value of the WEP encryption key for the currently configured WEP key identifier.
        The configuration parameter DRV_WIFI_WID_KEY_ID should be used for configuring the required WEP key identifier before the corresponding WEP key is programmed.
        The length of the field is 5 bytes for WEP-64 encryption and 13 bytes for WEP-128 encryption.
*/
#define DRV_WIFI_WID_WEP_KEY_VALUE                          0x3004

// *****************************************************************************
/*
    Summary:
        WID for pass phrase key for WPA/WPA2 Personal.
    Description:
        DRV_WIFI_WID_11I_PSK is valid in AP/STA mode.
        This field gives the pass phrase used to generate the Pre-Shared Key when WPA/WPA2 Personal is enabled.
        The length of the field can vary from 8 to 64 bytes. If the input is 64 bytes, then it is taken as the encryption/decryption key directly instead of pre-shared key.
*/
#define DRV_WIFI_WID_11I_PSK                            0x3008

// *****************************************************************************
/*
    Summary:
        WID for MAC address.
    Description:
        DRV_WIFI_WID_MAC_ADDR is valid in both AP/STA  modes.
        This field gives the MAC address of the device.
        The length of this field is 6 bytes. Note that in all the modes a GET access is allowed for this parameter.
*/
#define DRV_WIFI_WID_MAC_ADDR                           0x300C

// *****************************************************************************
/*
    Summary:
        WID for Pre-Shared Key when WPA/WPA2 Enable.
    Description:
        DRV_WIFI_WID_11I_PSK_VALUE is valid in AP/STA mode.
        This field returns the Pre-Shared Key generated when WPA/WPA2 is enabled (refer to DRV_WIFI_WID_802_11I_MODE).
        The length of the field is 64 bytes. This WID is used to store the Pre-Shared key generated in STA (or AP) mode and restore the same key when the mode is switched to AP (or STA) mode.
*/
#define DRV_WIFI_WID_11I_PSK_VALUE                          0x302a

// *****************************************************************************
/*
    Summary:
        WID for Scan ssid.
    Description:
        DRV_WIFI_WID_SCAN_SSID used for setting SCAN SSID.
*/
#define DRV_WIFI_WID_SCAN_SSID                              0x3032

// *****************************************************************************
/*
    Summary:
        WID for giving Debug print level and Module type to MAC
    Description:
        DRV_WIFI_DEBUG_MODULE_LEVEL used to send the debug print level and Module type.
        The value passed using this WID is 1 for ERROR level, 2 for DEBUG level, 3 for INFO level
        and 4 for FUNCTION POINT level prints and Module type in 32-bits.
*/
#define DRV_WIFI_DEBUG_MODULE_LEVEL                           0x3033

#define DRV_WIFI_WID_GET_SCAN_RESULTS                       0x3034

// *****************************************************************************
/*
    Summary:
        WID for join request.
    Description:
        DRV_WIFI_WID_JOIN_REQ used for sending STA mode join request.
*/
#define DRV_WIFI_WID_JOIN_REQ                               0x3205

// *****************************************************************************
/*
                    Binary Type WIDs
*/
// *****************************************************************************
/*
    Summary:
        WID for network information.
    Description:
        DRV_WIFI_WID_NETWORK_INFO is meant for only STA mode.
        It is used by the firmware to asynchronously send a message to host, on reception of a beacon or probe response frame when the STA is in scan mode.
        The following table shows the message body format of this asynchronous network information message.

            Parameter       Beacon/Probe Response length        RSSI value  Beacon/Probe Response frame body
                            (excluding FCS)

            No. of bytes    2                                   1           length
*/
#define DRV_WIFI_WID_NETWORK_INFO                           0x4005

#define DRV_WIFI_WID_STA_JOIN_INFO                          0x4008

// *****************************************************************************
/*
    Summary:
        WID for user status information.
    Description:
        DRV_WIFI_WID_USER_STATUS_INFO provide the user status information.
*/
#define DRV_WIFI_WID_USER_STATUS_INFO                       0x400B

// *****************************************************************************
/*
    Summary:
        WID for regulatory domain.
    Description:
        DRV_WIFI_WID_REG_DOMAIN is used to set the regulatory domain.
        Six bytes containing the regulatory name.
*/
#define DRV_WIFI_WID_REG_DOMAIN                             0x4010

// *****************************************************************************
/*
    Summary:
        WID for regulatory domain information.
    Description:
        DRV_WIFI_WID_REG_DOMAIN_INFO is used to request and report information about regulatory domains.

        Driver -> WLAN - Report request
        Single byte: 0 = All, 1 = Current

        WLAN -> Driver - Report of single regulatory domain
        Byte:
            0           - Index within a group of DRV_WIFI_WID_REG_DOMAIN_INFO reports. 0 = error.
            1           - Total number of reports in a group. 0 = error.
            2           - 0 = Not current, 1 = current
            3 to 8      - Six bytes containing the regulatory name.
                            note: Names shorter than 6 characters are left-justified with trailing zeros ('\0').
*/
#define DRV_WIFI_WID_REG_DOMAIN_INFO                        0x4011

// *****************************************************************************
/*
    Summary:
        WID for password for WPA3 Personal.
    Description:
        DRV_WIFI_WID_RSNA_PASSWORD is valid in AP/STA mode.
        This field gives the password used in Simultaneous Authentication of
        Equals for WPA3 Personal.
        The length of the field can vary from 0 to 63 bytes.
*/
#define DRV_WIFI_WID_RSNA_PASSWORD                          0x4012

typedef enum
{
    DRV_PIC32MZW_WIDOPTYPE_UNDEFINED,
    DRV_PIC32MZW_WIDOPTYPE_WRITE,
    DRV_PIC32MZW_WIDOPTYPE_QUERY
} DRV_PIC32MZW_WIDOPTYPE;

typedef struct
{
    DRV_PIC32MZW_WIDOPTYPE  opType;
    uint8_t                 *buffer;
    uint16_t                maxBufferLen;
    uint8_t                 *pInPtr;
    bool                    error;
} DRV_PIC32MZW_WIDCTX;

void DRV_PIC32MZW_ProcessHostRsp(uint8_t *pHostRsp);

bool DRV_PIC32MZW_MultiWIDInit(DRV_PIC32MZW_WIDCTX *pCtx, uint16_t bufferLen);
bool DRV_PIC32MZW_MultiWIDAddValue(DRV_PIC32MZW_WIDCTX *pCtx, uint16_t wid, uint32_t val);
bool DRV_PIC32MZW_MultiWIDAddData(DRV_PIC32MZW_WIDCTX *pCtx, uint16_t wid, const uint8_t *pData, uint16_t length);
bool DRV_PIC32MZW_MultiWIDAddString(DRV_PIC32MZW_WIDCTX *pCtx, uint16_t wid, const char * val);
bool DRV_PIC32MZW_MultiWIDAddQuery(DRV_PIC32MZW_WIDCTX *pCtx, uint16_t wid);
bool DRV_PIC32MZW_MultiWid_Write(DRV_PIC32MZW_WIDCTX *pCtx);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _WDRV_PIC32MZW_CFG_H */
