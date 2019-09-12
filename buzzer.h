//
//	BUZZER.h
//
// 	Dichiarazioni e funzione per i segnali acustici

#define BEEP_ON		LOW
#define BEEP_OFF	HIGH
#define ATTACK		150
#define DECAY		10

void beepOn() {
	digitalWrite(PIN_BUZZER, BEEP_ON);
}

void beepOff() {
	digitalWrite(PIN_BUZZER, BEEP_OFF);
}

void buzerrore() {
	beepOn();
	delay(ATTACK);
	beepOff();
	delay(DECAY);

	beepOn();
	delay(ATTACK);
	beepOff();
	delay(DECAY);

	beepOn();
	delay(ATTACK);
	beepOff();
}

void BuzzerInit() {
	pinMode(PIN_BUZZER, OUTPUT); 
	DEBUG("Buzzer OK\n");
}	
