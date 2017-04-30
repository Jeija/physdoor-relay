#include <EtherSia.h>
#include <Arduino.h>

#include "config.h"

EtherSia_W5100 ether(W5100_CSPIN);
HTTPServer http(ether);

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

	// Close relay when RELAY_TIMEOUT milliseconds have expired
	// Warning: millis() will overflow at some point (every 50 days according to Arduino docs),
	// so "millis() - relay_open_time" must remain on the left side of the equation so that it
	// underflows in case millis() gets reset to some value close to 0 (thanks unsigned long arithmetic).
	if (millis() - relay_open_time > ((unsigned long) RELAY_TIMEOUT)) {
		digitalWrite(RELAY_PIN1, LOW);
		digitalWrite(RELAY_PIN2, LOW);
	}

	ether.receivePacket();

	if (http.isGet(F("/"))) {
		http.printHeaders(http.typeHtml);
		http.println(F("<h1>physdoor board</h1>"));
		http.sendReply();
	} else if (http.isPost(F("/open"))) {
		http.printHeaders(http.typePlain);
		if (String(PASSWORD).equals(http.body())) {
			digitalWrite(RELAY_PIN1, HIGH);
			digitalWrite(RELAY_PIN2, HIGH);
			relay_open_time = millis();
			http.println(F("ok"));
		} else {
			http.println(F("invalid password"));
		}
		http.sendReply();
	} else {
		http.notFound();
	}
}
