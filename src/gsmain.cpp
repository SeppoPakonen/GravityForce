/*
 *   GRAVITY STRIKE -- gsmain.cpp
 *
 *
 *   (c) Copyright 2001/2002 by Jens Hassler <jh@gravity-strike.de>
 *       www.gravity-strike.de
 *
 *   see source.txt for details
 *
 */

#define DO_CRC_CHECKS  1
#define MAKE_CRC_FILE  0

#include "gsmain.h"             // main
#include "gsglob.h"
#include "gserror.h"
#include "gsmap.h"
#include "gsclear.h"
#include "gsplayer.h"
#include "gspanel.h"
#include "gseffect.h"
#include "gscontrl.h"
#include "gslevel.h"
#include "gslvlnw.h"
#include "gslvlsp.h"
#include "gsbase.h"
#include "gsobject.h"
#include "gslng.h"
#include "gsfade.h"
#include "gssound.h"
#include "gsnetw.h"
#include "gsreplay.h"
#include "gsenemy.h"
#include "gshelper.h"
#include "allegro5_wrapper.h"
#include "gsframes.h"
#include "gshiscor.h"
#include "gsplcomp.h"
#include "menu/gsmenu.h"
#include "contrib/md5class.h"
#include "gsscript.h"
#include "headless_screen.h"

// Import verbose flags from main.cpp
extern int verbose;
extern int extra_verbose;
extern int extra_verbose2;
extern char* headless_output_path;

// Import verbose flags from main.cpp
extern int verbose;
extern int extra_verbose;

gsGlobals   *globals    = NULL;
gsError     *errors     = NULL;
gsLanguage  *language   = NULL;
gsScript    *script     = NULL;
gsFade      *fade       = NULL;
gsSound     *gsound     = NULL;
gsNetwork   *net        = NULL;
gsMap       *playmap[2];
gsClear     *clearlist  = NULL;
gsPlayer    *player[8];             // maximum of 8 players
gsPanel     *gpanel[2];
gsEffect    *effects    = NULL;
gsBase      *bases      = NULL;
gsObject    *objects    = NULL;
gsEnemy     *enemies    = NULL;
gsImage     *images     = NULL;
gsReplay    *replay     = NULL;
gsHiscore   *hiscore    = NULL;
gsCompPlayer *comp_player[2];

void gs_fps_timer(void)
{
  globals->fpst = globals->fps;
  globals->fps  = 0;
}
END_OF_FUNCTION(fps_timer);

void game_timer(void)
{
  globals->game_time++;
  globals->overall_game_time++;
}
END_OF_FUNCTION(game_timer);

void gs_second_timer(void)
{
  if (!globals->pause)
    globals->second_time++;
}
END_OF_FUNCTION(gs_second_timer);

gsMain::gsMain()
{
}

gsMain::~gsMain()
{
  exit();
}


int gsMain::init()
{
  if (verbose) {
    printf("Initializing globals and errors...\n");
  }
  globals = new gsGlobals;
  errors  = new gsError;

  set_uformat(U_ASCII);

  if (verbose) {
    printf("Initializing Allegro library...\n");
  }
  errors->log(2, "init", "allegro");
  allegro_init();
  errors->log(2, "init", "library version", al_get_allegro_version());

  set_window_title("Gravity Strike");

  if (verbose) {
    printf("Setting up scripting system...\n");
  }
  // scripting
  errors->log(2, "init", "scripting");
  script = new gsScript;

  if (verbose) {
    printf("Loading game configuration...\n");
  }
  // game config
  errors->log(2, "init", "loading game config");
  script->load_game_config();

  // set keyboard layout
  char my_layout[3];
  switch (globals->language)
  {
    case LANG_DE :
      strcpy(my_layout, "de");
      break;
    case LANG_EN :
      strcpy(my_layout, "en");
      break;
    case LANG_CR :
      strcpy(my_layout, "en");
      break;
    default:
      strcpy(my_layout, "en");
      break;
  }
  // In Allegro 5, we don't need to set config data for keyboard layout
  // The keyboard layout is handled by the OS and Allegro 5 automatically

  errors->log(2, "init", "setting keyboard layout", my_layout);

  if (verbose) {
    printf("Initializing keyboard...\n");
  }
  errors->log(2, "init", "keyboard");
  install_keyboard();

  if (verbose) {
    printf("Reading level files...\n");
  }
  // level files and crc
  script->read_level_files(1);

  if (verbose) {
    printf("Performing CRC checks...\n");
  }
  errors->log(2, "init", "doing crc checks");
  hiscore = new gsHiscore();
  if (MAKE_CRC_FILE) hiscore->make_crc_file();
  if (DO_CRC_CHECKS)
  {
    if (hiscore->do_crc_checks())
    {
      extern int nocrcmessage;
      if (!nocrcmessage) allegro_message("ERROR: One or more of your global game files are invalid. Highscores will not work at all!\n");
      errors->log(2, "ERROR", "One or more global files are invalid. Highscores will not work at all!");
      globals->crc_failed = 1;
    }
  }

  if (verbose) {
    printf("Initializing mouse...\n");
  }
  errors->log(2, "init", "mouse");
  if (install_mouse() <= 0)
  {
    if (verbose) {
      printf("WARNING: No mouse detected. Running in mouseless environment.\n");
    }
    errors->log(2, "WARNING", "no mouse detected, continuing without mouse support!");
    // Continue execution instead of failing - some environments (remote, headless) may not have a mouse
    // Note: Allegro might handle mouse absence gracefully in the game itself
  }

  #ifdef ALLEGRO_WINDOWS
    set_mouse_speed(4,4);
  #endif

  if (verbose) {
    printf("Initializing joystick...\n");
  }
  errors->log(2, "init", "trying to initialize joystick");
  #ifdef ALLEGRO_DOS
    if (install_joystick(JOY_TYPE_4BUTTON) == 0) globals->joystick_installed = 1; else install_joystick(JOY_TYPE_NONE);
  #else
    if (install_joystick(JOY_TYPE_AUTODETECT) == 0) globals->joystick_installed = 1; else install_joystick(JOY_TYPE_NONE);
  #endif
  if (globals->joystick_installed) errors->log(2, "init", "joystick(s) found!", 2);
  else errors->log(2, "init", "sorry, no joystick available");

  if (verbose) {
    printf("Initializing timer...\n");
  }
  errors->log(2, "init", "timer");
  install_timer();

  LOCK_VARIABLE(globals->game_time);
  LOCK_VARIABLE(globals->overall_game_time);
  LOCK_VARIABLE(globals->fps);
  LOCK_VARIABLE(globals->fpst);
  LOCK_VARIABLE(globals->second_time);
  LOCK_FUNCTION(gs_fps_timer);
  LOCK_FUNCTION(game_timer);
  LOCK_FUNCTION(gs_second_timer);

  install_int(gs_fps_timer,1000);
  install_int_ex(game_timer,BPS_TO_TIMER(60));
  install_int_ex(gs_second_timer,SECS_TO_TIMER(1));

  if (verbose) {
    printf("Loading language files...\n");
  }
  // language files
  errors->log(2, "init", "language files");
  language = new gsLanguage;
  script->load_text();

  if (verbose) {
    printf("Initializing clear list...\n");
  }
  // clearlist
  errors->log(2, "init", "clear list");
  clearlist = new gsClear;

  if (verbose) {
    printf("Initializing sound system...\n");
  }
  errors->log(2, "init", "sound");
  gsound = new gsSound();

  if (verbose) {
    printf("Initializing network system...\n");
  }
  // network
  errors->log(2, "init", "networking");
  net = new gsNetwork();

  if (verbose) {
    printf("Loading data files...\n");
  }
  // init datafiles
  errors->log(2, "init", "datafiles");
  if (verbose) {
    printf("Loading data files from directory: '%s'\n", globals->data_dir);
    printf("Menu datafile name: '%s'\n", globals->menu_datafile_name);
    printf("Game datafile name: '%s'\n", globals->datafile_name);
    printf("Ship datafile name: '%s'\n", globals->shipdata_name);
    printf("Font datafile name: '%s'\n", globals->fontdata_name);
  }
  
  packfile_password(globals->pwd);
  
  if (verbose) {
    printf("Data loading: Password set to '%s'\n", globals->pwd);
  }
  
  // Function to get absolute path of executable directory and look for data files
  // First, try the default paths from the globals
  char full_path[255];
  char original_path[255];
  
  // Try to load menu datafile
  strcpy(original_path, globals->data_dir);
  strcat(original_path, globals->menu_datafile_name);
  strcpy(full_path, original_path);
  
  if (verbose) {
    printf("Attempting to load menu datafile: '%s'\n", full_path);
    printf("Data file name: '%s'\n", globals->menu_datafile_name);
    printf("Data directory: '%s'\n", globals->data_dir);
  }
  
  // First, try to check if file exists using basic file operations
  FILE *test_file = fopen(full_path, "rb");
  if (test_file) {
    if (verbose) {
      printf("File '%s' exists and is readable, size: ", full_path);
      fseek(test_file, 0, SEEK_END);
      long size = ftell(test_file);
      printf("%ld bytes\n", size);
      fclose(test_file);
    }
  } else {
    if (verbose) {
      printf("ERROR: File '%s' does not exist or is not readable\n", full_path);
    }
  }
  
  // Try to load with original path first
  globals->menudat = load_datafile(full_path);
  if (!globals->menudat) {
    if (verbose) {
      printf("Primary path load_datafile() failed for menu datafile\n");
      printf("Trying explicit './dat/' path: './dat/%s'\n", globals->menu_datafile_name);
    }
    // If it failed, try explicit './dat/' path
    sprintf(full_path, "./dat/%s", globals->menu_datafile_name);
    test_file = fopen(full_path, "rb");
    if (test_file) {
      if (verbose) {
        printf("File '%s' exists and is readable for menu datafile, size: ", full_path);
        fseek(test_file, 0, SEEK_END);
        long size = ftell(test_file);
        printf("%ld bytes\n", size);
        fclose(test_file);
      }
    } else {
      if (verbose) {
        printf("ERROR: File '%s' does not exist or is not readable for menu datafile\n", full_path);
      }
    }
    globals->menudat = load_datafile(full_path);
  }
  
  if (!globals->menudat && verbose) {
    printf("ERROR: Can't load menu datafile '%s'\n", full_path);
  }
  
  // Try to load game datafile with same detailed debugging
  strcpy(original_path, globals->data_dir);
  strcat(original_path, globals->datafile_name);
  if (verbose) {
    printf("Attempting to load game datafile: '%s'\n", original_path);
  }
  
  test_file = fopen(original_path, "rb");
  if (test_file) {
    if (verbose) {
      printf("File '%s' exists and is readable, size: ", original_path);
      fseek(test_file, 0, SEEK_END);
      long size = ftell(test_file);
      printf("%ld bytes\n", size);
      fclose(test_file);
    }
  } else {
    if (verbose) {
      printf("ERROR: File '%s' does not exist or is not readable\n", original_path);
    }
  }
  
  globals->gamedat = load_datafile(original_path);
  if (!globals->gamedat) {
    if (verbose) {
      printf("Primary path load_datafile() failed for game datafile\n");
      printf("Trying explicit './dat/' path: './dat/%s'\n", globals->datafile_name);
    }
    sprintf(full_path, "./dat/%s", globals->datafile_name);
    test_file = fopen(full_path, "rb");
    if (test_file) {
      if (verbose) {
        printf("File '%s' exists and is readable for game datafile, size: ", full_path);
        fseek(test_file, 0, SEEK_END);
        long size = ftell(test_file);
        printf("%ld bytes\n", size);
        fclose(test_file);
      }
    } else {
      if (verbose) {
        printf("ERROR: File '%s' does not exist or is not readable for game datafile\n", full_path);
      }
    }
    globals->gamedat = load_datafile(full_path);
  }
  
  if (!globals->gamedat && verbose) {
    printf("ERROR: Can't load game datafile\n");
  }
  
  // Try to load ship datafile with same detailed debugging
  strcpy(original_path, globals->data_dir);
  strcat(original_path, globals->shipdata_name);
  if (verbose) {
    printf("Attempting to load ship datafile: '%s'\n", original_path);
  }
  
  test_file = fopen(original_path, "rb");
  if (test_file) {
    if (verbose) {
      printf("File '%s' exists and is readable, size: ", original_path);
      fseek(test_file, 0, SEEK_END);
      long size = ftell(test_file);
      printf("%ld bytes\n", size);
      fclose(test_file);
    }
  } else {
    if (verbose) {
      printf("ERROR: File '%s' does not exist or is not readable\n", original_path);
    }
  }
  
  globals->shipdat = load_datafile(original_path);
  if (!globals->shipdat) {
    if (verbose) {
      printf("Primary path load_datafile() failed for ship datafile\n");
      printf("Trying explicit './dat/' path: './dat/%s'\n", globals->shipdata_name);
    }
    sprintf(full_path, "./dat/%s", globals->shipdata_name);
    test_file = fopen(full_path, "rb");
    if (test_file) {
      if (verbose) {
        printf("File '%s' exists and is readable for ship datafile, size: ", full_path);
        fseek(test_file, 0, SEEK_END);
        long size = ftell(test_file);
        printf("%ld bytes\n", size);
        fclose(test_file);
      }
    } else {
      if (verbose) {
        printf("ERROR: File '%s' does not exist or is not readable for ship datafile\n", full_path);
      }
    }
    globals->shipdat = load_datafile(full_path);
  }
  
  if (!globals->shipdat && verbose) {
    printf("ERROR: Can't load ship datafile\n");
  }
  
  // Try to load font datafile with same detailed debugging
  strcpy(original_path, globals->data_dir);
  strcat(original_path, globals->fontdata_name);
  if (verbose) {
    printf("Attempting to load font datafile: '%s'\n", original_path);
  }
  
  test_file = fopen(original_path, "rb");
  if (test_file) {
    if (verbose) {
      printf("File '%s' exists and is readable, size: ", original_path);
      fseek(test_file, 0, SEEK_END);
      long size = ftell(test_file);
      printf("%ld bytes\n", size);
      fclose(test_file);
    }
  } else {
    if (verbose) {
      printf("ERROR: File '%s' does not exist or is not readable\n", original_path);
    }
  }
  
  globals->fontdat = load_datafile(original_path);
  if (!globals->fontdat) {
    if (verbose) {
      printf("Primary path load_datafile() failed for font datafile\n");
      printf("Trying explicit './dat/' path: './dat/%s'\n", globals->fontdata_name);
    }
    sprintf(full_path, "./dat/%s", globals->fontdata_name);
    test_file = fopen(full_path, "rb");
    if (test_file) {
      if (verbose) {
        printf("File '%s' exists and is readable for font datafile, size: ", full_path);
        fseek(test_file, 0, SEEK_END);
        long size = ftell(test_file);
        printf("%ld bytes\n", size);
        fclose(test_file);
      }
    } else {
      if (verbose) {
        printf("ERROR: File '%s' does not exist or is not readable for font datafile\n", full_path);
      }
    }
    globals->fontdat = load_datafile(full_path);
  }
  
  if (!globals->fontdat && verbose) {
    printf("ERROR: Can't load font datafile\n");
  }
  
  packfile_password(NULL);
  
  // Extra verbose output: dump data structure information
  if (extra_verbose) {
    printf("=== EXTRA VERBOSE DUMP (before data file validation) ===\n");
    
    if (globals->menudat) {
      printf("MENU DATAFILE CONTENTS:\n");
      // Try to determine the number of objects in a safer way
      int count = 0;
      DATAFILE* df = globals->menudat;
      while (df && df->type != 0) {  // Use 0 as end marker instead of DAT_END
        count++;
        // Safety check to avoid infinite loops
        if (count > 1000) break;
        df++;
      }
      printf("- Number of objects in menudat: %d\n", count);
      
      // Print first few entries
      for (int i = 0; i < count && i < 10; i++) {
        printf("  [%d] type=0x%x, size=%ld", i, 
               globals->menudat[i].type, 
               (long)globals->menudat[i].size);
        printf(", dat=0x%p", globals->menudat[i].dat);
        printf("\n");
      }
    } else {
      printf("MENU DATAFILE: NULL (not loaded)\n");
    }
    
    if (globals->gamedat) {
      printf("\nGAME DATAFILE CONTENTS:\n");
      // Try to determine the number of objects in a safer way
      int count = 0;
      DATAFILE* df = globals->gamedat;
      while (df && df->type != 0) {  // Use 0 as end marker instead of DAT_END
        count++;
        // Safety check to avoid infinite loops
        if (count > 1000) break;
        df++;
      }
      printf("- Number of objects in gamedat: %d\n", count);
      
      // Print first few entries
      for (int i = 0; i < count && i < 10; i++) {
        printf("  [%d] type=0x%x, size=%ld", i, 
               globals->gamedat[i].type, 
               (long)globals->gamedat[i].size);
        printf(", dat=0x%p", globals->gamedat[i].dat);
        printf("\n");
      }
    } else {
      printf("GAME DATAFILE: NULL (not loaded)\n");
    }
    
    if (globals->shipdat) {
      printf("\nSHIP DATAFILE CONTENTS:\n");
      // Try to determine the number of objects in a safer way
      int count = 0;
      DATAFILE* df = globals->shipdat;
      while (df && df->type != 0) {  // Use 0 as end marker instead of DAT_END
        count++;
        // Safety check to avoid infinite loops
        if (count > 1000) break;
        df++;
      }
      printf("- Number of objects in shipdat: %d\n", count);
      
      // Print first few entries
      for (int i = 0; i < count && i < 10; i++) {
        printf("  [%d] type=0x%x, size=%ld", i, 
               globals->shipdat[i].type, 
               (long)globals->shipdat[i].size);
        printf(", dat=0x%p", globals->shipdat[i].dat);
        printf("\n");
      }
    } else {
      printf("SHIP DATAFILE: NULL (not loaded)\n");
    }
    
    if (globals->fontdat) {
      printf("\nFONT DATAFILE CONTENTS:\n");
      // Try to determine the number of objects in a safer way
      int count = 0;
      DATAFILE* df = globals->fontdat;
      while (df && df->type != 0) {  // Use 0 as end marker instead of DAT_END
        count++;
        // Safety check to avoid infinite loops
        if (count > 1000) break;
        df++;
      }
      printf("- Number of objects in fontdat: %d\n", count);
      
      // Print first few entries
      for (int i = 0; i < count && i < 10; i++) {
        printf("  [%d] type=0x%x, size=%ld", i, 
               globals->fontdat[i].type, 
               (long)globals->fontdat[i].size);
        printf(", dat=0x%p", globals->fontdat[i].dat);
        printf("\n");
      }
    } else {
      printf("FONT DATAFILE: NULL (not loaded)\n");
    }
    
    printf("=== END EXTRA VERBOSE DUMP ===\n");
  }
  
  // Check if data files loaded successfully
  if (!globals->menudat) { 
    if (verbose) {
      printf("ERROR: Can't load menu datafile!\n");
    }
    allegro_message("Can't load menu datafile!"); 
    return 1; 
  }
  if (!globals->gamedat) { 
    if (verbose) {
      printf("ERROR: Can't load game datafile!\n");
    }
    allegro_message("Can't load game datafile!"); 
    return 1; 
  }
  if (!globals->shipdat) { 
    if (verbose) {
      printf("ERROR: Can't load ship datafile!\n");
    }
    allegro_message("Can't load ship datafile!"); 
    return 1; 
  }
  if (!globals->fontdat) { 
    if (verbose) {
      printf("ERROR: Can't load font datafile!\n");
    }
    allegro_message("Can't load font datafile!"); 
    return 1; 
  }

  // delete temporary ships (from previous crashes while network gaminng :)
//  net->delete_tmp_ship_files();

  // init graphics
  extern int gfxdriver;

  if (verbose) {
    printf("Initializing graphics mode...\n");
  }
  errors->log(2, "init", "gfx mode");
  if (set_gfx_mode(gfxdriver, globals->menu_resolution_x, globals->menu_resolution_y, 0, 0))
  {
    if (verbose) {
      printf("ERROR: Failed to set graphics mode: %s\n", allegro_error);
    }
    errors->log(0, "FATAL ERROR", allegro_error);
    return 1;
  }
  set_display_switch_mode(SWITCH_BACKGROUND);

  if (verbose) {
    printf("Creating map images...\n");
  }
  errors->log(2, "init", "looking for map images to be created...");
  create_map_images();

  if (verbose) {
    printf("Initializing replay system...\n");
  }
  // TEMP
  replay = new gsReplay();
  // TEMP

  if (verbose) {
    printf("Initialization completed successfully.\n");
  }
  return 0;
}

void gsMain::exit()
{
errors->log(2, "game", "removing timer");
  remove_timer(); 

errors->log(2, "game", "unloading datafiles");
  if (globals->menudat) unload_datafile(globals->menudat); globals->menudat = NULL;
  if (globals->gamedat) unload_datafile(globals->gamedat); globals->gamedat = NULL;
  if (globals->shipdat) unload_datafile(globals->shipdat); globals->shipdat = NULL;
  if (globals->fontdat) unload_datafile(globals->fontdat); globals->fontdat = NULL;

errors->log(2, "game", "destroying sound, net, scripts, clearlist, hiscore");
  if (gsound)        delete(gsound);      gsound = NULL;
  if (net)           delete(net);         net = NULL;
  if (script)        delete(script);      script = NULL;
  if (clearlist)     delete(clearlist);   clearlist = NULL;
  if (hiscore)       delete(hiscore);     hiscore = NULL;

  // TEMP
  if (replay)        delete(replay);      replay = NULL;
  // TEMP

errors->log(2, "game", "destroying errors, language, globals");
  if (errors)        delete(errors);      errors = NULL;
  if (language)      delete(language);    language = NULL;
  if (globals)       delete(globals);     globals = NULL;

  allegro_exit();
}

void gsMain::create_map_image(int levnum)
{
  char tileset[255];
  char marpath[255];
  int zoom;
  BITMAP *tempbit, *minimap;
  int error = 0;
  int image_size = 115;

  sprintf(marpath, "%s%s/%s", globals->level_dir,
                              globals->level_files[levnum].leveldir,
                              globals->level_files[levnum].mapfile);

  char outfile[255];
  replace_extension(outfile, marpath, "tga", 50);

  //errors->log(2, "outfile", outfile, marpath);

  if (exists(outfile))
  {
    error = 1; // map is already created, nothing to do...
  }

  if (!error)
  {
    errors->log(2, "init", "creating map image for map...", get_filename(marpath));

    // path for tileset
    strcpy(tileset, globals->data_dir);
    strcat(tileset, globals->level_files[levnum].map_style);

    if (MapLoad(tileset))
    {
      // alternative path for tileset
      sprintf(tileset, "%s%s/%s", globals->level_dir,
                                  globals->level_files[levnum].leveldir,
                                  globals->level_files[levnum].map_style);
     if (MapLoad(tileset))
     {
        errors->log(0, "ERROR", "loading tileset", globals->level_files[levnum].map_style);
        return;
      }
    }

    if (MapLoadMAR(marpath, 0))
    {
      errors->log(0, "ERROR", "loading map", marpath);
      return;
    }

    // get farthest "0" tile
    int far_x = 0;
    int far_y = 0;
    for (int xe=99; xe > 0; xe--)
      for (int ye=99; ye > 0; ye--)
        if (!(BLKSTR*)MapGetBlock(xe, ye)->tl)
        {
          if (xe > far_x) far_x = xe;
          if (ye > far_y) far_y = ye;
        }

    //errors->log(2, get_filename(marpath), far_x, far_y);

    int farthest_line = MAX(far_x, far_y);
    if (farthest_line < 90) farthest_line += 10; else farthest_line = 100;

    // set up new zoom
    zoom = (farthest_line * globals->tile_w) / image_size;
    
    int draw_w = globals->tile_w * 10;
    int draw_h = globals->tile_h * 10;
  
    minimap = create_bitmap(image_size, image_size);
    tempbit = create_bitmap(draw_w, draw_h);
    clear_bitmap(tempbit); clear_bitmap(minimap);

    int newsize = (farthest_line/10)+1;
    if (newsize > 10) newsize = 10;

    for (int w=0; w < newsize; w++)
      for (int h=0; h < newsize; h++)
      {
        // draw 100 tiles
        MapDrawBG(tempbit, draw_w * w, draw_h * h, 0, 0, draw_w, draw_h);
      
        // scale
        stretch_blit(tempbit, minimap,
                     0, 0, draw_w, draw_h,
                     (draw_w / zoom) * w, (draw_h / zoom) * h,
                     draw_w / zoom, draw_h / zoom);
      }
  
    destroy_bitmap(tempbit);

    char str_mapsize[10];
    sprintf(str_mapsize, "%02dx%02d", far_x+1, far_y+1);
    text_mode(-1);

    aatextout_right(minimap, (FONT*)globals->fontdat[font_impact].dat, str_mapsize, image_size-4, image_size-17, makecol8(0, 0, 0));
    aatextout_right(minimap, (FONT*)globals->fontdat[font_impact].dat, str_mapsize, image_size-3, image_size-16, makecol8(89, 102, 251));

    save_tga(outfile, minimap, globals->mainpal);
  
    destroy_bitmap(minimap);
  
    MapFreeMem();
  }
}

void gsMain::create_map_images()
{
  char datafile[255];

  strcpy(datafile, globals->data_dir);
  strcat(datafile, globals->datafile_name);

  packfile_password(globals->pwd);
  DATAFILE *dat = load_datafile_object(datafile, "gamepal");
  packfile_password(NULL);
  globals->mainpal = (PALETTE *)dat->dat;
  set_palette((RGB *)globals->mainpal);

  for (int n=0; n < globals->max_levels; n++)
  {
    create_map_image(n);
  }

  unload_datafile_object(dat);
}

void gsMain::do_sp_level()
{
  splevel = new gsSingleLevel();

  if ( splevel->init_stuff() )
  {
    errors->log(0, "FATAL ERROR", splevel->get_error_str());
  }
  else
  {
    errors->log(2, "Game", "single player level created, starting");
    splevel->play();
    errors->log(2, "Game", "single player level done");
  }

  delete(splevel);
}


void gsMain::do_level()
{
  level = new gsLevel();

  if ( level->init_stuff() )
  {
    errors->log(0, "FATAL ERROR", level->get_error_str());
  }
  else
  {
    errors->log(2, "Game", "level created, starting");
    level->play();
    errors->log(2, "Game", "level done");
  }

  delete(level);
}

void gsMain::do_network_level()
{
  nwlevel = new gsNetLevel();

  if ( nwlevel->init_stuff() )
  {
    errors->log(0, "FATAL ERROR", nwlevel->get_error_str());
  }
  else
  {
    errors->log(2, "Game", "network level created, starting");
    nwlevel->play();
    errors->log(2, "Game", "network level done");
  }

  delete(nwlevel);
}

void gsMain::play()
{
  if (mainloop_verbose) {
    printf("Entering main play loop...\n");
  }
  
  int mode = MENU_MAIN;

  while (mode)
  {
    if (mainloop_verbose) {
      printf("Main loop iteration: mode=%d\n", mode);
    }
    
    if (mainloop_verbose) {
      printf("Creating new menu object...\n");
    }
    
    menu = new gsMenu();
  
    if (mainloop_verbose) {
      printf("Initializing menu with mode: %d\n", mode);
    }
    
    menu->init(mode);
    
    if (mainloop_verbose) {
      printf("Menu initialized successfully, calling do_it()...\n");
    }
    
    mode = menu->do_it();
    
    if (mainloop_verbose) {
      printf("Menu returned mode: %d\n", mode);
    }
  
    delete(menu);
    
    if (mainloop_verbose && mode == 0) {
      printf("Menu returned 0, exiting main loop\n");
    }

    if (mode == MENU_1PSTATISTICS) 
    {
      if (mainloop_verbose) {
        printf("Starting single player level...\n");
      }
      
      do 
      {
        if (mainloop_verbose) {
          printf("About to call do_sp_level() iteration\n");
        }
        do_sp_level();
        if (mainloop_verbose) {
          printf("do_sp_level() completed, checking level_restart: %d\n", globals->level_restart);
        }
      }
      while(globals->level_restart);
    }  

    if (mode == MENU_2PSTATISTICS) 
    {
      if (mainloop_verbose) {
        printf("Starting two player level...\n");
      }
      
      do 
      {
        if (mainloop_verbose) {
          printf("About to call do_level() iteration\n");
        }
        do_level();
        if (mainloop_verbose) {
          printf("do_level() completed, checking level_restart: %d\n", globals->level_restart);
        }
      }
      while(globals->level_restart);
    }  
    
    if (mode == MENU_NETSTATISTICS) {
      if (mainloop_verbose) {
        printf("Starting network level...\n");
      }
      
      if (mainloop_verbose) {
        printf("About to call do_network_level()\n");
      }
      do_network_level();
      if (mainloop_verbose) {
        printf("do_network_level() completed\n");
      }
    }
    
    if (mainloop_verbose) {
      printf("Main loop iteration completed, mode is now: %d\n", mode);
    }
  }
  
  if (mainloop_verbose) {
    printf("Exiting main play loop...\n");
  }
}


