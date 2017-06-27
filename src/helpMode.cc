/* HelpMode.cc
   Displayes the help screen

   Copyright (C) 2000  Mathias Broxvall

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

#include "helpMode.h"

#include "font.h"
#include "menuMode.h"
#include "menusystem.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_surface.h>

HelpMode *HelpMode::helpMode;
SDL_Surface *HelpMode::background;
SDL_Surface *HelpMode::page0, *HelpMode::page1;

#define CODE_BACK 1
#define CODE_MOREHELP 2

void HelpMode::init() {
  if (low_memory)
    background = NULL;
  else {
    background = loadImage("helpBackground.jpg");
  }

  helpMode = new HelpMode();
}
HelpMode::HelpMode() {
  timeLeft = 0.;
  isExiting = 0;
  page = 0;
}
void HelpMode::activated() {
  if (!background) { background = loadImage("helpBackground.jpg"); }

  /* Loads the background images. */
  bgTexture = LoadTexture(background, bgCoord);

  if (screenWidth < 1024)
    page0 = loadImage("help0_640.png");
  else
    page0 = loadImage("help0_1024.png");
  p0Texture = LoadTexture(page0, p0coord);
  SDL_FreeSurface(page0);

  if (screenWidth < 1024)
    page1 = loadImage("help1_640.png");
  else
    page1 = loadImage("help1_1024.png");

  p1Texture = LoadTexture(page1, p1coord);
  SDL_FreeSurface(page1);

  clearKeyboardFocus();

  isExiting = 0;
  timeLeft = 1.0;
  page = 0;
}
void HelpMode::deactivated() {
  glDeleteTextures(1, &bgTexture);
  glDeleteTextures(1, &p0Texture);
  glDeleteTextures(1, &p1Texture);

  if (low_memory && background) {
    SDL_FreeSurface(background);
    background = NULL;
  }
}
void HelpMode::display() {
  // Draw the background using the preloaded texture
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  Enter2DMode();
  draw2DRectangle(0, 0, screenWidth, screenHeight, bgCoord[0], bgCoord[1], bgCoord[2],
                  bgCoord[3], 1., 1., 1., 1., bgTexture);

  if (page == 0) {
    draw2DRectangle(0, 0, screenWidth, screenHeight, p0coord[0], p0coord[1], p0coord[2],
                    p0coord[3], 1., 1., 1., 1., p0Texture);
  }

  if (page == 1) {
    draw2DRectangle(0, 0, screenWidth, screenHeight, p1coord[0], p1coord[1], p1coord[2],
                    p1coord[3], 1., 1., 1., 1., p1Texture);
  }

  if (page == 2) {
    const char *text[21][2] = {
        {_("Your marble:"), _("Steer this with your mouse, numpad or arrows. ")},
        {" ", _("Left mouse or spacebar is jump.")},
        {" ", _("Shift gives top speed when using keyboard.")},
        {_("Mr Black:"), _("Your enemy. Defeat with Modpill - Spike.")},
        {_("Modpill Speed:"), _("Makes you go faster for short period of time.")},
        {_("Modpill Spikes:"), _("Weapon against Mr Black and grip for icy terrain.")},
        {_("Modpill Freeze:"), _("This will freeze you - avoid it.")},
        {_("Modpill Glass:"), _("Protects you from acid, but makes you more fragile.")},
        {_("Modpill Nitro:"), _("Move really fast. Active nitro with second button.")},
        {_("Switch:"), _("Activates/deactivates some object/element in the level.")},
        {_("Moving spike:"), _("Obstacle to avoid.")},
        {_("Flag:"), _("Collect for bonus points.")},
        {_("Normal terrain:"), _("Can be of any color and is safe to move on.")},
        {_("Acid terrain:"), _("Will kill your marble unless you have a glass modpill.")},
        {_("Icy terrain:"), _("Slippery terrain unless you have a spikes modpill.")},
        {_("Sandy terrain:"), _("Soft to land on but will slow you down.")},
        {_("Goal:"), _("Will take you to the next level. ")},
        {" ", _("Get here before time runs out.")},
        {_("Force Field:"), _("You cannot pass this unless you switch it off.")},
        {" ", _("Some are lethal and some are one way.")},
        {_("Tunnel:"), _("Takes you places, sometimes with great speed.")}};

    for (int i = 0; i < 20; i++) {
      if (screenWidth <= 800) {
        Font::drawSimpleText(text[i][0], 50, screenHeight / 2 - 200 + i * 20 + 10, 10, 0.9,
                             0.9, 0.25, 1.);
        Font::drawSimpleText(text[i][1], 210, screenHeight / 2 - 200 + i * 20 + 10, 10, 0.9,
                             0.9, 0.25, 1.);
      } else if (screenWidth <= 1280) {
        Font::drawSimpleText(text[i][0], 70, screenHeight / 2 - 200 + i * 24 + 10, 12, 0.9,
                             0.9, 0.25, 1.);
        Font::drawSimpleText(text[i][1], 270, screenHeight / 2 - 200 + i * 24 + 10, 12, 0.9,
                             0.9, 0.25, 1.);
      } else {
        Font::drawSimpleText(text[i][0], 90, screenHeight / 2 - 200 + i * 28, 14, 0.9, 0.9,
                             0.25, 1.);
        Font::drawSimpleText(text[i][1], 370, screenHeight / 2 - 200 + i * 28, 14, 0.9, 0.9,
                             0.25, 1.);
      }
    }
  }

  int fontSize = 24;
  clearSelectionAreas();
  addText_Right(CODE_MOREHELP, fontSize / 2, screenHeight - 45, _("More Help"),
                screenWidth - 16);
  addText_Left(CODE_BACK, fontSize / 2, screenHeight - 45, _("Back"), 16);

  drawMousePointer();
  displayFrameRate();

  Leave2DMode();
}
void HelpMode::key(int key) {
  int shift = (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
  if (key == SDLK_TAB) { moveKeyboardFocus(shift); }
  if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE)
    mouseDown(shift ? 3 : 1, -1, -1);
  if (key == SDLK_ESCAPE) GameMode::activate(MenuMode::menuMode);
}
void HelpMode::idle(Real td) {
  tickMouse(td);

  if (isExiting)
    timeLeft -= td;
  else
    timeLeft = fmin(1.0, timeLeft + td);
  if (timeLeft < 0.0) MenuMode::activate(MenuMode::menuMode);
}
void HelpMode::mouseDown(int button, int /*x*/, int /*y*/) {
  int selected = getSelectedArea();

  if (selected == CODE_MOREHELP) {
    page = (page + (button == 1 ? 1 : -1)) % 3;
  } else if (selected == CODE_BACK) {
    isExiting = 1;
  }
}
