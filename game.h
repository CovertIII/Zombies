typedef struct gametype *game;

game gm_init(void);

int gm_init_textures(game gm);
//void gm_init_sound(game gm);

void gm_load_level(game gm, char * lvl);

//void gm_free_level(game gm);

void gm_update(game gm, double dt);

int gm_progress(game gm);

void gm_free(game gm);


void gm_skey_down(game gm, int key);

void gm_skey_up(game gm, int key);	

void gm_render(game gm);

void gm_reshape(game gm, int width, int height);

vector2 gm_dim(game gm);
