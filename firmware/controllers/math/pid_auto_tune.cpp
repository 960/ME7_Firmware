/*
 * pid_auto_tune.cpp
 *
 * See https://github.com/br3ttb/Arduino-PID-AutoTune-Library/blob/master/PID_AutoTune_v0/PID_AutoTune_v0.cpp
 * See https://github.com/t0mpr1c3/Arduino-PID-AutoTune-Library/blob/master/PID_AutoTune_v0/PID_AutoTune_v0.cpp
 *
 *
 *  Created on: Sep 13, 2017
 */

// source of Tyreus-Luyben and Ciancone-Marlin rules:
// "Autotuning of PID Controllers: A Relay Feedback Approach",
//  by Cheng-Ching Yu, 2nd Edition, p.18
// Tyreus-Luyben is more conservative than Ziegler-Nichols
// and is preferred for lag dominated processes
// Ciancone-Marlin is preferred for delay dominated processes
// Ziegler-Nichols is intended for best disturbance rejection
// can lack robustness especially for lag dominated processes

// source for Pessen Integral, Some Overshoot, and No Overshoot rules:
// "Rule-Based Autotuning Based on Frequency Domain Identification"
// by Anthony S. McCormack and Keith R. Godfrey
// IEEE Transactions on Control Systems Technology, vol 6 no 1, January 1998.
// as reported on http://www.mstarlabs.com/control/znrule.html

#include "pid_auto_tune.h"
#include "efilib.h"
#include "efitime.h"

// see https://en.wikipedia.org/wiki/Ziegler%E2%80%93Nichols_method
// order must be match enumerated type for auto tune methods
Tuning tuningRule[PID_AutoTune::NO_OVERSHOOT_PID + 1] =
{
  { {  44, 24,   0 } },  // ZIEGLER_NICHOLS_PI
  { {  34, 40, 160 } },  // ZIEGLER_NICHOLS_PID
  { {  64,  9,   0 } },  // TYREUS_LUYBEN_PI
  { {  44,  9, 126 } },  // TYREUS_LUYBEN_PID
  { {  66, 80,   0 } },  // CIANCONE_MARLIN_PI
  { {  66, 88, 162 } },  // CIANCONE_MARLIN_PID
  { {  28, 50, 133 } },  // PESSEN_INTEGRAL_PID
  { {  60, 40,  60 } },  // SOME_OVERSHOOT_PID
  { { 100, 40,  60 } }   // NO_OVERSHOOT_PID
};

PID_AutoTune::PID_AutoTune() {
	reset();
}

void PID_AutoTune::Cancel()
{
  state = AUTOTUNER_OFF;
}

void PID_AutoTune::reset() {

	controlType = ZIEGLER_NICHOLS_PID;
	noiseBand = 0.5;
	state = AUTOTUNER_OFF; // cannot invoke setter here since is not initialized yet
	oStep = 10.0;
	memset(lastPeaks, 0, sizeof(lastPeaks));
	memset(lastInputs, 0, sizeof(lastInputs));


	input = output = 0;
	SetLookbackSec(10);

}

void PID_AutoTune::SetLookbackSec(int value)
{
  if (value < 1)
  {
    value = 1;
  }
  if (value < 25)
  {
    nLookBack = value * 4;
    sampleTime = 250;
  }
  else
  {
    nLookBack = 100;
    sampleTime = value * 10;
  }
}

double inline PID_AutoTune::fastArcTan(double x)
{
  // source: �Efficient approximations for the arctangent function�, Rajan, S. Sichun Wang Inkol, R. Joyal, A., May 2006
  //return CONST_PI / 4.0 * x - x * (abs(x) - 1.0) * (0.2447 + 0.0663 * abs(x));

  // source: "Understanding Digital Signal Processing", 2nd Ed, Richard G. Lyons, eq. 13-107
  return x / (1.0 + 0.28125 * pow(x, 2));
}

double PID_AutoTune::calculatePhaseLag(double inducedAmplitude)
{
  // calculate phase lag
  // NB hysteresis = 2 * noiseBand;
  double ratio = 2.0 * workingNoiseBand / inducedAmplitude;
  if (ratio > 1.0)
  {
    return CONST_PI / 2.0;
  }
  else
  {
    //return CONST_PI - asin(ratio);
    return CONST_PI - fastArcTan(ratio / sqrt( 1.0 - pow(ratio, 2)));
  }
}

void PID_AutoTune::setState(PidAutoTune_AutoTunerState state) {
	this->state = state;

}

void PID_AutoTune::setPeakType(PidAutoTune_Peak peakType) {
	this->peakType = peakType;
}

/**
 * returns true when done, otherwise returns false
 */
bool PID_AutoTune::Runtime()
{


  // check ready for new input
  unsigned long now = currentTimeMillis();

  if (state == AUTOTUNER_OFF)
  {
    // initialize working variables the first time around
	  setPeakType(NOT_A_PEAK);
    inputCount = 0;
    peakCount = 0;
    setpoint = input;
    outputStart = output;
    lastPeakTime[0] = now;
    workingNoiseBand = noiseBand;
    newWorkingNoiseBand = noiseBand;
    workingOutputstep = oStep;

#if defined (AUTOTUNE_RELAY_BIAS)
    relayBias = 0.0;
    stepCount = 0;
    lastStepTime[0] = now;
    sumInputSinceLastStep[0] = 0.0;
#endif

    // move to new state
    if (controlType == AMIGOF_PI)
    {
    	setState(STEADY_STATE_AT_BASELINE);
    }
    else
    {
    	    	setState(RELAY_STEP_UP);
    }
  }

  // otherwise check ready for new input
  else if ((now - lastTime) < sampleTime)
  {


    return false;
  }

  // get new input
  lastTime = now;
  double refVal = input;

#if defined (AUTOTUNE_RELAY_BIAS)
  // used to calculate relay bias
  sumInputSinceLastStep[0] += refVal;
#endif

  // local flag variable
  bool justChanged = false;

  // check input and change relay state if necessary
  if ((state == RELAY_STEP_UP) && (refVal > setpoint + workingNoiseBand))
  {

	  setState(RELAY_STEP_DOWN);
    justChanged = true;
  }
  else if ((state == RELAY_STEP_DOWN) && (refVal < setpoint - workingNoiseBand))
  {

	  setState(RELAY_STEP_UP);
    justChanged = true;
  }
  if (justChanged)
  {
    workingNoiseBand = newWorkingNoiseBand;

#if defined (AUTOTUNE_RELAY_BIAS)
    // check symmetry of oscillation
    // and introduce relay bias if necessary
    if (stepCount > 4)
    {
      double avgStep1 = 0.5 * (double) ((lastStepTime[0] - lastStepTime[1]) + (lastStepTime[2] - lastStepTime[3]));
      double avgStep2 = 0.5 * (double) ((lastStepTime[1] - lastStepTime[2]) + (lastStepTime[3] - lastStepTime[4]));
      if ((avgStep1 > 1e-10) && (avgStep2 > 1e-10))
      {
        double asymmetry = (avgStep1 > avgStep2) ?
                           (avgStep1 - avgStep2) / avgStep1 : (avgStep2 - avgStep1) / avgStep2;

#if defined (AUTOTUNE_DEBUG)
        Serial.print(F("asymmetry "));
        Serial.println(asymmetry);
#endif /* AUTOTUNE_DEBUG */


        if (asymmetry > AUTOTUNE_STEP_ASYMMETRY_TOLERANCE)
        {
          // relay steps are asymmetric
          // calculate relay bias using
          // "Autotuning of PID Controllers: A Relay Feedback Approach",
          //  by Cheng-Ching Yu, 2nd Edition, equation 7.39, p. 148

          // calculate change in relay bias
          double deltaRelayBias = - processValueOffset(avgStep1, avgStep2) * workingOstep;
          if (state == RELAY_STEP_DOWN)
          {
            deltaRelayBias = -deltaRelayBias;
          }

          if (abs(deltaRelayBias) > workingOstep * AUTOTUNE_STEP_ASYMMETRY_TOLERANCE)
          {
            // change is large enough to bother with
            relayBias += deltaRelayBias;

            /*
            // adjust step height with respect to output limits
            // commented out because the auto tuner does not
            // necessarily know what the output limits are
            double relayHigh = outputStart + workingOstep + relayBias;
            double relayLow  = outputStart - workingOstep + relayBias;
            if (relayHigh > outMax)
            {
              relayHigh = outMax;
            }
            if (relayLow  < outMin)
            {
              relayHigh = outMin;
            }
            workingOstep = 0.5 * (relayHigh - relayLow);
            relayBias = relayHigh - outputStart - workingOstep;
            */

#if defined (AUTOTUNE_DEBUG)
            Serial.print(F("deltaRelayBias "));
            Serial.println(deltaRelayBias);
            Serial.print(F("relayBias "));
            Serial.println(relayBias);
#endif /* AUTOTUNE_DEBUG */


            // reset relay step counter
            // to give the process value oscillation
            // time to settle with the new relay bias value
            stepCount = 0;
          }
        }
      }
    }

    // shift step time and integrated process value arrays
    for (byte i = (stepCount > 4 ? 4 : stepCount); i > 0; i--)
    {
      lastStepTime[i] = lastStepTime[i - 1];
      sumInputSinceLastStep[i] = sumInputSinceLastStep[i - 1];
    }
    stepCount++;
    lastStepTime[0] = now;
    sumInputSinceLastStep[0] = 0.0;

#if defined (AUTOTUNE_DEBUG)
    for (byte i = 1; i < (stepCount > 4 ? 5 : stepCount); i++)
    {
      Serial.print(F("step time "));
      Serial.println(lastStepTime[i]);
      Serial.print(F("step sum "));
      Serial.println(sumInputSinceLastStep[i]);
    }
#endif /* AUTOTUNE_DEBUG */

#endif // if defined AUTOTUNE_RELAY_BIAS

  } // if justChanged

  // set output
  // FIXME need to respect output limits
  // not knowing output limits is one reason
  // to pass entire PID object to autotune method(s)
  if (((byte) state & (STEADY_STATE_AFTER_STEP_UP | RELAY_STEP_UP)) > 0)
  {

#if defined (AUTOTUNE_RELAY_BIAS)
    setOutput(outputStart + workingOstep + relayBias);
#else

    setOutput(outputStart + workingOutputstep);
#endif

  }
  else if (state == RELAY_STEP_DOWN)
  {

#if defined (AUTOTUNE_RELAY_BIAS)
	  setOutput(outputStart - workingOstep + relayBias);
#else
	  	  setOutput(outputStart - workingOutputstep);
#endif

  }


  // store initial inputs
  // we don't want to trust the maxes or mins
  // until the input array is full
  inputCount++;
  if (inputCount <= nLookBack)
  {
    lastInputs[nLookBack - inputCount] = refVal;

	return false;
  }

  // shift array of process values and identify peaks
  inputCount = nLookBack;
  bool isMax = true;
  bool isMin = true;
  for (int i = inputCount - 1; i >= 0; i--)
  {
    double val = lastInputs[i];
    if (isMax)
    {
      isMax = (refVal >= val);
    }
    if (isMin)
    {
      isMin = (refVal <= val);
    }
    lastInputs[i + 1] = val;
  }
  lastInputs[0] = refVal;


  // for AMIGOf tuning rule, perform an initial
  // step change to calculate process gain K_process
  // this may be very slow for lag-dominated processes
  // and may never terminate for integrating processes
  if (((byte) state & (STEADY_STATE_AT_BASELINE | STEADY_STATE_AFTER_STEP_UP)) > 0)
  {
    // check that all the recent inputs are
    // equal give or take expected noise
    double iMax = lastInputs[0];
    double iMin = lastInputs[0];
    double avgInput = 0.0;
    for (byte i = 0; i <= inputCount; i++)
    {
      double val = lastInputs[i];
      if (iMax < val)
      {
        iMax = val;
      }
        if (iMin > val)
      {
        iMin = val;
      }
      avgInput += val;
    }
    avgInput /= (double)(inputCount + 1);




    // if recent inputs are stable
    if ((iMax - iMin) <= 2.0 * workingNoiseBand)
    {

#if defined (AUTOTUNE_RELAY_BIAS)
      lastStepTime[0] = now;
#endif

      if (state == STEADY_STATE_AT_BASELINE)
      {
    	  setState(STEADY_STATE_AFTER_STEP_UP);
        lastPeaks[0] = avgInput;
        inputCount = 0;

		return false;
      }
      // else state == STEADY_STATE_AFTER_STEP_UP
      // calculate process gain
      K_process = (avgInput - lastPeaks[0]) / workingOutputstep;

#if defined (AUTOTUNE_DEBUG)
      Serial.print(F("Process gain "));
      Serial.println(K_process);
#endif


      // bad estimate of process gain
      if (K_process < 1e-10) // zero
      {
    	  setState(AUTOTUNER_OFF);

		return false;
      }
      setState(RELAY_STEP_DOWN);

#if defined (AUTOTUNE_RELAY_BIAS)
      sumInputSinceLastStep[0] = 0.0;
#endif

#if EFI_UNIT_TEST
		printf(":( 5\r\n");
#endif /* EFI_UNIT_TEST */
		return false;
    }
    else
    {

		return false;
    }
  }

  // increment peak count
  // and record peak time
  // for both maxima and minima
  justChanged = false;
  if (isMax)
  {
    if (peakType == MINIMUM)
    {
      justChanged = true;
    }
    setPeakType(MAXIMUM);
  }
  else if (isMin)
  {
    if (peakType == MAXIMUM)
    {
      justChanged = true;
    }
    setPeakType(MINIMUM);
  }

  // update peak times and values
  if (justChanged)
  {
    peakCount++;




    // shift peak time and peak value arrays
    for (byte i = (peakCount > 4 ? 4 : peakCount); i > 0; i--)
    {
      lastPeakTime[i] = lastPeakTime[i - 1];
      lastPeaks[i] = lastPeaks[i - 1];
    }
  }
  if (isMax || isMin)
  {
    lastPeakTime[0] = now;
    lastPeaks[0] = refVal;


  }

  // check for convergence of induced oscillation
  // convergence of amplitude assessed on last 4 peaks (1.5 cycles)
  double inducedAmplitude = 0.0;
  double phaseLag;
  if (

#if defined (AUTOTUNE_RELAY_BIAS)
    (stepCount > STEPCOUNT - 1) &&
#endif

    justChanged &&
    (peakCount > STEPCOUNT - 1)
  )
  {
    double absMax = lastPeaks[1];
    double absMin = lastPeaks[1];
    for (byte i = 2; i < STEPCOUNT; i++)
    {
      double val = lastPeaks[i];
      inducedAmplitude += abs( val - lastPeaks[i - 1]);
      if (absMax < val)
      {
         absMax = val;
      }
      if (absMin > val)
      {
         absMin = val;
      }
    }


    inducedAmplitude /= 6.0;


    // source for AMIGOf PI auto tuning method:
    // "Revisiting the Ziegler-Nichols tuning rules for PI control �
    //  Part II. The frequency response method."
    // T. Hagglund and K. J. Astrom
    // Asian Journal of Control, Vol. 6, No. 4, pp. 469-482, December 2004
    // http://www.ajc.org.tw/pages/paper/6.4PD/AC0604-P469-FR0371.pdf
    if (controlType == AMIGOF_PI)
    {
      phaseLag = calculatePhaseLag(inducedAmplitude);

      // check that phase lag is within acceptable bounds, ideally between 120� and 140�
      // but 115� to 145� will just about do, and might converge quicker
      if (abs(phaseLag - CONST_PI * 130.0 / 180.0) > (CONST_PI * 15.0 / 180.0))
      {
        // phase lag outside the desired range
        // set noiseBand to new estimate
        // aiming for 135� = 0.75 * pi (radians)
        // sin(135�) = sqrt(2)/2
        // NB noiseBand = 0.5 * hysteresis
        newWorkingNoiseBand = 0.5 * inducedAmplitude * CONST_SQRT2_DIV_2;

#if defined (AUTOTUNE_RELAY_BIAS)
        // we could reset relay step counter because we can't rely
        // on constant phase lag for calculating
        // relay bias having changed noiseBand
        // but this would essentially preclude using relay bias
        // with AMIGOf tuning, which is already a compile option
        /*
        stepCount = 0;
        */
#endif


		return false;
      }
    }

    // check convergence criterion for amplitude of induced oscillation
    if (((0.5 * (absMax - absMin) - inducedAmplitude) / inducedAmplitude) < AUTOTUNE_PEAK_AMPLITUDE_TOLERANCE)
    {
    	setState(CONVERGED);
    }
  }

  bool tooManyCycles = peakCount >= 20;
  bool tooLongBetween =  ((now - lastPeakTime[0]) > (unsigned long) (AUTOTUNE_MAX_WAIT_MINUTES * 60000));
  // if the autotune has not already converged
  // terminate after 10 cycles
  // or if too long between peaks
  // or if too long between relay steps
  if (

#if defined (AUTOTUNE_RELAY_BIAS)
    ((now - lastStepTime[0]) > (unsigned long) (AUTOTUNE_MAX_WAIT_MINUTES * 60000)) ||
#endif

	tooLongBetween ||
	tooManyCycles
  )
  {

	  setState(FAILED);
  }

  if (((byte) state & (CONVERGED | FAILED)) == 0)
  {

	return false;
  }

  // autotune algorithm has terminated
  // reset autotuner variables

  setOutput( outputStart);

  if (state == FAILED)
  {
    // do not calculate gain parameters

    return true;
  }

  // finish up by calculating tuning parameters

  // calculate ultimate gain
  double Ku = 4.0 * workingOutputstep / (inducedAmplitude * CONST_PI);


  // calculate ultimate period in seconds
  double Pu = (double) 0.5 * ((lastPeakTime[1] - lastPeakTime[3]) + (lastPeakTime[2] - lastPeakTime[4])) / 1000.0;


  // calculate gain parameters using tuning rules
  // NB PID generally outperforms PI for lag-dominated processes

  // AMIGOf is slow to tune, especially for lag-dominated processes, because it
  // requires an estimate of the process gain which is implemented in this
  // routine by steady state change in process variable after step change in set point
  // It is intended to give robust tunings for both lag- and delay- dominated processes
  if (controlType == AMIGOF_PI)
  {
    // calculate gain ratio
    double kappa_phi = (1.0 / Ku) / K_process;


    // calculate phase lag
    phaseLag = calculatePhaseLag(inducedAmplitude);


    // calculate tunings
    Kp = (( 2.50 - 0.92 * phaseLag) / (1.0 + (10.75 - 4.01 * phaseLag) * kappa_phi)) * Ku;
    Ti = ((-3.05 + 1.72 * phaseLag) / pow(1.0 + (-6.10 + 3.44 * phaseLag) * kappa_phi, 2)) * Pu;
    Td = 0.0;

    // converged
    return true;
  }

  Kp = Ku / (double) tuningRule[controlType].divisor(KP_DIVISOR);
  Ti = Pu / (double) tuningRule[controlType].divisor(TI_DIVISOR);
  Td = tuningRule[controlType].PI_controller() ?
       0.0 : Pu / (double) tuningRule[controlType].divisor(TD_DIVISOR);

  // converged
  return true;
}

float PID_AutoTune::GetKp() const
{
  return Kp;
}

float PID_AutoTune::GetKi() const
{
  return Kp / Ti;
}

float PID_AutoTune::GetKd() const
{
  return Kp * Td;
}

void PID_AutoTune::setOutput(float output) {
	this->output = output;

}

void PID_AutoTune::SetOutputStep(double Step)
{
  oStep = Step;
}

double PID_AutoTune::GetOutputStep() const
{
  return oStep;
}

void PID_AutoTune::SetControlType(byte type)
{
  controlType = type;
}

byte PID_AutoTune::GetControlType() const
{
  return controlType;
}

