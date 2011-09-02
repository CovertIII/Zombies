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

GLuint count[4];

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
	char level[30];
	sprintf(level, "./lvl/lvl%d.txt", gm_lvl);
	gm_load_level(gm, level);
	gm_update(gm,0.0001);
	
	GLubyte *textureImage;
	int width, height;
    int hasAlpha;
    int success;
   	success = load_png("imgs/1one.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures( 1, &count[1]);
		glBindTexture( GL_TEXTURE_2D, count[1]);
    	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    	glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? 4 : 3, width,
    	        height, 0, hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,
    	        textureImage);
		free(textureImage);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	
	success = load_png("imgs/2two.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures( 1, &count[2]);
		glBindTexture( GL_TEXTURE_2D, count[2]);
    	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    	glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? 4 : 3, width,
    	        height, 0, hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,
    	        textureImage);
		free(textureImage);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	
	success = load_png("imgs/3three.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures( 1, &count[3]);
		glBindTexture( GL_TEXTURE_2D, count[3]);
    	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    	glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? 4 : 3, width,
    	        height, 0, hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,
    	        textureImage);
		free(textureImage);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	atexit(cleanup);
	
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
}

void processNormalKeys(unsigned char key, int xx, int yy) {
	if (key == 27) 
		exit(0);
	
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
			gm_update(gm,h);
			if (gm_timer > 3){
				gm_timer = 0;
				game_mode = PREGAME;
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

//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	gm_render(gm);
	gm_update_view(gm);
	
	if(game_mode == PREGAME){
		if(gm_timer < 1){
			glBindTexture( GL_TEXTURE_2D, count[3]);
		}
		else if(gm_timer > 1 && gm_timer < 2){
			glBindTexture( GL_TEXTURE_2D, count[2]);
		}
		else if(gm_timer > 2 && gm_timer < 3){
			glBindTexture( GL_TEXTURE_2D, count[1]);
		}
		
		vector2 dm = gm_dim(gm);
		
		glPushMatrix();
		glTranslatef(dm.x, dm.y, 0);
		glScalef(10, 10,0);
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
	
	init();
   
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

