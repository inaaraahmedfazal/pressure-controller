#include <bluefruit.h>
const int red_q2 = 5;
const int green_q1 = 6;
// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery
void setup() {
  Serial.begin(115200);
  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  //Not sure what this does
  bledfu.begin();

  Serial.println("Bluefruit52 BLEUART Example");
  Serial.println("---------------------------\n");

  // Set up device info service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Adafruit Feather Express nRF52840");
  bledis.begin();

  // Start UART?
  bleuart.begin();

  // Battery service?
  blebas.begin();
  blebas.write(100);

  pinMode(red_q2, OUTPUT);
  pinMode(green_q1, OUTPUT);
}

void advertise(void) {
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

bool central_connected = false;
int ctr = 0;

void connect_callback(uint16_t conn_handle) {
  //Set light green
  digitalWrite(green_q1, HIGH);
  central_connected = true;
    // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  // Turn off green light
  digitalWrite(green_q1, LOW);
  central_connected = false;
}

void loop() {
  digitalWrite(red_q2, HIGH);
  while(central_connected) {
    uint8_t value = static_cast<uint8_t>(ctr);
    bleuart.write(value);
    ctr++;
    delay(1000);
  }
}