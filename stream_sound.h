typedef struct al_stream_type * al_stream;

al_stream al_stream_init(void);

int al_stream_load_file(al_stream als, const char * file);

int al_stream_free_file(al_stream als);

int al_stream_play(al_stream als);

int al_stream_update(al_stream als);

