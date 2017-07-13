#include <CurieBLE.h>
#include <QuadDisplay.h>

#define CLIENTS_CNT 2

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

int BLOCKED = 2;
int BLOCKED_COUNT = 0;

BLEDevice peripheral[2];

void attachLed(BLEDevice peripheral, const char* charId1, const char* charId2);
void controlLed(BLEDevice peripheralDevice, int client);

void setup() {
  pinMode(11, OUTPUT);
  pinMode(10, INPUT);
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  Serial.begin(9600);
  while (!Serial.available());
  Serial.read();

  // initialize the BLE hardware
  BLE.begin();

  Serial.println("BLE Central - LED control");
  displayClear(9);
}
int scanning = 1;
int client   = 0;
int state = 0;
int talking = 2;
void loop() {
  // start scanning for peripherals
  if (scanning && !peripheral[client]) {
    Serial.print("Scanning for ");
    Serial.println(client);
    BLE.scanForUuid(SERVICES_UUIDS[client]);
    //Процесс скана запущен, больше не надо:
    scanning = 0;
  } else if (peripheral[client]) {
    // stop scanning
    BLE.stopScan();
    scanning = 0;
  }

  // check if a peripheral has been discovered
  if (peripheral[client] && peripheral[client].connected()) {
    //Serial.print("Connected to ");
    //Serial.println(peripheral[client].localName());
  } else {
    peripheral[client] = BLE.available();
    attachLed(peripheral[client], CHARS_UUIDS[client], BUTTS_UUIDS[client]);
  }

  if (peripheral[0] && peripheral[0].connected() && peripheral[1] && peripheral[1].connected())
  {
    LABEL: state = 0;
    BLOCKED = 2;
    int time_sec;
    while(digitalRead(10))
    {
      time_sec = map(analogRead(A0), 0, 1023, 0, 36) * 10;
      displayInt(9, time_sec);
    }
    while (time_sec > 0)
    {
      Serial.println(BLOCKED);
      if(BLOCKED_COUNT == 2) goto LABEL;
      if(state == 0)
      {
        Serial.println(time_sec);
        controlLed(peripheral[0], 0);
        controlLed(peripheral[1], 1);
        displayInt(9, time_sec);
        delay(1000);
        time_sec--;
      }
      else
      {
        int pot = 512;
        while((pot > 10) && (pot < 1010)) {pot = analogRead(A0); Serial.println(pot);}
        state = 0;
        //Если правильно
        if(pot >= 1010)
        {
          state = 2;
          controlLed(peripheral[talking], talking);
          talking = 2;
          goto LABEL;
        }
        else
        {
          state = 3;
          controlLed(peripheral[talking], talking);
          talking = 2;
          state = 0;
        }
      }
    }
    tone(11, 2000, 2000);
  }

  if (peripheral[client] && peripheral[client].connected()) {
    //Если тут все хорошо, перешли к следующему шагу:
    client = (client + 1) % CLIENTS_CNT;
    if (!peripheral[client]) {
      scanning = 1;
    }
  }
}

void attachLed(BLEDevice peripheral, const char* charId1, const char* charId2) {
  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    // connect to the peripheral
    Serial.print("Connecting... ");
    if (peripheral.connect()) {
      Serial.println("Connected!");
    } else {
      Serial.println("Failed to connect!");
      return;
    }

    // discover peripheral attributes
    Serial.println("Discovering attributes ...");
    if (peripheral.discoverAttributes()) {
      Serial.println("Attributes discovered");
    } else {
      Serial.println("Attribute discovery failed!");
      peripheral.disconnect();
      return;
    }

    // retrieve the LED characteristic
    BLECharacteristic ledCharacteristic = peripheral.characteristic(charId1);

    if (!ledCharacteristic) {
      Serial.println("Peripheral does not have LED characteristic!");
      peripheral.disconnect();
      return;
    } else if (!ledCharacteristic.canWrite()) {
      Serial.println("Peripheral does not have a writable LED characteristic!");
      peripheral.disconnect();
      return;
    } else {
      Serial.println("Characteristic accepted.");
    }
    BLECharacteristic butCharacteristic = peripheral.characteristic(charId2);

    if (!butCharacteristic) {
      Serial.println("Peripheral does not have BUT characteristic!");
      peripheral.disconnect();
      return;
    } else if (!butCharacteristic.canWrite()) {
      Serial.println("Peripheral does not have a writable BUT characteristic!");
      peripheral.disconnect();
      return;
    } else {
      Serial.println("Characteristic accepted.");
    }
  }
}

void controlLed(BLEDevice peripheralDevice, int client)
{
  if (!peripheralDevice.connected()) {
    Serial.print("Unexpectedly disconnected from ");
    Serial.println(client);
    return;
  }

  BLECharacteristic ledCh = peripheralDevice.characteristic(CHARS_UUIDS[client]);
  BLECharacteristic butCh = peripheralDevice.characteristic(BUTTS_UUIDS[client]);

  if (peripheralDevice.connected())
  {
    int buttonVal = byte(butCh.value()[0]);
    if (buttonVal) {
      Serial.print("Read from '");
      Serial.print(peripheralDevice.localName());
      Serial.print("': ");
      Serial.print(buttonVal);
      Serial.print(" at\t");
      Serial.println(millis() / 1000.0);
      Serial.print("BLOCKED = ");
      Serial.println(BLOCKED);
    }

    if(BLOCKED != client)
    {
      butCh.read();
      if (butCh.value()[0])
      {
        ledCh.writeByte(0x01);
        displayInt(9, client);
        state = 1;
        if(peripheralDevice.localName() == "LED_0") talking = 0;
        else talking = 1;
      }
      else ledCh.writeByte(0x00);
      if(state == 2) ledCh.writeByte(0x00);
      if(state == 3)
      {
        BLOCKED = client;
        BLOCKED_COUNT++;
      }
    }
  }
  //Serial.println("Peripheral next step");
  //  delay(1000);
}

