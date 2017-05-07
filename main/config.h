/*
 * Application Configuration
 *
 * PASSWORD: Password that must be provided as POST data to "/open"
 * RELAY_TIMEOUT: Number of milliseconds the relay will close the door opener circuit
 * PIEZO_TIMEOUT: Number of milliseconds after which the piezo buzzer stops automatically
 * PIEZO_BEEP_PERIOD: Number of milliseconds between single piezo buzzer beeps
 * AUTH_TIMEFRAME: Number of seconds an authentication hash is valid
 */
#define PASSWORD "wiebJatPabImDaikOpHi"
#define RELAY_TIMEOUT 5000
#define PIEZO_TIMEOUT 10000
#define PIEZO_BEEP_PERIOD 700
#define AUTH_TIMEFRAME 10

// Pin configuration
#define RELAY_PIN1 7
#define RELAY_PIN2 8
#define PIEZO_PIN A3
#define MONITORING_CONTACT_PIN A2

/*
 * Network Configuration
 *
 * MACADDR: MAC address, should be locally administered
 * NTPSERV: NTP Server IPv6 address, here: time1.google.com
 * NTP_POLLING_INTERVAL: Polling interval for NTP in seconds
 */
#define MACADDR "7a:4b:5e:49:8d:ba"
#define NTPSERV "2001:4860:4806::"
#define NTP_POLLING_INTERVAL 60

/*
 * Hardware Configuration
 *
 * W5100_CSPIN: SPI Chip Select (CS) Pin of W5100, default: D10
 * SERIALPORT: `Serial1` for hardware UART or `Serial` for Serial over USB (Leonardo / DFR0222)
 */
#define W5100_CSPIN 10
#define SERIALPORT Serial1
#define SERIALBAUD 115200
