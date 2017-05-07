/*
 * Application Configuration
 *
 * PASSWORD: Password that must be provided as POST data to "/open"
 * RELAY_TIMEOUT: Number of milliseconds the relay will close the door opener circuit
 * PIEZO_TIMEOUT: Number of milliseconds after which the piezo buzzer stops automatically
 * PIEZO_BEEP_PERIOD: Number of milliseconds between single piezo buzzer beeps
 */
#define PASSWORD "wiebJatPabImDaikOpHi"
#define RELAY_TIMEOUT 5000
#define RELAY_PIN1 7
#define RELAY_PIN2 8
#define PIEZO_PIN A3
#define PIEZO_TIMEOUT 10000
#define PIEZO_BEEP_PERIOD 700

/*
 * Network Configuration
 *
 * MACADDR: MAC address, should be locally administered
 */
#define MACADDR "7a:4b:5e:49:8d:ba"

/*
 * Hardware Configuration
 *
 * W5100_CSPIN: SPI Chip Select (CS) Pin of W5100, default: D10
 * SERIALPORT: `Serial1` for hardware UART or `Serial` for Serial over USB (Leonardo / DFR0222)
 */
#define W5100_CSPIN 10
#define SERIALPORT Serial1
#define SERIALBAUD 115200
