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

/*  Defer advertisements until setup function when we find out how many DS18B20 sensors are present
  advertise(cStatusTopic)
      .setDatatype("enum")
      .setFormat("error, ok");
  advertise(cTemperatureTopic)
      .setDatatype("float")
      .setFormat("-55:125")
      .setUnit(cUnitDegrees);
*/
}

void DS18B20Node::send()
{
  printCaption();

  for (int i = 0; i < _sensorFound; ++i) {
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
    setProperty(cStatusTopic + _sensorFound ? String(i) : "").send("error");
  }
}

void DS18B20Node::sendData(int i)
{
  if (Homie.isConnected())
  {
    setProperty(cStatusTopic).send("ok");
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
      for(int i = 0; i < _sensorFound; ++i) {
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
    Homie.getLogger() << cIndent << F("Found ") << dallasTemp->getDS18Count() << " sensors." << endl
                      << cIndent << F("Reading interval: ") << _measurementInterval << " s" << endl;
    for(int i = 0; i < _sensorFound; ++i) {
      advertise(cStatusTopic + i)
          .setDatatype("enum")
          .setFormat("error, ok");      
      advertise(cTemperatureTopic + i)
          .setDatatype("float")
          .setFormat("-55:125")
          .setUnit(cUnitDegrees);
    }
    if(!_sensorFound) {
      advertise(cStatusTopic)
          .setDatatype("enum")
          .setFormat("error, ok"); 
    }
  }
}
