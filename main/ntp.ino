/*
 * Based on EtherSia's reference SNTP implementation:
 * https://github.com/njh/EtherSia/blob/master/examples/SNTPClient/SNTPClient.ino
 */

#include "config.h"

UDPSocket *udp;

/*
 * last_request_time: millis() time the last NTP request was send, 0 = never
 * last_response_times: millis() time the last NTP response was received, 0 = never
 * last_response_timestamp: Last NTP timestamp received, 0 = no response received yet
 */
unsigned long last_request_time = 0;
unsigned long last_response_time = 0;
unsigned long last_response_timestamp = 0;

void ntp_init(UDPSocket *ntpsocket) {
	udp = ntpsocket;
	udp->setRemoteAddress(NTPSERV, NTP_PORT);
}

void handle_ntp() {
	if (udp->havePacket()) {
		ntpType *ntpPacket = (ntpType*)udp->payload();

		// If stratum value is 0, then it is a "Kiss-o'-Death" packet, ignore
		if (ntpPacket->stratum != 0) {
			// Extract the transmit timestamp from the packet
			// this is NTP time (seconds since Jan 1 1900)
			// The ntohl() function converts from network byte-order to native byte-order
			last_response_time = millis();
			last_response_timestamp = ntohl(ntpPacket->transmitTimestampSeconds);

			SERIALPORT.print("Got SNTP response, new UNIX epoch: ");
			SERIALPORT.println(epoch());
		}
	}

	// Preserve the structure of the if-condition (millis() - last_request_time) so that millis()
	// overflowing gets handled correctly by sending an NTP request earlier.
	if (last_request_time == 0 || millis() - last_request_time >= ((unsigned long) NTP_POLLING_INTERVAL * 1000)) {
		// Set the NTP packet to all-zeros
		ntpType ntpPacket;
		memset(&ntpPacket, 0, sizeof(ntpPacket));

		// Set NTP header flags (Leap Indicator=Not Synced, Version=4, Mode=Client)
		ntpPacket.flags = 0xe3;
		udp->send(&ntpPacket, sizeof(ntpPacket));

		last_request_time = millis();
		SERIALPORT.println("Sent SNTP request");
	}
}

/*
 * Calculate UNIX epoch: Seconds since Jan 1 1970
 * Unix time starts on Jan 1 1970. In seconds since Jan 1 1900 (NTP timestamp), that's 2208988800
 */
unsigned long epoch() {
	unsigned long seconds_since_request = (millis() - last_response_time) / 1000;
	return last_response_timestamp + seconds_since_request - 2208988800UL;
}
