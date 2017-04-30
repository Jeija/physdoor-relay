BOARD		:= arduino:avr:leonardo
TARGET		:= main/main.cpp
FLAGS		:= --useprogrammer --port usb -v
PROGRAMMER	:= arduino:usbasp

all: verify
	@echo Compilation succesfull. Type \"make flash\" to flash the firmware.

verify:
	arduino --verify --board $(BOARD) --pref programmer=$(PROGRAMMER) $(FLAGS) $(TARGET)

installdeps:
	arduino --install-library EtherSia

flash:
	arduino --upload --board $(BOARD) --pref programmer=$(PROGRAMMER) $(FLAGS) $(TARGET)
