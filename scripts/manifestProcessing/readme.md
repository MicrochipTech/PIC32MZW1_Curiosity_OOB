# Generating Device Manifest File from the OOB demo MSD

The WFI32E01 module on the curiosity board has an on-board TNG module. The primary security elements of the TNG are exposed via the OOB demo MSD drive. This includes the device certificate, public keys in different slots etc that are required to generate the device's manifest file by scripts in this folder. 

Follow these steps to generate the manifest file of a curiosity board running the OOB demo:

- Clone this repo to your PC.
- Make sure that you have python 3 installed in your PC
- Install python dependencies from the `requirements.txt` file in this scripts folder. 
    ```sh
    python -m pip install -r requirements.txt
    ```
- Power up the Curiosity board.
    - Make sure that a USB cable is connected to the PC from J204 (`USB Power`).
- When the OOB demo boots up, it enumerates a new removable drive in your PC. Note down the drive letter.
- Execute the following commands from the cloned repo.
    ```sh
    cd scripts\ManifestProcessing
    python createManifest_msd.py -d f:
    ```
    - ***Note*** The -d argument to the python file points to the drive letter of the device MSD

- A manifest file with the device serial number prefix will be generated in the scripts folder.
    - It will be copied to the MSD as well.
- Store the `json` file as well as the `log_signer` certificate.
    - `log_signer` certificate is a self signed certificate used to sign the manifest. This is essential to validate the authenticity of the manifest file during device registration.