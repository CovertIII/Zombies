typedef struct data_record_type * data_record;


data_record init_data_record();


void prepare_level_scores(data_record db);
void render_level_scores(data_record db);
