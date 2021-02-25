from cryptoauthlib import *
from common import *
from manifest_generation_helper import *
import argparse

import os 
import shutil 

parser = argparse.ArgumentParser(description='Generate MSD based Manifest File for PIC32MZW1 Curiosity Board')
parser.add_argument('-d','--drive', type=str, help='Drive path of device MSD')

args = parser.parse_args()

if not args.drive:
    print("Please provide a drive path")
    exit

logger_cert = create_log_signer()

shutil.copyfile("manifest_signer.crt", args.drive+"\\sec\\manifest_signer.crt") 


manifest_data, manifest_name = tng_data(path=args.drive)