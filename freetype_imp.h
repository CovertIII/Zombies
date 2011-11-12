/* I used the source found here:
 * http://musingsofninjarat.wordpress.com/opengl-freetype-texture-fonts/
 * to start this. I left the rat prefix as a credit the origonal author.
 *  Evidently this persom got it from a NeHe tutorial.
 * That code had some weaknesses in that you couldn't use it in more than 
 * one spot for multipule point sizes.  Also, some of the way you called the
 * functions was weird.  I made it better.  Here is my implimentation:
 */
typedef struct rat_texture_font
{
	float pt;
    float *wids,*hoss;
	int *qvws,*qvhs;
	float *qtws,*qths;
	unsigned int *textures;
	float ascend;
} rat_texture_font;

typedef struct rat_glyph_font
{
	float pt;
	FT_Face face;
} rat_glyph_font;


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
