#include <Arduino.h>
/*
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2019 Sensnology AB
 * Full contributor list: https://github.com/mysensors/MySensors/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * DESCRIPTION - Button
 *
 * Interrupt driven binary switch example with dual interrupts
 * Author: Patrick 'Anticimex' Fallberg
 * Connect one button or door/window reed switch between
 * digital I/O pin 3 (BUTTON_PIN below) and GND and the other
 * one in similar fashion on digital I/O pin 2.
 * This example is designed to fit Arduino Nano/Pro Mini
 *
 *  *******************************
 *
 * DESCRIPTION - Battery level
 *
 * This is an example that demonstrates how to report the battery level for a sensor
 * Instructions for measuring battery capacity on A0 are available here:
 * http://www.mysensors.org/build/battery
 *
 */

// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#include <MySensors.h>
#include "main.h"
void processBatteryInfo(void);
void processButton(void);
// Change to V_LIGHT if you use S_LIGHT in presentation below
MyMessage msg(PRIMARY_CHILD_ID, V_TRIPPED);
MyMessage msg2(SECONDARY_CHILD_ID, V_TRIPPED);

int BATTERY_SENSE_PIN = A0;  // select the input pin for the battery sense point

uint32_t SLEEP_TIME = 900000;  // sleep time between reads (seconds * 1000 milliseconds)
int oldBatteryPcnt = 0;

void setup()
{
	// Setup the buttons
	analogReference(INTERNAL);					// internal reference for battery voltage measurement
	pinMode(PRIMARY_BUTTON_PIN, INPUT_PULLUP);
	pinMode(SECONDARY_BUTTON_PIN, INPUT_PULLUP);
}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Battery Meter", "1.0");
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);

	// Register binary input sensor to sensor_node (they will be created as child devices)
	// You can use S_DOOR, S_MOTION or S_LIGHT here depending on your usage.
	// If S_LIGHT is used, remember to update variable type you send in. See "msg" above.
	present(PRIMARY_CHILD_ID, S_DOOR);
	present(SECONDARY_CHILD_ID, S_DOOR);
}

// Loop will iterate on changes on the BUTTON_PINs
void loop()
{
	sendHeartbeat();
	processBatteryInfo();
	processButton();

	// Sleep until something happens with the sensor
	smartSleep(PRIMARY_BUTTON_PIN-2, FALLING, SECONDARY_BUTTON_PIN-2, FALLING, SLEEP_TIME);
}

void processBatteryInfo(void)
{
		// get the battery Voltage
	int sensorValue = analogRead(BATTERY_SENSE_PIN);
	// 1M, 470K divider across battery and using internal ADC ref of 1.1V
	// Sense point is bypassed with 0.1 uF cap to reduce noise at that point
	// ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
	// 3.44/1023 = Volts per bit = 0.003363075

	int batteryPcnt = sensorValue / 10;

#ifdef MY_DEBUG
	Serial.println(sensorValue);
	Serial.print("Battery percent: ");
	Serial.print(batteryPcnt);
	Serial.println(" %");
#endif

	if (oldBatteryPcnt != batteryPcnt) {
		// Power up radio after sleep
		sendBatteryLevel(batteryPcnt);
		oldBatteryPcnt = batteryPcnt;
	}
}

void processButton(void)
{
	uint8_t value;
	static uint8_t swValue1=LOW;
	static uint8_t swValue2=LOW;

	// Short delay to allow buttons to properly settle
	wait(5);

	value = digitalRead(PRIMARY_BUTTON_PIN);

	if (value == LOW) {
		// Value has changed from last transmission, send the updated value
		swValue1 = (swValue1 == LOW) ? HIGH : LOW;		// Toggle switch state
		send(msg.set(swValue1));
	}

	value = digitalRead(SECONDARY_BUTTON_PIN);

	if (value == LOW) {
		// Value has changed from last transmission, send the updated value
		swValue2 = (swValue2 == LOW) ? HIGH : LOW;		// Toggle switch state
		send(msg2.set(swValue2));
	}
}