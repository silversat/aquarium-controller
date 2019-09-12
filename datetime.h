//
//	DateTime.h
//

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

DateTime CheckDatetimeConsistency() {
	DateTime datetime = rtc.now();
	if(datetime.hour() > 23 or datetime.minute() > 59 or datetime.second() > 59 or datetime.day() < 1 or datetime.day() > 31 or datetime.month() < 1 or datetime.month() > 12 or datetime.year() == 2165) {
		rtc.adjust(DateTime(2019, 1, 1, 12, 0, 0));		// January 1, 2019 at 12pm
		datetime = rtc.now();
	}
	return datetime;	
}

void printDateTime( DateTime dt ) {     	// Creata per semplificare il codice dell'impostazione di data e ora
	char buff[24];
	sprintf(buff, "%02d/%02d/%04d  %02d:%02d:%02d", dt.day(), dt.month(), dt.year(), dt.hour(), dt.minute(), dt.second());
	printString(buff, 0, 1);
}

void printDateTime( DateTimeImp dt ) {     	// Creata per semplificare il codice dell'impostazione di data e ora
	char buff[24];
	sprintf(buff, "%02d/%02d/%04d  %02d:%02d:%02d", dt.day, dt.month, dt.year, dt.hour, dt.minute, dt.second);
	printString(buff, 0, 1);
}

DateTime ImpostaDataOra() {  				// Funzione per impostazione data
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

	if(conferma == true) {	
		switch(kp_new) {
			case IR_OK:	
				datotempo = 10;
				kp_new = IR_NONE;		// avoid 'conferma' skipping
				conferma = false; 		// disattivo questa if in modo che Il tasto OK funzioni solo con le if di case datotempo = 6
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
			}
			if(kp_new == IR_DOWN) {
				ScrollHandler(dtimp.day, 1, giornimese[dtimp.month-1], ACT_DEC);
			}
			break;

		case 2:  									// Imposto mese
			stampafrecce(0, 2, 3, 15);

			if(kp_new == IR_UP) {
				ScrollHandler(dtimp.month, 1, 12, ACT_INC);
			}
			if(kp_new == IR_DOWN) {
				ScrollHandler(dtimp.month, 1, 12, ACT_DEC);
			}
			break;

		case 3:  						 			// Imposto anno
			stampafrecce(0, 2, 6, 4, 4);

			if(kp_new == IR_UP) {
				dtimp.year++;
			}
			if(kp_new == IR_DOWN) {
				dtimp.year--;
			}
			break;

		case 4:  									// Imposto ora
			stampafrecce(0, 2, 12, 5);

			if(kp_new == IR_UP)
				ScrollHandler(dtimp.hour, 0, 23, ACT_INC);

			if(kp_new == IR_DOWN) 
				ScrollHandler(dtimp.hour, 0, 23, ACT_DEC);

			break;

		case 5:  									// Imposto minuti
			stampafrecce(0, 2, 15, 3);

			if(kp_new == IR_UP)
				ScrollHandler(dtimp.minute, 0, 59, ACT_INC);

			if(kp_new == IR_DOWN) 
				ScrollHandler(dtimp.minute, 0, 59, ACT_DEC);

			break;

		case 6:  									// Imposto secondi
			stampafrecce(0, 2, 18, 0);

			if(kp_new == IR_UP)
				ScrollHandler(dtimp.second, 0, 59, ACT_INC);

			if(kp_new == IR_DOWN) 
				ScrollHandler(dtimp.second, 0, 59, ACT_DEC);

			break;

		case 10:
			if(conferma == false) {	
				printBlinkingString(confirm);

				if(kp_new == IR_OK or kp_new == IR_MENU) {	
					if(kp_new == IR_OK) {
						rtc.adjust(DateTime(dtimp.year, dtimp.month, dtimp.day, dtimp.hour, dtimp.minute, dtimp.second));
					}	
					conferma = true;
					dstatus = DS_IDLE_INIT;
				}
				if(kp_new == IR_RIGHT || kp_new == IR_LEFT || kp_new == IR_UP || kp_new == IR_DOWN) {	
					datotempo = 1;
					conferma = true;
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
	DateTime now = rtc.now();
	DEBUG("Current datetime: %02d-%02d-%4d %02d:%02d:%02d\n", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
}
