
cd ../../..

set LDSCRIPT = config/stm32f4ems/STM32F407xG.ld

set PROJECT_BOARD=ME7_Ecu_V2
set PROJECT_CPU=ARCH_STM32F4

call config/boards/common_make.bat

call config/boards/clean_env_variables.bat


