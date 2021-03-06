/*
 * AdcNode.cpp
 * Homie Node using the internal ESP ADC to measure voltage.
 *
 * Version: 1.2
 * Author: Lübbe Onken (http://github.com/luebbe)
 */

#pragma once

#include "SensorNode.hpp"
#include "constants.hpp"

class AdcNode : public SensorNode
{
private:
  // Read ADC every 10 seconds to have the current value available
  // when another part of the application needs it
  static const int READ_INTERVAL_MILLISECONDS = 10 * 1000;
  // Send ADC every 5 minutes
  static const int SEND_INTERVAL_MILLISECONDS = 300 * 1000;

  const char *cCaption = "• %s ADC:";
  const float cVoltMax = 3.3; // = 100% battery
  const float cVoltMin = 2.6; // =   0% battery

  HomieSetting<double> *_adcCorrection;
  HomieSetting<double> *_adcBattMax;
  HomieSetting<double> *_adcBattMin;

  unsigned long _lastReadTime;
  unsigned long _lastSendTime;
  unsigned long _sendInterval;

  float _batteryLevel = NAN;
  float _voltage = NAN;

  void readVoltage();
  void send();
  void sendError();
  void sendData();

protected:
  virtual void setup() override;
  virtual void loop() override;
  virtual void onReadyToOperate() override;

public:
  explicit AdcNode(const char *id,
                   const char *name,
                   const int sendInterval = SEND_INTERVAL_MILLISECONDS);

  float getBatteryLevel() const { return _batteryLevel; }
  float getVoltage() const { return _voltage; }
  String getVoltageStr();

  void beforeHomieSetup();
};
