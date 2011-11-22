#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include "vector2.h"
#include "sound_list.h"
#include "physics.h"

typedef struct sf_node {
	ALuint src;
    object * o;
	struct sf_node * next;
} sf_node;

typedef struct sf_type {
	sf_node * head;
	ALuint buf;
} sf_type;

sf_list sf_init(ALuint buffer){
	sf_list sl;
	sl = (sf_type*)malloc(sizeof(sf_type));
	if(!sl){return NULL;}

	sl->head = NULL;
	sl->buf = buffer;

	return sl;
}


void sf_add_snd(sf_list sl, object * o){
	sf_node * new;
	new = (sf_node*)malloc(sizeof(sf_node));
	if(!new){return;}

    new->o = o;
	alGenSources(1, &new->src);
	alSourcei(new->src, AL_BUFFER, sl->buf);
	alSource3f(new->src, AL_POSITION, new->o->p.x,new->o->p.y, 0);
	alSourcePlay(new->src);

	new->next = NULL;

	if(sl->head == NULL){
		sl->head = new;
		return;
	}

	new->next = sl->head;
	sl->head = new;
	return;
}

void sf_update(sf_list sl){
	sf_node * cycle = sl->head;
	sf_node * prev = NULL;
	while(cycle != NULL){
		int state;
		alGetSourcei(cycle->src, AL_SOURCE_STATE, &state);
		if(state == AL_STOPPED){
			if(prev == NULL){
				sf_node * tmp = cycle->next;
				sl->head = tmp;
				alDeleteSources(1, &cycle->src);
				free(cycle);
				cycle = tmp;
			}
			else{
				prev->next = cycle->next;
				alDeleteSources(1, &cycle->src);
				free(cycle);
				cycle = prev->next;
			}
		}
		else{cycle = cycle->next;}
	}
}

void sf_free(sf_list sl){
	sf_node * cycle = sl->head;
	while(cycle != NULL){
		sf_node * tmp = cycle->next;
		alDeleteSources(1, &cycle->src);
		free(cycle);
		cycle = tmp;
	}
}

