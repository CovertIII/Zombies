#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <GLUT/glut.h>
#include "load_sound.h"
#include "vector2.h"
#include "sound_list.h"
#include "test.h"
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

typedef struct {
	object o;
	float timer;
	int state; 
} _zombie;



typedef struct gametype {
	int h,w;
	
	ppl person[100];
	person_num;
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
	switch(lvl){
		case 1:
			gm->h = 100;
			gm->w = 100;
			
			gm->safe_zone.p.x = rand()%(gm->w - SAFE_ZONE_RAD);
			gm->safe_zone.p.y = rand()%(gm->h - SAFE_ZONE_RAD);
			gm->safe_zone.r = SAFE_ZONE_RAD;
			
			person_num = 10;
			for(i=0; i<person_num; i++){
				if(i < 3){
					gm->person[i].state = ZOMBIE;
				}
				else{
					gm->person[i].state = PERSON;
				}
				gm->person[i].o.r = PERSON_RAD;
				
				float dist;
				do{
					gm->person[i].o.p.x = rand()%gm->w;
					gm->person[i].o.p.y = rand()%gm->y;
				}while(v2Len(v2Sub(gm->person[i].o.p, gm->save_zone.p)) < SAFE_ZONE_RAD + PERSON_RAD);
				
				gm->person[i].o.v.x = rand()%50 - 25;
				gm->person[i].o.v.y = rand()%50 - 25;
			}
		
	}
	
}

void gm_add_sound(game gm){
	int i = gm->src_num - 1;
	alGenSources(1, &gm->src_z[i]);
	alSourcei(gm->src_z[i], AL_BUFFER, gm->buf_z);
		
	alSourcef(gm->src_z[i], AL_PITCH, 1.0f);
	alSourcef(gm->src_z[i], AL_GAIN, 1.0f);
	
	alSourcei(gm->src_z[i],AL_LOOPING,AL_TRUE);
	
	alSource3f(gm->src_z[i], AL_POSITION, gm->src[i].p.x, gm->src[i].p.y, 0);
	alSource3f(gm->src_z[i], AL_VELOCITY, gm->src[i].v.x, gm->src[i].v.y, 0);
	
	alSourcePlay(gm->src_z[i]);
}

void gm_update_sound(game gm){
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
}

void gm_update(game gm, double dt){
	int i;

	for(i=0; i<gm->src_num; i++){
		gm->src[i].v.x += 0;
		gm->src[i].v.y += 0;
		gm->src[i].p.x += gm->src[i].v.x*dt;
		gm->src[i].p.y += gm->src[i].v.y*dt;
	}
	
	gm->food.v.x += (-gm->food.v.x*4/gm->food.m + 2*gm->food.f.x)*dt;
	gm->food.v.y += (-gm->food.v.y*4/gm->food.m + 2*gm->food.f.y)*dt;
	gm->food.p.x += gm->food.v.x*dt;
	gm->food.p.y += gm->food.v.y*dt;


	gm->lst.v.x += (-gm->lst.v.x*4/gm->lst.m + 2*gm->lst.f.x)*dt;
	gm->lst.v.y += (-gm->lst.v.y*4/gm->lst.m + 2*gm->lst.f.y)*dt;
	gm->lst.p.x += gm->lst.v.x*dt;
	gm->lst.p.y += gm->lst.v.y*dt;
}

void gm_collision(game gm){
	int i;
	for(i = 0; i < gm->src_num; i++){
		if(bounce(&gm->src[i], gm->w, gm->h)){
			s_add_snd(gm->clicks, gm->src[i].p);
		}
		int j;
		for(j = i+1; j<gm->src_num; j++){
			if(collision(&gm->src[i], &gm->src[j])){
				s_add_snd(gm->clicks, gm->src[i].p);
			}
		}
		if(collision(&gm->src[i], &gm->food)){
			s_add_snd(gm->clicks, gm->food.p);
		}
	}

	bounce(&gm->food, gm->w, gm->h);

	if(bounce(&gm->lst, gm->w, gm->h)){
		s_add_snd(gm->clicks, gm->lst.p);
	}
}

int gm_logic_update(game gm){
	int w = gm->w;
	int h = gm->h;
	int i;
	for(i=0; i<gm->src_num; i++){
		if(collision(&gm->src[i], &gm->lst)){
			alSourcePlay(gm->die_src);
			return 1;
		}
	}

	if(collision_test(gm->food, gm->lst)){
		alSourcePlay(gm->eat_src);
		alSource3f(gm->eat_src, AL_POSITION, gm->food.p.x, gm->food.p.y, 0);
		gm->src_num++;
		gm_add_sound(gm);
		float dist;
		do{
			gm->src[i].p.x = rand()%w;
			gm->src[i].p.y = rand()%h;
			dist = v2Len(v2Sub(gm->src[i].p, gm->lst.p));
		}while(dist < 20);
		gm->src[i].v.x = rand()%50 - 25;
		gm->src[i].v.y = rand()%50 - 25;
		gm->src[i].m = 2;
		gm->src[i].r = 1.5;

		gm->food.p.x = rand()%(int)(w-2*gm->food.r)+gm->food.r;
		gm->food.p.y = rand()%(int)(h-2*gm->food.r)+gm->food.r;
		gm->food.v.x = 0;
		gm->food.v.y = 0;
	}
	return 0;
}

int gm_score(game gm){
	return gm->src_num;
}

void gm_render(game gm){
	int i;

	glBindTexture( GL_TEXTURE_2D, gm->bk_p);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(0,0);
	glTexCoord2f(0.0, 1.0); glVertex2f(0, gm->h);
	glTexCoord2f(1.0, 1.0); glVertex2f(gm->w, gm->h);
	glTexCoord2f(1.0, 0.0); glVertex2f(gm->w, 0);
	glEnd();
	glPopMatrix();
	glBindTexture( GL_TEXTURE_2D, gm->src_p);
	 
	for(i = 0; i<gm->src_num; i++){
		glPushMatrix();
		glTranslatef(gm->src[i].p.x, gm->src[i].p.y, 0);
		glScalef(gm->src[i].r,gm->src[i].r,0);
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


	glBindTexture( GL_TEXTURE_2D, gm->lst_p);
	glPushMatrix();
	glTranslatef(gm->lst.p.x, gm->lst.p.y, 0);
	glScalef(gm->lst.r,gm->lst.r,0);
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


	glBindTexture( GL_TEXTURE_2D, gm->food_p);
	glPushMatrix();
	glTranslatef(gm->food.p.x, gm->food.p.y, 0);
	glScalef(gm->food.r,gm->food.r,0);
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

void gm_free(game gm){
	s_free(gm->clicks);
	alDeleteSources(gm->src_num, gm->src_z);
	alDeleteSources( 1, &gm->die_src);
	alDeleteSources( 1, &gm->eat_src);
	alDeleteBuffers( 1, &gm->die_buf);
	alDeleteBuffers( 1, &gm->buf_z);
	alDeleteBuffers( 1, &gm->eat_buf);
	alDeleteBuffers( 1, &gm->buf_clk);

	glDeleteTextures(1, &gm->food_p);
	glDeleteTextures(1, &gm->lst_p);;
	glDeleteTextures(1, &gm->bk_p);
	glDeleteTextures(1, &gm->src_p);
	free(gm);
}


void gm_skey_down(game gm, int key){
	int force = 100;
	switch(key) {
		case GLUT_KEY_LEFT : 
			gm->lst.f.x=-force;
			break;
		case GLUT_KEY_RIGHT : 
			gm->lst.f.x= force;
			break;
		case GLUT_KEY_UP : 
			gm->lst.f.y= force;
			break;
		case GLUT_KEY_DOWN : 
			gm->lst.f.y=-force;
			break;
	}
}

void gm_skey_up(game gm, int key){
	switch (key) {
		case GLUT_KEY_LEFT : 
			if(gm->lst.f.x < 0) {gm->lst.f.x = 0;}
			break;
		case GLUT_KEY_RIGHT: 
			if(gm->lst.f.x > 0) {gm->lst.f.x = 0;}
			break;
		case GLUT_KEY_UP : 
			if(gm->lst.f.y > 0) {gm->lst.f.y = 0;}
			break;
		case GLUT_KEY_DOWN : 
			if(gm->lst.f.y < 0) {gm->lst.f.y = 0;}
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
