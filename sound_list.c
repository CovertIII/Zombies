#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include "vector2.h"
#include "physics.h"
#include "sound_list.h"

typedef struct s_node {
	ALuint src;
    int following;
    object * o;
	struct s_node * next;
} s_node;

typedef struct s_type {
	s_node * head;
    int num;
} s_type;

s_list s_init(void){
	s_list sl;
	sl = (s_type*)malloc(sizeof(s_type));
	if(!sl){return NULL;}

	sl->head = NULL;
    sl->num = 0;

	return sl;
}


void s_add_snd(s_list sl, ALuint buf, object * o, int following){
    //Checks to see if the object is already making a sound
    //If it is, it deletes the source and adds a new one
    if(following == 1){
        int chk = 0;
        s_node * cycle;
        for(cycle = sl->head; cycle != NULL; cycle = cycle->next)
        {
            if(cycle->o == o){
                chk = 1;
                break;
            }
        }
        if(chk == 1 && cycle->following == 1){
            alDeleteSources(1, &cycle->src);
            alGenSources(1, &cycle->src);
            alSourcei(cycle->src, AL_BUFFER, buf);
            alSource3f(cycle->src, AL_POSITION, o->p.x, o->p.y, 0);
            alSourcePlay(cycle->src);
            return;
        }
    }
        
	s_node * new;
	new = (s_node*)malloc(sizeof(s_node));
	if(!new){return;}
    
    new->o = o;
    o->snd = following;
    new->following = following;
	alGenSources(1, &new->src);
	alSourcei(new->src, AL_BUFFER, buf);
	alSource3f(new->src, AL_POSITION, o->p.x, o->p.y, 0);
	alSourcePlay(new->src);

	new->next = NULL;

	if(sl->head == NULL){
		sl->head = new;
		return;
	}

	new->next = sl->head;
	sl->head = new;
    sl->num++;
	return;
}

void s_update(s_list sl){
	s_node * cycle = sl->head;
	s_node * prev = NULL;
	while(cycle != NULL){
		int state;
        if(cycle->following == 1){
            alSource3f(cycle->src, AL_POSITION, cycle->o->p.x, cycle->o->p.y, 0);
            //alSource3f(cycle->src, AL_VELOCITY, cycle->o->v.x, cycle->o->v.y, 0);
        }
		alGetSourcei(cycle->src, AL_SOURCE_STATE, &state);
		if(state == AL_STOPPED){
			if(prev == NULL){
				s_node * tmp = cycle->next;
				sl->head = cycle->next;
				alDeleteSources(1, &cycle->src);
				free(cycle);
                sl->num++;
				cycle = tmp;
			}
			else{
				prev->next = cycle->next;
                cycle->o->snd = 0;
				alDeleteSources(1, &cycle->src);
				free(cycle);
                sl->num++;
				cycle = prev->next;
			}
		}
		else{
            prev = cycle;
            cycle = cycle->next;
        }
	}
}

void s_free(s_list sl){
	s_node * cycle = sl->head;
	while(cycle != NULL){
		s_node * tmp = cycle->next;
		alDeleteSources(1, &cycle->src);
		free(cycle);
		cycle = tmp;
	}
}

