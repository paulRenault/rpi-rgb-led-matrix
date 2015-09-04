#include "colorPulseGenerator.h"

#include <unistd.h>

ColorPulseGenerator::ColorPulseGenerator(rgb_matrix::RGBMatrix *m) : rgb_matrix::ThreadedCanvasManipulator(m), matrix_(m) {
    off_screen_canvas_ = m->CreateFrameCanvas();
}

void ColorPulseGenerator::Run() {
	uint32_t continuum = 0;
	while (running()) {
		usleep(5 * 1000);
		continuum += 1;
		continuum %= 3 * 255;
		int r = 0, g = 0, b = 0;
		if (continuum <= 255) {
			int c = continuum;
			b = 255 - c;
			r = c;
		} else if (continuum > 255 && continuum <= 511) {
			int c = continuum - 256;
			r = 255 - c;
			g = c;
		} else {
			int c = continuum - 512;
			g = 255 - c;
			b = c;
		}
		matrix_->transformer()->Transform(off_screen_canvas_)->Fill(r, g, b);
		off_screen_canvas_ = matrix_->SwapOnVSync(off_screen_canvas_);
	}
}