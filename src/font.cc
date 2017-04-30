/* font.cc
   Manage textured-font for drawing text

   Copyright (C) 2000  Mathias Broxvall
   		       Yannick Perret

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string.h>
#include <math.h>
#include "general.h"
#include "glHelp.h"
#include "animated.h"
#include "map.h"
#include "game.h"
#include "font.h"
#include "image.h"

using namespace std;


//
//
//  sorry, this code is a port from a C project, so it is not really
//    coherent with the rest of the code of trackballs.
//
//


/* data for font image */
unsigned char *font_data=NULL;
#define CHAR_W 32
#define CHAR_H 32
#define BCHAR_W 4
#define BCHAR_H 32
#define CHARS_W 512
#define CHARS_H 256
#define CHAR_WNB 16
#define CHAR_HNB 8
int char_corresp[256]=
{'A','Q','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
 'U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j',
 'o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3',
 '8','9',',',';',':','.','!','?','*','$','=','(',')','/','\\','§',
 '%','&','"','{','}','[',']','|','`','\'','_','^','@','°','£','¤',
 '\0','\0',256+'"',256+'C',256+'R','\0','\0',256+'<',256+'>','Å','Ä','Ö','å','ä','ö',' ',
 '\0',256+'?',256+'*',256+'O',256+'/',256+'o','P','R','S','T','k','l','m','n',256+'f',' ',
 '\0','4','5','6','7','-','+','<','>','#','~',256+'!','\0',256+'X',256+'V',256+'l',
};


/* access table */
/* Index by ascii value + 128 (so we can handle 8 bit ascii (negative values!) correctly. */
int char_access[2048]={0};

/* the theoretical ratio screen_x/screen_y */
#define SCREEN_RATIO (640./480.)

Font *current_fonts[8] = {NULL};
int nb_fonts = 0;

// global list of text drawn
TextLine textlines[MAX_TEXT_LINE];
int max_line_used = 0;

#define BASE_TIME 0.2


static Real total_time = 0.;

void Font::init() {
  int i;
  
  nb_fonts = 1;
  current_fonts[0] = new Font("font.png");
  //current_fonts[1] = new Font("font2.png");
  //current_fonts[2] = new Font("font3.png");

  // initialize texts
  for(i=0; i<MAX_TEXT_LINE; i++) {
  	textlines[i].text = NULL;
  	textlines[i].font = NULL;
  }
  max_line_used = 0;

}

// load the font texture and initialize all access data
Font::Font(const char *fname) {

  this->fontName = strdup(fname);
  this->color[0] = 1.;
  this->color[1] = 1.;
  this->color[2] = 1.;
  this->color[3] = 1.;
  this->fontTex = loadTexture(this->fontName, this);
  
}


Font::~Font() {
}

int Font::figureFontInfo(void *pixels, int tx, int ty)
{
  int i, j, jj, x, xs, k, y;
  if((tx != CHARS_W)||(ty != CHARS_H))
  {
    fprintf(stderr, "Font image is not %dx%d. Aborted.\n", CHARS_W, CHARS_H);
    return 0;
  }
  if(pixels == NULL)
    {
    fprintf(stderr, "Font image not loaded.\n");
    return 0;
    }

  /* create access table for chars */
  /* Changed to handle 8bit ascii */
  memset(char_access,0,2048);
  for(i=0;i<256;i++) {
	j=char_corresp[i]+128;
	if(j >= 0 && j < 2048)
	  char_access[j]=i;
  }

  /*
  for(i=0; i<2048; i++)
    {
    for(j=0; j<512; j++)
      {
      if (char_corresp[j] == i)
        {
        char_access[i] = j;
        break;
        }
      }
	  }*/

  /* Compute the X extention for each letter (the last non-transparent
     pixel for each letter area (32x32) */
  k = 0;
  /* For each char */
  for(jj=0; jj<CHAR_HNB; jj++)
    {
    j = CHAR_HNB-1-jj;
    for(i=0; i<CHAR_WNB; i++)
      {
      /* Check for pixels */
      xs = 0;
      for(x=CHAR_W-1; x>=0; x--)
        {
	for(y=0; y<CHAR_H; y++)
	  {
	    if ((((char *)pixels)[(i*CHAR_W
			       +x
			       +(j*CHAR_WNB*CHAR_W*CHAR_H+y*CHAR_WNB*CHAR_W))
			      *4+3]) != 0)
	    {/* Not transparent. It is the letter */
	    xs = x;
	    x = -1;  /* hard exit for loop 1 /o) */
	    break;   /* exit from loop 2 */
	    }
	  }
	}
      if (xs == 0)
        {
	this->char_sizes[k] = 0.;
	}
      else
      if (xs == CHAR_W-1)
        {
	this->char_sizes[k] = 1.;
	}
      else
        {
	this->char_sizes[k] = (float)(xs+1)/CHAR_W;
	}
      k++;
      }
    }
  /* now correct special characters (in particular the SPACE) */
  for(k=0; k<CHAR_HNB*CHAR_WNB; k++)
    {
    if (char_corresp[k] == ' ')
      {
      this->char_sizes[k] = .3;  /* SPACE is empty but correspond */
      /* to a real extention */
      }
    }

  /* Treat special '"' and '°' to not move cursor forward */
  /*
  this->char_sizes[5*CHAR_WNB+2]=0;
  this->char_sizes[4*CHAR_WNB+14]=0;
  */
}

int Font::addText(int font, const char *text, int x, int y, float sx, float sy, double delay,
                  double offset, double stab, double ampl, int fix) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	return(current_fonts[font]->l_addText(text, x, y, sx, sy, delay,
                           offset, stab, ampl, fix));
}


// add a text to be drawn (returns the ID of the text)
int Font::l_addText(const char *text, int x, int y, float sx, float sy, double delay,
                    double offset, double stab, double ampl, int fix) {
  int i, j;
  double cur;

  if ((text == NULL)||(strlen(text) == 0))
    return(-1);

  // looking for a free place
  for(i=0; i<MAX_TEXT_LINE; i++) {
  	if (textlines[i].text == NULL) {
		break;
	}
  }
  if (i == MAX_TEXT_LINE) {
  	fprintf(stderr, "Too many text drawn. Ignored.\n");
	return(-1);
  }
  // add the entry
  textlines[i].time = (float*)malloc(sizeof(float)*strlen(text));
  if (textlines[i].time == NULL) {
  	fprintf(stderr, "Connot perform allocation for new text. Ignored.\n");
	return(-1);
  }
  textlines[i].text = strdup(text);
  textlines[i].length = strlen(text);
  cur = -delay;
  for(j=0; j<textlines[i].length; j++) {
  	textlines[i].time[j] = cur;
	cur -= offset;
  }
  textlines[i].removing = 0;
  textlines[i].stable = 0;
  textlines[i].pulse = 0;
  textlines[i].x = x;
  textlines[i].y = y;
  textlines[i].sizex = sx;
  textlines[i].sizey = sy;
  if (stab < BASE_TIME)
    stab = BASE_TIME;
  textlines[i].tmax  = stab;
  textlines[i].amplitude = ampl;

  // BBOX values
  textlines[i].x0 = (int)(x - sx);
  textlines[i].x1 = (int)(x - sx + l_getTextWidth(text, sx));
  textlines[i].y0 = (int)(y - sy);
  textlines[i].y1 = (int)(y + sy);

  // which font
  textlines[i].font = this;

  textlines[i].time_pulse = 0.;

  if (fix == 1) {
  	for(j=0; j<textlines[i].length; j++) {
  		textlines[i].time[j] = -delay;
	}
  textlines[i].stable = 0;
  textlines[i].tmax = BASE_TIME;
  }

  if (fix == 2) {
  	for(j=0; j<textlines[i].length; j++) {
  		textlines[i].time[j] = BASE_TIME;
	}
  textlines[i].stable = 1;
  textlines[i].tmax = BASE_TIME;
  }

  // update max_line
  if (i >= max_line_used)
    max_line_used = i+1;
  return(1);
}

// remove really all texts
void Font::destroyAllText() {
  int i;

  for(i=0; i<max_line_used; i++) {
    destroyText(i);
  }
  max_line_used = 0;
}

// remove really the entry
void Font::destroyText(int tl) {
  if (tl < 0)
    return;
  if (textlines[tl].text == NULL)
    return;

  if (textlines[tl].text != NULL)
    free(textlines[tl].text);
  textlines[tl].text = NULL;
  if (textlines[tl].time != NULL)
    free(textlines[tl].time);
  textlines[tl].time = NULL;
  textlines[tl].stable = 0;
  textlines[tl].removing = 0;
  textlines[tl].length = 0;

  if (tl == max_line_used-1) {
    max_line_used--;
    if (max_line_used < 0)
      max_line_used = 0;
  }
}

// remove a text
void Font::l_delText(int tl) {
  if (tl < 0)
    return;
  if (textlines[tl].text == NULL)
    return;

  textlines[tl].removing = 1;
  textlines[tl].stable   = 0;
}

// remove all text
void Font::delAllText() {
  int i;

  for(i=0; i<MAX_TEXT_LINE; i++) {
    if (textlines[i].text != NULL) {
      textlines[i].removing = 1;
      textlines[i].stable   = 0;
    }
  }
}
// remove all text entries instantanly
void Font::l_destroyAllText() {
  int i;

  for(i=0; i<MAX_TEXT_LINE; i++) {
    if (textlines[i].text != NULL) {
      destroyText(i);
    }
  }
  max_line_used = 0;
}

/* find letter index */
static int get_letter_index_simple(int txt)
{
  return(char_access[txt+128]);
}


/* takes the address of the char because it is planed
   to extend the text with control sequences, in order to
   access to additionnal characters or functions */
/* shift is the number of control characters read */
static int get_letter_index(const char *txt, int *shift)
{
  *shift = 0;

  /* special: tabular */
  if (txt[0] == '\t') {
    return(-1);
  }
  /* extended char */
  if (txt[0] == '#')
    {
    /* '##' is a '#' */
    if (txt[1] == '#')
      {
      *shift = 1;
      return(get_letter_index_simple('#'));
      }
    else if ((txt[1] == '\'')||(txt[1] == '`')||(txt[1] == '~')||(txt[1] == '^'))
      {
      *shift = -1;
      return(get_letter_index_simple(txt[1]));
      }
    else if (txt[1] == '%')
      {
      *shift = -1;
      return(get_letter_index_simple('"'));
      }
    *shift = 1;
    return(get_letter_index_simple(256+txt[1]));
    }
  else  {/* standard char */
	return(get_letter_index_simple((int)txt[0]));
  }
}


void Font::draw_a_char(int index, int x, int y, float sx, float sy) {
  int i, j;

  j = index / CHAR_WNB;
  j = 15-j;
  i = index % CHAR_WNB;

  glTexCoord2f((float)i/(float)CHAR_WNB+(1./CHARS_W),
               (float)j/(float)CHAR_HNB);
  glVertex2f(x-sx, y+sy);
  glTexCoord2f((float)(i)/(float)CHAR_WNB+(1./CHARS_W),
               (float)(j+1)/(float)CHAR_HNB-(1./CHARS_H));
  glVertex2f(x-sx, y-sy);

  glTexCoord2f((float)(i+1)/(float)CHAR_WNB-(1./CHARS_W),
               (float)(j+1)/(float)CHAR_HNB-(1./CHARS_H));
  glVertex2f(x+sx, y-sy);

  glTexCoord2f((float)(i+1)/(float)CHAR_WNB-(1./CHARS_W),
               (float)(j)/(float)CHAR_HNB);
  glVertex2f(x+sx, y+sy);
}

void Font::draw_a_char3D(int index, float x, float y, float sx, float sy) {
  int i, j;

  j = index / CHAR_WNB;
  j = 15-j;
  i = index % CHAR_WNB;

  glTexCoord2f((float)i/(float)CHAR_WNB+(1./CHARS_W),
               (float)j/(float)CHAR_HNB);
  glVertex3f(x-sx, 0., y+sy);
  glTexCoord2f((float)(i)/(float)CHAR_WNB+(1./CHARS_W),
               (float)(j+1)/(float)CHAR_HNB-(1./CHARS_H));
  glVertex3f(x-sx, 0., y-sy);
  glTexCoord2f((float)(i+1)/(float)CHAR_WNB-(1./CHARS_W),
               (float)(j+1)/(float)CHAR_HNB-(1./CHARS_H));
  glVertex3f(x+sx, 0., y-sy);
  glTexCoord2f((float)(i+1)/(float)CHAR_WNB-(1./CHARS_W),
               (float)(j)/(float)CHAR_HNB);
  glVertex3f(x+sx, 0., y+sy);
}

void Font::draw_a_text(int tl) {
  int i, offset, x, index;
  double tm, tr, tim, tp;


  x = textlines[tl].x;
  for(i=0; i<textlines[tl].length; i++) {
  	index = get_letter_index(textlines[tl].text + i, &offset);
  	if (textlines[tl].time[i] < 0.) {
		x += (int)(2.*textlines[tl].sizex*char_sizes[index]);
		continue;
	}
	if (textlines[tl].time[i] < BASE_TIME) {
		tm = textlines[tl].time[i]/BASE_TIME;
		draw_a_char(index, x, textlines[tl].y,
	                    textlines[tl].sizex*tm, textlines[tl].sizey*tm);
	}
	else {
	        tim = textlines[tl].time[i]-BASE_TIME;
		if (textlines[tl].time[i] > textlines[tl].tmax) {
				tr = 0.;
		}
		else
			tr = ((textlines[tl].tmax-textlines[tl].time[i])/textlines[tl].tmax);
		if (tr < 0.)
			tr = 0.;
		// pulse
		if (textlines[tl].time_pulse > 0.) {
			if (textlines[tl].time_pulse < BASE_TIME)
				tp = 0.25*textlines[tl].time_pulse/BASE_TIME;
			else
				tp = 0.25;
		}
		tm = 1. + textlines[tl].amplitude * tr * sin(4.*3.14159*(tim))
		        + tp * sin(3.31*3.14159*(total_time+i*0.1));
		draw_a_char(index, x, textlines[tl].y,
	                    textlines[tl].sizex*tm, textlines[tl].sizey*tm);
	}
	i += offset;
	x += (int)(2.*textlines[tl].sizex*char_sizes[index]);
  }
}

void Font::draw_highlight(int tl) {
  int index, offset;
  float w;

  if ((tl < 0)||(textlines[tl].text == NULL))
    return;

  index = get_letter_index("#S ", &offset);

  glColor4f(1.,1.,1.,textlines[tl].time_pulse/BASE_TIME);
  w = (float)l_getTextWidth(textlines[tl].text, textlines[tl].sizex) + textlines[tl].sizex;
  draw_a_char(index, (int)(textlines[tl].x+w/2.-1.5*textlines[tl].sizex), textlines[tl].y,
              w/2., textlines[tl].sizey);
}

// returns the effective width of the text
int Font::getTextWidth(int font, const char *text, float sx) {
  	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	return(current_fonts[font]->l_getTextWidth(text, sx));
}

// returns the effective width of the text
int Font::l_getTextWidth(const char *text, float sx) {
  int i, l = 0, index, offset;

  if ((text == NULL)||(strlen(text) == 0))
    return(0);

  for(i=0; i<strlen(text); i++) {
  	index = get_letter_index(text + i, &offset);
	l += (int)(2.*sx*char_sizes[index]);
  }
  return(l);
}

int Font::addCenterText(int font, const char *text, int x, int y,
                        float sx, float sy, double delay, double offset,
						double stab, double ampl, int fix) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	return(current_fonts[font]->l_addCenterText(text, x, y, sx, sy, delay,
                           offset, stab, ampl, fix));
}

// same than 'add_text' but 'x' is the X-center of the text to draw
int Font::l_addCenterText(const char *text, int x, int y, float sx, float sy,
                          double delay, double offset, double stab,
	                  double ampl, int fix) {
  int width;

  width = l_getTextWidth(text, sx);
  return(l_addText(text, x-width/2, y, sx, sy, delay, offset, stab, ampl, fix));
}

int Font::addRightText(int font, const char *text, int x, int y,
                       float sx, float sy,
                       double delay, double offset, double stab,
                       double ampl, int fix) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	return(current_fonts[font]->l_addRightText(text, x, y, sx, sy, delay,
                           offset, stab, ampl, fix));
}

// same than 'add_text' but 'x' is the right-cborder of the text to draw
int Font::l_addRightText(const char *text, int x, int y, float sx, float sy,
                         double delay, double offset, double stab,
	                 double ampl, int fix) {
  int width;

  width = l_getTextWidth(text, sx);
  return(l_addText(text, x-width, y, sx, sy, delay, offset, stab, ampl, fix));
}


// returns true if some text is still drawn (usefull to wait for all text
//   to be removed with animation
int Font::isTextDrawn() {
  int i;

  for(i=0; i<=max_line_used; i++) {
  	if (textlines[i].text != NULL) {
		return(1);
	}
  }
  return(0);
}

// returns the ID of the selected text (-1 if none)
int Font::getSelectedText(int mx, int my) {
  int i;

  for(i=0; i<=max_line_used; i++) {
  	if (textlines[i].text != NULL) {
		if ((mx >= textlines[i].x0)&&(mx <= textlines[i].x1)&&
		    (my >= textlines[i].y0)&&(my <= textlines[i].y1))
			return(i);
	}
  }
  return(-1);
}

void Font::draw() {
  int i;
  
  Enter2DMode();
  
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  for(i=0; i<=max_line_used; i++) {
  	if (textlines[i].text != NULL) {
		bindTexture(textlines[i].font->fontName);
		glBegin(GL_QUADS);
		if (textlines[i].pulse) {
		  textlines[i].font->draw_highlight(i);
		}
                glColor4fv(textlines[i].font->color);
		textlines[i].font->draw_a_text(i);
		glEnd();
	}
  }

  
  Leave2DMode();
}


void Font::tick(Real t) {
  int i, j, stable;

  // update local elapsed time
  total_time += t;

  for(i=0; i<max_line_used; i++) {
  	if (textlines[i].text != NULL) {
		if (textlines[i].removing) {
			stable = 1;
			for(j=0; j<textlines[i].length; j++) {
				textlines[i].time[j] -= t;
				if (textlines[i].time[j] > 0)
					stable = 0;
			}
			if (stable)
			  destroyText(i);
			continue;
		}
		if (textlines[i].pulse)
			textlines[i].time_pulse += t;
		if ((textlines[i].time_pulse > 0.)&&(!textlines[i].pulse))
			textlines[i].time_pulse -= t;
		if ((textlines[i].stable)&&(!textlines[i].pulse))
			continue;
		stable = 1;
		for(j=0; j<textlines[i].length; j++) {
			textlines[i].time[j] += t;
			if (textlines[i].time[j] < textlines[i].tmax)
			  stable = 0;
		}
		if (stable)
			textlines[i].stable = 1;
	}
  }
}

void Font::setColor(int font, float r, float g, float b) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	current_fonts[font]->l_setColor(r, g, b);
}
void Font::setColor(int font, float r, float g, float b, float a) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	current_fonts[font]->l_setColor(r, g, b, a);
}
void Font::setTransp(int font, float a) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	current_fonts[font]->l_setTransp(a);
}
void Font::resetColor(int font) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	current_fonts[font]->l_resetColor();
}
void Font::l_resetColor() {
  color[0] = 1.;
  color[1] = 1.;
  color[2] = 1.;
  color[3] = 1.;
}
void Font::l_setColor(float r, float g, float b) {
  color[0] = r;
  color[1] = g;
  color[2] = b;
}
void Font::l_setColor(float r, float g, float b, float a) {
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
}
void Font::l_setTransp(float a) {
  color[3] = a;
}

void Font::changeText(int tl, const char *text) {
  int i;

  if (tl < 0)
    return;
  if (textlines[tl].text == NULL)
    return;

  free(textlines[tl].text);
  free(textlines[tl].time);
  textlines[tl].text = strdup(text);
  textlines[tl].length = strlen(text);
  textlines[tl].time = (float*)malloc(sizeof(float)*strlen(text));
  if (textlines[tl].time == NULL) {
    fprintf(stderr, "Cannot perform allocation in Font system. Text removed.\n");
    if (textlines[tl].text != NULL)
      free(textlines[tl].text);
    textlines[tl].text = NULL;
    return;
  }  

  for(i=0; i<strlen(text); i++) {
  	textlines[tl].time[i] = BASE_TIME; // animation for remove will not be correct
  }

}

void Font::setPulseText(int tl, int pulse) {
  if (tl < 0)
    return;
  if (textlines[tl].text == NULL)
    return;

  if (textlines[tl].pulse == pulse)
    return;

  if (pulse) {
    textlines[tl].pulse = 1;
    textlines[tl].time_pulse = 0.;
  } else {
    textlines[tl].pulse = 0;
  }
}

int Font::drawSimpleText(int font, const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	return(current_fonts[font]->l_drawSimpleText(text, x, y, sx, sy, r, g, b, a));
}


int Font::drawRightSimpleText(int font, const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	return(current_fonts[font]->l_drawRightSimpleText(text, x, y, sx, sy, r, g, b, a));
}


int Font::drawCenterSimpleText(int font, const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	return(current_fonts[font]->l_drawCenterSimpleText(text, x, y, sx, sy, r, g, b, a));
}

int Font::l_drawSimpleText(const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a) {
  int i, offset, index;
  double tm, tr, tim;
  float cx;

  if ((text == NULL)||(strlen(text) == 0))
    return(0);

  Enter2DMode();
  
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  bindTexture(fontName);
  glColor4f(r, g, b, a);

  glBegin(GL_QUADS);

  cx = (float)x;
  for(i=0; i<strlen(text); i++) {
  	index = get_letter_index(text + i, &offset);

	draw_a_char(index, (int)cx, y, sx, sy);
        i += offset;
        cx += (float)(2.*sx*char_sizes[index]);
	}

  glEnd();

  Leave2DMode();
}

int Font::l_drawRightSimpleText(const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a) {
  return(l_drawSimpleText(text, x-l_getTextWidth(text, sx), y, sx, sy, r, g, b, a));
  
}

int Font::l_drawCenterSimpleText(const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a) {
  return(l_drawSimpleText(text, x-l_getTextWidth(text, sx)/2, y, sx, sy, r, g, b, a));
}

int Font::drawSimpleText3D(int font, const char *text, float sx, float sy) {

	if ((font < 0)||(font >= nb_fonts))
	  font = 0;
	return(current_fonts[font]->l_drawSimpleText3D(text, sx, sy));
}

int Font::l_drawSimpleText3D(const char *text, float sx, float sy) {
  int i, offset, index;
  double tm, tr, tim;
  float cx, y;

  if ((text == NULL)||(strlen(text) == 0))
    return(0);
  
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  bindTexture(fontName);
  glBegin(GL_QUADS);

  cx = 0.;
  y = 0.;
  for(i=0; i<strlen(text); i++) {

  	index = get_letter_index(text + i, &offset);

	draw_a_char3D(index, cx, y, sx, sy);
	i += offset;
	cx += (float)(2.*sx*char_sizes[index]);
  }

  glEnd();
}




