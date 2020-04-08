//
//	NVRam.h
//

#define USE_RTC_NVRAM			// use local board RTC-NVRAM or EEPROM (if not defined)

#ifdef USE_RTC_NVRAM
	#include <Eeprom24C32_64.h>
	#define I2C_ADDR_NVRAM		0x57
	static Eeprom24C32_64 nvram(I2C_ADDR_NVRAM);
#else
	#include <EEPROM.h>
#endif

uint8_t readStaticMemory( int address ) {
	uint8_t ret = false;
	#ifdef USE_RTC_NVRAM
		ret = nvram.readByte(address);
	#else
		ret = EEPROM.read(address);
	#endif	
//	DEBUG(F("Reading: address=%d, data=%d\n"), address, ret);
	return ret;
}	

void writeStaticMemory( int address, uint8_t data ) {
	#ifdef USE_RTC_NVRAM
		nvram.writeByte(address, data);
		delay(10);
	#else
		EEPROM.write(address, data);
	#endif
//	DEBUG(F("Writing: address=%d, data=%d\n"), address, data);
}

void updateStaticMemory( int address, uint8_t data ) {
	#ifdef USE_RTC_NVRAM
		uint8_t ret = nvram.readByte(address);
		if(data != ret) {
			nvram.writeByte(address, data);
			delay(10);
		}
	#else
		EEPROM.update(address, data);
	#endif
//	DEBUG(F("Updating: address=%d, data=%d\n"), address, data);
}

int readStaticMemoryInt( int address ) {
	uint8_t high = readStaticMemory(address);
	uint8_t low = readStaticMemory(address+1);
//	DEBUG(F("Read int: address=%d, value=%d, high: %d, low: %d\n"), address, word(high,low), high, low);
	return word(high,low);
}

bool writeStaticMemoryInt( int address, int value ) {
	bool ret = false;
	if(value != readStaticMemoryInt(address)) {
		writeStaticMemory(address, highByte(value));
		writeStaticMemory(address+1, lowByte(value));
		ret = true;
//		DEBUG(F("Writing int: address=%d, value=%d, high: %d, low: %d\n"), address, value, highByte(value), lowByte(value));
	}
	return ret;
}

bool updateStaticMemoryInt( int address, int value ) {
	bool ret = false;
	if(value != readStaticMemoryInt(address)) {
		updateStaticMemory(address, highByte(value));
		updateStaticMemory(address+1, lowByte(value));
		ret = true;
//		DEBUG(F("Writing int: address=%d, value=%d, high: %d, low: %d\n"), address, value, highByte(value), lowByte(value));
	}
	return ret;
}

template <class T> int NvramWriteAnything(int ee, const T& value) {
	const byte* p = (const byte*)(const void*)&value;
	int i;
	for(i = 0; i < sizeof(value); i++) {
		writeStaticMemory(ee++, *p++);
	}
	return i;
}

template <class T> int NvramReadAnything(int ee, T& value) {
	byte* p = (byte*)(void*)&value;
	int i;
	for(i = 0; i < sizeof(value); i++) {
		*p++ = readStaticMemory(ee++);
	}
	return i;
}

void NvRamInit() {
	#ifdef USE_RTC_NVRAM
		nvram.initialize();
		DEBUG(F("NV Ram module OK\n"));
	#else
		DEBUG(F("onboard EEPROM OK\n"));
	#endif	
}
