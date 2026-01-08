/*
 *   GRAVITY STRIKE -- menu/gsmenu.cpp
 *
 *
 *   (c) Copyright 2001/2002 by Jens Hassler <jh@gravity-strike.de>
 *       www.gravity-strike.de
 *
 *   see source.txt for details
 *
 */

#include <time.h>
#include "allegro5_wrapper.h"
#include "gsmenu.h"
#include "mstars.h"
#include "mpixels.h"
#include "mplayer.h"
#include "mbullet.h"
#include "../gsglob.h"
#include "../gserror.h"
#include "../gsclear.h"
#include "../gshelper.h"
#include "../gscontrl.h"
#include "../gsfade.h"
#include "../gssound.h"
#include "../gsnetw.h"
#include "gsmd.h"
#include "mmain.h"
#include "moptions.h"
#include "mctrl.h"
#include "mctrl2.h"
#include "mctrlsp.h"
#include "mchship.h"
#include "mshpbld.h"
#include "moneplay.h"
#include "mtwoplay.h"
#include "mminig.h"
#include "m1pstats.h"
#include "m2pstats.h"
#include "mnwstats.h"
#include "mnetgini.h"
#include "mnetgame.h"
#include "../headless_screen.h"

gsMenu     *menu;

mPixelRain *mpixels = NULL;
mBullet    *mbullets = NULL;
mPlayer    *mplayer[2];

// External declarations
extern int mainloop_verbose;
extern int extra_verbose2;
extern char* headless_output_path;

gsControl  *controls = NULL;

gsMenu::gsMenu()
{
  mouse_x = 1;//320;
  mouse_y = 1;//240;
  last_second = actual_time = 0;
  endthis = esc_pressed = update_graphics = FALSE;
  current_menu = MENU_MAIN;
  exit_mode = 0;
  crosshair_count = 0;

  menu = this;
}

gsMenu::~gsMenu()
{
  exit_menu();
}

int gsMenu::init(int start_menu)
{
  text_mode(-1);
  strcpy(error_str, "");

  globals->rand_ini = time(NULL);
  srand(globals->rand_ini);

  // switch to menu resolution (if needed)
  globals->switch_to_menu_resolution();

  errors->log(2, "menu init", "keyboard config");
  controls = new gsControl();
  controls->load_key_cfg();

  // color stuff
  errors->log(2, "menu init", "player colors / sprites");
  globals->mainpal = (PALETTE *)globals->menudat[menupal].dat;
  set_palette((RGB *)globals->mainpal);
  globals->color[0]  = makecol8(255,255,255);        // white
  globals->color[1]  = makecol8(95,255,95);          // light green
  globals->color[2]  = makecol8(255,255,0);          // yellow
  globals->color[3]  = makecol8(255,100,100);        // light red
  globals->color[4]  = makecol8(100,100,255);        // light blue
  globals->color[5]  = makecol8(255,255,255);        // white
  globals->color[6]  = makecol8(255,255,0);          // yellow
  globals->color[7]  = makecol8(255,100,100);        // light red
  globals->color[8]  = makecol8(100,255,100);        // light green
  globals->color[9]  = makecol8(100,100,255);        // light blue

  globals->col_red       = makecol8(255,0,0);
  globals->col_bred      = makecol8(255,255,0);
  globals->col_green     = makecol8(0,255,0);
  globals->col_blue      = makecol8(0,0,255);
  globals->col_black     = makecol8(0,0,0);
  globals->col_white     = makecol8(255,255,255);
  globals->col_yellow    = makecol8(255,255,0);
  globals->col_orange    = makecol8(255,128,0);
  globals->col_lblue     = makecol8(20,20,200);
  globals->col_grey      = makecol8(147,147,147);
  globals->col_dgrey     = makecol8(47,47,47);
  globals->make_color_tables();

  errors->log(2, "menu init", "vscreen");
  globals->vscreen = create_bitmap(800, 600);

  if (!globals->vscreen)
  {
    strcpy(error_str, "Can't initialize virtual screen!");
    return 1;
  }

  clear(globals->vscreen);
  clear(screen);

  // init antialias routines
  errors->log(2, "menu init", "antialiased text");
  antialias_init((RGB*)globals->menudat[menupal].dat);

  errors->log(2, "menu init", "pixel effects");
  mpixels = new mPixelRain();

  errors->log(2, "menu init", "bullet effects");
  mbullets = new mBullet();

  errors->log(2, "menu init", "100 background stars");
  stars = new mStars();
  stars->add(100);

  errors->log(2, "menu init", "2 menu players");
  mplayer[0] = new mPlayer(0, 0, -50, 240);
  mplayer[1] = new mPlayer(1, 1, 680, 240);

//  if (gsound->get_play_music())
  {
    errors->log(2, "menu init", "starting music...");
    gsound->load_menu_music();
    gsound->set_music_volume(0);
    gsound->start_menu_music();
  }

  errors->log(2, "menu init", "menus");
  menu_main    = NULL;
  menu_options = NULL;
  menu_controls = NULL;
  menu_2pcontrols = NULL;
  menu_spcontrols = NULL;
  menu_chooseship = NULL;
  menu_shipbuilder = NULL;
  menu_oneplayer = NULL;
  menu_twoplayer = NULL;
  menu_minigame = NULL;
  menu_netgameini = NULL;
  menu_netgame = NULL;
  menu_1pstats = NULL;
  menu_2pstats = NULL;
  menu_netstats = NULL;

  switch (start_menu)
  {
    case MENU_SINGLEPLAYER :
         menu_oneplayer = new mOnePlayer(); current_menu = MENU_SINGLEPLAYER;
         menu_oneplayer->init();
         break;

    case MENU_MINIGAME :
         menu_minigame = new mMiniGame(); current_menu = MENU_MINIGAME;
         menu_minigame->init();
         break;

    case MENU_SPLITSCREEN :
         menu_twoplayer = new mTwoPlayer(); current_menu = MENU_SPLITSCREEN;
         menu_twoplayer->init();
         break;

    case MENU_NETGAMEINI :
         menu_netgameini = new mNetGameInit(); current_menu = MENU_NETGAMEINI;
         menu_netgameini->init();
         break;

    case MENU_NETGAME :
         menu_netgame = new mNetGame(); current_menu = MENU_NETGAME;
         menu_netgame->init();
         break;

    case MENU_1PSTATISTICS :
         menu_1pstats = new m1PStats(); current_menu = MENU_1PSTATISTICS;
         menu_1pstats->init();
         break;

    case MENU_2PSTATISTICS :
         menu_2pstats = new m2PStats(); current_menu = MENU_2PSTATISTICS;
         menu_2pstats->init();
         break;

    case MENU_NETSTATISTICS :
         menu_netstats = new mNetStats(); current_menu = MENU_NETSTATISTICS;
         menu_netstats->init();
         break;

    default :
         menu_main = new mMain(); current_menu = MENU_MAIN;
         menu_main->init();
         break;
  }

  set_palette(black_palette);
  errors->log(2, "menu init", "fading");
  fade = new gsFade();

  fade->initiate(FADE_TO_MAIN, 1, 60);

  return 0;
}

void gsMenu::exit_menu()
{
  // save options & keys
errors->log(2, "menu", "saving key config");
  controls->save_key_cfg();
errors->log(2, "menu", "saving config file");
  globals->save_config_file();


errors->log(2, "menu", "destroying music");
  gsound->stop_music();
  gsound->destroy_menu_music();

errors->log(2, "menu", "destroying bitmaps");
  if (globals->vscreen) destroy_bitmap(globals->vscreen); globals->vscreen = NULL;

errors->log(2, "menu", "destroying menus");
  if (mpixels)          delete(mpixels);      mpixels = NULL;
  if (stars)            delete(stars);        stars = NULL;
  if (controls)         delete(controls);     controls = NULL;
  if (mplayer[0])       delete(mplayer[0]);   mplayer[0] = NULL;
  if (mplayer[1])       delete(mplayer[1]);   mplayer[1] = NULL;
  if (fade)             delete(fade);         fade = NULL;
  if (mbullets)         delete(mbullets);     mbullets = NULL;
  if (menu_main)        delete(menu_main);    menu_main = NULL;
  if (menu_options)     delete(menu_options); menu_options = NULL;
  if (menu_controls)    delete(menu_controls); menu_controls = NULL;
  if (menu_2pcontrols)  delete(menu_2pcontrols); menu_2pcontrols = NULL;
  if (menu_spcontrols)  delete(menu_spcontrols); menu_spcontrols = NULL;
  if (menu_chooseship)  delete(menu_chooseship); menu_chooseship = NULL;
  if (menu_shipbuilder) delete(menu_shipbuilder); menu_shipbuilder = NULL;
  if (menu_oneplayer)   delete(menu_oneplayer); menu_oneplayer = NULL;
  if (menu_twoplayer)   delete(menu_twoplayer); menu_twoplayer = NULL;
  if (menu_minigame)    delete(menu_minigame); menu_minigame = NULL;
  if (menu_netgameini)  delete(menu_netgameini); menu_netgameini = NULL;
  if (menu_netgame)     delete(menu_netgame); menu_netgame = NULL;
  if (menu_1pstats)     delete(menu_1pstats); menu_1pstats = NULL;
  if (menu_2pstats)     delete(menu_2pstats); menu_2pstats = NULL;
  if (menu_netstats)    delete(menu_netstats); menu_netstats = NULL;

errors->log(2, "menu", "clearing clear list");
  clearlist->clear_list();
  antialias_exit();
}

void gsMenu::do_logic(int fs)
{
  rest(0);

  if (last_second != globals->second_time)
  {
    mplayer[0]->move_to(50+fxrand()%20, mouse_y, 87 + fxrand()%7);
    mplayer[1]->move_to(560-fxrand()%20, mouse_y, 180 + 87 + fxrand()%7);
    last_second = globals->second_time;
  }
 
  if (mplayer[0]->get_head() > 60 && mplayer[0]->get_head() < 110)
    if (fxrand() % 100 < 15) mplayer[0]->shoot();
  if (mplayer[1]->get_head() > 240 && mplayer[1]->get_head() < 300)
    if (fxrand() % 100 < 15) mplayer[1]->shoot();
 
  if (fxrand() % 200 < 50)
  {
    mpixels->add(40, fxrand()%250 + 310, fxrand()%40 + 30, 0, 0, -1-(fxrand() % 4), 2-(fxrand() % 4), 50, globals->tbl_blue, 10, 1000, 10);
//        mpixels->add(20, fxrand()%220 + 80, fxrand()%30 + 35, 0, 0, -1-(fxrand() % 4), 2-(fxrand() % 4), 50, mpixels->tbl_green, 10, 10);
  }
 
  // do menu logic
  stars->update();
  mpixels->move(globals->particle_gravity);
  mbullets->move();
  mplayer[0]->move();
  mplayer[1]->move();
  fade->update();

  update_mouse_crosshair();

  if (!fs)
  switch (current_menu)
  {
    case MENU_MAIN :
         switch (menu_main->check())
         {
           case MENU_QUIT :
                if (!esc_pressed)
                {
                  esc_pressed = TRUE;
                  fade->initiate(FADE_TO_BLACK, 1, 50);
                }
                break;
           case MENU_SINGLEPLAYER :
                current_menu = MENU_SINGLEPLAYER;
                delete(menu_main); menu_main = NULL;
                menu_oneplayer = new mOnePlayer();
                menu_oneplayer->init(1);
                break;
           case MENU_SPLITSCREEN :
                current_menu = MENU_SPLITSCREEN;
                delete(menu_main); menu_main = NULL;
                menu_twoplayer = new mTwoPlayer();
                menu_twoplayer->init(1);
                break;
           case MENU_NETGAMEINI :
                current_menu = MENU_NETGAMEINI;
                delete(menu_main); menu_main = NULL;
                menu_netgameini = new mNetGameInit();
                menu_netgameini->init(1);
                break;
           case MENU_MINIGAME :
                current_menu = MENU_MINIGAME;
                delete(menu_main); menu_main = NULL;
                menu_minigame = new mMiniGame();
                menu_minigame->init(1);
                break;
           case MENU_OPTIONS :
                current_menu = MENU_OPTIONS;
                delete(menu_main); menu_main = NULL;
                menu_options = new mOptions();
                menu_options->init(1);
                break;
           case MENU_CHOOSESHIP :
                current_menu = MENU_CHOOSESHIP;
                delete(menu_main); menu_main = NULL;
                menu_chooseship = new mChooseShip();
                menu_chooseship->init(1);
                break;
         }
         break;

    case MENU_SINGLEPLAYER :
         switch (menu_oneplayer->check())
         {
           case MENU_MAIN :
                current_menu = MENU_MAIN;
                delete(menu_oneplayer); menu_oneplayer = NULL;
                menu_main = new mMain();
                menu_main->init(1);
                break;
           case MENU_CHOOSESHIP :
                current_menu = MENU_CHOOSESHIP;
                delete(menu_oneplayer); menu_oneplayer = NULL;
                menu_chooseship = new mChooseShip();
                menu_chooseship->init(1);
                break;
           case MENU_SHIPBUILDER :
                current_menu = MENU_SHIPBUILDER;
                char tmp[20];
                strncpy(tmp, globals->pplayer[globals->ship_num].pship, 20);
                delete(menu_oneplayer); menu_oneplayer = NULL;
                menu_shipbuilder = new mShipBuilder();
                menu_shipbuilder->init(1, tmp, MENU_SINGLEPLAYER);
                break;
           case START_GAME :
                exit_mode = MENU_1PSTATISTICS;
                endthis = TRUE;
                break;
         }
         break;

    case MENU_SPLITSCREEN :
         switch (menu_twoplayer->check())
         {
           case MENU_MAIN :
                current_menu = MENU_MAIN;
                delete(menu_twoplayer); menu_twoplayer = NULL;
                menu_main = new mMain();
                menu_main->init(1);
                break;
           case MENU_CHOOSESHIP :
                current_menu = MENU_CHOOSESHIP;
                delete(menu_twoplayer); menu_twoplayer = NULL;
                menu_chooseship = new mChooseShip();
                menu_chooseship->init(1);
                break;
           case MENU_SHIPBUILDER :
                current_menu = MENU_SHIPBUILDER;
                char tmp[20];
                strncpy(tmp, globals->pplayer[globals->ship_num].pship, 20);
                delete(menu_twoplayer); menu_twoplayer = NULL;
                menu_shipbuilder = new mShipBuilder();
                menu_shipbuilder->init(1, tmp, MENU_SPLITSCREEN);
                break;
           case START_GAME :
                exit_mode = MENU_2PSTATISTICS;
                endthis = TRUE;
                break;
         }
         break;

    case MENU_MINIGAME :
         switch (menu_minigame->check())
         {
           case MENU_MAIN :
                current_menu = MENU_MAIN;
                delete(menu_minigame); menu_minigame = NULL;
                menu_main = new mMain();
                menu_main->init(1);
                break;
           case MENU_CHOOSESHIP :
                current_menu = MENU_CHOOSESHIP;
                delete(menu_minigame); menu_minigame = NULL;
                menu_chooseship = new mChooseShip();
                menu_chooseship->init(1);
                break;
           case MENU_SHIPBUILDER :
                current_menu = MENU_SHIPBUILDER;
                char tmp[20];
                strncpy(tmp, globals->pplayer[globals->ship_num].pship, 20);
                delete(menu_minigame); menu_minigame = NULL;
                menu_shipbuilder = new mShipBuilder();
                menu_shipbuilder->init(1, tmp, MENU_MINIGAME);
                break;
           case START_GAME :
                if (globals->players == 1) 
                  exit_mode = MENU_1PSTATISTICS;
                else
                  exit_mode = MENU_2PSTATISTICS;
                
                endthis = TRUE;
                break;
         }
         break;

    case MENU_NETGAMEINI :
         switch (menu_netgameini->check())
         {
           case MENU_MAIN :
                current_menu = MENU_MAIN;
                delete(menu_netgameini); menu_netgameini = NULL;
                menu_main = new mMain();
                menu_main->init(1);
                break;
           case MENU_NETGAME :
                current_menu = MENU_NETGAME;
                delete(menu_netgameini); menu_netgameini = NULL;
                menu_netgame = new mNetGame();
                menu_netgame->init(1);
                break;
         }
         break;

    case MENU_NETGAME :
         switch (menu_netgame->check())
         {
           case MENU_MAIN :
                current_menu = MENU_MAIN;
                delete(menu_netgame); menu_netgame = NULL;
                menu_main = new mMain();
                menu_main->init(1);
                break;
           case MENU_CHOOSESHIP :
                current_menu = MENU_CHOOSESHIP;
                delete(menu_netgame); menu_netgame = NULL;
                menu_chooseship = new mChooseShip();
                menu_chooseship->init(1);
                break;
           case MENU_SHIPBUILDER :
                current_menu = MENU_SHIPBUILDER;
                char tmp[20];
                strncpy(tmp, globals->pplayer[globals->ship_num].pship, 20);
                delete(menu_netgame); menu_netgame = NULL;
                menu_shipbuilder = new mShipBuilder();
                menu_shipbuilder->init(1, tmp, MENU_NETGAME);
                break;
           case START_NETGAME :
                exit_mode = MENU_NETSTATISTICS;
                endthis = TRUE;
                break;
         }
         break;

    case MENU_OPTIONS :
         switch (menu_options->check())
         {
           case MENU_MAIN :
                current_menu = MENU_MAIN;
                delete(menu_options); menu_options = NULL;
                menu_main = new mMain();
                menu_main->init(1);
                break;
           case MENU_CONTROLS :
                current_menu = MENU_CONTROLS;
                delete(menu_options); menu_options = NULL;
                menu_controls = new mPControls();
                menu_controls->init(1);
                break;
           case MENU_2PCONTROLS :
                current_menu = MENU_2PCONTROLS;
                delete(menu_options); menu_options = NULL;
                menu_2pcontrols = new mPControls2P();
                menu_2pcontrols->init(1);
                break;
           case MENU_SPCONTROLS :
                current_menu = MENU_SPCONTROLS;
                delete(menu_options); menu_options = NULL;
                menu_spcontrols = new mPControlsSP();
                menu_spcontrols->init(1);
                break;
         }
         break;

    case MENU_CONTROLS :
         switch (menu_controls->check())
         {
           case MENU_OPTIONS :
                current_menu = MENU_OPTIONS;
                delete(menu_controls); menu_controls = NULL;
                menu_options = new mOptions();
                menu_options->init(1);
                break;
           case MENU_2PCONTROLS :
                current_menu = MENU_2PCONTROLS;
                delete(menu_controls); menu_controls = NULL;
                menu_2pcontrols = new mPControls2P();
                menu_2pcontrols->init(1);
                break;
         }
         break;

    case MENU_2PCONTROLS :
         switch (menu_2pcontrols->check())
         {
           case MENU_OPTIONS :
                current_menu = MENU_OPTIONS;
                delete(menu_2pcontrols); menu_2pcontrols = NULL;
                menu_options = new mOptions();
                menu_options->init(1);
                break;
           case MENU_CONTROLS :
                current_menu = MENU_CONTROLS;
                delete(menu_2pcontrols); menu_2pcontrols = NULL;
                menu_controls = new mPControls();
                menu_controls->init(1);
                break;
         }
         break;

    case MENU_SPCONTROLS :
         switch (menu_spcontrols->check())
         {
           case MENU_OPTIONS :
                current_menu = MENU_OPTIONS;
                delete(menu_spcontrols); menu_spcontrols = NULL;
                menu_options = new mOptions();
                menu_options->init(1);
                break;
           case MENU_CONTROLS :
                current_menu = MENU_CONTROLS;
                delete(menu_spcontrols); menu_spcontrols = NULL;
                menu_controls = new mPControls();
                menu_controls->init(1);
                break;
         }
         break;

    case MENU_CHOOSESHIP :
         switch (menu_chooseship->check())
         {
           case MENU_MAIN :
                current_menu = MENU_MAIN;
                delete(menu_chooseship); menu_chooseship = NULL;
                menu_main = new mMain();
                menu_main->init(1);
                break;
           case MENU_SHIPBUILDER :
                current_menu = MENU_SHIPBUILDER;
                char tmp[30]; strncpy(tmp, menu_chooseship->get_chosen_ship(), 30);
                delete(menu_chooseship); menu_chooseship = NULL;
                menu_shipbuilder = new mShipBuilder();
                menu_shipbuilder->init(1, tmp);
                break;
           case MENU_SINGLEPLAYER :
                current_menu = MENU_SINGLEPLAYER;
                if (globals->ship_num == 0) strcpy(globals->pplayer[0].pship, menu_chooseship->get_chosen_ship());
                delete(menu_chooseship); menu_chooseship = NULL;
                menu_oneplayer = new mOnePlayer();
                menu_oneplayer->init(1);
                break;
           case MENU_SPLITSCREEN :
                current_menu = MENU_SPLITSCREEN;
                if (globals->ship_num == 0) strcpy(globals->pplayer[0].pship, menu_chooseship->get_chosen_ship());
                else if (globals->ship_num == 1) strcpy(globals->pplayer[1].pship, menu_chooseship->get_chosen_ship());
                delete(menu_chooseship); menu_chooseship = NULL;
                menu_twoplayer = new mTwoPlayer();
                menu_twoplayer->init(1);
                break;
           case MENU_NETGAME :
                current_menu = MENU_NETGAME;
                if (globals->ship_num == 0) strcpy(globals->pplayer[0].pship, menu_chooseship->get_chosen_ship());
                delete(menu_chooseship); menu_chooseship = NULL;
                menu_netgame = new mNetGame(1);
                menu_netgame->init(1, 1);
                break;
           case MENU_MINIGAME :
                current_menu = MENU_MINIGAME;
                if (globals->ship_num == 0) strcpy(globals->pplayer[0].pship, menu_chooseship->get_chosen_ship());
                else if (globals->ship_num == 1) strcpy(globals->pplayer[1].pship, menu_chooseship->get_chosen_ship());
                delete(menu_chooseship); menu_chooseship = NULL;
                menu_minigame = new mMiniGame();
                menu_minigame->init(1);
                break;
         }
         break;

    case MENU_SHIPBUILDER :
         switch (menu_shipbuilder->check())
         {
           case MENU_CHOOSESHIP :
                current_menu = MENU_CHOOSESHIP;
                delete(menu_shipbuilder); menu_shipbuilder = NULL;
                menu_chooseship = new mChooseShip();
                menu_chooseship->init(1);
                break;
           case MENU_SINGLEPLAYER :
                current_menu = MENU_SINGLEPLAYER;
                delete(menu_shipbuilder); menu_shipbuilder = NULL;
                menu_oneplayer = new mOnePlayer();
                menu_oneplayer->init(1);
                break;
           case MENU_SPLITSCREEN :
                current_menu = MENU_SPLITSCREEN;
                delete(menu_shipbuilder); menu_shipbuilder = NULL;
                menu_twoplayer = new mTwoPlayer();
                menu_twoplayer->init(1);
                break;
           case MENU_NETGAME :
                current_menu = MENU_NETGAME;
                delete(menu_shipbuilder); menu_shipbuilder = NULL;
                menu_netgame = new mNetGame(1);
                menu_netgame->init(1, 1);
                break;
           case MENU_MINIGAME :
                current_menu = MENU_MINIGAME;
                delete(menu_shipbuilder); menu_shipbuilder = NULL;
                menu_minigame = new mMiniGame();
                menu_minigame->init(1);
                break;
         }
         break;

    case MENU_1PSTATISTICS :
         switch (menu_1pstats->check())
         {
           case MENU_SINGLEPLAYER :
                current_menu = MENU_SINGLEPLAYER;
                delete(menu_1pstats); menu_1pstats = NULL;
                menu_oneplayer = new mOnePlayer();
                menu_oneplayer->init(1);
                break;
                
           case MENU_MINIGAME :
                current_menu = MENU_MINIGAME;
                delete(menu_1pstats); menu_1pstats = NULL;
                menu_minigame = new mMiniGame();
                menu_minigame->init(1);
                break;
         }
         break;

    case MENU_2PSTATISTICS :
         switch (menu_2pstats->check())
         {
           case MENU_SPLITSCREEN :
                current_menu = MENU_SPLITSCREEN;
                delete(menu_2pstats); menu_2pstats = NULL;
                menu_twoplayer = new mTwoPlayer();
                menu_twoplayer->init(1);
                break;
                
           case MENU_MINIGAME :
                current_menu = MENU_MINIGAME;
                delete(menu_2pstats); menu_2pstats = NULL;
                menu_minigame = new mMiniGame();
                menu_minigame->init(1);
                break;
         }
         break;

    case MENU_NETSTATISTICS :
         switch (menu_netstats->check())
         {
           case MENU_NETGAME :
                current_menu = MENU_NETGAME;
                delete(menu_netstats); menu_netstats = NULL;
                menu_netgame = new mNetGame();
                menu_netgame->init(1);
                break;
           case MENU_NETGAMEINI :
                current_menu = MENU_NETGAMEINI;
                delete(menu_netstats); menu_netstats = NULL;
                menu_netgameini = new mNetGameInit();
                menu_netgameini->init(1);
                break;
         }
         break;

  } // switch current_menu


}

void gsMenu::do_graphics(int fs)
{
  if (mainloop_verbose) {
    printf("Rendering frame - starting graphics update...\n");
  }

  // Record the start of rendering operations
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->push_render_scope("render_stars");
  }
  
  // stars
  if (mainloop_verbose) {
    printf("Rendering stars...\n");
  }
  stars->draw(globals->vscreen);
  
  // Record this operation
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->record_call("stars->draw", "sprite", globals->vscreen, 0, 0, 
                    get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                    HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen), 
                                                         get_bitmap_height(globals->vscreen)));
    hs->pop_render_scope(); // End stars render scope
  }

  // Record the start of player rendering
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->push_render_scope("render_players");
  }
  
  // players
  if (mainloop_verbose) {
    printf("Rendering players...\n");
  }
  mplayer[0]->draw(globals->vscreen);
  mplayer[1]->draw(globals->vscreen);

  // Record these operations
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->record_call("mplayer[0]->draw", "sprite", globals->vscreen, 0, 0,
                    get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                    HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                         get_bitmap_height(globals->vscreen)));
    hs->record_call("mplayer[1]->draw", "sprite", globals->vscreen, 0, 0,
                    get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                    HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                         get_bitmap_height(globals->vscreen)));
    hs->pop_render_scope(); // End players render scope
  }

  // Record the start of title rendering
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->push_render_scope("render_title");
  }
  
  // title
  if (mainloop_verbose) {
    RLE_SPRITE *titlebit = (RLE_SPRITE *)globals->menudat[title].dat;
    printf("Rendering title at position (x: %d, y: 30) with dimensions (w: %d, h: %d)\n", 
           320-(titlebit->w/2), titlebit->w, titlebit->h);
  }
  RLE_SPRITE *titlebit = (RLE_SPRITE *)globals->menudat[title].dat;
  int title_x = 320-(titlebit->w/2);
  draw_rle_sprite(globals->vscreen, titlebit, title_x, 30);

  // Record this operation
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->record_call("draw_rle_sprite", "title", titlebit, title_x, 30, titlebit->w, titlebit->h,
                    HeadlessScreen::calculate_bitmap_size(titlebit->w, titlebit->h));
    hs->pop_render_scope(); // End title render scope
  }

  // Record the start of menu rendering
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->push_render_scope("render_menu_elements");
  }
  
  // menu buttons
  if (mainloop_verbose) {
    printf("Rendering menu elements for current menu type: %d\n", current_menu);
  }
  switch (current_menu)
  {
    case MENU_MAIN      : 
      if (mainloop_verbose) printf("Drawing main menu...\n"); 
      menu_main->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_main->draw", "menu", menu_main, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_OPTIONS   : 
      if (mainloop_verbose) printf("Drawing options menu...\n"); 
      menu_options->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_options->draw", "menu", menu_options, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_CONTROLS  : 
      if (mainloop_verbose) printf("Drawing controls menu...\n"); 
      menu_controls->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_controls->draw", "menu", menu_controls, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_2PCONTROLS: 
      if (mainloop_verbose) printf("Drawing 2P controls menu...\n"); 
      menu_2pcontrols->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_2pcontrols->draw", "menu", menu_2pcontrols, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_SPCONTROLS: 
      if (mainloop_verbose) printf("Drawing SP controls menu...\n"); 
      menu_spcontrols->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_spcontrols->draw", "menu", menu_spcontrols, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_CHOOSESHIP: 
      if (mainloop_verbose) printf("Drawing choose ship menu...\n"); 
      menu_chooseship->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_chooseship->draw", "menu", menu_chooseship, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_SHIPBUILDER: 
      if (mainloop_verbose) printf("Drawing ship builder menu...\n"); 
      menu_shipbuilder->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_shipbuilder->draw", "menu", menu_shipbuilder, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_SINGLEPLAYER: 
      if (mainloop_verbose) printf("Drawing 1P menu...\n"); 
      menu_oneplayer->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_oneplayer->draw", "menu", menu_oneplayer, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_SPLITSCREEN: 
      if (mainloop_verbose) printf("Drawing 2P menu...\n"); 
      menu_twoplayer->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_twoplayer->draw", "menu", menu_twoplayer, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_MINIGAME  : 
      if (mainloop_verbose) printf("Drawing minigame menu...\n"); 
      menu_minigame->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_minigame->draw", "menu", menu_minigame, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_NETGAMEINI: 
      if (mainloop_verbose) printf("Drawing net game init menu...\n"); 
      menu_netgameini->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_netgameini->draw", "menu", menu_netgameini, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_NETGAME   : 
      if (mainloop_verbose) printf("Drawing net game menu...\n"); 
      menu_netgame->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_netgame->draw", "menu", menu_netgame, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_1PSTATISTICS: 
      if (mainloop_verbose) printf("Drawing 1P stats menu...\n"); 
      menu_1pstats->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_1pstats->draw", "menu", menu_1pstats, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_2PSTATISTICS: 
      if (mainloop_verbose) printf("Drawing 2P stats menu...\n"); 
      menu_2pstats->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_2pstats->draw", "menu", menu_2pstats, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
    case MENU_NETSTATISTICS: 
      if (mainloop_verbose) printf("Drawing net stats menu...\n"); 
      menu_netstats->draw(globals->vscreen); 
      if (headless_output_path) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        hs->record_call("menu_netstats->draw", "menu", menu_netstats, 0, 0,
                        get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                        HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                             get_bitmap_height(globals->vscreen)));
      }
      break;
  }
  
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->pop_render_scope(); // End menu elements render scope
  }

  // Record the start of collision checks
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->push_render_scope("collision_checks");
  }
  
  // coll checks
  if (mainloop_verbose) {
    printf("Checking collisions...\n");
  }
  mpixels->check_collisions(globals->vscreen);
  mbullets->check_collisions(globals->vscreen);

  // Record these operations
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->record_call("mpixels->check_collisions", "collision", mpixels, 0, 0,
                    get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                    HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                         get_bitmap_height(globals->vscreen)));
    hs->record_call("mbullets->check_collisions", "collision", mbullets, 0, 0,
                    get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                    HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                         get_bitmap_height(globals->vscreen)));
    hs->pop_render_scope(); // End collision checks scope
  }

  // Record the start of pixel/bullet rendering
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->push_render_scope("render_pixels_bullets");
  }
  
  // pixbulls
  if (mainloop_verbose) {
    printf("Rendering pixels and bullets...\n");
  }
  mpixels->draw(globals->vscreen);
  mbullets->draw(globals->vscreen);

  // Record these operations
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->record_call("mpixels->draw", "particles", mpixels, 0, 0,
                    get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                    HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                         get_bitmap_height(globals->vscreen)));
    hs->record_call("mbullets->draw", "projectile", mbullets, 0, 0,
                    get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                    HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                         get_bitmap_height(globals->vscreen)));
    hs->pop_render_scope(); // End pixels/bullets render scope
  }

  // Record the start of crosshair rendering
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->push_render_scope("render_crosshair");
  }
  
  // crosshair (mouse)
  if (mainloop_verbose) {
    BITMAP *crosshair_bitmap = (BITMAP*)globals->menudat[crosshair+crosshair_count].dat;
    printf("Rendering crosshair at position (x: %d, y: %d) with dimensions (w: %d, h: %d)\n", 
           mouse_x-4, mouse_y-4, crosshair_bitmap->w, crosshair_bitmap->h);
  }
  BITMAP *crosshair_bitmap = (BITMAP*)globals->menudat[crosshair+crosshair_count].dat;
  int crosshair_x = mouse_x-4, crosshair_y = mouse_y-4;
  draw_sprite(globals->vscreen, crosshair_bitmap, crosshair_x, crosshair_y);
  clearlist->add(globals->vscreen, crosshair_x, crosshair_y, 8, 8);

  // Record this operation
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->record_call("draw_sprite", "crosshair", crosshair_bitmap, crosshair_x, crosshair_y,
                    crosshair_bitmap->w, crosshair_bitmap->h,
                    HeadlessScreen::calculate_bitmap_size(crosshair_bitmap->w, crosshair_bitmap->h));
    hs->pop_render_scope(); // End crosshair render scope
  }

  // Record the start of player stats rendering
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->push_render_scope("render_player_stats");
  }
  
  // player frags
  if (mainloop_verbose) {
    printf("Rendering player stats...\n");
  }
  show_player_stats();

  // Record this operation
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->record_call("show_player_stats", "text", nullptr, 0, 0,
                    get_bitmap_width(globals->vscreen), get_bitmap_height(globals->vscreen),
                    HeadlessScreen::calculate_bitmap_size(get_bitmap_width(globals->vscreen),
                                                         get_bitmap_height(globals->vscreen)));
    hs->pop_render_scope(); // End player stats render scope
  }

  if (key[KEY_F12]) make_screenshot();

  // Record the screen blit operation
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->push_render_scope("screen_blit");
  }
  
  // flip to visible screen
  acquire_bitmap(screen);
  if (!fs) blit(globals->vscreen, screen, 0, 0, 0, 0, 640, 480);
  release_bitmap(screen);

  // Record this operation
  if (headless_output_path) {
    HeadlessScreen* hs = HeadlessScreen::get_instance();
    hs->record_call("blit", "screen_flip", nullptr, 0, 0, 640, 480,
                    HeadlessScreen::calculate_bitmap_size(640, 480));
    hs->pop_render_scope(); // End screen blit scope
  }

  // clear stuff on virtual screen
  if (!fs) clearlist->clear_screen();
}

int gsMenu::do_it()
{
  globals->game_time = globals->overall_game_time = globals->second_time = 0;
  int iteration_counter = 0;

  while (!(endthis))
  {
    // Start a new iteration for headless screen
    if (headless_output_path) {
      HeadlessScreen* hs = HeadlessScreen::get_instance();
      hs->start_iteration(iteration_counter++);
    }

    while (globals->game_time > actual_time)
    {
      // game logic
      actual_time++;

      if (esc_pressed)
        { if (gsound->get_music_volume() > 0) gsound->set_music_volume(gsound->get_music_volume()-3); }
      else
      if (!fade->get_fade_active())
        { if (gsound->get_music_volume() < globals->musicvol*1.5) gsound->set_music_volume(gsound->get_music_volume()+2); }

      if (key[KEY_ESC] && key_shifts & KB_SHIFT_FLAG && !esc_pressed)
      {
        fade->initiate(FADE_TO_BLACK, 1, 50);
        esc_pressed = TRUE;
        clear_keybuf();
      }

      if (esc_pressed && !fade->get_fade_active())
      {
        endthis = TRUE;
      }

      // do menu logic
      do_logic();

      update_graphics = TRUE;
    } // while game_time > actual_time

    if (update_graphics)
    {
      // draw menu graphics
      do_graphics();

      if (key[KEY_F12]) make_screenshot();

      update_graphics = FALSE;
      globals->fps++;
    } // if update_graphics

    // End the iteration for headless screen
    if (headless_output_path) {
      HeadlessScreen* hs = HeadlessScreen::get_instance();
      hs->end_iteration();
      
      // If extra_verbose2 is enabled, dump the iteration to stderr
      if (extra_verbose2) {
        HeadlessScreen* hs = HeadlessScreen::get_instance();
        std::string json_output = hs->to_json();
        fprintf(stderr, "HeadlessScreen iteration data:\n%s\n", json_output.c_str());
      }
    }
  } // while !endthis

  return exit_mode;
}

void gsMenu::show_player_stats()
{
  char text1[60];
  int tlen;
  int theight;
  FONT *f = (FONT *)globals->fontdat[FONT_LCD].dat;
  theight = text_height(f);
  text_mode(-1);

  if (mainloop_verbose) {
    printf("Rendering font at LCD (index %d) with height %d pixels\n", FONT_LCD, theight);
  }

  sprintf(text1, "%d", mplayer[0]->get_frags());
  tlen = text_length(f, text1);
  
  if (mainloop_verbose) {
    printf("Rendering text '%s' at position (x: 10, y: 450) with dimensions (w: %d, h: %d)\n", 
           text1, tlen, theight);
  }
  
  aatextout(globals->vscreen, f, text1, 10, 450, globals->col_grey);
  clearlist->add(globals->vscreen, 10, 450, tlen, theight);

  sprintf(text1, "%d", mplayer[1]->get_frags());
  tlen = text_length(f, text1);
  
  if (mainloop_verbose) {
    printf("Rendering text '%s' at position (x: %d, y: 450) with dimensions (w: %d, h: %d)\n", 
           text1, 627-tlen, tlen, theight);
  }
  
  aatextout(globals->vscreen, f, text1, 627-tlen, 450, globals->col_grey);
  clearlist->add(globals->vscreen, 627-tlen, 450, tlen, theight);

  if (current_menu == MENU_MAIN)
  {
    #ifdef ALLEGRO_WINDOWS
      sprintf(text1, "GS/Win v%s", VERSION);
    #endif
    #ifdef ALLEGRO_LINUX
      sprintf(text1, "GS/Lnx v%s", VERSION);
    #endif
    #ifdef ALLEGRO_DOS
      sprintf(text1, "GS/Dos v%s", VERSION);
    #endif
  
    f = (FONT *)globals->fontdat[FONT_IMPACT10].dat;
    int impact_height = text_height(f);
    
    if (mainloop_verbose) {
      printf("Rendering font at IMPACT10 (index %d) with height %d pixels\n", FONT_IMPACT10, impact_height);
    }
    
//    tlen = text_length(f, text1);
//    aatextout(globals->vscreen, f, text1, 50, 443, globals->col_grey);

    strcat(text1, " -- ");
    strcat(text1, "www.gravity-strike.de");
  
    tlen = text_length(f, text1);
    
    if (mainloop_verbose) {
      printf("Rendering text '%s' at position (x: %d, y: 453) with dimensions (w: %d, h: %d)\n", 
             text1, 320-tlen/2, tlen, impact_height);
    }
    
    aatextout(globals->vscreen, f, text1, 320-tlen/2, 453, globals->col_green);
  
    clearlist->add(globals->vscreen, 50, 440, 540, 50);
  }
}

void gsMenu::update_mouse_crosshair()
{
/*
  if (globals->game_time % 10 == 0)
  {
    if (crosshair_count < 4) crosshair_count++;
    else crosshair_count = 0;
  }
*/

  mpixels->add(5, mouse_x, mouse_y, 0, 0, 1-(fxrand() % 3), 1-(fxrand() % 3), 250, globals->tbl_green, 8, 1000, 8);

}

