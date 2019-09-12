//============================================================================================
//
//		AQUARIUM CONTROLLER
//		version 		@ SK_VERSION
//		last rev.date 	@ SK_DATE
//
//		Original project by riciweb (http://forum.arduino.cc/index.php?topic=141419.0)
//		modded by silversat (09/2013)
//
//		This controller has been developed onto an Arduino MEGA2560 board. 
//		The hardware includes:
//		- 8 relais parallel board (hobbycomponents.com)
//		- I2C 20x4 LCD display
//		- DS3231 RTC/ram
//		- one or two DS18b20	temperature sensor
//		- IR receiver (TK19) associated with an apple tv remote control
//		- a buzzer
//
//		Most of the hardware is configurable (configuration.h)
//
//============================================================================================
#define SK_VERSION				"3.5.0"
#define SK_FEATURE				"mega"
#define SK_DATE					"12-09-2019"
#define SK_AUTHOR				"c.benedetti"

#define DEBUG(...) Serial.printf( __VA_ARGS__ )		// activate debug
#ifndef DEBUG 
	#define DEBUG(...)
#else
	#define DEBUG_MESSAGES
#endif

#if defined(__SAM3X8E__)
	#define WATCDOG_RESET(...) 		watchdogReset( __VA_ARGS__ )
#else
	#include <avr/wdt.h>
	#define WATCDOG_RESET(...)		wdt_reset( __VA_ARGS__ )
#endif
#include <Wire.h>
#include <OneWire.h>
#include "RTClib.h"
#include <DallasTemperature.h>

#define BUFF_SIZE	24

uint8_t 	kp_new;
uint8_t		dstatus;					// dispatcher status container
bool	 	conferma;
char 		buff[BUFF_SIZE];
char		confirm[] = "* CONFIRM *";

#include "configuration.h"				// load board configuration (pins, etc)
#include "functions.h"
#include "keyboard.h"
#include "buzzer.h"
#include "alarm.h"
#if defined(OLED_32) || defined(OLED_64) || defined(OLED_128)
	#include "oled_i2c.h"
#else
	#include "lcd_i2c.h"
#endif
#include "datetime.h"
#include "nvram.h"
#include "temperature.h"
#include "menu.h"
#include "lights.h"
#if defined(RELE_PARALLEL)				// see defines in config.h
	#include "scheda_rele.h"
#else
	#include "scheda_rele_i2c.h"
#endif

void FunzionamentoNormale() {
	if(kp_new == IR_MENU) {
		dstatus = DS_SETUP;			// switch dispatcher to enter menu
		SetInitBit();
	} else {	
		if(CheckInitBit(true)) {						// check and clear init bit.
			displayClear();								// clear display
			printString("AQUARIUM CONTROLLER", 0, 0);	// Scrivo sul display il titolo della schermata 
		}

		printDateTime(datetime);						// Stampo data e ora

		printString("T: ", 0, 2);
		printString("PH:0,0", 12, 2);

		sprintf(buff, "LMed: %d%%", calcLuxAverage());
		printString(buff, 0,3);
		
		sprintf(buff, "Cond:%03d", 0);
		printString(buff, 12,3);
	}
}

void loop()	{	
	WATCDOG_RESET();	// security watchdog reset
	//-------------------------------------------------------
	//	These tasks have to be executed each loop cycle
	//-------------------------------------------------------
	datetime = rtc.now();
	GestioneLuci();
	GetTemperature();
	MantenimentoTempAcqua();
	AlarmSireneHandle();
	//-------------------------------------------------------

	kp_new = ReadKeyboard();		// read keyboard or IR remote

	switch(dstatus & DS_INIT_MASK) {
		case DS_IDLE:
			FunzionamentoNormale();
			break;
			
		case DS_SETUP:
			ScorriMenu(MainMenu, sizeof(MainMenu));
			break;
							
		case DS_SETUP_DATETIME:
			datetime = ImpostaDataOra(); 
			break;

		case DS_SETUP_TEMP:
			ImpostaTempAcqua(); 
			break;

		case DS_SETUP_INFOLIGHTS:
			InfoLuci();
			break;
		
		case DS_SETUP_LIGHTS:
			ScorriMenu(LightsMenu, sizeof(LightsMenu));
			break;
		
		case DS_SETUP_LIGHTS_0:
			ImpostaFunzLinee();
			break;

		case DS_SETUP_LIGHTS_1:
			ImpDatiFotoperiodo(LIGHT_LINE_1);
			break;

		case DS_SETUP_LIGHTS_2:
			ImpDatiFotoperiodo(LIGHT_LINE_2);
			break;

		case DS_SETUP_LIGHTS_3:
			ImpDatiFotoperiodo(LIGHT_LINE_3);
			break;

		case DS_SETUP_LIGHTS_4:
			ImpDatiFotoperiodo(LIGHT_LINE_4);
			break;

		case DS_SETUP_LIGHTS_5:
			ImpDatiFotoperiodo(LIGHT_LINE_5);
			break;

		case DS_SETUP_LIGHTS_6:
			ImpDatiFotoperiodo(LIGHT_LINE_6);
			break;
		
		default:
			FunzionamentoNormale();
	}
}

void setup() {
	Wire.begin();
	#ifdef DEBUG_MESSAGES
		Serial.begin(SERIAL_BAUD);	// serial port #1 used for debugging
		delay(100);					// wait serial port to stabilize
		Serial.printf("Aquarium Controller v.%s by %s (%s)\n", SK_VERSION, SK_AUTHOR, SK_DATE);
		Serial.print("Initializing system...\n");
	#endif
	
	DisplayInit();					// Initialize I2C display
	RtcInit();						// Initialize Real Time Clock
	NvRamInit();					// Initialize Non Volatile RAM
	AlarmInit();					// Initialize automatic alarm sirene (arduino pro-mini)
	BuzzerInit();					// Initialize Buzzer
	RelaisInit();					// Relais board init
	PwmLightsInit() ;				// PWM Lights init
	TempSensorsInit();				// Temperature sensors initialization
	KeyboardInit();					// Keyboard and IR receiver init
	
	dstatus = DS_IDLE_INIT;			// startup Dispatcher status
	conferma = true;
	
	for(byte line = 0; line < LIGHT_LINE_NUMBER; line++) {
		LoadLightStatus(line);
	}
	alrmsonoro = true;

	#if defined(__SAM3X8E__)
		watchdogSetup();			// if not called, wdt is disabled
		watchdogEnable(2000);		// watchdog time set to 2 secs
		DEBUG("SAM");
	#else
		wdt_enable(WDTO_2S);		// enable watchdog and set reset time to 2 secs.
		DEBUG("AVR");
	#endif
	DEBUG(" Watchdog enabled\nDone.\n");
}