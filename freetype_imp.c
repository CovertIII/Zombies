#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GLUT/glut.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include "freetype_imp.h"


/*Delaraction of private stuctures and functions*/
static rat_glyph_font *rat_glyph_font_load(FT_Library ftlib, char *filename,int pt);
static void rat_glyph_font_destroy(rat_glyph_font *font);
static int make_glyph_texture(rat_glyph_font *gf,rat_texture_font *tf,unsigned char ch);
static rat_texture_font *rat_texture_font_from_glyph_font(rat_glyph_font *font);
static void rat_stop_font_system(rat_font * font);
static void rat_texture_font_destroy(rat_texture_font *font);

//Implementation of the public functions
rat_font * rat_init()
{
    rat_font * font;
    font = (rat_font*)malloc(sizeof(rat_font));
	FT_Init_FreeType(&font->ftlib);
    return font;
}

void rat_load_font(rat_font * font, char * file, int pt){
    font->gfont = rat_glyph_font_load(font->ftlib, file, pt);
	if (font->gfont == NULL) {
		printf("Font %s failed to load.\n", file);
		return;
	}
    font->tfont = rat_texture_font_from_glyph_font(font->gfont);
}

void rat_font_free(rat_font * font){
    rat_glyph_font_destroy(font->gfont);
    rat_texture_font_destroy(font->tfont);
    rat_stop_font_system(font);
}

void rat_set_text_color(rat_font * font, float *rgba)
{
	memcpy(font->c, rgba, 4*sizeof(float));
}

void rat_get_text_color(rat_font * font, float *rgba)
{
	memcpy(rgba, font->c, 4*sizeof(float));
}

float rat_font_height(rat_font *font)
{
	return font->tfont->pt;
}

float rat_font_text_length(rat_font *font,char *text)
{
	register float len=0;
	char *ch=text;
	for (; *ch; ch++) len+=font->tfont->wids[*ch];
	return len;
}

float rat_font_glyph_length(rat_font *font,char ch)
{
	return font->tfont->wids[ch];
}

void rat_font_render_text(rat_font *font,float x,float y,char *text)
{
	char *ch;
    rat_texture_font * tfont = font->tfont;

		glPushMatrix();
		glColor4fv(font->c);
			for (ch=text; *ch; ch++)
			{
				glPushMatrix();
					glTranslatef(x,y-(tfont->pt-tfont->hoss[*ch]),0);
					glBindTexture(GL_TEXTURE_2D,tfont->textures[*ch]);
					glBegin(GL_QUADS);
						glTexCoord2f(0,0);
						glVertex2f(0,tfont->qvhs[*ch]);

						glTexCoord2f(0,tfont->qths[*ch]);
						glVertex2f(0,0);

						glTexCoord2f(tfont->qtws[*ch],tfont->qths[*ch]);
						glVertex2f(tfont->qvws[*ch],0);

						glTexCoord2f(tfont->qtws[*ch],0);
						glVertex2f(tfont->qvws[*ch],tfont->qvhs[*ch]);
					glEnd();
				glPopMatrix();
				glTranslatef(tfont->wids[*ch],0,0);
			}
		glPopMatrix();
}

// upper left corner is always zero
void rat_font_render_text_notform(rat_font *font,char *text)
{
	char *ch;
    rat_texture_font * tfont = font->tfont;

	glPushAttrib(GL_LIST_BIT|GL_CURRENT_BIT|GL_ENABLE_BIT|GL_TRANSFORM_BIT);
		glMatrixMode(GL_MODELVIEW);
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor4fv(font->c);

		glPushMatrix();
			for (ch=text; *ch; ch++)
			{
				glBindTexture(GL_TEXTURE_2D,tfont->textures[*ch]);
				glBegin(GL_QUADS);
					glTexCoord2f(0,0);
					glVertex2f(0,tfont->qvhs[*ch]);

					glTexCoord2f(0,tfont->qths[*ch]);
					glVertex2f(0,0);

					glTexCoord2f(tfont->qtws[*ch],tfont->qths[*ch]);
					glVertex2f(tfont->qvws[*ch],0);

					glTexCoord2f(tfont->qtws[*ch],0);
					glVertex2f(tfont->qvws[*ch],tfont->qvhs[*ch]);
				glEnd();
				glTranslatef(tfont->wids[*ch],0,0);
			}
		glPopMatrix();
	glPopAttrib();
}

//More or less the private functions of this class
static void rat_stop_font_system(rat_font * font)
{
	FT_Done_FreeType(font->ftlib);
}

static rat_glyph_font *rat_glyph_font_load(FT_Library ftlib, char *filename,int pt)
{
	rat_glyph_font *font=(rat_glyph_font *)malloc(sizeof(rat_glyph_font));

	printf("Loading font from file \"%s\" at ptsize %i...",filename,pt);

	// load the font from the file
	if (FT_New_Face(ftlib,filename,0,&(font->face)))
	{
		printf("failed load!\n");
		free((void *)font);
		return NULL;
	}

	//FT_Set_Char_Size(font->face,0,pt*64,72,72);
    FT_Set_Pixel_Sizes(font->face, 0, pt);
	font->pt=pt;

	printf("done.\n");
	return font;
}

static void rat_glyph_font_destroy(rat_glyph_font *font)
{
	printf("Destroying glyph font...");
	FT_Done_Face(font->face);
	free((void *)font);
	printf("done.\n");
}

inline static unsigned int _pow2(unsigned int i)
{
	register unsigned int p2;
	for (p2=1; p2<i; p2<<=1);
	return p2;
}

static int make_glyph_texture(rat_glyph_font *gf,rat_texture_font *tf,unsigned char ch)
{
	register unsigned int i,j;
	FT_Face face=gf->face;
	unsigned int *textures=tf->textures;
	unsigned int width,height;
	float texx,texy;

	if (FT_Load_Glyph(face,FT_Get_Char_Index(face,ch),FT_LOAD_DEFAULT))
		return 0;

    FT_Glyph glyph;
    if (FT_Get_Glyph(face->glyph,&glyph))
		return 0;

	FT_Glyph_To_Bitmap(&glyph,ft_render_mode_normal,0,1);
    FT_BitmapGlyph bitmap_glyph=(FT_BitmapGlyph)glyph;

	FT_Bitmap bitmap=bitmap_glyph->bitmap;

	width=_pow2(bitmap.width);
	height=_pow2(bitmap.rows);

	GLubyte* expanded_data=(GLubyte *)malloc(sizeof(GLubyte)*2*width*height);

	for (j=0; j<height;j++)
	{
		for (i=0; i<width; i++)
		{
			expanded_data[2*(i+j*width)]=
			expanded_data[2*(i+j*width)+1]=
				(i>=bitmap.width||j>=bitmap.rows)?
				0:bitmap.buffer[i+bitmap.width*j];
		}
	}

    glBindTexture(GL_TEXTURE_2D,textures[ch]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA16,width,height,
		0,GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE,expanded_data);

    free((void *)expanded_data);

	tf->wids[ch]=(float)(face->glyph->advance.x/64);
	tf->hoss[ch]=(float)((face->glyph->metrics.horiBearingY-face->glyph->metrics.height)>>6);

	tf->qvws[ch]=bitmap.width;
	tf->qvhs[ch]=bitmap.rows;

	tf->qtws[ch]=(float)bitmap.width/(float)width;
	tf->qths[ch]=(float)bitmap.rows/(float)height;

	return 1;
}

static rat_texture_font *rat_texture_font_from_glyph_font(rat_glyph_font *font)
{
	register unsigned char i;
	rat_texture_font *tf=(rat_texture_font *)malloc(sizeof(rat_texture_font));

	tf->pt=font->pt;

	// prepare the OpenGL textures / display lists
	tf->wids=(float *)malloc(sizeof(float)*255);
	tf->hoss=(float *)malloc(sizeof(float)*255);
	tf->qvws=(int *)malloc(sizeof(int)*255);
	tf->qvhs=(int *)malloc(sizeof(int)*255);
	tf->qtws=(float *)malloc(sizeof(float)*255);
	tf->qths=(float *)malloc(sizeof(float)*255);
	tf->textures=(unsigned int *)malloc(sizeof(unsigned int)*255);
	glGenTextures(255,tf->textures);

	for (i=0;i<255;i++)
	{
		if (!make_glyph_texture(font,tf,i))
		{
			glDeleteTextures(255,tf->textures);
			free((void *)tf->textures);
			free((void *)tf->wids);
			free((void *)tf->hoss);
			free((void *)tf->qvws);
			free((void *)tf->qvhs);
			free((void *)tf->qtws);
			free((void *)tf->qths);
			free((void *)tf);
			return NULL;
		}
	}

	return tf;
}

static void rat_texture_font_destroy(rat_texture_font *font)
{
	glDeleteTextures(255,font->textures);
	free((void *)font->wids);
	free((void *)font->textures);
	free((void *)font->qvws);
	free((void *)font->qvhs);
	free((void *)font->qtws);
	free((void *)font->qths);
	free((void *)font);
}
