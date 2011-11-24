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
	vector2 p1;
	vector2 p2;
} line;

int collision(object *ta, object *tb);

int r_collision(object *ta, object *tb);

int bounce(object * obj, int x, int y);

int collision_test(object ta, object tb);

int safe_zone_test(object ta, object tb);

int line_collision(vector2 pt1, vector2 pt2, object *c, float uk, float us);

int point_collision(vector2 pt, object *c);

int line_line(vector2 p, vector2 bp, vector2 q, vector2 bq);
