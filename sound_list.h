typedef struct s_type * s_list;

s_list s_init();

void s_add_snd(s_list sl, ALuint buf, object * o, int following);

void s_update(s_list snd);

void s_free(s_list snd);
