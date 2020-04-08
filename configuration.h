//
//	Display configuration
//

//#define OLED_32					// if defined, 128x32 oled display is used, otherwise LCD 20x4
//#define OLED_64					// if defined, 128x64 oled display is used, otherwise LCD 20x4
//#define OLED_128					// if defined, 128x128 oled display is used, otherwise LCD 20x4

#define RELE_PARALLEL				// if defined, use a parallel rele board, otherwise use a serial i2c rele board (PCF8574)

#define	BLINK_TIMER			500		// blinking intervam mS
#define	NINUTES_PER_DAY		1440
#define	SECONDS_PER_DAY		86400

//
//	Arduino PIN configuration
//
#define PIN_LIGHT_PWM_BASE	3		// PWM pin for led line 1 to LIGHT_LINE_NUMBER (3-8)
#define PIN_ONE_WIRE_BUS	46
#define PIN_IR_RECEIVER		49
#define PIN_PH_SENSOR		A8
#define PIN_EC_SENSOR		A11
#define PIN_BUZZER			11
#define PIN_RELAIS_1		22		// these are for the parallel relais board
#define PIN_RELAIS_2		23		// WARNING: you must configure 'relaisPin' array in 'scheda_rele.h' according with these defines
#define PIN_RELAIS_3		24		//			and SR_RELAIS_NUM into the relais board section below.
#define PIN_RELAIS_4		25
#define PIN_RELAIS_5		26
#define PIN_RELAIS_6		27
#define PIN_RELAIS_7		28
#define PIN_RELAIS_8		29
//
// seriale monitor port
//
#define	SERIAL_BAUD			115200
//
// relais board
//
#define	RL_ON					HIGH		// for inverted logic (RL_ON = LOW) change here
#define	RL_OFF					LOW			//		and here
#define	SR_RELAIS_NUM			0x08
#define	SR_RISCALDATORE			0x01		// water heater = relais #1
#define	SR_LUNAR_LIGHT			0x05		// lunar light = relais #5
#define	SR_WATER_VALVE_DRAIN	0x06		// drain water valve
#define	SR_WATER_VALVE_LOAD		0x07		// load water valve
#define	SR_CO2_VALVE			0x08		// CO2 valve
//
// temperature
//
#define	TEMP_SENSOR_RESOLUTION	10			// temperature sensor resolution in bit (9-12)
#define	TEMP_RANGE				2.5			// max delta-Temp allowed before alarm occurs
#define	TEMP_READ_INTERVAL		3000		// temperature time lapse (mSecs)
#define	PH_READ_INTERVAL		1000		// PH time lapse (mSecs)
#define	EC_READ_INTERVAL		1000		// PH time lapse (mSecs)
#define	TEMP_ALLOWED_MIN		10			// min allowed temperature when setting
#define	TEMP_ALLOWED_MAX		40			// max allowed temperature when setting
#define	TEMP_ALLOWED_DEFAULT	28			// default temperature when <min or >max in setting
#define TEMP_SAMPLES_NUM		10			// ring buffer temperature samples number
//
//	LIGHTS
//
#define	FADING_INTERVAL			110			// mSecs
#define	POWER_OFF				0			// Power OFF / mode OFF
#define	POWER_ON				1			// Power ON / mode ON
#define	POWER_AUTO				2			// Power mode AUTO
#define	POWER_ON_INC			3			// Power ON/increasing
#define	POWER_ON_DEC			4			// Power ON/decreasing
#define	LIGHT_LINE_NUMBER		5
#define	LIGHT_LINE_1			0
#define	LIGHT_LINE_2			1
#define	LIGHT_LINE_3			2
#define	LIGHT_LINE_4			3
#define	LIGHT_LINE_5			4
#define	LIGHT_LINE_6			5
//
//	LIGHTS SETUP FASES
//
#define LS_ON_HOUR				1
#define LS_ON_MIN				2
#define LS_OFF_HOUR				3
#define LS_OFF_MIN				4
#define LS_PAGE1_CONFIRM		5
#define LS_FAD_HOUR				6
#define LS_FAD_MIN				7
#define LS_PAGE2_CONFIRM		8

//
//	EEPROM/NVRAM data position
//
//	PH calibration data from 0x00 to 0x09
//	PH calibration data from 0x0A to 0x13
//	PLAFONIERE: 6 * 12 bytes each = 72 starting by NVRAM_START_ADDR
//
#define	NVRAM_PH_NEUTRAL		0x00	// address where PH neutral voltage is stored (4 bytes)
#define	NVRAM_PH_ACID			0x04	// address where PH acid voltage is stored (4 bytes)
#define	NVRAM_EC_KVALUEHIGH		0x08	// address where EC kvalue high is stored (4 bytes)
#define	NVRAM_EC_KVALUELOW		0x0C	// address where EC kvalue low is stored (4 bytes)
#define	NVRAM_TEMP_ADDR			0x14	// Static Memory address where EC data is stored
#define	NVRAM_START_ADDR		0x20
#define	PLAFO_MODE				0		// 0=OFF, 1=ON, 2=AUTO
#define	PLAFO_MAX_FADING		1		// Max fading value set
#define	PLAFO_MINS_ON			2		// NEW VERSION DATA: 2bytes each for INT saves
#define	PLAFO_MINS_OFF			4
#define	PLAFO_MINS_FAD			6
#define	PLAFO_MINS_FA			8
#define	PLAFO_MINS_IT			10

#define	PLAFO_NAME_1			"Warm white"
#define	PLAFO_NAME_2			"Natural white"
#define	PLAFO_NAME_3			"Cold white"
#define	PLAFO_NAME_4			"Red"
#define	PLAFO_NAME_5			"Blue"
#define	PLAFO_NAME_6			""
//
//	Action defines
//
#define	ACT_INC	1		// increment
#define	ACT_DEC	0		// decrement
//
//	DISPATCHER STATUS
//		The first nibble is the menu item. if 0, no menu displayed, normal run.
//		The second nibble (3 LSB) is the current menu item status indicator. MSB is the init flag
//			0x00	0000 0 000	running
//			0x19	0001 1 001	setup menu, voice 1, init function call
//			0x10	0001 0 000	setup menu, voice 1, already initialized
//			0x28	0010 1 000	setup menu, voice 2, init function call
//			0x21	0010 0 001	setup menu, voice 2, voice 1, already initialized
//
#define DS_INIT_BIT					0x03		// third bit is the init bit
#define DS_INIT_FLAG				0x08		// value of the init bit set (2^init-bit => 2^3=8)
#define	DS_INIT_MASK				0xF7		// clear Init bit mask
#define	DS_IDLE						0x00		//	Normal run mode
#define	DS_IDLE_INIT				0x08
#define	DS_SETUP					0x10		//	Setup mode (menu)
#define	DS_SETUP_DATETIME			0x11		//	Setup mode, datetime submenu
#define	DS_SETUP_LIGHTS				0x20					//	Setup mode, lights submenu 1 (base for Plafo array)
#define	DS_SETUP_LIGHTS_0			DS_SETUP_LIGHTS+1		//	Setup mode, lights Funz/LMax
#define	DS_SETUP_LIGHTS_1			DS_SETUP_LIGHTS+2		//	Setup mode, lights line 1
#define	DS_SETUP_LIGHTS_2			DS_SETUP_LIGHTS+3		//	Setup mode, lights line 2
#define	DS_SETUP_LIGHTS_3			DS_SETUP_LIGHTS+4		//	Setup mode, lights line 3
#define	DS_SETUP_LIGHTS_4			DS_SETUP_LIGHTS+5		//	Setup mode, lights line 4
#define	DS_SETUP_LIGHTS_5			DS_SETUP_LIGHTS+6		//	Setup mode, lights line 5
#define	DS_SETUP_LIGHTS_6			DS_SETUP_LIGHTS+7		//	Setup mode, lights line 6
#define	DS_SETUP_TEMP				0x31					//	Setup mode, temperature submenu
#define	DS_SETUP_INFOLIGHTS			0x41					//	Setup mode, info luci submenu
#define	DS_SETUP_CALIBRATION		0x51					//	Setup mode, sensors calibration
#define	DS_SETUP_CALIBRATION_PH		DS_SETUP_CALIBRATION+1	//	Setup mode, PH sensors calibration
#define	DS_SETUP_CALIBRATION_EC		DS_SETUP_CALIBRATION+2	//	Setup mode, EC sensors calibration
#define	DS_SETUP_TIMERUN			0x61					//	Setup mode, test time run
#define	DS_SETUP_SPARE_3			0x71					//	Setup mode, spare item
#define	DS_SETUP_SPARE_4			0x81					//	Setup mode, spare item
#define	DS_SETUP_SPARE_5			0x91					//	Setup mode, spare item
