"""
Manifest generation
"""
# (c) 2015-2019 Microchip Technology Inc. and its subsidiaries.
#
# Subject to your compliance with these terms, you may use Microchip software
# and any derivatives exclusively with Microchip products. It is your
# responsibility to comply with third party license terms applicable to your
# use of third party software (including open source software) that may
# accompany Microchip software.
#
# THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
# EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
# WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
# PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
# SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
# OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
# MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
# FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
# LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
# THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
# THIS SOFTWARE.


import unicodedata
import re
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
from cryptography.utils import int_from_bytes
from cryptoauthlib import *
from common import *
from jose import utils

from manifest_signing_helper import *


ATCA_SUCCESS = Status.ATCA_SUCCESS


def get_common_name(name):
    """
    Get the common name string from a distinguished name (RDNSequence)
    """
    for attr in name:
        if attr.oid == x509.oid.NameOID.COMMON_NAME:
            return attr.value
    return None


def make_valid_filename(s):
    """
    Convert an arbitrary string into one that can be used in an ascii filename.
    """
    if sys.version_info[0] <= 2:
        if not isinstance(s, unicode):
            s = str(s).decode('utf-8')
    else:
         s = str(s)
    # Normalize unicode characters
    s = unicodedata.normalize('NFKD', s).encode('ascii', 'ignore').decode('ascii')
    # Remove non-word and non-whitespace characters
    s = re.sub(r'[^\w\s-]', '', s).strip()
    # Replace repeated whitespace with an underscore
    s = re.sub(r'\s+', '_', s)
    # Replace repeated dashes with a single dash
    s = re.sub(r'-+', '-', s)
    return s


def tng_data(log_key_path='manifest_signer.key', log_cert_path='manifest_signer.crt',path=None):
    """Read the TNG keys and certificate chains from the device."""

    if not path:
        return None, None

    certs = []

    print('TNG Root Certificate:')

    with open(path+"sec\\rootCert.der", "rb") as f:
        root_cert_der = f.read()

    root_cert = x509.load_der_x509_certificate(root_cert_der, default_backend())
    certs.insert(0, root_cert)

    print(get_common_name(root_cert.subject))
    print(root_cert.public_bytes(encoding=Encoding.PEM).decode('utf-8'))

    print('TNG Root Public Key:')
    # Note that we could, of course, pull this from the root certificate above.
    # However, this demonstrates the tng_atcacert_root_public_key() function.
    root_public_key_raw = bytearray(64)

    with open(path+"sec\\root.key", "rb") as f:
        root_public_key_raw = f.read()

    root_public_key = ec.EllipticCurvePublicNumbers(
        curve=ec.SECP256R1(),
        x=int_from_bytes(root_public_key_raw[0:32], byteorder='big'),
        y=int_from_bytes(root_public_key_raw[32:64], byteorder='big'),
    ).public_key(default_backend())

    # Prove that cert public key and the public key from the func are the same
    cert_spk_der = root_cert.public_key().public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    func_spk_der = root_public_key.public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    assert cert_spk_der == func_spk_der

    print(root_public_key.public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.PEM
    ).decode('utf-8'))

    print('Validate Root Certificate:')
    root_public_key.verify(
        signature=root_cert.signature,
        data=root_cert.tbs_certificate_bytes,
        signature_algorithm=ec.ECDSA(root_cert.signature_hash_algorithm)
    )
    print('OK\n')

    print('TNG Signer Certificate:')
    signer_cert_der_size = AtcaReference(0)
    
    with open(path+"sec\\signerCert.der", "rb") as f:
        signer_cert_der = f.read()

    signer_cert = x509.load_der_x509_certificate(signer_cert_der, default_backend())
    certs.insert(0, signer_cert)

    print(get_common_name(signer_cert.subject))
    print(signer_cert.public_bytes(encoding=Encoding.PEM).decode('utf-8'))

    print('TNG Signer Public Key:')
    # Note that we could, of course, pull this from the signer certificate above.
    # However, this demonstrates the tng_atcacert_signer_public_key() function.
    signer_public_key_raw = bytearray(64)
    with open(path+"sec\\signer.key", "rb") as f:
        signer_public_key_raw = f.read()
    

    signer_public_key = ec.EllipticCurvePublicNumbers(
        curve=ec.SECP256R1(),
        x=int_from_bytes(signer_public_key_raw[0:32], byteorder='big'),
        y=int_from_bytes(signer_public_key_raw[32:64], byteorder='big'),
    ).public_key(default_backend())

    # Prove that cert public key and the public key from the func are the same
    cert_spk_der = signer_cert.public_key().public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    func_spk_der = signer_public_key.public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    assert cert_spk_der == func_spk_der

    print(signer_public_key.public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.PEM
    ).decode('utf-8'))

    # Note that this is a simple cryptographic validation and does not check
    # any of the actual certificate data (validity dates, extensions, names,
    # etc...)
    print('Validate Signer Certificate:')
    root_public_key.verify(
        signature=signer_cert.signature,
        data=signer_cert.tbs_certificate_bytes,
        signature_algorithm=ec.ECDSA(signer_cert.signature_hash_algorithm)
    )
    print('OK\n')

    print('TNG Device Certificate:')
    device_cert_der_size = AtcaReference(0)
    
    with open(path+"sec\\deviceCert.der", "rb") as f:
        device_cert_der = f.read()

    device_cert = x509.load_der_x509_certificate(device_cert_der, default_backend())
    certs.insert(0, device_cert)

    print(get_common_name(device_cert.subject))
    print(device_cert.public_bytes(encoding=Encoding.PEM).decode('utf-8'))

    print('TNG Device Public Key:')
    # Note that we could, of course, pull this from the device certificate above.
    # However, this demonstrates the tng_atcacert_device_public_key() function.
    device_public_key_raw = bytearray(64)
    
    with open(path+"sec\\device.key", "rb") as f:
        device_public_key_raw = f.read()

    device_public_key = ec.EllipticCurvePublicNumbers(
        curve=ec.SECP256R1(),
        x=int_from_bytes(device_public_key_raw[0:32], byteorder='big'),
        y=int_from_bytes(device_public_key_raw[32:64], byteorder='big'),
    ).public_key(default_backend())

    # Prove that cert public key and the public key from the func are the same
    cert_spk_der = device_cert.public_key().public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    func_spk_der = device_public_key.public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    assert cert_spk_der == func_spk_der

    print(device_public_key.public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.PEM
    ).decode('utf-8'))

    # Note that this is a simple cryptographic validation and does not check
    # any of the actual certificate data (validity dates, extensions, names,
    # etc...)

    print('Validate Device Certificate:')
    signer_public_key.verify(
        signature=device_cert.signature,
        data=device_cert.tbs_certificate_bytes,
        signature_algorithm=ec.ECDSA(device_cert.signature_hash_algorithm)
    )
    print('OK\n')

    device_entry = {
        'version': 1,
        'model': 'ATECC608A',
        'partNumber': 'ATECC608A-TNGTLS',
        'manufacturer': {
            'organizationName': 'Microchip Technology Inc',
            'organizationalUnitName': 'Secure Products Group'
        },
        'provisioner': {
            'organizationName': 'Microchip Technology Inc',
            'organizationalUnitName': 'Secure Products Group'
        },
        'distributor': {
            'organizationName': 'Microchip Technology Inc',
            'organizationalUnitName': 'Microchip Direct'
        }
    }

    device_entry['provisioningTimestamp'] = device_cert.not_valid_before.strftime('%Y-%m-%dT%H:%M:%S.%f')[:-3] + 'Z'

    
    with open(path+"serial.txt", "r") as f:
        serial_number = f.read()

    device_entry['uniqueId'] = serial_number

    device_entry['publicKeySet'] = {
        'keys': [
            {
                'kid': '0',
                'kty': 'EC',
                'crv': 'P-256',
                'x': None,
                'y': None,
                'x5c': [
                    base64.b64encode(device_cert_der).decode('ascii'),
                    base64.b64encode(signer_cert_der).decode('ascii')
                ]
            },
            {'kid': '1', 'kty': 'EC', 'crv': 'P-256', 'x': None, 'y': None},
            {'kid': '2', 'kty': 'EC', 'crv': 'P-256', 'x': None, 'y': None},
            {'kid': '3', 'kty': 'EC', 'crv': 'P-256', 'x': None, 'y': None},
            {'kid': '4', 'kty': 'EC', 'crv': 'P-256', 'x': None, 'y': None}
        ]
    }

    for key in device_entry['publicKeySet']['keys']:
        public_key = bytearray(64)
        print('reading slot {} public key'.format(key['kid']))
        
        with open(path+'sec\\slot'+key['kid']+'.key', "rb") as f:
            public_key = f.read()

        key['x'] = utils.base64url_encode(public_key[0:32]).decode('ascii')
        key['y'] = utils.base64url_encode(public_key[32:64]).decode('ascii')

    # If a logging key and certificate was provided create a manifest file
    log_key, log_cert = load_key_and_cert(log_key_path, log_cert_path)

    # Generate the key and certificate ids for JWS
    log_key_id = jws_b64encode(log_cert.extensions.get_extension_for_class(x509.SubjectKeyIdentifier).value.digest)
    log_cert_id = jws_b64encode(log_cert.fingerprint(hashes.SHA256()))

    # Precompute the JWT header
    jws_header = {'typ': 'JWT', 'alg': 'ES256', 'kid': log_key_id, 'x5t#S256': log_cert_id}
    jws_header = jws_b64encode(json.dumps(jws_header).encode('ascii'))

    manifest = json.dumps([create_signed_entry(device_entry, log_key, jws_header)], indent=2).encode('ascii')

    filename = make_valid_filename(device_entry['uniqueId']) + '_manifest' + '.json'
    with open(filename, 'wb') as f:
        f.write(manifest)

    #Write manifest to the device as well
    with open(path+filename, 'wb') as f:
        f.write(manifest)

    print('\n\nGenerated the manifest file ' + filename)

    return manifest, filename

