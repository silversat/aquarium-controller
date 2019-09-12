
String spaces( byte num ) {
	String ret = "";
	for(int i = 0; i < num; i++) {
		ret += " ";
	}
	return ret;
}

void SetInitBit() {
	bitSet(dstatus, DS_INIT_BIT);
}
	
void SetInitBit( byte status ) {
	dstatus = status;
	bitSet(dstatus, DS_INIT_BIT);
}
	
//void ClearInitBit() {
//	bitClear(dstatus, DS_INIT_BIT);
//}
	
boolean CheckInitBit( boolean clearflag ) {		// chech init bit. if clearflag set, clear it too.
	boolean ret = bitRead(dstatus, DS_INIT_BIT);
	if(clearflag and ret)
	bitClear(dstatus, DS_INIT_BIT);
		
	return ret;
}

void printDeviceNotFound( int device ) {
	DEBUG("Temperature device %d not found\n", device); 
}

byte decToBcd(byte val) { 	// Da decimale a binario
	return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val) {	// Da binario a decimale
	return ( (val/16*10) + (val%16) );
}

void ScrollHandler( uint8_t &value, uint8_t Min, uint8_t Max, uint8_t action) {  // controllo tutte le variabili che devono ciclare in incremento o decremento
	switch(action)	{	
		case ACT_INC: 
			if(value < Max) {
				value++;
			} else {
				value = Min; 
			}
			break;
			
		case ACT_DEC:
			if(value > Min) {
				value--;
			} else {
				value = Max;
			}
			break;
	}
}

char* ftoa(char *a, double f) {
	long unit = (long)f;
	long decimal = abs((long)((f - unit) * 100));
	sprintf(a, "%02lu.%02lu", unit, decimal);
	return a;
}
