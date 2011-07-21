typedef struct gametype *game;

game gm_init(void);

//int gm_init_textures(game gm);
//void gm_init_sound(game gm);

void gm_load_level(game gm, int lvl);

void gm_free_level(game gm);

void gm_update(game gm, double dt);

int gm_progress(game gm);

void gm_free(game gm);


gm_skey_down(game gm, int key);

gm_skey_up(game gm, int key);	

gm_render(game gm);

vector2 gm_dim(game gm);