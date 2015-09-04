#ifndef IMAGE_SCROLLER_H
#define IMAGE_SCROLLER_H

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
enum direction {horizontal,vertical};
#include <stdio.h>
/*
 * The following are demo image generators. They all use the utility
 * class ThreadedCanvasManipulator to generate new frames.
 */

// Simple generator that pulses through RGB and White.
namespace rgb_matrix {
class ImageScroller : public ThreadedCanvasManipulator
{
public:
	ImageScroller(RGBMatrix *m, int scroll_jumps, int scroll_ms = 30, direction direction = horizontal, bool repeat=false);
	~ImageScroller();
	void Run();
	bool LoadPPM(const char *filename);


private:
	// Read line, skip comments.
  char *ReadLine(FILE *f, char *buffer, size_t len);
	void scroll();
  struct Pixel {
    Pixel() : red(0), green(0), blue(0){}
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  };

  struct Image {
    Image() : width(-1), height(-1), image(NULL) {}
    ~Image() { Delete(); }
    void Delete() { delete [] image; Reset(); }
    void Reset() { image = NULL; width = -1; height = -1; }
    inline bool IsValid() { return image && height > 0 && width > 0; }
    const Pixel &getPixel(int x, int y) {
      static Pixel black;
      if (x < 0 || x >= width || y < 0 || y >= height) return black;
      return image[x + width * y];
    }

    int width;
    int height;
    Pixel *image;
  };

  const int scroll_jumps_;
  const int scroll_ms_;

  // Current image is only manipulated in our thread.
  Image current_image_;

  // New image can be loaded from another thread, then taken over in main thread.
  Mutex mutex_new_image_;
  Image new_image_;

  int32_t horizontal_position_;
	int32_t vertical_position_;
	direction direction_;
	bool repeat_;

  RGBMatrix* matrix_;
  FrameCanvas* offscreen_;
};
}
#endif //IMAGE_SCROLLER_H
