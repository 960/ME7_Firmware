
echo Compiler gcc version
arm-none-eabi-gcc -v

rd /s /q deliver
mkdir deliver
cd deliver
mkdir ME7_Ecu_V2
mkdir ME7_Ecu_V3

cd..
call clean.bat
call gen_config.bat

echo "TIMESTAMP %date% %time%"
set PROJECT_BOARD=ME7_Ecu_V2
set PROJECT_CPU=ARCH_STM32F7
make -j4

rem mv build\firmware.elf deliver/ME7_Ecu_V2\ME7_Ecu_V2.elf
rem this file is needed for DFU generation
mv build\firmware.hex deliver/ME7_Ecu_V2\ME7_Ecu_V2.hex

cp build\firmware.bin deliver/ME7_Ecu_V2\ME7_Ecu_V2.bin
echo Debug compilation results 2/2
echo "TIMESTAMP %date% %time%"
ls -l build
if not exist deliver/ME7_Ecu_V2/ME7_Ecu_V2.hex echo FAILED to compile DEFAULT with DEBUG
if not exist deliver/ME7_Ecu_V2/ME7_Ecu_V2.hex exit -1

rm -f deliver/ME7_Ecu_V2/ME7_Ecu_V2.dfu
echo %script_name%: invoking hex2dfu.exe
..\misc\encedo_hex2dfu\hex2dfu.exe -i deliver/ME7_Ecu_V2/ME7_Ecu_V2.hex -o deliver/ME7_Ecu_V2/ME7_Ecu_V2.dfu
call clean.bat

echo "TIMESTAMP %date% %time%"
set PROJECT_BOARD=ME7_Ecu_V3
set PROJECT_CPU=ARCH_STM32F7

make -j4
rem cp build\firmware.elf deliver/ME7_Ecu_V3\ME7_Ecu_V3.elf
cp build\firmware.bin deliver/ME7_Ecu_V3\ME7_Ecu_V3.bin
rem this file is needed for DFU generation
cp build\firmware.hex deliver/ME7_Ecu_V3\ME7_Ecu_V3.hex
echo Release compilation results 1/2
echo "TIMESTAMP %date% %time%"
ls -l build
if not exist deliver/ME7_Ecu_V3\ME7_Ecu_V3.hex echo FAILED to compile NO ASSERTS version
if not exist deliver/ME7_Ecu_V3\ME7_Ecu_V3.hex exit -1

rm -f deliver/ME7_Ecu_V3/ME7_Ecu_V3.dfu
echo %script_name%: invoking hex2dfu.exe
..\misc\encedo_hex2dfu\hex2dfu.exe -i deliver/ME7_Ecu_V3/ME7_Ecu_V3.hex -o deliver\ME7_Ecu_V3/ME7_Ecu_V3.dfu

echo %script_name%: deliver folder
ls -l deliver
cp 7za.exe deliver\7za.exe
cd deliver
7za a ME7_Ecu_V2.zip ME7_Ecu_V2\*.*
mv ME7_Ecu_V2.zip ME7_Ecu_V2\ME7_Ecu_V2.zip


7za a ME7_Ecu_V3.zip ME7_Ecu_V3\*.*
mv ME7_Ecu_V3.zip ME7_Ecu_V3\ME7_Ecu_V3.zip
cd..
call config/boards/clean_env_variables.bat
winscp.com /ini=nul /script=ftp_script.txt

echo clean_compile_two_versions: Looks good!
