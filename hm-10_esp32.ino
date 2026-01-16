#include <Arduino.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/* ===== HM-10 UUIDs ===== */
#define SERVICE_UUID "0000FFE0-0000-1000-8000-00805F9B34FB"
#define CHAR_UUID    "0000FFE1-0000-1000-8000-00805F9B34FB"

/* ===== UART (как у HM-10) ===== */
#define UART_RX_PIN 20
#define UART_TX_PIN 21
#define UART_BAUD   9600  

#define UART_BUF_SIZE 128
#define UART_PACKET_TIMEOUT_MS 50

HardwareSerial uart1(1);

BLECharacteristic *uartChar;
Preferences prefs;

String deviceName = "HM-10";
bool uartToBleEnabled = true;

bool debug_msg_on = true;

/* ===== AT ===== */
bool isAT(const String &s) {
  return s.startsWith("AT");
}

String handleAT(String cmd) {
  cmd.trim();

  if (cmd == "AT") return "OK";
  if (cmd == "AT+NAME?") return deviceName;

  if (cmd.startsWith("AT+NAME=")) {
    deviceName = cmd.substring(8);
    prefs.putString("name", deviceName);
    return "OK";
  }

  if (cmd == "AT+ECHO=1") {
  uartToBleEnabled = true;
  return "OK";
  }
  
  if (cmd == "AT+ECHO=0") {
    uartToBleEnabled = false;
    return "OK";
  }


  return "ERR";
}

class RxCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    std::string v = c->getValue();
    if (!v.length()) return;

    // ===== ЛОГ (HEX) =====
    Serial.print("[BLE RX HEX] ");
    if (debug_msg_on)
    for (uint8_t b : v) {
      if (b < 0x10) Serial.print("0");
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    Serial.println();

    // ===== BLE -> UART (СТРОГО binary, как HM-10) =====
    uart1.write((uint8_t*)v.data(), v.length());

    // ===== AT-команды (только для BLE-ответа) =====
    String in;
    for (uint8_t b : v) in += char(b);
    in.trim();

    if (isAT(in)) {
      String r = handleAT(in);
      uartChar->setValue(r.c_str());
      uartChar->notify();
    }
  }
};


/* ===== BLE server callbacks ===== */
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) override {
    Serial.println("[BLE] connected");
  }
  void onDisconnect(BLEServer*) override {
    Serial.println("[BLE] disconnected");
    BLEDevice::startAdvertising();
  }
};

void setupBLE() {
  BLEDevice::init(deviceName.c_str());

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);

  uartChar = service->createCharacteristic(
    CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  
  // <<< ВАЖНО: добавить CCCD как у HM-10
  uartChar->addDescriptor(new BLE2902());

  uartChar->setCallbacks(new RxCallbacks());
  service->start();

  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->setScanResponse(true);
  adv->start();

  Serial.println("[BLE] advertising started");
}

void setup() {
  /* ===== USB Serial (диагностика) ===== */
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("SERIAL OK");

  /* ===== UART как у HM-10 ===== */
  uart1.begin(
    UART_BAUD,
    SERIAL_8N1,
    UART_RX_PIN,
    UART_TX_PIN
  );

  Serial.print("UART started, baud = ");
  Serial.println(UART_BAUD);

  prefs.begin("hm10", false);
  deviceName = prefs.getString("name", "HM-10");

  Serial.println("ESP32-C3 HM emulator");
  Serial.println("Name: " + deviceName);

  setupBLE();
}

/*
void loop() {
  // UART -> BLE (как у HM-10) 
  while (uart1.available()) {
    uint8_t b = uart1.read();
  
    Serial.print("[UART RX HEX] ");
    if (b < 0x10) Serial.print("0");
    Serial.println(b, HEX);
  
    if (uartToBleEnabled) {
      uartChar->setValue(&b, 1);
      uartChar->notify();
    }
  }
}
*/

void loop() {
  static uint8_t uartBuf[UART_BUF_SIZE];
  static size_t uartLen = 0;
  static uint32_t lastByteTime = 0;

  // ===== Читаем UART =====
  while (uart1.available()) {
    uint8_t b = uart1.read();

    Serial.print("[UART RX HEX] ");
    if (b < 0x10) Serial.print("0");
    
    if (debug_msg_on)
      Serial.println(b, HEX);

    if (uartLen < UART_BUF_SIZE) {
      uartBuf[uartLen++] = b;
    }

    lastByteTime = millis();
  }

  // ===== Если данные есть и таймаут вышел — отправляем ВЕСЬ пакет =====
  if (uartLen > 0 && (millis() - lastByteTime) >= UART_PACKET_TIMEOUT_MS) {
    uartChar->setValue(uartBuf, uartLen);
    uartChar->notify();   // <<< ОДИН notify НА ВЕСЬ ПАКЕТ

    Serial.print("[UART -> BLE PACKET] len=");
    Serial.println(uartLen);

    uartLen = 0; // очистить буфер
  }
}
