#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>

int main() {
    printf("Testing Allegro 5 graphics initialization...\n");
    
    if (!al_init()) {
        printf("Failed to initialize Allegro 5\n");
        return -1;
    }
    
    if (!al_init_image_addon()) {
        printf("Failed to initialize Allegro 5 image addon\n");
        return -1;
    }
    
    printf("Allegro 5 initialized successfully\n");
    
    // Try to create a display
    ALLEGRO_DISPLAY *display = al_create_display(800, 600);
    if (!display) {
        printf("Failed to create display\n");
        return -1;
    }
    
    printf("Display created successfully\n");
    
    al_destroy_display(display);
    printf("Display destroyed\n");
    
    return 0;
}