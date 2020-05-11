/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

#ifndef ARDUINO_IOT_CLOUD_LITE_H
#define ARDUINO_IOT_CLOUD_LITE_H

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <ArduinoIoTCloud.h>



#include <WiFiNINA.h>

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static char const DEFAULT_BROKER_ADDRESS_SECURE_AUTH[] = "mqtts-sa.iot.arduino.cc";
static uint16_t const DEFAULT_BROKER_PORT_SECURE_AUTH = 8883;
static char const DEFAULT_BROKER_ADDRESS_USER_PASS_AUTH[] = "mqtts-up.iot.arduino.cc";
static uint16_t const DEFAULT_BROKER_PORT_USER_PASS_AUTH = 8884;

enum class NetworkConnectionState : unsigned int {
  INIT          = 0,
  CONNECTING    = 1,
  CONNECTED     = 2,
  DISCONNECTING = 3,
  DISCONNECTED  = 4,
  CLOSED        = 5,
  ERROR         = 6
};
/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudLite: public ArduinoIoTCloudClass
{
  public:

             ArduinoIoTCloudLite();
    virtual ~ArduinoIoTCloudLite();


    virtual void update        () override;
    virtual int  connected     () override;
    virtual void printDebugInfo() override;


    int begin(char const *  SSID, char const *  PASS, String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);


    inline String   getBrokerAddress() const { return _brokerAddress; }
    inline uint16_t getBrokerPort   () const { return _brokerPort; }

    // Clean up existing Mqtt connection, create a new one and initialize it
    int reconnect();

    friend class CloudSerialClass;


  protected:

    virtual int  connect       () override;
    virtual void disconnect    () override;
    NetworkConnectionState checkPhyConnection() override;

  private:
    static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;

    int _lastSyncRequestTickTime;
    String _brokerAddress;
    uint16_t _brokerPort;
    uint8_t _mqtt_data_buf[MQTT_TRANSMIT_BUFFER_SIZE];
    int _mqtt_data_len;
    bool _mqtt_data_request_retransmit;



    ArduinoIoTSynchronizationStatus _syncStatus;

    // Class attribute to define MTTQ topics 2 for stdIn/out and 2 for data, in order to avoid getting previous pupblished payload
    String _stdinTopic;
    String _stdoutTopic;
    String _shadowTopicOut;
    String _shadowTopicIn;
    String _dataTopicOut;
    String _dataTopicIn;

    inline String getTopic_stdin    () { return String("/a/d/" + getDeviceId() + "/s/i"); }
    inline String getTopic_stdout   () { return String("/a/d/" + getDeviceId() + "/s/o"); }
    inline String getTopic_shadowout() { return ( getThingId().length() == 0) ? String("")                            : String("/a/t/" + getThingId() + "/shadow/o"); }
    inline String getTopic_shadowin () { return ( getThingId().length() == 0) ? String("")                            : String("/a/t/" + getThingId() + "/shadow/i"); }
    inline String getTopic_dataout  () { return ( getThingId().length() == 0) ? String("/a/d/" + getDeviceId() + "/e/o") : String("/a/t/" + getThingId() + "/e/o"); }
    inline String getTopic_datain   () { return ( getThingId().length() == 0) ? String("/a/d/" + getDeviceId() + "/e/i") : String("/a/t/" + getThingId() + "/e/i"); }

    static void onMessage(int length);
    void handleMessage(int length);
    void sendPropertiesToCloud();
    void requestLastValue();
    ArduinoIoTConnectionStatus checkCloudConnection();
    int write(String const topic, byte const data[], int const length);
};

/******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/

extern ArduinoIoTCloudLite ArduinoCloud;

#endif