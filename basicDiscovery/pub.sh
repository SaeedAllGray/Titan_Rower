#!/usr/bin/bash

python basicDiscovery.py --endpoint a5snnrsfqks2n-ats.iot.eu-central-1.amazonaws.com --rootCA AmazonRootCA1.pem --cert 0e96553588cda650530439bbf11ec9d02bd6f050ec3058a429be802d66f24563-certificate.pem.crt --key 0e96553588cda650530439bbf11ec9d02bd6f050ec3058a429be802d66f24563-private.pem.key --thingName PublisherG06 --topic 'greengrass/group06' --mode publish --message 'Guren Tag! Sent from PublisherG06'
