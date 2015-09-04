#ifndef SIMPLE_SQUARE_H
#define SIMPLE_SQUARE_H

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
/*
 * The following are demo image generators. They all use the utility
 * class ThreadedCanvasManipulator to generate new frames.
 */

// Simple generator that pulses through RGB and White.

class SimpleSquare : public rgb_matrix::ThreadedCanvasManipulator
{
public:
	SimpleSquare(rgb_matrix::Canvas *m);
	void Run();
};

#endif //COLOR_PULSE_GENERATOR_H