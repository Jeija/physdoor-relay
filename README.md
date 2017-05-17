# Physdoor Door Opener Relay Board
**Hardware**: [DFRobot DFR0222](https://www.dfrobot.com/wiki/index.php/X-Board_Relay(SKU:DFR0222)) with ATMega32u4 + WizNet WIZ5100; two relays, one piezo beeper and a door monitoring contact connected.

**Software:** The firmware is based on the Arduino Prototyping platform, a Makefile is also provided so that the firmware can be compiled without using the Arduino IDE. The Relay Board uses the IPv6-only [EtherSia](https://github.com/njh/EtherSia) Arduino Library for network connectivity.

## Services
At startup, the board performs SLAAC to obtain a global IPv6 address. Make sure an IPv6 router is available on the local network, otherwise the board will fail to boot.

The Relay Board runs an HTTP server and an SNTP client for time synchronization. By default, the SNTP client connects to one of Google's NTP servers at `2001:4860:4806::`.

The HTTP server provides the following APIs:
* `GET` to `/`: Webpage containing only `"<h1>physdoor board</h1>"`, can be used for debugging or monitoring
* `POST` to `/open`: Open door, closes relay for configured time. `POST` data is authentication key as described in the section below.
* `POST` to `/beep_start`: Start piezo beeper, automtatically stops after configured time. `POST` data is authentication key as described in the section below.
* `POST` to `/beep_stop`: Stop piezo beeper immediately. `POST` data is authentication key as described in the section below.
* `GET` to `/doorstate`: Get state of monitoring contact: `open` or `closed`
* `GET` to `/epoch`: Get unix epoch of board's internal time (obtained using NTP), can be used for debugging or monitoring
* `POST` to `/set_password`: Set shared plaintext password, `POST` data is new password. Requires *admin button* to be pressed.
* `POST` to `/set_ntpserv`: Set IPv6 address of NTP server, `POST` data is new address. Requires *admin button* to be pressed.

The *admin button* is a button connected between `ADMIN_BUTTON_PIN` and GND that has to be pressed (so that `ADMIN_BUTTON_PIN` is pulled low) while executing administrative commands, otherwise those commands will return `no admin`. This way, physcial access to the DFR0222 is required for changing settings.
The other `POST` requests will respond with either `ok` for successful execution of the command or `invalid key` if the key provided as `POST` is invalid.

## Authentication
The relay board and the physdoor server share the password configured as `PASSWORD` in `config.h`. In order to protect against replay attacks, this password gets hashed together with a rounded timestamp so that each hash is only ever valid for a short time period (`AUTH_TIMEFRAME` setting, default 10 seconds). Therefore, the relay board and the physdoor server need to synchronize their system clocks using NTP. The relay board compares the hash provided as `POST` data to the hash that is valid for the current time period as well as the hashes for the previous and next time period, so that slight system clock offsets or network delays are accounted for.

### Manually Generate Hash
The hash for the current authentication period can be calculated as follows:
* Get current UNIX epoch timestamp (`date +%s`) and round it to `AUTH_TIMEFRAME`: `$(($(($(date +%s) / $AUTH_TIMEFRAME)) * $AUTH_TIMEFRAME))`
* Concatenate the rounded UNIX epoch to the shared plaintext password
* Calculate the SHA3-256 (Keccak) hash of the concatenated string
* Convert SHA3-256 hash to a hexadecimal string (length 64 characters), this is your key

This code can be used to generate a valid hash for the current authentication period and send an `/open` request to the relay board.
```bash
export RELAYPASSWORD=wiebJatPabImDaikOpHi # Replace placeholder password with your own
export RELAYIP=[2001:YOUR:BOARD:IP:HERE::]
export AUTH_TIMEFRAME=50
curl --data $(echo -n "$RELAYPASSWORD$(($(($(date +%s) / $AUTH_TIMEFRAME)) * $AUTH_TIMEFRAME))" | rhash --sha3-256 -p %x{sha3-256} -) http://$RELAYIP/open
```

## License and Attribution
* [3-clause BSD license](https://opensource.org/licenses/BSD-3-Clause)
* The [minimal Keccak hash algorithm implementation](https://github.com/gvanas/KeccakCodePackage/blob/master/Standalone/CompactFIPS202/Keccak-more-compact.c) used in `main/keccak.ino` is released to the public domain
* The HTTP and SNTP implementation are loosely based on the the examples provided by the [EtherSia](https://github.com/njh/ethersia) library, which is also licensed under the [3-clause BSD license](https://opensource.org/licenses/BSD-3-Clause)
