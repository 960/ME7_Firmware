#pragma once

template <int TrackingLength>
class GapTracker {
public:
	static constexpr int _len = TrackingLength;
	GapTracker() {
		reset();
	}

	void trackTooth(efitick_t nowNt) {
		// Indicates that we haven't seen a tooth yet
		if (m_lastToothTime == -1) {
			m_lastToothTime = nowNt;
			return;
		}

		auto gap = nowNt - m_lastToothTime;
		m_lastToothTime = nowNt;

		if (m_lastGap == 0) {
			m_lastGap = gap;
			return;
		}

		auto gapRatio = (float)gap / m_lastGap;
		m_lastGap = gap;

		// scoot the array over 1 - newest ratio at the head
		for (size_t i = _len - 1; i > 0; i--) {
			m_gapRatios[i] = m_gapRatios[i - 1];
		}

		m_gapRatios[0] = gapRatio;
	}

	bool isMatch(const float (&testGaps)[_len], float ratioMin, float ratioMax) const {
		for (size_t i = 0; i < _len; i++) {
			auto target = testGaps[i];

			// Target of 0 indicates that we're done - all previous checks have passed
			if (target == 0) {
				return true;
			}

			auto test = m_gapRatios[i];

			// A zero indicates that we don't have enough teeth tracked yet
			if (test == 0) {
				return false;
			}

			auto minAllowed = target * ratioMin;
			auto maxAllowed = target * ratioMax;


			// if the tested gap is outside the allowed range, it's not a match.
			if (test < minAllowed || test > maxAllowed) {
				return false;
			}
		}

		// No gap was out of spec - it's a match!
		return true;
	}

	bool isMatch(const float (&testGapsMin)[_len], const float (&testGapsMax)[_len]) {
		for (size_t i = 0; i < _len; i++) {
			auto test = m_gapRatios[i];

			// A zero indicates that we don't have enough teeth tracked yet
			if (test == 0) {
				return false;
			}

			auto minAllowed = testGapsMin[i];
			auto maxAllowed = testGapsMax[i];

			if (cisnan(minAllowed) || cisnan(maxAllowed)) {
				return true;
			}

			// if the tested gap is outside the allowed range, it's not a match.
			if (test < minAllowed || test > maxAllowed) {
				return false;
			}
		}

		// No gap was out of spec - it's a match!
		return true;
	}

	float getLastGap() const {
		return m_gapRatios[0];
	}

	void reset() {
		for (size_t i = 0; i < _len; i++) {
			m_gapRatios[i] = 0;
		}

		m_lastToothTime = -1;
		m_lastGap = 0;
	}

private:
	efitick_t m_lastToothTime = -1;
	efitick_t m_lastGap = 0;
	float m_gapRatios[_len];
};
