//
//	LIGHTS.h
//

#define		DEBUG_LIGHTS
#define		DISPLAY_MAX_ITEMS	DISPLAY_MAX_ROWS-1

typedef struct {				//Creo una struttura di variabili relative ad un Tasto
	uint8_t		hour;
	uint8_t 	min;
} time_type;

typedef	struct 	{				// I valori di Pos, vengono usati per determinare l'indirizzo di memoria in cui vengono registrati gli orari
	uint8_t 	workingMode;	// 0 se la linea e Off in manuale, 1 se la linea ON in manuale, 2 se la linea e' in AUT fuzionamento automatico
	uint8_t 	powerState;		// Tiene lo stato dell'alimentazione della linea (OFF, ON, ON_INC, ON_DEC)
	uint8_t 	pwmPin;			// Contiene il numero di pin di controllo PWM delle singole linee
	uint8_t 	pwmValue;		// Valore di Fading corrente
	uint8_t 	pwmMax;			// Contiene il valore di luminosita massima impostata
	int			minsOn;
	int			minsOff;
	int			minsFad;
	int			minsFA;
	int			minsIT;
	uint32_t 	fadingTimer;	// Usato per lo storage di millis() durante l'esecuzione del fading
	bool		startup;
} PlafoData;

typedef struct {				//Creo una struttura di variabili relative ad un Tasto
	uint8_t 	workingMode;
	uint8_t 	pwmMax;
} temp_data;

//char*		plafoNames[] = {NULL, NULL, NULL, NULL, NULL, NULL};
char*		plafoNames[] = {PLAFO_NAME_1, PLAFO_NAME_2, PLAFO_NAME_3, PLAFO_NAME_4, PLAFO_NAME_5, PLAFO_NAME_6};
PlafoData 	Plafo[LIGHT_LINE_NUMBER];
menu_type 	LightsMenu[LIGHT_LINE_NUMBER+2];

int LucePiena( uint8_t oretotaliftp, uint8_t minutitotaliftp, uint8_t oretotalifad, uint8_t minutitotalifad )	{ 
	return int(oretotaliftp * 60 + minutitotaliftp) - int ((oretotalifad * 60 + minutitotalifad) * 2);
}

void restartLights() {
	for(int idx = 0; idx < LIGHT_LINE_NUMBER; idx++) {
		Plafo[idx].startup = true;
	}
}

uint8_t getTimeHour( int timeInMins ) {
	return int(timeInMins / 60);
}

uint8_t getTimeMin( int timeInMins ) {
	return int(timeInMins % 60);
}

char* mins2timeString( char* buff, int mins ) {
	int hour, min;
	
	if(mins > MINUTES_PER_DAY) mins -= MINUTES_PER_DAY;
	hour = int(mins/60);
	min = (mins-(hour*60));
	sprintf(buff, "%02d:%02d", hour, min);
	return buff;
}

char* decodePowerState( char* buff, uint8_t state ) {
	if(state == POWER_OFF) {
		strcpy(buff, "Off");
	} else if(state == POWER_ON) {
		strcpy(buff, "On ");
	} else if(state == POWER_ON_INC) {
		strcpy(buff, "Inc");
	} else if(state == POWER_ON_DEC) {
		strcpy(buff, "Dec");
	} else if(state == POWER_AUTO) {
		strcpy(buff, "Aut");
	} else {
		strcpy(buff, "N/A");
	}
	return buff;
}

uint32_t TimeInSeconds() {									//Converte l'orara corrente in secondi
	DateTime dt = getDateTime();
	uint32_t secs = (((uint32_t)dt.hour() * 3600) + ((uint32_t)dt.minute() * 60) + dt.second());
	return secs;
}

uint32_t TimeInSeconds( uint8_t hours, uint8_t mins ) {		//Converte l'orario in secondi (da 0 a 86399 sec)
	return (((uint32_t)hours * 3600) + ((uint32_t)mins * 60));
}

uint32_t TimeInSeconds( int mins ) {						//Converte l'orario in secondi (da 0 a 86399 sec) {
	return ((uint32_t)mins * 60);
}

uint16_t TimeInMinutes() {									//Converte l'orario corrente in minuti
	DateTime dt = getDateTime();
	uint16_t mins = ((uint16_t)dt.hour()*60) + (uint16_t)dt.minute();
	return mins;
}

uint16_t TimeInMinutes( uint8_t hours, uint8_t mins ) {		//Converte l'orario in minuti
	return ((hours * 60) + mins);
}

uint16_t TimeInMinutes( time_type time ) {
	return ((time.hour * 60) + time.min);
}

void calcTotalLightTime( time_type &lux, int minsOn, int minsOff ) {
	int total = minsOff - minsOn;
	if(total < 0) total += MINUTES_PER_DAY;		// 24 * 60 = 1440 (MINUTES_PER_DAY)
	lux.hour = getTimeHour(total);
	lux.min = getTimeMin(total);
}

void calcMaxLightTime( time_type &lux, int minsOn, int minsOff, int minsFad ) {
	int max = minsOff - minsOn;
	if(max < 0) max += MINUTES_PER_DAY;		// 24 * 60 = 1440 (MINUTES_PER_DAY)
	max -= (minsFad * 2);
	lux.hour = getTimeHour(max);
	lux.min = getTimeMin(max);
}

int calcLuxAverage() {
	float luxmed = 0;
	for(int i = 0; i < DAYLIGHT_LINE_NUMBER; i++) {
		luxmed += ((uint32_t)Plafo[i].pwmValue * 255 / (uint32_t)Plafo[i].pwmMax);
	}
	luxmed = ((luxmed / DAYLIGHT_LINE_NUMBER) * 100) / 255;
	return int(luxmed);
}

void LoadLightStatus( uint8_t channel )	{	
	uint32_t currentTime, switchOn, switchOff, sunriseEnd, sunsetBegin;
	uint8_t baseAddr = (channel * 12) + NVRAM_START_ADDR;

	// Questa funzione viene eseguita in caso di riavvio di arduino per reset, per mancanza di luce o in caso di variazione degli orari dei 
	// fotoperiodi, in pratica carica nella struct i dati e calcola in caso di funzionamento automatico i dati necessari all'esecuzione del
	// del fading in particolare lo stato del fading al momento del ripristino, l'intevrello tra un passaggio di rampa ed un'altro, lo stato
	// delle linee di alimentazione, se e' in corso l'alba il tramonto, se c'e' luce piena e se e' tutto spento.
	// Viene eseguita una sola volta ed i dati calcolati o settati vengono poi usati dalla funzione GestioneLuci() per eseguire 
	// semplici confronti con i millis alleggerendo cosi il lavoro di Arduino.

	Plafo[channel].workingMode = readStaticMemory(baseAddr + PLAFO_MODE);
	Plafo[channel].pwmMax = readStaticMemory(baseAddr + PLAFO_MAX_FADING);
	Plafo[channel].minsOn = readStaticMemoryInt(baseAddr + PLAFO_MINS_ON);
	Plafo[channel].minsOff = readStaticMemoryInt(baseAddr + PLAFO_MINS_OFF);
	Plafo[channel].minsFA = readStaticMemoryInt(baseAddr + PLAFO_MINS_FA);
	Plafo[channel].minsIT = readStaticMemoryInt(baseAddr + PLAFO_MINS_IT);

	if(Plafo[channel].workingMode > POWER_AUTO) {
		Plafo[channel].workingMode = POWER_AUTO;
		writeStaticMemoryInt(baseAddr + PLAFO_MODE, POWER_AUTO);
	}
	if(Plafo[channel].minsOn > MINUTES_PER_DAY) {
		Plafo[channel].minsOn = 0;
		writeStaticMemoryInt(baseAddr + PLAFO_MINS_ON, 0);
	}
	if(Plafo[channel].minsOff > MINUTES_PER_2DAYS) {
		Plafo[channel].minsOff = 0;
		writeStaticMemoryInt(baseAddr + PLAFO_MINS_OFF, 0);
	}
	if(Plafo[channel].minsFA > MINUTES_PER_2DAYS) {
		Plafo[channel].minsFA = 0;
		writeStaticMemoryInt(baseAddr + PLAFO_MINS_FA, 0);
	}
	if(Plafo[channel].minsIT > MINUTES_PER_2DAYS) {
		Plafo[channel].minsIT = 0;
		writeStaticMemoryInt(baseAddr + PLAFO_MINS_IT, 0);
	}
	Plafo[channel].minsFad = (Plafo[channel].minsFA - Plafo[channel].minsOn);

	if(Plafo[channel].workingMode >= POWER_AUTO) {		// 0xFF is virgin eeprom data
		#if defined(DEBUG_LIGHTS)		
			char buff[8];

			if(channel == 0) {
				Serial.println("Chan\tpin\t   sunrise\t   sunset\t fad\tmax pwm");
			}
			Serial.print(" ");
			Serial.print(channel+1);
			Serial.print("\t ");
			Serial.print(Plafo[channel].pwmPin);
			Serial.print("\t");
			Serial.print(mins2timeString(buff, Plafo[channel].minsOn));
			Serial.print("\t");
			Serial.print(mins2timeString(buff, Plafo[channel].minsFA));
			Serial.print("\t");
			Serial.print(mins2timeString(buff, Plafo[channel].minsIT));
			Serial.print("\t");
			Serial.print(mins2timeString(buff, Plafo[channel].minsOff));
			Serial.print("\t");
			Serial.print(mins2timeString(buff, Plafo[channel].minsFad));
			Serial.print("\t");
			Serial.print(Plafo[channel].pwmMax);
			Serial.println();
		#endif
		
		Plafo[channel].startup = true;
		Plafo[channel].fadingTimer = myMillis();					// Valore necessario in gestione luci per scandire il fading
	}
}

void LoadAllLightStatus() {
	for(int idx = 0; idx < LIGHT_LINE_NUMBER; idx++) {
		LoadLightStatus(idx);								// load plafo line config from nvram
	}
}

void LightsHandler() {
	static uint8_t channel = 0;
	uint16_t pwmValue = Plafo[channel].pwmValue;				// save the old value

	if(Plafo[channel].workingMode == POWER_OFF) {
		if(pwmValue <= 0)	{
			if(Plafo[channel].powerState != POWER_OFF) {
				Plafo[channel].powerState = POWER_OFF;			// if reached pwm=0, switch OFF power rele
			}
		} else {	
			if(((myMillis() - Plafo[channel].fadingTimer) >= FADING_INTERVAL)) {	
				pwmValue--;
				Plafo[channel].fadingTimer = myMillis();
			}
		}
	} else if(Plafo[channel].workingMode == POWER_ON) {
		if(Plafo[channel].powerState != POWER_ON) {
			Plafo[channel].powerState = POWER_ON;				// immediatly switch ON power rele
		}
		if(pwmValue < Plafo[channel].pwmMax)	{				// and begins fading-up till max reached
			if(((myMillis() - Plafo[channel].fadingTimer) >= FADING_INTERVAL)) {	
				pwmValue++;
				Plafo[channel].fadingTimer = myMillis();
			}
		}
	} else if(Plafo[channel].workingMode == POWER_AUTO) {
		uint16_t curTime = TimeInMinutes();
		uint16_t minsOn = Plafo[channel].minsOn;
		uint16_t minsOff = Plafo[channel].minsOff;
		uint16_t pwmMax = Plafo[channel].pwmMax;
		uint16_t minsIT = Plafo[channel].minsIT;
		uint16_t minsFA = Plafo[channel].minsFA;
		
		if(curTime < minsOn and Plafo[channel].powerState > POWER_OFF) {				// adjust current time if overflowed to next day
			curTime += MINUTES_PER_DAY;
		}
		
		if(Plafo[channel].startup) {													// fase di startup
			if(curTime >= minsOn and curTime <= minsOff) {								// se in fase di luce
				uint16_t targetPwm;														// calcola il pwm in base a:
				if(curTime >= minsOn and curTime < minsFA) {							// 	- alba
					targetPwm = (float) pwmMax / (minsFA-minsOn) * (curTime-minsOn);
				} else if(curTime >= minsFA and curTime < minsIT) {						// 	- luce piena
					targetPwm = pwmMax;
				} else if(curTime >= minsIT and curTime < minsOff) {					// 	- tramonto
					targetPwm = (float) pwmMax / (minsOff-minsIT) * (minsOff-curTime);
				}
				
				if(Plafo[channel].powerState != POWER_ON) {
					Plafo[channel].powerState = POWER_ON;								// immediatly switch ON power rele
				}

				if(pwmValue < targetPwm) {												// and begins fading-up till max reached
					if(((myMillis() - Plafo[channel].fadingTimer) >= FADING_INTERVAL)) {	
						pwmValue += (1+int(pwmMax/64));									// calcola l'incremento pesato in base al pwmMax
						if(pwmValue > pwmMax) pwmValue = targetPwm;
						Plafo[channel].fadingTimer = myMillis();
					}
				} else {
					Plafo[channel].startup = false;
					DEBUG(F("Exiting from light %d startup\n"), channel+1);
				}
			} else {
				Plafo[channel].startup = false;
				DEBUG(F("Skipping light %d startup\n"), channel+1);
			}
		} else {
			if(curTime < minsOn or curTime > minsOff) {										// fase di buio
				if(Plafo[channel].powerState == POWER_ON_DEC or Plafo[channel].startup) {
					Plafo[channel].powerState = POWER_OFF;
					pwmValue = 0;
					DEBUG(F("Light %d is now off\n"), channel+1);
				}
			} else if(curTime >= minsOn and curTime < minsFA) {								// fase di alba
				if(Plafo[channel].powerState == POWER_OFF or Plafo[channel].startup) {
					Plafo[channel].powerState = POWER_ON_INC;
					pwmValue = 0;
					DEBUG(F("Light %d is fading up\n"), channel+1);
				}
			} else if(curTime >= minsFA and curTime < minsIT) {								// fase di luce piena
				if(Plafo[channel].powerState == POWER_ON_INC or Plafo[channel].startup) {
					Plafo[channel].powerState = POWER_ON;
					pwmValue = pwmMax;
					DEBUG(F("Light %d is now on\n"), channel+1);
				}
			} else if(curTime >= minsIT and curTime < minsOff) {							// fase di tramonto
				if(Plafo[channel].powerState == POWER_ON or Plafo[channel].startup) {
					Plafo[channel].powerState = POWER_ON_DEC;
					pwmValue = pwmMax;
					DEBUG(F("Light %d is fading down\n"), channel+1);
				}
			}

			if(Plafo[channel].powerState == POWER_ON_INC) {									// fase ALBA
				if(pwmValue < pwmMax) {
					pwmValue = (float) pwmMax / (minsFA-minsOn) * (curTime-minsOn);
				} else {
					Plafo[channel].powerState = POWER_ON;
					pwmValue = pwmMax; 
					DEBUG(F("Light %d is now on\n"), channel+1);
				}	
			} else if(Plafo[channel].powerState == POWER_ON_DEC) {							// fase TRAMONTO
				if(pwmValue > 0) {
					pwmValue = (float) pwmMax / (minsOff-minsIT) * (minsOff-curTime);
				} else {	
					Plafo[channel].powerState = POWER_OFF;
					pwmValue = 0; 
					DEBUG(F("Light %d is now off\n"), channel+1);
				}
			}
		}
	}
	
	if(pwmValue != Plafo[channel].pwmValue) {
		Plafo[channel].pwmValue = pwmValue;				// update new value
		analogWrite(Plafo[channel].pwmPin, pwmValue);	// and write it down
		DEBUG("Light %d pwmValue=%3d\n", channel+1, pwmValue);
	}
	if(++channel >= LIGHT_LINE_NUMBER) channel = 0;
}

void InfoLuci() {
	static uint8_t begin = 0;
	char buff[24];

	if(CheckInitBit(true)) {		// Con questa if salvo le variabili interessate solo la prima volta che entro nell'impostazione
		displayClearRow(0);
		if(LIGHT_LINE_NUMBER > DISPLAY_MAX_ITEMS) {
			if(begin+DISPLAY_MAX_ITEMS < LIGHT_LINE_NUMBER) {
				printString(">>", DISPLAY_MAX_COLS-2, 0);
			}
			if(begin > 0)  {
				printString("<<", 0, 0);
			}
		}
	}

	printTime(datetime, 6, 0);
	for(byte channel = begin; channel < (begin + DISPLAY_MAX_ITEMS); channel++) {
		if(channel < LIGHT_LINE_NUMBER) {
			uint8_t perc = (uint8_t) int((Plafo[channel].pwmValue * 100) / Plafo[channel].pwmMax);
			char state[4];
			decodePowerState(state, Plafo[channel].powerState);
			sprintf(buff, "L%d: %s Fad:%3d%% %s", channel+1, state, perc, Plafo[channel].workingMode == POWER_AUTO?"Aut":"Man");
			printString(buff, 0, channel-begin+1);
		} else {
			displayClearRow(channel-begin+1);
		}
	}

	if(kp_new == IR_OK or kp_new == IR_MENU) {
		SetInitBit(DS_SETUP);
	} else if(kp_new == IR_LEFT and begin > 0) {
		begin -= DISPLAY_MAX_ITEMS;
		SetInitBit();
	} else if(kp_new == IR_RIGHT and begin+DISPLAY_MAX_ITEMS < LIGHT_LINE_NUMBER) {
		begin += DISPLAY_MAX_ITEMS;
		SetInitBit();
	} else if(kp_new == IR_UP and fastTimeRun) {
		getDateTime( false, +1 );
	} else if(kp_new == IR_DOWN and fastTimeRun) {
		getDateTime( false, -1 );
	}
}

void ImpDatiFotoperiodo( uint8_t channel ) {
	static bool confirm;
	static time_type luxOn, luxOff, luxFad, LuxFA, luxIT, luxTotal, luxMax;
	static uint16_t minsMaxFad, minsCurFad;
	static uint8_t SetupPage, step, indBase, curPhase;
	static uint8_t LimitecaseInf, LimitecaseSup;
	static uint8_t tempMaxHour, tempMaxMin;

	//================================= FIRST ENTRY BLOCK =================================
	if(CheckInitBit(true)) {						// Con questa if salvo le variabili interessate solo la prima volta che entro nell'impostazione
		indBase = (channel * 12)+NVRAM_START_ADDR;	// in modo da poter poi salvare nella eprom solo i dati variati con le if di conferma

		if(Plafo[channel].minsFA > MINUTES_PER_DAY) Plafo[channel].minsFA -= MINUTES_PER_DAY;
		if(Plafo[channel].minsIT > MINUTES_PER_DAY) Plafo[channel].minsIT -= MINUTES_PER_DAY;
		if(Plafo[channel].minsOff > MINUTES_PER_DAY) Plafo[channel].minsOff -= MINUTES_PER_DAY;

		luxOn.hour = getTimeHour(Plafo[channel].minsOn);
		luxOn.min = getTimeMin(Plafo[channel].minsOn);
		luxOff.hour = getTimeHour(Plafo[channel].minsOff);
		luxOff.min = getTimeMin(Plafo[channel].minsOff);
		LuxFA.hour = getTimeHour(Plafo[channel].minsFA);
		LuxFA.min = getTimeMin(Plafo[channel].minsFA);
		luxIT.hour = getTimeHour(Plafo[channel].minsIT);
		luxIT.min = getTimeMin(Plafo[channel].minsIT);
		luxFad.hour = getTimeHour(Plafo[channel].minsFad);
		luxFad.min = getTimeMin(Plafo[channel].minsFad);
		
		SetupPage = 1; 			// il valore di questa variabile determina il titolo della schermata
		LimitecaseInf = 1;		// Valore iniziale del case, si inizia con impostare l'accesione delle luci che conincide con l'inizio dell'alba: OraIA 
		LimitecaseSup = 4;     	// si finisce inizialmente con l'impostazione della variabile dei minuti di spegnimento del fotoperiodo che coincide con i minuti di fine tramonto: MinFT
		curPhase = 1;
		step = 5;     			// la variabile parte impostazione forza il valore del case al punto in cui si chiede di confermare i dati immessi se si preme ok
		confirm = true;			// inizialmente e' 5: siamo alla conferma dell'immisione dei dati di accensioe e spegniemtno delle luci.
		displayClear();
		sprintf(buff, "Set L%1d %s", channel+1, plafoNames[channel]);
		printStringCenter(buff, 0);
		printString(" On     Off    Len ", 0, 1);
		stampafrecce(0, 3, 0, 18);
	}
	
	//=================================== DISPLAY BLOCK ===================================
	if(SetupPage == 1)	{
		calcTotalLightTime(luxTotal, TimeInMinutes(luxOn), TimeInMinutes(luxOff));
		sprintf(buff, "%02d:%02d  %02d:%02d  %02d:%02d", luxOn.hour, luxOn.min, luxOff.hour, luxOff.min, luxTotal.hour, luxTotal.min);
		printString(buff, 0, 2);
	} else if(SetupPage == 2)	{
		calcMaxLightTime(luxMax, TimeInMinutes(luxOn), TimeInMinutes(luxOff), TimeInMinutes(luxFad));
		sprintf(buff, " %02d:%02d      %02d:%02d  ", luxFad.hour, luxFad.min, luxMax.hour, luxMax.min);
		printString(buff, 0, 2);
	}

	//================================ KEYBOARD/REMOTE BLOCK ===============================
	if(kp_new == IR_OK and confirm) {
		curPhase = step;
		confirm = false;
		kp_new = IR_NONE;		// avoid 'conferma' skipping
	} else if((kp_new == IR_RIGHT or kp_new == IR_LEFT) and confirm) {
		ScrollHandler(curPhase, LimitecaseInf, LimitecaseSup, kp_new==IR_RIGHT?ACT_INC:ACT_DEC);
		if(curPhase == LS_ON_HOUR) {
			stampafrecce(0, 3, 0, 18);
		} else if(curPhase == LS_ON_MIN) {
			stampafrecce(0, 3, 3, 15);		// 3 spaces before, 15 after at row 3 and column 0
		} else if(curPhase == LS_OFF_HOUR) {
			stampafrecce(0, 3, 7, 11);		// 14 spaces before, 4 after at row 3 and column 0
		} else if(curPhase == LS_OFF_MIN) {
			stampafrecce(0, 3, 10, 8);		// 17 spaces before, 1 after at row 3 and column 0
		} else if(curPhase == LS_FAD_HOUR or curPhase == LS_FAD_MIN) {		// this is executed only one time at cursor position change
			if(curPhase == LS_FAD_HOUR)	stampafrecce(0, 3, 1, 17);			// HOUR no spaces before, 18 after at row 3 and column 0
			if(curPhase == LS_FAD_MIN) stampafrecce(0, 3, 4, 14);			// MINS 3 spaces before, 15 after at row 3 and column 0
			minsMaxFad = TimeInMinutes(luxTotal)/2;
			minsCurFad = TimeInMinutes(luxFad);
			tempMaxHour = getTimeHour(minsMaxFad-getTimeMin(minsCurFad));
			tempMaxMin = min((minsMaxFad-minsCurFad), 59);
		}
	} else if(kp_new == IR_MENU) {
		SetInitBit(DS_SETUP_LIGHTS);
	}

	//================================ SETUP PHASES BLOCK =================================
	if(curPhase == LS_ON_HOUR) {
		if(kp_new == IR_UP) {
			ScrollHandler(luxOn.hour, 0, 23, ACT_INC);
		} else if(kp_new == IR_DOWN) {
			ScrollHandler(luxOn.hour, 0, 23, ACT_DEC);
		}
	} else if(curPhase == LS_ON_MIN) {
		if(kp_new == IR_UP) {
			ScrollHandler(luxOn.min, 0, 59, ACT_INC);
		} else if(kp_new == IR_DOWN) {
			ScrollHandler(luxOn.min, 0, 59, ACT_DEC);
		}
	} else if(curPhase == LS_OFF_HOUR) {
		if(kp_new == IR_UP) {
			ScrollHandler(luxOff.hour, 0, 23, ACT_INC);
		} else if(kp_new == IR_DOWN) {
			ScrollHandler(luxOff.hour, 0, 23, ACT_DEC);
		}
	} else if(curPhase == LS_OFF_MIN) {
		if(kp_new == IR_UP) {
			ScrollHandler(luxOff.min, 0, 59, ACT_INC);
		} else if(kp_new == IR_DOWN) {
			ScrollHandler(luxOff.min, 0, 59, ACT_DEC);
		}
	} else if(curPhase == LS_PAGE1_CONFIRM) {
		printBlinkingString(confirm_msg);
		if(!confirm) {	
			if(kp_new == IR_OK)	{		// alla seconda pressione del tasto ok aggiorno le variabili in memoria, ma solo quelle affettivamente cambiate
										// La variazione di questi valori comporta anche il ricalcolo a secondo dei casi delle variabili di inizio 
										// alba e tramonto quindi anche queste vanno ricalcolate e memorizzate. vedi ultime due if
										
				Plafo[channel].minsOn = TimeInMinutes(luxOn);
				Plafo[channel].minsOff = TimeInMinutes(luxOff);
				
				// aggiorno le variabili che mi servono per spostare l'immisione dati alla parte riguardante l'mmissione della durata dell'alba 
				SetupPage = 2;
				LimitecaseInf = 6;
				LimitecaseSup = 7;
				step = 8;
				curPhase = LimitecaseInf;
				confirm = true;
				printString("Fad time   Max light", 0, 1);
				stampafrecce(0, 3, 1, 17);
			} else {		 								// se non premo ok ma uno qualsiasi dei tasti torno alla modifica dei dati
				if(kp_new == IR_RIGHT || kp_new == IR_LEFT || kp_new == IR_UP || kp_new == IR_DOWN)	{	
					confirm = true;
					curPhase = LimitecaseInf;
				}
			}
		}
	} else if(curPhase == LS_FAD_HOUR) {
		if(kp_new == IR_UP) {
			ScrollHandler(luxFad.hour, 0, tempMaxHour, ACT_INC);
		} else if(kp_new == IR_DOWN) {
			ScrollHandler(luxFad.hour, 0, tempMaxHour, ACT_DEC);
		}
	} else if(curPhase == LS_FAD_MIN) {
		if(kp_new == IR_UP) {
			ScrollHandler(luxFad.min, 0, tempMaxMin, ACT_INC);
		} else if(kp_new == IR_DOWN) {
			ScrollHandler(luxFad.min, 0, tempMaxMin, ACT_DEC);
		}
	} else if(curPhase == LS_PAGE2_CONFIRM) {
		printBlinkingString(confirm_msg);
		if(!confirm) {	
			if(kp_new == IR_OK)	{
				
				Plafo[channel].minsFad = TimeInMinutes(luxFad);								//  Calcolo tempo di fading
				Plafo[channel].minsFA = Plafo[channel].minsOn + Plafo[channel].minsFad;		//  Calcolo ora e minuti di fine alba (FA)
				Plafo[channel].minsIT = Plafo[channel].minsOff - Plafo[channel].minsFad;	//  Calcolo ora e minuti di inizio tramonto (IT)
				
				if(Plafo[channel].minsFA < Plafo[channel].minsOn) Plafo[channel].minsFA += MINUTES_PER_DAY;
				if(Plafo[channel].minsIT < Plafo[channel].minsFA) Plafo[channel].minsIT += MINUTES_PER_DAY;
				if(Plafo[channel].minsOff < Plafo[channel].minsIT) Plafo[channel].minsOff += MINUTES_PER_DAY;
				
				updateStaticMemoryInt((indBase + PLAFO_MINS_ON), Plafo[channel].minsOn);
				updateStaticMemoryInt((indBase + PLAFO_MINS_FA),Plafo[channel].minsFA);
				updateStaticMemoryInt((indBase + PLAFO_MINS_IT),Plafo[channel].minsIT);
				updateStaticMemoryInt((indBase + PLAFO_MINS_OFF), Plafo[channel].minsOff);
				
				LoadLightStatus(channel);
				confirm = true;
				dstatus = DS_IDLE_INIT;
			}
		} else {	
			if(kp_new == IR_RIGHT || kp_new == IR_LEFT || kp_new == IR_UP || kp_new == IR_DOWN)	{	
				confirm = true;
				curPhase = LimitecaseInf;
			}
		}
	}
}

void ImpostaFunzLinee() {							// Impostazione del modo di funzionamento e della luminosita massima delle singole linee
	static bool confirm = true;
	static temp_data Temp[LIGHT_LINE_NUMBER];		// Serve come appoggio dei dati durante la loro impostazione, altrimenti il loop vedrebbe le variabili mentre cambiano ed impazzirebbe
	static uint8_t step, channel;
	char dbuff[4];

	if(kp_new == IR_MENU)	{	
		SetInitBit(DS_SETUP_LIGHTS);
		return;
	}

	if(CheckInitBit(true)) {										// Attivazione schermata ed inizializzazione delle variabili
		for(uint8_t line = 0; line < LIGHT_LINE_NUMBER; line++)	{		//Carico i dati in una struct di appoggio
			Temp[line].workingMode = Plafo[line].workingMode;
			Temp[line].pwmMax = Plafo[line].pwmMax;
		}
		channel = 0;
		step = 0;
		confirm = true;
		displayClear();												//Scrivo su display le cose fisse
		printString("Imp.Funz. e Lum.Max", 0, 0);
	}
	if(confirm) {	
		if(kp_new == IR_OK)	{	
			step = 3;
			confirm = false;
			kp_new = IR_NONE;										// avoid 'conferma' skipping
		} else if(kp_new == IR_RIGHT) {
			ScrollHandler(step, 0, 2, ACT_INC);
		} else if(kp_new == IR_LEFT) {
			ScrollHandler(step, 0, 2, ACT_DEC);
		}
	}
	
	switch(step) {	
		case 0:														// Scelta della linea da impostare
			printString("^             ", 6, 3);
			
			if(kp_new == IR_UP) {
				ScrollHandler(channel, 0, LIGHT_LINE_NUMBER-1, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(channel, 0, LIGHT_LINE_NUMBER-1, ACT_DEC);
			}
			break;

		case 1:														// Scelta del modo di funzionamento della linea selezionata al case 0
			printString("    ^^^       ", 6, 3);
			
			if(kp_new == IR_UP) {
				ScrollHandler(Temp[channel].workingMode, POWER_OFF, POWER_AUTO, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(Temp[channel].workingMode, POWER_OFF, POWER_AUTO, ACT_DEC);
			}
			break;

		case 2:														// Impostazione della luminosita' massima della linea impostata al case 0
			printString("          ^^^ ", 6, 3);

			if(kp_new == IR_UP) {
				ScrollHandler(Temp[channel].pwmMax, 0, 255, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(Temp[channel].pwmMax, 0, 255, ACT_DEC);
			}
			break;

		case 3:
			printBlinkingString(confirm_msg);
			if(kp_new == IR_OK) {	
				for(uint8_t line = 0; line < LIGHT_LINE_NUMBER; line++)	{		//Salvataggio dei dati solo se cambiati e caricamento degli stessi nella struct principale
					uint8_t indBase = (line * 12)+NVRAM_START_ADDR;
					if(Temp[line].workingMode != Plafo[line].workingMode) {	
						updateStaticMemory(indBase + PLAFO_MODE, Temp[line].workingMode);
						Plafo[line].workingMode = Temp[line].workingMode;
						if(Plafo[line].workingMode == POWER_AUTO) {
							LoadLightStatus(line);
						}
					}
					if(Temp[line].pwmMax != Plafo[line].pwmMax) {	
						updateStaticMemory(indBase + PLAFO_MAX_FADING, Temp[line].pwmMax);
						Plafo[line].pwmMax = Temp[line].pwmMax;
						Plafo[line].fadingTimer = myMillis();
					}
				}
				dstatus = DS_IDLE_INIT;
			}
			if(kp_new == IR_RIGHT || kp_new == IR_LEFT || kp_new == IR_UP || kp_new == IR_DOWN) {	
				step = 0;
				confirm = true;
				printSpaces(20, 0, 3);
			}
			break;
	}
	sprintf(buff, "Chan [%1d]:[%3s] [%3d]", channel+1, decodePowerState(dbuff, Temp[channel].workingMode), Temp[channel].pwmMax); 
	printString(buff, 0, 2);
}

void PwmLightsInit() {
	int menuitem;
	LightsMenu[0].id = 1;										// current menu item storage
	strcpy(LightsMenu[0].desc, "Lights setup");					// menu main title
	LightsMenu[1].id = DS_SETUP_LIGHTS_0;
	strcpy(LightsMenu[1].desc, "Funz/LMax Linee");

	for(int idx = 0; idx < LIGHT_LINE_NUMBER; idx++) {
		Plafo[idx].pwmPin = PIN_LIGHT_PWM_BASE+idx;				// set PWM output pin for plafo 1 to 6
		Plafo[idx].pwmValue = 0;								// set default PWM value to 0
		pinMode(Plafo[idx].pwmPin, OUTPUT); 					// set pin as output
		analogWrite(Plafo[idx].pwmPin, Plafo[idx].pwmValue);	// and preset to OFF
		
		LoadLightStatus(idx);									// load plafo line config from nvram

		menuitem = idx+2;
		LightsMenu[menuitem].id = DS_SETUP_LIGHTS+menuitem;
		if(plafoNames[idx] == NULL) {
			sprintf(LightsMenu[menuitem].desc, "Fotoperiodo L%1d", idx+1);
		} else {
			sprintf(LightsMenu[menuitem].desc, "L%1d %s", idx+1, plafoNames[idx]);
		}
	}
	DEBUG(F("Plafo OK\n"));
}