/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    msd_app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "msd_app.h"
#include "bsp/bsp.h"
#include "string.h"
#include "system/fs/sys_fs.h"
#include "system/command/sys_command.h"
#include "atca_status.h"
#include "atca_basic.h"
#include "tng/tng_atcacert_client.h"
#include "app_control.h"
#include "ssl.h"
#include "wolfcrypt/asn.h"
#include "wolfcrypt/sha256.h"
#include "cJSON.h"

MSD_APP_DATA msd_appData;

void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void * pEventData, uintptr_t context) {
    /* This is an example of how the context parameter
       in the event handler can be used.*/

    MSD_APP_DATA * appData = (MSD_APP_DATA*) context;

    switch (event) {
        case USB_DEVICE_EVENT_RESET:
        case USB_DEVICE_EVENT_DECONFIGURED:

            /* Device was reset or de-configured. Update LED status */
            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Device is configured. Update LED status */
            break;

        case USB_DEVICE_EVENT_SUSPENDED:
            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS is detected. Attach the device. */
            USB_DEVICE_Attach(appData->usbDeviceHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is not detected. Detach the device */
            USB_DEVICE_Detach(appData->usbDeviceHandle);
            break;

            /* These events are not used in this demo */
        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        case USB_DEVICE_EVENT_SOF:
        default:
            break;
    }
}

static ATCA_STATUS MSD_APP_getDevSerial(uint8_t* sernum) {

    extern ATCAIfaceCfg atecc608_0_init_data;
    ATCA_STATUS atcaStat;
    atcaStat = atcab_init(&atecc608_0_init_data);
    if (ATCA_SUCCESS == atcaStat) {
        atcaStat = atcab_read_serial_number(sernum);
    }
    atcab_release();
    return atcaStat;
}

void MSD_APP_Initialize(void) {
    /* Place the App state machine in its initial state. */
    msd_appData.state = MSD_APP_STATE_INIT;

    /* Set device layer handle as invalid */
    msd_appData.usbDeviceHandle = USB_DEVICE_HANDLE_INVALID;

    /*Register a callback for FS mount*/
    msd_appData.fsMounted = false;

    msd_appData.checkHash = true;
}

static int MSD_APP_Write_errInfo(char* errorString) {
    SYS_FS_HANDLE fd = 0;
    size_t size;

    fd = SYS_FS_FileOpen(MSD_APP_ERR_FILE_NAME, SYS_FS_FILE_OPEN_WRITE);
    if (SYS_FS_HANDLE_INVALID != fd) {
        /*write a config file with default config data. Other option is to create a new diskImage. Not doing that to improve reproducibility with a 
         default MHC configuration*/
        size = SYS_FS_FileWrite(fd, errorString, strlen(errorString));
        SYS_FS_FileSync(fd);
        SYS_FS_FileClose(fd);

        if ((strlen(errorString)) != size) {
            SYS_CONSOLE_PRINT("error writing error file . Size mismatch (got %d on fd %x. FSError = %d) \r\n", (int) size, (int) fd, SYS_FS_Error());
            return -1;
        }
    } else {
        SYS_CONSOLE_PRINT("Error creating new error file (fsError=%d)\r\n", SYS_FS_Error());
        return -2;
    }

    return 0;
}

static uint8_t sernum[ATCA_SERIAL_NUM_SIZE];

static int write_file(const char* fileName, const void *buffer, size_t nbyte) {
    SYS_FS_HANDLE fd = 0;
    size_t size;
    SYS_CONSOLE_PRINT("Creating %s\r\n", fileName);
    fd = SYS_FS_FileOpen(fileName, SYS_FS_FILE_OPEN_WRITE);
    if (SYS_FS_HANDLE_INVALID != fd) {

        size = SYS_FS_FileWrite(fd, buffer, nbyte);
        SYS_FS_FileSync(fd);
        SYS_FS_FileClose(fd);

        if ((nbyte) != size) {
            SYS_CONSOLE_PRINT("error writing %s . Size mismatch (got %d on fd %x. FSError = %d) \r\n", fileName, (int) size, (int) fd, SYS_FS_Error());
            return -1;
        }
    } else {
        SYS_CONSOLE_PRINT("Error creating new %s (fsError=%d)\r\n", fileName, SYS_FS_Error());
        return -2;
    }
    return 0;
}

static int MSD_APP_Write_Config(void) {
    SYS_FS_RESULT fsResult = SYS_FS_RES_FAILURE;

#ifdef MSD_APP_TXT_CONFIG
    fsResult = SYS_FS_FileStat(MSD_APP_TXT_CONFIG_FILE_NAME, &msd_appData.fileStatus);
    if (SYS_FS_RES_FAILURE == fsResult) {
        /*No config file found . Create one. */
        SYS_CONSOLE_PRINT("No TXT config file found. Creating default at "MSD_APP_TXT_CONFIG_FILE_NAME"\r\n");
        if (0 != write_file(MSD_APP_TXT_CONFIG_FILE_NAME, MSD_APP_TXT_CONFIG_DATA, strlen(MSD_APP_TXT_CONFIG_DATA))) {
            return -3;
        }

    } else {
        SYS_CONSOLE_PRINT(" TXT Config file exists\r\n");
    }
#endif

    return 0;
}

static int MSD_APP_Read_cloud_config() {
    /*Read the MQTT config now*/

    SYS_FS_RESULT fsResult = SYS_FS_RES_FAILURE;
    SYS_FS_HANDLE fd = 0;
    size_t size, rSize;
    fsResult = SYS_FS_FileStat(MSD_APP_CLOUD_CONFIG_FILE_NAME, &msd_appData.fileStatus);
    if (SYS_FS_RES_FAILURE != fsResult) {
        fd = SYS_FS_FileOpen(MSD_APP_CLOUD_CONFIG_FILE_NAME, SYS_FS_FILE_OPEN_READ);
        if (SYS_FS_HANDLE_INVALID != fd) {
            /*get size of file*/
            SYS_FS_FileSeek(fd, 0L, SYS_FS_SEEK_END);
            size = SYS_FS_FileTell(fd);
            SYS_FS_FileSeek(fd, 0L, SYS_FS_SEEK_SET);
            char configString[size + 1];
            
            rSize = SYS_FS_FileRead(fd, configString, size);
            SYS_FS_FileClose(fd);

            if (rSize != size) {
                SYS_CONSOLE_PRINT("error reading Cloud config file . Size mismatch (got %d. Expected %d. FSError = %d) \r\n", (int) rSize, (int) size, SYS_FS_Error());
                return -1;
            }

            const cJSON *broker = NULL;
            const cJSON *clientID = NULL;

            cJSON *config_json = cJSON_Parse(configString);
            if (config_json == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL) {
                    SYS_CONSOLE_PRINT("Cloud config parse Error. Error before: %s\n", error_ptr);
                }
                cJSON_Delete(config_json);
                return -2;
            }

            bool err = false;
            broker = cJSON_GetObjectItemCaseSensitive(config_json, "broker");
            if (cJSON_IsString(broker) && (broker->valuestring != NULL)) {
                SYS_CONSOLE_PRINT("    Cloud config broker \"%s\"\r\n", broker->valuestring);
            } else {
                SYS_CONSOLE_PRINT("Error parsing broker from cloud config\r\n");
                err = true;
            }

            clientID = cJSON_GetObjectItemCaseSensitive(config_json, "clientID");
            if (cJSON_IsString(clientID) && (clientID->valuestring != NULL)) {
                SYS_CONSOLE_PRINT("    Cloud config clientID \"%s\"\r\n", clientID->valuestring);
            } else {
                SYS_CONSOLE_PRINT("Error parsing clientID from Cloud config\r\n");
                err = true;
            }

            cJSON_Delete(config_json);
            if (true == err) return -3;

            /*set read config into app control structure.*/
            app_controlData.mqttCtrl.mqttConfigValid = false;
            strncpy(app_controlData.mqttCtrl.mqttBroker, broker->valuestring, APP_CTRL_MAX_BROKER_NAME_LEN - 1);
            strncpy(app_controlData.mqttCtrl.clientId, clientID->valuestring, APP_CTRL_MAX_CLIENT_ID_LEN - 1);
            app_controlData.mqttCtrl.mqttConfigValid = true;
        }
    } else {
        SYS_CONSOLE_PRINT("Error opening Cloud config file (fsError=%d)\r\n", SYS_FS_Error());
        return -1;
    }

    return 0;
}

static int MSD_APP_Read_Config(void) {

#ifdef MSD_APP_TXT_CONFIG
    {
        SYS_FS_RESULT fsResult = SYS_FS_RES_FAILURE;
        SYS_FS_HANDLE fd = 0;
        size_t size, rSize;
        fsResult = SYS_FS_FileStat(MSD_APP_TXT_CONFIG_FILE_NAME, &msd_appData.fileStatus);
        if (SYS_FS_RES_FAILURE != fsResult) {
            fd = SYS_FS_FileOpen(MSD_APP_TXT_CONFIG_FILE_NAME, SYS_FS_FILE_OPEN_READ);
            if (SYS_FS_HANDLE_INVALID != fd) {
                /*get size of file*/
                SYS_FS_FileSeek(fd, 0L, SYS_FS_SEEK_END);
                size = SYS_FS_FileTell(fd);
                SYS_FS_FileSeek(fd, 0L, SYS_FS_SEEK_SET);

                char configString[size + 1];

                //SYS_CONSOLE_PRINT("Config file size : %d bytes \r\n", (int) size);

                rSize = SYS_FS_FileRead(fd, configString, size);
                SYS_FS_FileClose(fd);

                if (rSize != size) {
                    SYS_CONSOLE_PRINT("error reading TXT config file . Size mismatch (got %d. Expected %d. FSError = %d) \r\n", (int) rSize, (int) size, SYS_FS_Error());
                    return -3;
                } else {

                    char *ssid, *password, *authMode;

                    if(rSize < MSD_APP_TXT_CONFIG_FILE_MIN_SIZE)
                    {
                        SYS_CONSOLE_PRINT("error reading TXT config file . Size mismatch (got %d. Expected minimum %d. FSError = %d) \r\n", (int) rSize, MSD_APP_TXT_CONFIG_FILE_MIN_SIZE, SYS_FS_Error());
                        return -3;                        
                    }
                    
                    taskENTER_CRITICAL();
                    ssid = strtok(&configString[19], ",");
                    password = strtok(NULL, ",");
                    authMode = strtok(NULL, ",");
                    taskEXIT_CRITICAL();

                    //no SSID is passed when the auth mode is open
                    if (NULL == authMode) {
                        authMode = password;
                        password = NULL;
                    }

                    uint32_t mode = (uint32_t) atoi(authMode);
                    /*Validate the input*/
                    if ((NULL == ssid) || (mode > 3) || (mode <= 0)) {
                        SYS_CONSOLE_PRINT("error parsing TXT config. (ssid=%s, pass=%s, mode=%d\r\n)", ssid, password, mode);
                        return -4;
                    }

                    SYS_CONSOLE_PRINT("Applying TXT config. (ssid=%s, pass=%s, mode=%d)\r\n", ssid, password, mode);

                    /*set read config into app control structure.*/
                    strncpy(app_controlData.wifiCtrl.SSID, ssid, APP_CTRL_MAX_SSID_LEN - 1);
                    if(password)
                        strncpy(app_controlData.wifiCtrl.pass, password, APP_CTRL_MAX_WIFI_PASS_LEN - 1);
                    
                    app_controlData.wifiCtrl.wifiCtrlValid = false;
                    switch (mode) {
                        case 1:
                            app_controlData.wifiCtrl.authmode = WIFI_OPEN;
                            break;
                        case 2:
                        case 3:
                            app_controlData.wifiCtrl.authmode = WIFI_WPAWPA2MIXED;
                            break;
                        default:
                            app_controlData.wifiCtrl.authmode = WIFI_WPAWPA2MIXED;
                            SYS_CONSOLE_PRINT("Invalid auth mode in TXT config. Using SYS_WIFI_WPA2WPA3MIXED");
                    }
                    app_controlData.wifiCtrl.wifiCtrlValid = true;
                    //return 0;
                }
            }
        } else {
            SYS_CONSOLE_PRINT("Error opening TXT config file (fsError=%d)\r\n", SYS_FS_Error());
            return -2;
        }
    }
#endif // MSD_APP_TXT_CONFIG
    /*all good so far. Now read MQTT config*/
    return MSD_APP_Read_cloud_config();
}

static int MSD_APP_Write_Serial(void) {
    SYS_FS_HANDLE fd = 0;
    size_t size;
    SYS_FS_RESULT fsResult = SYS_FS_RES_FAILURE;

    /*Initialize device serial number into the controlData structure.*/
    char displayStr[ATCA_SERIAL_NUM_SIZE * 3];
    char packedDisplayStr[ATCA_SERIAL_NUM_SIZE * 3];
    size_t displen = sizeof (displayStr);
    size_t packedDispLen = sizeof (packedDisplayStr);
    atcab_bin2hex(sernum, ATCA_SERIAL_NUM_SIZE, displayStr, &displen);
    packHex(displayStr, displen, packedDisplayStr, &packedDispLen);
    strncpy(app_controlData.devSerialStr, packedDisplayStr, (ATCA_SERIAL_NUM_SIZE * 2));
    app_controlData.serialNumValid = true;

    fsResult = SYS_FS_FileStat(MSD_APP_SERIAL_FILE_NAME, &msd_appData.fileStatus);
    if (SYS_FS_RES_FAILURE == fsResult) {
        SYS_CONSOLE_PRINT("Creating serial file at "MSD_APP_SERIAL_FILE_NAME"\r\n");
        fd = SYS_FS_FileOpen(MSD_APP_SERIAL_FILE_NAME, SYS_FS_FILE_OPEN_WRITE);
        if (SYS_FS_HANDLE_INVALID != fd) {
            size = SYS_FS_FileWrite(fd, packedDisplayStr, packedDispLen);
            SYS_FS_FileClose(fd);

            if ((packedDispLen) != size) {
                SYS_CONSOLE_PRINT("error writing serial file . Size mismatch (got %d on fd %x. FSError = %d) \r\n", (int) size, (int) fd, SYS_FS_Error());
                return -1;
            }
        } else {
            SYS_CONSOLE_PRINT("Error creating new serial file (fsError=%d)\r\n", SYS_FS_Error());
            return -2;
        }
    } else {
        //SYS_CONSOLE_PRINT("Serial file already exists \r\n");
    }
    return 0;
}

static int getSubjectKeyID(uint8_t* derCert, size_t derCertSz, char* keyID) {
    DecodedCert cert;
    int ret;

    InitDecodedCert(&cert, (const byte*) derCert, derCertSz, 0);
    ret = ParseCert(&cert, CERT_TYPE, NO_VERIFY, 0);
    if (ret) {
        SYS_CONSOLE_PRINT(TERM_RED"ERROR PARSING KEY IDENTIFIER\r\n"TERM_RESET);
    } else {
        char displayStr[KEYID_SIZE * 3];
        char packedDisplayStr[APP_CTRL_CLIENTID_SIZE];
        size_t displen = sizeof (displayStr);
        size_t packedDispLen = sizeof (packedDisplayStr);

        atcab_bin2hex_(cert.extSubjKeyId, KEYID_SIZE, displayStr, &displen, false, false, false);
        packHex(displayStr, displen, packedDisplayStr, &packedDispLen); /*Check if this needs to be called*/

        SYS_CONSOLE_PRINT(TERM_YELLOW"Certificate Key ID: %.*s\r\n"TERM_RESET, packedDispLen, packedDisplayStr);
        strncpy(keyID, packedDisplayStr, packedDispLen);
    }
    FreeDecodedCert(&cert);
    return ret;
}

/*Pull device, signer and root certificates from the ECC608 device and write them to the MSD. Also re-create the cloud config file*/
static int MSD_APP_Write_certs(void) {
    ATCA_STATUS status;

    SYS_FS_RESULT fsResultRoot = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultSigner = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultDevice = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultClickme = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultCloudConfig = SYS_FS_RES_FAILURE;

    /*Not implementing a loop to make the code more readable*/
    fsResultSigner = SYS_FS_FileStat(MSD_APP_SIGNER_FILE_NAME, &msd_appData.fileStatus);
    fsResultDevice = SYS_FS_FileStat(MSD_APP_DEVCERT_FILE_NAME, &msd_appData.fileStatus);
    fsResultRoot = SYS_FS_FileStat(MSD_APP_ROOTCERT_FILE_NAME, &msd_appData.fileStatus);
    fsResultClickme = SYS_FS_FileStat(MSD_APP_CLICKME_FILE_NAME, &msd_appData.fileStatus);
    fsResultCloudConfig = SYS_FS_FileStat(MSD_APP_CLOUD_CONFIG_FILE_NAME, &msd_appData.fileStatus);


    if ((SYS_FS_RES_FAILURE == fsResultSigner) || (SYS_FS_RES_FAILURE == fsResultDevice) ||
            (SYS_FS_RES_FAILURE == fsResultRoot) || (SYS_FS_RES_FAILURE == fsResultClickme) ||
            (SYS_FS_RES_FAILURE == fsResultCloudConfig)) {

        /*Read root Certificate*/
        size_t rootCertSize = 0;
        status = tng_atcacert_root_cert_size(&rootCertSize);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("    MSD_APP_Write_certs: tng_atcacert_root_cert_size Failed \r\n");
            return status;
        }
        uint8_t rootCert[rootCertSize];
        status = tng_atcacert_root_cert((uint8_t*) & rootCert, &rootCertSize);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("    MSD_APP_Write_certs: tng_atcacert_root_cert Failed \r\n");
            return status;
        }
        /*Read signer cert*/
        size_t signerCertSize = 0;
        status = tng_atcacert_max_signer_cert_size(&signerCertSize);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("    MSD_APP_Write_certs: tng_atcacert_max_signer_cert_size Failed \r\n");
            return status;
        }
        uint8_t signerCert[signerCertSize];
        status = tng_atcacert_read_signer_cert((uint8_t*) & signerCert, &signerCertSize);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("    MSD_APP_Write_certs: tng_atcacert_read_signer_cert Failed \r\n");
            return status;
        }

        /*Read device cert signer by the signer above*/
        size_t deviceCertSize = 0;
        status = tng_atcacert_max_device_cert_size(&deviceCertSize);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("    MSD_APP_Write_certs: tng_atcacert_max_signer_cert_size Failed \r\n");
            return status;
        }

        uint8_t deviceCert[deviceCertSize];
        status = tng_atcacert_read_device_cert((uint8_t*) & deviceCert, &deviceCertSize, (uint8_t*) & signerCert);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("    MSD_APP_Write_certs: tng_atcacert_read_device_cert Failed (%x) \r\n", status);
            return status;
        }

        /*Generate a PEM device certificate.*/
        byte devPem[1024];
        XMEMSET(devPem, 0, 1024);
        int devPemSz;

        devPemSz = wc_DerToPem(deviceCert, deviceCertSize, devPem, sizeof (devPem), CERT_TYPE);
        if ((devPemSz <= 0)) {
            SYS_CONSOLE_PRINT("    Failed converting device Cert to PEM (%d)\r\n", devPemSz);
            return devPemSz;
        }

        /*Write PEM format device certificate to root folder*/
        char devPemFileName[strlen(app_controlData.devSerialStr) + 5];
        sprintf(devPemFileName, "%s.cer", app_controlData.devSerialStr);

        if (0 != write_file(devPemFileName, devPem, devPemSz)) {
            return -1;
        }

        /*Write files to FS*/
        if (0 != write_file(MSD_APP_ROOTCERT_FILE_NAME, rootCert, rootCertSize)) {
            return -2;
        }

        if (0 != write_file(MSD_APP_SIGNER_FILE_NAME, signerCert, signerCertSize)) {
            return -3;
        }

        if (0 != write_file(MSD_APP_DEVCERT_FILE_NAME, deviceCert, deviceCertSize)) {
            return -4;
        }

        char keyID[APP_CTRL_CLIENTID_SIZE];
        memset(keyID, '\0', APP_CTRL_CLIENTID_SIZE);
        if (0 != getSubjectKeyID(deviceCert, deviceCertSize, keyID)) {
            return -5;
        } else {
            char clickmeString[strlen(MSD_APP_CLICKME_DATA_TEMPLATE) + strlen(keyID)];
            sprintf(clickmeString, MSD_APP_CLICKME_DATA_TEMPLATE, keyID);
            if (0 != write_file(MSD_APP_CLICKME_FILE_NAME, clickmeString, strlen(clickmeString))) {
                return -6;
            }

            char voiceClickmeString[strlen(MSD_APP_VOICE_CLICKME_DATA_TEMPLATE) + strlen(keyID)];
            sprintf(voiceClickmeString, MSD_APP_VOICE_CLICKME_DATA_TEMPLATE, keyID);
            if (0 != write_file(MSD_APP_VOICE_CLICKME_FILE_NAME, voiceClickmeString, strlen(voiceClickmeString))) {
                return -7;
            }
        }
        /*Write cloud config if it does not exist. Writing with certs to avoid reading KeyID again*/
        if (SYS_FS_RES_FAILURE == fsResultCloudConfig) {
            char cloudConfigString[strlen(MSD_APP_CLOUD_CONFIG_DATA_TEMPLATE) + strlen(keyID) + APP_CTRL_MAX_BROKER_NAME_LEN];
            sprintf(cloudConfigString, MSD_APP_CLOUD_CONFIG_DATA_TEMPLATE, SYS_MQTT_INDEX0_BROKER_NAME, keyID);
            if (0 != write_file(MSD_APP_CLOUD_CONFIG_FILE_NAME, cloudConfigString, strlen(cloudConfigString))) {
                return -8;
            }
        }
    } else {
        //SYS_CONSOLE_PRINT("Cert files already exists \r\n");
    }
    return 0;
}

static int MSD_APP_Write_keys(void) {

    ATCA_STATUS status;
    SYS_FS_RESULT fsResultkey0 = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultkey1 = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultkey2 = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultkey3 = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultkey4 = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultRootKey = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultSigKey = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultDevKey = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultClickme = SYS_FS_RES_FAILURE;

    fsResultkey0 = SYS_FS_FileStat(MSD_APP_SLOT_0_PUBKEY_FILE_NAME, &msd_appData.fileStatus);
    fsResultkey1 = SYS_FS_FileStat(MSD_APP_SLOT_1_PUBKEY_FILE_NAME, &msd_appData.fileStatus);
    fsResultkey2 = SYS_FS_FileStat(MSD_APP_SLOT_2_PUBKEY_FILE_NAME, &msd_appData.fileStatus);
    fsResultkey3 = SYS_FS_FileStat(MSD_APP_SLOT_3_PUBKEY_FILE_NAME, &msd_appData.fileStatus);
    fsResultkey4 = SYS_FS_FileStat(MSD_APP_SLOT_4_PUBKEY_FILE_NAME, &msd_appData.fileStatus);
    fsResultRootKey = SYS_FS_FileStat(MSD_APP_ROOTCERT_FILE_NAME, &msd_appData.fileStatus);
    fsResultSigKey = SYS_FS_FileStat(MSD_APP_SIGNER_FILE_NAME, &msd_appData.fileStatus);
    fsResultDevKey = SYS_FS_FileStat(MSD_APP_DEVCERT_FILE_NAME, &msd_appData.fileStatus);
    fsResultClickme = SYS_FS_FileStat(MSD_APP_CLICKME_FILE_NAME, &msd_appData.fileStatus);



    if ((SYS_FS_RES_FAILURE == fsResultkey0) || (SYS_FS_RES_FAILURE == fsResultkey1) ||
            (SYS_FS_RES_FAILURE == fsResultkey2) || (SYS_FS_RES_FAILURE == fsResultkey3) ||
            (SYS_FS_RES_FAILURE == fsResultkey4) || (SYS_FS_RES_FAILURE == fsResultRootKey) ||
            (SYS_FS_RES_FAILURE == fsResultSigKey) || (SYS_FS_RES_FAILURE == fsResultDevKey) ||
            (SYS_FS_RES_FAILURE == fsResultClickme)) {

        uint8_t public_key[ATCA_PUB_KEY_SIZE];
        status = atcab_get_pubkey(0, public_key);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("Failed reading Slot0 Public Key");
            return status;
        }
        if (0 != write_file(MSD_APP_SLOT_0_PUBKEY_FILE_NAME, public_key, ATCA_PUB_KEY_SIZE)) {
            SYS_CONSOLE_PRINT("Failed writing Slot0 Public Key");
            return -1;
        }

        status = atcab_get_pubkey(1, public_key);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("Failed reading Slot1 Public Key");
            return status;
        }
        if (0 != write_file(MSD_APP_SLOT_1_PUBKEY_FILE_NAME, public_key, ATCA_PUB_KEY_SIZE)) {
            SYS_CONSOLE_PRINT("Failed writing Slot1 Public Key");
            return -1;
        }

        status = atcab_get_pubkey(2, public_key);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("Failed reading Slot2 Public Key");
            return status;
        }
        if (0 != write_file(MSD_APP_SLOT_2_PUBKEY_FILE_NAME, public_key, ATCA_PUB_KEY_SIZE)) {
            SYS_CONSOLE_PRINT("Failed writing Slot2 Public Key");
            return -1;
        }

        status = atcab_get_pubkey(3, public_key);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("Failed reading Slot3 Public Key");
            return status;
        }
        if (0 != write_file(MSD_APP_SLOT_3_PUBKEY_FILE_NAME, public_key, ATCA_PUB_KEY_SIZE)) {
            SYS_CONSOLE_PRINT("Failed writing Slot3 Public Key");
            return -1;
        }

        status = atcab_get_pubkey(4, public_key);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("Failed reading Slot4 Public Key");
            return status;
        }
        if (0 != write_file(MSD_APP_SLOT_4_PUBKEY_FILE_NAME, public_key, ATCA_PUB_KEY_SIZE)) {
            SYS_CONSOLE_PRINT("Failed writing Slot4 Public Key");
            return -1;
        }

        status = tng_atcacert_root_public_key(public_key);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("Failed reading root Public Key");
            return status;
        }
        if (0 != write_file(MSD_APP_ROOT_PUBKEY_FILE_NAME, public_key, ATCA_PUB_KEY_SIZE)) {
            SYS_CONSOLE_PRINT("Failed writing root Public Key");
            return -1;
        }

        status = tng_atcacert_signer_public_key(public_key, NULL);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("Failed reading Signer Public Key");
            return status;
        }
        if (0 != write_file(MSD_APP_SIGNER_PUBKEY_FILE_NAME, public_key, ATCA_PUB_KEY_SIZE)) {
            SYS_CONSOLE_PRINT("Failed writing Signer Public Key");
            return -1;
        }

        status = tng_atcacert_device_public_key(public_key, NULL);
        if (ATCA_SUCCESS != status) {
            SYS_CONSOLE_PRINT("Failed reading device Public Key");
            return status;
        }
        if (0 != write_file(MSD_APP_DEVICE_PUBKEY_FILE_NAME, public_key, ATCA_PUB_KEY_SIZE)) {
            SYS_CONSOLE_PRINT("Failed writing device Public Key");
            return -1;
        }

    } else {
        //SYS_CONSOLE_PRINT("Slot Key files already exists \r\n");
    }
    return ATCA_SUCCESS;
}

static void timerCallback(uintptr_t context) {
    //SYS_CONSOLE_PRINT("checking for file changes\r\n");
    msd_appData.checkHash = true;
}


uint8_t CACHE_ALIGN work[SYS_FS_FAT_MAX_SS];

static bool checkFSMount() {
#if SYS_FS_AUTOMOUNT_ENABLE
    return msd_appData.fsMounted;
#else
    if (SYS_FS_Mount(SYS_FS_MEDIA_IDX0_DEVICE_NAME_VOLUME_IDX0, SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0, FAT, 0, NULL) != SYS_FS_RES_SUCCESS) {
        return false;
    } else {
        return true;
    }
#endif            
}

static int updateVerFile(void) {
    SYS_FS_HANDLE fd = 0;
    size_t size, nbytes;

    nbytes = strlen(APP_VERSION);

    fd = SYS_FS_FileOpen(MSD_APP_VERSION_FILE_NAME, SYS_FS_FILE_OPEN_READ);
    
    /*Check if the version file needs an update if it exists*/
    if (SYS_FS_HANDLE_INVALID != fd) {
        char versionString[nbytes];
        size = SYS_FS_FileRead(fd, versionString, nbytes);
        SYS_FS_FileClose(fd);
        if (0==strncmp(versionString,APP_VERSION,nbytes)){
            return 0;
        }
    }
    
    /*write a version file in case of 1)file not found 2)version mismatch */
    fd = SYS_FS_FileOpen(MSD_APP_VERSION_FILE_NAME, SYS_FS_FILE_OPEN_WRITE);
    if (SYS_FS_HANDLE_INVALID != fd) {
        size = SYS_FS_FileWrite(fd, APP_VERSION, nbytes);
        SYS_FS_FileSync(fd);
        SYS_FS_FileClose(fd);
        if (nbytes != size) {
            SYS_CONSOLE_PRINT("error writing version . Size mismatch (got %d on fd %x. FSError = %d) \r\n", (int) size, (int) fd, SYS_FS_Error());
            return -2;
        }
    } else {
        SYS_CONSOLE_PRINT("Error creating new version file (fsError=%d)\r\n", SYS_FS_Error());
        return -3;
    }
    return 0;
}

void MSD_APP_Tasks(void) {
    SYS_FS_FORMAT_PARAM opt;
    //    SYS_FS_RESULT fsResult = SYS_FS_RES_FAILURE;

    /* Check the application's current state. */
    switch (msd_appData.state) {
            /* Application's initial state. */
        case MSD_APP_STATE_INIT:
            if (ATCA_SUCCESS == MSD_APP_getDevSerial(sernum)) {
                msd_appData.state = MSD_APP_STATE_WAIT_FS_MOUNT;
            } else {
                SYS_CONSOLE_PRINT(TERM_RED"MSD_APP: Error Reading Device Serial from TNG\r\n"TERM_RESET);
                //                MSD_APP_Write_errInfo("Error Reading Device Serial from TNG");
                msd_appData.state = MSD_APP_CONNECT_USB;
            }
            break;
        case MSD_APP_STATE_WAIT_FS_MOUNT:
            if (checkFSMount()) {
                SYS_CONSOLE_PRINT("MSD_APP: FS Mounted\r\n");
                if (app_controlData.switchData.bootSwitch) {
                    SYS_CONSOLE_PRINT(TERM_CYAN"MSD_APP: Factory config reset requested\r\n"TERM_RESET);
                    msd_appData.state = MSD_APP_STATE_CLEAR_DRIVE;
                }
                else if (SYS_FS_ERROR_NO_FILESYSTEM == SYS_FS_Error()) {
                    SYS_CONSOLE_PRINT(TERM_CYAN"MSD_APP: No Filesystem. Doing a format.\r\n"TERM_RESET);
                    msd_appData.state = MSD_APP_STATE_CLEAR_DRIVE;
                } else {
                    SYS_FS_DirectoryMake(MSD_APP_SEC_DIR_NAME);
                    msd_appData.state = MSD_APP_STATE_TOUCH_FILE;
                }
                /*Remove error file if it exists.*/
                SYS_FS_FileDirectoryRemove(MSD_APP_ERR_FILE_NAME);

            }
            break;

        case MSD_APP_STATE_CLEAR_DRIVE:
        {
            opt.fmt = SYS_FS_FORMAT_FAT;
            opt.au_size = 0;
            if (SYS_FS_DriveFormat(SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0, &opt, (void *) work, SYS_FS_FAT_MAX_SS) != SYS_FS_RES_SUCCESS) {
                /* Format of the disk failed. */
                SYS_CONSOLE_PRINT(TERM_RED" Media Format failed\r\n"TERM_RESET);
                msd_appData.state = MSD_APP_STATE_ERROR;
            } else {
                SYS_FS_DriveLabelSet(SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0, "CUROSITY");
                /* Format succeeded. Open a file. */
                SYS_FS_DirectoryMake(MSD_APP_SEC_DIR_NAME);
                msd_appData.state = MSD_APP_STATE_TOUCH_FILE;
            }
        }
            break;
        case MSD_APP_STATE_TOUCH_FILE:
            SYS_FS_CurrentDriveSet(SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0);
            extern ATCAIfaceCfg atecc608_0_init_data;
            ATCA_STATUS atcaStat;
            /*Write an version file*/
            updateVerFile();
            atcaStat = atcab_init(&atecc608_0_init_data);
            if (ATCA_SUCCESS == atcaStat) {

                if (0 != MSD_APP_Write_Serial()) {
                    MSD_APP_Write_errInfo("Error creating Serial number file");
                    msd_appData.state = MSD_APP_CONNECT_USB;
                    break;
                }

                if (0 != MSD_APP_Write_certs()) {
                    MSD_APP_Write_errInfo("Error creating certificates");
                    msd_appData.state = MSD_APP_CONNECT_USB;
                    atcab_release();
                    break;
                }
                if (0 != MSD_APP_Write_keys()) {
                    MSD_APP_Write_errInfo("Error creating keys");
                    msd_appData.state = MSD_APP_CONNECT_USB;
                    atcab_release();
                    break;
                }
            }
            atcab_release();

            /*Client ID etc is ready. Insert a default config file into FS if one does not exist.*/
            if (0 != MSD_APP_Write_Config()) {
                MSD_APP_Write_errInfo("Error Creating config file");
                msd_appData.state = MSD_APP_CONNECT_USB;
                break;
            }
            /*Read data from active config*/
            if (0 != MSD_APP_Read_Config()) {
                MSD_APP_Write_errInfo("invalid wifi or cloud Config File");
                msd_appData.state = MSD_APP_CONNECT_USB;
                break;
            }

            msd_appData.state = MSD_APP_CONNECT_USB;
            break;
        case MSD_APP_CONNECT_USB:
            msd_appData.usbDeviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);
            if (msd_appData.usbDeviceHandle != USB_DEVICE_HANDLE_INVALID) {
                /* Set the Event Handler. We will start receiving events after
                 * the handler is set */
                USB_DEVICE_EventHandlerSet(msd_appData.usbDeviceHandle, APP_USBDeviceEventHandler, (uintptr_t) & msd_appData);

                //check for file changes every 2 seconds
                SYS_TIME_HANDLE handle = SYS_TIME_CallbackRegisterMS(timerCallback, (uintptr_t) 0, 5000, SYS_TIME_PERIODIC);
                if (handle == SYS_TIME_HANDLE_INVALID) {
                    SYS_CONSOLE_PRINT(TERM_RED"Failed creating a timer for config check\r\n"TERM_RESET);
                }


                msd_appData.state = MSD_APP_STATE_RUNNING;
                break;
            } else {
                SYS_CONSOLE_PRINT("Retrying USB Device Open\r\n");
                //msd_appData.state = MSD_APP_STATE_ERROR;
                break;
            }
        case MSD_APP_STATE_RUNNING:
            break;
        case MSD_APP_STATE_ERROR:
            break;
        default:
            break;
    }
}
/*******************************************************************************
 End of File
 */
