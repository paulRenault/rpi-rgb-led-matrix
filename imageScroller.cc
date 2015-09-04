#include "imageScroller.h"

#include <unistd.h>
#include <assert.h>
using namespace rgb_matrix;

ImageScroller::ImageScroller(RGBMatrix *m, int scroll_jumps, int scroll_ms, direction direction, bool repeat)
: ThreadedCanvasManipulator(m), scroll_jumps_(scroll_jumps),
scroll_ms_(scroll_ms),
horizontal_position_(0),
vertical_position_(0),
repeat_(repeat),
direction_(direction),
matrix_(m) {
  offscreen_ = matrix_->CreateFrameCanvas();
}

ImageScroller::~ImageScroller() {
  Stop();
  WaitStopped();   // only now it is safe to delete our instance variables.
}

void ImageScroller::scroll(){
  //Scrolling direction
  if(direction_ == horizontal){
    horizontal_position_ += scroll_jumps_;
  }else{
    vertical_position_ += scroll_jumps_;
  }

  //Limits
  horizontal_position_%=matrix_->transformer()->Transform(offscreen_)->width(); //current_image_.width;//
  vertical_position_%=matrix_->transformer()->Transform(offscreen_)->height(); //current_image_.height;//

  //Limits
  if (horizontal_position_ < 0) horizontal_position_ = current_image_.width;
  if (vertical_position_ < 0) vertical_position_ = current_image_.height;
}

void ImageScroller::Run() {
  const int screen_height = matrix_->transformer()->Transform(offscreen_)->height();
  const int screen_width = matrix_->transformer()->Transform(offscreen_)->width();
  while (running()) {
    {
      MutexLock l(&mutex_new_image_);
      if (new_image_.IsValid()) {
        current_image_.Delete();
        current_image_ = new_image_;
        new_image_.Reset();
      }
    }
    if (!current_image_.IsValid()) {
      usleep(100 * 1000);
      continue;
    }

    //Set each pixel
    for (int x = 0; x < screen_width; ++x) {
      int pixelX = (horizontal_position_ + x);
      for (int y = 0; y < screen_height; ++y) {
        //Get the pixel in the bitmap
        int pixelY = (vertical_position_ + y);
          const Pixel &p = current_image_.getPixel(pixelX % current_image_.width, pixelY % current_image_.height);
          //Set the pixel in the screen buffer
          if(repeat_){// || (x < current_image_.width && y < current_image_.height)){
            matrix_->transformer()->Transform(offscreen_)->SetPixel(x, y, p.red, p.green, p.blue);
          }else if(
            //Horizontal check
            (x>=(current_image_.width-horizontal_position_) && x<=(screen_width-horizontal_position_)) ||
            (x>=(screen_width+current_image_.width-horizontal_position_)) ||
            //Vertical check
            (y>=(current_image_.height-vertical_position_) && y<=(screen_height-vertical_position_)) ||
            (y>=(screen_height+current_image_.height-vertical_position_))
          ){
            matrix_->transformer()->Transform(offscreen_)->SetPixel(x, y, 0, 0, 0);
          }else
            matrix_->transformer()->Transform(offscreen_)->SetPixel(x, y, p.red, p.green, p.blue);
      }
    }

      //Draw the image
      offscreen_ = matrix_->SwapOnVSync(offscreen_);

      //Move the bitmap on the screen
      scroll();

      if (scroll_ms_ <= 0) {
        // No scrolling. We don't need the image anymore.
        current_image_.Delete();
      } else {
        usleep(scroll_ms_ * 1000);
      }
    }
  }

  char * ImageScroller::ReadLine(FILE *f, char *buffer, size_t len) {
    char *result;
    do {
      result = fgets(buffer, len, f);
    } while (result != NULL && result[0] == '#');
    return result;
  }

  bool ImageScroller::LoadPPM(const char *filename) {
    FILE *f = fopen(filename, "r");
    // check if file exists
    if (f == NULL && access(filename, F_OK) == -1) {
      fprintf(stderr, "File \"%s\" doesn't exist\n", filename);
      return false;
    }
    if (f == NULL) return false;  //Already check before

    //Reading buffer
    char header_buf[256];

    //Read the first line of the file to get the ppm type
    const char *line = ReadLine(f, header_buf, sizeof(header_buf));
    #define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: %s |%s", filename, m, line); \
    fclose(f); return false; }

    //Check if the ppm type is P6
    if (sscanf(line, "P6 ") == EOF)
    EXIT_WITH_MSG("Can only handle P6 as PPM type.");

    //Read line two of the file to get the bitmap width & height
    line = ReadLine(f, header_buf, sizeof(header_buf));
    int new_width, new_height;
    if (!line || sscanf(line, "%d %d ", &new_width, &new_height) != 2)
    EXIT_WITH_MSG("Width/height expected");

    //Read line three of the file to get color range
    int value;
    line = ReadLine(f, header_buf, sizeof(header_buf));
    if (!line || sscanf(line, "%d ", &value) != 1 || value != 255)
    EXIT_WITH_MSG("Only 255 for maxval allowed.");

    //Calculate the size of the image in pixel
    const size_t pixel_count = new_width * new_height;
    Pixel *new_image = new Pixel [ pixel_count ];
    assert(sizeof(Pixel) == 3);   // we make that assumption.
    if (fread(new_image, sizeof(Pixel), pixel_count, f) != pixel_count) {
      line = "";
      EXIT_WITH_MSG("Not enough pixels read.");
    }
    #undef EXIT_WITH_MSG
    fclose(f);
    fprintf(stderr, "Read image '%s' with %dx%d\n", filename,
    new_width, new_height);
    horizontal_position_ = 0;
    MutexLock l(&mutex_new_image_);
    new_image_.Delete();  // in case we reload faster than is picked up
    new_image_.image = new_image;
    new_image_.width = new_width;
    new_image_.height = new_height;
    return true;
  }
