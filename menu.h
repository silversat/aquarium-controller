//
//	Menu.h
//

typedef struct {
	byte	id;
//	char*	desc;
	char	desc[20];
} menu_type;

menu_type MainMenu[] = {
	{1, "SETUP"},						// il primo valore contiene la voce di menu selezionata per ultima
	{DS_SETUP_INFOLIGHTS, "Info Luci"},
	{DS_SETUP_LIGHTS, "Imposta Luci"},
	{DS_SETUP_TEMP, "Temperatura"}, 
	{DS_SETUP_DATETIME, "Data/Ora"},
//	{DS_SETUP_SPARE_1, "Spare item 1"},
//	{DS_SETUP_SPARE_2, "Spare item 2"},
//	{DS_SETUP_SPARE_3, "Spare item 3"},
//	{DS_SETUP_SPARE_4, "Spare item 4"},
//	{DS_SETUP_SPARE_5, "Spare item 5"}
};

void arrayRotateLeft( int arr[], int size ) {
    int temp = arr[0]; 
    for(int i = 0; i < size-1; i++) {
        arr[i] = arr[i+1]; 				//move all element to the left except first one
    }
    arr[size-1] = temp; 				//assign remembered value to last element
}

void arrayRotateRight( int arr[], int size ) {
    int temp = arr[size-1]; 			//remember last element
    for(int i = size-1; i >= 0; i--) {
        arr[i+1] = arr[i]; 				//move all element to the right except last one
	}
	arr[0] = temp; 						//assign remembered value to first element
}

void ScorriMenu( menu_type arraymenu[], int size )  {
	uint8_t nItems = (size/sizeof(menu_type))-1;
//	uint8_t nRow = nItems/2;
	uint8_t nRow = DISPLAY_MAX_ROWS/2;
	static int arr[16];					// ???
	static boolean stampato;

	if(CheckInitBit( true )) {			// read and clear init bit
		displayClear();       			// Cancello il display e stampo il campo zero dell'array che contiene sempre l'intestazione del menu
		printStringCenter(arraymenu[0].desc, 0);
		printChar('>', 0, nRow);
		for(uint8_t i = 0; i <= nItems-1; i++) {
			arr[i] = i+1;
		}
		while(arr[nRow-1] != arraymenu[0].id) {
			arrayRotateRight(arr, nItems);
		}
		stampato = false;
	}
	if(kp_new == IR_UP) {	
		ScrollHandler(arraymenu[0].id, 1, nItems, ACT_DEC);
		arrayRotateRight(arr, nItems);
		stampato = false;
	}
	if(kp_new == IR_DOWN) {	
		ScrollHandler(arraymenu[0].id, 1, nItems, ACT_INC);
		arrayRotateLeft(arr, nItems);
		stampato = false;
	}
	if(kp_new == IR_OK)	{	
		dstatus = arraymenu[arraymenu[0].id].id;
		SetInitBit();
	}
	if(kp_new == IR_MENU) {
		SetInitBit(DS_IDLE);
	}
			
	// stampo le voci in base al tasto premuto
	if(!stampato) {	
		int startcol, itemlen;
		for(uint8_t i = 0; i < DISPLAY_MAX_ROWS-1; i++) {
			startcol = i<nRow?nRow-i:i-nRow+2;
			itemlen = strlen(arraymenu[arr[i]].desc);
			if(itemlen + startcol >= DISPLAY_MAX_COLS) {
				strncpy(buff, arraymenu[arr[i]].desc, DISPLAY_MAX_COLS);
				printString(buff, startcol, i+1);
			} else {
				printString(arraymenu[arr[i]].desc, startcol, i+1, false);
				printSpaces(DISPLAY_MAX_COLS-itemlen-startcol, false);
			}
		}
		printCommit();
		stampato = true;
	}
}
