# Frequently Asked Questions: Secure Cloud Connectivity and Voice Control Demo for Microchip PIC32MZW1 Curiosity Board

#### 1.  The board is not able to connect to the cloud from my office network

The most common issue in this scenario is the firewall configurations of the office network. Cloud connection is done over the secure MQTT port 8883. Many office firewalls block this port. Please talk to the IT department of your office to allow port 8883 in your firewall.

V2.0.0 onwards, the demo uses ALPN and connects over port 443 instead of port 8883.

#### 2.  **Why am I not able to connect to an access point configured to use channel 13**

For regulatory compliance, the OOB demo is configured to use RF channels that works worldwide. Since channel 13 is restricted in some countries, it is disabled. Refer to the device software user guide to get information about regulatory domain selection APIs.

#### 3.  **Why does the device image corrupt when I try to edit the configuration file?**

The configuration is stored in a FAT16 filesystem hosted in the external SPI flash and exposed via USB as a mass storage device. We are limited to FAT16 since we need to use a small file system. However, FAT16 causes some issues with modern editors and browsers. Please read the demo README for more details. 

While downloading configurations, download them to your PC and then copy it to the device. While editing the configuration files, use Notepad.exe
