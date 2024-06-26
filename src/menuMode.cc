/* menuMode.cc
   The inital menu with game options and highscores

   Copyright (C) 2000 - 2006  Mathias Broxvall

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

#include "menuMode.h"

#include "editMode.h"
#include "font.h"
#include "helpMode.h"
#include "hofMode.h"
#include "menusystem.h"
#include "settingsMode.h"
#include "setupMode.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

MenuMode *MenuMode::menuMode = NULL;

#define MENU_NEWGAME 1
#define MENU_QUIT 2
#define MENU_SETTINGS 3
#define MENU_HOF 4
#define MENU_HELP 5
#define MENU_EDITOR 6

#define SLIDE_TRANSITION 3
#define SLIDE_SINGLE 12
#define SLIDE_TOTAL 15

#define SLIDE_MODE_STATIC 0
#define SLIDE_MODE_STRAIGHT 1
#define SLIDE_MODE_DIAGONAL 2
#define SLIDE_MODE_ZOOM 3
#define NUM_SLIDE_MODES 4

#define NUM_SLIDES 10

MenuMode *MenuMode::init() {
  if (!menuMode) menuMode = new MenuMode();
  return menuMode;
}
void MenuMode::cleanup() {
  if (menuMode) delete menuMode;
}
MenuMode::MenuMode() {
  offset = 0.0;
  slides[0] = 0, slides[1] = 0;
  slideTime = 0.;
  slideMode[0] = SLIDE_MODE_STATIC;
  slideMode[1] = SLIDE_MODE_STATIC;
}

char *story;

void MenuMode::display() const {
  char str[256];

  /* This text used to be drawn on the main screen, possibly as a single long text
   * scrolling leftwards */
  story =
      _("Trackballs is a game similar to the classical game Marble Madness from the 80's. \
By steering a marble ball through a labyrinth filled with sharp spikes, \
pools of acid and other obstacles the player collects points. \
When the ball reaches the destination you continue on the next, more difficult, level \
- unless, of course, the time runs out.                                               \
You steer the ball using the mouse and by pressing >spacebar< you can jump a short distance.");

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Enter2DMode();

  /* Draw slides */
  if (slideTime < SLIDE_SINGLE) {
    drawSlide(0, slideTime + SLIDE_TRANSITION, 1.0);
  } else {
    drawSlide(0, slideTime + SLIDE_TRANSITION,
              1.0 - (slideTime - SLIDE_SINGLE) / (double)SLIDE_TRANSITION);
    drawSlide(1, slideTime - SLIDE_SINGLE,
              (slideTime - SLIDE_SINGLE) / (double)SLIDE_TRANSITION);
  }

  /* Draw header */
  draw2DRectangle(screenWidth / 2 - 512 / 2, 20, 512, 128, 0., 0., 1., 1.,
                  Color(1., 1., 1., 1.), textures[loadTexture("header.png")]);

  clearSelectionAreas();
  int fontSize = computeMenuSize();
  int sep = 5 * fontSize / 2;
  int baseline = screenHeight - sep * 3;
  int left = computeScreenBorder();
  int right = screenWidth - computeScreenBorder();

  addText_Left(MENU_NEWGAME, fontSize, baseline + sep * 0, _("New Game"), left);
  addText_Right(MENU_QUIT, fontSize, baseline + sep * 0, _("Quit"), right);
  addText_Left(MENU_SETTINGS, fontSize, baseline + sep * 1, _("Settings"), left);
  addText_Right(MENU_HOF, fontSize, baseline + sep * 1, _("Hall of Fame"), right);
  addText_Left(MENU_HELP, fontSize, baseline + sep * 2, _("Help"), left);
  addText_Right(MENU_EDITOR, fontSize, baseline + sep * 2, _("Map Editor"), right);

  /* Draw story text */
  /*
  Font::drawSimpleText(0, story, (int)(screenWidth-offset), storyText_y, 24., 24., 220/256.0,
  220/256.0, 64/256.0, 1.0);
  */

  sprintf(str, "v%s", VERSION);
  drawRightSimpleText(str, screenWidth - 15, screenHeight - 15, 10,
                      Color(SRGBColor(0.50, 0.50, 0.50, 0.75)));

  drawMousePointer();
  displayFrameRate();
  Leave2DMode();
}
void MenuMode::key(int key) {
  if (key == SDLK_TAB) { moveKeyboardFocus(SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)); }
  if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE) doSelection();
}
void MenuMode::doSelection() {
  int selection = getSelectedArea();

  switch (selection) {
  case MENU_QUIT:
    /* activate the null mode, after which event loop exits */
    GameMode::activate(NULL);
    break;
  case MENU_NEWGAME:
    GameMode::activate(SetupMode::init());
    break;
  case MENU_HOF:
    GameMode::activate(HallOfFameMode::init());
    break;
  case MENU_SETTINGS:
    GameMode::activate(SettingsMode::init());
    break;
  case MENU_HELP:
    GameMode::activate(HelpMode::init());
    break;
  case MENU_EDITOR:
    GameMode::activate(EditMode::init());
    break;
  }
}

void MenuMode::tick(Real td) {
  slideTime += td;
  if (slideTime > SLIDE_TOTAL) loadSlide();

  tickMouse(td);
  const Uint8 *keystate = SDL_GetKeyboardState(NULL);
  if (keystate[SDL_SCANCODE_SPACE] || keystate[SDL_SCANCODE_RETURN]) doSelection();
}
void MenuMode::mouseDown(int /*mouseState*/, int /*x*/, int /*y*/) { doSelection(); }

void MenuMode::activated() {
  clearKeyboardFocus();
  clearSelectionAreas();
  loadSlide();
  loadSlide();
}
void MenuMode::deactivated() {
  /* Delete old slides */
  if (slides[0] != 0) {
    glDeleteTextures(1, &slides[0]);
    slides[0] = 0;
  }
  if (slides[1] != 0) {
    glDeleteTextures(1, &slides[1]);
    slides[1] = 0;
  }
}

void MenuMode::loadSlide() {
  char str[256];
  SDL_Surface *slide;

  /* Delete old slide */
  if (slides[0] != 0) {
    glDeleteTextures(1, &slides[0]);
    slides[0] = 0;
  }

  /* Swap slide 1 and slide 2 */
  slides[0] = slides[1];
  slideMin[0][0] = slideMin[1][0];
  slideMin[0][1] = slideMin[1][1];
  slideMax[0][0] = slideMax[1][0];
  slideMax[0][1] = slideMax[1][1];
  slideMode[0] = slideMode[1];

  /* Load the image */
  snprintf(str, sizeof(str), "slide-%02d.jpg", (rand() % NUM_SLIDES) + 1);
  slide = loadImage(str);

  /* Create a texture of it, and free the image data */
  GLfloat texcoord[4];
  slides[1] = LoadTexture(slide, texcoord);
  SDL_FreeSurface(slide);
  slideMin[1][0] = texcoord[0];
  slideMin[1][1] = texcoord[1];
  slideMax[1][0] = texcoord[2];
  slideMax[1][1] = texcoord[3];
  slideMode[1] = rand() % NUM_SLIDE_MODES;
  slideTime = 0.0;
}

void MenuMode::drawSlide(int slide, double time, double alpha) const {
  double slideWidth = slideMax[slide][0];
  double slideHeight = slideMax[slide][1];
  double screenAspect = screenWidth / (double)screenHeight;
  double slideAspect = slideMax[slide][0] / slideMax[slide][1];
  double viewWidth, viewHeight, viewOffsetX = 0., viewOffsetY = 0.;

  time = time / (double)SLIDE_TOTAL;
  if (slideAspect < screenAspect) {
    viewWidth = slideWidth;
    viewHeight = viewWidth / screenAspect;
  } else {
    viewHeight = slideHeight;
    viewWidth = viewHeight * screenAspect;
  }

  switch (slideMode[slide]) {
  case SLIDE_MODE_STATIC:
    if (slideAspect < screenAspect) {
      viewOffsetX = 0.0;
      viewOffsetY = (slideHeight - viewHeight) / 2.0;
    } else {
      viewOffsetX = (slideWidth - viewWidth) / 2.0;
      viewOffsetY = 0.0;
    }
    break;
  case SLIDE_MODE_STRAIGHT:
    viewHeight *= 0.8;
    viewWidth *= 0.8;

    if (slideAspect < screenAspect) {
      viewOffsetX = (slideWidth - viewWidth) * 0.5;
      viewOffsetY = time * (slideHeight - viewHeight);
    } else {
      viewOffsetX = time * (slideWidth - viewWidth);
      viewOffsetY = (slideHeight - viewHeight) * 0.5;
    }
    break;
  case SLIDE_MODE_DIAGONAL:
    viewHeight *= 0.5;
    viewWidth *= 0.5;
    viewOffsetX = (slideWidth - viewWidth) * time;
    viewOffsetY = (slideHeight - viewHeight) * time;
    break;
  case SLIDE_MODE_ZOOM:
    viewHeight *= 1.0 - 0.5 * time;
    viewWidth *= 1.0 - 0.5 * time;
    viewOffsetX = (slideWidth - viewWidth) * 0.5;
    viewOffsetY = (slideHeight - viewHeight) * 0.5;
    break;
  }

  if ((viewWidth + viewOffsetX > slideWidth || viewHeight + viewOffsetY > slideHeight) && 0) {
    warning(
        "mode: %d\n time: %f\n slideWidth=%f\n slideHeight=%f\n viewWidth=%f\n "
        "viewHeight=%f\n oX: %f\n oY: %f",
        slideMode[slide], time, slideWidth, slideHeight, viewWidth, viewHeight, viewOffsetX,
        viewOffsetY);
  }

  draw2DRectangle(0, 0, screenWidth, screenHeight, viewOffsetX, viewOffsetY, viewWidth,
                  viewHeight, Color(1., 1., 1., alpha), slides[slide]);
}
