typedef struct rat_font
{
    FT_Library ftlib;
    rat_glyph_font * gfont;
    rat_texture_font * tfont;
    float c[4];
} rat_font;

rat_font * rat_init();

void rat_load_font(rat_font * font, char * file, int pt);

void rat_free(rat_font * font);

void rat_set_text_color(rat_font * font, float *rgba);

void rat_get_text_color(rat_font * font, float *rgba);

float rat_font_height(rat_font *font);

float rat_font_text_length(rat_font *font,char *text);

float rat_font_glyph_length(rat_font *font,char ch);

void rat_font_render_text(rat_font *font,float x,float y,char *text);

// upper left corner is always zero
void rat_font_render_text_notform(rat_font *font,char *text);
