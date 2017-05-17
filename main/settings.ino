#include <EEPROM.h>

#include "config.h"

void writeSetting(uint16_t offset, char *value) {
	EEPROM.write(offset, EEPROM_MAGIC1);
	EEPROM.write(offset + 1, EEPROM_MAGIC2);

	for (uint16_t i = 0;;) {
		EEPROM.write(offset + i + 2, value[i]);
		if (value[i] == 0x00) break;
		++i;
	}
}

bool readSetting(uint16_t offset, char *target) {
	if (EEPROM.read(offset) != EEPROM_MAGIC1 || EEPROM.read(offset + 1) != EEPROM_MAGIC2)
		return false;

	for (uint16_t i = 0;;) {
		target[i] = EEPROM.read(offset + i + 2);
		if (target[i] == 0x00) break;
		++i;
	}

	return true;
}
