#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <GLUT/GLUT.h>
#include <string.h>
#include <time.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include "freetype_imp.h"
#include "data_interface.h"

#define USERLIST 0
#define GAMELIST 1
#define LEVELLIST 2

typedef struct user_list{
    char * name;
    int id;
    struct user_list * next;
} user_list;

typedef struct level_list{
    int id;
    char * name;
    int level_id;
    double time;
    int num_saved;
    struct level_list * next;
} level_list;

typedef struct game_list{
    int id;
    char * name;
    int max_level;
    unsigned long int date;
    int elapsed_time;
    int people_saved;
    int deaths;
    struct game_list * next;
} game_list;

typedef struct data_record_type{
    char * file_name;
    
    int h_cursor;
    int v_cursor;
    int v_max;
    char tmp_name[100];

    int user_id;
    int game_id;

    user_list * user_l;
    level_list * level_l;
    game_list * game_l;

    rat_font * font;
} data_record_type;


int ck_create_tables(data_record db);

data_record init_data_record(char * file_name){
    data_record_type * db;
    db = (data_record)malloc(sizeof(data_record_type));
    if(!db){return NULL;}

    db->h_cursor = 0;
    db->tmp_name[0] = '|';
    db->tmp_name[1] = '\0';
    db->v_cursor = 0;
    db->user_l = NULL;
    db->level_l = NULL;
    db->game_l = NULL;

    db->font = rat_init();
    rat_load_font(db->font, "/Library/Fonts/Impact.ttf", 28);

    db->file_name = (char*)malloc(sizeof(file_name) + 1); 
    strcpy(db->file_name, file_name);

    ck_create_tables(db);

    return db;
}

int ck_create_tables(data_record db){
    printf("Checking to see if database is empty...\n");
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    char * extra;
    char stmt[200];
    sprintf(stmt,
            "SELECT COUNT(name) as cnt FROM sqlite_master;"
           );
    int result; 
    result = sqlite3_open(db->file_name, &sdb);
    printf("Open result: %d; ", result);
    if(result != 0 ){return 1;}
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d; ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d; ", result);
    int count  = sqlite3_column_int(sql, 0);
    printf("Count: %d\n", count);
    sqlite3_finalize(sql);

    if(count > 0){
        printf("Database Not Empty.\n");
        sqlite3_close(sdb);
        return 0;
    }
    
    
    printf("Database Empty.  Making tables...\n");
    sprintf(stmt, "CREATE TABLE user (id INTEGER PRIMARY KEY, name TEXT);");
    printf("%s\n", stmt);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d; ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    sqlite3_finalize(sql);


    sprintf(stmt, "CREATE TABLE level_stats (id INTEGER PRIMARY KEY, game_session_id NUMERIC, level_id NUMERIC, time NUMERIC, people_saved NUMERIC);");
    printf("%s\n", stmt);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d; ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    sqlite3_finalize(sql);


    sprintf(stmt, "CREATE TABLE game_session (user_id NUMERIC, id INTEGER PRIMARY KEY, date_time_start NUMERIC, date_time_end NUMERIC, deaths NUMERIC);");
    printf("%s\n", stmt);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d; ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    sqlite3_finalize(sql);

    sqlite3_close(sdb);
}

void prepare_user_list(data_record db){
    printf("Caching user list...\n");
    user_list * node = db->user_l;

    while(node != NULL){
        node = node->next;
        free(node);
    }

    sqlite3 * sdb;
    sqlite3_stmt * sql;
    char * extra;
    char stmt[] = "SELECT id, name FROM user ORDER BY name;";
    int result; 
    result = sqlite3_open(db->file_name, &sdb);
    printf("Open result: %d; ", result);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d\n", result);
    int max = 0;
    while((result = sqlite3_step(sql)) == SQLITE_ROW){
        node = (user_list*)malloc(sizeof(user_list));
        node->next = db->user_l;
        db->user_l = node;  
        node->id = sqlite3_column_int(sql, 0);
        node->name = (char*)malloc(sizeof(sqlite3_column_text(sql, 1)));
        strcpy(node->name, sqlite3_column_text(sql, 1));
        max++;
    }
    db->v_max = max-1;
    sqlite3_finalize(sql);
    sqlite3_close(sdb);
}

void render_user_list(data_record db){
	char buf[100];

	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio = glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT);
    int height = 600;
    int width = height*ratio;
   
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);

    user_list * node;
    int i = 0;
    for(node = db->user_l; node != NULL; node = node->next){
        
        float c[4]; 
        if(db->v_cursor == i){
            c[0] = 1;
            c[1] = 1;
            c[2] = 1;
            c[3] = 1;
            db->user_id = node->id;
        }
        else{
            c[0] = .1;
            c[1] = .1;
            c[2] = .1;
            c[3] = .1;
        }
        rat_set_text_color(db->font, c);
        float len;
        sprintf(buf, "%s", node->name);	
        len = rat_font_text_length(db->font, buf);
        rat_font_render_text(db->font,width/2 - 50,height-40-30*i, buf);
        i++;
    }

    /*This part will render the name typed in by the user*/

    float c[4]; 
    if(db->v_cursor == -1){
        db->tmp_name[db->h_cursor] = '|';
        c[0] = 1;
        c[1] = 1;
        c[2] = 1;
        c[3] = 1;
    }
    else{
        db->tmp_name[db->h_cursor] = ' ';
        c[0] = .1;
        c[1] = .1;
        c[2] = .1;
        c[3] = .1;
    }
    rat_set_text_color(db->font, c);
    float len;
    len = rat_font_text_length(db->font, db->tmp_name);
    rat_font_render_text(db->font,width/2 - 50,height-4, db->tmp_name);
}


void prepare_game_list(data_record db){
    printf("Caching games played list...\n");
    game_list * node = db->game_l;

    while(node != NULL){
        node = node->next;
        free(node);
    }

    sqlite3 * sdb;
    sqlite3_stmt * sql;
    char * extra;
    char stmt[] = "SELECT hlvl, "
      "name, "
      "time, "
      "hlvl, "
      "elapsed, "
      "people_saved, "
      "deaths "
      "FROM game_view LIMIT 10;";
    printf("%s\n", stmt);
    int result; 
    result = sqlite3_open(db->file_name, &sdb);
    printf("Open result: %d; ", result);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d\n", result);
    int max = 0;
    while((result = sqlite3_step(sql)) == SQLITE_ROW){
        node = (game_list*)malloc(sizeof(game_list));
        node->next = db->game_l;
        db->game_l = node;  
        node->id = sqlite3_column_int(sql, 0);
        node->name = (char*)malloc(sizeof(sqlite3_column_text(sql, 1)));
        strcpy(node->name, sqlite3_column_text(sql, 1));
        node->date = sqlite3_column_int(sql, 2);
        node->max_level = sqlite3_column_int(sql, 3);
        node->elapsed_time = sqlite3_column_int(sql, 4);
        node->people_saved = sqlite3_column_int(sql, 5);
        node->deaths = sqlite3_column_int(sql, 6);
        max++;
    }
    sqlite3_finalize(sql);
    sqlite3_close(sdb);
}


void render_game_list(data_record db){
	char buf[100];

	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio = glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT);
    int height = 600;
    int width = height*ratio;
   
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);

    game_list * node;
    int i = 0;
    for(node = db->game_l; node != NULL; node = node->next){
        float c[4]; 
        c[0] = .1;
        c[1] = .1;
        c[2] = .1;
        c[3] = .1;
        rat_set_text_color(db->font, c);
        float len;
        sprintf(buf, "%s %d %d %d %d", node->name,
                node->max_level,
                node->elapsed_time,
                node->people_saved,
                node->deaths);	
        len = rat_font_text_length(db->font, buf);
        rat_font_render_text(db->font,width/2 - 50,height-40-30*i, buf);
        i++;
    }
}

void game_start_session(data_record db){
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    char * extra;
    char stmt[200];
    sprintf(stmt,
            "INSERT INTO game_session (user_id, date_time_start) VALUES ('%d','%d');",
            db->user_id,
            time(NULL)
           );
    printf("%s\n", stmt);
    int result; 
    result = sqlite3_open(db->file_name, &sdb);
    printf("game_st Open result: %d\n", result);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d\n", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    db->game_id = sqlite3_last_insert_rowid(sdb);
    sqlite3_finalize(sql);
    sqlite3_close(sdb);
}


void game_finish_session(data_record db, int deaths){
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    char * extra;
    char stmt[200];
    sprintf(stmt,
            "UPDATE game_session SET date_time_end = '%d', deaths = '%d' WHERE id = '%d';",
            time(NULL),
            deaths,
            db->game_id
           );
    printf("%s\n", stmt);
    int result; 
    result = sqlite3_open(db->file_name, &sdb);
    printf("game_st Open result: %d\n", result);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d\n", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    db->game_id = sqlite3_last_insert_rowid(sdb);
    sqlite3_finalize(sql);
    sqlite3_close(sdb);
}

void game_record_lvl_stats(data_record db, int lvl, double time, int extra_ppl){
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    char * extra;
    char stmt[200];
    sprintf(stmt,
            "INSERT INTO level_stats (game_session_id,"
                 "level_id,"
                 "time,"
                 "people_saved) VALUES ('%d', '%d', '%f','%d');",
            db->game_id,
            lvl,
            time,
            extra_ppl
           );
    printf("%s\n", stmt);
    int result; 
    result = sqlite3_open(db->file_name, &sdb);
    printf("game_st Open result: %d\n", result);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d\n", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    sqlite3_finalize(sql);
    sqlite3_close(sdb);
}

void user_skey_down(data_record db, int key){
	switch(key) {
		case GLUT_KEY_UP : 
            db->v_cursor = db->v_cursor < 0 ? -1 : db->v_cursor - 1;
			break;
		case GLUT_KEY_DOWN : 
            db->v_cursor = db->v_cursor >= db->v_max ? db->v_max : db->v_cursor + 1;
			break;
	}
}

int user_nkey_down(data_record db, unsigned char key){
	if(key==10 || key==13){
        if(db->v_cursor == -1 && db->h_cursor > 1){
            sqlite3 * sdb;
            sqlite3_stmt * sql;
            char * extra;
            char stmt[200];
            db->tmp_name[db->h_cursor] = '\0';
            sprintf(stmt,
                    "INSERT INTO user (name) VALUES ('%s');",
                    db->tmp_name
                   );
            printf("%s\n", stmt);
            int result; 
            result = sqlite3_open(db->file_name, &sdb);
            printf("game_st Open result: %d\n", result);
            result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
            printf("prepare result: %d\n", result);
            result = sqlite3_step(sql);
            printf("Step result: %d\n", result);
            db->user_id = sqlite3_last_insert_rowid(sdb);
            sqlite3_finalize(sql);
            sqlite3_close(sdb);
        }
		return 1;
	}
	else if(key>=1 && key < 127 && db->h_cursor < 500 && db->v_cursor == -1){
		db->tmp_name[db->h_cursor]=key;
		db->h_cursor++;
		db->tmp_name[db->h_cursor+1] = '\0';
		return 0;
	}
	else if (key == 127 && db->h_cursor > 0 && db->v_cursor == -1) {
		db->h_cursor--;
		db->tmp_name[db->h_cursor+1] = '\0';
		return 0;
	}
    return 0;
}

/*prepare_level_scores();
render_level_scores();
prepare_game_scores();
render_game_scores();

record_level_stats();
*/
