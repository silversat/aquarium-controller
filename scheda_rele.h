//
//	SCHEDA_RELAIS.H
//	this code has been developed onto a 'funduino' Keyes ralais board HCARDU0018 ver.8R1A
//	the board include 8 relais. 4 relais board at the time is enough (HCARDU0015 ver.4R1B)
//
byte relaisPin[SR_RELAIS_NUM] = { PIN_RELAIS_1, PIN_RELAIS_2, PIN_RELAIS_3, PIN_RELAIS_4,
								  PIN_RELAIS_5, PIN_RELAIS_6, PIN_RELAIS_7, PIN_RELAIS_8 };

void relais( byte idx, boolean action ) {
	digitalWrite(relaisPin[idx-1], action);
}

boolean	relaisStatus( byte idx ) {
	return digitalRead(relaisPin[idx-1]);
}

void RelaisInit() {
	for( int i = 0; i < SR_RELAIS_NUM; i++ ) {
		pinMode(relaisPin[i], OUTPUT);			// set pin in output mode
		delay(100);								// wait 100 mS in order the bus stabilize.
		digitalWrite(relaisPin[i], RL_OFF);		// switch relais off (security default)
	}
	DEBUG("Relais board OK\n");
}	