#ifndef GRAY_SCALE_BLOCK_H
#define GRAY_SCALE_BLOCK_H

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
/*
 * The following are demo image generators. They all use the utility
 * class ThreadedCanvasManipulator to generate new frames.
 */

// Simple generator that pulses through RGB and White.

class GrayScaleBlock : public rgb_matrix::ThreadedCanvasManipulator
{
public:
	GrayScaleBlock(rgb_matrix::Canvas *m);
	void Run();
};

#endif //COLOR_PULSE_GENERATOR_H