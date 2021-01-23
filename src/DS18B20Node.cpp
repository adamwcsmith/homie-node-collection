/*
 * DS18B20Node.cpp
 * Homie Node for Dallas 18B20 sensors.
 *
 * Version: 1.0
 * Author: Lübbe Onken (http://github.com/luebbe)
 * Author: Marcus Klein (http://github.com/kleini)
 */

#include "DS18B20Node.hpp"

DS18B20Node::DS18B20Node(const char *id, const char *name, const int sensorPin, const int measurementInterval)
    : SensorNode(id, name, "DS18B20"),
      _sensorPin(sensorPin),
      _measurementInterval(measurementInterval),
      _lastMeasurement(0)
{
  if (_sensorPin > DEFAULTPIN)
  {
    oneWire = new OneWire(_sensorPin);
    dallasTemp = new DallasTemperature(oneWire);
  }

  asprintf(&_caption, cCaption, name, sensorPin);
  Homie.getLogger() << F("DS18B20Node::DS18B20Node() completed") << endl;
}

void DS18B20Node::send()
{
  Homie.getLogger() << F("DS18B20Node::send()") << endl;
  printCaption();

  for (int i = 0; i < _numSensors; ++i) {
    if (DEVICE_DISCONNECTED_C == temperatures[i])
    {
      Homie.getLogger() << cIndent << F("Error reading from Sensor ") << i << endl;
      sendError(i);
    }
    else
    {
      Homie.getLogger() << cIndent << F("Temperature: ") << temperatures[i] << " °C from sensor " << i << endl;
      sendData(i);
    }
  }
}

void DS18B20Node::sendError(int i)
{  
  Homie.getLogger() << F("DS18B20Node::sendError()") << i << endl;
  if (Homie.isConnected())
  {
    setProperty(cStatusTopic + (_sensorFound ? String(i) : String(""))).send("error");
  }
}

void DS18B20Node::sendData(int i)
{
  Homie.getLogger() << F("DS18B20Node::sendData()") << i << endl;
  if (Homie.isConnected())
  {
    setProperty(cStatusTopic + (_sensorFound ? String(i) : String(""))).send("ok");
    setProperty(cTemperatureTopic + String(i)).send(String(temperatures[i]));
  }
}

void DS18B20Node::loop()
{
  if (_sensorFound)
  {
    if ((millis() - _lastMeasurement >= _measurementInterval * 1000UL) || (_lastMeasurement == 0))
    {
      Homie.getLogger() << F("DS18B20Node::loop() ready to check temps") << endl;
      dallasTemp->requestTemperatures();
      Homie.getLogger() << F("DS18B20Node::loop() requested: will read") << endl;
      for(int i = 0; i < _numSensors; ++i) {
        Homie.getLogger() << F("DS18B20Node::loop() sub-loop: ") << i << endl;
        temperatures[i] = dallasTemp->getTempCByIndex(i);
        fixRange(&temperatures[i], cMinTemp, cMaxTemp);
      }
      Homie.getLogger() << F("DS18B20Node::loop() about to send") << endl;
      send();

      _lastMeasurement = millis();
    }
  }
}

void DS18B20Node::onReadyToOperate()
{
  Homie.getLogger() << F("DS18B20Node::onReadyToOperate()") << endl;
  if (!_sensorFound)
  {
    sendError();
  }
};

void DS18B20Node::setup()
{
  printCaption();

  if (dallasTemp)
  {
    dallasTemp->begin();
    _numSensors = dallasTemp->getDS18Count();
    _sensorFound = (_numSensors > 0);
    temperatures = new float[_numSensors];
    Homie.getLogger() << cIndent << F("Found ") << _numSensors << " sensors." << endl
                      << cIndent << F("Reading interval: ") << _measurementInterval << " s" << endl;
//    int statusTopicLen = strlen(cStatusTopic);
//    char statusStr[statusTopicLen + 5];
//    statusStr[0] = '\0';
//    strncat(statusStr, cStatusTopic, statusTopicLen);
//    int temperatureTopicLen = strlen(cTemperatureTopic);
//    char temperatureStr[temperatureTopicLen + 5];
//    temperatureStr[0] = '\0';
//    strncat(temperatureStr, cTemperatureTopic, temperatureTopicLen);
    char numStr[5] = "0"; // don't connect more than 9999 sensors
    if(!_sensorFound) {
      Homie.getLogger() << F("No senseors found!  advertising ") << cStatusTopic << endl;
      advertise(cStatusTopic)
          .setDatatype("enum")
          .setFormat("error, ok"); 
    } else {
      for(int i = 0; i < _numSensors; ++i) {
        itoa(i, numStr, 10);
//      statusStr[statusTopicLen] = '\0';
//      strncat(statusStr, numStr, 4);
//      temperatureStr[temperatureTopicLen] = '\0';
//      strncat(temperatureStr, numStr, 4);
//      Homie.getLogger() << F("advertising ") << statusStr << endl;
//      advertise(statusStr)
        Homie.getLogger() << F("advertising ") << cStatusTopic << numStr << endl;
        advertise((String(cStatusTopic) + String(numStr)).c_str() )
          .setDatatype("enum")
          .setFormat("error, ok");      
//      Homie.getLogger() << F("advertising ") << temperatureStr << endl;
//      advertise(temperatureStr)
        Homie.getLogger() << F("advertising ") << cTemperatureTopic << numStr << endl;
        advertise((String(cTemperatureTopic) + String(numStr)).c_str() )
          .setDatatype("float")
          .setFormat("-55:125")
          .setUnit(cUnitDegrees);
      }
    }
  }
  Homie.getLogger() << F("setup complete") << endl;
}
