/*
 *
 * Created Jun 2, 2022
 * Author: HoaLe
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include "SoundData.h"
#include "XT_DAC_Audio.h"

const char* ssid     = "KIT";
const char* password = "KIT123!@#";

#define BOMB_NO 6
#define SERVER_PORT 4080


byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
WiFiServer server(SERVER_PORT);

XT_Wav_Class Sound(sample);
XT_DAC_Audio_Class DacAudio(GPIO_NUM_25,0);

const uint8_t GPIO_PIN_LIST[BOMB_NO] = {
  GPIO_NUM_12, // 12
  GPIO_NUM_14, // 14
  GPIO_NUM_27, // 27
  GPIO_NUM_33, // ?
  GPIO_NUM_26, // 25
  GPIO_NUM_32 // 32
};

void setup() {
  // Serial
  Serial.begin(115200);

  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password, 1, 1, 1)) { // channel 1, Hidden, limit 1 connection
    log_e("Soft AP creation failed.");
    while(1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");

  // GPIO
  for (size_t i = 0; i < BOMB_NO; i++)
  {
    pinMode(GPIO_PIN_LIST[i], OUTPUT); // output
    digitalWrite(GPIO_PIN_LIST[i], LOW);
  }
}

void playsound() {
  DacAudio.FillBuffer();     
  if (Sound.Playing==false) {
    Serial.println("Playsound.");
    DacAudio.Play(&Sound);
  }
}

unsigned long currentMill = 0;
uint8_t currentBomb = 100;

void loop() {
  WiFiClient client = server.available();
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    while (client.connected()) {            // loop while the client's connected
      if (currentBomb != 100) {
        if (millis() > currentMill + 2300) {
          digitalWrite(GPIO_PIN_LIST[currentBomb], LOW);
          client.write(currentBomb);
          currentBomb = 100;
          DacAudio.StopAllSounds();
        } else {
          playsound();
        }
      }

      if (client.available()) {             // if there's bytes to read from the client,
        uint8_t c = client.read() - 48;             // read a byte, then
        Serial.print("Receive sign from kit: ");
        Serial.println(c);                    // print it out the serial monitor
        currentBomb = c;
        digitalWrite(GPIO_PIN_LIST[c], HIGH);
        currentMill = millis();
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}