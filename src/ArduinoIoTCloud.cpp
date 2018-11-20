#include <ArduinoECCX08.h>
#include "utility/ECCX08Cert.h"
#include "CloudSerial.h"
#include "ArduinoIoTCloud.h"

const static int keySlot                                   = 0;
const static int compressedCertSlot                        = 10;
const static int serialNumberAndAuthorityKeyIdentifierSlot = 11;
const static int thingIdSlot                               = 12;

ArduinoIoTCloudClass::ArduinoIoTCloudClass() :
  _thing_id     (""),
  _bearSslClient(NULL),
  _mqttClient   (MQTT_RECEIVE_BUFFER_SIZE)
{
}

ArduinoIoTCloudClass::~ArduinoIoTCloudClass()
{
  if (_bearSslClient) {
    delete _bearSslClient;
    _bearSslClient = NULL;
  }
}

int ArduinoIoTCloudClass::begin(Client& net, String brokerAddress)
{
  // store the broker address as class member
  _brokerAddress = brokerAddress;

  byte thingIdBytes[72];

  if (!ECCX08.begin()) {
    return 0;
  }

  if (!ECCX08.readSlot(thingIdSlot, thingIdBytes, sizeof(thingIdBytes))) {
    return 0;
  }
  _id = (char*)thingIdBytes;

  if (!ECCX08Cert.beginReconstruction(keySlot, compressedCertSlot, serialNumberAndAuthorityKeyIdentifierSlot)) {
    return 0;
  }

  ECCX08Cert.setSubjectCommonName(_id);
  ECCX08Cert.setIssuerCountryName("US");
  ECCX08Cert.setIssuerOrganizationName("Arduino LLC US");
  ECCX08Cert.setIssuerOrganizationalUnitName("IT");
  ECCX08Cert.setIssuerCommonName("Arduino");

  if (!ECCX08Cert.endReconstruction()) {
    return 0;
  }

  if (_bearSslClient) {
    delete _bearSslClient;
  }
  _bearSslClient = new BearSSLClient(net);
  _bearSslClient->setEccSlot(keySlot, ECCX08Cert.bytes(), ECCX08Cert.length());

  // Begin function for the MQTTClient
  mqttClientBegin(*_bearSslClient);

  Thing.begin();

  return 1;
}

// private class method used to initialize mqttClient class member. (called in the begin class method)
void ArduinoIoTCloudClass::mqttClientBegin(Client& net)
{
  // MQTT topics definition
  _stdoutTopic  = "/a/d/" + _id + "/s/o";
  _stdinTopic   = "/a/d/" + _id + "/s/i";
  if(_thing_id == "") {
    _dataTopicIn  = "/a/d/" + _id + "/e/i";
    _dataTopicOut = "/a/d/" + _id + "/e/o";
  }
  else {
    _dataTopicIn  = "/a/t/" + _thing_id + "/e/i";
    _dataTopicOut = "/a/t/" + _thing_id + "/e/o";
  }

  // use onMessage as callback for received mqtt messages
  _mqttClient.onMessageAdvanced(ArduinoIoTCloudClass::onMessage);
  _mqttClient.begin(_brokerAddress.c_str(), 8883, net);

  // Set MQTT connection options
  _mqttClient.setOptions(mqttOpt.keepAlive, mqttOpt.cleanSession, mqttOpt.timeout);
}

int ArduinoIoTCloudClass::connect()
{
  // Username: device id
  // Password: empty
  if (!_mqttClient.connect(_id.c_str())) {
    return 0;
  }
  _mqttClient.subscribe(_stdinTopic);
  _mqttClient.subscribe(_dataTopicIn);

  return 1;
}

bool ArduinoIoTCloudClass::disconnect()
{
  return _mqttClient.disconnect();
}

void ArduinoIoTCloudClass::poll()
{
  update();
}

void ArduinoIoTCloudClass::update()
{
  // If user call update() without parameters use the default ones
  update(MAX_RETRIES, RECONNECTION_TIMEOUT);
}

bool ArduinoIoTCloudClass::mqttReconnect(int const maxRetries, int const timeout)
{
  // Counter for reconnection retries
  int retries = 0;
  unsigned long start = millis();

  // Check for MQTT broker connection, of if maxReties limit is reached
  // if MQTTClient is connected , simply do nothing and retun true
  while(!_mqttClient.connected() && (retries++ < maxRetries) && (millis() - start < timeout)) {

    // Get last MTTQClient error, (a common error may be a buffer overflow)
    lwmqtt_err_t err = _mqttClient.lastError();

    // try establish the MQTT broker connection
    connect();
  }

  // It was impossible to establish a connection, return
  if ((retries == maxRetries) || (millis() - start >= timeout))
    return false;

  return true;
}

void ArduinoIoTCloudClass::update(int const reconnectionMaxRetries, int const reconnectionTimeoutMs)
{
  // Method's argument controls
  int const maxRetries = (reconnectionMaxRetries > 0) ? reconnectionMaxRetries : MAX_RETRIES;
  int const timeout    = (reconnectionTimeoutMs  > 0) ? reconnectionTimeoutMs  : RECONNECTION_TIMEOUT;

  // If the reconnect() culd not establish the connection, return the control to the user sketch
  if (!mqttReconnect(maxRetries, timeout))
    return;

  // MTTQClient connected!, poll() used to retrieve data from MQTT broker
  _mqttClient.loop();

  uint8_t data[MQTT_RECEIVE_BUFFER_SIZE];
  int const length = Thing.encode(data, sizeof(data));
  if (length > 0) {
    writeProperties(data, length);
  }
}

int ArduinoIoTCloudClass::reconnect(Client& net)
{
  // check if MQTT client is still connected
  if (_mqttClient.connected()) {
    while(!_mqttClient.disconnect());
  }

  // Re-initialize _bearSslClient
  if (_bearSslClient) {
      delete _bearSslClient;
    }
  _bearSslClient = new BearSSLClient(net);
  _bearSslClient->setEccSlot(keySlot, ECCX08Cert.bytes(), ECCX08Cert.length());

  // Initialize again the MQTTClient, otherwise it would not be able to receive messages through its callback
  mqttClientBegin(*_bearSslClient);
  // Connect to the broker
  return connect();
}

void ArduinoIoTCloudClass::onGetTime(unsigned long(*callback)(void))
{
  ArduinoBearSSL.onGetTime(callback);
}

int ArduinoIoTCloudClass::connected()
{
  return _mqttClient.connected();
}

int ArduinoIoTCloudClass::writeProperties(const byte data[], int const length)
{
  return _mqttClient.publish(_dataTopicOut.c_str(), (const char*)data, length);
}

int ArduinoIoTCloudClass::writeStdout(const byte data[], int const length)
{
  return _mqttClient.publish(_stdoutTopic.c_str(), (const char*)data, length);
}

void ArduinoIoTCloudClass::onMessage(MQTTClient* /*client*/, char topic[], char bytes[], int const length)
{
  ArduinoCloud.handleMessage(topic, bytes, length);
}

void ArduinoIoTCloudClass::handleMessage(char topic[], char bytes[], int const length)
{
  if (_stdinTopic == topic) {
    CloudSerial.appendStdin((uint8_t*)bytes, length);
  }
  if (_dataTopicIn == topic) {
    Thing.decode((uint8_t*)bytes, length);
  }
}

ArduinoIoTCloudClass ArduinoCloud;
