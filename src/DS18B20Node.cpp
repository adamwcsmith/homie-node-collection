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
}

void DS18B20Node::send()
{
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
  if (Homie.isConnected())
  {
    setProperty(cStatusTopic + (_sensorFound ? String(i) : String(""))).send("error");
  }
}

void DS18B20Node::sendData(int i)
{
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
      dallasTemp->requestTemperatures();
      for(int i = 0; i < _numSensors; ++i) {
        temperatures[i] = dallasTemp->getTempCByIndex(i);
        fixRange(&temperatures[i], cMinTemp, cMaxTemp);
      }
      send();

      _lastMeasurement = millis();
    }
  }
}

void DS18B20Node::onReadyToOperate()
{
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
    char numStr[5] = "0"; // don't connect more than 9999 sensors
    if(!_sensorFound) {
      Homie.getLogger() << F("No senseors found! Advertising only ") << cStatusTopic << endl;
      advertise(cStatusTopic)
          .setDatatype("enum")
          .setFormat("error, ok"); 
    } else {
      for(int i = 0; i < _numSensors; ++i) {
        itoa(i, numStr, 10);
        Homie.getLogger() << F("advertising ") << cStatusTopic << numStr << endl;
        advertise((String(cStatusTopic) + String(numStr)).c_str() )
          .setDatatype("enum")
          .setFormat("error, ok");      

        Homie.getLogger() << F("advertising ") << cTemperatureTopic << numStr << endl;
        advertise((String(cTemperatureTopic) + String(numStr)).c_str() )
          .setDatatype("float")
          .setFormat("-55:125")
          .setUnit(cUnitDegrees);
      }
    }
  }
}
