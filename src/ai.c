#include "vector2.h"
#include "physics.h"

void ai_chase(object * target, object * chaser, double vec, double force){
    vector2 line = v2Sub(chaser->p, target->p);
    float time = v2Len(line)/vec;

    vector2 p2 = v2Add(target->p, v2sMul(time, target->v));
    vector2 p1 = chaser->p;
    vector2 pp = v2Sub(p2,p1);
    vector2 fc = v2sMul(force, v2Unit(pp));
    chaser->f = v2Add(chaser->f, fc);
}

void ai_seek(vector2 target, object * chaser, double force, double length){
    vector2 line = v2Sub(chaser->p, target);
    float len = v2Len(line);
    float var = len > 20 ? 20 : len;
//	var = len < 5 ? 0 : var;
    vector2 fc = v2sMul(-force/len * var , line);
    chaser->f = v2Add(chaser->f, fc);
}


void ai_avoid(vector2 target, object * chaser, double force){
    vector2 line = v2Sub(chaser->p, target);
    vector2 fc = v2sMul(force/v2Len(line), v2Unit(line));
    chaser->f = v2Add(chaser->f, fc);
}
