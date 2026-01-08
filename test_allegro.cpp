#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>
#include <stdio.h>

int main() {
    printf("Testing Allegro 5 initialization...\n");
    
    if (!al_init()) {
        printf("Failed to initialize Allegro 5\n");
        return -1;
    }
    
    printf("Allegro 5 initialized\n");
    
    al_install_keyboard();
    al_install_mouse();
    al_init_primitives_addon();
    
    printf("Installed keyboard, mouse and primitives addon\n");
    
    // Try to create a window
    ALLEGRO_DISPLAY *display = al_create_display(800, 600);
    if (!display) {
        printf("Failed to create display (may be due to headless environment)\n");
        printf("This is expected in a headless environment\n");
        return 0;
    }
    
    printf("Display created successfully\n");
    
    al_destroy_display(display);
    printf("Display destroyed\n");
    
    return 0;
}