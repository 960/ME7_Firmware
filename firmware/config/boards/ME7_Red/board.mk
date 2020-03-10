
BOARDS_DIR = $(PROJECT_DIR)/config/boards

# Target ECU board design
BOARDSRC_CPP = $(BOARDS_DIR)/ME7_Red/board_configuration.cpp

  MCU_DEFS = -DSTM32F767xx
  BOARDSRC = $(CHIBIOS)/os/hal/boards/ST_NUCLEO144_F767ZI/board.c
  BOARDINC = $(BOARDS_DIR)/nucleo_f767		# For board.h
  BOARDINC += $(PROJECT_DIR)/config/stm32f7ems	# efifeatures/halconf/chconf.h
  LDSCRIPT= $(BOARDS_DIR)/nucleo_f767/STM32F76xxI.ld
  
 DEFAULT_ENGINE_TYPE = -DDEFAULT_ENGINE_TYPE=MINIMAL_PINS
 
 
DDEFS += $(MCU_DEFS) -DEFI_USE_OSC=TRUE -DEFI_FATAL_ERROR_PIN=GPIOF_15 -DFIRMWARE_ID=\"microRusEfi\" $(DEFAULT_ENGINE_TYPE)
