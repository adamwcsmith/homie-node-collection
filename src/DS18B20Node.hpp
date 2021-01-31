/*
 * DS18B20Node.hpp
 * Homie Node for Dallas 18B20 sensors.
 *
 * Version: 1.0
 * Author: Lübbe Onken (http://github.com/luebbe)
 * Author: Marcus Klein (http://github.com/kleini)
 */

#pragma once

#include <OneWire.h>
#include <DallasTemperature.h>

#include "SensorNode.hpp"
#include "constants.hpp"

#define DEFAULTPIN -1

#ifndef DS18B20_USECELSIUS   // #define in calling code to override (false means use Farenheit)
#define DS18B20_USECELSIUS true
#endif

#ifndef DS18B20_MULTISENSOR  // #define in calling code to override (true means look for more than one sensor)
#define DS18B20_MULTISENSOR false
#endif

class DS18B20Node : public SensorNode
{
private:
  const float cMinTempC = -55.0;
  const float cMaxTempC = 125.0;
  const float cMinTempF = -67.0;
  const float cMaxTempF = 257.0;
  const char *cFormatC = "-55.0:125.0";
  const char *cFormatF = "-67.0:257.0";
  const char *cCaption = "• %s DS18B20 pin[%d]:";
  const bool _cUseCelsius = DS18B20_USECELSIUS;
  const bool _cMultiSensor = DS18B20_MULTISENSOR;

  int _sensorPin = DEFAULTPIN;
  bool _sensorFound = false;
  int _numSensors = 0;
  unsigned long _measurementInterval;
  unsigned long _lastMeasurement;

  float *temperatures;

  OneWire *oneWire;
  DallasTemperature *dallasTemp;

  void send();
  void sendError(int i = 0);
  void sendData(int i = 0);

protected:
  virtual void setup() override;
  virtual void loop() override;
  virtual void onReadyToOperate() override;

public:
  explicit DS18B20Node(const char *id,
                       const char *name,
                       const int sensorPin = DEFAULTPIN,
                       const int measurementInterval = MEASUREMENT_INTERVAL);

  float getTemperature(int i = 0) const { return temperatures[i]; }
};
