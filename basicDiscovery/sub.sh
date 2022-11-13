#!/usr/bin/bash

python3 basicDiscovery.py --endpoint a5snnrsfqks2n-ats.iot.eu-central-1.amazonaws.com --rootCA AmazonRootCA1.pem --cert 31f08dd056fa78e924e1f6b3886d50e44fa6749194f7290c6ac0684c7547bded-certificate.pem.crt --key 31f08dd056fa78e924e1f6b3886d50e44fa6749194f7290c6ac0684c7547bded-private.pem.key --thingName SubscriberG06 --topic 'greengrass/group06' --mode subscribe
