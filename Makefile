BOARD		:= arduino:avr:leonardo
TARGET		:= main/main.cpp
FLAGS		:= -v

all: verify
	@echo Compilation succesfull. Type \"make flash\" to flash the firmware.

verify:
	arduino --verify --board $(BOARD) $(FLAGS) $(TARGET)

installdeps:
	arduino --install-library EtherSia

flash:
	arduino --upload --board $(BOARD)  $(FLAGS) $(TARGET)
