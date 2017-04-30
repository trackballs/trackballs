/* font.h
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

#ifndef FONT_H
#define FONT_H


#include <GL/gl.h>


class Font;

/* structure of a text to be draw */
typedef struct
{
  char *text;   // the text
  float *time;  // current time value for each character
  int length;   // length of the text
  int x, y;     // starting pos. to draw
  int removing; // true if text is going away
  int stable;   // true if no more animation is needed
  int pulse;    // if true, make the text pulse
  float time_pulse;
  float sizex, sizey; // size of characters
  float tmax;   // time before stabilization
  float amplitude;  // amplitude of oscillations
  // BBOX information
  int x0, y0, x1, y1;
  // font to use
  Font *font;
}TextLine;


// table of texts
#define MAX_TEXT_LINE 256


class Font {
public:
  static void init();

  Font(const char *fname);
  ~Font();
  /* functions with a global access (not object-dependant)
   * can be called anyway, after 'init'.
   */
  static void draw();
  static void tick(Real t);
  static int addText(int font, const char *text, int x, int y, float sx, float sy, double delay,
                     double offset, double stab, double ampl, int fix);
  static int drawSimpleText(int font, const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a);
  static int drawSimpleText3D(int font, const char *text, float sx, float sy);
  static int drawRightSimpleText(int font, const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a);
  static int drawCenterSimpleText(int font, const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a);
  static int addRightText(int font, const char *text, int x, int y, float sx, float sy,
               double delay, double offset, double stab,
	       double ampl, int fix);
  static int addCenterText(int font, const char *text, int x, int y, float sx, float sy,
               double delay, double offset, double stab,
	       double ampl, int fix);
  static void resetColor(int font);
  static void changeText(int tl, const char *text);
  static void delAllText();
  static void setColor(int font, float r, float g, float b);
  static void setColor(int font, float r, float g, float b, float a);
  static void setTransp(int font, float a);
  static int isTextDrawn();
  static void destroyText(int tl);
  static void destroyAllText();
  static int getSelectedText(int mx, int my);
  static void setPulseText(int tl, int pulse);
  static int getTextWidth(int font, const char *text, float sx);

  /* functions with an object-related access (may only be called
   *    by the static functions)
   */
  int l_addCenterText(const char *text, int x, int y, float sx, float sy,
               double delay, double offset, double stab,
	       double ampl, int fix);
  int l_addRightText(const char *text, int x, int y, float sx, float sy,
               double delay, double offset, double stab,
	       double ampl, int fix);
  int l_drawSimpleText(const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a);
  int l_drawRightSimpleText(const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a);
  int l_drawCenterSimpleText(const char *text, int x, int y, float sx, float sy, float r, float g, float b, float a);
  int l_drawSimpleText3D(const char *text, float sx, float sy);
  int l_getTextWidth(const char *text, float sx);
  void l_setColor(float r, float g, float b);
  void l_setColor(float r, float g, float b, float a);
  void l_setTransp(float a);
  void l_resetColor();
  int l_addText(const char *text, int x, int y, float sx, float sy,
               double delay, double offset, double stab,
	       double ampl, int fix);
  void l_delText(int tl);
  void l_delAllText();
  void l_destroyAllText();
  int figureFontInfo(void *, int, int);


protected:
  void draw_a_text(int tl);
  void draw_a_char(int index, int x, int y, float sx, float sy);
  void draw_a_char3D(int index, float x, float y, float sx, float sy);
  void draw_highlight(int tl);
  GLuint fontTex;
  char *fontName;
  float char_sizes[512];
  float color[4];

};


#endif
