/**
 * @file obd_error_codes.h
 * @brief Standart OBD-II error codes
 *
 * More info at http://www.obd-codes.com/faq/obd2-codes-explained.php
 *
 * @date Dec 20, 2013
 * @author Andrey Belomutskiy, (c) 2012-2017
 */

#ifndef OBD_ERROR_CODES_H_
#define OBD_ERROR_CODES_H_

// this header should not depend on anything - actually chconf.h usually depends
// on this header

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// for now I want most enums to be 32 bit integers. At some point maybe we will
// make the one-byte this is about offsets and sizes in TunerStudio
#define ENUM_32_BITS 2000000000

typedef enum {


//Todo: Separate out the related user fault messages from the internal
//      Make all of the sensor errors range/time based
//      Make the ETB error deviation/time based and remove the rest position error
//      Add a max/min-duty/time error who shuts down ETB if exeed
//      Add the  sub-sensors for ETB/pedal and a deviation/time error-code
//      Add a error code if they are not assigned

//      Enum the 50-100 most relevant codes to show as text in TS
//      Also separate active vs stored so the active error goes away by itself.

  // Actual Error Codes

	Electronic_Throttle_Position_Control_Error = 1,  // For example 1 short blink, break some seconds and 1 short


  OBD_Manifold_Absolute_Pressure_Circuit_Malfunction = 2,
  OBD_Intake_Air_Temperature_Circuit_Malfunction = 3,
  OBD_Engine_Coolant_Temperature_Circuit_Malfunction = 4,
  OBD_Throttle_Position_Sensor_Circuit_Malfunction = 5,
  OBD_Throttle_Position_Sensor_Range_Performance_Problem = 6,
  OBD_Crankshaft_Position_Sensor_A_Circuit_Malfunction = 7,
  OBD_Camshaft_Position_Sensor_Circuit_Range_Performance = 8,
  OBD_Oil_Pressure_Sensor_Malfunction = 9,
  OBD_System_Voltage_Malfunction = 10,
  OBD_PCM_Processor_Fault = 11,
  CUSTOM_INVALID_TPS_SETTING = 12,

  CUSTOM_ERR_PIN_ALREADY_USED_1 = 13,
  CUSTOM_ERR_PIN_ALREADY_USED_2 = 14,


  // Custom Codes/debug/duplicates

  // Todo: Figure out which ones that can be removed, who are catched by other
  // error codes and so on
  // Figure out who/change the ones that can fit in as a actual relevant user-fault code

  OBD_Barometric_Press_Circ = 2226,
  OBD_Barometric_Press_Circ_Range_Perf = 2227,
  CUSTOM_UNEXPECTED_TDC_ANGLE = 6546, // assertAngleRange(tdcPosition, "tdcPos#a1" (Trigger_structure)
  CUSTOM_INVALID_GLOBAL_OFFSET = 6547, // assertAngleRange(CONFIG_PARAM(globalTriggerAngleOffset),
                                       // "tdcPos#a2"(Trigger_structure)

  CUSTOM_NAN_ENGINE_LOAD = 6000, // NaN engine load (fuel_math) and NaN engine load (advance_map)
  CUSTOM_WRONG_ALGORITHM = 6001, //"wrong algorithm for MAP-based timing" (advance_map)
  CUSTOM_NAN_ENGINE_LOAD_2 = 6002, // NaN engine load (fuel_map)
  CUSTOM_EMPTY_FSIO_STACK = 6005,
  CUSTOM_UNKNOWN_FSIO = 6006,
  CUSTOM_NO_FSIO = 6007,
  CUSTOM_FSIO_STACK_SIZE = 6008,
  CUSTOM_FSIO_UNEXPECTED = 6009,

  CUSTOM_FSIO_PARSING = 6010,
  CUSTOM_FSIO_INVALID_EXPRESSION = 6011,
  CUSTOM_INTEPOLATE_ERROR = 6012, // "preCalc: Same x1 and x2 in interpolate:
                                  // %.2f/%.2f", x1, x2) (table_helper)
  // "interpolate%s: why param"   and some more            (interpolation.h)
  CUSTOM_INTEPOLATE_ERROR_2 = 6013, // y is NaN in interpolate3d (interpolation.h)
  CUSTOM_INTEPOLATE_ERROR_3 = 6014, // x is NaN in interpolate3d (interpolation.h)
  CUSTOM_INTEPOLATE_ERROR_4 = 6015, // preCalc: Same x1 and x2 in interpolate (table helper.h)
  CUSTOM_PARAM_RANGE = 6016, // y is NaN (table_helper)
  CUSTOM_MAF_NEEDED = 6017, // MAF sensor needed for current fuel algorithm (engine_math)
  CUSTOM_UNKNOWN_ALGORITHM = 6018, // Unexpected engine load parameter (engine_math)
  CUSTOM_OBD_COIL_PIN_NOT_ASSIGNED = 6019, // no_pin_cl #%s", (output)->getName()) (Spark_logic) (Doesnt work)

  CUSTOM_OBD_INJECTION_NO_PIN_ASSIGNED =6020, // no_pin_inj #%s", output->name) (engine_math)  (Dont work)
  CUSTOM_OBD_UNEXPECTED_INJECTION_MODE = 6021, // Unexpected injection mode (engine_math)
  CUSTOM_OBD_ANGLE_CONSTRAINT_VIOLATION = 6022, // angle constraint violation in findTriggerPosition
            // (trigger_tructure)
  CUSTOM_OBD_UNKNOWN_FIRING_ORDER = 6023, // How?? The firing orderss are hardcoded??
  CUSTOM_OBD_WRONG_FIRING_ORDER = 6024, // Same with this
  CUSTOM_OBD_IGNITION_MODE = 6025, // unsupported ignitionMode (engine_math) How to trigger?
  CUSTOM_UNEXPECTED_ENGINE_TYPE = 6027, // WTF??


  CUSTOM_OBD_NAN_INJECTION = 6030, // NaN injection pulse (main_trigger callback)
  CUSTOM_OBD_NEG_INJECTION = 6031,                 // Negative injection pulse (main_trigger callback)
  CUSTOM_ZERO_DWELL = 6032, // dwell is zero? (spark_logic)
  CUSTOM_DWELL_TOO_LONG = 6033,
  CUSTOM_SKIPPING_STROKE = 6034,
  CUSTOM_OBD_TRG_DECODING = 6035,
  // todo: looks like following two errors always happen together, it's just
  // timing affects which one is published?
  CUSTOM_SYNC_ERROR = 6036,
  CUSTOM_SYNC_COUNT_MISMATCH = 6037,
  /**
   * This error happens if some pinout configuration changes were applied but
   * ECU was not reset afterwards.
   */
  CUSTOM_OBD_ANALOG_INPUT_NOT_CONFIGURED = 6038,
  CUSTOM_OBD_WRONG_ADC_MODE = 6039, // Shows up instead of "No load/Wrong algo"
                                    // When I set map sensor unassigned
  CUSTOM_OBD_LOW_CAN_PERIOD = 6040, // Too low can
  CUSTOM_OBD_KNOCK_PROCESSOR = 6041,
  CUSTOM_OBD_LOCAL_FREEZE = 6042,
  CUSTOM_OBD_MMC_ERROR = 6043,
  CUSTOM_LOGGING_BUFFER_OVERFLOW = 6044,
  /**
   * This is not engine miss detection - this is only internal scheduler state
   * validation Should not happen
   */
  CUSTOM_OBD_SKIPPED_SPARK = 6045,
  CUSTOM_OBD_6047 = 6047,
  CUSTOM_OBD_PIN_CONFLICT = 6048,
  CUSTOM_OBD_LOW_FREQUENCY = 6049,

  CUSTOM_OBD_ZERO_CYLINDER_COUNT = 6051,
  CUSTOM_OBD_TS_PAGE_MISMATCH = 6052,
  CUSTOM_OBD_TS_OUTPUT_MISMATCH = 6053,
  CUSTOM_TOO_LONG_CRANKING_FUEL_INJECTION = 6054,
  CUSTOM_INTERPOLATE_NAN = 6055,
  ERROR_HISTO_NAME = 6056,
  CUSTOM_AUX_OUT_OF_ORDER = 6057,
  CUSTOM_OBD_MMC_START1 = 6060,
  CUSTOM_OBD_MMC_START2 = 6061,
  CUSTOM_OBD_70 = 6070,
CUSTOM_OBD_91 = 6091,
  CUSTOM_OBD_93 = 6093,
  CUSTOM_DWELL = 6098,
  CUSTOM_TS_OVERFLOW = 6099,
  CUSTOM_ERR_OP_MODE = 6100,
  CUSTOM_ERR_TRIGGER_ZERO = 6101,
  CUSTOM_ERR_6102 = 6102,
  CUSTOM_ERR_2ND_WATCHDOG = 6103,
  CUSTOM_ERR_INVALID_INJECTION_MODE = 6104,
  CUSTOM_ERR_WAVE_1 = 6105,
  CUSTOM_ERR_WAVE_2 = 6106,
  CUSTOM_ERR_TEST_ERROR = 6107,
  CUSTOM_ERR_IGNITION_MODE = 6108,
  CUSTOM_ERR_CAN_CONFIGURATION = 6109,

  CUSTOM_ERR_INTERPOLATE = 6110,
  CUSTOM_ERR_NOT_INITIALIZED_TRIGGER = 6111,
  CUSTOM_ERR_MAP_TYPE = 6112,
  CUSTOM_ERR_THERM = 6113,
  CUSTOM_ERR_NATURAL_LOGARITHM_ERROR = 6114,
  CUSTOM_ERR_LOOPED_QUEUE = 6115,
  CUSTOM_ERR_PWM_1 = 6116,
  CUSTOM_ERR_PWM_2 = 6117,
  CUSTOM_ERR_DWELL_DURATION = 6118,
  CUSTOM_ERR_NO_SHAPE = 6119,

  CUSTOM_ERR_SGTP_ARGUMENT = 6121,
  CUSTOM_ERR_INVALID_PIN = 6130,
  CUSTOM_ERR_PIN_REPO = 6131,
  CUSTOM_ERR_UNKNOWN_PORT = 6132,


  CUSTOM_ERR_ICU_STATE = 6135,
  CUSTOM_ERR_TCHARGE_NOT_READY = 6136,
CUSTOM_ERR_TRIGGER_WAVEFORM_TOO_LONG = 6137,
  CUSTOM_ERR_FUEL_TABLE_NOT_READY = 6138,
  CUSTOM_ERR_TCHARGE_NOT_READY2 = 6139,

  CUSTOM_ERR_COMMAND_LOWER_CASE_EXPECTED = 6140,
  CUSTOM_ERR_FLASH_CRC_FAILED = 6141,
  CUSTOM_ERR_NOT_INPUT_PIN = 6142,
  CUSTOM_ERR_SKIPPED_TOOTH_SHAPE = 6143,
  CUSTOM_ERR_UNEXPECTED_SHAFT_EVENT = 6144,
  CUSTOM_ERR_SD_MOUNT_FAILED = 6145,
  CUSTOM_ERR_SD_SEEK_FAILED = 6146,
  CUSTOM_ERR_OUT_OF_ORDER = 6147,
  CUSTOM_ERR_T2_CHARGE = 6148,

  CUSTOM_ERR_ASSERT = 6500,
  CUSTOM_ERR_ASSERT_VOID = 6501,
  ERROR_FL_STACK_OVERFLOW = 6502,
  CUSTOM_ERR_FSIO_POOL = 6503,
  CUSTOM_FLSTACK = 6504,
  CUSTOM_ERR_NAN_TCHARGE = 6505,
  CUSTOM_EGO_TYPE = 6506,
  CUSTOM_LIST_LOOP = 6507,
  CUSTOM_ERR_LOCK_ISSUE = 6508,
  CUSTOM_CONFIG_NOT_READY = 6509,
  CUSTOM_ERR_TRG_ANGLE_ORDER = 6510,
  CUSTOM_ERR_STATE_NULL = 6511,
  CUSTOM_ERR_SAME_ANGLE = 6512,
  ERROR_TRIGGER_DRAMA = 6513,
  CUSTOM_MAP_ANGLE_PARAM = 6514,
  CUSTOM_ERR_DISPLAY_MODE = 6515,
  CUSTOM_ERR_ADC_UNKNOWN_CHANNEL = 6516,
  CUSTOM_ERR_ADC_USED = 6517,
  CUSTOM_ERR_ADC_DEPTH_SLOW = 6518,
  CUSTOM_ERR_ADC_DEPTH_FAST = 6519,
  CUSTOM_ERR_ICU = 6520,
  CUSTOM_ERR_ICU_AF = 6521,
  CUSTOM_ERR_ICU_DRIVER = 6522,
  CUSTOM_ERR_ICU_PIN = 6523,
  CUSTOM_ERR_UNEXPECTED_SPI = 6524,
  CUSTOM_ERR_EXT_MODE = 6525,
  CUSTOM_ERR_TIMER_OVERFLOW = 6526,
  CUSTOM_ERR_NULL_TIMER_CALLBACK = 6527,
  CUSTOM_ERR_SCHEDULING_ERROR = 6528,
  CUSTOM_ERR_LOGGING_NOT_READY = 6529,
  ERROR_NAN_FIND_INDEX = 6530,
  ERROR_NULL_BUFFER = 6531,
  CUSTOM_ERR_BUFF_INIT_ERROR = 6532,
  CUSTOM_ERR_INTERPOLATE_PARAM = 6533,
  ERROR_LOGGING_SIZE_CALC = 6534,
  CUSTOM_ERR_ADC_CHANNEL = 6535,
  CUSTOM_ERR_ANGLE = 6536,
  CUSTOM_ERR_LOGGING_NULL = 6537,
  CUSTOM_ERR_PARSING_ERROR = 6538,
  CUSTOM_ERR_INJECTOR_LAG = 6539,
  CUSTOM_ERR_AXIS_ORDER = 6540,
  CUSTOM_HW_TIMER = 6541,
  CUSTOM_INJ_DURATION = 6542,
  CUSTOM_ADD_BASE = 6543,
  CUSTOM_ERR_MAP_AVG_OFFSET = 6691,
  CUSTOM_ANGLE_NAN = 6633,
  CUSTOM_ERR_6544 = 6544, // angle, "findAngle#a33" (engine_math)
  CUSTOM_ERR_6545 = 6545, // angle, "findAngle#a1" (Trigger structure)
  CUSTOM_UNEXPECTED_MAP_VALUE = 6548, // No MAP values (map_averaging)
  CUSTOM_ERR_6549 = 6549, // Duplicate, no map angles (map averaging), advance,
                          // "findAngle#a5 (Spark logic)
  CUSTOM_ERR_6550 = 6550, //"findAngle#a6" (park logic)
CUSTOM_TRIGGER_SYNC_ANGLE = 6551,
	CUSTOM_TRIGGER_SYNC_ANGLE2 = 6552,
  CUSTOM_ERR_6553 = 6553, // fixAngle(result, "inj offset#2" (fuel_math)
  CUSTOM_ERR_6554 = 6554, // assertAngleRange(baseAngle, "baseAngle_r" and
                          // fixAngle(angle, "addFuel#1" (engine_math)
  CUSTOM_ERR_6555 = 6555, // fixAngle2(angle, "addFuel#2" (trigger_structure)
  CUSTOM_ERR_6556 = 6556, // fixAngle(onTime, "onTime" (Aux_Valves)
  CUSTOM_ERR_6557 = 6557, // fixAngle(offTime, "offTime" (Aux_valves)
  CUSTOM_ERR_6558 = 6558, // fixAngle(vvtPosition, "vvtPosition" (Trigger_central)
CUSTOM_TRIGGER_SYNC_ANGLE_RANGE = 6559,

	CUSTOM_ERR_TRIGGER_ANGLE_RANGE = 6560,
  CUSTOM_ERR_6561 = 6561, // fixAngle(angleDiff, "angleDiff" (Trigger decoder)
  CUSTOM_ERR_6562 = 6562, // fixAngle(cylinderStart, "cylinderStart" (Map_averaging)
  CUSTOM_ERR_6563 = 6563, // assertAngleRange(samplingDuration, "samplingDuration" and
            // fixAngle(samplingEnd, "samplingEnd" (Map averaging)
  CUSTOM_ERR_6564 = 6564, // fixAngle(angle, "waveAn"(Map_averaging)
  CUSTOM_ERR_6566 = 6566, // assertAngleRange(ignitionPositionWithinEngineCycle,
                          // "aPWEC" (Spark_logic)
  CUSTOM_ERR_6567 = 6567,
  CUSTOM_ERR_6568 = 6568,
  CUSTOM_ERR_6569 = 6569,
  CUSTOM_ERR_6570 = 6570,
  CUSTOM_ERR_6571 = 6571,
  CUSTOM_ERR_ARRAY_REMOVE = 6572,
  CUSTOM_ERR_6573 = 6573,
  CUSTOM_ERR_6574 = 6574,
  CUSTOM_ERR_6575 = 6575,
  CUSTOM_ERR_6576 = 6576,
  CUSTOM_ERR_6577 = 6577,
  CUSTOM_ERR_6578 = 6578,
  CUSTOM_DUTY_TOO_LOW = 6579,
  CUSTOM_ERR_6580 = 6580,
  CUSTOM_ERR_6581 = 6581, // PWM nesting issue
  CUSTOM_ERR_6582 = 6582,
  CUSTOM_ERR_6583 = 6583,
  CUSTOM_ERR_6584 = 6584,
  CUSTOM_ERR_6585 = 6585,
  CUSTOM_ERR_6586 = 6586,
  CUSTOM_ERR_6587 = 6587,
	CUSTOM_NULL_SHAPE = 6588,
  CUSTOM_SPARK_ANGLE_1 = 6589,

  CUSTOM_ERR_6590 = 6590,
  CUSTOM_ERR_6591 = 6591,
  CUSTOM_ERR_6592 = 6592,
  CUSTOM_ERR_6593 = 6593,
  CUSTOM_SHAPE_LEN_ZERO = 6594,
	CUSTOM_TRIGGER_CYCLE = 6595,
	CUSTOM_TRIGGER_CYCLE_NAN = 6596,
  CUSTOM_OMODE_UNDEF = 6597,
  CUSTOM_ERR_6598 = 6598,
  CUSTOM_ERR_6599 = 6599,

  CUSTOM_ENGINE_REF = 6600,
  CUSTOM_CONSOLE_TOO_MANY = 6601,
  CUSTOM_APPEND_NULL = 6602,
  CUSTOM_ERR_6603 = 6603,
  CUSTOM_ERR_6604 = 6604,
  CUSTOM_ERR_6605 = 6605,
  CUSTOM_ERR_6606 = 6606,
  CUSTOM_APPEND_STACK = 6607,
  CUSTOM_RM_STACK_1 = 6608, // getCurrentRemainingStack() > 512, "init s" (rusefi.cpp)
  CUSTOM_RM_STACK = 6609, // getCurrentRemainingStack() > 512, "init s" (rusefi.cpp)
  CUSTOM_ERR_6610 = 6610, //  "NaN angle from table" (advance map) and out of
                          //  order                         (interpolation)
	CUSTOM_NULL_ENGINE_PTR = 6611,
  CUSTOM_APPLY_STACK = 6620, // getCurrentRemainingStack() > 256, "apply c"
                             // (Engine configuration)
  CUSTOM_ERR_6621 = 6621, // modePtr!=NULL, "pin mode not initialized" (Efi Gpio)
  CUSTOM_ERR_6622 = 6622, // mode <= OM_OPENDRAIN_INVERTED, "invalid
                          // pin_output_mode_e"                      (Efi Gpio)
  CUSTOM_ERR_6623 = 6623, // current->momentX <= current->next->momentX, "list
                          // order"                        (Event queue)
  CUSTOM_ERR_6624 = 6624, // getCurrentRemainingStack() > 256, "lowstck#2y"
                          // (Single_timer executor)
  CUSTOM_ERR_6625 = 6625, // nextEventTimeNt > nowNt, "setTimer constraint"
                          // (Single_timer executor)
  CUSTOM_STACK_6627 = 6627, // getCurrentRemainingStack() > 128, "lowstck#3
                            // (Main trigger callback)
  CUSTOM_ERR_6628 =
      6628, // trgEventIndex < engine->engineCycleEventCount, "handleFuel/event
            // index          (Main Trigger callback)
  CUSTOM_STACK_6629 = 6629, // getCurrentRemainingStack() > 128, "lowstck#2
                            // (Main trigger callback)
  CUSTOM_IGN_MATH_STATE = 6630, // invalid ignMathCalculateAtIndex (Main trigger callback)
  CUSTOM_ERR_6631 = 6631, // engine!=NULL, "null engine" (Main trigger callback,
                          // initMainEventListener)
  CUSTOM_ERR_6632 = 6632, // getCurrentRemainingStack() > 256, "lowstckRCL" (RPM
                          // Calkulator, rpmShaftPositionCallback)
  CUSTOM_ERR_6634 = 6634, // isValidRpm(rpm), "RPM check expected" (RPM Calkulator)
  CUSTOM_ERR_6635 = 6635, // !cisnan(delayUs), "NaN delay?" (RPM Calculator)
  CUSTOM_ERR_6636 = 6636, // getCurrentRemainingStack() > 128, "lowstck#8" (Trigger central)
  CUSTOM_CONF_NULL = 6637, // CUSTOM_CONF_NULL, engine!=NULL, "configuration"
                           // (Trigger central, handleshaftignal)
  CUSTOM_TRIGGER_EVENT_TYPE = 6638,
	CUSTOM_ERR_6639 = 6639,

	CUSTOM_TRIGGER_UNEXPECTED = 6640,
	CUSTOM_ERR_6641 = 6641,
	CUSTOM_TRIGGER_STACK = 6642,
  CUSTOM_ERR_6643 = 6643, // stateIndex < PWM_PHASE_MAX_COUNT, "invalid
                          // stateIndex"                             (Alternator)
  CUSTOM_IDLE_WAVE_CNT = 6644, // state->multiWave.waveCount == 1, "invalid idle waveCount"
  CUSTOM_ERR_6645 = 6645, // efiAssertVoid(stateIndex < PWM_PHASE_MAX_COUNT,
                          // "invalid stateIndex");
  CUSTOM_ERR_6646 = 6646, // state->multiWave.waveCount == 1, "invalid idle waveCount");
  CUSTOM_ERR_6647 = 6647, // Cylinder ID (assertcylinderid)
  CUSTOM_ERR_6648 = 6648, // injector id (setinjectorenabled)
  CUSTOM_ERR_6649 = 6649, // lowstck#9 (map averaging)
  CUSTOM_ERR_6650 = 6650, // lowstck#9a (map averaging)
  CUSTOM_ERR_6651 = 6651, // WC: NULL name (engine niffer)
  CUSTOM_ERR_6652 = 6652, // lowstck#2c (engine_sniffer)
  CUSTOM_ERR_6653 = 6653, // chart not initialized (engine_sniffer)
  CUSTOM_ERR_6654 = 6654, // wave analyzer NOT INITIALIZED
  CUSTOM_ERR_6655 = 6655, // too many ICUs
  CUSTOM_ERR_6656 = 6656, // lcdSleep true
  CUSTOM_ERR_6657 = 6657, // lcd_HD44780_set_position invalid row
  CUSTOM_ERR_6658 = 6658, // lwStAdcSlow
  CUSTOM_ERR_6659 = 6659, // lwStAdcFast
  CUSTOM_ERR_6660 = 6660, //"TriggerShape is NULL" (Trigger Vw)
  CUSTOM_ERR_6661 = 6661, // getCurrentRemainingStack() > 64, "lowStckOnEv"
                          // (engine_controller, doPeriodicSlowCallback)
  CUSTOM_ERR_6662 = 6662, // numBytes <= 2, "invalid numBytes" (Obd2.cpp)
  CUSTOM_ERR_6663 = 6663, // stateIndex < PWM_PHASE_MAX_COUNT, "invalid
                          // stateIndex" (pwm_generator_logic)
  CUSTOM_ERR_6664 = 6664, // state->multiWave.waveCount <= PWM_PHASE_MAX_WAVE_PER_PWM,
            // "invalid waveCount"           (pwm_generator_logic)
  CUSTOM_ERR_6666 = 6666, // value >=0 && value <100, "value" (rtc_helper)
  CUSTOM_ERR_ADCANCE_CALC_ANGLE = 6667,               // fixAngle(angle, "getAdvance" (advance_map)
  CUSTOM_ERR_6670 = 6670, // assertIsrContext (wave_analyzer) and index <
                          // BOUND_LENGTH, "histogram issue" (histogram)
  CUSTOM_ERR_6671 = 6671, // getCurrentRemainingStack() > 128, "lowstck#9c" (adc_inputs)
  CUSTOM_ERR_6672 = 6672, // driver != NULL, "di: driver is NULL" (digital_input_hw)
  CUSTOM_ERR_6673 = 6673, // driver->state == ICU_READY, "di: driver not ready"
                          // (digital_input_hw)
  CUSTOM_STACK_SPI = 6674,
  CUSTOM_ERR_6675 = 6675,
  CUSTOM_ERR_6676 = 6676,
  CUSTOM_IH_STACK = 6677,
  CUSTOM_EC_NULL = 6678,
  CUSTOM_ERR_6679 = 6679,
  CUSTOM_ERR_6680 = 6680,
  CUSTOM_ERR_6681 = 6681,
  CUSTOM_ERR_6682 = 6682,
  CUSTOM_SAME_TWICE = 6683,
  CUSTOM_ERR_6684 = 6684,
  CUSTOM_ERR_6685 = 6685,
  CUSTOM_ERR_6686 = 6686, // Wrong firingorder Length?? These four can never bee triggered?
  CUSTOM_ERR_6687 = 6687, // Wrong firingorder Length??
  CUSTOM_ERR_6688 = 6688, // No ignition advance
  CUSTOM_ERR_6689 = 6689,
  CUSTOM_ERR_6690 = 6690, // Duplicate
  CUSTOM_ERR_MAP_START_ASSERT = 6690,

  CUSTOM_ERR_MAP_CYL_OFFSET = 6692,
  CUSTOM_ERR_PWM_DUTY_ASSERT = 6693,
  CUSTOM_ERR_ZERO_CRANKING_FUEL = 6694,
  CUSTOM_ERR_6695 = 6695, // Pwm
  CUSTOM_ERR_6696 = 6696, // Slow not invoked yet
  CUSTOM_ERR_6697 = 6697, // invalid knock window
  CUSTOM_ERR_ARRAY_IS_FULL = 6698,
  CUSTOM_ERR_ARRAY_REMOVE_ERROR = 6699,
  CUSTOM_ERR_INVALID_INPUT_ICU_PIN = 6700,

  CUSTOM_CJ125_0 = 6700,
  CUSTOM_CJ125_1 = 6701,
  CUSTOM_CJ125_2 = 6702,
  CUSTOM_ERR_6703 = 6703,
  CUSTOM_ERR_BOTH_FRONTS_REQUIRED = 6704,
  CUSTOM_TLE8888 = 6705,
  CUSTOM_ERR_6706 = 6706,
	CUSTOM_ERR_TIMER_STATE = 6617,
	CUSTOM_ERR_ETB_TARGET = 6668,
	CUSTOM_STACK_ADC_6671 = 6671,
	CUSTOM_ICU_DRIVER = 6672,
	CUSTOM_ICU_DRIVER_STATE = 6673,
	CUSTOM_SPARK_ANGLE_9 = 6689,
	CUSTOM_NULL_EXECUTOR = 6695,
	CUSTOM_SLOW_NOT_INVOKED = 6696,
	CUSTOM_PWM_CYCLE_START = 6697,
	CUSTOM_KNOCK_WINDOW = 6706,
	CUSTOM_ERR_TIMER_TEST_CALLBACK_NOT_HAPPENED = 6707,
	CUSTOM_ERR_TIMER_TEST_CALLBACK_WRONG_TIME = 6708,
  CUSTOM_ERR_PIN_COUNT_TOO_LARGE = 6709,
  CUSTOM_DUTY_INVALID = 6710,
  CUSTOM_DUTY_TOO_HIGH = 6711,
  CUSTOM_ERR_PWM_STATE_ASSERT = 6712,
  CUSTOM_ERR_PWM_CALLBACK_ASSERT = 6713,
  CUSTOM_ERR_PWM_SWITCH_ASSERT = 6714,
CUSTOM_INVALID_ADC = 6720,
CUSTOM_ERR_TASK_TIMER_OVERFLOW = 6722,
  CUSTOM_ERR_TRIGGER_SYNC = 9000,
	CUSTOM_INVALID_MODE_SETTING = 6721,
	CUSTOM_OBD_TRIGGER_WAVEFORM = 9001,
  /**
   * This is not engine miss detection - this is only internal scheduler state
   * validation Should not happen
   */
  CUSTOM_OBD_SKIPPED_FUEL = 9010,
  CUSTOM_RE_ADDING_INTO_EXECUTION_QUEUE = 9011,
  /**
   * This indicates an issue with coil control - pin was not high when we were
   * trying to set it low.
   */
  CUSTOM_OUT_OF_ORDER_COIL = 9012,
  /**
   * Commanded fuel exceeds your fuel injector flow
   */
  CUSTOM_TOO_LONG_FUEL_INJECTION = 9013,

  // this is needed for proper enum size, this matters for malfunction_central
  Internal_ForceMyEnumIntSize_cranking_obd_code = ENUM_32_BITS,
	
} obd_code_e;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OBD_ERROR_CODES_H_ */