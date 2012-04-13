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
#include "load_sound.h"
#include "vector2.h"
#include "physics.h"
#include "load_png.h"
#include "sound_list.h"
#include "ai.h"
#include "game.h"

#define ZOMBIE 0
#define Z_P 1
#define PERSON 2
#define P_Z 3
#define SAFE 4
#define DONE 5

#define STINK_PARTICLE_NUM 6 

//Different Emotional states
#define NORMAL 0
#define SCARED 1

#define ATTACHED 0
#define NOT_ATTACHED 1

#define PERSON_RAD 2
#define PERSON_MASS 0.5

#define VIEWRATIO 100
#define MAX_TIME 3

enum{
    al_saved1_buf,
	al_saved2_buf,
    al_wall_buf,
    al_pwall_buf,
    al_scared_buf,
    al_pdeath_buf,
    al_attached1_buf,
	al_attached2_buf,
	al_attached3_buf,
    al_hdeath_buf,
    al_p_z_buf,
    al_buf_num
};

typedef struct {
	object o;
	float timer;
	int state; 
	int emo; 
	int ready;
    double mx_f;

    int chase;
    int parent_id;
} ppl;

typedef struct {
	object o;
	
	float timer;
	int state; 
	
	int spring_state;
	int person_id;
} _hero;

typedef struct {
    object n[STINK_PARTICLE_NUM];
    double time[STINK_PARTICLE_NUM];
    int id;
} stink_struct;

typedef struct gametype {
	char * res_path;
	char * res_buf;
	
    vector2 ms;  /*mouse location*/
	int m;		 /*Whether to use mouse or not */
	vector2 ak;  /*arrow key presses*/
	
	vector2 vmin; /*sceen size */
	vector2 vmax; /*sceen size */
	int screenx;
    int screeny;
	double viewratio;
	int zoom;
	
	
	double timer;
	double total_time;

    int c; //Varibale to keep track if the c key is being pressed down.
	
	double h,w; //Height and width of the level
	ppl person[100]; //Zombies and people array
	int person_num; //How many there are
	_hero hero;
	int ppl_chain[100];
	int chain_num;
	object safe_zone;
	int save_count;  //How many people you have to save to win the level
	line walls[100];
	int wall_num;

    vector2 tp;
	

    //Experiental Particle thiny for zombies
    stink_struct stnk[100];
    int stnk_num;

    int gm_state;

	ALuint buf[al_buf_num];
    s_list saved_src;
	
	GLuint zombie_tex;
	GLuint person_tex;
	GLuint safe_tex;
	GLuint eye_tex;
	GLuint safezone_tex;
	GLuint hero_tex;
	GLuint heroattached_tex;
	GLuint herosafe_tex;
	GLuint person_s_tex;
	GLuint p_z_tex;
	GLuint h_z_tex;
	GLuint hzombie_tex;
	GLuint bk;
	GLuint rope_tex;
	GLuint blank_tex;
    GLuint extra_tex;
    GLuint stink_tex;

    rat_font * font;
	
} gametype;

int load_level_file(game gm, char * file);
void chain_remove(game gm, int index);
void chain_ready_zero(game gm);
void chain_cut(game gm, int index);
void stink_add(game gm, int id);
void stink_step(game gm, double dt);
void stink_render(game gm);
void zb_chase_hero(game gm);


game gm_init(char * res_path){
  gametype * gm;
  gm = (game)malloc(sizeof(gametype));
  if(!gm) {return NULL;}

  gm->res_path = (char*)malloc(strlen(res_path)*sizeof(char)+sizeof(char)*50);
  gm->res_buf  = (char*)malloc(strlen(res_path)*sizeof(char)+sizeof(char)*50);
  strcpy(gm->res_path, res_path);
  printf("game.c - path mem: %ld\n", strlen(res_path)*sizeof(char)+sizeof(char)*50);
	
  gm->font = rat_init();
  strcpy(gm->res_buf, gm->res_path);
  strcat(gm->res_buf, "/imgs/FreeUniversal.ttf");
  rat_load_font(gm->font, gm->res_buf, 28);
	
  gm->viewratio = VIEWRATIO;
  gm->zoom = 0;
  gm->timer = 0;
  gm->c=0;
  gm->m=0;

  return gm;
}

int gm_init_textures(game gm){
    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/zombie.png");
    load_texture(gm->res_buf, &gm->zombie_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/person_s.png");
    load_texture(gm->res_buf, &gm->person_s_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/person.png");
    load_texture(gm->res_buf, &gm->person_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/safe.png");
    load_texture(gm->res_buf, &gm->safe_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/eye.png");
    load_texture(gm->res_buf, &gm->eye_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/safezone.png");
    load_texture(gm->res_buf, &gm->safezone_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/hero.png");
    load_texture(gm->res_buf, &gm->hero_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/p_z.png");
    load_texture(gm->res_buf, &gm->p_z_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/h_z.png");
    load_texture(gm->res_buf, &gm->h_z_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/hzombie.png");
    load_texture(gm->res_buf, &gm->hzombie_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/heroattached.png");
    load_texture(gm->res_buf, &gm->heroattached_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/herosafe.png");
    load_texture(gm->res_buf, &gm->herosafe_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/rope.png");
    load_texture(gm->res_buf, &gm->rope_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/blank.png");
    load_texture(gm->res_buf, &gm->blank_tex);


    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/extra.png");
    load_texture(gm->res_buf, &gm->extra_tex);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/imgs/stink.png");
    load_texture(gm->res_buf, &gm->stink_tex);
}	

void gm_init_sounds(game gm){
	alGenBuffers(al_buf_num, gm->buf);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf,"/snd/saved1.ogg"); 
	  snd_load_file(gm->res_buf, gm->buf[al_saved1_buf]);
	
    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/snd/saved2.ogg");
    snd_load_file(gm->res_buf, gm->buf[al_saved2_buf]);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/snd/wall.ogg");
    snd_load_file(gm->res_buf, gm->buf[al_wall_buf]);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/snd/pdeath.ogg");
    snd_load_file(gm->res_buf, gm->buf[al_pdeath_buf]);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/snd/attached1.ogg");
    snd_load_file(gm->res_buf, gm->buf[al_attached1_buf]);
	
    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/snd/attached2.ogg");
    snd_load_file(gm->res_buf, gm->buf[al_attached2_buf]);
	
    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/snd/attached3.ogg");
    snd_load_file(gm->res_buf, gm->buf[al_attached3_buf]);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/snd/scared.ogg");
    snd_load_file(gm->res_buf, gm->buf[al_scared_buf]);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/snd/p_z.ogg");
    snd_load_file(gm->res_buf, gm->buf[al_p_z_buf]);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/snd/hdeath.ogg");
    snd_load_file(gm->res_buf, gm->buf[al_hdeath_buf]);

    strcpy(gm->res_buf, gm->res_path);
    strcat(gm->res_buf, "/snd/pwall.ogg");
    snd_load_file(gm->res_buf, gm->buf[al_pwall_buf]);

	gm->saved_src = s_init();

	ALfloat	listenerOri[]={0.0,1.0,0.0, 0.0,0.0,1.0};
	alListenerfv(AL_ORIENTATION,listenerOri);
	alListenerf(AL_GAIN,12);
	alListener3f(AL_POSITION, gm->hero.o.p.x, 0, gm->hero.o.p.y);
	alListener3f(AL_VELOCITY, 0, 0, 0);
}

void gm_set_view(int width, int height, game gm){
	double ratio = (double)width/(double)height; 
	vector2 w = {gm->viewratio, gm->viewratio};
	w.x = ratio * w.y;
	gm->vmin.x = gm->hero.o.p.x - w.x/2.0f;
	gm->vmin.y = gm->hero.o.p.y - w.y/2.0f;
	gm->vmax.x = gm->hero.o.p.x + w.x/2.0f;
	gm->vmax.y = gm->hero.o.p.y + w.y/2.0f;
	
	gm->screenx = width;
    gm->screeny = height;
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
	vector2 w = {gm->viewratio, gm->viewratio};
	w.x = ratio * w.y;
	double diff = w.x - (gm->vmax.x - gm->vmin.x);
	gm->vmax.x += diff;
    glLoadIdentity();
    gluOrtho2D(gm->vmin.x, gm->vmax.x, gm->vmin.y, gm->vmax.y);
    glMatrixMode(GL_MODELVIEW);
}

int gm_load_level(game gm, char * lvl){
	gm->ak.x =0;
	gm->ak.y = 0;

	return load_level_file(gm, lvl);
}

void gm_update_sound(game gm){
	
}

void gm_update(game gm, int width, int height, double dt){
	int i, k;
	/*Timers */
	if (dt > 0.1f){dt = 0.01;}

    stink_step(gm, dt);	

	if(gm->gm_state == 0){
		gm->timer += dt;
	}
	
	for(i = 0; i < gm->person_num; i++){
		gm->person[i].timer -= dt;
		if(gm->person[i].state == P_Z && gm->person[i].timer <= 0.0f){
			gm->person[i].state = ZOMBIE;
            stink_add(gm, i);
            s_add_snd(gm->saved_src, gm->buf[al_p_z_buf], &gm->person[i].o, 1, 1);
			gm->person[i].o.v.x = rand()%50 - 25;
			gm->person[i].o.v.y = rand()%50 - 25;
            gm->person[i].mx_f = v2Len(gm->person[i].o.v);
		}
	}
	gm->hero.timer -= dt;
	if(gm->hero.state == P_Z && gm->hero.timer <= 0.0f){
		gm->hero.state = ZOMBIE;
        s_add_snd(gm->saved_src, gm->buf[al_p_z_buf], &gm->hero.o, 0.1, 1);
	}
	
	
	/*Zero forces on all objects*/
	for(i=0; i < gm->person_num; i++){
		gm->person[i].o.f.x = 0;
		gm->person[i].o.f.y = 0;
	}
	gm->hero.o.f.x=0;
	gm->hero.o.f.y=0;
	
	/*Add forces*/
	double msd = v2Len(v2Sub(gm->hero.o.p, gm->ms));
    vector2 msu = v2Unit(v2Sub(gm->hero.o.p, gm->ms));
    if(msd < gm->hero.o.r){msd = 0;}
    vector2 msf = v2sMul(-5*msd, msu);
	
	if(!gm->m){
		msf.x = 0;
		msf.y = 0;	
	}

	gm->hero.o.f.x += gm->ak.x*100 - gm->hero.o.v.x + msf.x;
	gm->hero.o.f.y += gm->ak.y*100 - gm->hero.o.v.y + msf.y;
	

    //Ai Forces

    zb_chase_hero(gm);
	
	/*Slows a person down when they are in the safe zone or if they are turning into a zombie*/
	for(i = 0; i < gm->person_num; i++){
		if(gm->person[i].state == SAFE ||gm->person[i].state == P_Z){
			gm->person[i].o.f.x +=  -gm->person[i].o.v.x;
			gm->person[i].o.f.y +=  -gm->person[i].o.v.y;
		}
        if((gm->person[i].state == ZOMBIE && gm->person[i].chase == 0 ) || (gm->person[i].state == PERSON && gm->person[i].ready == 0)){
            vector2 dir = v2Unit(gm->person[i].o.v);
			gm->person[i].o.f.x +=  -gm->person[i].o.v.x + gm->person[i].mx_f*dir.x;
			gm->person[i].o.f.y +=  -gm->person[i].o.v.y + gm->person[i].mx_f*dir.y;
        }
        if(gm->person[i].chase == 1){
			gm->person[i].o.f.x +=  -gm->person[i].o.v.x;
			gm->person[i].o.f.y +=  -gm->person[i].o.v.y;
        }

	}

	/*Spring force in the people chain*/
	if(gm->chain_num > 0){
		float ks = 50, r = 5, kd=1;
		i = gm->ppl_chain[gm->chain_num-1];
		vector2 l = v2Sub(gm->hero.o.p, gm->person[i].o.p);
		vector2 dl = v2Sub(gm->hero.o.v, gm->person[i].o.v);
		
		vector2 fa, fb;
		fa = v2sMul(-ks*(v2Len(l)-r) - kd * v2Dot(dl, l)/v2Len(l) , v2Unit(l));
		fb = v2sMul(-1,fa);
    	
		gm->hero.o.f = v2Add(gm->hero.o.f, fa);
		gm->person[i].o.f = v2Add(gm->person[i].o.f, fb);
		
		gm->person[i].o.f.x +=  -gm->person[i].o.v.x;
		gm->person[i].o.f.y +=  -gm->person[i].o.v.y;

		int k;
		for(k = gm->chain_num-1; k > 0; k--){
			i = gm->ppl_chain[k-1];
			int h = gm->ppl_chain[k];
			vector2 l = v2Sub(gm->person[h].o.p, gm->person[i].o.p);
			vector2 dl = v2Sub(gm->person[h].o.v, gm->person[i].o.v);

			vector2 fa, fb;
			fa = v2sMul(-ks*(v2Len(l)-r) - kd * v2Dot(dl, l)/v2Len(l) , v2Unit(l));
			fb = v2sMul(-1,fa);

			gm->person[h].o.f = v2Add(gm->person[h].o.f, fa);
			gm->person[i].o.f = v2Add(gm->person[i].o.f, fb);

			gm->person[i].o.f.x +=  -gm->person[i].o.v.x;
			gm->person[i].o.f.y +=  -gm->person[i].o.v.y;
		}
	}

	/*Wall Colisions*/
	for(i = 0; i < gm->person_num; i++){
		if(bounce(&gm->person[i].o, gm->w, gm->h) && gm->person[i].state == PERSON){
            s_add_snd(gm->saved_src, gm->buf[al_pwall_buf], &gm->person[i].o,0.2, 0);
		}
	}	
	if(bounce(&gm->hero.o, gm->w, gm->h)){
        s_add_snd(gm->saved_src, gm->buf[al_wall_buf], &gm->hero.o, 0.2, 0);
    }

	/*Line collisions*/
	for(i =0; i < gm->person_num; i++){
		for(k=0; k < gm->wall_num; k++){
			if(line_collision(gm->walls[k].p1, gm->walls[k].p2, &gm->person[i].o, 0.2, 0.3) && gm->person[i].state == PERSON){
                s_add_snd(gm->saved_src, gm->buf[al_pwall_buf], &gm->person[i].o,0.2, 0);
            }
		}
	}

	for(k=0; k < gm->wall_num; k++){
		if(line_collision(gm->walls[k].p1, gm->walls[k].p2, &gm->hero.o, 0.2, 0.3))
        {
			s_add_snd(gm->saved_src, gm->buf[al_wall_buf], &gm->hero.o,0.2, 0);
        }
	}


    if(gm->hero.spring_state == ATTACHED)
    {
        vector2 p = gm->hero.o.p;
        vector2 bp = gm->person[gm->ppl_chain[gm->chain_num-1]].o.p;
        for(i = 0; i < gm->wall_num; i++){
           if(line_line(p, bp, gm->walls[i].p1, gm->walls[i].p2)){
                chain_ready_zero(gm);
				gm->chain_num = 0;
	            gm->c = 1;
				gm->hero.spring_state = NOT_ATTACHED;
                i = gm->wall_num;
           } 
        }
		int k;
		for(k = gm->chain_num-1; k > 0; k--){
            int j = gm->ppl_chain[k-1];
            int h = gm->ppl_chain[k];
            vector2 p = gm->person[j].o.p;
            vector2 bp = gm->person[h].o.p;
			for(i = 0; i < gm->wall_num; i++){
	           if(line_line(p, bp, gm->walls[i].p1, gm->walls[i].p2)){
                    chain_cut(gm, j);
	                i = gm->wall_num;
                    k = 0;
	           } 
	        }	
		}
    }

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
			if(gm->hero.state == PERSON && gm->person[i].state == PERSON)
			{				
				int k = 0;
				int leave = 0;
				for(k=0; k<gm->chain_num; k++){
					if(i == gm->ppl_chain[k]){
						leave = 1;
					}
				}
				
				if(leave == 0){
					gm->hero.spring_state = ATTACHED;
					gm->ppl_chain[gm->chain_num] = i;
					gm->chain_num++;
					
					int rand_num = rand()%3;
					switch (rand_num) {
						case 0:
							s_add_snd(gm->saved_src, gm->buf[al_attached1_buf], &gm->person[i].o,1, 1);
							break;
						case 1:
							s_add_snd(gm->saved_src, gm->buf[al_attached2_buf], &gm->person[i].o,1, 1);
							break;
						case 2:
							s_add_snd(gm->saved_src, gm->buf[al_attached3_buf], &gm->person[i].o,1, 1);
							break;
					}
					gm->hero.person_id = i;
					gm->person[i].ready = 1;
					gm->person[i].mx_f = 0;
				}
			}
			else if(gm->person[i].state == ZOMBIE && gm->hero.state == PERSON){
				gm->hero.spring_state = NOT_ATTACHED;
				gm->hero.timer = MAX_TIME;
				gm->hero.state = P_Z;
				chain_ready_zero(gm);
				gm->chain_num = 0;
                s_add_snd(gm->saved_src, gm->buf[al_hdeath_buf], &gm->hero.o,0.1, 1);
			}
			else if(gm->person[i].state == PERSON && gm->hero.state == ZOMBIE){
				gm->person[i].timer = MAX_TIME;
				gm->person[i].state = P_Z;
                s_add_snd(gm->saved_src, gm->buf[al_pdeath_buf], &gm->person[i].o,1, 1);
			}
		}
		
		/*This bit of magic lets the hero take a person info a circle and save them.*/
		vector2 p = gm->safe_zone.p;
		if((gm->person[i].state == PERSON && gm->person[i].ready == 0) || 
			gm->person[i].state == ZOMBIE ||
			gm->person[i].state == P_Z){
			collision(&gm->person[i].o, &gm->safe_zone); 	
		}
		else if(gm->person[i].state == SAFE){
			r_collision(&gm->safe_zone, &gm->person[i].o);
		}
		else if (safe_zone_test(gm->safe_zone, gm->person[i].o)){
			gm->hero.spring_state = NOT_ATTACHED;
			gm->person[i].state = SAFE;
			int rand_num = rand()%2;
			if (rand_num == 0) {
				s_add_snd(gm->saved_src, gm->buf[al_saved1_buf], &gm->person[i].o,1, 1);
			}
			else {
				s_add_snd(gm->saved_src, gm->buf[al_saved2_buf], &gm->person[i].o,1, 1);
			}
			chain_remove(gm, i);
		}
		gm->safe_zone.p = p;
		

        gm->person[i].emo = NORMAL;
		for(k = 0; k < gm->person_num; k++){
            if(k != i && gm->person[k].state == ZOMBIE && gm->person[i].state == PERSON){
                if(v2Len(v2Sub(gm->person[i].o.p, gm->person[k].o.p)) < 10 && gm->person[i].emo == NORMAL){
                    //s_add_snd(gm->saved_src, gm->buf[al_scared_buf], &gm->person[i].o, 1);
                    gm->person[i].emo = SCARED;
                }
                else if(v2Len(v2Sub(gm->person[i].o.p, gm->person[k].o.p)) > 10 && gm->person[i].emo == SCARED){
                    gm->person[i].emo = NORMAL;
                }
            }
        }

		/*This deals with collisions between people*/	
		for(k = 1+i; k < gm->person_num; k++){
			if(collision(&gm->person[i].o, &gm->person[k].o)){
				if(gm->person[i].state == PERSON && gm->person[k].state == ZOMBIE){					
					gm->person[i].timer = MAX_TIME;
					gm->person[i].state = P_Z;
					gm->person[i].parent_id = k;
					chain_cut(gm, i);
                    s_add_snd(gm->saved_src, gm->buf[al_pdeath_buf], &gm->person[i].o,1, 4);
				}
				if(gm->person[i].state == ZOMBIE && gm->person[k].state == PERSON){
					gm->person[k].timer = MAX_TIME;
					gm->person[k].state = P_Z;
					gm->person[k].parent_id = i;
					chain_cut(gm, k);
                    s_add_snd(gm->saved_src, gm->buf[al_pdeath_buf], &gm->person[i].o,1, 4);
				}
			}
		}
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
	
	/*Advances our view level */
	double ratio = (double)width/(double)height; 
	vector2 w = {gm->viewratio, gm->viewratio};
	w.x = ratio * w.y;
	
	gm->vmin.x -= gm->zoom*dt*1*w.x; 
	gm->vmin.y -= gm->zoom*dt*1*w.y;
	gm->vmax.x += gm->zoom*dt*1*w.x;
	gm->vmax.y += gm->zoom*dt*1*w.y;
	
	gm->viewratio = gm->vmax.y - gm->vmin.y;
	
	
	alListener3f(AL_POSITION, gm->hero.o.p.x, gm->hero.o.p.y, 0);
    s_update(gm->saved_src);
}

void gm_render(game gm){
    gm_update_view(gm);
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

	for(i = 0; i<gm->wall_num; i++){
		glBindTexture( GL_TEXTURE_2D, gm->rope_tex);
		glPushMatrix();
		glBegin(GL_QUADS);
		
		vector2 p1,p2;
		p1 = gm->walls[i].p1;
		p2 = gm->walls[i].p2;
		vector2 temp;

		glTexCoord2f(1.0, 0.0);
		temp = v2Add(p1, v2sMul(2.0, v2Unit(v2Rotate(M_PI/2, v2Sub(p2,p1)))));
		glVertex3f(temp.x, temp.y, 0.0);

		glTexCoord2f(0.0, 0.0);
		temp = v2Add(p1, v2sMul(2.0, v2Unit(v2Rotate(-M_PI/2, v2Sub(p2,p1)))));
		glVertex3f(temp.x, temp.y, 0.0);

		glTexCoord2f(0.0, 1.0);
		temp = v2Add(p2, v2sMul(2.0, v2Unit(v2Rotate(-M_PI/2, v2Sub(p2,p1)))));
		glVertex3f(temp.x, temp.y, 0.0);

		glTexCoord2f(1.0, 1.0);
		temp = v2Add(p2, v2sMul(2.0, v2Unit(v2Rotate(M_PI/2, v2Sub(p2,p1)))));
		glVertex3f(temp.x, temp.y, 0.0);

		glEnd();
		glPopMatrix();
	}
	
	if(gm->chain_num > 0){
		i = gm->ppl_chain[gm->chain_num-1];
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
		int k;
		for(k = gm->chain_num-1; k > 0; k--){
			i = gm->ppl_chain[k];
			int h = gm->ppl_chain[k-1];
			glPushMatrix();
			glBegin(GL_QUADS);

			vector2 p1,p2;
			p1 = v2Add(gm->person[i].o.p, v2sMul(gm->person[i].o.r, v2Unit(v2Sub(gm->person[h].o.p, gm->person[i].o.p))));
			p2 = v2Add(gm->person[h].o.p, v2sMul(gm->person[h].o.r, v2Unit(v2Sub(gm->person[i].o.p, gm->person[h].o.p))));

			glTexCoord2f(1.0, 0.0);
			vector2 temp = v2Add(p1, v2sMul(2.0, v2Unit(v2Rotate(M_PI/2, v2Sub(gm->person[i].o.p, gm->person[h].o.p)))));
			glVertex3f(temp.x, temp.y, 0.0);

			glTexCoord2f(0.0, 0.0);
			temp = v2Add(p1, v2sMul(2.0, v2Unit(v2Rotate(-M_PI/2, v2Sub(gm->person[i].o.p, gm->person[h].o.p)))));
			glVertex3f(temp.x, temp.y, 0.0);

			glTexCoord2f(0.0, 1.0);
			temp = v2Add(p2, v2sMul(2.0, v2Unit(v2Rotate(-M_PI/2, v2Sub(gm->person[i].o.p, gm->person[h].o.p)))));
			glVertex3f(temp.x, temp.y, 0.0);

			glTexCoord2f(1.0, 1.0);
			temp = v2Add(p2, v2sMul(2.0, v2Unit(v2Rotate(M_PI/2, v2Sub(gm->person[i].o.p, gm->person[h].o.p)))));
			glVertex3f(temp.x, temp.y, 0.0);

			glEnd();
			glPopMatrix();
		}
	}

    stink_render(gm);
	
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



    glBindTexture( GL_TEXTURE_2D, gm->safe_tex);
    glPushMatrix();
	glTranslatef(gm->tp.x, gm->tp.y, 0);
	glScalef(0.5, 0.5,0);
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
		if(gm->person[i].state == PERSON && gm->person[i].emo == NORMAL){
			glBindTexture( GL_TEXTURE_2D, gm->person_tex);
		}
        else if(gm->person[i].state == PERSON && gm->person[i].emo == SCARED){
			glBindTexture( GL_TEXTURE_2D, gm->person_s_tex);
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
		
		if(gm->person[i].state == SAFE){
			vector2 ep;
			ep.x = gm->person[i].o.p.x + 20.0f/64.0f*gm->person[i].o.r;
			ep.y = gm->person[i].o.p.y + 18.0f/64.0f*gm->person[i].o.r;
			
			ep = v2Add(ep, v2sMul(5.0f/64.0f*gm->person[i].o.r, v2Unit(v2Sub(gm->hero.o.p, ep))));
			
			glBindTexture( GL_TEXTURE_2D, gm->eye_tex);
			glPushMatrix();
			
			glTranslatef(ep.x, ep.y, 0);
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
			
			ep.x = gm->person[i].o.p.x - 23.0f/64.0f*gm->person[i].o.r;
			ep.y = gm->person[i].o.p.y + 19.0f/64.0f*gm->person[i].o.r;
			
			ep = v2Add(ep, v2sMul(5.0f/64.0f*gm->person[i].o.r, v2Unit(v2Sub(gm->hero.o.p, ep))));
			
			glPushMatrix();
			glTranslatef(ep.x, ep.y, 0);
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
}

void gm_stats(game gm, double * time, int * people){
	int i, add = 0;
	for(i = 0; i < gm->person_num; i++){
		if(gm->person[i].state == SAFE){
			add++;
		}
	}
    *people = add;
    *time = gm->timer;

}

void gm_message_render(game gm, int width, int height){
	int i, add = 0;
    int check = 0;
	for(i = 0; i < gm->person_num; i++){
		if(gm->person[i].state == SAFE){
			add++;
            check++;
		}
        if(gm->person[i].state == PERSON){
            check++;
        }
	}

	/*Messages*/
	char buf[100];

	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio = (double)width/(double)height;
    height = 600;
    width = height*ratio;
   
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);

	/*
    float c[4] = {1,.5,.4,.4};
    rat_set_text_color(gm->font, c);
	sprintf(buf, "Saved %d of %d",add, gm->save_count);	
    float len = rat_font_text_length(gm->font, buf);
    rat_font_render_text(gm->font,20,height-4, buf);
    */

    float len;
	sprintf(buf, "%.1lf", gm->timer);	
    len = rat_font_text_length(gm->font, buf);
    rat_font_render_text(gm->font,width/2 - 50,height-4, buf);
	
	/*
    sprintf(buf, "Chain Num: %d", gm->chain_num);	
    len = rat_font_text_length(gm->font, buf);
    rat_font_render_text(gm->font, width - 250,height-4, buf);
	*/

    float co[4] = {0,0,0,0};
    rat_set_text_color(gm->font, co);

	if(add >= gm->save_count && gm->hero.state == SAFE){
        sprintf(buf, "Press space to continue.");	
        len = rat_font_text_length(gm->font, buf);
        rat_font_render_text(gm->font,(width-len)/2,height/2, buf);
    }
    else if(gm->hero.state == DONE){
        gm->gm_state = 1;
        sprintf(buf, "Level Complete");	
        len = rat_font_text_length(gm->font, buf);
        rat_font_render_text(gm->font,(width-len)/2,height/2, buf);
    }
    else if(gm->hero.state == P_Z || gm->hero.state == ZOMBIE){
        gm->gm_state = 1;
        sprintf(buf, "Infected!");	
        len = rat_font_text_length(gm->font, buf);
        rat_font_render_text(gm->font,(width-len)/2,height/2, buf);
    }
    else if(check < gm->save_count){
        gm->gm_state = 1;
        sprintf(buf, "Too many Zombies!");	
        len = rat_font_text_length(gm->font, buf);
        rat_font_render_text(gm->font,(width-len)/2,height/2, buf);
	}
    glBindTexture(GL_TEXTURE_2D, gm->blank_tex);
    for (i = 0; i < gm->save_count; i++)
    {
        glPushMatrix();
        glTranslatef(i*31 + 20, height - 20, 0);
        glScalef(15, 15,0);
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
    glBindTexture(GL_TEXTURE_2D, gm->extra_tex);
    for (i = 0; i < add; i++)
    {
        glPushMatrix();
        glTranslatef(i*31 + 20, height - 20, 0);
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

    glPopMatrix();
}

int gm_progress(game gm){
	int i, add = 0;
    int check = 0;
    int person = 0;
	for(i = 0; i < gm->person_num; i++){
		if(gm->person[i].state == SAFE){
			add++;
            check++;
		}
        if(gm->person[i].state == PERSON){
            check++;
            person++;
        }
	}
	if(gm->hero.state == ZOMBIE || check < gm->save_count){
        gm->gm_state = 1;
		return -1;
	}
	if(gm->hero.state == P_Z || check < gm->save_count){
        gm->gm_state = 1;
    }
	if((add >= gm->save_count && gm->hero.state == SAFE && gm->c == 1) ||
        (person == 0 && add >= gm->save_count && gm->hero.state == SAFE)){
		gm->hero.state = DONE;
		return add - gm->save_count + 1;
	}
	
	return 0;
}

void gm_free(game gm){
	free(gm);
}

void gm_mouse(game gm, int x, int y){
	double vdiffy = gm->vmax.y - gm->vmin.y;
	double vdiffx = gm->vmax.x - gm->vmin.x;
    
    gm->ms.x = vdiffx / (double)gm->screenx * x + gm->vmin.x;
    gm->ms.y = vdiffy / (double)gm->screeny * y + gm->vmin.y;
}

void gm_nkey_down(game gm, unsigned char key){
	switch(key) {
		case 'z':
			gm->zoom = 1;
			break;
		
		case 'x':
			gm->zoom = -1;
			break;
			
		case ' ':
			chain_ready_zero(gm);
			gm->chain_num = 0;
            gm->c = 1;
			gm->hero.spring_state = NOT_ATTACHED;
			break;

        case 'c':
            gm->c = 1;
			break;
			
        case 'm':
		    gm->m = gm->m ? 0 : 1;
			break;
	}
}

void gm_nkey_up(game gm, unsigned char key){
	switch(key) {
		case 'z':
			gm->zoom = 0;
			break;
		
		case 'x':
			gm->zoom = 0;
			break;
		case 'c':
			gm->c = 0;
			break;
		case ' ':
			gm->c = 0;
			break;
	}
	
}


void gm_skey_down(game gm, int key){
	switch(key) {
		case SDLK_LEFT : 
			gm->ak.x = -1;
			break;
		case SDLK_RIGHT : 
			gm->ak.x = 1;
			break;
		case SDLK_UP : 
			gm->ak.y = 1;
			break;
		case SDLK_DOWN : 
			gm->ak.y = -1;
			break;
	}
}

void gm_skey_up(game gm, int key){
	switch (key) {
		case SDLK_LEFT : 
			if(gm->ak.x < 0) {gm->ak.x = 0;}
			break;
		case SDLK_RIGHT: 
			if(gm->ak.x > 0) {gm->ak.x = 0;}
			break;
		case SDLK_UP : 
			if(gm->ak.y > 0) {gm->ak.y = 0;}
			break;
		case SDLK_DOWN : 
			if(gm->ak.y < 0) {gm->ak.y = 0;}
			break;
	}
}

vector2 gm_dim(game gm){
	vector2 dim = gm->hero.o.p;
	return dim;
}

void gm_free_level(game gm){
	glDeleteTextures(1, &gm->bk);
}

int gm_load_bk(game gm, char * file){
        if(!load_texture(file, &gm->bk)){
            strcpy(gm->res_buf, gm->res_path);
            strcat(gm->res_buf, "/imgs/bk.png");
            load_texture(gm->res_buf, &gm->bk);
        }
}

int load_level_file(game gm, char * file){
		int i;
		FILE *loadFile;

		loadFile = fopen(file, "r");

		if(loadFile == NULL)
		{
			printf("File %s unable to load.\n", file); 
			return 0;
		}
		

		char type[5];
		int result;
		
		gm->chain_num = 0;
		
		gm->safe_zone.m = 1000;
		gm->safe_zone.v.x = 0;
		gm->safe_zone.v.y = 0;
		gm->person_num = 0;
		gm->wall_num = 0;
        gm->save_count = 1;
		gm->timer = 0;
		i = 1;
        gm->stnk_num = 0;
		while( (result = fscanf(loadFile, "%s", type)) != EOF){
			if(!strncmp(type, "hw", 2)){
				result = fscanf(loadFile, "%lf %lf", &gm->h, &gm->w);
				if(result != 2){
					printf("Error loading level file %s on line %d. Result: %d\n", file, i, result);
					printf("Error: Height and width.\n");
				}
			}
			else if(!strncmp(type, "sz", 2)){
				result = fscanf(loadFile, "%lf %lf %lf", 
					&gm->safe_zone.p.x,
					&gm->safe_zone.p.y,
					&gm->safe_zone.r);
				if(result != 3){
					printf("Error loading level file %s on line %d. Result: %d\n", file, i, result);
					printf("Error: Safe Zone not loaded.\n");
				}
			}
			else if(!strncmp(type, "sc", 2)){
				result = fscanf(loadFile, "%d", &gm->save_count);
				if(result != 1){
					printf("Error loading level file %s on line %d. Result: %d\n", file, i, result);
					printf("Error: Save count not loaded.\n");
				}
			}
			else if(!strncmp(type, "p", 1)){
				int num = gm->person_num;
				gm->person[num].state = PERSON;
				gm->person[num].emo = NORMAL;
				gm->person[num].ready = 0;
				result = fscanf(loadFile, "%lf %lf %lf %lf %lf %lf", 
					&gm->person[num].o.p.x,
					&gm->person[num].o.p.y,
					&gm->person[num].o.v.x,
					&gm->person[num].o.v.y,
					&gm->person[num].o.r,
					&gm->person[num].o.m);
				if(result != 6){
					printf("Error loading level file %s on line %d. Result: %d\n", file, i, result);
					printf("Error: A person was not loaded.\n");
				}
				else{
                    gm->person[num].mx_f = v2Len(gm->person[num].o.v);
                    gm->person[num].o.snd = 0;
					gm->person[num].chase = 0;
					gm->person[num].parent_id = -1;
					gm->person_num++;
				}
			}
			else if(!strncmp(type, "z", 1)){
				int num = gm->person_num;
				gm->person[num].state = ZOMBIE;
				gm->person[num].emo = NORMAL;
				gm->person[num].ready = 0;
				result = fscanf(loadFile, "%lf %lf %lf %lf %lf %lf", 
					&gm->person[num].o.p.x,
					&gm->person[num].o.p.y,
					&gm->person[num].o.v.x,
					&gm->person[num].o.v.y,
					&gm->person[num].o.r,
					&gm->person[num].o.m);
				if(result != 6){
					printf("Error loading level file %s on line %d. Result: %d\n", file, i, result);
					printf("Error: A zombie was not loaded.\n");
				}
				else{
                    stink_add(gm, gm->person_num);
                    gm->person[num].mx_f = v2Len(gm->person[num].o.v);
					gm->person[num].chase = 0;
					gm->person[num].parent_id = -1;
                    gm->person[num].o.snd = 0;
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
					printf("Error loading level file %s on line %d. Result: %d\n", file, i, result);
					printf("Error: The hero was not loaded!\n");
				}
                gm->hero.o.snd = 0;
			}
			else if(!strncmp(type, "w", 1)){
				int n = gm->wall_num;
				result = fscanf(loadFile, "%lf %lf %lf %lf", 
					&gm->walls[n].p1.x,
					&gm->walls[n].p1.y,
					&gm->walls[n].p2.x,
					&gm->walls[n].p2.y);
				if(result != 4){
					printf("Error loading level file %s on line %d. Result: %d\n", file, i, result);
					printf("Error: Wall not loaded\n");
				}
				else{
					gm->wall_num++;
				}
			}
		}
        gm->gm_state = 0;

		fclose(loadFile);
		printf("Level file %s loaded.\n", file);
		return 1;
}

void chain_remove(game gm, int index){
	int k;
	int match;
	for(k = 0; k < gm->chain_num; k++){
		if(gm->ppl_chain[k] == index){
			match = k;
			break;
		}
	}
	for(k = match; k < gm->chain_num; k++){
		gm->ppl_chain[k] = gm->ppl_chain[k+1];
	}
	gm->chain_num--;
}

void chain_cut(game gm, int index){
	int k;
	int match = -1;
	for(k = 0; k < gm->chain_num; k++){
		if(gm->ppl_chain[k] == index){
			match = k;
			break;
		}
	}
	if(match == -1){return;}
    for(k = match; k >= 0; k--){
        int temp = gm->ppl_chain[k];
        gm->person[temp].ready = 0;
    }
    gm->chain_num = gm->chain_num - match - 1;
    for(k = 0; k < gm->chain_num; k++){
        gm->ppl_chain[k] = gm->ppl_chain[k + match + 1];
    }
    if(gm->chain_num == 0){
        gm->hero.spring_state = NOT_ATTACHED;
    }
}

void chain_ready_zero(game gm){
	int k;
	for(k = 0; k < gm->chain_num; k++){
		int h = gm->ppl_chain[k];
		gm->person[h].ready = 0;
	}
}


void stink_add(game gm, int id){
    int k;
    int num = gm->stnk_num;
    gm->stnk[num].id = id;    

    for(k = 0; k < STINK_PARTICLE_NUM; k++){
        gm->stnk[num].time[k] = (rand()%100 + 50)/100.0f;
        gm->stnk[num].n[k].p.x = gm->person[id].o.p.x + 2*cos((M_PI*2)/360.0f*(float)(rand()%360));
        gm->stnk[num].n[k].p.y = gm->person[id].o.p.y + 2*sin((M_PI*2)/360.0f*(float)(rand()%360));
        gm->stnk[num].n[k].v.x += rand()%4 - 2;
        gm->stnk[num].n[k].v.y += rand()%4 - 2;
    }
    gm->stnk_num++;
}

void stink_step(game gm, double dt){
    int k;
    int num;

    for(num = 0; num < gm->stnk_num; num++){
        int id = gm->stnk[num].id;
        for(k = 0; k < STINK_PARTICLE_NUM; k++){
            if(gm->stnk[num].time[k] <=0){
                gm->stnk[num].time[k] = (rand()%100 + 50)/100.0f;
        gm->stnk[num].n[k].p.x = gm->person[id].o.p.x + 2*cos((M_PI*2)/360.0f*(float)(rand()%360));
        gm->stnk[num].n[k].p.y = gm->person[id].o.p.y + 2*sin((M_PI*2)/360.0f*(float)(rand()%360));
                gm->stnk[num].n[k].v.x = gm->person[id].o.v.x/2.0f + rand()%6 - 3;
                gm->stnk[num].n[k].v.y = gm->person[id].o.v.y/2.0f +rand()%6 - 3;
                gm->stnk[num].n[k].th = rand()%360;
            }
            else{
                gm->stnk[num].time[k] -= dt;
                gm->stnk[num].n[k].p.x += gm->stnk[num].n[k].v.x*dt;
                gm->stnk[num].n[k].p.y += gm->stnk[num].n[k].v.y*dt;
            }
        }
    }
}

void stink_render(game gm){
    int k;
    int num;
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    for(num = 0; num < gm->stnk_num; num++){
        for(k = 0; k < STINK_PARTICLE_NUM; k++){
            glColor4f(1,1,1, gm->stnk[num].time[k]*4);
            glBindTexture( GL_TEXTURE_2D, gm->stink_tex);
            glPushMatrix();
            glTranslatef(gm->stnk[num].n[k].p.x, gm->stnk[num].n[k].p.y, 0);
            glRotatef(gm->stnk[num].n[k].th, 0, 0, 1);
            glScalef(8,8,0);
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
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


void zb_chase_hero(game gm){
    int k;
    for(k=0; k < gm->person_num; k++){
        if(gm->person[k].state == ZOMBIE && gm->person[k].chase == 0 && gm->person[k].o.r > 2){
            if(v2Len(v2Sub(gm->person[k].o.p, gm->hero.o.p)) <= 20){
                gm->person[k].chase = 1;
            }
        }
        else if(gm->person[k].state == ZOMBIE && gm->person[k].chase == 1 && gm->person[k].o.r > 2 ){
            if(v2Len(v2Sub(gm->person[k].o.p, gm->hero.o.p)) >= 30){
                gm->person[k].chase = 0;
            }
        }
        if(gm->person[k].chase == 1 && gm->hero.state ==  PERSON){
            ai_chase(&gm->hero.o, &gm->person[k].o, 70.0f, 30.0f);
            //ai_seek(gm->hero.o.p, &gm->person[k].o, 2.0f);
            //ai_avoid(gm->hero.o.p, &gm->person[k].o, 200.0f);
        }
		else if(gm->person[k].state == ZOMBIE && gm->hero.state ==  ZOMBIE){
            //ai_chase(&gm->hero.o, &gm->person[k].o, 70.0f, 30.0f);
            ai_seek(gm->hero.o.p, &gm->person[k].o, 2.0f);
            //ai_avoid(gm->hero.o.p, &gm->person[k].o, 200.0f);
        }
		
		if(gm->person[k].state == ZOMBIE && gm->person[k].parent_id >= 0){
            //ai_chase(&gm->hero.o, &gm->person[k].o, 70.0f, 30.0f);
            ai_seek(gm->person[gm->person[k].parent_id].o.p, &gm->person[k].o, 2.0f);
            //ai_avoid(gm->hero.o.p, &gm->person[k].o, 200.0f);
        }
        
   }

}

