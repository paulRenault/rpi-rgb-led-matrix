#ifndef ROTATING_BLOCK_GENERATOR_H
#define ROTATING_BLOCK_GENERATOR_H

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
/*
 * The following are demo image generators. They all use the utility
 * class ThreadedCanvasManipulator to generate new frames.
 */

// Simple generator that pulses through RGB and White.

class RotatingBlockGenerator : public rgb_matrix::ThreadedCanvasManipulator
{
public:
	RotatingBlockGenerator(rgb_matrix::Canvas *m);
	void Run();
private:
  void Rotate(int x, int y, float angle,float *new_x, float *new_y);
  uint8_t scale_col(int val, int lo, int hi);
};

#endif //ROTATING_BLOCK_GENERATOR_H