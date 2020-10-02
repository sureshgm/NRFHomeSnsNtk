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
 * REVISION HISTORY
 * Version 1.0 - Henrik Ekblad
 * Contribution by a-lurker and Anticimex,
 * Contribution by Norbert Truchsess <norbert.truchsess@t-online.de>
 * Contribution by Ivo Pullens (ESP8266 support)
 *
 * DESCRIPTION
 * The EthernetGateway sends data received from sensors to the WiFi link.
 * The gateway also accepts input on ethernet interface, which is then sent out to the radio network.
 *
 * VERA CONFIGURATION:
 * Enter "ip-number:port" in the ip-field of the Arduino GW device. This will temporarily override any serial configuration for the Vera plugin.
 * E.g. If you want to use the default values in this sketch enter: 192.168.178.66:5003
 *
 * LED purposes:
 * - To use the feature, uncomment any of the MY_DEFAULT_xx_LED_PINs in your sketch, only the LEDs that is defined is used.
 * - RX (green) - blink fast on radio message received. In inclusion mode will blink fast only on presentation received
 * - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
 * - ERR (red) - fast blink on error during transmission error or receive crc error
 *
 * See https://www.mysensors.org/build/connect_radio for wiring instructions.
 *
 * If you are using a "barebone" ESP8266, see
 * https://www.mysensors.org/build/esp8266_gateway#wiring-for-barebone-esp8266
 *
 * Inclusion mode button:
 * - Connect GPIO5 (=D1) via switch to GND ('inclusion switch')
 *
 * Hardware SHA204 signing is currently not supported!
 *
 * Make sure to fill in your ssid and WiFi password below for ssid & pass.
 */
#include <Arduino.h>
// Enable debug prints to serial monitor
#define MY_DEBUG

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 115200

// Enables and select radio type (if attached)
#define MY_RADIO_RF24
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#define MY_GATEWAY_ESP8266

#define MY_WIFI_SSID "Sure_2G4"
#define MY_WIFI_PASSWORD "Gms@1109"

// Enable UDP communication
//#define MY_USE_UDP  // If using UDP you need to set MY_CONTROLLER_IP_ADDRESS or MY_CONTROLLER_URL_ADDRESS below

// Set the hostname for the WiFi Client. This is the hostname
// it will pass to the DHCP server if not static.
#define MY_HOSTNAME "ESP8266_GGW27GW"

// Enable MY_IP_ADDRESS here if you want a static ip address (no DHCP)
//#define MY_IP_ADDRESS 192,168,178,87

// If using static ip you can define Gateway and Subnet address as well
//#define MY_IP_GATEWAY_ADDRESS 192,168,178,1
//#define MY_IP_SUBNET_ADDRESS 255,255,255,0

// The port to keep open on node server mode
#define MY_PORT 5003

// How many clients should be able to connect to this gateway (default 1)
#define MY_GATEWAY_MAX_CLIENTS 2

// Controller ip address. Enables client mode (default is "server" mode).
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere.
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 68
//#define MY_CONTROLLER_URL_ADDRESS "my.controller.org"

// Enable inclusion mode
//#define MY_INCLUSION_MODE_FEATURE

// Enable Inclusion mode button on gateway
#define MY_INCLUSION_BUTTON_FEATURE
// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 60
// Digital pin used for inclusion mode button
#define MY_INCLUSION_MODE_BUTTON_PIN D1

// Set blinking period
//#define MY_DEFAULT_LED_BLINK_PERIOD 300

// Flash leds on rx/tx/err
// Led pins used if blinking feature is enabled above
//#define MY_DEFAULT_ERR_LED_PIN 16  // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  16  // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  16  // the PCB, on board LED

#include <MySensors.h>
#include "DimmableLight.h"

/************************************************************
 * This is code for dimmable light
 * 
 * 
 * 
 * 
 * 
 * 
 * ********************************************************/

int16_t LastLightState=LIGHT_OFF;
int16_t LastDimValue=100;
const int LED_Pin = D0;
MyMessage lightMsg(CHILD_ID_LIGHT, V_LIGHT);
MyMessage dimmerMsg(CHILD_ID_LIGHT, V_DIMMER);

void Init_Node(void)
{
    	//Retreive our last light state from the eprom
	int LightState = loadState(EPROM_LIGHT_STATE);
	if (LightState<=1) {
		LastLightState=LightState;
		int DimValue=loadState(EPROM_DIMMER_LEVEL);
		if ((DimValue>0)&&(DimValue<=100)) {
			//There should be no Dim value of 0, this would mean LIGHT_OFF
			LastDimValue=DimValue;
		}
	}

	//Here you actually switch on/off the light with the last known dim level
	SetCurrentState2Hardware();
	Serial.println( "Node ready to receive messages..." );
    pinMode(LED_Pin, OUTPUT);
}

void Present_DimmableLight(void)
{
    // Send the Sketch Version Information to the Gateway
	sendSketchInfo(SN, SV);

	present(CHILD_ID_LIGHT, S_DIMMER );
}

void receive(const MyMessage &message)
{
	if (message.getType() == V_LIGHT) {
		Serial.println( "V_LIGHT command received..." );

		int lstate= atoi( message.data );
		if ((lstate<0)||(lstate>1)) {
			Serial.println( "V_LIGHT data invalid (should be 0/1)" );
			return;
		}
		LastLightState=lstate;
		saveState(EPROM_LIGHT_STATE, LastLightState);

		if ((LastLightState==LIGHT_ON)&&(LastDimValue==0)) {
			//In the case that the Light State = On, but the dimmer value is zero,
			//then something (probably the controller) did something wrong,
			//for the Dim value to 100%
			LastDimValue=100;
			saveState(EPROM_DIMMER_LEVEL, LastDimValue);
		}

		//When receiving a V_LIGHT command we switch the light between OFF and the last received dimmer value
		//This means if you previously set the lights dimmer value to 50%, and turn the light ON
		//it will do so at 50%
	} else if (message.getType() == V_DIMMER) {
		Serial.println( "V_DIMMER command received..." );
		int dimvalue= atoi( message.data );
		if ((dimvalue<0)||(dimvalue>100)) {
			Serial.println( "V_DIMMER data invalid (should be 0..100)" );
			return;
		}
		if (dimvalue==0) {
			LastLightState=LIGHT_OFF;
		} else {
			LastLightState=LIGHT_ON;
			LastDimValue=dimvalue;
			saveState(EPROM_DIMMER_LEVEL, LastDimValue);
		}
	} else {
		Serial.println( "Invalid command received..." );
		return;
	}

	//Here you set the actual light state/level
	SetCurrentState2Hardware();
}

void SetCurrentState2Hardware()
{
	if (LastLightState==LIGHT_OFF) {
		Serial.println( "Light state: OFF" );
        digitalWrite(LED_Pin, LOW);
	} else {
		Serial.print( "Light state: ON, Level: " );
		Serial.println( LastDimValue );
        digitalWrite(LED_Pin, HIGH);
	}

	//Send current state to the controller
	SendCurrentState2Controller();
}

void SendCurrentState2Controller()
{
	if ((LastLightState==LIGHT_OFF)||(LastDimValue==0)) {
		send(dimmerMsg.set((int16_t)0));
	} else {
		send(dimmerMsg.set(LastDimValue));
	}
}

/**********************************************************
 * 
 * */


void setup()
{
	// Setup locally attached sensors
	Init_Node();
}

void presentation()
{
	// Present locally attached sensors here
	Present_DimmableLight();
}

void loop()
{
	// Send locally attached sensors data here
}
