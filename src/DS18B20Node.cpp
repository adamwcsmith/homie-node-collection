/*
 * DS18B20Node.cpp
 * Homie Node for Dallas 18B20 sensors.
 *
 * Version: 1.0
 * Author: Lübbe Onken (http://github.com/luebbe)
 * Author: Marcus Klein (http://github.com/kleini)
 */

#include "DS18B20Node.hpp"

DS18B20Node::DS18B20Node(const char *id, const char *name, const int sensorPin, 
                          const int measurementInterval)
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
    if (temperatures[i] == (_cUseCelsius ? DEVICE_DISCONNECTED_C : DEVICE_DISCONNECTED_F))
    {
      Homie.getLogger() << cIndent << F("Error reading from Sensor ") << i << endl;
      sendError(i);
    }
    else
    {
      Homie.getLogger() << cIndent << F("Temperature: ") << temperatures[i] << F(" ")
                                   << (_cUseCelsius ? cUnitDegreesC : cUnitDegreesF) << " from sensor " << i << endl;
      sendData(i);
    }
  }
}

void DS18B20Node::sendError(int i)
{  
  if (Homie.isConnected())
  {
    setProperty(cStatusTopic + (_cMultiSensor ? String(i) : String(""))).send("error");
  }
}

void DS18B20Node::sendData(int i)
{
  if (Homie.isConnected())
  {
    setProperty(cStatusTopic + (_cMultiSensor ? String(i) : String(""))).send("ok");
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
        temperatures[i] = (_cUseCelsius ? dallasTemp->getTempCByIndex(i) : dallasTemp->getTempFByIndex(i));
        fixRange(&temperatures[i], (_cUseCelsius ? cMinTempC : cMinTempF), (_cUseCelsius ? cMaxTempC : cMaxTempF));
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
    if(_numSensors > 0) {
      _sensorFound = true;
      if(!_cMultiSensor) { _numSensors = 1; }  // limit to one sensor unless DS18B20_MULTISENSOR has been defined
    };
    if(!_sensorFound) {
      Homie.getLogger() << F("No sensors found! Advertising just status topic ") << cStatusTopic << endl;
      advertise(cStatusTopic)
          .setDatatype("enum")
          .setFormat("error, ok"); 
     } else {  // _sensorFound == true
      Homie.getLogger() << cIndent << F("Found ") << _numSensors << F(" sensors.") 
                        << ( _cMultiSensor ? F("") : F(" (Use #define DS18B20_MULTISENSOR to allow multiple sensors)") ) << endl
                        << cIndent << F("Reading interval: ") << _measurementInterval << F(" s") << endl;
      temperatures = new float[_numSensors];
      char numStr[5] = ""; // don't connect more than 9999 sensors.  Note, null string used below when not in multisensor mode.
      for(int i = 0; i < _numSensors; ++i) {
        if(_cMultiSensor) { itoa(i, numStr, 10); }   // if not supporting multiple sensors, leave as null for bare status topic
        Homie.getLogger() << F("advertising ") << cStatusTopic << numStr << endl;
        advertise( strdup((String(cStatusTopic) + String(numStr)).c_str()) )
          .setDatatype("enum")
          .setFormat("error, ok");      
        Homie.getLogger() << F("advertising ") << cTemperatureTopic << numStr << F(" with unit ") << (_cUseCelsius ? cUnitDegreesC : cUnitDegreesF)
                          << F(" and format ") << (_cUseCelsius ? cFormatC : cFormatF) <<  endl;
        advertise( strdup((String(cTemperatureTopic) + String(numStr)).c_str()) )
          .setDatatype("float")
          .setFormat( (_cUseCelsius ? cFormatC : cFormatF) )
          .setUnit(_cUseCelsius ? cUnitDegreesC : cUnitDegreesF);
      }
    }
  }
}
