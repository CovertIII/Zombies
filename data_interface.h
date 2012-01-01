typedef struct data_record_type * data_record;


data_record init_data_record();

void stats_list_prep(data_record db);
void render_level_scores(data_record db, int width, int height);
