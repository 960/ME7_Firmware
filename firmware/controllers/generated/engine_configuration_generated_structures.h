// this section was generated automatically by rusEfi tool ConfigDefinition.jar based on gen_config.bat integration\rusefi_config.txt Sun Aug 30 19:19:41 CEST 2020
// by class com.rusefi.output.CHeaderConsumer
// begin
#ifndef CONTROLLERS_GENERATED_ENGINE_CONFIGURATION_GENERATED_STRUCTURES_H
#define CONTROLLERS_GENERATED_ENGINE_CONFIGURATION_GENERATED_STRUCTURES_H
#include "rusefi_types.h"
// start of stft_cell_cfg_s
struct stft_cell_cfg_s {
	/**
	 * offset 0
	 */
	int8_t maxAdd;
	/**
	 * offset 1
	 */
	int8_t maxRemove;
	/**
	 * offset 2
	 */
	uint16_t timeConstant;
	/** total size 4*/
};

typedef struct stft_cell_cfg_s stft_cell_cfg_s;

// start of stft_s
struct stft_s {
	/**
	 * Below this RPM, the idle region is active
	 * offset 0
	 */
	uint8_t maxIdleRegionRpm;
	/**
	 * Below this engine load, the overrun region is active
	 * offset 1
	 */
	uint8_t maxOverrunLoad;
	/**
	 * Above this engine load, the power region is active
	 * offset 2
	 */
	uint8_t minPowerLoad;
	/**
	 * When close to correct AFR, pause correction. This can improve stability by not chan the adjustment if the error is extremely small, but is not required.
	 * offset 3
	 */
	uint8_t deadband;
	/**
	 * Below this temperature, correction is disabled.
	 * offset 4
	 */
	int8_t minClt;
	/**
	 * Below this AFR, correction is paused
	 * offset 5
	 */
	uint8_t minAfr;
	/**
	 * Above this AFR, correction is paused
	 * offset 6
	 */
	uint8_t maxAfr;
	/**
	 * Delay after starting the engine before beginning closed loop correction.
	 * offset 7
	 */
	uint8_t startupDelay;
	/**
	 * offset 8
	 */
	stft_cell_cfg_s cellCfgs[STFT_CELL_COUNT];
	/** total size 24*/
};

typedef struct stft_s stft_s;

// start of pid_s
struct pid_s {
	/**
	 * offset 0
	 */
	float pFactor;
	/**
	 * offset 4
	 */
	float iFactor;
	/**
	 * offset 8
	 */
	float dFactor;
	/**
	 * Linear addition to PID logic
	 * offset 12
	 */
	int16_t offset;
	/**
	 * PID dTime
	 * offset 14
	 */
	int16_t periodMs;
	/**
	 * Output min value
	 * offset 16
	 */
	int16_t minValue;
	/**
	 * Output max value
	 * offset 18
	 */
	int16_t maxValue;
	/** total size 20*/
};

typedef struct pid_s pid_s;

// start of cranking_parameters_s
struct cranking_parameters_s {
	/**
	 * Base duration of the fuel injection during cranking, this is modified by the multipliers for CLT, IAT, TPS ect, to give the final cranking pulse width.
	 * offset 0
	 */
	float baseFuel;
	/**
	 * offset 4
	 */
	int16_t pad;
	/**
	 * This sets the RPM limit below which the ECU will use cranking fuel and ignition logic, typically this is around 350-450rpm. 
	 * set cranking_rpm X
	 * offset 6
	 */
	int16_t rpm;
	/** total size 8*/
};

typedef struct cranking_parameters_s cranking_parameters_s;

// start of gppwm_channel
struct gppwm_channel {
	/**
	 * Select a pin to use for PWM or on-off output.
	 * offset 0
	 */
	output_pin_e pin;
	/**
	 * If an error (with a sensor, etc) is detected, this value is used instead of reading from the table.
	 * This should be a safe value for whatever hardware is connected to prevent damage.
	 * offset 1
	 */
	uint8_t dutyIfError;
	/**
	 * Select a frequency to run PWM at.
	 * Set this to 0hz to enable on-off mode.
	 * offset 2
	 */
	uint16_t pwmFrequency;
	/**
	 * In on-off mode, turn the output on when the table value is above this duty.
	 * offset 4
	 */
	uint8_t onAboveDuty;
	/**
	 * In on-off mode, turn the output off when the table value is below this duty.
	 * offset 5
	 */
	uint8_t offBelowDuty;
	/**
	 * Selects the load axis to use for the table.
	 * offset 6
	 */
	gppwm_channel_e loadAxis;
	/**
	 * offset 7
	 */
	uint8_t pad;
	/**
	 * offset 8
	 */
	uint8_t loadBins[GPPWM_LOAD_COUNT];
	/**
	 * offset 16
	 */
	uint8_t rpmBins[GPPWM_RPM_COUNT];
	/**
	 * offset 24
	 */
	gppwm_table_t table;
	/** total size 88*/
};

typedef struct gppwm_channel gppwm_channel;

// start of air_pressure_sensor_config_s
struct air_pressure_sensor_config_s {
	/**
	 * kPa value at low volts
	 * offset 0
	 */
	float lowValue;
	/**
	 * kPa value at high volts
	 * offset 4
	 */
	float highValue;
	/**
	 * offset 8
	 */
	adc_channel_e hwChannel;
	/**
	 * offset 9
	 */
	uint8_t align[3];
	/** total size 12*/
};

typedef struct air_pressure_sensor_config_s air_pressure_sensor_config_s;

/**
 * @brief MAP avera configuration

*/
// start of MAP_sensor_config_s
struct MAP_sensor_config_s {
	/**
	 * offset 0
	 */
	air_pressure_sensor_config_s sensor;
	/** total size 12*/
};

typedef struct MAP_sensor_config_s MAP_sensor_config_s;

/**
 * @brief Thermistor known values

*/
// start of thermistor_conf_s
struct thermistor_conf_s {
	/**
	 * these values are in Celcius
	 * offset 0
	 */
	float tempC_1;
	/**
	 * offset 4
	 */
	float tempC_2;
	/**
	 * offset 8
	 */
	float tempC_3;
	/**
	 * offset 12
	 */
	float resistance_1;
	/**
	 * offset 16
	 */
	float resistance_2;
	/**
	 * offset 20
	 */
	float resistance_3;
	/**
	 * Pull-up resistor value on your board
	 * offset 24
	 */
	float bias_resistor;
	/** total size 28*/
};

typedef struct thermistor_conf_s thermistor_conf_s;

/**
 * @brief Oil pressure sensor interpolation

*/
// start of oil_pressure_config_s
struct oil_pressure_config_s {
	/**
	 * offset 0
	 */
	adc_channel_e hwChannel;
	/**
	 * offset 1
	 */
	uint8_t align[3];
	/**
	 * offset 4
	 */
	float v1;
	/**
	 * offset 8
	 */
	float value1;
	/**
	 * offset 12
	 */
	float v2;
	/**
	 * offset 16
	 */
	float value2;
	/** total size 20*/
};

typedef struct oil_pressure_config_s oil_pressure_config_s;

/**
 * @brief Thermistor curve parameters

*/
// start of ThermistorConf
struct ThermistorConf {
	/**
	 * offset 0
	 */
	thermistor_conf_s config;
	/**
	 * offset 28
	 */
	adc_channel_e adcChannel;
	/**
	 * offset 29
	 */
	uint8_t pad[3];
	/** total size 32*/
};

typedef struct ThermistorConf ThermistorConf;

// start of injector_s
struct injector_s {
	/**
	 * This is your injector flow at the fuel pressure used in the vehicle. cc/min, cubic centimetre per minute
	 * By the way, g/s = 0.125997881 * (lb/hr)
	 * g/s = 0.125997881 * (cc/min)/10.5
	 * g/s = 0.0119997981 * cc/min
	 * offset 0
	 */
	float size;
	/**
	 * set_flat_injector_lag LAG
	 * set_injector_lag VOLTAGE LAG
	 * offset 4
	 */
	float battLagCorrBins[VBAT_INJECTOR_CURVE_SIZE];
	/**
	 * ms delay between injector open and close dead times
	 * offset 36
	 */
	float battLagCorr[VBAT_INJECTOR_CURVE_SIZE];
	/** total size 68*/
};

typedef struct injector_s injector_s;

// start of antilag_s
struct antilag_s {
	/**
	 * This level of throttle position (TP(Main)) needs to be exceeded before Anti-Lag becomes active. Anti-Lag will stay active as long as TP(Main) is above this value. Once the throttle position falls below this value, Anti-Lag will remain active until the AL Deactivation Timeout has elapsed.
	 * offset 0
	 */
	int antiLagTpsTreshold;
	/**
	 * Minimum Coolant temperature before AntiLag arms
	 * offset 4
	 */
	int antiLagCoolantTreshold;
	/**
	 * Additional fuel when AntiLag are armed. Should only be used together with retard
	 * offset 8
	 */
	int antiLagExtraFuel;
	/**
	 * This function overrides the current position of the idle speed solenoid, E-Throttle or stepper motor. This override is used to bleed extra air into the engine when Anti-Lag is active. A standard Idle valve does generally not flow enough air for proper Anti-Lag operation, an after market upgrade is often necessary.
	 * offset 12
	 */
	int antilagExtraAir;
	/**
	 * This function specifies the time after the engine speed (RPM) falls below the AL Enable RPM AND the throttle position (TP(Main)) falls below the AL Enable TP that Anti-Lag remains active for. This is the time that the turbo will remain spooled for once stepping off the throttle.
	 * offset 16
	 */
	int antilagTimeout;
	/**
	 * Sets the current Ignition Retard value as the current Ignition Timing.
	 * offset 20
	 */
	int antiLagRetard;
	/**
	 * Sets the Boost Valve Duty when AntiLag are active
	 * offset 24
	 */
	int antiLagBoost;
	/**
	 * This engine speed (RPM) needs to be exceeded before Anti-Lag becomes active. Anti-Lag will stay active as long as the engine speed (RPM) is above this value. Once the engine speed (RPM) falls below this value, Anti-Lag will remain active until the AL Deactivation Timeout has elapsed.
	 * offset 28
	 */
	int antiLagRpmTreshold;
	/**
	 * offset 32
	 */
	antiLagAirSupply_e antiLagAirSupply;
	/**
	 * The Anti-lag system will remain OFF until the system is armed. This means Anti-Lag will not work until the arming condition(s) is met. Various methods can be used including a Digital Input, an Always ON option allowing the system to be armed permanently.
	 * offset 36
	 */
	antiLagActivationMode_e antiLagActivationMode;
	/** total size 40*/
};

typedef struct antilag_s antilag_s;

// start of specs_s
struct specs_s {
	/**
	 * Engine displacement, in litres
	 * see also cylindersCount
	 * offset 0
	 */
	float displacement;
	/**
	 * offset 4
	 */
	cylinders_count_t cylindersCount;
	/**
	 * offset 8
	 */
	firing_order_e firingOrder;
	/** total size 12*/
};

typedef struct specs_s specs_s;

/**
 * @brief Trigger wheel(s) configuration

*/
// start of trigger_config_s
struct trigger_config_s {
	/**
	 * set trigger_type X
	 * offset 0
	 */
	trigger_type_e type;
	/**
	 * offset 4
	 */
	uint8_t unused95[4];
	/**
	 * offset 8
	 */
	int numTeeth;
	/**
	 * offset 12
	 */
	int missingTeeth;
	/** total size 16*/
};

typedef struct trigger_config_s trigger_config_s;

// start of afr_sensor_s
struct afr_sensor_s {
	/**
	 * offset 0
	 */
	adc_channel_e hwChannel;
	/**
	 * offset 1
	 */
	uint8_t alignAf[3];
	/**
	 * offset 4
	 */
	float v1;
	/**
	 * offset 8
	 */
	float value1;
	/**
	 * offset 12
	 */
	float v2;
	/**
	 * offset 16
	 */
	float value2;
	/** total size 20*/
};

typedef struct afr_sensor_s afr_sensor_s;

// start of idle_hardware_s
struct idle_hardware_s {
	/**
	 * offset 0
	 */
	int solenoidFrequency;
	/**
	 * offset 4
	 */
	output_pin_e solenoidPin;
	/**
	 * offset 5
	 */
	brain_pin_e pinStepperDirection;
	/**
	 * offset 6
	 */
	brain_pin_e stepperStepPin;
	/**
	 * offset 7
	 */
	pin_output_mode_e solenoidPinMode;
	/** total size 8*/
};

typedef struct idle_hardware_s idle_hardware_s;

// start of etb_io
struct etb_io {
	/**
	 * offset 0
	 */
	brain_pin_e directionPin1;
	/**
	 * offset 1
	 */
	brain_pin_e directionPin2;
	/**
	 * offset 2
	 */
	brain_pin_e controlPin1;
	/**
	 * offset 3
	 */
	brain_pin_e disablePin;
	/** total size 4*/
};

typedef struct etb_io etb_io;

// start of launch_s
struct launch_s {
	/**
	 * offset 0
	 */
	launchActivationMode_e launchActivationMode;
	/**
	 * A secondary Rev limit engaged by the driver to help launch the vehicle faster
	 * offset 4
	 */
	int launchRpm;
	/**
	 * offset 8
	 */
	int launchTimingRetard;
	/**
	 * Disabled above this speed
	 * offset 12
	 */
	int launchSpeedTreshold;
	/**
	 * Disabled below this rpm
	 * offset 16
	 */
	int launchRpmTreshold;
	/**
	 * Range from Launch Rpm for Timing Retard to activate
	 * offset 20
	 */
	int launchAdvanceRpmRange;
	/**
	 * Extra Fuel Added
	 * offset 24
	 */
	int launchFuelAdded;
	/**
	 * Duty Cycle for the Boost Solenoid
	 * offset 28
	 */
	int launchBoostDuty;
	/**
	 * RPM Range for Hard Cut
	 * offset 32
	 */
	int hardCutRpmRange;
	/**
	 * offset 36
	 */
	int unused97;
	/**
	 * offset 40
	 */
	int launchTpsTreshold;
	/**
	 * Time in Seconds
	 * offset 44
	 */
	float launchActivateDelay;
	/** total size 48*/
};

typedef struct launch_s launch_s;

// start of vvtsettings
struct vvtsettings {
	/**
	 * offset 0
	 */
	vvtLoadAxis_e vvtLoadAxis;
	/**
	 * offset 4
	 */
	float minVvtTemperature;
	/**
	 * offset 8
	 */
	int vvtPwmFrequency;
	/**
	 * offset 12
	 */
	vvtType_e vvtType;
	/**
	 * offset 16
	 */
	pid_s vvtPid;
	/**
	 * offset 36
	 */
	int maxVvtDeviation;
	/**
	 * offset 40
	 */
	output_pin_e vvtControlPin;
	/**
	 * offset 41
	 */
	pin_output_mode_e vvtControlPinMode;
	/**
	 * offset 42
	 */
	uint8_t pad[2];
	/** total size 44*/
};

typedef struct vvtsettings vvtsettings;

// start of engine_configuration_s
struct engine_configuration_s {
	/**
	 * offset 0
	 */
	int unused81;
	/**
	 * offset 4
	 */
	injector_s injector;
	/**
	 * offset 72
	 */
	trigger_config_s trigger;
	/**
	 * offset 88
	 */
	specs_s specs;
	/**
	 * set rpm_hard_limit X
	 * offset 100
	 */
	int rpmLimit;
	/**
	 * This setting controls which fuel quantity control algorithm is used.
	 * See also useTPSAdvanceTable
	 * set algorithm X
	 * offset 104
	 */
	engine_load_mode_e fuelAlgorithm;
	/**
	 * This is the injection strategy during engine start. See Fuel/Injection settings for more detail. It is suggested to use "Simultaneous".
	 * offset 108
	 */
	injection_mode_e crankingInjectionMode;
	/**
	 * This is where the fuel injection type is defined: "Simultaneous" means all injectors will fire together at once. "Sequential" fires the injectors on a per cylinder basis, which requires individually wired injectors. "Batched" will fire the injectors in groups. If your injectors are individually wired you will also need to enable "Two wire batch emulation". 
	 * set injection_mode X
	 * See also twoWireBatchInjection
	 * offset 112
	 */
	injection_mode_e injectionMode;
	/**
	 * this is about deciding when the injector starts it's squirt
	 * See also injectionPhase map
	 * todo: do we need even need this since we have the map anyway?
	 * offset 116
	 */
	angle_t extraInjectionOffset;
	/**
	 * Ignition advance angle used during engine cranking, 5-10 degrees will work as a base setting for most engines.
	 * set cranking_timing_angle X
	 * offset 120
	 */
	angle_t fixedCrankingTiming;
	/**
	 * "One Coil" is for use on distributed ignition system. "Individual Coils" is to be used when you have one coil per cylinder (COP or similar). "Wasted" means one coil is driving two spark plugs in two cylinders, with one of the sparks not doing anything since it's happening on the exhaust cycle
	 * set ignition_mode X
	 * offset 124
	 */
	ignition_mode_e ignitionMode;
	/**
	 * this value could be used to offset the whole ignition timing table by a constant
	 * offset 128
	 */
	angle_t ignitionOffset;
	/**
	 * Dynamic uses the timing map to decide the ignition timing, Static timing fixes the timing to the value set below (only use for checking static timing).
	 * offset 132
	 */
	timing_mode_e timingMode;
	/**
	 * This value is the ignition timing used when in 'fixed timing' mode, i.e. constant timing
	 * This mode is useful when adjusting distributor location.
	 * offset 136
	 */
	angle_t fixedModeTiming;
	/**
	 * Angle between Top Dead Center (TDC) and the first trigger event.
	 * Knowing this angle allows us to control timing and other angles in reference to TDC.
	 * set global_trigger_offset_angle X
	 * offset 140
	 */
	angle_t globalTriggerAngleOffset;
	/**
	 * offset 144
	 */
	operation_mode_e ambiguousOperationMode;
	/**
	offset 148 bit 0 */
	bool enableThrottlePumps : 1;
	/**
	offset 148 bit 1 */
	bool cj125isLsu49 : 1;
	/**
	offset 148 bit 2 */
	bool etb_use_two_wires : 1;
	/**
	offset 148 bit 3 */
	bool isDoubleSolenoidIdle : 1;
	/**
	offset 148 bit 4 */
	bool enableTpsTpsAe : 1;
	/**
	offset 148 bit 5 */
	bool useTLE8888_cranking_hack : 1;
	/**
	offset 148 bit 6 */
	bool useInstantRpmForIdle : 1;
	/**
	 * If your fuel regulator does not have vacuum line
	offset 148 bit 7 */
	bool absoluteFuelPressure : 1;
	/**
	offset 148 bit 8 */
	bool launchControlEnabled : 1;
	/**
	offset 148 bit 9 */
	bool antiLagEnabled : 1;
	/**
	offset 148 bit 10 */
	bool useRunningMathForCranking : 1;
	/**
	offset 148 bit 11 */
	bool displayLogicLevelsInEngineSniffer : 1;
	/**
	offset 148 bit 12 */
	bool triggerTestMinGap : 1;
	/**
	offset 148 bit 13 */
	bool triggerTestMaxEvents : 1;
	/**
	offset 148 bit 14 */
	bool triggerDisableError : 1;
	/**
	offset 148 bit 15 */
	bool useCicPidForIdle : 1;
	/**
	offset 148 bit 16 */
	bool isCJ125Enabled : 1;
	/**
	offset 148 bit 17 */
	bool vvtCamSensorUseRise : 1;
	/**
	 * Useful for individual intakes
	offset 148 bit 18 */
	bool measureMapOnlyInOneCylinder : 1;
	/**
	offset 148 bit 19 */
	bool stepperForceParkingEveryRestart : 1;
	/**
	 * Smarter cranking logic.
	 * See also startOfCrankingPrimingPulse
	offset 148 bit 20 */
	bool isFasterEngineSpinUpEnabled : 1;
	/**
	 * This setting disables fuel injection while the engine is in overrun, this is useful as a fuel saving measure and to prevent back firing.
	offset 148 bit 21 */
	bool enableDfco : 1;
	/**
	 * This setting allows the ECU to open the IAC during overrun conditions to help reduce engine breaking, this can be helpful for large engines in light weight cars.
	offset 148 bit 22 */
	bool useIacTableForCoasting : 1;
	/**
	offset 148 bit 23 */
	bool enableTriggerFilter : 1;
	/**
	offset 148 bit 24 */
	bool useSerialPort : 1;
	/**
	 * This setting should only be used if you have a stepper motor idle valve and a stepper motor driver installed.
	offset 148 bit 25 */
	bool useStepperIdle : 1;
	/**
	offset 148 bit 26 */
	bool enableVerboseCanTx : 1;
	/**
	 *  +This will cause the alternator to be operated in a basic on or off mode, this is the simplest alternator control.
	offset 148 bit 27 */
	bool onOffAlternatorLogic : 1;
	/**
	offset 148 bit 28 */
	bool alignEngineSnifferAtTDC : 1;
	/**
	 * This setting allows the ETB to act as the idle air control valve and move to regulate the airflow at idle.
	offset 148 bit 29 */
	bool useETBforIdleControl : 1;
	/**
	offset 148 bit 30 */
	bool idleIncrementalPidCic : 1;
	/**
	offset 148 bit 31 */
	bool enableAemXSeries : 1;
	/**
	 * offset 152
	 */
	int unused99;
	/**
	 * Enables lambda sensor closed loop feedback for fuelling.
	offset 156 bit 0 */
	bool fuelClosedLoopCorrectionEnabled : 1;
	/**
	 * If set to true, will use the specified duration for cranking dwell. If set to false, will use the specified dwell angle. Unless you have a really good reason to, leave this set to true to use duration mode.
	offset 156 bit 1 */
	bool enableFixedDwellCranking : 1;
	/**
	offset 156 bit 2 */
	bool useLinearCltSensor : 1;
	/**
	offset 156 bit 3 */
	bool canReadEnabled : 1;
	/**
	offset 156 bit 4 */
	bool canWriteEnabled : 1;
	/**
	offset 156 bit 5 */
	bool useLinearIatSensor : 1;
	/**
	offset 156 bit 6 */
	bool tachPulseDurationAsDutyCycle : 1;
	/**
	 * This enables smart alternator control and activates the extra alternator settings.
	offset 156 bit 7 */
	bool enableAlternatorControl : 1;
	/**
	offset 156 bit 8 */
	bool trigger1Edge : 1;
	/**
	 * This setting flips the signal from the secondary engine speed sensor.
	offset 156 bit 9 */
	bool trigger2Edge : 1;
	/**
	offset 156 bit 10 */
	bool cutFuelOnHardLimit : 1;
	/**
	offset 156 bit 11 */
	bool cutSparkOnHardLimit : 1;
	/**
	offset 156 bit 12 */
	bool enableLaunchFuelCut : 1;
	/**
	 * This is the Cut Mode normally used
	offset 156 bit 13 */
	bool enableLaunchIgnCut : 1;
	/**
	offset 156 bit 14 */
	bool useIdleTimingPidControl : 1;
	/**
	offset 156 bit 15 */
	bool useTPSBasedVeTable : 1;
	/**
	offset 156 bit 16 */
	bool invertCamVVTSignal : 1;
	/**
	offset 156 bit 17 */
	bool todoClutchDownPinInverted : 1;
	/**
	offset 156 bit 18 */
	bool useHbridges : 1;
	/**
	offset 156 bit 19 */
	bool multisparkEnable : 1;
	/**
	offset 156 bit 20 */
	bool enableCanVss : 1;
	/**
	offset 156 bit 21 */
	bool enableInnovateLC2 : 1;
	/**
	offset 156 bit 22 */
	bool enableLaunchBoost : 1;
	/**
	 * If enabled, adjust at a constant rate instead of a rate proportional to the current lambda error. This mode may be easier to tune, and more tolerant of sensor noise. Use of this mode is required if you have a narrowband O2 sensor.
	offset 156 bit 23 */
	bool stftIgnoreErrorMagnitude : 1;
	/**
	offset 156 bit 24 */
	bool silentTriggerError : 1;
	/**
	offset 156 bit 25 */
	bool useSolenoidIdle : 1;
	/**
	offset 156 bit 26 */
	bool useSecondEtb : 1;
	/**
	offset 156 bit 27 */
	bool enableAntiLagAir : 1;
	/**
	offset 156 bit 28 */
	bool enableAntiLagFuel : 1;
	/**
	offset 156 bit 29 */
	bool syncMode : 1;
	/**
	offset 156 bit 30 */
	bool useLaunchActivateDelay : 1;
	/**
	offset 156 bit 31 */
	bool isVvtControlEnabled : 1;
	/**
	 * offset 160
	 */
	int unused75;
	/**
	offset 164 bit 0 */
	bool enableInjectors : 1;
	/**
	offset 164 bit 1 */
	bool enableIgnition : 1;
	/**
	 * When enabled if TPS is held above 95% no fuel is injected while cranking to clear excess fuel from the cylinders.
	offset 164 bit 2 */
	bool isCylinderCleanupEnabled : 1;
	/**
	offset 164 bit 3 */
	bool secondTriggerChannelEnabled : 1;
	/**
	 * This setting overrides the normal multiplication values that have been set for the idle air control valve during cranking. If this setting is enabled the "IAC multiplier" table in the Cranking settings tab needs to be adjusted appropriately or potentially no IAC opening will occur.
	offset 164 bit 4 */
	bool overrideCrankingIacSetting : 1;
	/**
	 * This activates a separate ignition timing table for idle conditions, this can help idle stability by using ignition retard and advance either side of the desired idle speed. Extra retard at low idle speeds will prevent stalling and extra advance at high idle speeds can help reduce engine power and slow the idle speed.
	offset 164 bit 5 */
	bool useSeparateAdvanceForIdle : 1;
	/**
	 * This activates a separate fuel table for Idle, this allows fine tuning of the idle fuelling.
	offset 164 bit 6 */
	bool useSeparateVeForIdle : 1;
	/**
	 * Usually if we have no trigger events that means engine is stopped
	 * Unless we are troubleshooting and spinning the engine by hand - this case a longer
	 * delay is needed
	offset 164 bit 7 */
	bool isManualSpinningMode : 1;
	/**
	 * This is needed if your coils are individually wired and you wish to use batch injection.
	 * enable two_wire_batch_injection
	offset 164 bit 8 */
	bool twoWireBatchInjection : 1;
	/**
	offset 164 bit 9 */
	bool useOnlyRisingEdgeForTrigger : 1;
	/**
	 * This is needed if your coils are individually wired (COP) and you wish to use batch ignition (wasted spark).
	offset 164 bit 10 */
	bool twoWireBatchIgnition : 1;
	/**
	offset 164 bit 11 */
	bool useFixedBaroCorrFromMap : 1;
	/**
	 * This activates a separate advance table for cranking conditions, this allows cranking advance to be RPM dependant.
	offset 164 bit 12 */
	bool enableCrankingTimingTable : 1;
	/**
	 * This enables the various ignition corrections during cranking (IAT, CLT, FSIO and PID idle).
	offset 164 bit 13 */
	bool useAdvanceCorrectionsForCranking : 1;
	/**
	 * This flag allows to use TPS for ignition lookup while in Speed Density Fuel Mode
	offset 164 bit 14 */
	bool useTPSAdvanceTable : 1;
	/**
	 * This flag allows to use a special 'PID Multiplier' table (0.0-1.0) to compensate for nonlinear nature of IAC-RPM controller
	offset 164 bit 15 */
	bool useIacPidMultTable : 1;
	/**
	offset 164 bit 16 */
	bool isBoostControlEnabled : 1;
	/**
	 * Interpolates the Ignition Retard from 0 to 100% within the RPM Range
	offset 164 bit 17 */
	bool launchSmoothRetard : 1;
	/**
	offset 164 bit 18 */
	bool verboseTriggerSynchDetails : 1;
	/**
	offset 164 bit 19 */
	bool sensorType : 1;
	/**
	 * Used on some German vehicles around late 90s: cable-operated throttle and DC motor idle air valve.
	 * Set the primary TPS to the cable-operated throttle's sensor
	 * Set the secondary TPS to the mini ETB's position sensor(s).
	offset 164 bit 20 */
	bool dcMotorIdleValve : 1;
	/**
	offset 164 bit 21 */
	bool enableAntiLagBoost : 1;
	/**
	offset 164 bit 22 */
	bool enableAntiLagRetard : 1;
	/**
	offset 164 bit 23 */
	bool launchDisableBySpeed : 1;
	/**
	offset 164 bit 24 */
	bool enableLaunchRetard : 1;
	/**
	offset 164 bit 25 */
	bool tuneCj125Pid : 1;
	/**
	offset 164 bit 26 */
	bool cj125AutoChangeMode : 1;
	/**
	offset 164 bit 27 */
	bool diagnoseTLE8888 : 1;
	/**
	offset 164 bit 28 */
	bool enableLaunchFuel : 1;
	/**
	offset 164 bit 29 */
	bool invertLaunchSwitch : 1;
	/**
	offset 164 bit 30 */
	bool isVerboseTriggerSynchDetails : 1;
	/**
	offset 164 bit 31 */
	bool verboseVVTDecoding : 1;
	/**
	 * Useful in Research&Development phase
	 * offset 168
	 */
	adc_channel_e auxFastSensor1_adcChannel;
	/**
	 * First throttle body, second sensor.
	 * offset 169
	 */
	adc_channel_e tps1_2AdcChannel;
	/**
	 * Second throttle body, second sensor.
	 * offset 170
	 */
	adc_channel_e tps2_2AdcChannel;
	/**
	 * Electronic throttle pedal position input
	 * Second channel
	 * See also tps1_1AdcChannel
	 * offset 171
	 */
	adc_channel_e throttlePedalPositionSecondAdcChannel;
	/**
	 * First throttle body, first sensor. See also pedalPositionAdcChannel
	 * offset 172
	 */
	adc_channel_e tps1_1AdcChannel;
	/**
	 * This is the processor input pin that the battery voltage circuit is connected to, if you are unsure of what pin to use, check the schematic that corresponds to your PCB.
	 * offset 173
	 */
	adc_channel_e vbattAdcChannel;
	/**
	 * This is the processor pin that your fuel level sensor in connected to. This is a non standard input so will need to be user defined.
	 * offset 174
	 */
	adc_channel_e fuelLevelSensor;
	/**
	 * Second throttle body position sensor, single channel so far
	 * set_analog_input_pin tps2 X
	 * offset 175
	 */
	adc_channel_e tps2_1AdcChannel;
	/**
	 * offset 176
	 */
	adc_channel_e high_fuel_pressure_sensor_1;
	/**
	 * offset 177
	 */
	adc_channel_e high_fuel_pressure_sensor_2;
	/**
	 * See hasMafSensor
	 * offset 178
	 */
	adc_channel_e mafAdcChannel;
	/**
	 * Electronic throttle pedal position input
	 * First channel
	 * See also tps1_1AdcChannel
	 * set_analog_input_pin pps X
	 * offset 179
	 */
	adc_channel_e throttlePedalPositionAdcChannel;
	/**
	 * offset 180
	 */
	uint8_t unused120[2];
	/**
	 * offset 182
	 */
	adc_channel_e auxVoltage1;
	/**
	 * offset 183
	 */
	adc_channel_e auxVoltage2;
	/**
	 * offset 184
	 */
	adc_channel_e externalKnockSenseAdc;
	/**
	 * offset 185
	 */
	adc_channel_e hipOutputChannel;
	/**
	 * A/C button input handled as analogue input
	 * offset 186
	 */
	adc_channel_e acSwitchAdc;
	/**
	 * offset 187
	 */
	adc_channel_e vRefAdcChannel;
	/**
	 * offset 188
	 */
	output_pin_e pinInjector[INJECTION_PIN_COUNT];
	/**
	 * offset 200
	 */
	output_pin_e pinCoil[IGNITION_PIN_COUNT];
	/**
	 * offset 212
	 */
	output_pin_e pinAlternator;
	/**
	 * offset 213
	 */
	output_pin_e pinFuelPump;
	/**
	 * offset 214
	 */
	output_pin_e pinFan;
	/**
	 * offset 215
	 */
	output_pin_e pinMainRelay;
	/**
	 * offset 216
	 */
	output_pin_e pinAcRelay;
	/**
	 * This implementation produces one pulse per engine cycle. See also dizzySparkOutputPin.
	 * offset 217
	 */
	output_pin_e pinTacho;
	/**
	 * offset 218
	 */
	output_pin_e pinStartRelay;
	/**
	 * offset 219
	 */
	output_pin_e boostControlPin;
	/**
	 * See also startStopButtonPin
	 * offset 220
	 */
	output_pin_e starterControlPin;
	/**
	 * offset 221
	 */
	uint8_t unused104[3];
	/**
	 * offset 224
	 */
	pin_output_mode_e pinTachoMode;
	/**
	 * offset 225
	 */
	pin_output_mode_e pinFuelPumpMode;
	/**
	 * offset 226
	 */
	pin_output_mode_e pinInjectorMode;
	/**
	 * offset 227
	 */
	pin_output_mode_e sparkEdge;
	/**
	 * offset 228
	 */
	pin_output_mode_e pinFanMode;
	/**
	 * offset 229
	 */
	pin_output_mode_e pinAlternatorMode;
	/**
	 * offset 230
	 */
	pin_output_mode_e pinMainRelayMode;
	/**
	 * offset 231
	 */
	pin_output_mode_e pinAcRelayMode;
	/**
	 * On some vehicles we can disable starter once engine is already running
	 * offset 232
	 */
	pin_output_mode_e starterRelayDisableMode;
	/**
	 * offset 233
	 */
	pin_output_mode_e pinStepperDirectionMode;
	/**
	 * offset 234
	 */
	pin_output_mode_e boostControlPinMode;
	/**
	 * offset 235
	 */
	pin_output_mode_e pinStepperEnableMode;
	/**
	 * offset 236
	 */
	uint8_t unused17[3];
	/**
	 * offset 239
	 */
	brain_pin_e pinStepperEnable;
	/**
	offset 240 bit 0 */
	bool enableSoftwareKnock : 1;
	/**
	offset 240 bit 1 */
	bool unusedBit_160_1 : 1;
	/**
	offset 240 bit 2 */
	bool unusedBit_160_2 : 1;
	/**
	offset 240 bit 3 */
	bool unusedBit_160_3 : 1;
	/**
	offset 240 bit 4 */
	bool unusedBit_160_4 : 1;
	/**
	offset 240 bit 5 */
	bool unusedBit_160_5 : 1;
	/**
	offset 240 bit 6 */
	bool unusedBit_160_6 : 1;
	/**
	offset 240 bit 7 */
	bool unusedBit_160_7 : 1;
	/**
	offset 240 bit 8 */
	bool unusedBit_160_8 : 1;
	/**
	offset 240 bit 9 */
	bool unusedBit_160_9 : 1;
	/**
	offset 240 bit 10 */
	bool unusedBit_160_10 : 1;
	/**
	offset 240 bit 11 */
	bool unusedBit_160_11 : 1;
	/**
	offset 240 bit 12 */
	bool unusedBit_160_12 : 1;
	/**
	offset 240 bit 13 */
	bool unusedBit_160_13 : 1;
	/**
	offset 240 bit 14 */
	bool unusedBit_160_14 : 1;
	/**
	offset 240 bit 15 */
	bool unusedBit_160_15 : 1;
	/**
	offset 240 bit 16 */
	bool unusedBit_160_16 : 1;
	/**
	offset 240 bit 17 */
	bool unusedBit_160_17 : 1;
	/**
	offset 240 bit 18 */
	bool unusedBit_160_18 : 1;
	/**
	offset 240 bit 19 */
	bool unusedBit_160_19 : 1;
	/**
	offset 240 bit 20 */
	bool unusedBit_160_20 : 1;
	/**
	offset 240 bit 21 */
	bool unusedBit_160_21 : 1;
	/**
	offset 240 bit 22 */
	bool unusedBit_160_22 : 1;
	/**
	offset 240 bit 23 */
	bool unusedBit_160_23 : 1;
	/**
	offset 240 bit 24 */
	bool unusedBit_160_24 : 1;
	/**
	offset 240 bit 25 */
	bool unusedBit_160_25 : 1;
	/**
	offset 240 bit 26 */
	bool unusedBit_160_26 : 1;
	/**
	offset 240 bit 27 */
	bool unusedBit_160_27 : 1;
	/**
	offset 240 bit 28 */
	bool unusedBit_160_28 : 1;
	/**
	offset 240 bit 29 */
	bool unusedBit_160_29 : 1;
	/**
	offset 240 bit 30 */
	bool unusedBit_160_30 : 1;
	/**
	offset 240 bit 31 */
	bool unusedBit_160_31 : 1;
	/**
	 * offset 244
	 */
	int unused117;
	/**
	 * offset 248
	 */
	brain_pin_e canTxPin;
	/**
	 * offset 249
	 */
	brain_pin_e canRxPin;
	/**
	 * Some Subaru and some Mazda use double-solenoid idle air valve
	 * offset 250
	 */
	brain_pin_e secondSolenoidPin;
	/**
	 * offset 251
	 */
	brain_pin_e binarySerialRxPin;
	/**
	 * offset 252
	 */
	brain_pin_e binarySerialTxPin;
	/**
	 * offset 253
	 */
	uint8_t unused111[5];
	/**
	 * offset 258
	 */
	output_pin_e triggerErrorPin;
	/**
	 * offset 259
	 */
	output_pin_e debugTriggerSync;
	/**
	 * offset 260
	 */
	pin_output_mode_e triggerErrorPinMode;
	/**
	 * offset 261
	 */
	pin_output_mode_e debugTriggerSyncMode;
	/**
	 * set_aux_tx_pin X
	 * offset 262
	 */
	brain_pin_e auxSerialTxPin;
	/**
	 * set_aux_rx_pin X
	 * offset 263
	 */
	brain_pin_e auxSerialRxPin;
	/**
	 * offset 264
	 */
	switch_input_pin_e tcuUpshiftButtonPin;
	/**
	 * offset 265
	 */
	switch_input_pin_e tcuDownshiftButtonPin;
	/**
	 * Some vehicles have a switch to indicate that clutch pedal is all the way up
	 * offset 266
	 */
	switch_input_pin_e clutchUpPin;
	/**
	 * offset 267
	 */
	switch_input_pin_e antiLagActivatePin;
	/**
	 * offset 268
	 */
	switch_input_pin_e launchActivatePin;
	/**
	 * Brake pedal switch
	 * offset 269
	 */
	switch_input_pin_e brakePedalPin;
	/**
	 * Throttle Pedal not pressed switch - used on some older vehicles like early Mazda Miata
	 * offset 270
	 */
	switch_input_pin_e throttlePedalUpPin;
	/**
	 * some cars have a switch to indicate that clutch pedal is all the way down
	 * offset 271
	 */
	switch_input_pin_e clutchDownPin;
	/**
	 * See also starterControlPin
	 * offset 272
	 */
	switch_input_pin_e startStopButtonPin;
	/**
	 * offset 273
	 */
	uint8_t unused105[3];
	/**
	 * offset 276
	 */
	pin_input_mode_e startStopButtonMode;
	/**
	 * offset 277
	 */
	pin_input_mode_e clutchDownPinMode;
	/**
	 * offset 278
	 */
	pin_input_mode_e clutchUpPinMode;
	/**
	 * offset 279
	 */
	pin_input_mode_e brakePedalPinMode;
	/**
	 * offset 280
	 */
	pin_input_mode_e throttlePedalUpPinMode;
	/**
	 * offset 281
	 */
	uint8_t unused108[3];
	/**
	 * Camshaft input could be used either just for engine phase detection if your trigger shape does not include cam sensor as 'primary' channel, or it could be used for Variable Valve timing on one of the camshafts.
	 * TODO #660
	 * offset 284
	 */
	brain_input_pin_e pinCam[CAM_INPUTS_COUNT];
	/**
	 * offset 288
	 */
	brain_input_pin_e pinTrigger[TRIGGER_INPUT_PIN_COUNT];
	/**
	 * offset 291
	 */
	uint8_t unused109;
	/**
	 * offset 292
	 */
	brain_input_pin_e vehicleSpeedSensorInputPin;
	/**
	 * offset 293
	 */
	uint8_t unused54[3];
	/**
	 * offset 296
	 */
	etb_io etbIo[ETB_COUNT];
	/**
	 * offset 304
	 */
	etb_io etbIo2[ETB_COUNT];
	/**
	 * offset 312
	 */
	int unused107;
	/**
	 * offset 316
	 */
	debug_mode_e debugMode;
	/**
	 * offset 320
	 */
	cranking_parameters_s cranking;
	/**
	 * offset 328
	 */
	float primingSquirtDurationMs;
	/**
	 * Used if enableFixedDwellCranking is TRUE
	 * offset 332
	 */
	float ignitionDwellForCrankingMs;
	/**
	 * While cranking (which causes battery voltage to drop) we can calculate dwell time in shaft
	 * degrees, not in absolute time as in running mode.
	 * set cranking_charge_angle X
	 * offset 336
	 */
	float crankingChargeAngle;
	/**
	 * offset 340
	 */
	MAP_sensor_config_s map;
	/**
	 * todo: merge with channel settings, use full-scale Thermistor here!
	 * offset 352
	 */
	ThermistorConf clt;
	/**
	 * offset 384
	 */
	ThermistorConf iat;
	/**
	 * todo: finish implementation #332
	 * offset 416
	 */
	ThermistorConf auxTempSensor1;
	/**
	 * todo: finish implementation #332
	 * offset 448
	 */
	ThermistorConf auxTempSensor2;
	/**
	 * offset 480
	 */
	afr_sensor_s afr;
	/**
	 * @see hasBaroSensor
	 * offset 500
	 */
	air_pressure_sensor_config_s baroSensor;
	/**
	 * AFR, WBO, EGO - whatever you like to call it
	 * offset 512
	 */
	ego_sensor_e afr_type;
	/**
	 * Ratio/coefficient of input voltage dividers on your PCB. For example, use '2' if your board divides 5v into 2.5v. Use '1.66' if your board divides 5v into 3v.
	 * offset 516
	 */
	float analogInputDividerCoefficient;
	/**
	 * This is the ratio of the resistors for the battery voltage, measure the voltage at the battery and then adjust this number until the gauge matches the reading.
	 * offset 520
	 */
	float vbattDividerCoeff;
	/**
	 * Cooling fan turn-on temperature threshold, in Celsius
	 * offset 524
	 */
	float fanOnTemperature;
	/**
	 * Cooling fan turn-off temperature threshold, in Celsius
	 * offset 528
	 */
	float fanOffTemperature;
	/**
	 * This coefficient translates vehicle speed input frequency (in Hz) into vehicle speed, km/h
	 * offset 532
	 */
	float vehicleSpeedCoef;
	/**
	 * set can_mode X
	 * offset 536
	 */
	can_nbc_e canNbcType;
	/**
	 * offset 540
	 */
	int unused122;
	/**
	 * CANbus thread period, ms
	 * offset 544
	 */
	int canSleepPeriodMs;
	/**
	 * offset 548
	 */
	int unused123;
	/**
	 * Same RPM is used for two ways of producing simulated RPM. See also triggerSimulatorPins (with wires)
	 * See also directSelfStimulation (no wires, bypassing input hardware)
	 * rpm X
	 * offset 552
	 */
	int triggerSimulatorFrequency;
	/**
	 * offset 556
	 */
	int boostPwmFrequency;
	/**
	 * offset 560
	 */
	int alternatorPwmFrequency;
	/**
	 * At what trigger index should some ignition-related math be executed? This is a performance trick to reduce load on synchronization trigger callback.
	 * offset 564
	 */
	int ignMathCalculateAtIndex;
	/**
	 * set global_fuel_correction X
	 * offset 568
	 */
	float globalFuelCorrection;
	/**
	 * offset 572
	 */
	float adcVcc;
	/**
	 * maximum total number of degrees to subtract from ignition advance
	 * when knocking
	 * offset 576
	 */
	float maxKnockSubDeg;
	/**
	 * value between 0 and 100 used in Manual mode
	 * offset 580
	 */
	float manIdlePosition;
	/**
	 * offset 584
	 */
	float fuelLevelEmptyTankVoltage;
	/**
	 * offset 588
	 */
	float fuelLevelFullTankVoltage;
	/**
	 * offset 592
	 */
	idle_hardware_s idle;
	/**
	 * offset 600
	 */
	uint32_t tunerStudioSerialSpeed;
	/**
	 * offset 604
	 */
	uint32_t verboseCanBaseAddress;
	/**
	 * offset 608
	 */
	uint8_t mc33_hvolt;
	/**
	 * offset 609
	 */
	uint8_t multisparkMaxSparkingAngle;
	/**
	 * offset 610
	 */
	uint8_t multisparkMaxExtraSparkCount;
	/**
	 * offset 611
	 */
	uint8_t tachPulsePerRev;
	/**
	 * offset 612
	 */
	int16_t idlePidDeactivationTpsThreshold;
	/**
	 * offset 614
	 */
	int16_t stepperParkingExtraSteps;
	/**
	 * Closed throttle. todo: extract these two fields into a structure
	 * See also tps1_1AdcChannel
	 * set tps_min X
	 * offset 616
	 */
	int16_t tpsMin;
	/**
	 * Full throttle. tpsMax value as 10 bit ADC value. Not Voltage!
	 * See also tps1_1AdcChannel
	 * set tps_max X
	 * offset 618
	 */
	int16_t tpsMax;
	/**
	 * offset 620
	 */
	uint16_t tps2SecondaryMin;
	/**
	 * offset 622
	 */
	uint16_t tps2SecondaryMax;
	/**
	 * TPS error detection, what TPS % value is unrealistically low
	 * offset 624
	 */
	int16_t tpsErrorDetectionTooLow;
	/**
	 * TPS error detection, what TPS % value is unrealistically high
	 * offset 626
	 */
	int16_t tpsErrorDetectionTooHigh;
	/**
	 * offset 628
	 */
	uint16_t tps1SecondaryMin;
	/**
	 * offset 630
	 */
	uint16_t tps1SecondaryMax;
	/**
	 * offset 632
	 */
	int16_t unusedantiLagRpmTreshold;
	/**
	 * Maximum time to crank starter
	 * offset 634
	 */
	int16_t startCrankingDuration;
	/**
	 * offset 636
	 */
	uint16_t multisparkMaxRpm;
	/**
	 * offset 638
	 */
	int16_t acCutoffLowRpm;
	/**
	 * offset 640
	 */
	int16_t acCutoffHighRpm;
	/**
	 * offset 642
	 */
	int16_t acIdleRpmBump;
	/**
	 * set warningPeriod X
	 * offset 644
	 */
	int16_t warningPeriod;
	/**
	 * offset 646
	 */
	int16_t etbFreq;
	/**
	 * offset 648
	 */
	pid_s idleRpmPid2;
	/**
	 * See useIdleTimingPidControl
	 * offset 668
	 */
	pid_s idleTimingPid;
	/**
	 * See cltIdleRpmBins
	 * offset 688
	 */
	pid_s idleRpmPid;
	/**
	 * offset 708
	 */
	pid_s alternatorControl;
	/**
	 * offset 728
	 */
	pid_s etb;
	/**
	 * offset 748
	 */
	pid_s boostPid;
	/**
	 * See also idleRpmPid
	 * offset 768
	 */
	idle_mode_e idleMode;
	/**
	 * offset 772
	 */
	boostType_e boostType;
	/**
	 * offset 776
	 */
	stft_s stft;
	/**
	 * offset 800
	 */
	antilag_s antiLag;
	/**
	 * offset 840
	 */
	vvtsettings vvt;
	/**
	 * offset 884
	 */
	launch_s launch;
	/**
	 * Relative to the target idle RPM
	 * offset 932
	 */
	int16_t idlePidRpmUpperLimit;
	/**
	 * This sets the temperature above which no priming pulse is used, The value at -40 is reduced until there is no more priming injection at this temperature.
	 * offset 934
	 */
	int16_t primeInjFalloffTemperature;
	/**
	 * offset 936
	 */
	float knockDetectionWindowStart;
	/**
	 * offset 940
	 */
	float knockDetectionWindowEnd;
	/**
	 * offset 944
	 */
	float idleStepperReactionTime;
	/**
	 * offset 948
	 */
	float knockVThreshold;
	/**
	 * offset 952
	 */
	int idleStepperTotalSteps;
	/**
	 * TODO: finish this #413
	 * offset 956
	 */
	float noAccelAfterHardLimitPeriodSecs;
	/**
	 * offset 960
	 */
	float tachPulseDuractionMs;
	/**
	 * Trigger cycle index at which we start tach pulse (performance consideration)
	 * offset 964
	 */
	int tachPulseTriggerIndex;
	/**
	 * Length of time the deposited wall fuel takes to dissipate after the start of acceleration. 
	 * offset 968
	 */
	float wwaeTau;
	/**
	 * offset 972
	 */
	float fuelRailPressure;
	/**
	 * offset 976
	 */
	float alternator_derivativeFilterLoss;
	/**
	 * offset 980
	 */
	float alternator_antiwindupFreq;
	/**
	 * Closed throttle#2. todo: extract these two fields into a structure
	 * See also tps2_1AdcChannel
	 * set tps2_min X
	 * offset 984
	 */
	int16_t tps2Min;
	/**
	 * Full throttle#2. tpsMax value as 10 bit ADC value. Not Voltage!
	 * See also tps1_1AdcChannel
	 * set tps2_max X
	 * offset 986
	 */
	int16_t tps2Max;
	/**
	 * kPa value which is too low to be true
	 * offset 988
	 */
	float mapErrorDetectionTooLow;
	/**
	 * kPa value which is too high to be true
	 * offset 992
	 */
	float mapErrorDetectionTooHigh;
	/**
	 * offset 996
	 */
	uint16_t multisparkSparkDuration;
	/**
	 * offset 998
	 */
	uint16_t multisparkDwell;
	/**
	 * 0 = No fuel settling on port walls 1 = All the fuel settling on port walls setting this to 0 disables the wall wetting enrichment. 
	 * offset 1000
	 */
	float wwaeBeta;
	/**
	 * offset 1004
	 */
	float throttlePedalUpVoltage;
	/**
	 * Pedal in the floor
	 * offset 1008
	 */
	float throttlePedalWOTVoltage;
	/**
	 * on ECU start turn fuel pump on to build fuel pressure
	 * offset 1012
	 */
	int16_t startUpFuelPumpDuration;
	/**
	 * If RPM is close enough let's leave IAC alone, and maybe engage timing PID correction
	 * offset 1014
	 */
	int16_t idlePidRpmDeadZone;
	/**
	 * This is the target battery voltage the alternator PID control will attempt to maintain
	 * offset 1016
	 */
	float targetVBatt;
	/**
	 * Turns off alternator output above specified TPS, enabling this reduced parasitic drag on the engine at full load.
	 * offset 1020
	 */
	float alternatorOffAboveTps;
	/**
	 * Prime pulse for cold engine, duration in ms
	 * Linear interpolation between -40F/-40C and fallout temperature
	 * 
	 * See also isFasterEngineSpinUpEnabled
	 * set cranking_priming_pulse X
	 * offset 1024
	 */
	float startOfCrankingPrimingPulse;
	/**
	 * This is the duration in cycles that the IAC will take to reach its normal idle position, it can be used to hold the idle higher for a few seconds after cranking to improve startup.
	 * offset 1028
	 */
	int16_t afterCrankingIACtaperDuration;
	/**
	 * Extra IAC, in percent between 0 and 100, tapered between zero and idle deactivation TPS value
	 * offset 1030
	 */
	int16_t iacByTpsTaper;
	/**
	 * This is the number of engine cycles that the TPS position change can occur over, a longer duration will make the enrichment more active but too long may affect steady state driving, a good default is 30-60 cycles. 
	 * offset 1032
	 */
	int tpsAccelLength;
	/**
	 * TPS change in % per engine cycle.
	 * offset 1036
	 */
	float maxDeltaTps;
	/**
	 * offset 1040
	 */
	int engineLoadAccelLength;
	/**
	 * offset 1044
	 */
	float loadBasedAeMaxEnleanment;
	/**
	 * offset 1048
	 */
	float engineLoadDecelEnleanmentMultiplier;
	/**
	 * offset 1052
	 */
	float loadBasedAeMaxEnrich;
	/**
	 * offset 1056
	 */
	float loadBasedAeMult;
	/**
	 * offset 1060
	 */
	uint32_t uartConsoleSerialSpeed;
	/**
	 * offset 1064
	 */
	float maxDeltaTpsEnlean;
	/**
	 * offset 1068
	 */
	float tpsTpsEnleanFactor;
	/**
	 * ExpAverage alpha coefficient
	 * offset 1072
	 */
	float slowAdcAlpha;
	/**
	 * offset 1076
	 */
	uint32_t auxSerialSpeed;
	/**
	 * offset 1080
	 */
	float throttlePedalSecondaryUpVoltage;
	/**
	 * Pedal in the floor
	 * offset 1084
	 */
	float throttlePedalSecondaryWOTVoltage;
	/**
	 *  set can_baudrate
	 * offset 1088
	 */
	can_baudrate_e canBaudRate;
	/**
	 * offset 1089
	 */
	uint8_t unused101[3];
	/**
	 * kPa value at which we need to cut fuel and spark, 0 if not enabled
	 * offset 1092
	 */
	float boostCutPressure;
	/**
	 * Fixed timing, useful for TDC testing
	 * offset 1096
	 */
	float fixTiming;
	/**
	 * MAP voltage for low point
	 * offset 1100
	 */
	float mapLowValueVoltage;
	/**
	 * MAP voltage for low point
	 * offset 1104
	 */
	float mapHighValueVoltage;
	/**
	 * EGO value correction
	 * offset 1108
	 */
	float egoValueShift;
	/**
	 * This is the IAC position during cranking, some engines start better if given more air during cranking to improve cylinder filling.
	 * offset 1112
	 */
	int crankingIACposition;
	/**
	 * offset 1116
	 */
	float tChargeMinRpmMinTps;
	/**
	 * offset 1120
	 */
	float tChargeMinRpmMaxTps;
	/**
	 * offset 1124
	 */
	float tChargeMaxRpmMinTps;
	/**
	 * offset 1128
	 */
	float tChargeMaxRpmMaxTps;
	/**
	 * Narrow Band WBO Approximation
	 * offset 1132
	 */
	float narrowToWideOxygenBins[NARROW_BAND_WIDE_BAND_CONVERSION_SIZE];
	/**
	 * offset 1164
	 */
	float narrowToWideOxygen[NARROW_BAND_WIDE_BAND_CONVERSION_SIZE];
	/**
	 * set vvt_mode X
	 * offset 1196
	 */
	vvt_mode_e vvtMode;
	/**
	 * offset 1200
	 */
	float autoTuneCltThreshold;
	/**
	 * offset 1204
	 */
	float autoTuneTpsRocThreshold;
	/**
	 * offset 1208
	 */
	float autoTuneTpsQuietPeriod;
	/**
	 * Fuel multiplier taper, see also postCrankingDurationSec
	 * offset 1212
	 */
	float postCrankingFactor;
	/**
	 * See also postCrankingFactor
	 * offset 1216
	 */
	float postCrankingDurationSec;
	/**
	 * per-cylinder timing correction
	 * offset 1220
	 */
	cfg_float_t_1f timing_offset_cylinder[IGNITION_PIN_COUNT];
	/**
	 * offset 1268
	 */
	float idlePidActivationTime;
	/**
	 * offset 1272
	 */
	oil_pressure_config_s oilPressure;
	/**
	 * This sets the RPM limit above which the fuel cut is deactivated, activating this maintains fuel flow at high RPM to help cool pistons
	 * offset 1292
	 */
	int16_t rpmMaxDfco;
	/**
	 * This sets the RPM limit below which the fuel cut is deactivated, this prevents jerking or issues transitioning to idle
	 * offset 1294
	 */
	int16_t rpmMinDfco;
	/**
	 * percent between 0 and 100 below which the fuel cut is deactivated, this helps low speed drivability.
	 * offset 1296
	 */
	int16_t tpsTresholdDfco;
	/**
	 * Fuel cutoff is deactivated below this coolant threshold.
	 * offset 1298
	 */
	int16_t cltTresholdDfco;
	/**
	 * Increases PID reaction for RPM<target by adding extra percent to PID-error
	 * offset 1300
	 */
	int16_t pidExtraForLowRpm;
	/**
	 * MAP value above which fuel injection is re-enabled.
	 * offset 1302
	 */
	int16_t mapTresholdDfco;
	/**
	 * offset 1304
	 */
	float tChargeAirCoefMin;
	/**
	 * offset 1308
	 */
	float tChargeAirCoefMax;
	/**
	 * offset 1312
	 */
	float tChargeAirFlowMax;
	/**
	 * offset 1316
	 */
	float tChargeAirIncrLimit;
	/**
	 * offset 1320
	 */
	float tChargeAirDecrLimit;
	/**
	 * offset 1324
	 */
	tChargeMode_e tChargeMode;
	/**
	 * iTerm min value
	 * offset 1328
	 */
	int16_t etb_iTermMin;
	/**
	 * iTerm max value
	 * offset 1330
	 */
	int16_t etb_iTermMax;
	/**
	 * offset 1332
	 */
	float etbDeadband;
	/**
	 * When the current RPM is closer than this value to the target, closed-loop idle timing control is enabled.
	 * offset 1336
	 */
	int16_t idleTimingPidWorkZone;
	/**
	 * If the RPM closer to target than this value, disable timing correction to prevent oscillation
	 * offset 1338
	 */
	int16_t idleTimingPidDeadZone;
	/**
	 * Taper out idle timing control over this range as the engine leaves idle conditions
	 * offset 1340
	 */
	int16_t idlePidFalloffDeltaRpm;
	/**
	 * A delay in cycles between fuel-enrich. portions
	 * offset 1342
	 */
	int16_t tpsAccelFractionCycles;
	/**
	 * A fraction divisor: 100 or less = entire portion at once, or split into diminishing fractions
	 * offset 1344
	 */
	float tpsAccelFractionDivisor;
	/**
	 * iTerm min value
	 * offset 1348
	 */
	int16_t idlerpmpid_iTermMin;
	/**
	 * iTerm max value
	 * offset 1350
	 */
	int16_t idlerpmpid_iTermMax;
	/**
	 *  ETB idle authority
	 * offset 1352
	 */
	float etbIdleThrottleRange;
	/**
	 * Trigger comparator center point voltage
	 * offset 1356
	 */
	uint8_t triggerCompCenterVolt;
	/**
	 * Trigger comparator hysteresis voltage (Min)
	 * offset 1357
	 */
	uint8_t triggerCompHystMin;
	/**
	 * Trigger comparator hysteresis voltage (Max)
	 * offset 1358
	 */
	uint8_t triggerCompHystMax;
	/**
	 * VR-sensor saturation RPM
	 * offset 1359
	 */
	uint8_t triggerCompSensorSatRpm;
	/**
	 * set can_vss X
	 * offset 1360
	 */
	can_vss_nbc_e canVssNbcType;
	/**
	 * offset 1364
	 */
	gppwm_channel gppwm[GPPWM_CHANNELS];
	/**
	 * offset 1716
	 */
	uint16_t mc33_i_boost;
	/**
	 * offset 1718
	 */
	uint16_t mc33_i_peak;
	/**
	 * offset 1720
	 */
	uint16_t mc33_i_hold;
	/**
	 * offset 1722
	 */
	uint16_t mc33_t_max_boost;
	/**
	 * offset 1724
	 */
	uint16_t mc33_t_peak_off;
	/**
	 * offset 1726
	 */
	uint16_t mc33_t_peak_tot;
	/**
	 * offset 1728
	 */
	uint16_t mc33_t_bypass;
	/**
	 * offset 1730
	 */
	uint16_t mc33_t_hold_off;
	/**
	 * offset 1732
	 */
	uint16_t mc33_t_hold_tot;
	/**
	 * offset 1734
	 */
	uint16_t unused102;
	/**
	 * offset 1736
	 */
	float vvtToothMinAngle;
	/**
	 * offset 1740
	 */
	float vvtToothMaxAngle;
	/**
	 * Angle between cam sensor and VVT zero position
	 * set vvt_offset X
	 * offset 1744
	 */
	float vvtOffset;
	/**
	 * On single-coil or wasted spark setups you have to lower dwell at high RPM
	 * offset 1748
	 */
	float sparkDwellRpmBins[DWELL_CURVE_SIZE];
	/**
	 * offset 1780
	 */
	float sparkDwellValues[DWELL_CURVE_SIZE];
	/**
	 * offset 1812
	 */
	tle8888_mode_e tle8888mode;
	/**
	 * offset 1813
	 */
	uint8_t unusedSomethingWasHere[3];
	/**
	 * offset 1816
	 */
	tle8888filter_e tle8888VrFilter;
	/**
	 * offset 1820
	 */
	tle8888detection_voltage_e tle8888VrDetectionVoltage;
	/**
	 * offset 1824
	 */
	tle8888vrpeak_time_e tle8888VrPeakTime;
	/**
	 * offset 1828
	 */
	trigger_filter_e triggerFilter;
	/**
	 * offset 1832
	 */
	float syncRatioFrom;
	/**
	 * offset 1836
	 */
	float syncRatioTo;
	/**
	 * 0.1 is a good default value
	 * offset 1840
	 */
	float idle_antiwindupFreq;
	/**
	 * 0.1 is a good default value
	 * offset 1844
	 */
	float idle_derivativeFilterLoss;
	/**
	 * offset 1848
	 */
	float stoichRatioPrimary;
	/**
	 * offset 1852
	 */
	float knockBandCustom;
	/**
	 * offset 1856
	 */
	int mainUnusedEnd[436];
	/** total size 3600*/
};

typedef struct engine_configuration_s engine_configuration_s;

// start of persistent_config_s
struct persistent_config_s {
	/**
	 * offset 0
	 */
	engine_configuration_s engineConfiguration;
	/**
	 * offset 3600
	 */
	float cj125Pfactor;
	/**
	 * offset 3604
	 */
	float cj125Ifactor;
	/**
	 * offset 3608
	 */
	fsio_table_8x8_f32t vvtTable;
	/**
	 * offset 3864
	 */
	float vvtLoadBins[VVT_LOAD_COUNT];
	/**
	 * offset 3896
	 */
	float vvtRpmBins[VVT_RPM_COUNT];
	/**
	 * CLT-based idle position for coasting (used in Auto-PID Idle mode)
	 * offset 3928
	 */
	float iacCoastingBins[CLT_CURVE_SIZE];
	/**
	 *  CLT-based idle position for coasting (used in Auto-PID Idle mode)
	 * offset 3992
	 */
	float iacCoasting[CLT_CURVE_SIZE];
	/**
	 * offset 4056
	 */
	int unused125[24];
	/**
	 * offset 4152
	 */
	iac_pid_mult_t iacPidMultTable;
	/**
	 * offset 4216
	 */
	uint8_t iacPidMultLoadBins[IAC_PID_MULT_SIZE];
	/**
	 * offset 4224
	 */
	uint8_t iacPidMultRpmBins[IAC_PID_MULT_SIZE];
	/**
	 * Optional timing advance table for Cranking (see enableCrankingTimingTable)
	 * offset 4232
	 */
	float crankingAdvanceBins[CRANKING_ADVANCE_CURVE_SIZE];
	/**
	 * Optional timing advance table for Cranking (see enableCrankingTimingTable)
	 * offset 4248
	 */
	float crankingAdvance[CRANKING_ADVANCE_CURVE_SIZE];
	/**
	 * target TPS value, 0 to 100%
	 * TODO: use int8 data date once we template interpolation method
	 * offset 4264
	 */
	float etbBiasBins[ETB_BIAS_CURVE_LENGTH];
	/**
	 * PWM bias, 0 to 100%
	 * offset 4288
	 */
	float etbBiasValues[ETB_BIAS_CURVE_LENGTH];
	/**
	 * offset 4312
	 */
	float loadBasedAeDecayBins[MAP_ACCEL_TAPER];
	/**
	 * offset 4344
	 */
	float loadBasedAeDecayMult[MAP_ACCEL_TAPER];
	/**
	 * CLT-based target RPM for automatic idle controller
	 * offset 4376
	 */
	float cltIdleRpmBins[CLT_CURVE_SIZE];
	/**
	 * See idleRpmPid
	 * offset 4440
	 */
	float cltIdleRpm[CLT_CURVE_SIZE];
	/**
	 * Cranking fuel correction coefficient based on TPS
	 * offset 4504
	 */
	float crankingTpsCoef[CRANKING_CURVE_SIZE];
	/**
	 * offset 4536
	 */
	float crankingTpsBins[CRANKING_CURVE_SIZE];
	/**
	 * offset 4568
	 */
	int unused124[16];
	/**
	 * offset 4632
	 */
	float afterstartCoolantBins[AFTERSTART_HOLD_CURVE_SIZE];
	/**
	 * offset 4664
	 */
	float afterstartHoldTime[AFTERSTART_HOLD_CURVE_SIZE];
	/**
	 * offset 4696
	 */
	float afterstartEnrich[AFTERSTART_ENRICH_CURVE_SIZE];
	/**
	 * offset 4728
	 */
	float afterstartDecayTime[AFTERSTART_DECAY_CURVE_SIZE];
	/**
	 * offset 4760
	 */
	boost_table_t boostTableOpenLoop;
	/**
	 * offset 4824
	 */
	uint8_t boostRpmBins[BOOST_RPM_COUNT];
	/**
	 * offset 4832
	 */
	boost_table_t boostTableClosedLoop;
	/**
	 * offset 4896
	 */
	uint8_t boostTpsBins[BOOST_LOAD_COUNT];
	/**
	 * offset 4904
	 */
	pedal_to_tps_t pedalToTpsTable;
	/**
	 * offset 4968
	 */
	uint8_t pedalToTpsPedalBins[PEDAL_TO_TPS_SIZE];
	/**
	 * offset 4976
	 */
	uint8_t pedalToTpsRpmBins[PEDAL_TO_TPS_SIZE];
	/**
	 * CLT-based cranking position multiplier for simple manual idle controller
	 * offset 4984
	 */
	float cltCrankingCorrBins[CLT_CRANKING_CURVE_SIZE];
	/**
	 * CLT-based cranking position multiplier for simple manual idle controller
	 * offset 5016
	 */
	float cltCrankingCorr[CLT_CRANKING_CURVE_SIZE];
	/**
	 * Optional timing advance table for Idle (see useSeparateAdvanceForIdle)
	 * offset 5048
	 */
	float idleAdvanceBins[IDLE_ADVANCE_CURVE_SIZE];
	/**
	 * Optional timing advance table for Idle (see useSeparateAdvanceForIdle)
	 * offset 5080
	 */
	float idleAdvance[IDLE_ADVANCE_CURVE_SIZE];
	/**
	 * Optional VE table for Idle (see useSeparateVEForIdle)
	 * offset 5112
	 */
	float idleVeBins[IDLE_VE_CURVE_SIZE];
	/**
	 *  Optional VE table for Idle (see useSeparateVEForIdle)
	 * offset 5144
	 */
	float idleVe[IDLE_VE_CURVE_SIZE];
	/**
	 * offset 5176
	 */
	float cltFuelCorrBins[CLT_CURVE_SIZE];
	/**
	 * offset 5240
	 */
	float cltFuelCorr[CLT_CURVE_SIZE];
	/**
	 * offset 5304
	 */
	float iatFuelCorrBins[IAT_CURVE_SIZE];
	/**
	 * offset 5368
	 */
	float iatFuelCorr[IAT_CURVE_SIZE];
	/**
	 * offset 5432
	 */
	float crankingFuelCoef[CRANKING_CURVE_SIZE];
	/**
	 * offset 5464
	 */
	float crankingFuelBins[CRANKING_CURVE_SIZE];
	/**
	 * offset 5496
	 */
	float crankingCycleCoef[CRANKING_CURVE_SIZE];
	/**
	 * offset 5528
	 */
	float crankingCycleBins[CRANKING_CURVE_SIZE];
	/**
	 * CLT-based idle position multiplier for simple manual idle controller
	 * offset 5560
	 */
	float cltIdleCorrBins[CLT_CURVE_SIZE];
	/**
	 *  CLT-based idle position multiplier for simple manual idle controller
	 * offset 5624
	 */
	float cltIdleCorr[CLT_CURVE_SIZE];
	/**
	 * offset 5688
	 */
	angle_table_t ignitionIatCorrTable;
	/**
	 * offset 6712
	 */
	float ignitionIatCorrLoadBins[IGN_LOAD_COUNT];
	/**
	 * offset 6776
	 */
	float ignitionIatCorrRpmBins[IGN_RPM_COUNT];
	/**
	 * offset 6840
	 */
	angle_table_t injectionPhase;
	/**
	 * offset 7864
	 */
	float injPhaseLoadBins[FUEL_LOAD_COUNT];
	/**
	 * offset 7928
	 */
	float injPhaseRpmBins[FUEL_RPM_COUNT];
	/**
	 * offset 7992
	 */
	ignition_table_t advanceTable;
	/**
	 * offset 9016
	 */
	float smap_table[IGN_LOAD_COUNT];
	/**
	 * offset 9080
	 */
	float srpm_table[IGN_RPM_COUNT];
	/**
	 * offset 9144
	 */
	ve_table_t veTable;
	/**
	 * offset 10168
	 */
	float fmap_table[FUEL_LOAD_COUNT];
	/**
	 * offset 10232
	 */
	float frpm_table[FUEL_RPM_COUNT];
	/**
	 * offset 10296
	 */
	afr_table_t afrTable;
	/**
	 * offset 10552
	 */
	float afrLoadBins[FUEL_LOAD_COUNT];
	/**
	 * offset 10616
	 */
	float afrRpmBins[FUEL_RPM_COUNT];
	/**
	 * offset 10680
	 */
	tps_tps_table_t tpsTpsAccelTable;
	/**
	 * offset 10936
	 */
	float tpsTpsAccelFromRpmBins[TPS_TPS_ACCEL_TABLE];
	/**
	 * offset 10968
	 */
	float tpsTpsAccelToRpmBins[TPS_TPS_ACCEL_TABLE];
	/** total size 11000*/
};

typedef struct persistent_config_s persistent_config_s;

#endif
// end
// this section was generated automatically by rusEfi tool ConfigDefinition.jar based on gen_config.bat integration\rusefi_config.txt Sun Aug 30 19:19:41 CEST 2020
