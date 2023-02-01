/**
 * ESP32 AWS Library
 *
 * Functions to get the crawler coordinates from the Camera over AWS IoT
 *
 * Authors: Vipul Deshpande, Jaime Burbano
 *
 * Reference:
 * https://aws.amazon.com/blogs/compute/building-an-aws-iot-core-device-using-aws-serverless-and-an-esp32/
 */

/*
  Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
  Permission is hereby granted, free of charge, to any person obtaining a copy of this
  software and associated documentation files (the "Software"), to deal in the Software
  without restriction, including without limitation the rights to use, copy, modify,
  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "AWS.h"
#include "environment.h"

/* The MQTT topics that this device should publish/subscribe to */
#define AWS_IOT_PUBLISH_TOPIC "tank/pub"
// #define AWS_IOT_SUBSCRIBE_TOPIC "tank/target"
const String AWS_IOT_SUBSCRIBE_TOPIC[] = {"tank/#"};//, "tank/target"};

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

extern Environment env;

myawsclass::myawsclass()
{
}

void messageHandler(String &topic, String &payload)
{
  const size_t CAPACITY = JSON_ARRAY_SIZE(MAX_TARGET_PTS * 2);
  StaticJsonDocument<CAPACITY> doc;

  if (topic == "tank/rover")
  {
    deserializeJson(doc, payload);

    JsonArray array = doc.as<JsonArray>();

    if (array.size() == 3)
    {
      env.currentPosValid = true;
      xSemaphoreTake(env.currentPosMutex, portMAX_DELAY);
      env.currentPos = Coordinate(array[0].as<int>(), array[1].as<int>());
      env.currentPosAngle = static_cast<float>(array[2].as<int>());
      xSemaphoreGive(env.currentPosMutex);
      Serial.printf("tank/rover updated: (%d, %d, %d)\r\n", (int)env.currentPos.x, (int)env.currentPos.y, (int)env.currentPosAngle.angle);
    }
    else
    {
      Serial.println("incorrect tank/rover dimension: " + topic + " - " + payload);
    }
  }
  else if (topic == "tank/target")
  {
    // put message into targetPath
    deserializeJson(doc, payload);
    JsonArray array = doc.as<JsonArray>();
    env.targetPath.clear();
    env.targetPathCurrent = 0;
    for (int i = 0; i < array.size() / 2; i++)
    {
      int x = array[i * 2].as<int>();
      int y = array[i * 2 + 1].as<int>();
      env.targetPath.push_back(Coordinate(x, y));
    }
    Serial.print("tank/target updated: ");

    // initialize targetPath
    if (env.targetPath.size() > 0)
    {
      env.targetPathValid = true;
    }

    // print targetPath
    for (Coordinate c : env.targetPath)
    {
      Serial.printf("(%d, %d) ", (int)c.x, (int)c.y);
    }
    Serial.println();
  }
  else if (topic == "tank/debug")
  {
    // debug only, override rover velocity
    deserializeJson(doc, payload);

    JsonArray array = doc.as<JsonArray>();

    if (array.size() == 2)
    {
      env.debugMode = true;

      env.currentVelocity.linear = array[0].as<float>();
      env.currentVelocity.angular = array[1].as<float>();
      Serial.printf("tank/debug updated: (%f, %f)\r\n", env.currentVelocity.linear, env.currentVelocity.angular);
    }
    else
    {
      Serial.println("incorrect tank/debug dimension: " + topic + " - " + payload);
    }
  }
  else
  {
    Serial.println("unknown incoming msg: " + topic + " - " + payload);
  }
}

void myawsclass::stayConnected()
{
  client.loop();
}

void myawsclass::connectAWS()
{

  WiFi.mode(WIFI_STA);
#if EDUROAM
  WiFi.begin(WIFI_SSID, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, EAP_PASSWORD);
#else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

  Serial.println("Connecting to Wi-Fi");

  const char cursors[4] = {'|', '/', '-', '\\'};
  int cursor_id = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("\rConnecting ");
    Serial.print(cursors[cursor_id]);
    cursor_id = (cursor_id + 1) % 4;
  }

  Serial.println("\r\nCONNECTED!");

  /* Configure WiFiClientSecure to use the AWS IoT device credentials */
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  /* Connect to the MQTT broker on the AWS endpoint we defined earlier */
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  /* Create a message handler */
  client.onMessage(messageHandler);

  Serial.println("Connecting to AWS IOT");

  // change keepAlive from 10 to 1800 s
  client.setOptions(1800, true, 1000);
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  /* Subscribe to a topic */
  int s = sizeof(AWS_IOT_SUBSCRIBE_TOPIC) / sizeof(AWS_IOT_SUBSCRIBE_TOPIC[0]);
  for (int i = 0; i < s; i++)
  {
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC[i]);
  }

  Serial.println("AWS IoT Connected!");
}

void myawsclass::publishMessage(int16_t sensorValue)
{

  // StaticJsonDocument<200> doc;
  StaticJsonDocument<50> doc;
  // doc["time"] = millis();
  doc["sensor"] = sensorValue;
  char jsonBuffer[512];
  // char jsonBuffer[32];
  serializeJson(doc, jsonBuffer); /* print to client */

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

// void myawsclass::publishMessage(int16_t sen0, int16_t sen1, int16_t sen2)
// {

//   // StaticJsonDocument<200> doc;
//   StaticJsonDocument<50> doc;
//   // doc["time"] = millis();
//   doc["sensor0"] = sen0;
//   doc["sensor1"] = sen1;
//   doc["sensor2"] = sen2;
//   char jsonBuffer[512];
//   // char jsonBuffer[32];
//   serializeJson(doc, jsonBuffer); /* print to client */

//   client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
// }

myawsclass awsobject = myawsclass(); /* creating an object of class aws */
