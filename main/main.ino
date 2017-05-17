#include <EtherSia.h>
#include <Arduino.h>
#include <string.h>

#include "config.h"
#include "ntp.h"

EtherSia_W5100 ether(W5100_CSPIN);
HTTPServer http(ether);
UDPSocket ntpsocket(ether);

// Frequency is approx. 2000Hz, duration in ms
void sound(uint16_t duration) {
	for (uint16_t i = 0; i < duration * 2; ++i) {
		digitalWrite(PIEZO_PIN, HIGH);
		delayMicroseconds(250);
		digitalWrite(PIEZO_PIN, LOW);
		delayMicroseconds(250);
	}
}

#if 0
void printHash(uint8_t hash[32]) {
	for (uint8_t i = 0; i < 32; ++i)
		SERIALPORT.print(hash[i], HEX);
	SERIALPORT.println("\r\n");
}
#endif

bool checkKey(String key) {
	#define HASHLEN 32

	if (key.length() != HASHLEN * 2) return false;
	SERIALPORT.println("Checking key: " + key);

	// Retrieve password from EEPROM or use default one
	char passwd[EEPROM_PASSWORD_MAXLEN];
	if (!readSetting(EEPROM_OFFSET_PASSWORD, passwd))
		strcpy(passwd, DEFAULT_PASSWORD);

	// Convert key from HEX to binary format
	uint8_t key_binary[HASHLEN];
	for (uint8_t i = 0; i < HASHLEN; ++i)
		key_binary[i] = strtol(key.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);

	// Round NTP echo to time slots with duration AUTH_TIMEFRAME
	unsigned long current_hashtime = (epoch() / AUTH_TIMEFRAME) * AUTH_TIMEFRAME;
	unsigned long last_hashtime = (epoch() / AUTH_TIMEFRAME - 1) * AUTH_TIMEFRAME;
	unsigned long next_hashtime = (epoch() / AUTH_TIMEFRAME + 1) * AUTH_TIMEFRAME;

	// Generate strings to hash: Shared password with time as decimal string concatenated
	String current_hashstring = String(passwd) + String(current_hashtime);
	String last_hashstring = String(passwd) + String(last_hashtime);
	String next_hashstring = String(passwd) + String(next_hashtime);

	// Check hashes of current, previous and next authentication timeframe; all are considered equally valid
	uint8_t hash[HASHLEN];
	FIPS202_SHA3_256(current_hashstring.c_str(), current_hashstring.length(), hash);
	if (memcmp(hash, key_binary, HASHLEN) == 0) return true;
	FIPS202_SHA3_256(last_hashstring.c_str(), last_hashstring.length(), hash);
	if (memcmp(hash, key_binary, HASHLEN) == 0) return true;
	FIPS202_SHA3_256(next_hashstring.c_str(), next_hashstring.length(), hash);
	return memcmp(hash, key_binary, HASHLEN) == 0;
}

void setup() {
	MACAddress macAddress(MACADDR);

	// Setup serial port
	SERIALPORT.begin(SERIALBAUD);
	SERIALPORT.println("[PhysDoor Board]");
	SERIALPORT.print("MAC address:        ");
	macAddress.println(SERIALPORT);

	// Setup relay port
	pinMode(RELAY_PIN1, OUTPUT);
	pinMode(RELAY_PIN2, OUTPUT);
	pinMode(PIEZO_PIN, OUTPUT);
	pinMode(MONITORING_CONTACT_PIN, INPUT_PULLUP);
	pinMode(ADMIN_BUTTON_PIN, INPUT_PULLUP);

	// Setup ethernet
	if (ether.begin(macAddress) == false) {
		SERIALPORT.println("Failed to configure Ethernet, no IPv6 router on network?");
	} else {
		SERIALPORT.print("Link-local address: ");
		ether.linkLocalAddress().println(SERIALPORT);
		SERIALPORT.print("Global address:     ");
		ether.globalAddress().println(SERIALPORT);
		SERIALPORT.println("Ethernet link is ready");
	}

	// Setup SNTP client
	ntp_init(&ntpsocket);
}

void loop() {
	static unsigned long relay_open_time = 0;
	static unsigned long piezo_start_time = 0;
	static unsigned long piezo_nextbeep_time = 0;

	// Close relay when RELAY_TIMEOUT milliseconds have expired
	// Warning: millis() will overflow at some point (every 50 days according to Arduino docs),
	// so "millis() - relay_open_time" must remain on the left side of the equation so that it
	// underflows in case millis() gets reset to some value close to 0 (thanks unsigned long arithmetic).
	// Same thing with "millis() - piezo_start_time"
	if (millis() - relay_open_time > ((unsigned long) RELAY_TIMEOUT)) {
		digitalWrite(RELAY_PIN1, LOW);
		digitalWrite(RELAY_PIN2, LOW);
	}

	// piezo_start_time = 0 --> Disable piezo buzzer
	if (piezo_start_time != 0 && millis() - piezo_start_time < ((unsigned long) PIEZO_TIMEOUT)) {
		if (millis() > piezo_nextbeep_time) {
			sound(200);
			piezo_nextbeep_time = millis() + PIEZO_BEEP_PERIOD;
		}
	}

	ether.receivePacket();

	/*** Handle SNTP Protocol ***/
	handle_ntp();

	/*** Handle HTTP Requests ***/
	if (http.isGet(F("/"))) {
		http.printHeaders(http.typeHtml);
		http.println(F("<h1>physdoor board</h1>"));
		http.sendReply();
	} else if (http.isPost(F("/open"))) {
		http.printHeaders(http.typePlain);
		if (checkKey(http.body())) {
			digitalWrite(RELAY_PIN1, HIGH);
			digitalWrite(RELAY_PIN2, HIGH);
			relay_open_time = millis();
			http.println(F("ok"));
		} else {
			http.println(F("invalid key"));
		}
		http.sendReply();
	} else if (http.isPost(F("/beep_start"))) {
		http.printHeaders(http.typePlain);
		if (checkKey(http.body())) {
			piezo_start_time = millis();
			piezo_nextbeep_time = millis();
			http.println(F("ok"));
		} else {
			http.println(F("invalid key"));
		}
		http.sendReply();
	} else if (http.isPost(F("/beep_stop"))) {
		http.printHeaders(http.typePlain);
		if (checkKey(http.body())) {
			piezo_start_time = 0;
			http.println(F("ok"));
		} else {
			http.println(F("invalid key"));
		}
		http.sendReply();
	} else if (http.isGet(F("/doorstate"))) {
		http.printHeaders(http.typeHtml);
		http.println(digitalRead(MONITORING_CONTACT_PIN) ? "open" : "closed");
		http.sendReply();
	} else if (http.isGet(F("/epoch"))) {
		http.printHeaders(http.typeHtml);
		http.println(epoch());
		http.sendReply();
	} else if (http.isPost(F("/set_password"))) {
		http.printHeaders(http.typeHtml);
		if (digitalRead(ADMIN_BUTTON_PIN) == 0) {
			writeSetting(EEPROM_OFFSET_PASSWORD, http.body());
			http.println(F("ok"));
		} else {
			http.println(F("no admin"));
		}
		http.sendReply();
	} else if (http.isPost(F("/set_ntpserv"))) {
		http.printHeaders(http.typeHtml);
		if (digitalRead(ADMIN_BUTTON_PIN) == 0) {
			writeSetting(EEPROM_OFFSET_NTPIP, http.body());
			http.println(F("ok"));
		} else {
			http.println(F("no admin"));
		}
		http.sendReply();
	} else {
		http.notFound();
	}
}
