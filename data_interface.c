#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <GLUT/GLUT.h>
#include <string.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include "freetype_imp.h"
#include "data_interface.h"

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
    double time;
    int people_saved;
    int max_level;
    struct game_list * next;
} game_list;

typedef struct data_record_type{
    char * file_name;
    
    int h_cursor;
    int v_cursor;
    char * tmp_name;

    int user_id;
    int game_id;

    user_list * user_l;
    level_list * level_l;
    game_list * game_l;

    rat_font * font;
} data_record_type;


data_record init_data_record(char * file_name){
    data_record_type * db;
    db = (data_record)malloc(sizeof(data_record_type));
    if(!db){return NULL;}

    db->h_cursor = 0;
    db->user_l = NULL;
    db->level_l = NULL;
    db->game_l = NULL;



    db->file_name = (char*)malloc(sizeof(file_name) + 1); 
    strcpy(db->file_name, file_name);
    
    return db;
}

void prepare_user_list(data_record db){
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
    printf("Open result: %d\n", result);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d\n", result);
    printf("id  name\n");
    while((result = sqlite3_step(sql)) == SQLITE_ROW){
        node = (user_list*)malloc(sizeof(user_list));
        node->next = db->user_l;
        db->user_l = node;  
        node->id = sqlite3_column_int(sql, 0);
        node->name = (char*)malloc(sizeof(sqlite3_column_text(sql, 1)));
        strcpy(node->name, sqlite3_column_text(sql, 1));
        printf("%d %s\n", node->id, node->name);
    }
    printf("result: %d\n", result);
    sqlite3_finalize(sql);
    sqlite3_close(sdb);
}

void render_user_list(data_record db){


}
/*prepare_level_scores();
render_level_scores();
prepare_game_scores();
render_game_scores();

record_level_stats();
game_start_session();
game_finish_session();

user_key_down();
*/
