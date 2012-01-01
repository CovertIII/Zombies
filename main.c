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
#include "data_interface.h"
#include "freetype_imp.h"
#include "vector2.h"
#include "game.h"

#define TIMERMSECS 17

#define PREGAME 0
#define GAME 1
#define POSTGAME 2
#define GAMEOVER 3
#define USERSELECT 4
#define WIN 5


int windowWidth = 1024;
int windowHeight = 728;
static SDL_Surface *gScreen;

int lastFrameTime = 0;

int game_mode = USERSELECT;
double gm_timer = 0.0f;
int gm_lvl = 1;
int lives = 3;
int extra_ppl = 0;
int total_deaths = 0;
GLuint lives_tex;
GLuint extra_tex;
game gm;

char * argv1;

ALCdevice * device;
ALCcontext * context;
ALenum error;

int message = 0;

rat_font * font;
rat_font * sfont;
rat_font * ssfont;

data_record stats;

void processNormalKeys(unsigned char key);
void releaseNormalKeys(unsigned char key);
void pressKey(int key);
void releaseKey(int key);

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

    stats = init_data_record(".zombie_stats.db");
    stats_list_prep(stats);

    font = rat_init();
    rat_load_font(font, "/Library/Fonts/MarkerFelt.ttc", 72*2);
    sfont = rat_init();
    rat_load_font(sfont, "/Library/Fonts/MarkerFelt.ttc", 100);
    ssfont = rat_init();
    rat_load_font(ssfont, "/Library/Fonts/MarkerFelt.ttc", 30);
	

    //NSString * path;
    //path = [[NSBundle mainBundle] pathForResource: @"hero" ofType: @"png"];
    //load_texture([path cStringUsingEncoding:1],   &lives_tex);
    load_texture("imgs/hero.png",   &lives_tex);
    //path = [[NSBundle mainBundle] pathForResource: @"extra" ofType: @"png"];
    //load_texture([path cStringUsingEncoding:1],   &lives_tex);
    load_texture("imgs/extra.png",   &extra_tex);

	gm = gm_init();	
	gm_init_sounds(gm);
	gm_init_textures(gm);
	char level[30];
	if(argc == 1){
		sprintf(level, "./lvl/lvl%d.txt", gm_lvl);
		gm_load_level(gm, level);
		//sprintf(level, "lvl%d", gm_lvl);
        //path = [[NSBundle mainBundle] pathForResource: [NSString stringWithFormat:@"%s", level] ofType: @"txt"];
		//gm_load_level(gm, [path cStringUsingEncoding:1]);
	}
	if(argc == 2){
		argv1 = argv[1];
		gm_load_level(gm, argv1);
	}
	
	gm_set_view(gScreen->w, gScreen->h,gm);
	gm_update_view(gm);
	gm_update(gm,gScreen->w, gScreen->h,0.0001);
	
	gm_update(gm, gScreen->w, gScreen->h, 0.0001);
	
	atexit(cleanup);
	
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
}

void processNormalKeys(unsigned char key) {
	if (key == 27) {
        if(game_mode == PREGAME || game_mode == GAME || game_mode == POSTGAME){
                game_finish_session(stats, total_deaths);
        }
		exit(0);
    }


	
	gm_nkey_down(gm, key);	
    if(game_mode == USERSELECT){
        if(user_nkey_down(stats, key) == 1){
            game_start_session(stats);
            gm_timer = 0.0f;
            game_mode = PREGAME;
        }
    }
    else if(game_mode == GAMEOVER || game_mode == WIN){ 
        if(key == ' '){
            gm_timer = 0;
            total_deaths = 0;
            gm_lvl = 1;
            lives = 3;
            //gm_free_level(gm);
            char level[30];
            //sprintf(level, "lvl%d", gm_lvl);
            //path = [[NSBundle mainBundle] pathForResource: [NSString stringWithFormat:@"%s", level] ofType: @"txt"];
            //gm_load_level(gm, [path cStringUsingEncoding:1]);
            sprintf(level, "./lvl/lvl%d.txt", gm_lvl);
            gm_load_level(gm, level);
            gm_update(gm,gScreen->w, gScreen->h,.01);
            game_mode = USERSELECT;
            stats_list_prep(stats);
        }
    }
    else{
        if(key == 'r'){
            game_mode = GAME;
            gm_load_level(gm, argv1);
        }
    }
}

void releaseNormalKeys(unsigned char key) {
	gm_nkey_up(gm, key);	
}


void pressKey(int key) {
	if(game_mode == GAME || game_mode == POSTGAME){
		gm_skey_down(gm, key);	
	}
    user_skey_down(stats, key);
}

void releaseKey(int key) {
		gm_skey_up(gm, key);
}

void numbers(void)
{
	int now =  SDL_GetTicks ();	
    int elapsedMilliseconds = now - lastFrameTime;
    float elapsedTime = elapsedMilliseconds / 1000.0f;
    lastFrameTime = now;
	float h = elapsedTime;
	gm_timer += h;
	
	switch(game_mode){
        case USERSELECT:
            break;
		case PREGAME:
			if(gm_timer >= 4){
				gm_timer = 0;
				game_mode = GAME;
			}
			break;
		case GAME:
			gm_update(gm,gScreen->w, gScreen->h,h);
			
			int state = gm_progress(gm);
            if(state > 0){
                int ppl;
                double tmp_time;
                gm_stats(gm, &tmp_time, &ppl);
                game_record_lvl_stats(stats, gm_lvl, tmp_time, ppl);
				gm_timer = 0;
				game_mode = POSTGAME;
				gm_lvl++;	
                extra_ppl += state - 1;
                while(extra_ppl >= 3){
                    lives++;
                    extra_ppl -= 3;
                    extra_ppl = extra_ppl < 0 ? 0 : extra_ppl;
                }
            }
            else if(state < 0){
				gm_timer = 0;
                lives--;
                total_deaths++;
                extra_ppl = extra_ppl <= 0 ? 0 : extra_ppl--;
                if(lives < 0 ){
                    game_mode = GAMEOVER;
                    game_finish_session(stats, total_deaths);
                }else
                {
                    game_mode = POSTGAME;
                }
            }

			break;
		case POSTGAME:
			gm_update(gm,gScreen->w, gScreen->h, h);
			if (gm_timer > 4){
				gm_timer = 0;
				game_mode = PREGAME;
				//gm_free_level(gm);
				char level[30];
                //sprintf(level, "lvl%d", gm_lvl);
                //path = [[NSBundle mainBundle] pathForResource: [NSString stringWithFormat:@"%s", level] ofType: @"txt"];
                //gm_load_level(gm, [path cStringUsingEncoding:1]);
				sprintf(level, "./lvl/lvl%d.txt", gm_lvl);
				if(gm_load_level(gm, level) == 0){
                    game_mode = WIN;
                    game_finish_session(stats, total_deaths);
                }
				gm_update(gm,gScreen->w, gScreen->h, h);
			}
			break;
        case GAMEOVER:
			gm_update(gm,gScreen->w, gScreen->h,h);
            break;
        case WIN:
			gm_update(gm,gScreen->w, gScreen->h,h);
            break;
	}
}

void showhud(void){
    float ratio = (double)gScreen->w/(double)gScreen->h;
    int height = 600;
    int width = height*ratio;
    int i;
    glBindTexture(GL_TEXTURE_2D, lives_tex);
    for (i = 0; i < lives; i++)
    {
        glPushMatrix();
        glTranslatef(i*26 + 20, 20, 0);
        glScalef(12, 12,0);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(-1.0, -1.0, 0.0);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(-1.0, 1.0, 0.0);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(1.0, 1.0, 0.0);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(1.0, -1.0, 0.0);
        glEnd();
        glPopMatrix();
    }
    glBindTexture(GL_TEXTURE_2D, extra_tex);
    for (i = 0; i < extra_ppl; i++)
    {
        glPushMatrix();
        glTranslatef(width - i*26 - 20, 20, 0);
        glScalef(12, 12,0);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(-1.0, -1.0, 0.0);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(-1.0, 1.0, 0.0);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(1.0, 1.0, 0.0);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(1.0, -1.0, 0.0);
        glEnd();
        glPopMatrix();
    }
}

static void drawGL ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float ratio = (double)gScreen->w/(double)gScreen->h;   
    int height = 600;
    int width = height*ratio;

    switch(game_mode){
        case USERSELECT:
            stats_render(stats, gScreen->w, gScreen->h);
            break;
        case PREGAME:
            gm_render(gm);
            gm_message_render(gm, gScreen->w, gScreen->h);
            showhud();
            float c[4] = {0,0,0,.1};
            rat_set_text_color(font, c);
            char buf[180];
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
            break;
        case GAME:
            gm_render(gm);
            gm_message_render(gm, gScreen->w, gScreen->h);
            showhud();
            break;
        case POSTGAME:
            gm_render(gm);
            gm_message_render(gm, gScreen->w, gScreen->h);
            showhud();
            break;
        case GAMEOVER:
            gm_render(gm);

            glPushMatrix();
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            float ratio = (double)gScreen->w/(double)gScreen->h;
            height = 600;
            width = height*ratio;
           
            gluOrtho2D(0, width, 0, height);
            glMatrixMode(GL_MODELVIEW);

            c[0] = 0; c[1] = 0; c[2] =0; c[3]= .1;
            rat_set_text_color(font, c);
            sprintf(buf, "Game Over!");
            top = rat_font_height(font);
            len = rat_font_text_length(font, buf);
            rat_font_render_text(font,(width-len)/2,(height+top)/2 + 100, buf);

            c[0] = 0; c[1] = 0; c[2] =0; c[3]= .1;
            rat_set_text_color(ssfont, c);
            sprintf(buf, "Push space to continue or esc to exit.");
            top = rat_font_height(ssfont);
            len = rat_font_text_length(ssfont, buf);
            rat_font_render_text(ssfont,(width-len)/2,(height+top)/2 - 100, buf);
            showhud();
            break;
        case WIN:
            gm_render(gm);

            glPushMatrix();
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            ratio = (double)gScreen->w/(double)gScreen->h;
            height = 600;
            width = height*ratio;
           
            gluOrtho2D(0, width, 0, height);
            glMatrixMode(GL_MODELVIEW);

            c[0] = 0; c[1] = 0; c[2] =0; c[3]= .1;
            rat_set_text_color(font, c);
            sprintf(buf, "You Win!");
            top = rat_font_height(font);
            len = rat_font_text_length(font, buf);
            rat_font_render_text(font,(width-len)/2,(height+top)/2 + 100, buf);

            c[0] = 0; c[1] = 0; c[2] =0; c[3]= .1;
            rat_set_text_color(ssfont, c);
            sprintf(buf, "Push space to continue or esc to exit.");
            top = rat_font_height(ssfont);
            len = rat_font_text_length(ssfont, buf);
            rat_font_render_text(ssfont,(width-len)/2,(height+top)/2 - 100, buf);
            showhud();
            break;
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
                    processNormalKeys(event.key.keysym.sym);
                    pressKey(event.key.keysym.sym);
					break;
				case SDL_KEYUP:
                    releaseNormalKeys(event.key.keysym.sym);
                    releaseKey(event.key.keysym.sym);
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
