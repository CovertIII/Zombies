typedef struct data_record_type * data_record;


data_record init_data_record(char * file_name, char * resource_path);

void stats_list_prep(data_record db);

void render_level_scores(data_record db, int width, int height);

void stats_reload_fonts(data_record db);

int db_get_lives(data_record db);
