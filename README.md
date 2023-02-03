# Don't read me

## Milestone 1

### Assignment

* Screenshots
  * Greengrass group  
    ![GG group](doc/1_GGgroup.png)
  * Message publish & subscribe  
    ![message publish & subscribe](doc/1_pub_sub.png)

* Greengrass is an Edge or cloud computing service? Explain briefly your answer.
  * an edge computing runtime & a cloud service
  * consist of:
    * a edge runtime running on Greengrass core device
      * supposed to be placed physically close to the IoT devices
      * with fast & stable connection with IoT devices
      * for quick, immediate & real-time responses
    * a cloud service
      * for configuration & authentication of Greengrass devices

* How many cores can a Greengrass group have?
  * exactly 1.

* Complete the blank spaces in the figure to describe the basic architecture of AWS Greengrass and explain each element.
  * Edge
    * Device
    * MQTT
    * Greengrass core
    * Device
  * IoT Core

* What type of protocols are used by AWS Greengrass in order to secure the communication between the devices?
  * Reference: <https://docs.aws.amazon.com/greengrass/v1/developerguide/gg-sec.html>
  * ![AWS IoT Greengrass security model](https://docs.aws.amazon.com/images/greengrass/v1/developerguide/images/gg-security.png)
  * HTTPS
    * with TLS for secure communication
    * between AWS IoT services & devices
    * also there is authentication with the cloud IoT core using the X.509 certificates
  * MQTT with TLS
    * between core device and devices, and between AWS IoT Core and core device

* What are the private and public keys? Explain briefly how they are used in the system.
  * used in asymmetric cryptography
  * Private key: Secretly stored in a device
  * Public key: can be shared to other devices
  * The principle is that D_1(E_2(M)) = D_2(E_1(M)) = M, where M is a message, E_1 & E_2 are encryption with a private key & public key respectively, and D_1 & D_2 are decryption with a private key & public key respectively
  * In Greengrass, they are used when:
    * a Greengrass Core authenticates TLS handshake with a AWS IoT core
    * authenticate TLS handshare for MQTT connections
    * encrypt local data

* What happens if you don't have these certificates (keys) in your folder where the basicDiscovery.py file is?
  * Running basicDiscovery.py requires the certificates in the arguments
  * If trying to run basicDiscovery.py with wrong certificates, it will give error messages:
    * Incorrect CA cert.: [X509: NO_CERTIFICATE_OR_CRL_FOUND] no certificate or crl found (_ssl.c:4123)
    * Incorrect device cert.: [SSL] PEM lib (_ssl.c:3874)
    * Because basicDiscovery.py trys to authenticate with cloud IoT Greengrass service at the beginning, but it failed due to the absense of the certificates.

* What is the root-ca certificate that you have in the devices folder?
  * a public key certificate that identifies a publicly-known root certificate authority (CA)
  * basis of an X.509 certificate
    * each X.509 cert. is hashed by a CA, and can be verified with the root-CA cert.

* What types of elements can be chosen from the AWS IoT console when you create a new subscription in a greengrass group.
  * for each subscription we can choose a source, a target and Greengrass topic(s).
    * source / target: a lambda function, a service, a client device or a connector
    * Greengrass topics: formatted with a 'topic filter'

* What are the Greengrass topics?
  * They are MQTT topics
    * a string that serves as an identifier for MQTT messages
    * device can publish & subscribe messages with particular topics that it desires

* Can a device act as publisher and subscriber in different GG topics? If yes, how?
  * Yes
  1. add new subscriptions in AWS console
     * set the device as a source with a topic in a sub., but as a target with another topic in another sub.
  2. run a local programme in the device (e.g., basicDiscovery.py) that publish & subscribe the topics at the same time

* Can multiple devices be subscribed to the same publisher? If yes, how?
  * Yes
  1. add new subscriptions in AWS console, with these devices set as the targets, and set the source as the same publisher
  2. run a local programme on these devices that subscribe for the topics from the publisher.

* What is the discovery service?
  * Reference: <https://docs.aws.amazon.com/greengrass/v1/developerguide/gg-discover-api.html>
  * a cloud service
  * a Greengrass client device always starts the discovery before connecting to the Greengrass core
  * The service provide necessary information for communication with the Greengrass core, e.g.:
    * The group(s) to which the device belongs
    * The IP address and port for the Greengrass core in the group
    * The group CA certificate, which can be used to authenticate the Greengrass core device.

* Why do you need to specify your aws end point while running the discovery service?
  * The service is provided by an AWS cloud server
  * the 'end point' informs the the device where the AWS cloud server is

### Presentation

* Reference: <https://docs.aws.amazon.com/greengrass/v1/developerguide/module1.html>

* Install AWS Greengrass Core V1 on EC2 virtual machine and set up the network settings (Module 1)

* Create 1 Greengrass Core (GreengrassCore06) & 2 Greengrass Device (PublisherG06 & SubscriberG06) under AWS IoT Things <https://eu-central-1.console.aws.amazon.com/iot/home?region=eu-central-1#/thinghub>; all Things come with certificates and I've put those in the OneDrive folder above (Module 2, 4)

* Create 1 Greengrass Group V1 (GreengrassGroup06) and put the 3 Things above inside. <https://eu-central-1.console.aws.amazon.com/iot/home?region=eu-central-1#/greengrass/groups/7c306694-9437-4a83-9b3d-d963159936d8> (Module 2)

* Set up the MQTT topic subscription of greengrass/group06 <https://eu-central-1.console.aws.amazon.com/iot/home?region=eu-central-1#/greengrass/groups/7c306694-9437-4a83-9b3d-d963159936d8/subscriptions> (Module 2)

* Get `basicDiscovery.py` from <https://docs.aws.amazon.com/greengrass/v1/developerguide/IoT-SDK.html>
  * copy certificates into the same folder as `basicDiscovery.py`
  * edit main function for MQTT publication on the message being published
  * edit `def customOnMessage()` for MQTT subscription callback, which would decode the message into a list of x, y & z
  
## Milestone 3

1. What is the secret.h file?

The Secrets.h file is a header file
   * It contains sensitive information
    Information includes:
        * Wi-Fi SSID
        * Credentials for connecting to Wi-Fi
        * Amazon Root CA certificate
        * Device private key
        * Device public key
    Used for connecting the ESP32 to a Wi-Fi network and AWS Greengrass.
   
2. How did they use the AWS Library?

AWS library for ESP32 can be used to connect the device to AWS IoT and interact with other AWS services.
* Once the library is installed, the developer can use the provided API to connect to AWS IoT, publish and subscribe to MQTT topics, access other AWS services like S3 and more.
* ~~The Secrets.h file should include AWS Root CA certificate, device private key and public key as well as the endpoint for the authentication and secure communication process.~~

3. What are the topics esp32/rover and esp32/target being used for?

* "esp32/rover" topic is ~~used for publishing sensor data and current status of the Rover.~~
 * source: published by the python programme in the VirtualBox VM
 * message: position & direction with respective to the game field
   * from image processing of the webcam image
 * usage: feedback of rover position
* "esp32/target" topic is likely used for receiving goal coordinates ~~or instructions from the AWS Greengrass~~ to reach a certain target.
  * source: by the same python programme as above
  * goal coordinate as specified in scenario1.jpg (or anything else being called under the argument `-i something.jpg` with the python script)
    * in the demo code, there is only 1 goal at a time; once the rover arrived the current code, the demo code with update the next goal, but we would like to change the content to an array of goals
* These topics can be used to exchange data between the ESP32 and AWS Greengrass in order to achieve the obstacle avoidance and goal-reaching behavior of the Rover

4. What changes should we make in the secrets.h file in order for the ESP32 to connect to AWS?

To connect to The changes made to the secrets.h file are for connecting the ESP32 to a Wi-Fi network and enabling it to interact with AWS IoT services. 
We had to update:
WIFI_SSID
WIFI_PASSWORD

~~in our case~~To connect `eduroam` (which uses WPA Enterprise), we had to go with slightly different inputs
WIFI_SSIDD
EAP_ANONYMOUS_IDENTITY
EAP_IDENTITY

In `AWS.cpp` we also changed WiFi.begin() to call the WPA Enterprise API.

5. What happens if we do not provide the AWS endpoint?

Without the endpoint, the ESP32 will not be able to connect to the AWS IoT service, and will not be able to get the IP address of the MQTT broker, publish or subscribe to MQTT topics or access other AWS services.

6. Can we publish anything from the ESP32 onto AWS?

Yes, once the ESP32 is connected to AWS IoT service, it can publish data to specific MQTT topics on the AWS IoT service. ~~The data can be sensor data, status updates, or any other information that is relevant to the project. The ESP32 can also subscribe to topics to receive data or instructions from the AWS IoT service.~~ However, we are not planning to publish anything from ESP32 currently.

7. What changes do we need to make in the code in order to publish from the rover to AWS?

We can use the function `myawsclass::publishMessage()` in `main.cpp` (e.g. in loop(), where it reads sensor data and put that on publishMessage()) in ESP32 to publish something.

8. What is the importance of the stayConnected() method?

~~The stayConnected() method is used to maintain an active connection to the AWS IoT service, ensuring that the device can send and receive data and function properly.~~

* <https://ubidots.com/community/t/solved-what-is-the-function-of-client-loop/913>
* allow the client to process incoming messages to send publish data and makes a refresh of the connection

9. Why are we using AWS IoT instead of AWS GG?

We uses AWS IoT service (cloud service) to configure the let devices to discover the IP address of other devices / Greengrass core, and configure the MQTT messages being transferred.

We uses GG as a MQTT brokers to exchange data.

We are not using AWS IoT because the data (image) transfer is slow and has a large bandwidth, and we are not using the Greengrass core because the GG core we used is rather weak (AWS EC2 t2.micro has 1 CPU core & 1 GB RAM only), and we found the computing speed for image processing is better on VM run on the local machine. We then transfer the ~~output~~ processed data to the rover through AWS GG.

10. Can a device act as both the Publisher and Subscriber? If yes, how?

Yes, a device can act as both a publisher and a subscriber in the context of AWS Greengrass. Once the device is connected to the AWS Greengrass, it can use the publish and subscribe APIs provided by the AWS IoT SDK to send and receive data over the local MQTT message broker.
~~This allows for local communication between devices and the ability to process data, make decisions and take actions while disconnected from the internet.
The device can publish data to specific MQTT topics and subscribe to receive updates or instructions from other devices or the cloud.~~

For example, on ESP32,`MQTTClient::publish(TOPIC_NAME, message_buffer)` and `MQTTClient::subscribe(TOPIC_NAME)` can be used to configure the topic being published/subscribed, and `stayConnected()` to update the data to the MQTT broker. Also, under AWS IOT Greengrass group setting, subscriptions to & from the device have to be set corrospondingly.

11. What do we mean by the devices and topics in AWS IoT?

In AWS IoT, "Devices" refer to the physical or virtual Internet-connected devices that interact with the AWS IoT service and 
"Topics" refer to the channels of communication that devices use to send and receive data over the MQTT protocol. Each device can publish data to a specific topic or subscribe to a topic to receive data from other devices. This allows for organized and filtered data communication between the devices and the AWS cloud.

12. What do we do if some student (from Group2) would like to receive the messages from certain camera (say Group1) on their ESP32?

If a student from Group2 would like to receive messages from a certain camera (from Group1) on their ESP32, the following steps can be taken:

   * ~~Make sure that the camera is configured to publish data to a specific topic on the AWS IoT service.~~
   * ~~Ensure that the student's ESP32 is properly configured and connected to the AWS IoT service.~~
   * ~~On the student's ESP32, use the subscribe API provided by the AWS IoT SDK to subscribe to the topic that the camera is publishing to.~~
   * ~~Once the ESP32 is subscribed to the topic, it will receive any data that is published to that topic by the camera.~~
   * ~~The student's ESP32 can then process or act on the received data as required.~~

~~It's important to note that the student's ESP32 should have the necessary permission and authorizations to subscribe to the topic and to receive the messages.~~

One of the method is to add Group2's subscriber devices to AWS IoT Greengrass group in Group1, configure Group2's subscriber to connect to Group1's MQTT broker (Greengrass core) & configure the subscription in the group from Group1's camera to Group2's subscriber;

Another method is to modify Group1's camera such that it publishes to 2 MQTT brokers, one from Group1 & the other from Group2, and each group let their subscribers to connect to their MQTT broker, and configure their AWS IoT Greengrass group accordingly.
