//
//	Display configuration
//

//#define OLED_32					// if defined, 128x32 oled display is used, otherwise LCD 20x4
//#define OLED_64					// if defined, 128x64 oled display is used, otherwise LCD 20x4
//#define OLED_128					// if defined, 128x128 oled display is used, otherwise LCD 20x4

#define RELE_PARALLEL				// if defined, use a parallel rele board, otherwise use a serial i2c rele board (PCF8574)

#define	BLINK_TIMER			500		// blinking intervam mS
#define	NINUTES_PER_DAY		1440

//
//	Arduino PIN configuration
//
#define PIN_LIGHT_PWM_BASE	3		// PWM pin for led line 1 to LIGHT_LINE_NUMBER (3-8)
#define PIN_ONE_WIRE_BUS	46
#define PIN_IR_RECEIVER		49
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
#define	TEMPO_LETTURA			5000		// temperature time lapse
#define	SMEMORY_TEMP_ADDR		60			// Static Memory address where temp data is stored
#define	TEMP_ALLOWED_MIN		10			// min allowed temperature when setting
#define	TEMP_ALLOWED_MAX		40			// max allowed temperature when setting
#define	TEMP_ALLOWED_DEFAULT	28			// default temperature when <min or >max in setting
//
//	LIGHTS
//
#define	LIGHT_LINE_NUMBER		5
#define	LIGHT_LINE_1			0
#define	LIGHT_LINE_2			1
#define	LIGHT_LINE_3			2
#define	LIGHT_LINE_4			3
#define	LIGHT_LINE_5			4
#define	LIGHT_LINE_6			5
#define	LIGHT_MODE_OFF			0
#define	LIGHT_MODE_ON			1
#define	LIGHT_MODE_AUTO			2
//
//	PLAFONIERE
//
#define	PLAFO_MODE				0		// 0=OFF, 1=ON, 2=AUTO
#define	PLAFO_MAX_FADING		1		// Max fading value set
#define	PLAFO_MINS_ON			2		// NEW VERSION DATA
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
#define DS_INIT_BIT				0x03		// third bit is the init bit
#define DS_INIT_FLAG			0x08		// value of the init bit set (2^init-bit => 2^3=8)
#define	DS_INIT_MASK			0xF7		// clear Init bit mask
#define	DS_IDLE					0x00		//	Normal run mode
#define	DS_IDLE_INIT			0x08
#define	DS_SETUP				0x10		//	Setup mode (menu)
#define	DS_SETUP_DATETIME		0x11		//	Setup mode, datetime submenu
#define	DS_SETUP_LIGHTS			0x20					//	Setup mode, lights submenu 1 (base for Plafo array)
#define	DS_SETUP_LIGHTS_0		DS_SETUP_LIGHTS+1		//	Setup mode, lights Funz/LMax
#define	DS_SETUP_LIGHTS_1		DS_SETUP_LIGHTS+2		//	Setup mode, lights line 1
#define	DS_SETUP_LIGHTS_2		DS_SETUP_LIGHTS+3		//	Setup mode, lights line 2
#define	DS_SETUP_LIGHTS_3		DS_SETUP_LIGHTS+4		//	Setup mode, lights line 3
#define	DS_SETUP_LIGHTS_4		DS_SETUP_LIGHTS+5		//	Setup mode, lights line 4
#define	DS_SETUP_LIGHTS_5		DS_SETUP_LIGHTS+6		//	Setup mode, lights line 5
#define	DS_SETUP_LIGHTS_6		DS_SETUP_LIGHTS+7		//	Setup mode, lights line 6
#define	DS_SETUP_TEMP			0x31		//	Setup mode, temperature submenu
#define	DS_SETUP_INFOLIGHTS		0x41		//	Setup mode, info luci submenu
#define	DS_SETUP_SPARE_1		0x51		//	Setup mode, spare item
#define	DS_SETUP_SPARE_2		0x61		//	Setup mode, spare item
#define	DS_SETUP_SPARE_3		0x71		//	Setup mode, spare item
#define	DS_SETUP_SPARE_4		0x81		//	Setup mode, spare item
#define	DS_SETUP_SPARE_5		0x91		//	Setup mode, spare item
