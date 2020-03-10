BOARDS_DIR = $(PROJECT_DIR)/config/boards

BOARDSRC_CPP = $(PROJECT_DIR)/config/boards/me7_pnp/board_configuration.cpp

# Required include directories
  MCU_DEFS = -DSTM32F407xx
  BOARDSRC = $(CHIBIOS)/os/hal/boards/ST_STM32F4_DISCOVERY/board.c
  BOARDINC = $(BOARDS_DIR)/ME7_Assembled
  BOARDINC += $(PROJECT_DIR)/config/stm32f4ems	# For board.h
  BOARDINC += $(BOARDS_DIR)/st_stm32f4
LDSCRIPT= $(PROJECT_DIR)/config/stm32f4ems/STM32F407xG.ld
#
# Override DEFAULT_ENGINE_TYPE
DDEFS += -DDEFAULT_ENGINE_TYPE=MINIMAL_PINS -DSTM32F407xx -DEFI_FATAL_ERROR_PIN=GPIOF_15 -DFIRMWARE_ID=\"microRusEfi\"

