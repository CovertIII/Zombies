typedef struct gametype *game;

game gm_init(char * res_path);

int gm_init_textures(game gm);
void gm_init_sounds(game gm);

int gm_load_level_svg(game gm, char * file_path);

//void gm_free_level(game gm);

void gm_update(game gm, int height, int width, double dt);

int gm_progress(game gm);

void gm_free(game gm);

void gm_mouse(game gm, int x, int y);

void gm_skey_down(game gm, int key);

void gm_skey_up(game gm, int key);	

void gm_render(game gm);

object gm_get_hero(game gm);

void gm_set_db_string(game gm, char * db_path);

void gm_set_hero(game gm, object hero);

char * gm_portal(game gm);

char * gm_portal_check(game gm);

void gm_reshape(game gm, int width, int height);

vector2 gm_dim(game gm);
