#include "brightnessPulseGenerator.h"

#include <unistd.h>

BrightnessPulseGenerator::BrightnessPulseGenerator(rgb_matrix::RGBMatrix *m) : rgb_matrix::ThreadedCanvasManipulator(m), matrix_(m) {}

void BrightnessPulseGenerator::Run() {
	const uint8_t max_brightness = matrix_->brightness();
    const uint8_t c = 255;
    uint8_t count = 0;

    while (running()) {
      if (matrix_->brightness() < 1) {
        matrix_->SetBrightness(max_brightness);
        count++;
      } else {
        matrix_->SetBrightness(matrix_->brightness() - 1);
      }

      switch (count % 4) {
        case 0: matrix_->Fill(c, 0, 0); break;
        case 1: matrix_->Fill(0, c, 0); break;
        case 2: matrix_->Fill(0, 0, c); break;
        case 3: matrix_->Fill(c, c, c); break;
      }

      usleep(20 * 1000);
    }
}