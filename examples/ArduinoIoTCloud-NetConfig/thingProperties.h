#if defined(ARDUINO_SAMD_MKRGSM1400) || defined(ARDUINO_SAMD_MKRNB1500) || defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310) \
|| (defined(BOARD_HAS_SECRET_KEY) && !defined(ARDUINO_UNOR4_WIFI)) || defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#error "This example is not compatible with this board."
#endif
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "ConfiguratorAgents/agents/BLE/BLEAgent.h"
#include "ConfiguratorAgents/agents/Serial/SerialAgent.h"

#if !(defined(HAS_TCP) || defined(HAS_LORA))
  #error  "Please check Arduino IoT Cloud supported boards list: https://github.com/arduino-libraries/ArduinoIoTCloud/#what"
#endif

void onLedChange();

bool led;
int potentiometer;
int seconds;

GenericConnectionHandler ArduinoIoTPreferredConnection;
KVStore kvStore;
NetworkConfiguratorClass NetworkConfigurator(ArduinoIoTPreferredConnection);
BLEAgentClass BLEAgent;
SerialAgentClass SerialAgent;

void initProperties() {
  NetworkConfigurator.addAgent(BLEAgent);
  NetworkConfigurator.addAgent(SerialAgent);
  NetworkConfigurator.setStorage(kvStore);
  ArduinoCloud.setConfigurator(NetworkConfigurator);
#if defined(ARDUINO_OPTA)
  ArduinoCloud.setReconfigurePin(BTN_USER, INPUT);
#endif

  ArduinoCloud.addProperty(led, Permission::Write).onUpdate(onLedChange);
  ArduinoCloud.addProperty(potentiometer, Permission::Read).publishOnChange(10);
  ArduinoCloud.addProperty(seconds, Permission::Read).publishOnChange(1);

}
