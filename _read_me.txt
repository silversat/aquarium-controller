- su MEGA2560, modificare la definizione dell'interrupt usato dall'IR nel file IRremoteInt.h:
	#define IR_USE_TIMER2     // tx = pin 9
	//#define IR_USE_TIMER5   // tx = pin 46

- il buzzer per default è montato sul pin 11
- il ds18b20 per default è montato sul pin 46
- il display: SDA: 20, SCL, 21
- il ricevitore IR sul pin 49.
- water level sensor pin 18
- water turbidity sensor pin A4
- water PH sensor pin A8
- water EC sensor pin A11
