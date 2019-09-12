//
//	TEMPERATURE.h
//
OneWire oneWire(PIN_ONE_WIRE_BUS);				// define one-wire bus instance
DallasTemperature sensors(&oneWire);			// define temp sensors on the bus
DeviceAddress Termometro1, Termometro2;			// temp sensor addresses
float tmed;										// temp sensor read values
boolean Tsensor1, Tsensor2;						// found temp sensors during setup one-wire bus scan
unsigned long readTimer = 0;

extern void relais( byte num, boolean action );

float Tempacqua, Tempacquaset, Tempvecchia;
bool alrmsonoro = true;
bool alarm_cycle = false;
unsigned long alarm_millis;

//----- DEFCON settings -----
#define DEFCON_1	1		// the higher DEFCON (catastrofe!)
#define DEFCON_2	2		//
#define DEFCON_3	3		//
#define DEFCON_4	4		//
#define DEFCON_5	5		// the lowest DEFCON (all right)
int	defcon = DEFCON_5;


// Impostazione della temperatura dell'acqua, e sua memorizzazione in eeprom,
// potendo memorizzare in memoria solo byte (interi da 0 a 255) ho stabilito 40 gradi come valore massimo
// e incrementi di immissione di 0,5 gradi in modo che quando
// scrivo in memoria divido il valore acquisito per l'incremento in modo da ottenere l'intero da memorizzare,
// quando invece leggo il valore, lo rimoltiplico per l'incremento per ottenere nuovamente il valore impostato
// esempio imposto il valore a 27,5 gradi in memoria scrivo 55 ossia 27,5/0,5
// leggo il valore in memoria: 55, lo moltiplico per 0,5 ed ottengo 27,5 ossia il valore impostato
// se decidessimo per un incremento di 0,25 sarebbe la stessa cosa, ma mi e' sembrato eccessivo.	
//
void ImpostaTempAcqua() {  
	if(CheckInitBit(true)) {		// Leggo in memoria il valore impostato e predispongo la schermata del display
		Tempacquaset = Tempacqua;
		if(Tempacquaset > TEMP_ALLOWED_MAX or Tempacquaset < TEMP_ALLOWED_MIN) {
			Tempacquaset = TEMP_ALLOWED_DEFAULT;
		}
		Tempvecchia = Tempacquaset;
		displayClear();
		printStringCenter("Water Temp Setup", 0);
		printString("Temp.:", 0, 2);
	}

	printNumber(Tempacquaset, 6, 2);

	if(conferma) {
		stampafrecce(6, 3, 0, 1); 
		stampafrecce();

		switch(kp_new) {
			case IR_UP:	
				if(Tempacquaset < TEMP_ALLOWED_MAX) {
					Tempacquaset = Tempacquaset + 0.5;
				} else {
					buzerrore();
				}
				break;	

			case IR_DOWN:	
				if(Tempacquaset > TEMP_ALLOWED_MIN) {
					Tempacquaset = Tempacquaset - 0.5;
				} else {
					buzerrore();
				}
				break;	

			case IR_OK:
				conferma = false; // disattivo questa if in modo che Il tasto OK funzioni solo con la if di conferma definitiva
				break;

			case IR_MENU:
				dstatus = DS_IDLE_INIT;
				break;
		}
	} else {	
		printBlinkingString(confirm);
		if(kp_new == IR_OK or kp_new == IR_MENU)	{	
			if(kp_new == IR_OK and Tempvecchia != Tempacquaset)	{	
				WriteStaticMemory(SMEMORY_TEMP_ADDR, int(Tempacquaset / 0.5));
				Tempacqua = Tempacquaset;
			}
			conferma = true;
			dstatus = DS_IDLE_INIT;
		}

		if((kp_new == IR_RIGHT) || (kp_new == IR_LEFT) || (kp_new == IR_UP) || (kp_new == IR_DOWN)) {	
			printSpaces(20, 0, 3);
			conferma = true;
		}
	}
}

void MantenimentoTempAcqua () {
	if(tmed < Tempacqua) {
		relais(SR_RISCALDATORE, RL_ON); 
	} else {
		relais(SR_RISCALDATORE, RL_OFF);
	}
	
	if((tmed < (Tempacqua - TEMP_RANGE)) or (tmed > (Tempacqua + TEMP_RANGE))) {
		defcon = DEFCON_3;
	} else if(defcon == DEFCON_3) {
		defcon = DEFCON_4;
	}
	
	switch( defcon ) {
		case DEFCON_1:
			break;
			
		case DEFCON_2:
			break;
			
		case DEFCON_3:
			if(kp_new == IR_MENU and alrmsonoro) {		// if ESC key pressed, deactivate acoustic alarm.
				alrmsonoro = false;
				beepOff();								// stop alarm
			}
			if((millis() - alarm_millis > 500) or (millis() < alarm_millis)) {	
				alarm_millis = millis();
				alarm_cycle = !alarm_cycle;
			}
			if(dstatus == DS_IDLE) {
				if(alarm_cycle) {
					printNumber(tmed, 3, 2);
					printChar(0b011011111);
					if(alrmsonoro) {
						alarm(true, true);				// status = in alarm, beep ON
					}
				} else {
					printSpaces(7, 3, 2);
					alarm(true, false);					// status = in alarm, beep OFF
				}
			}
			break;
			
		case DEFCON_4:
			alrmsonoro = true;		// reset audible alarm
			beepOff();				// stop alarm
			defcon = DEFCON_5;		// restore DEFCON to 5
			break;
			
		case DEFCON_5:
			if(dstatus == DS_IDLE) {
				printNumber(tmed, 3, 2);
				printChar(0b011011111);
			}
			break;
	}
}

void GetTemperature() {
	float t1, t2;
	
	if (millis() - readTimer > TEMPO_LETTURA) {	
		readTimer = millis();
		sensors.requestTemperatures();
		
		if(Tsensor1) {
			t1 = sensors.getTempC(Termometro1);
//			DEBUG("T1=%s", ftoa(buff, t1));
		}	
			
		if(Tsensor2) {	
			t2 = sensors.getTempC(Termometro2);
//			DEBUG(", T2=%s", ftoa(buff, t2));
		}	
			
		if(Tsensor1 && Tsensor2) {
			tmed = (t1 + t2)/2;
		} else {
			if(Tsensor1) {
				tmed = t1;
			} else {
				tmed = 0;
			}
		}	
		if(tmed != 0) {
//			DEBUG(", Tmed=%s\n", ftoa(buff, tmed));
		}	
	}
}

void TempSensorsInit() {
	Tempacqua = ReadStaticMemory(SMEMORY_TEMP_ADDR) * 0.5;		// read desired water temp from nvram
	
	sensors.begin();
	Tsensor1 = sensors.getAddress(Termometro1, 0);		// get sensor 1 address, if present (TsensorX true if sensor present, TermometroX contains the address)
	Tsensor2 = sensors.getAddress(Termometro2, 1);		// same for sensor 2

	if(!Tsensor1) {
		printDeviceNotFound(1);
	}
	if(!Tsensor2) {
		printDeviceNotFound(2);
	}
	if(Tsensor1 && Tsensor2) {												// at least one sensor is present	
		sensors.setResolution(Termometro1, TEMP_SENSOR_RESOLUTION);			// configure it
		if(Termometro1 == Termometro2) {									// if only one sensor is present, both Termometro1 and Termometro2 have the same address
			Tsensor2 = false;												// if so, disable sensor 2
		} else {
			sensors.setResolution(Termometro2, TEMP_SENSOR_RESOLUTION);		// else configure it
		}
	}
	DEBUG("Temperature sensor OK\n");
}
