#include <pthread.h>
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
#include "physics.h"
#include "sound_list.h"
#include "stream_sound.h"
#include "game.h"

#define TIMERMSECS 17

#define PREGAME 0
#define GAME 1
#define POSTGAME 2
#define GAMEOVER 3
#define USERSELECT 4
#define WIN 5

enum{
    al_game_over_buf,
    al_lvl_complete_buf,
    al_win_buf,
    al_loose1_buf,
	al_loose2_buf,
    al_count_buf,
    al_buf_num
};

char * res_path;
char * res_buf;

int sound_track = 1;

ALuint al_buf[al_buf_num];
s_list src_list;
al_stream als;

object snd_obj;
double cnt_down = 0;

int windowWidth = 1024;
int windowHeight = 728;
static SDL_Surface *gScreen;

int lastFrameTime = 0;
int nowfullscreen = 1;

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
int level_test = 0;

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

/*Put the recording of the stats on a different thread
 * so there is not a glitch in the game
 */
struct thread_data{
    data_record stats;
    int gm_lvl;
    double tmp_time;
    int ppl;
};

struct thread_data threadData;

void *save_lvl_stats(void *threadarg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    game_record_lvl_stats(my_data->stats, my_data->gm_lvl, my_data->tmp_time, my_data->ppl);
}
/* done with threading stuff */


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

static void createSurface (int fullscreen, int width, int height)
{
    Uint32 flags = 0;
    
    flags = SDL_OPENGL;
    if (fullscreen)
        flags |= SDL_FULLSCREEN;
	
    
    gScreen = SDL_SetVideoMode (width, height, 0, flags);
	
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
/*Set some openGL stuff up*/ 
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor (0.2f, 0.2f, 0.2f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
	
/*Set some openAL stuff up*/
	device = alcOpenDevice(NULL);
	if(device) {
		context = alcCreateContext(device, NULL);
	}
	alcMakeContextCurrent(context);

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	alGenBuffers(al_buf_num, al_buf);

/*Getting Resource path*/	
  NSString *respath = [[NSBundle mainBundle] resourcePath];
  res_path = (char*)malloc(strlen([respath cStringUsingEncoding:1])*sizeof(char)+sizeof(char)*50);
  res_buf = (char*)malloc(strlen([respath cStringUsingEncoding:1])*sizeof(char)+sizeof(char)*50);
  printf("Memsize = %ld\n", strlen([respath cStringUsingEncoding:1])*sizeof(char)+sizeof(char)*50);
  strcpy(res_path, [respath cStringUsingEncoding:1]);
  printf("Resource Location: %s\n", res_path);
	
//Loading HUD sounds
	strcpy(res_buf, res_path);
	strcat(res_buf, "/snd/game_over.ogg");
	snd_load_file(res_buf, al_buf[al_game_over_buf]);

	strcpy(res_buf, res_path);
	strcat(res_buf, "/snd/win.ogg");
	snd_load_file(res_buf, al_buf[al_win_buf]);

	strcpy(res_buf, res_path);
	strcat(res_buf, "/snd/lvl_complete.ogg");
	snd_load_file(res_buf, al_buf[al_lvl_complete_buf]);
	
	strcpy(res_buf, res_path);
	strcat(res_buf, "/snd/loose2.ogg");
	snd_load_file(res_buf, al_buf[al_loose1_buf]);
	
	strcpy(res_buf, res_path);
	strcat(res_buf, "/snd/loose3.ogg");
	snd_load_file(res_buf, al_buf[al_loose2_buf]);	

	strcpy(res_buf, res_path);
	strcat(res_buf, "/snd/wall2.ogg");
	snd_load_file(res_buf, al_buf[al_count_buf]);

	src_list = s_init();

// Init Database 
if(argc == 1){
  NSString * path;
  path = NSHomeDirectory ();
  path = [path stringByAppendingString:@"/.zombie.db"];
  printf("Database path: %s\n", [path cStringUsingEncoding:1]);
  stats = init_data_record([path cStringUsingEncoding:1], res_path);
  stats_list_prep(stats);
}

// Loads Fonts for HUD
  font = rat_init();
  strcpy(res_buf, res_path);
  strcat(res_buf, "/imgs/MedievalSharp.ttf");
  rat_load_font(font, res_buf, 72*2);
  sfont = rat_init();
  rat_load_font(sfont, res_buf, 100);
  ssfont = rat_init();
  rat_load_font(ssfont, res_buf, 30);
 
/*Loads Images for HUD*/
  strcpy(res_buf, res_path);
  strcat(res_buf, "/imgs/hero.png");
  load_texture(res_buf, &lives_tex);
  
  strcpy(res_buf, res_path);
  strcat(res_buf, "/imgs/extra.png");
  load_texture(res_buf, &extra_tex);

/*Init game_engine*/
  gm = gm_init(res_path);	
  gm_init_textures(gm);
  gm_init_sounds(gm);

	char level[30];
	if(argc == 1){
		srand(time(NULL));
		gm_lvl = 8;
        gm_free_level(gm);
        strcpy(res_buf, res_path);
        sprintf(level, "/lvl/lvl%d.png", gm_lvl);
        strcat(res_buf, level);
        gm_load_bk(gm, res_buf);
		strcpy(res_buf, res_path);
		sprintf(level, "/lvl/lvl%d.svg", gm_lvl);
		strcat(res_buf, level);
		gm_load_level_svg(gm, res_buf);
		level_test = 0;
	}
	if(argc == 2){
		argv1 = argv[1];
        gm_free_level(gm);
        char bk[400];
        strcpy(bk, argv1);
        strcat(bk, ".png");
        gm_load_bk(gm, bk);
		gm_load_level_svg(gm, argv1);
		game_mode = GAME;
		level_test = 1;
	}
	
	gm_set_view(gScreen->w, gScreen->h, gm);
	gm_update_view(gm);
	gm_update(gm,gScreen->w, gScreen->h,0.0001);
	
	
/*Load backgourd music and start playing*/
   
  als = al_stream_init();
  strcpy(res_buf, res_path);
  strcat(res_buf, "/snd/track1.ogg");
  al_stream_load_file(als, res_buf);
  al_stream_play(als);

	atexit(cleanup);
}

void reset(int width, int height){
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor (0.2f, 0.2f, 0.2f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
	
	strcpy(res_buf, res_path);
	strcat(res_buf, "/imgs/hero.png");
	load_texture(res_buf, &lives_tex);
	
	strcpy(res_buf, res_path);
	strcat(res_buf, "/imgs/extra.png");
	load_texture(res_buf, &extra_tex);
	
	gm_init_textures(gm);
	
	if(level_test != 1){
		stats_reload_fonts(stats);
	}
	
	font = rat_init();
	strcpy(res_buf, res_path);
	strcat(res_buf, "/imgs/MedievalSharp.ttf");
	rat_load_font(font, res_buf, 72*2);
	sfont = rat_init();
	rat_load_font(sfont, res_buf, 100);
	ssfont = rat_init();
	rat_load_font(ssfont, res_buf, 30);
	
	char level[30];
	if(level_test == 1){
        char bk[400];
        strcpy(bk, argv1);
        strcat(bk, ".png");
        gm_load_bk(gm, bk);
	}else{
		strcpy(res_buf, res_path);
        sprintf(level, "/lvl/lvl%d.png", gm_lvl);
        strcat(res_buf, level);
        gm_load_bk(gm, res_buf);	
	}
	
	gm_set_view(gScreen->w, gScreen->h, gm);
}

void processNormalKeys(unsigned char key) {
	if (key == 27) {
        if((game_mode == PREGAME || game_mode == GAME || game_mode == POSTGAME) && !level_test){
            game_finish_session(stats, total_deaths);
        }
		exit(0);
    }


	
	  gm_nkey_down(gm, key);	
    if(game_mode == USERSELECT){
        if(user_nkey_down(stats, key) == 1){
			total_deaths = 0;
            gm_lvl = 1;
            lives = 3;
            
			gm_free_level(gm);
            char level[30];
            strcpy(res_buf, res_path);
            sprintf(level, "/lvl/lvl%d.png", gm_lvl);
            strcat(res_buf, level);
            gm_load_bk(gm, res_buf);
			strcpy(res_buf, res_path);
			sprintf(level, "/lvl/lvl%d.svg", gm_lvl);
			strcat(res_buf, level);
			gm_load_level_svg(gm, res_buf);
			
            game_start_session(stats);
            gm_timer = 0.0f;
            game_mode = PREGAME;
            s_add_snd(src_list, al_buf[al_count_buf], &snd_obj, 1, 0);
        }
    }
    else if(game_mode == GAMEOVER || game_mode == WIN){ 
        if(key == ' '){
            gm_timer = 0;
            
			
            
            gm_update(gm,gScreen->w, gScreen->h,.01);
            game_mode = USERSELECT;
            stats_list_prep(stats);
        }
    }
    else{
        if(key == 'r' && level_test){
            game_mode = GAME;
            gm_free_level(gm);
            char bk[400];
            strcpy(bk, argv1);
            strcat(bk, ".png");
            gm_load_bk(gm, bk);
            gm_load_level_svg(gm, argv1);
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
    if(level_test == 0){
        user_skey_down(stats, key);
    }
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
	
    ALint state;
    state = al_stream_update(als);
    if(state == AL_STOPPED){
        al_stream_free_file(als);    
		strcpy(res_buf, res_path);
        if(sound_track == 1){
            strcat(res_buf, "/snd/track2.ogg");
            sound_track = 2;
        }
        else{
            strcat(res_buf, "/snd/track1.ogg");
            sound_track = 1;
        }
        al_stream_load_file(als, res_buf);
        al_stream_play(als);
    }

	switch(game_mode){
        case USERSELECT:
			gm_update(gm,gScreen->w, gScreen->h,h);
            break;
		case PREGAME:
            cnt_down += h;
            if(cnt_down > 1){
                cnt_down = 0;
                s_add_snd(src_list, al_buf[al_count_buf], &snd_obj, 1, 0);
            }
			if(gm_timer >= 4){
				gm_timer = 0;
				game_mode = GAME;
			}
			break;
		case GAME:
			gm_update(gm,gScreen->w, gScreen->h,h);
			
			int state = gm_progress(gm);
            if(state > 0){
                s_add_snd(src_list, al_buf[al_lvl_complete_buf], &snd_obj, 0.05, 0);
                int ppl;
                double tmp_time;
                gm_stats(gm, &tmp_time, &ppl);
                if(!level_test){
                	threadData.stats = stats;
                	threadData.gm_lvl = gm_lvl;
                	threadData.tmp_time = tmp_time;
                	threadData.ppl = ppl;
                	pthread_t thread;
                	
                	int rc = pthread_create(&thread, NULL, save_lvl_stats, (void *) &threadData);
                	if (rc){
                	    printf("ERROR; return code from pthread_create() is %d\n", rc);
                	}
                }

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
                if(lives < 0 && level_test == 0 ){
                    game_mode = GAMEOVER;
                    s_add_snd(src_list, al_buf[al_game_over_buf], &snd_obj,0.1, 0);
                    game_finish_session(stats, total_deaths);
                }else
                {
					int rand_num = rand()%2;	
					if (rand_num == 0) {
						s_add_snd(src_list, al_buf[al_loose1_buf], &snd_obj,0.1, 0);
					}
					else{
						s_add_snd(src_list, al_buf[al_loose2_buf], &snd_obj,0.1, 0);
					}
					
                    game_mode = POSTGAME;
                }
            }

			break;
		case POSTGAME:
			gm_update(gm,gScreen->w, gScreen->h, h);
			if (gm_timer > 4){				
				gm_timer = 0;
                if(!level_test){
                    game_mode = PREGAME;
                    gm_free_level(gm);
                    char level[30];
                    strcpy(res_buf, res_path);
                    sprintf(level, "/lvl/lvl%d.png", gm_lvl);
                    strcat(res_buf, level);
                    gm_load_bk(gm, res_buf);

                    strcpy(res_buf, res_path);
                    sprintf(level, "/lvl/lvl%d.svg", gm_lvl);
                    strcat(res_buf, level);
                    if(gm_load_level_svg(gm, res_buf) == 0 && level_test == 0){
                        game_mode = WIN;
                        s_add_snd(src_list, al_buf[al_win_buf], &snd_obj, 1, 0);
                        game_finish_session(stats, total_deaths);
                    }
                }
                else{
                    gm_load_level_svg(gm, argv1);
                    game_mode = GAME;
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
			gm_render(gm);
            if(stats_render(stats, gScreen->w, gScreen->h)==0){
				      char buf[180];
							float c[4] = {.1,.4,.15,1};
	            rat_set_text_color(sfont, c);
							sprintf(buf, "Zombies!");	
	            float len = rat_font_text_length(sfont, buf);
	            rat_font_render_text(sfont,(width-len)/2,(height) - 4, buf);
						}
            break;
        case PREGAME:
            gm_render(gm);
            gm_message_render(gm, gScreen->w, gScreen->h);
            showhud();
            float c[4] = {0,0,0,.1};
            rat_set_text_color(font, c);
						rat_set_text_color(sfont, c);
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
				case SDL_MOUSEMOTION:
          gm_mouse(gm, event.button.x, windowHeight - event.button.y);
					break;
				case SDL_MOUSEBUTTONDOWN:
          gm_mouse(gm, event.button.x, windowHeight - event.button.y);
					break;
				case SDL_KEYDOWN:
          processNormalKeys(event.key.keysym.sym);
          pressKey(event.key.keysym.sym);
					if (event.key.keysym.sym == 'f') {
						if(nowfullscreen == 1){
							windowWidth = 1280;
							windowHeight = 800;
							createSurface (nowfullscreen, windowWidth, windowHeight);
							reset(1280, 800);
							nowfullscreen = 0;
						} else {
							windowWidth = 1024;
							windowHeight = 768;
							createSurface (nowfullscreen, windowWidth, windowHeight);
							reset(windowWidth, windowHeight);
							nowfullscreen = 1;
						}
						
					}
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
		
		/*
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
		*/
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
    createSurface (0, windowWidth, windowHeight);
    printAttributes ();
    init(argc, argv);
    mainLoop ();
    SDL_Quit();
	
    return 0;
}
