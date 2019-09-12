//
//	LIGHTS.h
//
#define	DISPLAY_MAX_ITEMS	DISPLAY_MAX_ROWS-1

typedef struct {					//Creo una struttura di variabili relative ad un Tasto
	uint8_t		hour;
	uint8_t 	min;
} time_type;

typedef	struct 	{					// I valori di Pos, vengono usati per determinare l'indirizzo di memoria in cui vengono registrati gli orari
	uint8_t 		Funzionamento;	// 0 se la linea e Off in manuale, 1 se la linea ON in manuale, 2 se la linea e' in AUT fuzionamento automatico
	uint8_t 		MaxFading;		// Contiene il valore di luminosita massima impostata
	uint8_t 		Fading;			// Valore di Fading corrente
	int				minsOn;
	int				minsOff;
	int				minsFad;
	int				minsFA;
	int				minsIT;
	boolean 		Alba;			// stato alba
	boolean 		Tramonto;		// stato tramonto
	boolean 		StatoPower;		// Tiene lo stato dell'alimentazione della linea
	unsigned long	DeltaFading;	// Usato per lo storage del intervallo in millis per l'incremento/decremento del fading
	unsigned long 	Tempoprec;		// Usato per lo storage di millis() durante l'esecuzione del fading
	uint8_t 		pwmPin;			// Contiene il numero di pin di controllo PWM delle singole linee
} PlafoData;

typedef struct {					//Creo una struttura di variabili relative ad un Tasto
	byte Funzionamento;
	byte MaxFading;
} temp_data;

//char*		plafoNames[] = {NULL, NULL, NULL, NULL, NULL, NULL};
char*		plafoNames[] = {PLAFO_NAME_1, PLAFO_NAME_2, PLAFO_NAME_3, PLAFO_NAME_4, PLAFO_NAME_5, PLAFO_NAME_6};
PlafoData 	Plafo[LIGHT_LINE_NUMBER];
menu_type 	LightsMenu[LIGHT_LINE_NUMBER+2];

int LucePiena( uint8_t oretotaliftp, uint8_t minutitotaliftp, uint8_t oretotalifad, uint8_t minutitotalifad )	{ 
	return int(oretotaliftp * 60 + minutitotaliftp) - int ((oretotalifad * 60 + minutitotalifad) * 2);
}

uint8_t getTimeHour( int timeInMins ) {
	return int(timeInMins / 60);
}

uint8_t getTimeMin( int timeInMins ) {
	return int(timeInMins % 60);
}

uint32_t TimeInSeconds( uint8_t hours, uint8_t mins ) {		//Converte l'orario in secondi (da 0 a 86399 sec) {
	return ((hours * 1440) + (mins * 60));
}

uint32_t TimeInSeconds( int mins ) {							//Converte l'orario in secondi (da 0 a 86399 sec) {
	return (mins * 60);
}

uint16_t TimeInMinutes( uint8_t hours, uint8_t mins ) {		//Converte l'orario in minuti
	return ((hours * 60) + mins);
}

uint16_t TimeInMinutes( time_type time ) {
	return ((time.hour * 60) + time.min);
}

void calcTotalLightTime( time_type &lux, int minsOn, int minsOff ) {
	int total = minsOff - minsOn;
	if(total < 0) total += NINUTES_PER_DAY;		// 24 * 60 = 1440 (NINUTES_PER_DAY)
	lux.hour = getTimeHour(total);
	lux.min = getTimeMin(total);
}

void calcMaxLightTime( time_type &lux, int minsOn, int minsOff, int minsFad ) {
	int max = minsOff - minsOn;
	if(max < 0) max += NINUTES_PER_DAY;		// 24 * 60 = 1440 (NINUTES_PER_DAY)
	max -= (minsFad * 2);
	lux.hour = getTimeHour(max);
	lux.min = getTimeMin(max);
}

uint8_t calcLuxAverage() {
	uint8_t luxmed = 0;
	for(int i = 0; i < LIGHT_LINE_NUMBER; i++) {
		luxmed += Plafo[i].Fading;
	}
	luxmed = ((luxmed / LIGHT_LINE_NUMBER) * 100) / 256;
	return luxmed;
}

char *decodeMode( char* buff, uint8_t mode ) {
	if(mode == LIGHT_MODE_OFF) {
		strcpy(buff, "Off");
	} else	if(mode == LIGHT_MODE_ON) {
		strcpy(buff, "On ");
	} else if(mode == LIGHT_MODE_AUTO) {
		strcpy(buff, "Aut");
	}
	return buff;
}

void LoadLightStatus( uint8_t channel )	{	
	static unsigned long Temporeale, Accensione, Spegnimento, Finealba, Iniziotramonto;
	uint8_t baseAddr = (channel * 12);

	// Questa funzione viene eseguita in caso di riavvio di arduino per reset, per mancanza di luce o in caso di variazione degli orari dei 
	// fotoperiodi, in pratica carica nella struct i dati e calcola in caso di funzionamento automatico i dati necessari all'esecuzione del
	// del fading in particolare lo stato del fading al momento del ripristino, l'intevrello tra un passaggio di rampa ed un'altro, lo stato
	// delle linee di alimentazione, se e' in corso l'alba il tramonto, se c'e' luce piena e se e' tutto spento.
	// Viene eseguita una sola volta ed i dati calcolati o settati vengono poi usati dalla funzione GestioneLuci () per eseguire 
	// semplici confronti con i millis alleggerendo cosi il lavoro di Arduino.

	Plafo[channel].Funzionamento = ReadStaticMemory(baseAddr + PLAFO_MODE);
	Plafo[channel].MaxFading = ReadStaticMemory(baseAddr + PLAFO_MAX_FADING);
	Plafo[channel].minsOn = readStaticMemoryInt(baseAddr + PLAFO_MINS_ON);
	Plafo[channel].minsOff = readStaticMemoryInt(baseAddr + PLAFO_MINS_OFF);
	Plafo[channel].minsFad = readStaticMemoryInt(baseAddr + PLAFO_MINS_FAD);
	Plafo[channel].minsFA = readStaticMemoryInt(baseAddr + PLAFO_MINS_FA);
	Plafo[channel].minsIT = readStaticMemoryInt(baseAddr + PLAFO_MINS_IT);

	if(Plafo[channel].Funzionamento > LIGHT_MODE_AUTO) {
		Plafo[channel].Funzionamento = LIGHT_MODE_AUTO;
		writeStaticMemoryInt(baseAddr + PLAFO_MODE, LIGHT_MODE_AUTO);
	}
	if(Plafo[channel].minsOn > NINUTES_PER_DAY) {
		Plafo[channel].minsOn = 0;
		writeStaticMemoryInt(baseAddr + PLAFO_MINS_ON, 0);
	}
	if(Plafo[channel].minsOff > NINUTES_PER_DAY) {
		Plafo[channel].minsOff = 0;
		writeStaticMemoryInt(baseAddr + PLAFO_MINS_OFF, 0);
	}
	if(Plafo[channel].minsFad > NINUTES_PER_DAY) {
		Plafo[channel].minsFad = 0;
		writeStaticMemoryInt(baseAddr + PLAFO_MINS_FAD, 0);
	}
	if(Plafo[channel].minsFA > NINUTES_PER_DAY) {
		Plafo[channel].minsFA = 0;
		writeStaticMemoryInt(baseAddr + PLAFO_MINS_FA, 0);
	}
	if(Plafo[channel].minsIT > NINUTES_PER_DAY) {
		Plafo[channel].minsIT = 0;
		writeStaticMemoryInt(baseAddr + PLAFO_MINS_IT, 0);
	}

	if(Plafo[channel].Funzionamento >= LIGHT_MODE_AUTO) {		// 0xFF is virgin eeprom data

		// Calcolo degli orari di avvenimento degli eventi del fotoperiodo, in funzione dgli orari impostati 
		// ed in relazione al momento in cui avvengo nel corso delle 24 ore 

		Temporeale = TimeInSeconds(datetime.hour(), datetime.minute()) + datetime.second();
		Accensione = TimeInSeconds(Plafo[channel].minsOn);
		Spegnimento = TimeInSeconds(Plafo[channel].minsOff);
		Finealba = TimeInSeconds(Plafo[channel].minsFA);
		Iniziotramonto = TimeInSeconds(Plafo[channel].minsIT);

		/* Trasformazione degli orari calcolati sopra, in forma di tappe orarie in cui avvengono gli eventi, ripetto al momento di accensione
		p.e. se alba inizia alle 10,00, il fading dura 2,00 ore e lo spegnimento avviene alle 22,00 le variabili diventeranno cosi:
		Accensione = 10,00			Rimane uguale
		Finealba = 2,00				Avviene a 2,00 ore dall'accensione e corrisponde esattamente alla durata del fading ;-)
		Iniziotramonto = 10,00		Avviene a 8,00 ore dall'accensione
		Spegnimento = 12,00 		Avviene a 10,00 ore dall'accensione */

		if(Temporeale < Accensione) {
			Temporeale = 86400 + Temporeale - Accensione;
		} else {
			Temporeale = Temporeale - Accensione;
		}
		if(Spegnimento < Accensione) {
			Spegnimento = 86400 + Spegnimento - Accensione;
		} else {
			Spegnimento = Spegnimento - Accensione;
		}
		Iniziotramonto = Spegnimento - Finealba; // Finealba corrisponde alla durata del Fading

		// Calcolo dell'intervallo temporale tra l'incremento/decremento delle rampe durante alba/tramonto, espresso in millesimi di secondo
		Plafo[channel].DeltaFading = (TimeInSeconds(Plafo[channel].minsFad) * 1000) / Plafo[channel].MaxFading;

		// Inizio dei controlli per scoprire in base all'ora attuale in quale momento del fotoperiodo della linea in questione mi trovo...

		if(Temporeale > Spegnimento) {					// Se le luci sono spente
			Plafo[channel].Alba = false;
			Plafo[channel].Tramonto = false;
			Plafo[channel].StatoPower = false;
			analogWrite(Plafo[channel].pwmPin, 0x00);	// Azzero il fading
		} else {										// Se l'ora attuale e' all'interno del periodo di alba
			if(Temporeale < Finealba) {	
				Plafo[channel].Alba = true;
				Plafo[channel].Tramonto = false;
				Plafo[channel].Fading = (Temporeale * 1000) / Plafo[channel].DeltaFading; // Calcolo il valore di fading in base a Temporeale
			} else {									// Se Temporeale e' all'interno del periodo di luce piena
				if(Temporeale < Iniziotramonto) {	
					Plafo[channel].Alba = false;
					Plafo[channel].Tramonto = false;
					Plafo[channel].Fading = Plafo[channel].MaxFading;
				} else {								// Se temporeale e' all'interno del periodo di tramonto
					Plafo[channel].Alba = false;
					Plafo[channel].Tramonto = true;
					Plafo[channel].Fading = Plafo[channel].MaxFading - (((Temporeale - Iniziotramonto) * 1000) / Plafo[channel].DeltaFading); // Calcolo il valore di fading in base a Temporeale
				}
			}
			// Comandi fissi se le luci non sono spente
			analogWrite(Plafo[channel].pwmPin, Plafo[channel].Fading);	// Setto la luminosita della linea in base al fading calcolato
			Plafo[channel].Tempoprec = millis();						// Valore necessario in gestione luci per scandire il fading
			Plafo[channel].StatoPower = true;							// Flag per la verifica dello stato di alimentazione della linea
		}
	}
}

void GestioneLuci()	{
	for(int channel = 0; channel < LIGHT_LINE_NUMBER; channel++) {
		if(Plafo[channel].Funzionamento == LIGHT_MODE_OFF) {
			if(Plafo[channel].Fading == 0)	{		// Se sono arrivato al fading minimo e sono in OFF manuale disattivo lo switch
				Plafo[channel].StatoPower = false;
			} else {	
				if(((millis() - Plafo[channel].Tempoprec) >= 110) && (Plafo[channel].Fading > 0)) {	
					Plafo[channel].Tempoprec = Plafo[channel].Tempoprec + 110;
					Plafo[channel].Fading --;
					analogWrite(Plafo[channel].pwmPin,Plafo[channel].Fading);
				}
			}
		} else if(Plafo[channel].Funzionamento == LIGHT_MODE_ON) {
			if(Plafo[channel].Fading != Plafo[channel].MaxFading)	{	// Se sono arrivato al fading massimo e sono in ON manuale disattivo lo switch
				Plafo[channel].StatoPower = true;
				if(((millis() - Plafo[channel].Tempoprec) >= 110) && (Plafo[channel].Fading < Plafo[channel].MaxFading)) {	
					Plafo[channel].Tempoprec = Plafo[channel].Tempoprec + 110;
					Plafo[channel].Fading ++;
					analogWrite(Plafo[channel].pwmPin,Plafo[channel].Fading);
				}
			}
		} else if(Plafo[channel].Funzionamento == LIGHT_MODE_AUTO) {
			if(Plafo[channel].Alba == true) {
				if((millis() - Plafo[channel].Tempoprec) >= Plafo[channel].DeltaFading) {	
					Plafo[channel].Tempoprec = Plafo[channel].Tempoprec + Plafo[channel].DeltaFading;
					if(Plafo[channel].Fading < Plafo[channel].MaxFading) {	
						Plafo[channel].Fading += 1; 
						analogWrite(Plafo[channel].pwmPin, Plafo[channel].Fading);
					} else {
						Plafo[channel].Alba = false; 
					}	
				}
			} else if(Plafo[channel].Tramonto == true) {
				if((millis() - Plafo[channel].Tempoprec) >= Plafo[channel].DeltaFading)	{	
					Plafo[channel].Tempoprec = Plafo[channel].Tempoprec + Plafo[channel].DeltaFading;
					if(Plafo[channel].Fading > 0) {
						Plafo[channel].Fading -= 1; 
						analogWrite(Plafo[channel].pwmPin, Plafo[channel].Fading);
					} else {	
						Plafo[channel].Tramonto = false; 
						Plafo[channel].StatoPower = false;
					}
				}
			} else {
				if(Plafo[channel].minsOn >= TimeInMinutes(datetime.hour(), datetime.minute()) and Plafo[channel].Alba == false)	{	
					Plafo[channel].Alba = true; 
					Plafo[channel].Tempoprec = millis();
					Plafo[channel].Fading = 0;
					Plafo[channel].StatoPower = true;
				} else if(Plafo[channel].minsIT >= TimeInMinutes(datetime.hour(), datetime.minute()) and Plafo[channel].Tramonto == false) {	
					Plafo[channel].Tramonto = true; 
					Plafo[channel].Tempoprec = millis();
					Plafo[channel].Fading = Plafo[channel].MaxFading;
				}
			}
		}
	}
}

void InfoLuci() {
	static uint8_t begin = 0;
	char buff[24];

	if(CheckInitBit(true)) {		// Con questa if salvo le variabili interessate solo la prima volta che entro nell'impostazione
  		printStringCenter("Info Luci", 0);
		if(LIGHT_LINE_NUMBER > DISPLAY_MAX_ITEMS) {
			if(begin+DISPLAY_MAX_ITEMS < LIGHT_LINE_NUMBER) {
				printString(">>", DISPLAY_MAX_COLS-2, 0);
			}
			if(begin > 0)  {
				printString("<<", 0, 0);
			}
		}
	}
	
	for(byte channel = begin; channel < (begin + DISPLAY_MAX_ITEMS); channel++) {
		if(channel < LIGHT_LINE_NUMBER) {
			int perc = (Plafo[channel].Fading * 100) / Plafo[channel].MaxFading;
			sprintf(buff, "L%d: %s Fad:%3d%% %s", channel+1, Plafo[channel].StatoPower?"On ":"Off", perc, Plafo[channel].Funzionamento == LIGHT_MODE_AUTO?"Aut":"Man");
			printString(buff, 0, channel-begin+1);
		} else {
			displayClearRow(channel-begin+1);
		}
	}
	
	if(kp_new == IR_OK or kp_new == IR_MENU) {
		SetInitBit(DS_SETUP);
	} else if((kp_new == IR_UP or kp_new == IR_LEFT) and begin > 0) {
		begin -= DISPLAY_MAX_ITEMS;
		SetInitBit();
	} else if((kp_new == IR_DOWN or kp_new == IR_RIGHT) and begin+DISPLAY_MAX_ITEMS < LIGHT_LINE_NUMBER) {
		begin += DISPLAY_MAX_ITEMS;
		SetInitBit();
	}
}

void ImpDatiFotoperiodo( uint8_t channel ) {
	static time_type luxOn, luxOff, luxFad, LuxFA, luxIT, luxTotal, luxMax;
	static uint8_t SetupPage, step, indBase, DatoFotoperiodo;
	static uint8_t LimitecaseInf, LimitecaseSup;

	if(CheckInitBit(true)) {			// Con questa if salvo le variabili interessate solo la prima volta che entro nell'impostazione
		indBase = (channel * 12);		// in modo da poter poi salvare nella eprom solo i dati variati con le if di conferma

		luxOn.hour = getTimeHour(Plafo[channel].minsOn);
		luxOn.min = getTimeMin(Plafo[channel].minsOn);
		luxOff.hour = getTimeHour(Plafo[channel].minsOff);
		luxOff.min = getTimeMin(Plafo[channel].minsOff);
		luxFad.hour = getTimeHour(Plafo[channel].minsFad);
		luxFad.min = getTimeMin(Plafo[channel].minsFad);
		LuxFA.hour = getTimeHour(Plafo[channel].minsFA);
		LuxFA.min = getTimeMin(Plafo[channel].minsFA);
		luxIT.hour = getTimeHour(Plafo[channel].minsIT);
		luxIT.min = getTimeMin(Plafo[channel].minsIT);
		
		SetupPage = 1; 			// il valore di questa variabile determina il titolo della schermata
		LimitecaseInf = 1;		// Valore iniziale del case, si inizia con impostare l'accesione delle luci che conincide con l'inizio dell'alba: OraIA 
		LimitecaseSup = 4;     	// si finisce inizialmente con l'impostazione della variabile dei minuti di spegnimento del fotoperiodo che coincide con i minuti di fine tramonto: MinFT
		DatoFotoperiodo = 1;
		step = 5;     			// la variabile parte impostazione forza il valore del case al punto in cui si chiede di confermare i dati immessi se si preme ok
		conferma = true;        // inizialmente e' 5: siamo alla conferma dell'immisione dei dati di accensioe e spegniemtno delle luci.
		displayClear();
		sprintf(buff, "Set L%1d %s", channel+1, plafoNames[channel]);
		printStringCenter(buff, 0);
	}
	
	if(SetupPage == 1)	{
		calcTotalLightTime(luxTotal, TimeInMinutes(luxOn), TimeInMinutes(luxOff));
		printString(" On     Off    Len ", 0, 1);
		sprintf(buff, "%02d:%02d  %02d:%02d  %02d:%02d", luxOn.hour, luxOn.min, luxOff.hour, luxOff.min, luxTotal.hour, luxTotal.min);
		printString(buff, 0, 2);
	} else if(SetupPage == 2)	{
		calcMaxLightTime(luxMax, TimeInMinutes(luxOn), TimeInMinutes(luxOff), TimeInMinutes(luxFad));
		printString("Fad time   Max light", 0, 1);
		sprintf(buff, " %02d:%02d      %02d:%02d  ", luxFad.hour, luxFad.min, luxMax.hour, luxMax.min);
		printString(buff, 0, 2);
	}

	switch(kp_new) {
		case IR_OK:
			if(conferma) {
				DatoFotoperiodo = step;
				conferma = false;
				kp_new = IR_NONE;		// avoid 'conferma' skipping
			}	
			break;

		case IR_RIGHT:
			if(conferma) {
				ScrollHandler(DatoFotoperiodo, LimitecaseInf, LimitecaseSup, ACT_INC);
			}
			break;

		case IR_LEFT:
			if(conferma) {
				ScrollHandler(DatoFotoperiodo, LimitecaseInf, LimitecaseSup, ACT_DEC);
			}
			break;

		case IR_MENU:
			SetInitBit(DS_SETUP_LIGHTS);
			break;
	}		

	switch(DatoFotoperiodo) {	
		case 1:        										// immissione dell'ora di accensione, coincide con Ora Inizio Alba OraIA
			stampafrecce(0, 3, 0, 18);

			if(kp_new == IR_UP) {
				ScrollHandler(luxOn.hour, 0, 23, ACT_INC);
			}
			if(kp_new == IR_DOWN) {
				ScrollHandler(luxOn.hour, 0, 23, ACT_DEC);
			}
			break;							// Chiude case 1 di DatoFotoperiodo per impostazione OraOn

		case 2:								// immissione dei minuti dell'ora di accensione, coincide con Ora Inizio Alba OraIA
			stampafrecce(0, 3, 3, 15);		// 3 spaces before, 15 after at row 3 and column 0

			if(kp_new == IR_UP) {
				ScrollHandler(luxOn.min, 0, 59, ACT_INC);
			}
			if(kp_new == IR_DOWN) {
				ScrollHandler(luxOn.min, 0, 59, ACT_DEC);
			}
			break; // Chiude case 2 di DatoFotoperiodo per impostazione MinIA 

		case 3:
			stampafrecce(0, 3, 7, 11);		// 14 spaces before, 4 after at row 3 and column 0

			if(kp_new == IR_UP) {
				ScrollHandler(luxOff.hour, 0, 23, ACT_INC);
			}
			if(kp_new == IR_DOWN) {
				ScrollHandler(luxOff.hour, 0, 23, ACT_DEC);
			}
			break; // Chiude case 3 di DatoFotoperiodo per impostazione OraTFtp   

		case 4:
			stampafrecce(0, 3, 10, 8);		// 17 spaces before, 1 after at row 3 and column 0

			if(kp_new == IR_UP) {
				ScrollHandler(luxOff.min, 0, 59, ACT_INC);
			}
			if(kp_new == IR_DOWN) {
				ScrollHandler(luxOff.min, 0, 59, ACT_DEC);
			}
			break; // Chiude case 4 di DatoFotoperiodo per impostazione MinTFtp   

		case 5:
			printBlinkingString(confirm);
			if(!conferma) {	
				if(kp_new == IR_OK)	{		// alla seconda pressione del tasto ok aggiorno le variabili in memoria, ma solo quelle affettivamente cambiate
											// La variazione di questi valori comporta anche il ricalcolo a secondo dei casi delle variabili di inizio 
											// alba e tramonto quindi anche queste vanno ricalcolate e memorizzate. vedi ultime due if
											
					Plafo[channel].minsOn = TimeInMinutes(luxOn);
					Plafo[channel].minsOff = TimeInMinutes(luxOff);
					writeStaticMemoryInt((indBase + PLAFO_MINS_ON), Plafo[channel].minsOn);
					writeStaticMemoryInt((indBase + PLAFO_MINS_OFF), Plafo[channel].minsOff);
					
					LoadLightStatus(channel);
					
					// aggiorno le variabili che mi servono per spostare l'immisione dati alla parte riguardante l'mmissione della durata dell'alba 
					SetupPage = 2;
					LimitecaseInf = 6;
					LimitecaseSup = 7;
					step = 8;
					DatoFotoperiodo = LimitecaseInf;
					conferma = true;
					
					// prepare values for next page 
					int minsMaxFad = TimeInMinutes(luxTotal) / 2;
					luxTotal.hour = getTimeHour(minsMaxFad);
					luxTotal.min = getTimeMin(minsMaxFad);
				} else {		 // se non premo ok ma uno qualsiasi dei tasti torno alla modifica dei dati
					if(kp_new == IR_RIGHT || kp_new == IR_LEFT || kp_new == IR_UP || kp_new == IR_DOWN)	{	
						conferma = true;
						DatoFotoperiodo = LimitecaseInf;
					}
				}
			}
			break; // Chiude case 5 di DatoFotoperiodo per registrazione dati in memoria 

		case 6:
			stampafrecce(0, 3, 1, 17);		// no spaces before, 18 after at row 3 and column 0

			if(kp_new == IR_UP) {
				ScrollHandler(luxFad.hour, 0, luxTotal.hour, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(luxFad.hour, 0, luxTotal.hour, ACT_DEC);
			}
			break; // Chiude case 6 per impostazione Ore fading

		case 7: 
			stampafrecce(0, 3, 4, 14);		// 3 spaces before, 15 after at row 3 and column 0

			if(kp_new == IR_UP) {
				ScrollHandler(luxFad.min, 0, luxTotal.min, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(luxFad.min, 0, luxTotal.min, ACT_DEC);
			}
			break; // Chiude case 7 per impostazione di minuti di fading

		case 8:
			printBlinkingString(confirm);
			if(!conferma) {	
				if(kp_new == IR_OK)	{	
					Plafo[channel].minsFad = TimeInMinutes(luxFad);
					writeStaticMemoryInt((indBase + PLAFO_MINS_FAD), Plafo[channel].minsFad);

					//  Calcolo ora e minuti di fine alba	(FA)
					Plafo[channel].minsFA = Plafo[channel].minsOn + Plafo[channel].minsFad;
					if(Plafo[channel].minsFA > NINUTES_PER_DAY) Plafo[channel].minsFA -= NINUTES_PER_DAY;

					//  Calcolo ora e minuti di inizio tramonto (IT)
					Plafo[channel].minsIT = Plafo[channel].minsOff - Plafo[channel].minsFad;
					if(Plafo[channel].minsIT < 0) Plafo[channel].minsIT += NINUTES_PER_DAY;

					writeStaticMemoryInt((indBase + PLAFO_MINS_FA),Plafo[channel].minsFA);
					writeStaticMemoryInt((indBase + PLAFO_MINS_IT),Plafo[channel].minsIT);

					LoadLightStatus(channel);
					conferma = true;
					dstatus = DS_IDLE_INIT;
				}
			} else {	
				if(kp_new == IR_RIGHT || kp_new == IR_LEFT || kp_new == IR_UP || kp_new == IR_DOWN)	{	
					conferma = true;
					DatoFotoperiodo = LimitecaseInf;
				}
			}
			break; // Chiude case 8 di DatoFotoperiodo per registrazione dati in memoria 
	}
}

void ImpostaFunzLinee() {							// Impostazione del modo di funzionamento e della luminosita massima delle singole linee
	static temp_data Temp[LIGHT_LINE_NUMBER];		// Serve come appoggio dei dati durante la loro impostazione, altrimenti il loop vedrebbe le variabili mentre cambiano ed impazzirebbe
	static uint8_t step, channel;
	char dbuff[4];

	if(kp_new == IR_MENU)	{	
		SetInitBit(DS_SETUP_LIGHTS);
		return;
	}

	if(CheckInitBit(true)) {										// Attivazione schermata ed inizializzazione delle variabili
		for(uint8_t line = 0; line < LIGHT_LINE_NUMBER; line++)	{		//Carico i dati in una struct di appoggio
			Temp[line].Funzionamento = Plafo[line].Funzionamento;
			Temp[line].MaxFading = Plafo[line].MaxFading;
		}
		channel = 0;
		step = 0;
		conferma = true;
		displayClear();												//Scrivo su display le cose fisse
		printString("Imp.Funz. e Lum.Max", 0, 0);
	}
	if(conferma) {	
		if(kp_new == IR_OK)	{	
			step = 3;
			conferma = false;
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
				ScrollHandler(Temp[channel].Funzionamento, LIGHT_MODE_OFF, LIGHT_MODE_AUTO, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(Temp[channel].Funzionamento, LIGHT_MODE_OFF, LIGHT_MODE_AUTO, ACT_DEC);
			}
			break;

		case 2:														// Impostazione della luminosita' massima della linea impostata al case 0
			printString("          ^^^ ", 6, 3);

			if(kp_new == IR_UP) {
				ScrollHandler(Temp[channel].MaxFading, 0, 255, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler(Temp[channel].MaxFading, 0, 255, ACT_DEC);
			}
			break;

		case 3:
			printBlinkingString(confirm);
			if(kp_new == IR_OK) {	
				for(uint8_t line = 0; line < LIGHT_LINE_NUMBER; line++)	{		//Salvataggio dei dati solo se cambiati e caricamento degli stessi nella struct principale
					uint8_t indBase = (line * 12);
					if(Temp[line].Funzionamento != Plafo[line].Funzionamento) {	
						UpdateStaticMemory(indBase + PLAFO_MODE, Temp[line].Funzionamento);
						Plafo[line].Funzionamento = Temp[line].Funzionamento;
						if(Plafo[line].Funzionamento == LIGHT_MODE_AUTO) {
							LoadLightStatus(line);
						}
					}
					if(Temp[line].MaxFading != Plafo[line].MaxFading) {	
						UpdateStaticMemory(indBase + PLAFO_MAX_FADING, Temp[line].MaxFading);
						Plafo[line].MaxFading = Temp[line].MaxFading;
						Plafo[line].Tempoprec = millis();
					}
				}
				dstatus = DS_IDLE_INIT;
			}
			if(kp_new == IR_RIGHT || kp_new == IR_LEFT || kp_new == IR_UP || kp_new == IR_DOWN) {	
				step = 0;
				conferma = true;
				printSpaces(20, 0, 3);
			}
			break;
	}
	sprintf(buff, "Chan [%1d]:[%3s] [%3d]", channel+1, decodeMode(dbuff, Temp[channel].Funzionamento), Temp[channel].MaxFading); 
	printString(buff, 0, 2);
}

void PwmLightsInit() {
	LightsMenu[0].id = 1;									// current menu item storage
	strcpy(LightsMenu[0].desc, "Lights setup");				// menu main title
	LightsMenu[1].id = DS_SETUP_LIGHTS_0;
	strcpy(LightsMenu[1].desc, "Funz/LMax Linee");

	for(int idx = 0; idx < LIGHT_LINE_NUMBER; idx++) {
		int menuitem = idx+2;
		
		Plafo[idx].pwmPin = PIN_LIGHT_PWM_BASE+idx;			// set PWM output pin for plafo 1 to 6
		pinMode(Plafo[idx].pwmPin, OUTPUT); 				// set pin as output
		analogWrite(Plafo[idx].pwmPin, 0);					// and preset to OFF
		
		LightsMenu[menuitem].id = DS_SETUP_LIGHTS+menuitem;
		if(plafoNames[idx] == NULL) {
			sprintf(LightsMenu[menuitem].desc, "Fotoperiodo L%1d", idx+1);
		} else {
			sprintf(LightsMenu[menuitem].desc, "L%1d %s", idx+1, plafoNames[idx]);
		}
	}
	DEBUG("Plafo OK\n");
}