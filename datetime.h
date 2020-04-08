//
//	DateTime.h
//

#define	TEST_TIME_LAPSE

extern		void LoadAllLightStatus();
extern		void restartLights();

typedef struct {
	int 	year;		// Year
	uint8_t month;		// Month 1-12
	uint8_t day;		// Day 1-31
	uint8_t hour;		// Hours 0-23
	uint8_t minute;		// Minutes 0-59
	uint8_t second;		// Seconds 0-59
} DateTimeImp;

RTC_DS3231	rtc;		// @ 0x68
DateTime	datetime;
byte 		datotempo;
byte 		giornimese[12] = {31,28,31,30,31,30,31,31,30,31,30,31};  // creo un'array per il controllo sull'immisione dei giorni
bool 		fastTimeRun = false;

uint32_t myMillis() {
	if(fastTimeRun) {
		return millis()*120;		// 120mS al mS = 2min/sec
	} else {
		return millis();
	}
}

DateTime getDateTime( bool renewTime=false, int incHour=0 ) {
	if(fastTimeRun or renewTime) {
		static DateTime dt = rtc.now();
		static int hour = dt.hour();
		static int minute = dt.minute();
		static int second = 0;
		static uint32_t timer = myMillis();
		
		if(renewTime) {
			dt = rtc.now();
			hour = dt.hour();
			minute = dt.minute();
			second = 0;
			timer = myMillis();
		}

		if(incHour != 0) {						// manually increment/decrement hour (key up/down)
			hour += incHour;
			if(hour >= 24) hour -= 24;
			if(hour < 0) hour += 24;
			restartLights();
		}
		
		if(myMillis() - timer > 10000) {			// 1min/500mS = 2min/sec
			minute++;
			if(minute >= 60) {
				minute -= 60;
				hour++;
				if(hour >= 24) hour -= 24;
				if(hour < 0) hour += 24;
			}
			timer = myMillis();
		}
		return DateTime (dt.year(), dt.month(), dt.day(), hour, minute, second);
	} else {
		return rtc.now();
	}
}

DateTime CheckDatetimeConsistency() {
	DateTime datetime = getDateTime();
	if(datetime.hour() > 23 or datetime.minute() > 59 or datetime.second() > 59 or datetime.day() < 1 or datetime.day() > 31 or datetime.month() < 1 or datetime.month() > 12 or datetime.year() == 2165) {
		rtc.adjust(DateTime(2019, 1, 1, 12, 0, 0));		// January 1, 2019 at 12pm
		datetime = getDateTime();
	}
	return datetime;	
}

void printTime( DateTime dt, uint8_t col=0, uint8_t row=1 ) {     	// Creata per semplificare il codice dell'impostazione di data e ora
	char buff[12];
	sprintf(buff, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
	printString(buff, col, row);
}

void printDateTime( DateTime dt, uint8_t col=0, uint8_t row=1 ) {     	// Creata per semplificare il codice dell'impostazione di data e ora
	char buff[24];
	sprintf(buff, "%02d-%02d-%04d  %02d:%02d:%02d", dt.day(), dt.month(), dt.year(), dt.hour(), dt.minute(), dt.second());
	printString(buff, col, row);
}

void printDateTime( DateTimeImp dt, uint8_t col=0, uint8_t row=1 ) {   	// Creata per semplificare il codice dell'impostazione di data e ora
	char buff[24];
	sprintf(buff, "%02d-%02d-%04d  %02d:%02d:%02d", dt.day, dt.month, dt.year, dt.hour, dt.minute, dt.second);
	printString(buff, col, row);
}

void displayRunStatus( bool status, uint8_t row ) {
	if(status) {
		printStringCenter(" Fast ", row);
	} else {
		printStringCenter("Normal", row);
	}
}

void fastTimeSetup() {
	static bool confirm, status;
	
	if(CheckInitBit(true)) {					// if di inizializzazazione della procedura: viene eseguita una sola volta
		status = fastTimeRun;					// false=NORMAL, true=FAST	
		displayClear();
		printStringCenter("Time Run Mode", 0);
		displayRunStatus( status, 2);
		confirm = true;
	}
	if(kp_new == IR_OK and confirm) {
		confirm = false;
		kp_new = IR_NONE;		// avoid 'conferma' skipping
	} else if(kp_new == IR_RIGHT or kp_new == IR_UP or kp_new == IR_LEFT or kp_new == IR_DOWN) {
		status = !status;
		displayRunStatus( status, 2);
	} else if(kp_new == IR_MENU) {
		SetInitBit(DS_SETUP);
	}

	if(!confirm) {
		if(fastTimeRun != status) {
			fastTimeRun = status;
			if(!fastTimeRun) {
				getDateTime(true);
			}
			LoadAllLightStatus();
		}
		SetInitBit(DS_SETUP);
	}
}

DateTime ImpostaDataOra() {  				// Funzione per impostazione data
	static bool confirm = true;
	static DateTimeImp dtimp;
	
	if(CheckInitBit(true)) {					// if di inizializzazazione della procedura: viene eseguita una sola volta
		datetime = CheckDatetimeConsistency();
		dtimp.year = datetime.year();
		dtimp.month = datetime.month();
		dtimp.day = datetime.day();
		dtimp.hour = datetime.hour();
		dtimp.minute = datetime.minute();
		dtimp.second = datetime.second();
		datotempo = 1;
		displayClear();
		printStringCenter("Date-Time Setup", 0);

		if((datetime.year() % 4) == 0) {
			giornimese[1] = 29;				// se l'anno e' bisestile febbraio ha 29 giorni...
		} else {
			giornimese[1] = 28;				// altrimenti febbraio ha 28 giorni
		}
	}
	printDateTime(dtimp);					// Stampo data e ora

	if(confirm == true) {	
		switch(kp_new) {
			case IR_OK:	
				datotempo = 10;
				kp_new = IR_NONE;		// avoid 'conferma' skipping
				confirm = false; 		// disattivo questa if in modo che Il tasto OK funzioni solo con le if di case datotempo = 6
				break;
			case IR_MENU:
				SetInitBit(DS_SETUP);
				break;
			case IR_RIGHT:
				ScrollHandler(datotempo, 1, 6, ACT_INC);
				break;
			case IR_LEFT:
				ScrollHandler(datotempo, 1, 6, ACT_DEC);
				break;
			default:
				break;
		}		
	}

	switch(datotempo) { 
		case 1:  											// Imposto giorno
			stampafrecce(0, 2, 0, 18);						// print two arrows
			displayClearRow(3);
			if(kp_new == IR_UP) {
				ScrollHandler(dtimp.day, 1, giornimese[dtimp.month-1], ACT_INC);		// doesn't work correctly on year change!!!!!!!
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(dtimp.day, 1, giornimese[dtimp.month-1], ACT_DEC);
			}
			break;

		case 2:  									// Imposto mese
			stampafrecce(0, 2, 3, 15);
			if(kp_new == IR_UP) {
				ScrollHandler(dtimp.month, 1, 12, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(dtimp.month, 1, 12, ACT_DEC);
			}
			break;

		case 3:  						 			// Imposto anno
			stampafrecce(0, 2, 6, 4, 4);
			if(kp_new == IR_UP) {
				dtimp.year++;
			} else if(kp_new == IR_DOWN) {
				dtimp.year--;
			}
			break;

		case 4:  									// Imposto ora
			stampafrecce(0, 2, 12, 5);
			if(kp_new == IR_UP) {
				ScrollHandler(dtimp.hour, 0, 23, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(dtimp.hour, 0, 23, ACT_DEC);
			}
			break;

		case 5:  									// Imposto minuti
			stampafrecce(0, 2, 15, 3);
			if(kp_new == IR_UP) {
				ScrollHandler(dtimp.minute, 0, 59, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(dtimp.minute, 0, 59, ACT_DEC);
			}
			break;

		case 6:  									// Imposto secondi
			stampafrecce(0, 2, 18, 0);
			if(kp_new == IR_UP) {
				ScrollHandler(dtimp.second, 0, 59, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(dtimp.second, 0, 59, ACT_DEC);
			}
			break;

		case 10:
			if(!confirm) {	
				printBlinkingString(confirm_msg);
				if(kp_new == IR_OK or kp_new == IR_MENU) {	
					if(kp_new == IR_OK) {
						rtc.adjust(DateTime(dtimp.year, dtimp.month, dtimp.day, dtimp.hour, dtimp.minute, dtimp.second));
					}	
					confirm = true;
					dstatus = DS_IDLE_INIT;
				}
				if(kp_new == IR_RIGHT || kp_new == IR_LEFT || kp_new == IR_UP || kp_new == IR_DOWN) {	
					datotempo = 1;
					confirm = true;
				}
			}
			break;
	}
}

void RtcInit() {
	if(rtc.begin()) {
		if (rtc.lostPower()) {
			DEBUG(F("RTC lost power, lets set the time!"));
			// following line sets the RTC to the date & time this sketch was compiled
			rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
			// This line sets the RTC with an explicit date & time, for example to set
			// January 21, 2014 at 3am you would call:
			// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
		}
		DEBUG(F("RTC module OK\n"));
	} else {
		DEBUG(F("Couldn't find RTC module\n"));
	}
	#if defined DEBUG_MESSAGES
		DateTime dt = getDateTime();
		Serial.printf(F("Current datetime: %02d-%02d-%4d %02d:%02d:%02d\n"), dt.day(), dt.month(), dt.year(), dt.hour(), dt.minute(), dt.second());
	#endif
	fastTimeRun = false;
}
