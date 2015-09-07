// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
#include "transformer.h"
#include "graphics.h"

#include "colorPulseGenerator.h"
#include "brightnessPulseGenerator.h"
#include "simpleSquare.h"
#include "grayScaleBlock.h"
#include "rotatingBlockGenerator.h"
#include "imageScroller.h"

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>

#define DEFAULT_PATH "/root/thethingbox/rpi-rgb-led-matrix-sh/v.ppm"

using std::min;
using std::max;

using namespace rgb_matrix;

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s <options> -D <demo-nr> [optional parameter]\n",
          progname);
  fprintf(stderr, "Options:\n"
          "\t-r <rows>     : Panel rows. '16' for 16x32 (1:8 multiplexing),\n"
	  "\t                '32' for 32x32 (1:16), '8' for 1:4 multiplexing; "
          "Default: 32\n"
          "\t-P <parallel> : For Plus-models or RPi2: parallel chains. 1..3. "
          "Default: 1\n"
          "\t-c <chained>  : Daisy-chained boards. Default: 1.\n"
          "\t-L            : 'Large' display, composed out of 4 times 32x32\n"
          "\t-p <pwm-bits> : Bits used for PWM. Something between 1..11\n"
          "\t-l            : Don't do luminance correction (CIE1931)\n"
          "\t-D <demo-nr>  : Always needs to be set\n"
          "\t-d            : run as daemon. Use this when starting in\n"
          "\t                /etc/init.d, but also when running without\n"
          "\t                terminal (e.g. cron).\n"
          "\t-t <seconds>  : Run for these number of seconds, then exit.\n"
          "\t                (if neither -d nor -t are supplied, waits for <RETURN>)\n"
          "\t-b <brightnes>: Sets brightness percent. Default: 100.\n"
          "\t-R <rotation> : Sets the rotation of matrix. Allowed: 0, 90, 180, 270. Default: 0.\n"
		  "\t-o <orientation> : 0 for horizontal & 1 for vertical\n"
		  "\t-v				: repeat the image if the image is smaller than the screen size\n");
  fprintf(stderr, "Demos, choosen with -D\n");
  fprintf(stderr, "\t0  - some rotating square\n"
          "\t1  - forward scrolling an image (-m <scroll-ms>)\n"
          "\t2  - backward scrolling an image (-m <scroll-ms>)\n"
          "\t3  - test image: a square\n"
          "\t4  - Pulsing color\n"
          "\t5  - Grayscale Block\n"
          "\t6  - Abelian sandpile model (-m <time-step-ms>)\n"
          "\t7  - Conway's game of life (-m <time-step-ms>)\n"
          "\t8  - Langton's ant (-m <time-step-ms>)\n"
          "\t9  - Volume bars (-m <time-step-ms>)\n"
          "\t10 - Evolution of color (-m <time-step-ms>)\n"
          "\t11 - Brightness pulse generator\n");
  fprintf(stderr, "Example:\n\t%s -t 10 -D 1 runtext.ppm\n"
          "Scrolls the runtext for 10 seconds\n", progname);
  return 1;
}

int main(int argc, char *argv[]) {
  GPIO io;
  //bool as_daemon = false; old value
  bool as_daemon = false;
  int runtime_seconds = -1;
  //int demo = -1; old value
  int demo = 1;
  int rows = 16;//32;
  int chain = 1;
  int parallel = 1;
  int scroll_ms = 30;
  int pwm_bits = -1;
  int brightness = 100;
  int rotation = 0;
  bool large_display = false;
  bool do_luminance_correct = true;
  bool repeat = false;
  direction orientation = horizontal;

  const char *demo_parameter = DEFAULT_PATH;

  int opt;
  while ((opt = getopt(argc, argv, "dlD:t:r:P:c:p:b:m:LR:o:v")) != -1) {
    switch (opt) {
    case 'D':
      demo = atoi(optarg);
      break;

    case 'd':
      as_daemon = true;
      break;

    case 't':
      runtime_seconds = atoi(optarg);
      break;

    case 'r':
      rows = atoi(optarg);
      break;

    case 'P':
      parallel = atoi(optarg);
      break;

    case 'c':
      chain = atoi(optarg);
      break;

    case 'm':
      scroll_ms = atoi(optarg);
      break;

    case 'p':
      pwm_bits = atoi(optarg);
      break;

    case 'b':
      brightness = atoi(optarg);
      break;

    case 'l':
      do_luminance_correct = !do_luminance_correct;
      break;

    case 'L':
      // The 'large' display assumes a chain of four displays with 32x32
      chain = 4;
      rows = 32;
      large_display = true;
      break;

    case 'R':
      rotation = atoi(optarg);
      break;

    case 'o':
      //Scrolling direction
      orientation = (atoi(optarg)==0)?horizontal:vertical;
      break;

    case 'v':
      repeat = true;
      break;
    default:
      return usage(argv[0]);
    }
  }


  if (optind < argc) {
    demo_parameter = argv[optind];
  }

  if (demo < 0) {
    fprintf(stderr, "Expected required option -D <demo>\n");
    return usage(argv[0]);
  }

  if (getuid() != 0) {
    fprintf(stderr, "Must run as root to be able to access /dev/mem\n"
            "Prepend 'sudo' to the command:\n\tsudo %s ...\n", argv[0]);
    return 1;
  }

  if (rows != 8 && rows != 16 && rows != 32) {
    fprintf(stderr, "Rows can one of 8, 16 or 32 "
            "for 1:4, 1:8 and 1:16 multiplexing respectively.\n");
    return 1;
  }

  if (chain < 1) {
    fprintf(stderr, "Chain outside usable range\n");
    return 1;
  }
  if (chain > 8) {
    fprintf(stderr, "That is a long chain. Expect some flicker.\n");
  }
  if (parallel < 1 || parallel > 3) {
    fprintf(stderr, "Parallel outside usable range.\n");
    return 1;
  }

  if (brightness < 1 || brightness > 100) {
    fprintf(stderr, "Brightness is outside usable range.\n");
    return 1;
  }

  if (rotation % 90 != 0) {
    fprintf(stderr, "Rotation %d not allowed! Only 0, 90, 180 and 270 are possible.\n", rotation);
    return 1;
  }

  // Initialize GPIO pins. This might fail when we don't have permissions.
  if (!io.Init())
    return 1;

  // Start daemon before we start any threads.
  if (as_daemon) {
    if (fork() != 0)
      return 0;
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }

  // The matrix, our 'frame buffer' and display updater.
  RGBMatrix *matrix = new RGBMatrix(&io, rows, chain, parallel);
  matrix->set_luminance_correct(do_luminance_correct);
  matrix->SetBrightness(brightness);
  if (pwm_bits >= 0 && !matrix->SetPWMBits(pwm_bits)) {
    fprintf(stderr, "Invalid range of pwm-bits\n");
    return 1;
  }

  LinkedTransformer *transformer = new LinkedTransformer();
  matrix->SetTransformer(transformer);

  if (large_display) {
    // Mapping the coordinates of a 32x128 display mapped to a square of 64x64
    transformer->AddTransformer(new LargeSquare64x64Transformer());
  }

  //Set the led-matrix orientation
  if (rotation > 0) {
    led_matrix_orientation led_matrix_orientation;
    switch (rotation) {
      case HORIZONTAL:
        led_matrix_orientation = HORIZONTAL;
        break;
      case HORIZONTAL_UPSIDE_DOWN:
        led_matrix_orientation = HORIZONTAL_UPSIDE_DOWN;
        break;
      case VERTICAL:
        led_matrix_orientation = VERTICAL;
        break;
      case VERTICAL_UPSIDE_DOWN:
        led_matrix_orientation = VERTICAL_UPSIDE_DOWN;
        break;
      default:
        led_matrix_orientation = HORIZONTAL;
        break;
    }
    transformer->AddTransformer(new RotateTransformer(led_matrix_orientation));
  }

  Canvas *canvas = matrix;

  // The ThreadedCanvasManipulator objects are filling
  // the matrix continuously.
  ThreadedCanvasManipulator *image_gen = NULL;
  switch (demo) {
  case 0:
    image_gen = new RotatingBlockGenerator(canvas);
    break;

  case 1:
  case 2:
    if (demo_parameter) {
      ImageScroller *scroller = new ImageScroller(matrix,
                                                  demo == 1 ? 1 : -1,
                                                  scroll_ms,orientation,repeat);
      if (!scroller->LoadPPM(demo_parameter))
        return 1;
      image_gen = scroller;
    } else {
      fprintf(stderr, "Demo %d Requires PPM image as parameter\n", demo);
      return 1;
    }
    break;

  case 3:
    image_gen = new SimpleSquare(canvas);
    break;

  case 4:
    image_gen = new ColorPulseGenerator(matrix);
    break;

  case 5:
    image_gen = new GrayScaleBlock(canvas);
    break;

  case 6:
    //image_gen = new Sandpile(canvas, scroll_ms);
    break;

  case 7:
    //image_gen = new GameLife(canvas, scroll_ms);
    break;

  case 8:
    //image_gen = new Ant(canvas, scroll_ms);
    break;

  case 9:
    //image_gen = new VolumeBars(canvas, scroll_ms, canvas->width()/2);
    break;

  case 10:
    //image_gen = new GeneticColors(canvas, scroll_ms);
    break;

  case 11:
    image_gen = new BrightnessPulseGenerator(matrix);
    break;
  }

  if (image_gen == NULL)
    return usage(argv[0]);

  // Image generating demo is crated. Now start the thread.
  image_gen->Start();

  // Now, the image genreation runs in the background. We can do arbitrary
  // things here in parallel. In this demo, we're essentially just
  // waiting for one of the conditions to exit.
  if (as_daemon) {
    sleep(runtime_seconds > 0 ? runtime_seconds : INT_MAX);
  } else if (runtime_seconds > 0) {
    sleep(runtime_seconds);
  } else {
    // Things are set up. Just wait for <RETURN> to be pressed.
    printf("Press <RETURN> to exit and reset LEDs\n");
    getchar();
  }

  // Stop image generating thread.
  delete image_gen;
  delete canvas;

  transformer->DeleteTransformers();
  delete transformer;

  return 0;
}
