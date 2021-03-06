#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include </usr/include/time.h>
#include <math.h>
#include <sqlite3.h>
#include <curl/curl.h>
#include <GLUT/GLUT.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
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
    int level_id;
    char * slevel_id;
    unsigned char * name;
    char * date_string;
    double time;
    int num_saved;
    struct level_list * next;
} level_list;

typedef struct game_list{
    int id;
    char * name;
    int max_level;
    unsigned long int date;
    char * date_string;
    int elapsed_time;
    int people_saved;
    int deaths;
    struct game_list * next;
} game_list;

typedef struct data_record_type{
    char * file_name;
		
	char * res_path;
	char * res_buf;		
    
    int disp;
    int h_cursor;
    int v_cursor;
    int v_max;
    char tmp_name[100];

    int max_level;
    int show_level;

    int user_id;
    int save_count;
    int game_id;

    user_list * user_l;
    level_list * level_l;
    game_list * game_l;

    rat_font * font;
} data_record_type;


int ck_create_tables(data_record db);
void prepare_user_list(data_record db);
void prepare_game_list(data_record db);
void prepare_level_scores(data_record db);

data_record init_data_record(char * file_name, char * resource_path){
    data_record_type * db;
    db = (data_record)malloc(sizeof(data_record_type));
    if(!db){return NULL;}

    db->h_cursor = 0;
    db->disp = 0;
    db->tmp_name[0] = '|';
    db->tmp_name[1] = '\0';
    db->v_cursor = -1;
    db->user_l = NULL;
    db->level_l = NULL;
    db->game_l = NULL;
    db->show_level = 1;

    db->res_path = (char*)malloc(strlen(resource_path)*sizeof(char)+sizeof(char)*100);
    db->res_buf  = (char*)malloc(strlen(resource_path)*sizeof(char)+sizeof(char)*100);
    strcpy(db->res_path, resource_path);
	printf("data_interfae.c - path mem: %ld\n", strlen(resource_path)*sizeof(char)+sizeof(char)*50);

	
    db->font = rat_init();
    strcpy(db->res_buf, db->res_path);
    strcat(db->res_buf, "/imgs/FreeUniversal.ttf");
    rat_load_font(db->font, db->res_buf, 28);

    db->file_name = (char*)malloc(sizeof(file_name) + 2); 
    strcpy(db->file_name, file_name);

    ck_create_tables(db);

    return db;
}

void stats_reload_fonts(data_record db){
	db->font = rat_init();
    strcpy(db->res_buf, db->res_path);
    strcat(db->res_buf, "/imgs/FreeUniversal.ttf");
    rat_load_font(db->font, db->res_buf, 28);
}

int ck_create_tables(data_record db){
    printf("Checking to see if database is empty...\n");
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[700];
    sprintf(stmt,
            "SELECT COUNT(name) as cnt FROM sqlite_master;"
           );
    int result; 
    printf("Opening database file %s\n", db->file_name);
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
    sprintf(stmt, "CREATE TABLE user (id INTEGER PRIMARY KEY, name TEXT, lives NUMERIC DEFAULT 3, continues INTEGER DEFAULT 0, deaths INTEGER DEFAULT 0);");
    printf("%s\n", stmt);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d; ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    sqlite3_finalize(sql);


    sprintf(stmt, "CREATE TABLE level_stats (id INTEGER PRIMARY KEY, game_session_id NUMERIC, level_id TEXT, datetime INTEGER, time NUMERIC, people_saved NUMERIC, user_id INTEGER);");
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

    sprintf(stmt, "CREATE VIEW game_view AS SELECT user.name, "
	"game_session.date_time_start, "
	"datetime(game_session.date_time_start, 'unixepoch', 'localtime') as time, "
        "MAX(level_stats.level_id) AS hlvl, "
	"game_session.date_time_end - game_session.date_time_start AS elapsed, "
        "SUM(level_stats.people_saved) as people_saved, "
	"game_session.deaths "
    "FROM game_session "
    "JOIN user ON (game_session.user_id = user.id) "
    "JOIN level_stats ON (level_stats.game_session_id = game_session.id) "
    "GROUP BY level_stats.game_session_id "
    "ORDER BY hlvl DESC, elapsed;");
    printf("%s\n", stmt);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d; ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    sqlite3_finalize(sql);
    
    sprintf(stmt, "CREATE VIEW level_view AS "
        "SELECT user.name, "
        "game_session.date_time_start, "
        "level_stats.level_id, "
        "level_stats.time, "
        "level_stats.people_saved, "
        "level_stats.people_saved / level_stats.time*1000 score "
        "FROM level_stats "
        "JOIN game_session ON (game_session.id = level_stats.game_session_id) "
        "JOIN user ON (user.id = game_session.user_id);");
    printf("%s\n", stmt);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
    printf("prepare result: %d; ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    sqlite3_finalize(sql);

    sqlite3_close(sdb);
	return 1;
}

void stats_list_prep(data_record db){
    db->h_cursor = 0;
    db->disp = 0;
    db->tmp_name[0] = '|';
    db->tmp_name[1] = '\0';
    db->v_cursor = -1;
    db->show_level = 1;

    prepare_user_list(db);
    prepare_game_list(db);
    prepare_level_scores(db);
}


void prepare_user_list(data_record db){
    printf("Caching user list...\n");
    user_list * node = db->user_l;
    user_list * temp;

    while(node != NULL){
        temp = node;
        node = node->next;
        free(temp);
    }

    db->user_l = NULL;

    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[] = "SELECT id, name FROM user ORDER BY name COLLATE NOCASE DESC;";
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

void render_user_list(data_record db, int width, int height){
	char buf[100];

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
            c[0] = 0;
            c[1] = 0;
            c[2] = 0;
            c[3] = 1;
        }
        rat_set_text_color(db->font, c);
				if(i < 10){
					rat_font_render_text(db->font,width/2 - 250,height-180-30*i, node->name);	
				}
				else{
					rat_font_render_text(db->font,width/2  + 150,height-180-30*(i-10), node->name);
				}
        

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
        c[0] = 0;
        c[1] = 0;
        c[2] = 0;
        c[3] = 1;
    }
    rat_set_text_color(db->font, c);
 		float len;
		len = rat_font_text_length(db->font, db->tmp_name);
    rat_font_render_text(db->font,(width - len)/2,height-140, db->tmp_name);

		c[0] = 0;
    c[1] = 0;
    c[2] = 0;
    c[3] = 1;
		rat_set_text_color(db->font, c);
		
		if(db->user_l == NULL){
			sprintf(buf, "Type in your name");	
		}
		else{
			sprintf(buf, "Type in your name or select it below");	
		}
		len = rat_font_text_length(db->font, buf);
		rat_font_render_text(db->font,(width - len)/2 ,height-104, buf);
}


void prepare_game_list(data_record db){
    printf("Caching games played list...\n");
    game_list * node = db->game_l;
    game_list * temp;

    while(node != NULL){
        temp = node;
        node = node->next;
        free(temp);
    }

    db->game_l = NULL;

    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[] = "SELECT hlvl, "
      "name, "
      "date_time_start, "
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
        if(db->game_l == NULL){
            db->game_l = node;
        }
        else {
            game_list * findend = db->game_l;
            game_list * prev = NULL;
            while(findend != NULL){
                prev = findend;
                findend = findend->next;
            }
            prev->next = node;
        }
        node->next = NULL;

        //Fill up the structure with data
        node->id = sqlite3_column_int(sql, 0);
        node->name = (char*)malloc(sizeof(sqlite3_column_text(sql, 1)));
        strcpy(node->name, sqlite3_column_text(sql, 1));

        node->date = sqlite3_column_int(sql, 2);

		char buf[200];
        time_t stamp = (time_t)node->date;
		struct tm * tmptr = gmtime(&stamp);
        strftime(buf, 200, "%b %d, %Y", tmptr);
        node->date_string = (char*)malloc(strlen(buf)*sizeof(char)+1);
        strcpy(node->date_string, buf);
      
        node->max_level = sqlite3_column_int(sql, 3);
        node->elapsed_time = sqlite3_column_int(sql, 4);
        node->people_saved = sqlite3_column_int(sql, 5);
        node->deaths = sqlite3_column_int(sql, 6);
        max++;
    }

    sqlite3_finalize(sql);
    sqlite3_close(sdb);

}


void render_game_list(data_record db, int width, int height){
	char buf[100];

    float c[4]; 
    c[0] = .1;
    c[1] = .1;
    c[2] = .1;
    c[3] = 1;
    rat_set_text_color(db->font, c);

    //Prints the header
    sprintf(buf, "Game Highscores");
    rat_font_render_text(db->font,width/2,height-7, buf);

    sprintf(buf, "Name");
    rat_font_render_text(db->font,width/2 - 250,height-40, buf);

    sprintf(buf, "L");
    rat_font_render_text(db->font,width/2 - 25,height-40, buf);

    sprintf(buf, "Time");
    rat_font_render_text(db->font,width/2 + 25,height-40, buf);

    sprintf(buf, "S");
    rat_font_render_text(db->font,width/2 + 130,height-40, buf);

    sprintf(buf, "D");
    rat_font_render_text(db->font,width/2 + 200,height-40, buf);


    //Printf the data of the game high scores.
    c[0] = .2;
    c[1] = .1;
    c[2] = .2;
    c[3] = 1;
    rat_set_text_color(db->font, c);
    game_list * node;
    int i = 0;
    for(node = db->game_l; node != NULL; node = node->next){
        float len;
        
        //sprintf(buf, "%ld", node->date);
        rat_font_render_text(db->font,10,height-80-30*i, node->date_string);

        sprintf(buf, "%s", node->name);
        len = rat_font_text_length(db->font, buf);
        rat_font_render_text(db->font,width/2 - 220,height-80-30*i, buf);

        sprintf(buf, "%d", node->max_level);
        len = rat_font_text_length(db->font, buf);
        rat_font_render_text(db->font,width/2 - 25,height-80-30*i, buf);

        int min = floor(node->elapsed_time/60);
        int sec = node->elapsed_time%60;

        sprintf(buf, "%d : %d", min, sec);
        len = rat_font_text_length(db->font, buf);
        rat_font_render_text(db->font,width/2 + 25,height-80-30*i, buf);

        sprintf(buf, "%d", node->people_saved);
        len = rat_font_text_length(db->font, buf);
        rat_font_render_text(db->font,width/2 + 130,height-80-30*i, buf);

        sprintf(buf, "%d", node->deaths);
        len = rat_font_text_length(db->font, buf);
        rat_font_render_text(db->font,width/2 + 200,height-80-30*i, buf);
        i++;
    }
}

int stats_render(data_record db, int width, int height){

	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio = (double)width/(double)height; 
    height = 600;
    width = height*ratio;
   
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);


    switch(db->disp){
        case 0:
            render_user_list(db, width, height);
			return 0;
            break;
        case 2:
            render_level_scores(db, width, height);
			return 2;
            break;
    }
}

void game_start_session(data_record db){
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[200];
    sprintf(stmt,
            "INSERT INTO game_session (user_id, date_time_start) VALUES ('%d','%d');",
            db->user_id,
            (int)time(NULL)
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


void game_over(data_record db, int deaths){
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[200];
    int result; 

    sprintf(stmt,
            "UPDATE user SET continues = continues + 1, lives = 3, deaths = deaths + %d WHERE id = '%d';",
            deaths,
            db->user_id
           );
    printf("%s\n", stmt);
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


void game_leave(data_record db, int lives, int deaths){
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[200];
    int result; 

    sprintf(stmt,
            "UPDATE user SET lives = '%d', deaths = deaths + '%d' WHERE id = '%d';",
            lives,
            deaths,
            db->user_id
           );
    printf("%s\n", stmt);
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

/*
 * SQL Code to get save count for all levels
  
SELECT SUM(A.mp) as cur_ct FROM
  (SELECT MAX(people_saved) mp, level_id 
    FROM level_stats 
    WHERE user_id = 1
    GROUP BY level_id) as A;
*/
int db_get_save_count(data_record db){
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[200];
    int result;

    sprintf(stmt, "SELECT SUM(A.mp) as cur_ct FROM "
                     "(SELECT MAX(people_saved) mp, level_id"
                     " FROM level_stats "
                     " WHERE user_id = %d"
                     " GROUP BY level_id) as A;", db->user_id);
    printf("%s\n", stmt);
    result = sqlite3_open(db->file_name, &sdb);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1, &sql, &extra);
    printf("prepare result: %d  ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    int save_count = sqlite3_column_int(sql, 0);
    printf("Save Count for %d: %d\n", db->user_id, save_count);
    sqlite3_finalize(sql);
    sqlite3_close(sdb);

    return save_count;
}


int db_get_lives(data_record db){
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[200];
    int result;

    sprintf(stmt, "SELECT lives FROM user WHERE id = %d", db->user_id);
    printf("%s\n", stmt);
    result = sqlite3_open(db->file_name, &sdb);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1, &sql, &extra);
    printf("prepare result: %d  ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    int lives = sqlite3_column_int(sql, 0);
    printf("Lives for %d: %d\n", db->user_id, lives);
    sqlite3_finalize(sql);
    sqlite3_close(sdb);

    return lives;
}

int db_get_continues(data_record db){
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[200];
    int result;

    sprintf(stmt, "SELECT continues FROM user WHERE id = %d", db->user_id);
    printf("%s\n", stmt);
    result = sqlite3_open(db->file_name, &sdb);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1, &sql, &extra);
    printf("prepare result: %d  ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    int lives = sqlite3_column_int(sql, 0);
    printf("Lives for %d: %d\n", db->user_id, lives);
    sqlite3_finalize(sql);
    sqlite3_close(sdb);

    return lives;
}

int db_get_user_id(data_record db){
    return db->user_id;
}



void game_record_lvl_stats(data_record db, char * lvl, double time_lvl, int extra_ppl){
    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[200];
    sprintf(stmt,
            "INSERT INTO level_stats (game_session_id,"
                 "level_id,"
                 "time,"
                 "datetime,"
                 "user_id,"
                 "people_saved) VALUES ('%d', '%s', '%f','%d','%d','%d');",
            db->game_id,
            lvl,
            time_lvl,
            (int)time(NULL),
            db->user_id,
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
    prepare_level_scores(db);

    /*
    sprintf(stmt, "SELECT name FROM user WHERE id = %d;", db->user_id);
    printf("%s\n", stmt);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1, &sql, &extra);
    printf("prepare result: %d  ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    char * tmp_name = (char*)malloc(sizeof(char)*strlen(sqlite3_column_text(sql, 0)));
    strcpy(tmp_name, sqlite3_column_text(sql, 0));
    printf("Current Name: %s\n", tmp_name);
    sqlite3_finalize(sql);

    //This will push the stats to a server
    CURL *curl;
    CURLcode res;
     
     struct curl_httppost *formpost=NULL;
     struct curl_httppost *lastptr=NULL;
     
     curl_global_init(CURL_GLOBAL_ALL);

     int current_time = (int)time(NULL);
     char str_buf[100];
     
     // Fill in the filename field 
     sprintf(str_buf, "%d\0", lvl);
     curl_formadd(&formpost,
                  &lastptr,
                  CURLFORM_COPYNAME, "level_id",
                  CURLFORM_COPYCONTENTS, str_buf,
                  CURLFORM_END);
     
     curl_formadd(&formpost,
                  &lastptr,
                  CURLFORM_COPYNAME, "name",
                  CURLFORM_COPYCONTENTS, tmp_name,
                  CURLFORM_END);

     sprintf(str_buf, "%d\0", current_time);
     curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "date_time",
                 CURLFORM_COPYCONTENTS, str_buf,
                 CURLFORM_END);
    
     sprintf(str_buf, "%d\0", extra_ppl);
     curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "save_count",
                 CURLFORM_COPYCONTENTS, str_buf,
                 CURLFORM_END);

     sprintf(str_buf, "%f\0", time_lvl);
     curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "time",
                 CURLFORM_COPYCONTENTS, str_buf,
                 CURLFORM_END);
     curl = curl_easy_init();
     if(curl) {
       // what URL that receives this POST  
       curl_easy_setopt(curl, CURLOPT_URL, "http://thewordandprayer.com/Zombies/level_insert.php");
       curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
       res = curl_easy_perform(curl);
    
        // always cleanup  
        curl_easy_cleanup(curl);
     
        // then cleanup the formpost chain  
        curl_formfree(formpost);
      }
      */
}

void user_skey_down(data_record db, int key){
	switch(key) {
		case SDLK_UP : 
            db->v_cursor = db->v_cursor < 0 ? -1 : db->v_cursor - 1;
			break;
		case SDLK_DOWN : 
            db->v_cursor = db->v_cursor >= db->v_max ? db->v_max : db->v_cursor + 1;
			break;
		case SDLK_LEFT : 
            if(db->disp == 0){
                db->disp = 2;
                db->show_level = db->max_level;
            }
            else if(db->disp == 2){
                db->show_level--;
                if(db->show_level < 1){
                    db->disp = 0;
                }
            }
			break;
		case SDLK_RIGHT : 
            if(db->disp == 0){
                db->disp = 2;
                db->show_level = 1;
            }
            else if(db->disp == 2){
                db->show_level++;
                if(db->show_level > db->max_level){
                    db->disp = 0;
                }
            }
			break;
	}
}

int user_nkey_down(data_record db, unsigned char key){
    if(db->disp == 0){
        if(key==10 || key==13){
            if(db->v_cursor == -1 && db->h_cursor > 1 && strlen(db->tmp_name) > 3){
                printf("%s %d\n", db->tmp_name, strlen(db->tmp_name));
                sqlite3 * sdb;
                sqlite3_stmt * sql;
                const char * extra;
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
        else if(((key >= 97 && key < 127) || key == 32) && db->h_cursor < 500 && db->v_cursor == -1){
            db->tmp_name[db->h_cursor]=key;
            db->h_cursor++;
            db->tmp_name[db->h_cursor+1] = '\0';
            return 0;
        }
        else if ((key == SDLK_DELETE || key == 8) && db->h_cursor > 0 && db->v_cursor == -1) {
            db->h_cursor--;
            db->tmp_name[db->h_cursor+1] = '\0';
            return 0;
        }
    }
    return 0;
}


void prepare_level_scores(data_record db){
    printf("Caching level stats...\n");

    sqlite3 * sdb;
    sqlite3_stmt * sql;
    const char * extra;
    char stmt[300];
    printf("%s\n", stmt);
    int result; 

    result = sqlite3_open(db->file_name, &sdb);
    printf("Open result: %d; ", result);

    sprintf(stmt, "SELECT COUNT(DISTINCT level_id) FROM level_stats;");
    printf("%s\n", stmt);
    result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1, &sql, &extra);
    printf("prepare result: %d  ", result);
    result = sqlite3_step(sql);
    printf("Step result: %d\n", result);
    db->max_level = sqlite3_column_int(sql, 0);
    printf("Max lvl: %d \n", db->max_level);

    sqlite3_finalize(sql);


    level_list * node = db->level_l;
    level_list * temp;

    while(node != NULL){
        temp = node;
        node = node->next;
        free(temp);
    }

    db->level_l = NULL;

    int i;
    for (i = 1; i <= db->max_level; i++) {
        sprintf(stmt, "SELECT CAST(trim(level_id,'lvl') AS INTEGER) AS lvl_int, level_id, datetime, user.name, time, people_saved FROM level_stats LEFT OUTER JOIN user ON (user.id = level_stats.user_id) WHERE lvl_int = %d ORDER BY people_saved DESC, time LIMIT 10;", i);
        result = sqlite3_prepare_v2(sdb, stmt, sizeof(stmt) + 1 , &sql, &extra);
        printf("prepare result: %d\n", result);
        
        while((result = sqlite3_step(sql)) == SQLITE_ROW){
            node = (level_list*)malloc(sizeof(level_list));
            if(db->level_l == NULL){
                db->level_l = node;
            }
            else {
                level_list * findend = db->level_l;
                level_list * prev = NULL;
                while(findend != NULL){
                    prev = findend;
                    findend = findend->next;
                }
                prev->next = node;
            }
            node->next = NULL;

            
            //Fill up the structure with data
            node->name = (char*)malloc(sizeof(char)*strlen(sqlite3_column_text(sql, 3)));
            strcpy(node->name, sqlite3_column_text(sql, 3));

            node->slevel_id = (char*)malloc(sizeof(char)*strlen(sqlite3_column_text(sql, 1)));
            strcpy(node->slevel_id, sqlite3_column_text(sql, 1));
            
            time_t stamp = sqlite3_column_int(sql, 2);
            const struct tm * timeptr = localtime(&stamp);
            unsigned char nbuf[100];		
            strftime(nbuf, 100, "%b %d, %Y", timeptr);
            node->date_string = (char*)malloc(sizeof(char)*strlen(nbuf)+1);
            strcpy(node->date_string, nbuf);

            node->level_id = sqlite3_column_int(sql, 0);
            node->time = sqlite3_column_double(sql, 4);
            node->num_saved = sqlite3_column_int(sql, 5);
        }

        sqlite3_finalize(sql);
    }

    sqlite3_close(sdb);
}

void stats_render_lvl(data_record db, char * lvl, int width, int height){
    char buf[7];
    int lvl_num;
	sscanf(lvl, "lvl%d", &lvl_num);
    if(lvl_num <= db->max_level){
        db->show_level = lvl_num;
        glPushMatrix();
        glTranslatef(0, -200, 0);
        render_level_scores(db, width, height);
        glPopMatrix();
    }
}

void render_level_scores(data_record db, int width, int height)
{
	char buf[100];

    float c[4]; 
    c[0] = .1;
    c[1] = .1;
    c[2] = .1;
    c[3] = 1;
    rat_set_text_color(db->font, c);

    //Prints the header
    sprintf(buf, "Level %d", db->show_level);
    rat_font_render_text(db->font,width/2,height-7, buf);

    sprintf(buf, "Name");
    rat_font_render_text(db->font,width/2 - 250,height-40, buf);

    sprintf(buf, "Time");
    rat_font_render_text(db->font,width/2 - 25,height-40, buf);

    sprintf(buf, "S");
    rat_font_render_text(db->font,width/2 + 150,height-40, buf);

    //Printf the data of the game high scores.
    c[0] = .2;
    c[1] = .1;
    c[2] = .2;
    c[3] = 1;
    rat_set_text_color(db->font, c);
    level_list * node;
    int i = 0;
    for(node = db->level_l; node != NULL; node = node->next){
        if(node->level_id == db->show_level){
            float len;

            rat_font_render_text(db->font,10,height-80-30*i, node->date_string);

            sprintf(buf, "%s", node->name);
            len = rat_font_text_length(db->font, buf);
            rat_font_render_text(db->font,width/2 - 225,height-80-30*i, buf);

            int min = floor((int)node->time/60);
            double sec = (int)node->time%60 + node->time - floor((int)node->time);

            sprintf(buf, "%d : %2.2f", min, sec);
            len = rat_font_text_length(db->font, buf);
            rat_font_render_text(db->font,width/2 - 25,height-80-30*i, buf);

            sprintf(buf, "%d", node->num_saved);
            len = rat_font_text_length(db->font, buf);
            rat_font_render_text(db->font,width/2 + 150,height-80-30*i, buf);
            i++;
        }
        if(i > 15){break;}
    }
}

