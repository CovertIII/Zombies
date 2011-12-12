#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <GLUT/glut.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include "freetype_imp.h"
#include "vector2.h"
#include "game.h"

#define TIMERMSECS 17

#define PREGAME 0
#define GAME 1
#define POSTGAME 2


int windowWidth = 1024;
int windowHeight = 728;
static SDL_Surface *gScreen;

int lastFrameTime = 0;

int game_mode = PREGAME;
double gm_timer = 0.0f;
int gm_lvl = 1;
game gm;

char * argv1;

ALCdevice * device;
ALCcontext * context;
ALenum error;

rat_font * font;
rat_font * sfont;

static void initAttributes ()
{
    int value;
    value = 16;
    SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, value);
    
	value = 1;
    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, value);
}

static void printAttributes ()
{
    int nAttr;
    int i;
    
    int  attr[] = { SDL_GL_RED_SIZE, SDL_GL_BLUE_SIZE, SDL_GL_GREEN_SIZE,
	SDL_GL_ALPHA_SIZE, SDL_GL_BUFFER_SIZE, SDL_GL_DEPTH_SIZE };
	
    char *desc[] = { "Red size: %d bits\n", "Blue size: %d bits\n", "Green size: %d bits\n",
		"Alpha size: %d bits\n", "Color buffer size: %d bits\n", 
	"Depth bufer size: %d bits\n" };
	
    nAttr = sizeof(attr) / sizeof(int);
    
    for (i = 0; i < nAttr; i++) {
		
        int value;
        SDL_GL_GetAttribute (attr[i], &value);
        printf (desc[i], value);
    } 
}

static void createSurface (int fullscreen)
{
    Uint32 flags = 0;
    
    flags = SDL_OPENGL;
    if (fullscreen)
        flags |= SDL_FULLSCREEN;
	
    
    gScreen = SDL_SetVideoMode (windowWidth, windowHeight, 0, flags);
	
    if (gScreen == NULL) {
		
        fprintf (stderr, "Couldn't set 640x480 OpenGL video mode: %s\n",
                 SDL_GetError());
		SDL_Quit();
		exit(2);
	}
}

void cleanup (void) {
	gm_free(gm);

	context = alcGetCurrentContext();
	device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void init(int argc, char** argv){
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor (0.2f, 0.2f, 0.2f, 1.0f);
	
	device = alcOpenDevice(NULL);
	if(device) {
		context = alcCreateContext(device, NULL);
	}
	alcMakeContextCurrent(context);
		
	font = rat_init();
  rat_load_font(font, "/Library/Fonts/Arial.ttf", 72*2);
  sfont = rat_init();
  rat_load_font(sfont, "/Library/Fonts/Arial.ttf", 100);
	
	gm = gm_init();	
	//gm_init_sounds(gm);
	gm_init_textures(gm);
	char level[30];
	if(argc == 1){
		sprintf(level, "lvl%d", gm_lvl);
		gm_load_level(gm, level);
	}
	if(argc == 2){
		argv1 = argv[1];
		gm_load_level(gm, argv1);
	}
	
	gm_set_view(gScreen->w, gScreen->h,gm);
	gm_update_view(gm);
	gm_reshape(gm, gScreen->w, gScreen->h);	
	gm_update(gm,gScreen->w, gScreen->h,0.0001);
	

	atexit(cleanup);
	
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
}

static void numbers(void)
{	
	int now =  SDL_GetTicks ();
    int elapsedMilliseconds = now - lastFrameTime;
    float elapsedTime = elapsedMilliseconds / 1000.0f;
    lastFrameTime = now;
	float h = elapsedTime;
	gm_timer += h;
	
	switch(game_mode){
		case PREGAME:
			if(gm_timer > 3){
				gm_timer = 0;
				game_mode = GAME;
			}
			break;
		case GAME:
			gm_update(gm,gScreen->w, gScreen->h,h);
			
			int state = gm_progress(gm);
			switch(state){
				case -1:
				gm_timer = 0;
				game_mode = POSTGAME;
				break;
				
				case 1:
				gm_timer = 0;
				game_mode = POSTGAME;
				gm_lvl++;	
			}
			break;
		case POSTGAME:
			gm_update(gm,gScreen->w, gScreen->h,h);
			if (gm_timer > 3){
				gm_timer = 0;
				game_mode = PREGAME;
				//gm_free_level(gm);
				char level[30];
				sprintf(level, "lvl%d", gm_lvl);
				gm_load_level(gm, level);
				gm_set_view(gScreen->w, gScreen->h,gm);
				gm_update_view(gm);
				gm_update(gm,gScreen->w, gScreen->h,h);
			}
			break;
	}
}


static void drawGL ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	gm_update_view(gm);
	gm_render(gm);
	
	gm_message_render(gm, windowHeight, windowWidth );
    if(game_mode == PREGAME){
        int width = windowWidth;
        int height = windowHeight;
        float c[4] = {0,0,0,.1};
        rat_set_text_color(font, c);
        char buf[18];
        if(gm_timer < 3){
            sprintf(buf, "%d",(int)round(3.5-gm_timer));	
        }
        else{
            sprintf(buf, "GO!");	
        }

        float top = rat_font_height(font);
        float len = rat_font_text_length(font, buf);
        rat_font_render_text(font,(width-len)/2,(height+top)/2, buf);

        sprintf(buf, "Level %d",gm_lvl);	
        top = rat_font_height(sfont);
        len = rat_font_text_length(sfont, buf);
        rat_font_render_text(sfont,(width-len)/2,(height)/2 + top + 100, buf);

    }
}

static void mainLoop ()
{
    SDL_Event event;
    int done = 0;
    int fps = 157;
		int delay = 1000/fps;
    int thenTicks = -1;
    int nowTicks;
	
	
    while ( !done ) {
	
		while ( SDL_PollEvent (&event) ) {
			switch (event.type) {
				case SDL_KEYDOWN:
				gm_nkey_down(gm, event.key.keysym.sym);
				gm_skey_down(gm, event.key.keysym.sym);
					switch( event.key.keysym.sym ){
						case SDLK_SPACE:
							break;

						case SDLK_ESCAPE:
							exit(0);
							break;
						default:
							break;
					}
					break;
				case SDL_KEYUP:
					gm_nkey_up(gm, event.key.keysym.sym);
					gm_skey_up(gm, event.key.keysym.sym);
					switch( event.key.keysym.sym ){
						case SDLK_SPACE:
							break;
							
						case SDLK_ESCAPE:
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		}

		numbers();
		drawGL ();
    SDL_GL_SwapBuffers (); 
		
		
		if (thenTicks > 0) {
            nowTicks = SDL_GetTicks ();
            delay += (1000/fps - (nowTicks-thenTicks));
            thenTicks = nowTicks;
            if (delay < 0)
                delay = 1000/fps;
        }
        else {
            thenTicks = SDL_GetTicks ();
        }
		
		SDL_Delay (delay);
	}
}


int main(int argc, char *argv[])
{
	if ( SDL_Init (SDL_INIT_VIDEO) < 0 ) {
		
        fprintf(stderr, "Couldn't initialize SDL: %s\n",
				SDL_GetError());
		exit(1);
	}
	
    initAttributes ();
    createSurface (0);
    printAttributes ();
    init(argc, argv);
    mainLoop ();
    SDL_Quit();
	
    return 0;
}
