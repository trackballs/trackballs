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

#include "general.h"
#include "gameMode.h"
#include "menuMode.h"
#include "glHelp.h"
#include "SDL/SDL_image.h"
#include "helpMode.h"
#include "font.h"
#include "menusystem.h"

using namespace std;

HelpMode *HelpMode::helpMode;
SDL_Surface *HelpMode::background;
SDL_Surface *HelpMode::page0, *HelpMode::page1;

#define CODE_BACK 1
#define CODE_MOREHELP 2

void HelpMode::init() {
  char str[256];
  if (low_memory)
    background = NULL;
  else {
    snprintf(str, sizeof(str), "%s/images/helpBackground.jpg", SHARE_DIR);
    background = IMG_Load(str);
    if (!background) {
      printf(_("Error: Failed to load %s\n"), str);
      exit(0);
    }
  }

  helpMode = new HelpMode();
}
HelpMode::HelpMode() {}
void HelpMode::activated() {
  char str[256];

  if (!background) {
    snprintf(str, sizeof(str), "%s/images/helpBackground.jpg", SHARE_DIR);
    background = IMG_Load(str);
    if (!background) {
      printf(_("Error: Failed to load %s\n"), str);
      exit(0);
    }
  }

  /* Loads the background images. */
  GLfloat texcoord[4];
  bgTexture = LoadTexture(background, texcoord);
  texMaxX = texcoord[0];
  texMinY = texcoord[1];
  texMinX = texcoord[2];
  texMaxY = texcoord[3];

  if (screenWidth < 1024)
    snprintf(str, sizeof(str), "%s/images/help0_640.png", SHARE_DIR);
  else
    snprintf(str, sizeof(str), "%s/images/help0_1024.png", SHARE_DIR);
  page0 = IMG_Load(str);
  if (!page0) {
    printf(_("Error: Failed to load %s\n"), str);
    exit(0);
  }
  p0Texture = LoadTexture(page0, texcoord);
  SDL_FreeSurface(page0);
  p0MaxX = texcoord[0];
  p0MinY = texcoord[1];
  p0MinX = texcoord[2];
  p0MaxY = texcoord[3];

  if (screenWidth < 1024)
    snprintf(str, sizeof(str), "%s/images/help1_640.png", SHARE_DIR);
  else
    snprintf(str, sizeof(str), "%s/images/help1_1024.png", SHARE_DIR);
  page1 = IMG_Load(str);
  if (!page1) {
    printf(_("Error: Failed to load %s\n"), str);
    exit(0);
  }
  p1Texture = LoadTexture(page1, texcoord);
  SDL_FreeSurface(page1);
  p1MaxX = texcoord[0];
  p1MinY = texcoord[1];
  p1MinX = texcoord[2];
  p1MaxY = texcoord[3];

  isExiting = 0;
  timeLeft = 1.0;
  page = 0;

  Font::destroyAllText();
  Font::setColor(0, 1., 1., 1., 1.);
  Font::addRightText(0, _("MORE HELP"), screenWidth - 15, screenHeight - 20, 15, 15, 0.1, 0.1,
                     3., 0.5, 0);
}
void HelpMode::deactivated() {
  glDeleteTextures(1, &bgTexture);
  glDeleteTextures(1, &p0Texture);
  glDeleteTextures(1, &p1Texture);

  if (low_memory && background) {
    SDL_FreeSurface(background);
    background = NULL;
  }
  Font::destroyAllText();
}
void HelpMode::display() {
  char str[256];
  int w, h;

  // Draw the background using the preloaded texture
  glColor3f(timeLeft, timeLeft, timeLeft);

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glColor3f(1.0, 1.0, 1.0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  Enter2DMode();
  glBindTexture(GL_TEXTURE_2D, bgTexture);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(0, 0);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(screenWidth, 0);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(0, screenHeight);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(screenWidth, screenHeight);
  glEnd();

  if (page == 0) {
    glBindTexture(GL_TEXTURE_2D, p0Texture);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(p0MaxX, p0MinY);
    glVertex2i(0, 0);
    glTexCoord2f(p0MinX, p0MinY);
    glVertex2i(screenWidth, 0);
    glTexCoord2f(p0MaxX, p0MaxY);
    glVertex2i(0, screenHeight);
    glTexCoord2f(p0MinX, p0MaxY);
    glVertex2i(screenWidth, screenHeight);
    glEnd();
  }

  if (page == 1) {
    glBindTexture(GL_TEXTURE_2D, p1Texture);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(p1MaxX, p1MinY);
    glVertex2i(0, 0);
    glTexCoord2f(p1MinX, p1MinY);
    glVertex2i(screenWidth, 0);
    glTexCoord2f(p1MaxX, p1MaxY);
    glVertex2i(0, screenHeight);
    glTexCoord2f(p1MinX, p1MaxY);
    glVertex2i(screenWidth, screenHeight);
    glEnd();
  }
  Leave2DMode();

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

    // Enter2DMode();
    for (int i = 0; i < 20; i++) {
      // draw2DString(infoFont,text[i][0],screenWidth/2-310,screenHeight/2-200+i*20,220,220,64);
      // draw2DString(infoFont,text[i][1],screenWidth/2-150,screenHeight/2-200+i*20,220,220,64);
      if (screenWidth <= 640) {
        Font::drawSimpleText(0, text[i][0], screenWidth / 2 - 310,
                             screenHeight / 2 - 200 + i * 20 + 10, 10.f, 10., 0.9, 0.9, 0.25,
                             1.);
        Font::drawSimpleText(0, text[i][1], screenWidth / 2 - 150,
                             screenHeight / 2 - 200 + i * 20 + 10, 10., 10., 0.9, 0.9, 0.25,
                             1.);
        // Font::drawCenterSimpleText(0,"See also the forums at:",screenWidth/2,10,10.,10.,
        // 0.9,0.9,0.25,1.0);
        // Font::drawCenterSimpleText(0,"trackballs.theunix.org",screenWidth/2,30,10.,10.,
        // 0.9,0.9,0.25,1.0);
      } else if (screenWidth == 800) {
        Font::drawSimpleText(0, text[i][0], screenWidth / 2 - 330,
                             screenHeight / 2 - 200 + i * 20 + 10, 10.f, 10., 0.9, 0.9, 0.25,
                             1.);
        Font::drawSimpleText(0, text[i][1], screenWidth / 2 - 130,
                             screenHeight / 2 - 200 + i * 20 + 10, 10., 10., 0.9, 0.9, 0.25,
                             1.);
        // Font::drawCenterSimpleText(0,"See also the trackballs forums
        // at:",screenWidth/2,30,20.,20., 0.9,0.9,0.25,1.0);
        // Font::drawCenterSimpleText(0,"trackballs.theunix.org",screenWidth/2,70,20.,20.,
        // 0.9,0.9,0.25,1.0);
      } else if (screenWidth >= 1024) {
        Font::drawSimpleText(0, text[i][0], screenWidth / 2 - 360,
                             screenHeight / 2 - 200 + i * 20, 10.f, 10., 0.9, 0.9, 0.25, 1.);
        Font::drawSimpleText(0, text[i][1], screenWidth / 2 - 100,
                             screenHeight / 2 - 200 + i * 20, 10., 10., 0.9, 0.9, 0.25, 1.);
        // Font::drawCenterSimpleText(0,"See also the trackballs forums
        // at:",screenWidth/2,30,24.,24., 0.9,0.9,0.25,1.0);
        // Font::drawCenterSimpleText(0,"trackballs.theunix.org",screenWidth/2,80,24.,24.,
        // 0.9,0.9,0.25,1.0);
      }
    }
    // Leave2DMode();
  }

  int fontSize = 24;
  clearSelectionAreas();
  addText_Right(CODE_MOREHELP, fontSize / 2, screenHeight - 45, _("More Help"),
                screenWidth - 16);
  addText_Left(CODE_BACK, fontSize / 2, screenHeight - 45, _("Back"), 16);
  /*
  if(selected == 0)
        drawSurface(moreHelpHigh,screenWidth-moreHelpHigh->w,screenHeight-45,moreHelpHigh->w,moreHelpHigh->h);
  else
        drawSurface(moreHelp,screenWidth-moreHelp->w,screenHeight-45,moreHelp->w,moreHelp->h);
  */

  /*
  if(selected == 1)
        drawSurface(backHigh,-40,screenHeight-45,back->w,back->h);
  else
        drawSurface(back,-40,screenHeight-45,back->w,back->h);
  */

  drawMousePointer();
  displayFrameRate();
}
void HelpMode::key(int key) {
  if (key == SDLK_SPACE) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    mouseDown(1, x, y);
  }
  if (key == SDLK_RETURN) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    mouseDown(3, x, y);
  }
}
void HelpMode::idle(Real td) {
  int mouseX, mouseY;
  int x, y;

  tickMouse(td);

  SDL_GetMouseState(&x, &y);
  Uint8 *keystate = SDL_GetKeyState(NULL);
  if (keystate[SDLK_LEFT]) {
    x -= (int)(150 / fps);
    SDL_WarpMouse(x, y);
  }
  if (keystate[SDLK_RIGHT]) {
    x += (int)(150 / fps);
    SDL_WarpMouse(x, y);
  }
  if (keystate[SDLK_UP]) {
    y -= (int)(150 / fps);
    SDL_WarpMouse(x, y);
  }
  if (keystate[SDLK_DOWN]) {
    y += (int)(150 / fps);
    SDL_WarpMouse(x, y);
  }

  /*
  Uint8 mouseState=SDL_GetMouseState(&mouseX,&mouseY);
  if(mouseX >= screenWidth-moreHelp->w && mouseY > screenHeight-50)
        selected=0;
  else if(mouseX <= back->w && mouseY > screenHeight-50)
        selected=1;
  else
        selected=-1;
  */

  if (isExiting)
    timeLeft -= td;
  else
    timeLeft = min(1.0, timeLeft + td);
  if (timeLeft < 0.0) MenuMode::activate(MenuMode::menuMode);
}
void HelpMode::mouseDown(int button, int x, int y) {
  int selected = getSelectedArea();

  if (selected == CODE_MOREHELP) {
    page = (page + (button == 1 ? 1 : -1)) % 3;
  } else if (selected == CODE_BACK) {
    isExiting = 1;
  }
}
