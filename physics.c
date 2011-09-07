#include "vector2.h"
#include "physics.h"

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
int line_collision(vector2 pt1, vector2 pt2, object *c, float uk, float us){
	vector2 pt2t, ct, pt4;
	
	pt2t = v2Sub(pt2, pt1);
	ct = v2Sub(c->p, pt1);
	pt4 = v2sMul(v2Dot(pt2t, ct)/v2Dot(pt2t, pt2t), pt2t);
	
	if(v2Dot(pt2t, ct)>0 && v2Len(pt2t)>v2Len(pt4)){
		vector2 n = v2Unit(v2Sub(ct, pt4));
		vector2 vn = v2sMul(v2Dot(c->v, n), n);
		vector2 vt = v2Sub(c->v, vn);
		vector2 fn = v2sMul(-v2Dot(c->f, n), n);
		vector2 ft = v2Add(c->f, fn);
		
		if((v2Len(v2Sub(ct, pt4))<c->r+.3) && v2Dot(c->f, n) < 0){
			//Fricational Forces
			if(v2Dot(pt4, c->v) < 0 && v2Len(c->v) > 1){
				ft = v2Add(v2sMul(-v2Len(v2sMul(uk, fn)),v2Unit(vt)),ft);
				c->f = v2Add(ft, c->f);
			}
			else if(v2Dot(pt4, c->v) > 0 && v2Len(c->v) > 1){
				ft = v2Add(v2sMul(-v2Len(v2sMul(uk, fn)),v2Unit(vt)),ft);
				c->f = v2Add(ft, c->f);
			}
			else if(v2Len(ft) < v2Len(fn)*us)
			{
				vt.x=0;
				vt.y=0;
				ft.x=0;
				ft.y=0;
			}
		}
		
		//Contact Force
		if((v2Len(v2Sub(ct, pt4))<c->r+.3) && (v2Len(vn) < 5) && v2Dot(c->f, n) < 0){	
			c->f = ft;
			c->v = vt;
		}
		
		
		//Bounce
		if(v2Len(v2Sub(ct, pt4))<c->r){
			c->p = v2Add(c->p, v2sMul(c->r - v2Len(v2Sub(ct, pt4)), v2Unit(n)));
			vector2 vnf = v2sMul(-1, vn); 
			c->v = v2Add(vnf, vt);
			return 1;
		}
	}
	if(point_collision(pt1, c) || point_collision(pt2, c)){
		return 1;
	}
	return 0;
}

int point_collision(vector2 pt, object *c){
	if(v2Len(v2Sub(pt, c->p)) < c->r){
		vector2 n = v2Unit(v2Sub(c->p, pt));
		vector2 vn = v2sMul(v2Dot(c->v, n), n);
		vector2 vt = v2Sub(c->v, vn);

		vector2 vnf = v2sMul(-1, vn); 
		c->v = v2Add(vnf, vt);

		double diff = c->r - v2Len(v2Sub(c->p, pt));
		c->p = v2Add(c->p, v2sMul(diff, n));
		return 1;
	}
	return 0;
}	

