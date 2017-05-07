#include <EtherSia.h>
#include <Arduino.h>

#include "config.h"

EtherSia_W5100 ether(W5100_CSPIN);
HTTPServer http(ether);

// Frequency is approx. 2000Hz, duration in ms
void sound(uint16_t duration) {
	for (uint16_t i = 0; i < duration * 2; ++i) {
		digitalWrite(PIEZO_PIN, HIGH);
		delayMicroseconds(250);
		digitalWrite(PIEZO_PIN, LOW);
		delayMicroseconds(250);
	}
}

bool checkKey(String key) {
	return String(PASSWORD).equals(key);
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

	// Setup ethernet
	if (ether.begin(macAddress) == false) {
		SERIALPORT.println("Failed to configure Ethernet");
	} else {
		SERIALPORT.print("Link-local address: ");
		ether.linkLocalAddress().println(SERIALPORT);
		SERIALPORT.print("Global address:     ");
		ether.globalAddress().println(SERIALPORT);
		SERIALPORT.println("Ethernet link is ready");
	}
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
			http.println(F("invalid password"));
		}
		http.sendReply();
	} else if (http.isPost(F("/beep_start"))) {
		http.printHeaders(http.typePlain);
		if (checkKey(http.body())) {
			piezo_start_time = millis();
			piezo_nextbeep_time = millis();
			http.println(F("ok"));
		} else {
			http.println(F("invalid password"));
		}
		http.sendReply();
	} else if (http.isPost(F("/beep_stop"))) {
		http.printHeaders(http.typePlain);
		if (checkKey(http.body())) {
			piezo_start_time = 0;
			http.println(F("ok"));
		} else {
			http.println(F("invalid password"));
		}
		http.sendReply();
	} else {
		http.notFound();
	}
}
