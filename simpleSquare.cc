#include "simpleSquare.h"
#include "graphics.h"

#include <unistd.h>


using namespace rgb_matrix;

SimpleSquare::SimpleSquare(Canvas *m) : ThreadedCanvasManipulator(m) {}

void SimpleSquare::Run() {
	const int width = canvas()->width() - 1;
    const int height = canvas()->height() - 1;
    // Borders
    DrawLine(canvas(), 0, 0,      width, 0,      Color(255, 0, 0));
    DrawLine(canvas(), 0, height, width, height, Color(255, 255, 0));
    DrawLine(canvas(), 0, 0,      0,     height, Color(0, 0, 255));
    DrawLine(canvas(), width, 0,  width, height, Color(0, 255, 0));

    // Diagonals.
    DrawLine(canvas(), 0, 0,        width, height, Color(255, 255, 255));
    DrawLine(canvas(), 0, height, width, 0,        Color(255,   0, 255));
}