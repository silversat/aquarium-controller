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
//		- I2C 20x4 LCD display or i2c OLED 32x128, 64x128, 128x128
//		- DS3231 RTC/nvram
//		- one or two DS18b20 temperature sensor
//		- IR receiver (TK19) associated with an apple tv remote control
//		- a buzzer
//
//		Most of the hardware is configurable (configuration.h)
//
//============================================================================================
#define SK_VERSION				"3.6.1"
#define SK_DATE					"13-04-2020"
#define SK_FEATURE				"mega"
#define SK_AUTHOR				"c.benedetti"

#define DEBUG(...) Serial.printf( __VA_ARGS__ )		// activate debug
#ifndef DEBUG 
	#define DEBUG(...)
#else
	#define DEBUG_MESSAGES
#endif

#if defined(__SAM3X8E__)
	#define SK_FEATURE				"ARM"
	#define WATCDOG_RESET(...) 		watchdogReset( __VA_ARGS__ )
	#define PWM_RESOLUTION	1024.0
	#define ADC_RESOLUTION	4095.0
	#define ADC_RES_BITS	12
#else
	#define SK_FEATURE				"AVR"
	#include <avr/wdt.h>
	#define WATCDOG_RESET(...)		wdt_reset( __VA_ARGS__ )
	#define PWM_RESOLUTION	255
	#define ADC_RESOLUTION	1023.0
	#define ADC_RES_BITS	10
#endif

#include <Wire.h>
#include <OneWire.h>
#include "RTClib.h"
#include <DallasTemperature.h>

#define BUFF_SIZE		24
#define MAIN_PAGE_FIRST	0
#define MAIN_PAGE_LAST	3

uint8_t 	kp_new;
uint8_t		dstatus;					// dispatcher status container
int			main_page = 0;
char 		buff[BUFF_SIZE];
char		confirm_msg[] = "* CONFIRM *";

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
#include "menu.h"
#include "lights.h"
#if defined(RELE_PARALLEL)				// see defines in config.h
	#include "scheda_rele.h"
#else
	#include "scheda_rele_i2c.h"
#endif
#include "temperature.h"
#include "ph_ec_sensor.h"
#include "level.h"
#include "turbidity.h"

void NormalOperation() {
	if(kp_new == IR_MENU) {
		dstatus = DS_SETUP;								// switch dispatcher to enter menu
		SetInitBit();
	} else if(kp_new == IR_OK) {
		main_page = MAIN_PAGE_FIRST;
		SetInitBit();
	} else if(kp_new == IR_RIGHT) {
		main_page++;
		if(main_page > MAIN_PAGE_LAST) main_page = MAIN_PAGE_FIRST;
		SetInitBit();
	} else if(kp_new == IR_LEFT) {
		main_page--;
		if(main_page < MAIN_PAGE_FIRST) main_page = MAIN_PAGE_LAST;
		SetInitBit();
	} else {	
		if(CheckInitBit(true)) {							// check and clear init bit.
			displayClear();									// clear display
			if(main_page == MAIN_PAGE_FIRST) {
				printString("AQUARIUM CONTROLLER", 0, 0);	// Scrivo sul display il titolo della schermata 
				printString("T:", 0, 2);
				printString("PH:", 12, 2);
				printString("EC:", 12, 3);
			} else if(main_page == 1) {
				printStringCenter("Water status 1", 0);
				printString("    Level:", 2, 1);
				printString(" Temperat:", 2, 2);
				printString("Turbidity:", 2, 3);
			} else if(main_page == 2) {
				printStringCenter("Water status 2", 0);
				printString("PH:", 2, 1);
				printString("EC:", 2, 2);
				printString("   ", 2, 3);
			} else if(main_page == 3) {
				sprintf(buff, "System: %s", SK_FEATURE);
				printStringCenter(buff, 0);
				sprintf(buff, "Version: %s", SK_VERSION);
				printStringCenter(buff, 1);
				sprintf(buff, "Date: %s", SK_DATE);
				printStringCenter(buff, 2);
				sprintf(buff, "Author: %s", SK_AUTHOR);
				printStringCenter(buff, 3);
			}
		}
		if(main_page == 0) {
			printDateTime(datetime);						// Stampo data e ora
			sprintf(buff, "DLMed:%3d%%", calcLuxAverage());
			printString(buff, 0,3);
		} else if(main_page == 1) {
		}
	}
}

void loop()	{
	static float tmed = 0;
	
	WATCDOG_RESET();	// security watchdog reset
	//-------------------------------------------------------
	//	These tasks have to be executed each loop cycle
	//-------------------------------------------------------
	datetime = getDateTime();
	LightsHandler();
	tmed = WaterTemperatureHandler();
	WaterLevelHandler();
	WaterTurbidityHandler();
	Water_PH_Handler(tmed);
	Water_EC_Handler(tmed);
	AlarmSireneHandle();
	//-------------------------------------------------------

	kp_new = ReadKeyboard();		// read keyboard or IR remote

	switch(dstatus & DS_INIT_MASK) {
		case DS_IDLE:
			NormalOperation();
			break;
			
		case DS_SETUP:
			ScorriMenu(MainMenu, sizeof(MainMenu));
			break;
							
		case DS_SETUP_DATETIME:
			datetime = ImpostaDataOra(); 
			break;

		case DS_SETUP_CALIBRATION:
			ScorriMenu(CalMenu, sizeof(CalMenu));
			break;

		case DS_SETUP_CALIBRATION_PH:
			sensorsCalibration_PH(tmed);
			break;

		case DS_SETUP_CALIBRATION_EC:
			sensorsCalibration_EC(tmed);
			break;

		case DS_SETUP_TIMERUN:
			fastTimeSetup();
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
			NormalOperation();
	}
}

void setup() {
	Wire.begin();
	#ifdef DEBUG_MESSAGES
		Serial.begin(SERIAL_BAUD);	// serial port #1 used for debugging
		delay(100);					// wait serial port to stabilize
		Serial.printf(F("Aquarium Controller (%s) v.%s by %s (%s)\n"), SK_FEATURE, SK_VERSION, SK_AUTHOR, SK_DATE);
		Serial.print(F("Initializing system...\n"));
	#endif
	
	RtcInit();						// Initialize Real Time Clock
	NvRamInit();					// Initialize Non Volatile RAM
	DisplayInit();					// Initialize LCD/I2C display
	AlarmInit();					// Initialize automatic alarm sirene (arduino pro-mini)
	BuzzerInit();					// Initialize Buzzer
	RelaisInit();					// Relais board init
	PwmLightsInit() ;				// PWM Lights init
	TempSensorsInit();				// Temperature sensors initialization
	PH_SensorInit();				// PH sensor initialization
	EC_SensorInit();				// PH sensor initialization
	LevelSensorInit();				// Level sensor initialization
	TurbiditySensorInit();			// Turbidity sensor initialization
	KeyboardInit();					// Keyboard and IR receiver init
	
	dstatus = DS_IDLE_INIT;			// startup Dispatcher status
	alrmsonoro = true;
	main_page = MAIN_PAGE_FIRST;

	#if defined(__SAM3X8E__)
		analogReadResolution(ADC_RES_BITS);
		analogWriteResolution(ADC_RES_BITS);
		watchdogSetup();			// if not called, wdt is disabled
		watchdogEnable(2000);		// watchdog time set to 2 secs
	#else
		wdt_enable(WDTO_2S);		// enable watchdog and set reset time to 2 secs.
	#endif
	DEBUG(F("Analog ADC/DAC set to %d bits (resolution %s)\n"), ADC_RES_BITS, ftoa(buff, ADC_RESOLUTION));
	DEBUG(F("PWM set to %d resolution\n"), PWM_RESOLUTION);
	DEBUG(F("%s Watchdog enabled\nInitialization ended.\n"), SK_FEATURE);
}
