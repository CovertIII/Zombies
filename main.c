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
#include "freetype_imp.h"
#include "vector2.h"
#include "game.h"

#define TIMERMSECS 17

#define PREGAME 0
#define GAME 1
#define POSTGAME 2
#define GAMEOVER 3

int lastFrameTime = 0;

int game_mode = PREGAME;
double gm_timer = 0.0f;
int gm_lvl = 1;
int lives = 3;
GLuint lives_tex;
game gm;


char * argv1;

ALCdevice * device;
ALCcontext * context;
ALenum error;

int message = 0;

rat_font * font;
rat_font * sfont;

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
    rat_load_font(font, "imgs/MarkerFelt.ttc", 72*2);
    sfont = rat_init();
    rat_load_font(sfont, "imgs/MarkerFelt.ttc", 100);
	

    load_texture("imgs/hero.png",   &lives_tex);

	gm = gm_init();	
	//gm_init_sounds(gm);
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
	if (key == 27) 
		exit(0);
		
	if(key == 'r'){
		game_mode = GAME;
		gm_load_level(gm, argv1);
	}

	
	if(key == 'm'){
		message = message ? 0 : 1;
	}
	
	gm_nkey_down(gm, key);	
}

void releaseNormalKeys(unsigned char key, int xx, int yy) {
	gm_nkey_up(gm, key);	
}


void pressKey(int key, int xx, int yy) {
	if(game_mode == GAME || game_mode == POSTGAME){
		gm_skey_down(gm, key);	
	}
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
		case PREGAME:
			if(gm_timer >= 4){
				gm_timer = 0;
				game_mode = GAME;
				gm_message_render(gm);
			}
			break;
		case GAME:
			gm_update(gm,h);
			
			int state = gm_progress(gm);
			switch(state){
				case -1:
				gm_timer = 0;
                lives--;
				game_mode = lives < 0 ? GAMEOVER : POSTGAME;
				break;
				
				case 1:
				gm_timer = 0;
				game_mode = POSTGAME;
				gm_lvl++;	
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
			if (gm_timer > 4){
				gm_timer = 0;
				game_mode = PREGAME;
                gm_lvl = 1;
                lives = 3;
				//gm_free_level(gm);
				char level[30];
				sprintf(level, "./lvl/lvl%d.txt", gm_lvl);
				gm_load_level(gm, level);
				gm_update(gm,h);
			}
            break;
	}
	
	glutPostRedisplay();
}

void display(void) {
	//-----This is the stuff involved with drawing the screen----//	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	gm_render(gm);
	
    gm_message_render(gm);

    //This displays the number of lives left
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
    if(game_mode == PREGAME){
        int width = glutGet(GLUT_WINDOW_WIDTH);
        int height = glutGet(GLUT_WINDOW_HEIGHT);
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
    else if (game_mode == GAMEOVER){
        int width = glutGet(GLUT_WINDOW_WIDTH);
        int height = glutGet(GLUT_WINDOW_HEIGHT);
        float c[4] = {0,0,0,.1};
        rat_set_text_color(font, c);
        char buf[18];
        sprintf(buf, "Game Over!");

        float top = rat_font_height(font);
        float len = rat_font_text_length(font, buf);
        rat_font_render_text(font,(width-len)/2,(height+top)/2, buf);
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

