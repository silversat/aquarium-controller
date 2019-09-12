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

uint8_t ReadStaticMemory( int address ) {
	uint8_t ret = false;
	#ifdef USE_RTC_NVRAM
		ret = nvram.readByte(address);
	#else
		ret = EEPROM.read(address);
	#endif	
//	DEBUG(F("Reading: address=%d, data=%d\n"), address, ret);
	return ret;
}	

void WriteStaticMemory( int address, uint8_t data ) {
	#ifdef USE_RTC_NVRAM
		nvram.writeByte(address, data);
		delay(10);
	#else
		EEPROM.write(address, data);
	#endif
//	DEBUG(F("Writing: address=%d, data=%d\n"), address, data);
}

void UpdateStaticMemory( int address, uint8_t data ) {
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
	uint8_t high = ReadStaticMemory(address);
	uint8_t low = ReadStaticMemory(address+1);
//	DEBUG(F("Read int: address=%d, value=%d, high: %d, low: %d\n"), address, word(high,low), high, low);
	return word(high,low);
}

bool writeStaticMemoryInt( int address, int value ) {
	bool ret = false;
	if(value != readStaticMemoryInt(address)) {
		WriteStaticMemory(address, highByte(value));
		WriteStaticMemory(address+1, lowByte(value));
		ret = true;
//		DEBUG(F("Writing int: address=%d, value=%d, high: %d, low: %d\n"), address, value, highByte(value), lowByte(value));
	}
	return ret;
}

void NvRamInit() {
	#ifdef USE_RTC_NVRAM
		nvram.initialize();
		DEBUG(F("NV Ram module OK\n"));
	#else
		DEBUG(F("onboard EEPROM OK\n"));
	#endif	
}
