/*
 * IoT-Module: Módulo IoT para automação residencial.
 * @Author: Ricardo M. Oliveira
 */
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include "DHT.h"
#include <SPI.h>
#include <EEPROM.h>

#define DHTPIN  5     // The pin used for DHT
#define DHTTYPE DHT11 // The type of DHT used
#define nodeID  1     // The Node ID set to the Module to connect to a RF24Mesh Network
#define PIRPIN  6     // The pin used for the PIR motion sensor

DHT dht(DHTPIN, DHTTYPE);

RF24 radio(7, 8);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

void setup() {
  Serial.begin(115200);

  mesh.setNodeID(nodeID);
  Serial.println("Connecting to the mesh...");
  mesh.begin();
  dht.begin();
  pinMode(PIRPIN, INPUT);
}

void loop() {
  mesh.update();

  readHumidityAndTemperature();
  readPresence();

  delay(5000);
}

void readPresence() {
  int presence = digitalRead(PIRPIN);
  if (!mesh.write(&presence, 'P', sizeof(int))) {
    if ( ! mesh.checkConnection() ) {
      Serial.println("Renewing Address");
      mesh.renewAddress();
    } else {
      Serial.println("Send fail, Test OK");
    }
  } else {
    Serial.print("Send OK: "); Serial.println(presence);
  }
}

void readHumidityAndTemperature() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Sending Humidity of ");
  Serial.println(h);
  send(h, 'H');
  Serial.print("Sending Temperature of ");
  Serial.println(t);
  send(t, 'T');
}

void send(float data, char type) {
  if (!mesh.write(&data, type, sizeof(float))) {

    if ( ! mesh.checkConnection() ) {
      //refresh the network address
      Serial.println("Renewing Address");
      mesh.renewAddress();
    } else {
      Serial.println("Send fail, Test OK");
    }
  } else {
    Serial.print("Send OK: "); Serial.println(data);
  }
}
