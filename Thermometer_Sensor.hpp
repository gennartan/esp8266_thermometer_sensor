#ifndef __THERMOMETER_SENSOR_HPP__
#define __THERMOMETER_SENSOR_HPP__

#include <Arduino.h>
#include <OneWire.h>
#include <iostream>
#include <deque>
#include <numeric>      // std::accumulate

class Thermometer_Sensor {
	public:
		Thermometer_Sensor(int pin, int n_smooth=5);
		int get_pin();
		std::string get_name();
		float measure_temperature();
		float measure_temperature_smoothed();
	private:
		int pin;
		OneWire ds;
		std::string name;
		byte addr[8];

		int n_smooth;
		std::deque<float> temperature_history;
};

#endif
