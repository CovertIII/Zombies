#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <GLUT/glut.h>
#include "vector2.h"
#include "game.h"

#define TIMERMSECS 17

#define PREGAME 0
#define GAME 1
#define POSTGAME 2

int lastFrameTime = 0;

int game_mode = PREGAME;
double gm_timer = 0.0f;
int gm_lvl = 1;
game gm;

ALCdevice * device;
ALCcontext * context;
ALenum error;

static void renderBitmapString(
						float x, 
						float y, 
						void *font,
						char *string) {  
	char *c;
	glRasterPos2f(x, y);
	for (c=string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
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

void init(void){
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor (0.2f, 0.2f, 0.2f, 1.0f);
	
	device = alcOpenDevice(NULL);
	if(device) {
		context = alcCreateContext(device, NULL);
	}
	alcMakeContextCurrent(context);
	
	gm = gm_init();	
	//gm_init_sounds(gm);
	gm_init_textures(gm);
	gm_load_level(gm, gm_lvl);

	atexit(cleanup);
	
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
}

void processNormalKeys(unsigned char key, int xx, int yy) {
	if (key == 27) 
		exit(0);
}

void pressKey(int key, int xx, int yy) {
	gm_skey_down(gm, key);	
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
			if(gm_timer > 3){
				gm_timer = 0;
				game_mode = GAME;
			}
			break;
		case GAME:
			gm_update(gm,h);
			
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
			if (gm_timer > 1){
				game_mode = PREGAME;
				//gm_free_level(gm);
				gm_load_level(gm, gm_lvl);
			}
			break;
	}
	
	glutPostRedisplay();
}

void display(void) {
	//-----This is the stuff involved with drawing the screen----//	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	gm_render(gm);

    glutSwapBuffers();
	
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	
	double ratio = glutGet(GLUT_WINDOW_WIDTH)/(double)glutGet(GLUT_WINDOW_HEIGHT);
	vector2 w = gm_dim(gm);
	double wx = ratio * w.y;
    glLoadIdentity();
    gluOrtho2D(0, wx, 0, w.y);
    glMatrixMode(GL_MODELVIEW);
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
	
	init();
   
	glutIgnoreKeyRepeat(1);
	glutSpecialFunc(pressKey);
	glutSpecialUpFunc(releaseKey); 
	glutKeyboardFunc(processNormalKeys);
	
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
	glutPostRedisplay();
	
	glutTimerFunc(TIMERMSECS, numbers, 0);
    glutMainLoop();


    return EXIT_SUCCESS;
}

