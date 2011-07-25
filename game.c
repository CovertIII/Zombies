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

#define ATTACHED 0
#define NOT_ATTACHED 1

#define MAX_TIME 5
#define SAFE_ZONE_RAD 10
#define PERSON_RAD 2
#define PERSON_MASS 0.5


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
	int h,w;
	vector2 ak;  /*arrow key presses*/
	
	ppl person[100];
	int person_num;
	_hero hero;
	object safe_zone;
	
} gametype;

static void circle(float pos_x, float pos_y, float size);
static int bounce(object * obj, int x, int y);
static int collision(object *ta, object *tb);
static int collision_test(object ta, object tb);

game gm_init(void){
	gametype * gm;
	gm = (game)malloc(sizeof(gametype));
	if(!gm) {return NULL;}

	return gm;
}

void gm_load_level(game gm, int lvl){
	int i;
	gm->ak.x =0;
	gm->ak.y = 0;
	switch(lvl){
		case 1:
			gm->h = 100;
			gm->w = 100;
			
			gm->safe_zone.p.x = rand()%(gm->w - SAFE_ZONE_RAD);
			gm->safe_zone.p.y = rand()%(gm->h - SAFE_ZONE_RAD);
			gm->safe_zone.r = SAFE_ZONE_RAD;
			gm->safe_zone.m = 1000000;
			
			gm->person_num = 10;
			for(i=0; i<gm->person_num; i++){
				if(i < 3){
					gm->person[i].state = ZOMBIE;
				}
				else{
					gm->person[i].state = PERSON;
				}
				gm->person[i].o.r = PERSON_RAD;
				gm->person[i].o.m = PERSON_MASS;
				
				float dist;
				do{
					gm->person[i].o.p.x = rand()%gm->w;
					gm->person[i].o.p.y = rand()%gm->h;
				}while(v2Len(v2Sub(gm->person[i].o.p, gm->safe_zone.p)) < SAFE_ZONE_RAD + PERSON_RAD);
				
				gm->person[i].o.v.x = rand()%50 - 25;
				gm->person[i].o.v.y = rand()%50 - 25;
			}

			gm->hero.o.p = gm->safe_zone.p;
			gm->hero.o.v.x = 0;
			gm->hero.o.v.y = 0;
			gm->hero.o.r = PERSON_RAD;
			gm->hero.o.m = PERSON_MASS;
			gm->hero.state = PERSON;
			gm->hero.spring_state = NOT_ATTACHED;
	}
	
}

void gm_add_sound(game gm){
	/*
	int i = gm->src_num - 1;
	alGenSources(1, &gm->src_z[i]);
	alSourcei(gm->src_z[i], AL_BUFFER, gm->buf_z);
		
	alSourcef(gm->src_z[i], AL_PITCH, 1.0f);
	alSourcef(gm->src_z[i], AL_GAIN, 1.0f);
	
	alSourcei(gm->src_z[i],AL_LOOPING,AL_TRUE);
	
	alSource3f(gm->src_z[i], AL_POSITION, gm->src[i].p.x, gm->src[i].p.y, 0);
	alSource3f(gm->src_z[i], AL_VELOCITY, gm->src[i].v.x, gm->src[i].v.y, 0);
	
	alSourcePlay(gm->src_z[i]);
	*/
}

void gm_update_sound(game gm){
	/*
	s_update(gm->clicks);

	int i;
	for(i = 0; i < gm->src_num; i++){
		alSource3f(gm->src_z[i], AL_POSITION, gm->src[i].p.x, gm->src[i].p.y, 0);
		alSource3f(gm->src_z[i], AL_VELOCITY, gm->src[i].v.x, gm->src[i].v.y, 0);
		
	}
	
	alSource3f(gm->eat_src, AL_POSITION, gm->lst.p.x, gm->lst.p.y, 0);
	alSource3f(gm->eat_src, AL_VELOCITY, gm->lst.v.x, gm->lst.v.y, 0);
	
	alListener3f(AL_POSITION, gm->lst.p.x, gm->lst.p.y, 0);
	alListener3f(AL_VELOCITY, gm->lst.v.x, gm->lst.v.y, 0);
	*/
}

void gm_update(game gm, double dt){
	int i, k;
	/*Wall Colisions*/
	for(i = 0; i < gm->person_num; i++){
		bounce(&gm->person[i].o, gm->w, gm->h);
	}	
	bounce(&gm->hero.o, gm->w, gm->h);

	/*Circle Collisons*/
	for(i = 0; i < gm->person_num; i++){
		collision(&gm->person[i].o, &gm->hero.o);

		vector2 p = gm->safe_zone.p;
		collision(&gm->person[i].o, &gm->safe_zone);
		gm->safe_zone.p = p;
		for(k = 1+i; k < gm->person_num; k++){
			collision(&gm->person[i].o, &gm->person[k].o);
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
	gm->hero.o.f.x = gm->ak.x*100 - gm->hero.o.v.x;
	gm->hero.o.f.y = gm->ak.y*100 - gm->hero.o.v.y;

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
	glColor3f(0,0,1);
	circle(gm->safe_zone.p.x, gm->safe_zone.p.y, gm->safe_zone.r);

	glColor3f(1,0,0);
	circle(gm->hero.o.p.x, gm->hero.o.p.y, gm->hero.o.r);

	int i;
	for(i=0; i<gm->person_num; i++){
		if(gm->person[i].state == PERSON){
			glColor3f(1,1,1);
		}
		else{
			glColor3f(0,1,0);
		}
		circle(gm->person[i].o.p.x, gm->person[i].o.p.y, gm->person[i].o.r);
	}

	
}

int gm_progress(game gm){
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
	vector2 dim;
	dim.x = gm->w;
	dim.y = gm->h;
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
