#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <GLUT/glut.h>
#include "load_sound.h"
#include "vector2.h"
#include "sound_list.h"
#include "game.h"
#include "load_png.h"

#define ZOMBIE 0
#define Z_P 1
#define PERSON 2
#define P_Z 3
#define SAFE 4
#define DONE 5

#define ATTACHED 0
#define NOT_ATTACHED 1

#define MAX_TIME 5.0f
#define SAFE_ZONE_RAD 10
#define PERSON_RAD 2
#define PERSON_MASS 0.5

#define VIEWRATIO 50


typedef struct {
	vector2 p;
	vector2 v;
	vector2 f;
	double m;
	double r;
	double th;
	double w; 
} object;

typedef struct {
	object o;
	float timer;
	int state; 
} ppl;

typedef struct {
	object o;
	
	float timer;
	int state; 
	
	int spring_state;
	int person_id;
} _hero;

typedef struct gametype {
	vector2 ak;  /*arrow key presses*/
	vector2 vmin;
	vector2 vmax;
	
	int h,w; //Height and width of the level
	ppl person[100]; //Zombies and people array
	int person_num; //How many there are
	_hero hero;
	object safe_zone;
	int save_count;  //How many people you have to save to win the level
	
	
	GLuint zombie_tex;
	GLuint person_tex;
	GLuint safe_tex;
	GLuint safezone_tex;
	GLuint hero_tex;
	GLuint heroattached_tex;
	GLuint herosafe_tex;
	GLuint p_z_tex;
	GLuint h_z_tex;
	GLuint hzombie_tex;
	GLuint bk;
	GLuint rope_tex;
	
} gametype;

static void circle(float pos_x, float pos_y, float size);
static int bounce(object * obj, int x, int y);
static int collision(object *ta, object *tb);
static int collision_test(object ta, object tb);
int safe_zone_test(object ta, object tb);
int r_collision(object *ta, object *tb);
int load_level_file(game gm, char * file);

game gm_init(void){
	gametype * gm;
	gm = (game)malloc(sizeof(gametype));
	if(!gm) {return NULL;}
	
	glLineWidth(4);

	return gm;
}

int gm_init_textures(game gm){
	GLubyte *textureImage;
	int width, height;
    int hasAlpha;
    int success;
   	success = load_png("imgs/zombie.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures( 1, &gm->zombie_tex);
		glBindTexture( GL_TEXTURE_2D, gm->zombie_tex);
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
	
	success = load_png("imgs/person.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures( 1, &gm->person_tex);
		glBindTexture( GL_TEXTURE_2D, gm->person_tex);
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
	
	success = load_png("imgs/safe.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures( 1, &gm->safe_tex);
		glBindTexture( GL_TEXTURE_2D, gm->safe_tex);
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
	
	success = load_png("imgs/safezone.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures( 1, &gm->safezone_tex);
		glBindTexture( GL_TEXTURE_2D, gm->safezone_tex);
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
	
	success = load_png("imgs/hero.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures( 1, &gm->hero_tex);
		glBindTexture( GL_TEXTURE_2D, gm->hero_tex);
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
	
	success = load_png("imgs/p_z.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures( 1, &gm->p_z_tex);
		glBindTexture( GL_TEXTURE_2D, gm->p_z_tex);
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
	
	success = load_png("imgs/h_z.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures(1, &gm->h_z_tex);
		glBindTexture( GL_TEXTURE_2D, gm->h_z_tex);
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
	
	success = load_png("imgs/hzombie.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures(1, &gm->hzombie_tex);
		glBindTexture( GL_TEXTURE_2D, gm->hzombie_tex);
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
	
	success = load_png("imgs/heroattached.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures(1, &gm->heroattached_tex);
		glBindTexture( GL_TEXTURE_2D, gm->heroattached_tex);
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
	
	success = load_png("imgs/herosafe.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures(1, &gm->herosafe_tex);
		glBindTexture( GL_TEXTURE_2D, gm->herosafe_tex);
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
	
	success = load_png("imgs/rope.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures(1, &gm->rope_tex);
		glBindTexture( GL_TEXTURE_2D, gm->rope_tex);
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
	
	success = load_png("imgs/bk.png", &width, &height, &hasAlpha, &textureImage);
    if (success) {
		glGenTextures(1, &gm->bk);
		glBindTexture( GL_TEXTURE_2D, gm->bk);
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
}

void gm_set_view(game gm){
	double ratio = glutGet(GLUT_WINDOW_WIDTH)/(double)glutGet(GLUT_WINDOW_HEIGHT);
	vector2 w = {VIEWRATIO, VIEWRATIO};
	w.x = ratio * w.y;
	gm->vmin.x = gm->hero.o.p.x - w.x/2.0f;
	gm->vmin.y = gm->hero.o.p.y - w.y/2.0f;
	gm->vmax.x = gm->hero.o.p.x + w.x/2.0f;
	gm->vmax.y = gm->hero.o.p.y + w.y/2.0f;
	
}

void gm_update_view(game gm){
	double minx = 1.0f/4.0*(gm->vmax.x - gm->vmin.x) + gm->vmin.x;
	double maxx = gm->vmax.x - 1.0f/4.0*(gm->vmax.x - gm->vmin.x);
	
	if(gm->hero.o.p.x < minx && gm->vmin.x > 0){
		gm->vmin.x -= minx - gm->hero.o.p.x;
		gm->vmax.x -= minx - gm->hero.o.p.x;
	}
	
	else if (gm->hero.o.p.x > maxx && gm->vmax.x < gm->w){
		gm->vmin.x += gm->hero.o.p.x - maxx;
		gm->vmax.x += gm->hero.o.p.x - maxx;
	}
	
	double miny = 1.0f/4.0*(gm->vmax.y - gm->vmin.y) + gm->vmin.y;
	double maxy = gm->vmax.y - 1.0f/4.0*(gm->vmax.y - gm->vmin.y);
	if(gm->hero.o.p.y < miny  && gm->vmin.y > 0){
		gm->vmin.y -= miny - gm->hero.o.p.y;
		gm->vmax.y -= miny - gm->hero.o.p.y;
	}
	
	else if (gm->hero.o.p.y > maxy  && gm->vmax.y < gm->h){
		gm->vmin.y += gm->hero.o.p.y - maxy;
		gm->vmax.y += gm->hero.o.p.y - maxy;
	}
	
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(gm->vmin.x, gm->vmax.x, gm->vmin.y, gm->vmax.y);
    glMatrixMode(GL_MODELVIEW);
}


void gm_reshape(game gm, int width, int height){
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	double ratio = glutGet(GLUT_WINDOW_WIDTH)/(double)glutGet(GLUT_WINDOW_HEIGHT);
	vector2 w = {VIEWRATIO, VIEWRATIO};
	w.x = ratio * w.y;
	double diff = w.x - (gm->vmax.x - gm->vmin.x);
	gm->vmax.x += diff;
    glLoadIdentity();
    gluOrtho2D(gm->vmin.x, gm->vmax.x, gm->vmin.y, gm->vmax.y);
    glMatrixMode(GL_MODELVIEW);
}

void gm_load_level(game gm, char * lvl){
	int i;
	gm->ak.x =0;
	gm->ak.y = 0;

	load_level_file(gm, lvl);
}

void gm_init_sound(game gm){
	
}

void gm_update_sound(game gm){
	
}

void gm_update(game gm, double dt){
	int i, k;
	/*Timers */
	for(i = 0; i < gm->person_num; i++){
		gm->person[i].timer -= dt;
		if(gm->person[i].state == P_Z && gm->person[i].timer <= 0.0f){
			gm->person[i].state = ZOMBIE;
			gm->person[i].o.v.x = rand()%50 - 25;
			gm->person[i].o.v.y = rand()%50 - 25;
		}
	}
	gm->hero.timer -= dt;
	if(gm->hero.state == P_Z && gm->hero.timer <= 0.0f){
		gm->hero.state = ZOMBIE;
	}
	
	
	/*Wall Colisions*/
	for(i = 0; i < gm->person_num; i++){
		bounce(&gm->person[i].o, gm->w, gm->h);
	}	
	bounce(&gm->hero.o, gm->w, gm->h);

	/*Circle Collisons*/
	
	if(gm->hero.state == P_Z || gm->hero.state == ZOMBIE){
		vector2 p = gm->safe_zone.p;
		collision(&gm->hero.o, &gm->safe_zone); 
		gm->safe_zone.p = p;	
	}
	
	if(safe_zone_test(gm->safe_zone, gm->hero.o) && gm->hero.state == PERSON){
		gm->hero.state = SAFE;	
	} 
	else if(!safe_zone_test(gm->safe_zone, gm->hero.o) && gm->hero.state == SAFE){
		gm->hero.state = PERSON;	
	}
	if(gm->hero.state == DONE){
		r_collision(&gm->safe_zone, &gm->hero.o);
	}
	
	
	
	for(i = 0; i < gm->person_num; i++){
		if(collision(&gm->person[i].o, &gm->hero.o)){
			if(gm->hero.state == PERSON && gm->hero.spring_state == NOT_ATTACHED && gm->person[i].state == PERSON)
			{
			gm->hero.spring_state = ATTACHED;
			gm->hero.person_id = i;
			}
			else if(gm->person[i].state == ZOMBIE && gm->hero.state == PERSON){
				gm->hero.spring_state = NOT_ATTACHED;
				gm->hero.timer = MAX_TIME;
				gm->hero.state = P_Z;
			}
			else if(gm->person[i].state == PERSON && gm->hero.state == ZOMBIE){
				gm->person[i].timer = MAX_TIME;
				gm->person[i].state = P_Z;
			}
		}
		
		/*This bit of magic lets the hero take a person info a circle and save them.*/
		vector2 p = gm->safe_zone.p;
		int hpid = gm->hero.person_id;
		if((i != hpid || gm->hero.spring_state == NOT_ATTACHED) && gm->person[i].state != SAFE){
			collision(&gm->person[i].o, &gm->safe_zone); 	
		}
		else if(gm->person[i].state == SAFE){
			r_collision(&gm->safe_zone, &gm->person[i].o);
		}
		else if (safe_zone_test(gm->safe_zone, gm->person[i].o)){
			gm->hero.spring_state = NOT_ATTACHED;
			gm->person[i].state = SAFE;
		}
		gm->safe_zone.p = p;
		
		/*This deals with collisions between people*/	
		for(k = 1+i; k < gm->person_num; k++){
			if(collision(&gm->person[i].o, &gm->person[k].o)){
				if(gm->person[i].state == PERSON && gm->person[k].state == ZOMBIE){					
					gm->person[i].timer = MAX_TIME;
					gm->person[i].state = P_Z;
				}
				if(gm->person[i].state == ZOMBIE && gm->person[k].state == PERSON){
					gm->person[k].timer = MAX_TIME;
					gm->person[k].state = P_Z;
				}
				if((i == gm->hero.person_id &&
					gm->person[i].state == P_Z &&
					gm->hero.spring_state == ATTACHED) ||
					(k == gm->hero.person_id &&
					gm->person[k].state == P_Z &&
					gm->hero.spring_state == ATTACHED)){
						gm->hero.spring_state = NOT_ATTACHED;
					}
			}
		}
	}	

	/*Zero forces on all objects*/
	for(i=0; i < gm->person_num; i++){
		gm->person[i].o.f.x = 0;
		gm->person[i].o.f.y = 0;
	}
	gm->hero.o.f.x=0;
	gm->hero.o.f.y=0;
	
	/*Add forces*/
	gm->hero.o.f.x += gm->ak.x*100 - gm->hero.o.v.x;
	gm->hero.o.f.y += gm->ak.y*100 - gm->hero.o.v.y;
	
	/*Slows a person down when they are in the safe zone or if they are turning into a zombie*/
	for(i = 0; i < gm->person_num; i++){
		if(gm->person[i].state == SAFE ||gm->person[i].state == P_Z){
			gm->person[i].o.f.x +=  -gm->person[i].o.v.x;
			gm->person[i].o.f.y +=  -gm->person[i].o.v.y;
		}
	}
	
	/*Spring force between hero and person*/
	if(gm->hero.spring_state == ATTACHED){
		float ks = 50, r = 5, kd=1;
		i = gm->hero.person_id;
		vector2 l = v2Sub(gm->hero.o.p, gm->person[i].o.p);
		vector2 dl = v2Sub(gm->hero.o.v, gm->person[i].o.v);
		
		vector2 fa, fb;
		fa = v2sMul(-ks*(v2Len(l)-r) - kd * v2Dot(dl, l)/v2Len(l) , v2Unit(l));
		fb = v2sMul(-1,fa);
    	
		gm->hero.o.f = v2Add(gm->hero.o.f, fa);
		gm->person[i].o.f = v2Add(gm->person[i].o.f, fb);
		
		gm->person[i].o.f.x +=  -gm->person[i].o.v.x;
		gm->person[i].o.f.y +=  -gm->person[i].o.v.y;
	}

	/*Integrate*/
	gm->hero.o.v.x += gm->hero.o.f.x/gm->hero.o.m*dt;
	gm->hero.o.v.y += gm->hero.o.f.y/gm->hero.o.m*dt;
	gm->hero.o.p.x += gm->hero.o.v.x*dt;
	gm->hero.o.p.y += gm->hero.o.v.y*dt;

	for(i=0; i < gm->person_num; i++){
		gm->person[i].o.v.x += gm->person[i].o.f.x/gm->person[i].o.m*dt;
		gm->person[i].o.v.y += gm->person[i].o.f.y/gm->person[i].o.m*dt;

		gm->person[i].o.p.x += gm->person[i].o.v.x*dt;
		gm->person[i].o.p.y += gm->person[i].o.v.y*dt;
	}
}

void gm_render(game gm){
	int i;
	glColor3f(0.8,0.8,0.8);
	
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture( GL_TEXTURE_2D, gm->bk);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(0,0);
	glTexCoord2f(0.0, 1.0); glVertex2f(0, gm->h);
	glTexCoord2f(1.0, 1.0); glVertex2f(gm->w, gm->h);
	glTexCoord2f(1.0, 0.0); glVertex2f(gm->w, 0);
	glEnd();
	glPopMatrix();
		
	glBindTexture( GL_TEXTURE_2D, gm->safezone_tex);
	glPushMatrix();
	glTranslatef(gm->safe_zone.p.x, gm->safe_zone.p.y, 0);
	glScalef(gm->safe_zone.r, gm->safe_zone.r,0);
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
	
	if(gm->hero.spring_state == ATTACHED){
		i = gm->hero.person_id;
		glBindTexture( GL_TEXTURE_2D, gm->rope_tex);
		glPushMatrix();
		glBegin(GL_QUADS);
		
		vector2 p1,p2;
		p1 = v2Add(gm->person[i].o.p, v2sMul(gm->person[i].o.r, v2Unit(v2Sub(gm->hero.o.p, gm->person[i].o.p))));
		p2 = v2Add(gm->hero.o.p, v2sMul(gm->hero.o.r, v2Unit(v2Sub(gm->person[i].o.p, gm->hero.o.p))));

		glTexCoord2f(1.0, 0.0);
		vector2 temp = v2Add(p1, v2sMul(2.0, v2Unit(v2Rotate(M_PI/2, v2Sub(gm->person[i].o.p, gm->hero.o.p)))));
		glVertex3f(temp.x, temp.y, 0.0);

		glTexCoord2f(0.0, 0.0);
		temp = v2Add(p1, v2sMul(2.0, v2Unit(v2Rotate(-M_PI/2, v2Sub(gm->person[i].o.p, gm->hero.o.p)))));
		glVertex3f(temp.x, temp.y, 0.0);

		glTexCoord2f(0.0, 1.0);
		temp = v2Add(p2, v2sMul(2.0, v2Unit(v2Rotate(-M_PI/2, v2Sub(gm->person[i].o.p, gm->hero.o.p)))));
		glVertex3f(temp.x, temp.y, 0.0);

		glTexCoord2f(1.0, 1.0);
		temp = v2Add(p2, v2sMul(2.0, v2Unit(v2Rotate(M_PI/2, v2Sub(gm->person[i].o.p, gm->hero.o.p)))));
		glVertex3f(temp.x, temp.y, 0.0);

		glEnd();
		glPopMatrix();
	}
	
	switch(gm->hero.state){
		case PERSON:
			glBindTexture( GL_TEXTURE_2D, gm->hero_tex);
			break;
		case P_Z:
			glBindTexture( GL_TEXTURE_2D, gm->h_z_tex);
			break;
		case ZOMBIE:
			glBindTexture( GL_TEXTURE_2D, gm->hzombie_tex);
			break;
		case SAFE:
		case DONE:
			glBindTexture( GL_TEXTURE_2D, gm->herosafe_tex);
			break;
	}
	if(gm->hero.spring_state == ATTACHED){
		glBindTexture( GL_TEXTURE_2D, gm->heroattached_tex);
	}
	
	
	glPushMatrix();
	glTranslatef(gm->hero.o.p.x, gm->hero.o.p.y, 0);
	glScalef(gm->hero.o.r, gm->hero.o.r,0);
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

	for(i=0; i<gm->person_num; i++){
		if(gm->person[i].state == PERSON){
			glBindTexture( GL_TEXTURE_2D, gm->person_tex);
		}
		else if(gm->person[i].state == P_Z){
			glBindTexture( GL_TEXTURE_2D, gm->p_z_tex);
		}
		else if(gm->person[i].state == ZOMBIE){
			glBindTexture( GL_TEXTURE_2D, gm->zombie_tex);
		}
		
		else if(gm->person[i].state == SAFE){
			glBindTexture( GL_TEXTURE_2D, gm->safe_tex);
		}
		glPushMatrix();
		glTranslatef(gm->person[i].o.p.x, gm->person[i].o.p.y, 0);
		glScalef(gm->person[i].o.r, gm->person[i].o.r,0);
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

int gm_progress(game gm){
	int i, add = 0;
	if(gm->hero.state == ZOMBIE){
		return -1;
	}
	for(i = 0; i < gm->person_num; i++){
		if(gm->person[i].state == SAFE){
			add++;
		}
	}
	if(add >= gm->save_count && gm->hero.state == SAFE){
		gm->hero.state = DONE;
		return 1;
	}
	
	return 0;
}

void gm_free(game gm){

	free(gm);
}

void gm_skey_down(game gm, int key){
	switch(key) {
		case GLUT_KEY_LEFT : 
			gm->ak.x = -1;
			break;
		case GLUT_KEY_RIGHT : 
			gm->ak.x = 1;
			break;
		case GLUT_KEY_UP : 
			gm->ak.y = 1;
			break;
		case GLUT_KEY_DOWN : 
			gm->ak.y = -1;
			break;
	}
}

void gm_skey_up(game gm, int key){
	switch (key) {
		case GLUT_KEY_LEFT : 
			if(gm->ak.x < 0) {gm->ak.x = 0;}
			break;
		case GLUT_KEY_RIGHT: 
			if(gm->ak.x > 0) {gm->ak.x = 0;}
			break;
		case GLUT_KEY_UP : 
			if(gm->ak.y > 0) {gm->ak.y = 0;}
			break;
		case GLUT_KEY_DOWN : 
			if(gm->ak.y < 0) {gm->ak.y = 0;}
			break;
	}
}

vector2 gm_dim(game gm){
	vector2 dim = gm->hero.o.p;
	return dim;
}

int collision(object *ta, object *tb){
	object a=*ta, b=*tb;
	
	if(v2SPow(v2Sub(a.p, b.p)) < (a.r+b.r)*(a.r+b.r)) {
		vector2 n, vn1, vn1f, vn2, vn2f, vt1, vt2;
		float m1 = a.m, m2 = b.m;
		
		n = v2Unit(v2Sub(a.p, b.p)); //n = (r1-r2)/|r1-r2|
		
		ta->p = v2Add(a.p, v2sMul( (a.r+b.r-v2Len(v2Sub(a.p, b.p)))*b.m/(a.m+b.m) , n) );
		tb->p = v2Sub(b.p, v2sMul( (a.r+b.r-v2Len(v2Sub(a.p, b.p)))*a.m/(a.m+b.m) , n) );
		
		vn1 = v2sMul(v2Dot(a.v, v2Neg(n)), v2Neg(n)); // vn1 = [v1⋄(-n)](-n)
		vn2 = v2sMul(v2Dot(b.v, n), n); // vn2 = [v2⋄n]n
		
		vt1 = v2Sub(vn1, a.v); // vt1 = vn1 - v1
		vt2 = v2Sub(vn2, b.v); // vt2 = vn2 - v2
		
		vn1f = v2sMul(1/(m1+m2), v2Add(v2sMul(m1-m2, vn1), v2sMul(2*m2, vn2))); //v1f = (v1i*(m1-m2)+2*m2*v2i)/(m1+m2)
		vn2f = v2sMul(1/(m1+m2), v2Add(v2sMul(m2-m1, vn2), v2sMul(2*m1, vn1))); //v1f = (v2i*(m2-m1)+2*m1*v1i)/(m1+m2)
		
		ta->v = v2Sub(vn1f, vt1);  
		tb->v = v2Sub(vn2f, vt2);
		return 1;
	}
	else{return 0;}
}

int collision_test(object ta, object tb){
	object a=ta, b=tb;
	
	if(v2SPow(v2Sub(a.p, b.p)) < (a.r+b.r)*(a.r+b.r)) {
		return 1;
	}
	return 0;
}

int safe_zone_test(object ta, object tb){
	object a=ta, b=tb;
	
	if(v2SPow(v2Sub(a.p, b.p)) < (a.r-b.r)*(a.r-b.r)) {
		return 1;
	}
	return 0;
}

int r_collision(object *ta, object *tb){
	object a=*ta, b=*tb;
	a.r -= 2*b.r;
	
	if(v2SPow(v2Sub(a.p, b.p)) > (a.r + b.r)*(a.r + b.r)) {
		vector2 n, vn1, vn1f, vn2, vn2f, vt1, vt2;
		float m1 = a.m, m2 = b.m;
		
		n = v2Unit(v2Sub(a.p, b.p)); //n = (r1-r2)/|r1-r2|
		
		ta->p = v2Add(a.p, v2sMul( (a.r+b.r-v2Len(v2Sub(a.p, b.p)))*b.m/(a.m+b.m) , n) );
		tb->p = v2Sub(b.p, v2sMul( (a.r+b.r-v2Len(v2Sub(a.p, b.p)))*a.m/(a.m+b.m) , n) );
		
		vn1 = v2sMul(v2Dot(a.v, v2Neg(n)), v2Neg(n)); // vn1 = [v1⋄(-n)](-n)
		vn2 = v2sMul(v2Dot(b.v, n), n); // vn2 = [v2⋄n]n
		
		vt1 = v2Sub(vn1, a.v); // vt1 = vn1 - v1
		vt2 = v2Sub(vn2, b.v); // vt2 = vn2 - v2
		
		vn1f = v2sMul(1/(m1+m2), v2Add(v2sMul(m1-m2, vn1), v2sMul(2*m2, vn2))); //v1f = (v1i*(m1-m2)+2*m2*v2i)/(m1+m2)
		vn2f = v2sMul(1/(m1+m2), v2Add(v2sMul(m2-m1, vn2), v2sMul(2*m1, vn1))); //v1f = (v2i*(m2-m1)+2*m1*v1i)/(m1+m2)
		
		ta->v = v2Sub(vn1f, vt1);  
		tb->v = v2Sub(vn2f, vt2);
		return 1;
	}
	else{return 0;}
}

void circle(float pos_x, float pos_y, float size) {
	glPushMatrix();
	glTranslatef(pos_x,pos_y,0);
	float counter;
	glBegin(GL_POLYGON);
	for (counter = 0; counter <= 2*3.14159; counter = counter + 3.14159/8) {
		glVertex3f ((size)*cos(counter), (size)*sin(counter), 0.0);
	}
  	glEnd();
	glPopMatrix();
}

int bounce(object * obj, int x, int y){
	int state = 0;
	if(obj->p.x < obj->r){
		obj->p.x = obj->r;
		obj->v.x *= -1;
		state = 1;
	}
	if(obj->p.x > x - obj->r){
		obj->p.x = x - obj->r;
		obj->v.x *= -1;
		state = 1;
	}
	if(obj->p.y < obj->r){
		obj->p.y = obj->r;
		obj->v.y *= -1;
		state = 1;
	}
	if(obj->p.y > y - obj->r){
		obj->p.y = y - obj->r;
		obj->v.y *= -1;
		state = 1;
	}
	return state;
}

int load_level_file(game gm, char * file){
		int i,j;
		FILE *loadFile;

		loadFile = fopen(file, "r");

		if(loadFile == NULL)
		{
			printf("File %s unable to load.\n", file); 
			return 0;
		}
		
		char type[5];
		int result;
		
		gm->safe_zone.m = 5000;
		gm->person_num = 0;
		i = 1;
		while( (result = fscanf(loadFile, "%s", type)) != EOF){
			if(!strncmp(type, "hw", 2)){
				result = fscanf(loadFile, "%d %d", &gm->h, &gm->w);
				if(result != 2){
					printf("Error loading level file %s on line %d.\n", file, i);
					printf("Error: Height and width.\n");
				}
			}
			else if(!strncmp(type, "sz", 2)){
				result = fscanf(loadFile, "%lf %lf %lf", 
					&gm->safe_zone.p.x,
					&gm->safe_zone.p.y,
					&gm->safe_zone.r);
				if(result != 3){
					printf("Error loading level file %s on line %d.\n", file, i);
					printf("Error: Safe Zone not loaded.\n");
				}
			}
			else if(!strncmp(type, "sc", 2)){
				result = fscanf(loadFile, "%d", &gm->save_count);
				if(result != 1){
					printf("Error loading level file %s on line %d.\n", file, i);
					printf("Error: Save count not loaded.\n");
				}
			}
			else if(!strncmp(type, "p", 1)){
				int num = gm->person_num;
				gm->person[num].state = PERSON;
				result = fscanf(loadFile, "%lf %lf %lf %lf %lf %lf", 
					&gm->person[num].o.p.x,
					&gm->person[num].o.p.y,
					&gm->person[num].o.v.x,
					&gm->person[num].o.v.y,
					&gm->person[num].o.r,
					&gm->person[num].o.m);
				if(result != 6){
					printf("Error loading level file %s on line %d.\n", file, i);
					printf("Error: A person was not loaded.\n");
				}
				else{
					gm->person_num++;
				}
			}
			else if(!strncmp(type, "z", 1)){
				int num = gm->person_num;
				gm->person[num].state = ZOMBIE;
				result = fscanf(loadFile, "%lf %lf %lf %lf %lf %lf", 
					&gm->person[num].o.p.x,
					&gm->person[num].o.p.y,
					&gm->person[num].o.v.x,
					&gm->person[num].o.v.y,
					&gm->person[num].o.r,
					&gm->person[num].o.m);
				if(result != 6){
					printf("Error loading level file %s on line %d.\n", file, i);
					printf("Error: A zombie was not loaded.\n");
				}
				else{
					gm->person_num++;
				}
			}
			else if(!strncmp(type, "h", 1)){
				gm->hero.state = PERSON;
				gm->hero.spring_state = NOT_ATTACHED;
				result = fscanf(loadFile, "%lf %lf %lf %lf %lf %lf", 
					&gm->hero.o.p.x,
					&gm->hero.o.p.y,
					&gm->hero.o.v.x,
					&gm->hero.o.v.y,
					&gm->hero.o.r,
					&gm->hero.o.m);
				if(result != 6){
					printf("Error loading level file %s on line %d.\n", file, i);
					printf("Error: The hero was not loaded!\n");
				}
			}
			i++;
		}
		gm_set_view(gm);
		gm_update_view(gm);
		fclose(loadFile);
		return 1;
}