# /*
# * Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
# *
# * Licensed under the Apache License, Version 2.0 (the "License").
# * You may not use this file except in compliance with the License.
# * A copy of the License is located at
# *
# *  http://aws.amazon.com/apache2.0
# *
# * or in the "license" file accompanying this file. This file is distributed
# * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# * express or implied. See the License for the specific language governing
# * permissions and limitations under the License.
# */


import os
import sys
import time
import uuid
import json
import logging
import argparse
from AWSIoTPythonSDK.core.greengrass.discovery.providers import DiscoveryInfoProvider
from AWSIoTPythonSDK.core.protocol.connection.cores import ProgressiveBackOffCore
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
from AWSIoTPythonSDK.exception.AWSIoTExceptions import DiscoveryInvalidRequestException

import random

AllowedActions = ['both', 'publish', 'subscribe']

# General message notification callback


def customOnMessage(message):
    print('Received message on topic %s: %s' %
          (message.topic, message.payload))

    messageDecoded = json.loads(message.payload)

    print('The value of X is: ',
          messageDecoded['sensorValues']['x'])
    print('The value of Y is: ',
          messageDecoded['sensorValues']['y'])
    print('The value of Z is: ',
          messageDecoded['sensorValues']['z'])
    print('The sum is: ', messageDecoded['sensorValues']['x'] +
          messageDecoded['sensorValues']['y'] + messageDecoded['sensorValues']['z'])


MAX_DISCOVERY_RETRIES = 10
GROUP_CA_PATH = "./groupCA/"

# Read in command-line parameters
parser = argparse.ArgumentParser()
parser.add_argument("-e", "--endpoint", action="store", required=True,
                    dest="host", help="Your AWS IoT custom endpoint")
parser.add_argument("-r", "--rootCA", action="store",
                    required=True, dest="rootCAPath", help="Root CA file path")
parser.add_argument("-c", "--cert", action="store",
                    dest="certificatePath", help="Certificate file path")
parser.add_argument("-k", "--key", action="store",
                    dest="privateKeyPath", help="Private key file path")
parser.add_argument("-n", "--thingName", action="store",
                    dest="thingName", default="Bot", help="Targeted thing name")
parser.add_argument("-t", "--topic", action="store", dest="topic",
                    default="sdk/test/Python", help="Targeted topic")
parser.add_argument("-m", "--mode", action="store", dest="mode", default="both",
                    help="Operation modes: %s" % str(AllowedActions))
parser.add_argument("-M", "--message", action="store", dest="message", default="Hello World!",
                    help="Message to publish")
# --print_discover_resp_only used for delopyment testing. The test run will return 0 as long as the SDK installed correctly.
parser.add_argument("-p", "--print_discover_resp_only",
                    action="store_true", dest="print_only", default=False)

args = parser.parse_args()
host = args.host
rootCAPath = args.rootCAPath
certificatePath = args.certificatePath
privateKeyPath = args.privateKeyPath
clientId = args.thingName
thingName = args.thingName
topic = args.topic
print_only = args.print_only

if args.mode not in AllowedActions:
    parser.error("Unknown --mode option %s. Must be one of %s" %
                 (args.mode, str(AllowedActions)))
    exit(2)

if not args.certificatePath or not args.privateKeyPath:
    parser.error(
        "Missing credentials for authentication, you must specify --cert and --key args.")
    exit(2)

if not os.path.isfile(rootCAPath):
    parser.error("Root CA path does not exist {}".format(rootCAPath))
    exit(3)

if not os.path.isfile(certificatePath):
    parser.error("No certificate found at {}".format(certificatePath))
    exit(3)

if not os.path.isfile(privateKeyPath):
    parser.error("No private key found at {}".format(privateKeyPath))
    exit(3)

# Configure logging
logger = logging.getLogger("AWSIoTPythonSDK.core")
logger.setLevel(logging.DEBUG)
streamHandler = logging.StreamHandler()
formatter = logging.Formatter(
    '%(asctime)s - %(name)s - %(levelname)s - %(message)s')
streamHandler.setFormatter(formatter)
logger.addHandler(streamHandler)

# Progressive back off core
backOffCore = ProgressiveBackOffCore()

# Discover GGCs
discoveryInfoProvider = DiscoveryInfoProvider()
discoveryInfoProvider.configureEndpoint(host)
discoveryInfoProvider.configureCredentials(
    rootCAPath, certificatePath, privateKeyPath)
discoveryInfoProvider.configureTimeout(10)  # 10 sec

retryCount = MAX_DISCOVERY_RETRIES if not print_only else 1
discovered = False
groupCA = None
coreInfo = None
while retryCount != 0:
    try:
        discoveryInfo = discoveryInfoProvider.discover(thingName)
        caList = discoveryInfo.getAllCas()
        coreList = discoveryInfo.getAllCores()

        # We only pick the first ca and core info
        groupId, ca = caList[0]
        coreInfo = coreList[0]
        print("Discovered GGC: %s from Group: %s" %
              (coreInfo.coreThingArn, groupId))

        print("Now we persist the connectivity/identity information...")
        groupCA = GROUP_CA_PATH + groupId + "_CA_" + str(uuid.uuid4()) + ".crt"
        if not os.path.exists(GROUP_CA_PATH):
            os.makedirs(GROUP_CA_PATH)
        groupCAFile = open(groupCA, "w")
        groupCAFile.write(ca)
        groupCAFile.close()

        discovered = True
        print("Now proceed to the connecting flow...")
        break
    except DiscoveryInvalidRequestException as e:
        print("Invalid discovery request detected!")
        print("Type: %s" % str(type(e)))
        print("Error message: %s" % str(e))
        print("Stopping...")
        break
    except BaseException as e:
        print("Error in discovery!")
        print("Type: %s" % str(type(e)))
        print("Error message: %s" % str(e))
        retryCount -= 1
        print("\n%d/%d retries left\n" % (retryCount, MAX_DISCOVERY_RETRIES))
        print("Backing off...\n")
        backOffCore.backOff()

if not discovered:
    # With print_discover_resp_only flag, we only woud like to check if the API get called correctly.
    if print_only:
        sys.exit(0)
    print("Discovery failed after %d retries. Exiting...\n" %
          (MAX_DISCOVERY_RETRIES))
    sys.exit(-1)

# Iterate through all connection options for the core and use the first successful one
myAWSIoTMQTTClient = AWSIoTMQTTClient(clientId)
myAWSIoTMQTTClient.configureCredentials(
    groupCA, privateKeyPath, certificatePath)
myAWSIoTMQTTClient.onMessage = customOnMessage

connected = False
for connectivityInfo in coreInfo.connectivityInfoList:
    currentHost = connectivityInfo.host
    currentPort = connectivityInfo.port
    print("Trying to connect to core at %s:%d" % (currentHost, currentPort))
    myAWSIoTMQTTClient.configureEndpoint(currentHost, currentPort)
    try:
        myAWSIoTMQTTClient.connect()
        connected = True
        break
    except BaseException as e:
        print("Error in connect!")
        print("Type: %s" % str(type(e)))
        print("Error message: %s" % str(e))

if not connected:
    print("Cannot connect to core %s. Exiting..." % coreInfo.coreThingArn)
    sys.exit(-2)

# Successfully connected to the core
if args.mode == 'both' or args.mode == 'subscribe':
    myAWSIoTMQTTClient.subscribe(topic, 0, None)
time.sleep(2)

while True:
    if args.mode == 'both' or args.mode == 'publish':

        # generate random num from 1 to 100
        value = []
        for i in range(3):
            value.append(random.randrange(1, 100))

        # pack & publish the message
        message = {"Group": "G06", "sensorValues": {
            "x": value[0], "y": value[1], "z": value[2]}}
        messageJson = json.dumps(message)
        myAWSIoTMQTTClient.publish(topic, messageJson, 0)
        if args.mode == 'publish':
            print('Published topic %s: %s\n' % (topic, messageJson))
    time.sleep(1)
