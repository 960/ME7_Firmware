/*
 * @file trigger_vw.cpp
 *
 * @date Aug 25, 2018
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "trigger_vw.h"
#include "trigger_universal.h"

void setVwConfiguration(TriggerWaveform *s) {
	efiAssertVoid(CUSTOM_ERR_6660, s != NULL, "TriggerWaveform is NULL");

	s->initialize(FOUR_STROKE_CRANK_SENSOR);

	s->isSynchronizationNeeded = true;


	int totalTeethCount = 60;
	int skippedCount = 2;

	float engineCycle = FOUR_STROKE_ENGINE_CYCLE;
	float toothWidth = 0.5;

	addSkippedToothTriggerEvents(T_PRIMARY, s, 60, 2, toothWidth, 0, engineCycle,
			NO_LEFT_FILTER, 690);

	float angleDown = engineCycle / totalTeethCount * (totalTeethCount - skippedCount - 1 + (1 - toothWidth) );
	s->addEventClamped(0 + angleDown + 12, T_PRIMARY, TV_RISE, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	s->addEventClamped(0 + engineCycle, T_PRIMARY, TV_FALL, NO_LEFT_FILTER, NO_RIGHT_FILTER);

	s->setTriggerSynchronizationGap2(1.6, 4);
}
void setVwVvtConfiguration(TriggerWaveform *s) {
	/*
	s->initialize(FOUR_STROKE_CAM_SENSOR);

	float crankDelta = 720 / 60 / 2; // 6


	float crankToothAngle = 6;
	float crankAngle = 2 * crankToothAngle;

	float engineCycle = FOUR_STROKE_ENGINE_CYCLE;
	float toothWidth = 0.5;



		int offset = 2 * 20;

		s->setTriggerSynchronizationGap3(0, 2, 3);
		     //	180			  360		    540		       720       0
     // ---------|___|---------|___|---------|_________|----|________|
          //....18...30............60.............90............120.......0
		//		  40 	140	 Gap2 40   140        140      40    140  gap1
		// 6 KW...............

// falling cam edges
// 18....47....78....108 teeth
// 107...                deg

		//s->addEvent720(0, T_PRIMARY, TV_RISE);  // Begin

		for (int i = 0; i < 15; i++) {
		s->addEvent720(crankAngle, T_SECONDARY, TV_FALL); // 12 deg
		s->addEvent720(crankAngle + crankToothAngle, T_SECONDARY, TV_RISE); // + 6 deg
		crankAngle += 180;
		}
		s->addEvent720(180, T_PRIMARY, TV_FALL);  // 180 deg

		for (int i = 0; i < 3; i++) {
		s->addEvent720(crankAngle, T_SECONDARY, TV_FALL); // 6 deg
		s->addEvent720(crankAngle + crankToothAngle, T_SECONDARY, TV_RISE); //  + 6 deg
		crankAngle += 220;    //180 + 40
		}

		s->addEvent720(220, T_PRIMARY, TV_RISE);  // 220
		// 220 to 360
		for (int i = 0; i < 11; i++) {
		s->addEvent720(crankAngle, T_SECONDARY, TV_FALL); // 6 deg
		s->addEvent720(crankToothAngle + 3, T_SECONDARY, TV_RISE); // 6 deg + 3 deg
		crankAngle += 360;
		}
		s->addEvent720(360, T_PRIMARY, TV_FALL);           // 360

		// Tooth 30 to 58
		for (int i = 0; i < 2; i++) {
		s->addEvent720(crankAngle, T_SECONDARY, TV_FALL); // 6 deg
		s->addEvent720(crankToothAngle + 3, T_SECONDARY, TV_RISE); // 6 deg + 3 deg
		}

		// missing teeth
		s->addEvent720(crankAngle + 6, T_SECONDARY, TV_FALL); // 18 deg
		s->addEvent720(crankAngle + crankToothAngle + 6, T_SECONDARY, TV_RISE); // 12 deg + 3 deg
		crankAngle += 360;
		s->addEvent720(360, T_PRIMARY, TV_FALL);


// Second Revolution--------------------------------------------

		// Tooth 60 to 90
		for (int i = 0; i < 29; i++) {
		s->addEvent720(crankAngle, T_SECONDARY, TV_FALL);   // 6 deg
		s->addEvent720(crankAngle + crankToothAngle, T_SECONDARY, TV_RISE); // 6 deg + 3 deg
		crankAngle += 540;
		}
		s->addEvent720(540, T_PRIMARY, TV_FALL);				//540

		// Tooth 90 to 118
		for (int i = 0; i < 27; i++) {
		s->addEvent720(crankAngle, T_SECONDARY, TV_FALL);
		s->addEvent720(crankAngle + crankToothAngle, T_SECONDARY, TV_RISE);
		}

		// missing teeth
		s->addEvent720(crankAngle + 6, T_SECONDARY, TV_FALL); // 18 deg
		s->addEvent720(crankAngle + crankToothAngle + 6, T_SECONDARY, TV_RISE); // 18 deg + 6 deg
		crankAngle += 720;


		s->addEvent720(650, T_PRIMARY, TV_RISE);


*/
}
