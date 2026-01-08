/*
    Gravity Strike -- main

    (c) 2001/2002 by Jens Hassler
*/

#include "gsmain.h"
#include "gsglob.h"

#include "headless_screen.h"

int gfxdriver = GFX_AUTODETECT;
int debuglevel = 0;
int noclip = 0;
int nocrcmessage = 0;
int verbose = 0;  // Added verbose flag
int extra_verbose = 0;  // Added extra verbose flag
int extra_verbose2 = 0; // Added extra verbose 2 flag for headless output
char* headless_output_path = nullptr; // Path for headless screen output
// mainloop_verbose is defined in gsglob.cpp


void parse_cmd_line(int argc, char *argv[])
{
  printf("Parsing command line arguments: argc=%d\n", argc);
  if (argc > 1)
  for (int n=1; n < argc; n++)
  {
    printf("Argument %d: %s\n", n, argv[n]);
    #ifdef ALLEGRO_WINDOWS
    if (!strcmp(argv[n], "/window"))
        gfxdriver = GFX_DIRECTX_WIN;
    #endif

    if (!strcmp(argv[n], "/debug"))
      debuglevel = 2;

    if (!strcmp(argv[n], "/noclip"))
      noclip = 1;
      
    if (!strcmp(argv[n], "/nocrcmessage"))
      nocrcmessage = 1;

    if (!strcmp(argv[n], "-v") || !strcmp(argv[n], "--verbose"))
      verbose = 1;
    if (!strcmp(argv[n], "-v2") || !strcmp(argv[n], "--extra-verbose"))
      extra_verbose = 1;
    if (!strcmp(argv[n], "-v3") || !strcmp(argv[n], "--mainloop-verbose")) {
      mainloop_verbose = 1;
    }
    if (!strcmp(argv[n], "-v4") || !strcmp(argv[n], "--extra-verbose-2")) {
      extra_verbose2 = 1;
    }
    if ((!strcmp(argv[n], "-vo") || !strcmp(argv[n], "--headless-output")) && n + 1 < argc) {
      headless_output_path = argv[n + 1];
      n++; // Skip the next argument since we're using it as the path
    }

    #ifdef ALLEGRO_LINUX
    if (!strcmp(argv[n], "/xdga2"))
      gfxdriver = GFX_XDGA2;
    else if (!strcmp(argv[n], "/xdga_win"))
      gfxdriver = GFX_XDGA;
    else if (!strcmp(argv[n], "/xdga"))
      gfxdriver = GFX_XDGA_FULLSCREEN;
    else if (!strcmp(argv[n], "/xwindows_win"))
      gfxdriver = GFX_XWINDOWS;
    else if (!strcmp(argv[n], "/xwindows"))
      gfxdriver = GFX_XWINDOWS_FULLSCREEN;

    #endif

  }
}

int main(int argc, char *argv[])
{
  printf("Gravity Strike starting...\n");
  fflush(stdout);  // Make sure output is flushed immediately
  parse_cmd_line(argc, argv);

  if (verbose) {
    printf("Gravity Strike starting with verbose output...\n");
    printf("Attempting to initialize game...\n");
  }
  
  if (mainloop_verbose) {
    printf("Main loop verbose output enabled (-v3 flag)\n");
  }

  // Initialize HeadlessScreen if output path is specified
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->init(headless_output_path);
    printf("HeadlessScreen initialized with output path: %s\n", headless_output_path);
    
    // Also enable mainloop verbose when headless output is enabled
    mainloop_verbose = 1;
  }

  if (extra_verbose2) {
    printf("Extra verbose 2 enabled (-v4 flag)\n");
  }

  // main game object
  printf("Creating gsMain object...\n");
  fflush(stdout);
  gsMain *gs_main = new gsMain();

  printf("Initializing game...\n");
  fflush(stdout);
  int init_result = gs_main->init();
  
  if (verbose) {
    if (init_result == 0) {
      printf("Initialization successful, starting game...\n");
    } else {
      printf("Initialization failed with error code: %d\n", init_result);
    }
  }

  if (init_result == 0)  // Fixed: only play if init succeeds (returns 0)
  {
    printf("Starting game loop...\n");
    fflush(stdout);
    gs_main->play();
    
    // Flush headless screen output if it was enabled
    if (headless_output_path) {
      HeadlessScreen* hs = HeadlessScreen::get_instance();
      hs->flush_output();
    }
  }
  else
  {
    printf("Game initialization failed with error code: %d\n", init_result);
    fflush(stdout);
  }

  if (verbose) {
    printf("Game finished, cleaning up...\n");
  }

  delete(gs_main);

  if (verbose) {
    printf("Exiting Gravity Strike.\n");
  }

  return 0;
}

