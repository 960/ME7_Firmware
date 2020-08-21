/*
 * @file biquad.h
 *
 * @date Aug 10, 2020
 * @author Matthew Kennedy, (c) 2020
 */

#pragma once

class Biquad {
public:
	Biquad();

    float filter(float input);

	void configureBandpass(float samplingFrequency, float centerFrequency, float Q);

	// Default Q = 1/sqrt(2) = 0.707 gives a maximally flat passband without any overshoot near the cutoff (ie, Butterworth)
	void configureLowpass(float samplingFrequency, float cutoffFrequency, float Q = 0.707f);

private:
    float a0, a1, a2, b1, b2;
    float z1, z2;
};
