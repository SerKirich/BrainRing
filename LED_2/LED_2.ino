#include <CurieBLE.h>

#define LED_NUMBER 1
#define LED_PIN 2
#define BUT_PIN 3

const char* SERVICES_UUIDS[2] = {
  "119b10000-e8f2-537e-4f6c-d104768a0001",
  "119b10000-e8f2-537e-4f6c-d104768a0002"
};

const char* CHARS_UUIDS[2] = {
  "19b10001-e8f2-537e-4f6c-d104768a0001",
  "19b10001-e8f2-537e-4f6c-d104768a0002"
};

const char* BUTTS_UUIDS[2] = {
  "19b10001-e8f2-537e-4f6c-d104768a0003",
  "19b10001-e8f2-537e-4f6c-d104768a0004"
};

BLEService ledService(SERVICES_UUIDS[LED_NUMBER]); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEUnsignedCharCharacteristic switchCharacteristic(CHARS_UUIDS[LED_NUMBER], BLERead | BLEWrite | BLENotify);
BLEUnsignedCharCharacteristic buttonCharacteristic(BUTTS_UUIDS[LED_NUMBER], BLERead | BLEWrite | BLENotify);

const int ledPin = 13; // pin to use for the LED

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUT_PIN, INPUT);
  Serial.begin(9600);

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin initialization
  BLE.begin();

  // set advertised local name and service UUID:
  BLE.setLocalName("LED_1");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);
  ledService.addCharacteristic(buttonCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.setValue(0);
  buttonCharacteristic.setValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();
  bool morg = millis() % 2000 > 1000;
  digitalWrite(ledPin, morg);

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      if(digitalRead(BUT_PIN)) buttonCharacteristic.writeByte(0x00);
      else buttonCharacteristic.writeByte(0x01);
      if(switchCharacteristic.written())
      {
        Serial.println(switchCharacteristic.value());
        if(switchCharacteristic.value()) digitalWrite(LED_PIN, HIGH);
        else digitalWrite(LED_PIN, LOW);
      }
      Serial.println("Connected");
        bool morg = millis() % 250 > 125;
        digitalWrite(ledPin, morg);
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}

