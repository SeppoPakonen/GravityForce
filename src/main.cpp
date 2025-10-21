/*
    Gravity Strike -- main

    (c) 2001/2002 by Jens Hassler
*/

#include "gsmain.h"
#include "gsglob.h"

int gfxdriver = GFX_AUTODETECT;
int debuglevel = 0;
int noclip = 0;
int nocrcmessage = 0;
int verbose = 0;  // Added verbose flag
int extra_verbose = 0;  // Added extra verbose flag
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

