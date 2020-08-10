/**
 * @file    interpolation.cpp
 * @brief	Linear interpolation algorithms
 *
 * See test_interpolation_3d.cpp
 *
 *
 * @date Oct 17, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 * @author Dmitry Sidin, (c) 2015
 */

#include "global.h"


#include "interpolation.h"


#define BINARY_PERF true

;

FastInterpolation::FastInterpolation() {
	init(0, 0, 1, 1);
}

FastInterpolation::FastInterpolation(float x1, float y1, float x2, float y2) {
	init(x1, y1, x2, y2);
}

void FastInterpolation::init(float x1, float y1, float x2, float y2) {
	if (x1 == x2) {
		warning(CUSTOM_ERR_INTERPOLATE, "init: Same x1 and x2 in interpolate: %.2f/%.2f", x1, x2);
		return;
	}
	a = INTERPOLATION_A(x1, y1, x2, y2);
	b = y1 - a * x1;
}

float FastInterpolation::getValue(float x) const {
	return a * x + b;
}

/** @brief	Linear interpolation by two points
 *
 * @param	x1 key of the first point
 * @param	y1 value of the first point
 * @param	x2 key of the second point
 * @param	y2 value of the second point
 * @param	X key to be interpolated
 *
 * @note	For example, "interpolateMsg("", engineConfiguration.tpsMin, 0, engineConfiguration.tpsMax, 100, adc);"
 */
float interpolateMsg(const char *msg, float x1, float y1, float x2, float y2, float x) {
	if (cisnan(x1) || cisnan(x2) || cisnan(y1) || cisnan(y2)) {
		warning(CUSTOM_INTEPOLATE_ERROR, "interpolate%s: why param", msg);
		return NAN;
	}
	if (cisnan(x)) {
		warning(CUSTOM_INTEPOLATE_ERROR, "interpolate%s: why X", msg);
		return NAN;
	}
	// todo: double comparison using EPS
	if (x1 == x2) {
		/**
		 * we could end up here for example while resetting bins while chan engine type
		 */
		warning(CUSTOM_INTEPOLATE_ERROR, "interpolate%s: Same x1 and x2 in interpolate: %.2f/%.2f", msg, x1, x2);
		return NAN;
	}

	// a*x1 + b = y1
	// a*x2 + b = y2
//	efiAssertVoid(CUSTOM_ERR_ASSERT_VOID, x1 != x2, "no way we can interpolate");
	float a = INTERPOLATION_A(x1, y1, x2, y2);
	if (cisnan(a)) {
		warning(CUSTOM_INTEPOLATE_ERROR, "interpolate%s: why a", msg);
		return NAN;
	}
	float b = y1 - a * x1;
	float result = a * x + b;

	return result;
}

float interpolateClamped(float x1, float y1, float x2, float y2, float x) {
	if (x <= x1)
		return y1;
	if (x >= x2)
		return y2;

	float a = INTERPOLATION_A(x1, y1, x2, y2);
	float b = y1 - a * x1;
	return a * x + b;
}

/**
 * Another implementation, which one is faster?
 */
int findIndex2(const float array[], unsigned size, float value) {
	efiAssert(CUSTOM_ERR_ASSERT, !cisnan(value), "NaN in findIndex2", 0);
	efiAssert(CUSTOM_ERR_ASSERT, size > 1, "size in findIndex", 0);
//	if (size <= 1)
//		return size && *array <= value ? 0 : -1;

	signed i = 0;
	//unsigned b = 1 << int(log(float(size) - 1) / 0.69314718055994530942);
	unsigned b = size >> 1; // in our case size is always a power of 2
	efiAssert(CUSTOM_ERR_ASSERT, b + b == size, "Size not power of 2", -1);
	for (; b; b >>= 1) {
		unsigned j = i | b;
		/**
		 * it should be
		 * "if (j < size && array[j] <= value)"
		 * but in our case size is always power of 2 thus size is always more then j
		 */
		// efiAssert(CUSTOM_ERR_ASSERT, j < size, "size", 0);
		if (array[j] <= value)
			i = j;
	}
	return i || *array <= value ? i : -1;
}

/**
 * in order to use binary search we need to know that axis elements are sorted
 */
void ensureArrayIsAscending(const char *msg, const float array[], int size) {
	for (int i = 0; i < size - 1; i ++) {
		if (array[i] >= array[i+ 1]) {
			// todo: this should become a warning under https://github.com/rusefi/rusefi/issues/440
			warning(CUSTOM_ERR_AXIS_ORDER, "invalid axis %s at %.2f", msg, array[i]);
		}
	}
}

int findIndex(const float array[], int size, float value) {
	return findIndexMsg("", array, size, value);
}

namespace priv
{
/**
 * @brief	One-dimensional table lookup with linear interpolation
 *
 * @see setLinearCurve()
 */
float interpolate2d(const char *msg, float value, const float bin[], const float values[], int size) {
	if (isnan(value)) {
		warning(CUSTOM_INTERPOLATE_NAN, "NaN in interpolate2d %s", msg);
		return NAN;
	}
	int index = findIndexMsg(msg, bin, size, value);

	if (index == -1)
		return values[0];
	if (index == size - 1)
		return values[size - 1];

	return interpolateMsg(msg, bin[index], values[index], bin[index + 1], values[index + 1], value);
}
}

/**
 * Sets specified value for specified key in a correction curve
 * see also setLinearCurve()
 */
void setCurveValue(float bins[], float values[], int size, float key, float value) {
	int index = findIndexMsg("tbVl", bins, size, key);
	if (index == -1)
		index = 0;
	values[index] = value;
}

void initInterpolation() {
	

}
