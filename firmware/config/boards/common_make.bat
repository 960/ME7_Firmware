set script_name=common_make.bat
echo Entering %script_name%

mkdir .dep
rem todo: start using env variable for number of threads or for '-r'
make -j4 -r
if not exist build/firmware.hex echo FAILED to compile by %script_name% with %PROJECT_BOARD% %DEBUG_LEVEL_OPT% and %EXTRA_PARAMS%
if not exist build/firmware.hex exit -1

mkdir deliver
cd deliver
mkdir %PROJECT_BOARD%
cd..
rm -f deliver/%PROJECT_BOARD%/firmware.dfu
echo %script_name%: invoking hex2dfu.exe
..\misc\encedo_hex2dfu\hex2dfu.exe -i build/firmware.hex -o deliver/%PROJECT_BOARD%/firmware.dfu
cp build/firmware.bin deliver/%PROJECT_BOARD%/

echo %script_name%: deliver folder
ls -l deliver

call config/boards/clean_env_variables.bat