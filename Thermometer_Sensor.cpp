#include "Thermometer_Sensor.hpp"
#include <iostream>


Thermometer_Sensor::Thermometer_Sensor(int pin, int n_smooth) : pin(pin), n_smooth(n_smooth)
{
	ds = OneWire(pin);

	int retry = 10;
	while (!ds.search(addr) && retry > 0)
	{
		Serial.printf("No addresses found for thermometer on pin %d\n", pin);
		retry--;
		delay(500);
	}
	if (retry <= 0)
	{
		// todo: raise exception
	}

	char board_uid[32];
	sprintf(board_uid, "%02X%02X%02X%02X%02X%02X%02X%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
	name = std::string(board_uid);


	float initial_temperature = measure_temperature();
	for (int i=0; i<n_smooth; i++)
	{
		temperature_history.push_back(initial_temperature);
	}
}

int Thermometer_Sensor::get_pin()
{
	return pin;
}

std::string Thermometer_Sensor::get_name()
{
	return name;
}

float Thermometer_Sensor::measure_temperature()
{
	byte i;
	byte present;
	byte data[12];

	float celsius;

	if (OneWire::crc8(addr, 7) != addr[7])
	{
		Serial.println("CRC is not valid!");
		return 0;
	}

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1);        // start conversion, with parasite power on at the end

	delay(1000);     // maybe 750ms is enough, maybe not
					 // we might do a ds.depower() here, but the reset will take care of it.

	present = ds.reset();
	ds.select(addr);
	ds.write(0xBE);         // Read Scratchpad

	for ( i = 0; i < 9; i++)
	{           // we need 9 bytes
		data[i] = ds.read();
	}
	OneWire::crc8(data, 8);


	// Convert the data to actual temperature
	// because the result is a 16 bit signed integer, it should
	// be stored to an "int16_t" type, which is always 16 bits
	// even when compiled on a 32 bit processor.
	int16_t raw = (data[1] << 8) | data[0];
	byte cfg = (data[4] & 0x60);
	// at lower res, the low bits are undefined, so let's zero them
	if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
	else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
	else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
										  //// default is 12 bit resolution, 750 ms conversion time
	celsius = (float)raw / 16.0;
	return celsius;
}


float Thermometer_Sensor::measure_temperature_smoothed()
{
	float new_temperature = measure_temperature();
	temperature_history.push_back(new_temperature);
	while (temperature_history.size() > n_smooth)
	{
		temperature_history.pop_front();
	}

	float mean = std::accumulate(temperature_history.begin(), temperature_history.end(), 0.0) / temperature_history.size();
	return mean;
}
