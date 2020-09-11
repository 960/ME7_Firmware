/**
 * @file	bluetooth.cpp
 *
 *
 * It looks like Bluetooth modules arrive in all kinds of initial configuration.
 * Sometimes we need to execute a one-time initialization including settings the baud rate. rusEFI setting uartConsoleSerialSpeed or tunerStudioSerialSpeed
 * has to match BT module configuration.
 *
 *
 * @author andreika, (c) 2017
 */

#include "global.h"
#include "os_access.h"
#include "allsensors.h"
#include "tunerstudio.h"

#include "tunerstudio_io.h"
#include "bluetooth.h"
#include "engine_configuration.h"

#if EFI_BLUETOOTH_SETUP

static bool btProcessIsStarted = false;
static bool btProcessIsRunning = false;
static const char *commands[5];
static int numCommands = 0;
static int setBaudIdx = -1;

static char cmdHello[5];
static char cmdBaud[25];
static char cmdName[30];
static char cmdPin[16];

static const int baudRates[] = { 0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, -1 };
static const int baudRateIndexList[] = { 4 /*9600*/, 6 /*38400*/, 8 /*115200*/, 7, 5, 3, 2, 1, -1 };
static const int btModuleTimeout = TIME_MS2I(1000);

static ts_channel_s *tsChannel;


static THD_WORKING_AREA(btThreadStack, UTILITY_THREAD_STACK_SIZE);
static thread_t *btThread = nullptr;
static thread_reference_t btThreadRef = nullptr; // used by thread suspend/resume as a flag



EXTERN_ENGINE;


// Main communication code
// We assume that the user has disconnected the software before starting the code.
static void runCommands() {
	uint8_t buffer[2];

	if (!btProcessIsStarted)
		return;
	
	chThdSleepMilliseconds(1000);	// safety

	// Store current serial port speed - we're going to change it
	int savedSerialSpeed = CONFIG(tunerStudioSerialSpeed);
	
	int prevBaudIdx = -1, baudIdx = -1, baudListIdx = 0;
	int cmdIdx = 0;
	
	// run all commands
	while (true) {
		if (baudIdx < 0) {
			// try all available baud rates, in order of preference
			baudIdx = baudRateIndexList[baudListIdx++];
			// we've tried all baud rates?
			if (baudIdx < 0)
				break;
		}
		
		bool restoreAndExit = (cmdIdx >= numCommands || baudIdx < 0);
		
		// if the baud rate is changed, reinit the UART
		if (baudIdx != prevBaudIdx || restoreAndExit) {
#if EFI_USB_SERIAL
			// if we have USB we assume BT operates on primary TTL
			// todo: we need to clean a lot in this area :(
#ifdef EFI_CONSOLE_SERIAL_DEVICE
			extern SerialConfig serialConfig;
			sdStop(EFI_CONSOLE_SERIAL_DEVICE);
#endif /*  EFI_CONSOLE_SERIAL_DEVICE */
#ifdef EFI_CONSOLE_UART_DEVICE
			extern UARTConfig uartConfig;
			uartStop(EFI_CONSOLE_UART_DEVICE);
#endif /* EFI_CONSOLE_UART_DEVICE */

#else
			// deinit UART
			if (!stopTsPort(tsChannel)) {

				return;
			}
#endif /* EFI_USB_SERIAL */
			chThdSleepMilliseconds(10);	// safety
			// change the port speed
			CONFIG(tunerStudioSerialSpeed) = restoreAndExit ? savedSerialSpeed : baudRates[baudIdx];

#if EFI_USB_SERIAL
#ifdef EFI_CONSOLE_SERIAL_DEVICE
			serialConfig.speed = CONFIG(tunerStudioSerialSpeed);
			sdStart(EFI_CONSOLE_SERIAL_DEVICE, &serialConfig);
#endif /*  EFI_CONSOLE_SERIAL_DEVICE */
#ifdef EFI_CONSOLE_UART_DEVICE
			uartConfig.speed = CONFIG(tunerStudioSerialSpeed);
			uartStart(EFI_CONSOLE_UART_DEVICE, &uartConfig);
#endif /* EFI_CONSOLE_UART_DEVICE */
#else
			// init UART
			startTsPort(tsChannel);
#endif /* EFI_USB_SERIAL */
			chThdSleepMilliseconds(10);	// safety
			prevBaudIdx = baudIdx;
		}

		// exit if all commands were sent
		if (restoreAndExit)
			break;

		// send current command
		sr5WriteData(tsChannel, (uint8_t *)commands[cmdIdx], strlen(commands[cmdIdx]));
		
		// waiting for an answer
		bool wasAnswer = false;
		if (sr5ReadDataTimeout(tsChannel, buffer, 2, btModuleTimeout) == 2) {
			wasAnswer = (buffer[0] == 'O' && buffer[1] == 'K') || 
				(buffer[0] == '+' && (buffer[1] >= 'A' && buffer[1] <= 'Z'));
		}

		// wait 1 second and skip all remaining response bytes from the bluetooth module
		while (true) {
			if (sr5ReadDataTimeout(tsChannel, buffer, 1, btModuleTimeout) < 1)
				break;
		}
		
		if (wasAnswer) {
			// if we changed the baud rate
			if (commands[cmdIdx] == cmdBaud) {
				// this is what we've set
				baudIdx = setBaudIdx;
				// if we fail somehow, try all other baud rates
				baudListIdx = 0;
			}
			// move to the next command
			cmdIdx++;
		} else {
			// try other baud rate
			baudIdx = -1;
		}
	}



}

static THD_FUNCTION(btThreadEntryPoint, arg) {
	(void) arg;
	chRegSetThreadName("bluetooth thread");

	// now wait
	chSysLock();
	msg_t msg = chThdSuspendTimeoutS(&btThreadRef, BLUETOOTH_COMMAND_TIMEOUT);
	chSysUnlock();
	
	if (msg == MSG_TIMEOUT) {

		return;
	} else {
		// call this when the thread is resumed (after the disconnect)
		btProcessIsRunning = true;
		runCommands();
		btProcessIsRunning = false;
	}

	// release the command
	btProcessIsStarted = false;
	chThdExit(MSG_OK);
}

void bluetoothStart(ts_channel_s *btChan, bluetooth_module_e moduleType, const char *baudRate, const char *name, const char *pinCode) {
	static const char *usage = "Usage: bluetooth_hc06 <baud> <name> <pincode>";

	tsChannel = btChan;
	
	// if a binary protocol uses USB, we cannot init the bluetooth module!
	if (!CONFIG(useSerialPort)) {
		return;
	}

	if (btProcessIsStarted) {
		return;
	}

	numCommands = 0;

	// now check the arguments and add other commands:
	// 1) baud rate
	int baud = (baudRate != NULL) ? atoi(baudRate) : 0;
	int i;
	// find a known baud rate in our list
	setBaudIdx = -1;
	for (i = 1; baudRates[i] > 0; i++) {
		if (baudRates[i] == baud) {
			setBaudIdx = i;
			break;
		}
	}
	// check the baud rate index
	if (setBaudIdx < 1) {
		if (baud == 0) {
			return;
		}
	} else {
		// ok, add command!
		
	}
	
	// 2) check name
	if (name == NULL || strlen(name) < 1 || strlen(name) > 20) {
		return;
	}

	// 3) check pin code
	int numDigits = 0;
	// check the pincode
	if (pinCode != NULL && strlen(pinCode) == 4) {
		for (i = 0; i < 4; i++) {
			if (pinCode[i] >= '0' && pinCode[i] <= '9')	// digit
				numDigits++;
		}
	}
	if (numDigits != 4) {
		return;
	}

	// ok, add commands!
	switch (moduleType) {
	case BLUETOOTH_HC_05:
		chsnprintf(cmdHello, sizeof(cmdHello), "AT\r\n");
		chsnprintf(cmdBaud, sizeof(cmdBaud), "AT+UART=%d,0,0\r\n", baud);	// baud rate, 0=(1 stop bit), 0=(no parity bits)
		chsnprintf(cmdName, sizeof(cmdName), "AT+NAME=%s\r\n", name);
		chsnprintf(cmdPin, sizeof(cmdPin), "AT+PSWD=%s\r\n", pinCode);
		// todo: add more commands?
		// AT+RMAAD
		// AT+ROLE=0
		break;
	case BLUETOOTH_HC_06:
		chsnprintf(cmdHello, sizeof(cmdHello), "AT");
		chsnprintf(cmdBaud, sizeof(cmdBaud), "AT+BAUD%c", '0' + setBaudIdx);
		chsnprintf(cmdName, sizeof(cmdName), "AT+NAME%s", name);
		chsnprintf(cmdPin, sizeof(cmdPin), "AT+PIN%s", pinCode);
		break;
	case BLUETOOTH_SPP:
		chsnprintf(cmdHello, sizeof(cmdHello), "AT\r\n");
		chsnprintf(cmdBaud, sizeof(cmdBaud), "AT+BAUD%c\r\n", '0' + setBaudIdx);
		chsnprintf(cmdName, sizeof(cmdName), "AT+NAME%s\r\n", name);
		chsnprintf(cmdPin, sizeof(cmdPin), "AT+PIN%s\r\n", pinCode);
		break;
	default:
		// todo: add support for other BT module types

		return;
	}
	commands[numCommands++] = cmdHello; // this command is added to test a connection
	commands[numCommands++] = cmdBaud;
	commands[numCommands++] = cmdName;
   	commands[numCommands++] = cmdPin;
   	
   	// create a thread to execute these commands later
   	btThread = chThdCreateStatic(btThreadStack, sizeof(btThreadStack), NORMALPRIO, (tfunc_t)btThreadEntryPoint, NULL);
   	
	btProcessIsStarted = true;
}
	
void bluetoothSoftwareDisconnectNotify() {
	if (btProcessIsStarted) {
		// start communication with the module
		chThdResume(&btThreadRef, MSG_OK);
		// wait the thread to finish
		chThdWait(btThread);
	}
}

void bluetoothCancel() {
	if (!btProcessIsStarted) {
		return;
	}

	if (btProcessIsRunning)
		return;
	
	// terminate thread
	chThdTerminate(btThread);
	
	btProcessIsStarted = false;

}

#endif /* EFI_BLUETOOTH_SETUP */
