#ifndef BRIGTHNESS_PULSE_GENERATOR_H
#define BRIGTHNESS_PULSE_GENERATOR_H

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
/*
 * The following are demo image generators. They all use the utility
 * class ThreadedCanvasManipulator to generate new frames.
 */

// Simple generator that pulses through RGB and White.

class BrightnessPulseGenerator : public rgb_matrix::ThreadedCanvasManipulator
{
public:
	BrightnessPulseGenerator(rgb_matrix::RGBMatrix *m);
	 void Run(); 
private:
	rgb_matrix::RGBMatrix *const matrix_;
	rgb_matrix::FrameCanvas *off_screen_canvas_;
};

#endif //COLOR_PULSE_GENERATOR_H