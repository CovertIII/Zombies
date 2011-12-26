#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <GLUT/glut.h>
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
    prepare_user_list(stats);
    prepare_game_list(stats);
    prepare_level_scores(stats);

    font = rat_init();
    rat_load_font(font, "imgs/MarkerFelt.ttc", 72*2);
    sfont = rat_init();
    rat_load_font(sfont, "imgs/MarkerFelt.ttc", 100);
    ssfont = rat_init();
    rat_load_font(ssfont, "imgs/MarkerFelt.ttc", 30);
	

    load_texture("imgs/hero.png",   &lives_tex);
    load_texture("imgs/extra.png",   &extra_tex);

	gm = gm_init();	
	gm_init_sounds(gm);
	gm_init_textures(gm);
	char level[30];
	if(argc == 1){
		sprintf(level, "./lvl/lvl%d.txt", gm_lvl);
		gm_load_level(gm, level);
	}
	if(argc == 2){
		argv1 = argv[1];
		gm_load_level(gm, argv1);
	}
	
	
	gm_update(gm,0.0001);
	
	atexit(cleanup);
	
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
}

void processNormalKeys(unsigned char key, int xx, int yy) {
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
    else if(game_mode == GAMEOVER){
        if(key == ' '){
            gm_timer = 0;
            total_deaths = 0;
            gm_lvl = 1;
            lives = 3;
            //gm_free_level(gm);
            char level[30];
            sprintf(level, "./lvl/lvl%d.txt", gm_lvl);
            gm_load_level(gm, level);
            gm_update(gm,.01);
            game_mode = USERSELECT;
            prepare_user_list(stats);
        }
    }
    else{
        if(key == 'r'){
            game_mode = GAME;
            gm_load_level(gm, argv1);
        }
    }
}

void releaseNormalKeys(unsigned char key, int xx, int yy) {
	gm_nkey_up(gm, key);	
}


void pressKey(int key, int xx, int yy) {
	if(game_mode == GAME || game_mode == POSTGAME){
		gm_skey_down(gm, key);	
	}

    user_skey_down(stats, key);
}

void releaseKey(int key, int xx, int yy) {
		gm_skey_up(gm, key);
}

void numbers(int value)
{
	glutTimerFunc(TIMERMSECS, numbers, 0);
	
	int now = glutGet(GLUT_ELAPSED_TIME);
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
			gm_update(gm,h);
			
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
				game_mode = lives < 0 ? GAMEOVER : POSTGAME;
                if(lives < 0 ){
                    game_finish_session(stats, total_deaths);
                }
            }

			break;
		case POSTGAME:
			gm_update(gm,h);
			if (gm_timer > 4){
				gm_timer = 0;
				game_mode = PREGAME;
				//gm_free_level(gm);
				char level[30];
				sprintf(level, "./lvl/lvl%d.txt", gm_lvl);
				gm_load_level(gm, level);
				gm_update(gm,h);
			}
			break;
        case GAMEOVER:
			gm_update(gm,h);
            break;
	}
	
	glutPostRedisplay();
}

void showhud(void){
    float ratio = glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT);
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

void display(void) {
	//-----This is the stuff involved with drawing the screen----//	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float ratio = glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT);
    int height = 600;
    int width = height*ratio;

    switch(game_mode){
        case USERSELECT:
            stats_render(stats);
            break;
        case PREGAME:
            gm_render(gm);
            gm_message_render(gm);
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
            gm_message_render(gm);
            showhud();
            break;
        case POSTGAME:
            gm_render(gm);
            gm_message_render(gm);
            showhud();
            break;
        case GAMEOVER:
            gm_render(gm);
            gm_message_render(gm);
            c[0] = 0; c[1] = 0; c[2] =0; c[3]= .1;
            rat_set_text_color(font, c);
            sprintf(buf, "Game Over!");

            top = rat_font_height(font);
            len = rat_font_text_length(font, buf);
            rat_font_render_text(font,(width-len)/2,(height+top)/2, buf);
            c[0] = 0; c[1] = 0; c[2] =0; c[3]= .1;
            rat_set_text_color(ssfont, c);
            sprintf(buf, "Push space to continue or esc to exit.");

            top = rat_font_height(ssfont);
            len = rat_font_text_length(ssfont, buf);
            rat_font_render_text(ssfont,(width-len)/2,(height+top)/2 - 100, buf);
            showhud();
            break;
    }
    glutSwapBuffers();
	
}

void reshape(int width, int height)
{
	gm_reshape(gm, width, height);
}

void idle(void)
{
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    
    glutCreateWindow("Zombies!");
	
	init(argc, argv);
   
	glutIgnoreKeyRepeat(1);
	glutSpecialFunc(pressKey);
	glutSpecialUpFunc(releaseKey); 
	glutKeyboardFunc(processNormalKeys);
	glutKeyboardUpFunc(releaseNormalKeys);
	
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
	glutPostRedisplay();
	
	glutTimerFunc(TIMERMSECS, numbers, 0);
    glutMainLoop();


    return EXIT_SUCCESS;
}

