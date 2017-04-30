/* guile.cc
   Handles the interface to guile (ie. initialization of callbacks)

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

#include "general.h"
#include "black.h"
#include "cyclicPlatform.h"
#include "game.h"
#include "player.h"
#include "flag.h"
#include "cactus.h"
#include "bird.h"
#include "teleport.h"
#include "goal.h"
#include "game.h"
#include "colorModifier.h"
#include "heightModifier.h"
#include "spike.h"
#include "sideSpike.h"
#include "map.h"
#include "trigger.h"
#include "modPill.h"
#include "baby.h"
#include <libguile.h>
#include "sign.h"
#include "forcefield.h"
#include "switch.h"
#include "settings.h"
#include "smartTrigger.h"
#include "pipe.h"
#include "pipeConnector.h"
#include "diamond.h"
#include "sound.h"
#include "animator.h"
#include "mainMode.h"
#include "fountain.h"
#include "weather.h"

using namespace std;

scm_t_bits smobAnimated_tag;
scm_t_bits smobGameHook_tag;

#define IS_ANIMATED(x) (SCM_NIMP(x) && SCM_CELL_TYPE(x) == smobAnimated_tag)
#define IS_GAMEHOOK(x) (SCM_NIMP(x) && (SCM_CELL_TYPE(x) == smobGameHook_tag || SCM_CELL_TYPE(x) == smobAnimated_tag))

SCM smobAnimated_make(Animated *a) {
  SCM smob;
  SCM_NEWSMOB(smob,smobAnimated_tag,a);
  return smob;
}
SCM smobGameHook_make(GameHook *h) {
  SCM smob;
  SCM_NEWSMOB(smob,smobGameHook_tag,h);
  return smob;
}
scm_sizet smobAnimated_free(SCM smob) {return 0;}
scm_sizet smobGameHook_free(SCM smob) {return 0;}


/************* new_mr_black ***********/
SCM_DEFINE (new_mr_black, "new-mr-black", 2, 0, 0,
			(SCM x, SCM y),
			"Creates Mr. Black at specified position.")
#define FUNC_NAME s_new_mr_black
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  if(Game::current)
	return smobAnimated_make(new Black(scm_num2dbl(x,FUNC_NAME),scm_num2dbl(y,FUNC_NAME)));
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************** new_baby *****************/
SCM_DEFINE (new_baby, "new-baby", 2, 0, 0,
			(SCM x, SCM y),
			"Creates a baby ball at specified position.")
#define FUNC_NAME s_new_baby
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  if(Game::current)
	return smobAnimated_make(new Baby(scm_num2dbl(x,FUNC_NAME),scm_num2dbl(y,FUNC_NAME)));
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************* add_teleport ***********/
SCM_DEFINE (add_teleport, "add-teleport", 5, 0, 0,
			(SCM x, SCM y, SCM dx, SCM dy, SCM radius),
			"Creates a teleporter at specified position, with specified destination.")
#define FUNC_NAME s_add_teleport
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(dx),dx,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(dy),dy,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(radius),radius,SCM_ARG5,FUNC_NAME);
  if(Game::current)
	return smobAnimated_make(new Teleport(scm_num2int(x,SCM_ARG1,FUNC_NAME),
	                  scm_num2int(y,SCM_ARG2,FUNC_NAME),scm_num2int(dx,SCM_ARG3,FUNC_NAME),
			  scm_num2int(dy,SCM_ARG4,FUNC_NAME),scm_num2dbl(radius,FUNC_NAME)));
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************* add_bird ***********/
SCM_DEFINE (add_bird, "add-bird", 6, 0, 0,
			(SCM x, SCM y, SCM dx, SCM dy, SCM size, SCM speed),
			"Creates a bird at specified position, with specified destination.")
#define FUNC_NAME s_add_bird
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(dx),dx,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(dy),dy,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(size),size,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(speed),speed,SCM_ARG5,FUNC_NAME);
  if(Game::current)
	return smobAnimated_make(new Bird(scm_num2int(x,SCM_ARG1,FUNC_NAME),
	                  scm_num2int(y,SCM_ARG2,FUNC_NAME),scm_num2int(dx,SCM_ARG3,FUNC_NAME),
			  scm_num2int(dy,SCM_ARG4,FUNC_NAME),
			  scm_num2dbl(size,FUNC_NAME),scm_num2dbl(speed,FUNC_NAME)));
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************** add_cyclic_platform *************/

SCM_DEFINE (add_cyclic_platform, "add-cyclic-platform", 8, 0, 0,
			(SCM x1,SCM y1,SCM x2,SCM y2,SCM low,SCM high,SCM offset,SCM speed),
			"Creates a new cyclic platform.")
#define FUNC_NAME s_add_cyclic_platform
{
  const char *s_add_cyclic_platform="add-cyclic-platform";
  SCM_ASSERT(SCM_NUMBERP(x1),x1,SCM_ARG1,s_add_cyclic_platform);
  SCM_ASSERT(SCM_NUMBERP(y1),y1,SCM_ARG2,s_add_cyclic_platform);
  SCM_ASSERT(SCM_NUMBERP(x2),x2,SCM_ARG3,s_add_cyclic_platform);
  SCM_ASSERT(SCM_NUMBERP(y2),y2,SCM_ARG4,s_add_cyclic_platform);
  SCM_ASSERT(SCM_REALP(low),low,SCM_ARG5,s_add_cyclic_platform);
  SCM_ASSERT(SCM_REALP(high),high,SCM_ARG6,s_add_cyclic_platform);
  SCM_ASSERT(SCM_REALP(offset),offset,SCM_ARG7,s_add_cyclic_platform);
  SCM_ASSERT(SCM_REALP(speed),speed,SCM_ARG7,s_add_cyclic_platform);
  CyclicPlatform *platform = new CyclicPlatform(scm_num2int(x1,SCM_ARG1,s_add_cyclic_platform),
												scm_num2int(y1,SCM_ARG1,s_add_cyclic_platform),
												scm_num2int(x2,SCM_ARG1,s_add_cyclic_platform),
												scm_num2int(y2,SCM_ARG1,s_add_cyclic_platform),
												scm_num2dbl(low,s_add_cyclic_platform),
												scm_num2dbl(high,s_add_cyclic_platform),
												scm_num2dbl(offset,s_add_cyclic_platform),
												scm_num2dbl(speed,s_add_cyclic_platform));
  return smobGameHook_make(platform);
}
#undef FUNC_NAME

/**************** set_position *************/
SCM_DEFINE (set_position, "set-position", 3, 1, 0,
			(SCM obj, SCM x, SCM y, SCM z),
			"Sets the position of specified object. Accepts optional z-coordinate.")
#define FUNC_NAME s_set_position
{
  SCM_ASSERT(IS_ANIMATED(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG3,FUNC_NAME);
  Animated *anim = (Animated*) SCM_CDR(obj);
  anim->position[0] = scm_num2dbl(x,FUNC_NAME);
  anim->position[1] = scm_num2dbl(y,FUNC_NAME);
  if(SCM_NUMBERP(z))
	anim->position[2] = scm_num2dbl(z,FUNC_NAME);
  else
	anim->has_moved();
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/**************** set_modtime *************/
SCM_DEFINE (set_modtime, "set-modtime", 3, 0, 0,
	    (SCM s_obj, SCM s_mod, SCM s_time),
	    "Alters time left for object to have modification.")
#define FUNC_NAME s_set_modtime
{
  SCM_ASSERT(IS_ANIMATED(s_obj),s_obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(s_mod),s_mod,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(s_time),s_time,SCM_ARG3,FUNC_NAME);
  int mod = scm_num2int(s_mod,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(mod>=0 && mod < NUM_MODS, s_mod, SCM_ARG2, FUNC_NAME);
  double time = scm_num2dbl(s_time,FUNC_NAME);
  Animated *anim = (Animated*) SCM_CDR(s_obj);
  Ball *ball = dynamic_cast<Ball*>(anim);
  if(ball)
    ball->modTimeLeft[mod] = time;
  return s_obj;
}
#undef FUNC_NAME

/**************** set_acceleration *************/
SCM_DEFINE (set_acceleration, "set-acceleration", 2, 0, 0,
	    (SCM obj, SCM accel),
	    "Sets acceleration of given mr black")
#define FUNC_NAME s_set_acceleration
{
  SCM_ASSERT(IS_ANIMATED(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(accel),accel,SCM_ARG2,FUNC_NAME);
  Animated *anim = (Animated*) SCM_CDR(obj);
  Black *black = dynamic_cast<Black*>(anim);
  if(black) {
    black->acceleration = scm_num2dbl(accel,FUNC_NAME);
  }
  return obj;
}
#undef FUNC_NAME

/**************** set_horizon *************/
SCM_DEFINE (set_horizon, "set-horizon", 2, 0, 0,
	    (SCM obj, SCM horizon),
	    "Sets horizon of given mr black")
#define FUNC_NAME s_set_horizon
{
  SCM_ASSERT(IS_ANIMATED(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(horizon),horizon,SCM_ARG2,FUNC_NAME);
  Animated *anim = (Animated*) SCM_CDR(obj);
  Black *black = dynamic_cast<Black*>(anim);
  if(black)
    black->horizon = scm_num2dbl(horizon,FUNC_NAME);
  return obj;
}
#undef FUNC_NAME


/******************* set_primary_color *************/
SCM_DEFINE (set_primary_color, "set-primary-color", 4, 1, 0,
	    (SCM obj, SCM r, SCM g, SCM b, SCM a),
	    "Sets the primary color of object.")
#define FUNC_NAME s_set_primary_color
{
  SCM_ASSERT(IS_ANIMATED(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(r),r,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(g),g,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(b),b,SCM_ARG4,FUNC_NAME);
  Animated *anim = (Animated*) SCM_CDR(obj);
  anim->primaryColor[0] = scm_num2dbl(r,FUNC_NAME);
  anim->primaryColor[1] = scm_num2dbl(g,FUNC_NAME);
  anim->primaryColor[2] = scm_num2dbl(b,FUNC_NAME);
  if(SCM_NUMBERP(a)) anim->primaryColor[3] = scm_num2dbl(a,FUNC_NAME);
  return obj;
}
#undef FUNC_NAME

/******************* set_secondary_color *************/
SCM_DEFINE (set_secondary_color, "set-secondary-color", 4, 1, 0,
	    (SCM obj, SCM r, SCM g, SCM b, SCM a),
	    "Sets the secondary color (if applicable) of object.")
#define FUNC_NAME s_set_secondary_color
{
  SCM_ASSERT(IS_ANIMATED(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(r),r,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(g),g,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(b),b,SCM_ARG4,FUNC_NAME);
  Animated *anim = (Animated*) SCM_CDR(obj);
  anim->secondaryColor[0] = scm_num2dbl(r,FUNC_NAME);
  anim->secondaryColor[1] = scm_num2dbl(g,FUNC_NAME);
  anim->secondaryColor[2] = scm_num2dbl(b,FUNC_NAME);
  if(SCM_NUMBERP(a)) anim->secondaryColor[3] = scm_num2dbl(a,FUNC_NAME);
  return obj;
}
#undef FUNC_NAME

/******************* set_specular_color *************/
SCM_DEFINE (set_specular_color, "set-specular-color", 4, 1, 0,
	    (SCM obj, SCM r, SCM g, SCM b, SCM a),
	    "Sets the specular color (if applicable) of object.")
#define FUNC_NAME s_set_specular_color
{
  SCM_ASSERT(IS_ANIMATED(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(r),r,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(g),g,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(b),b,SCM_ARG4,FUNC_NAME);
  Animated *anim = (Animated*) SCM_CDR(obj);
  anim->specularColor[0] = scm_num2dbl(r,FUNC_NAME);
  anim->specularColor[1] = scm_num2dbl(g,FUNC_NAME);
  anim->specularColor[2] = scm_num2dbl(b,FUNC_NAME);
  if(SCM_NUMBERP(a)) anim->specularColor[3] = scm_num2dbl(a,FUNC_NAME);
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME


/******************** player ********************/
SCM_DEFINE (player, "player", 0, 0, 0,
	    (),
	    "Returns the current player.")
#define FUNC_NAME s_player
{
  if(Game::current)
    return smobAnimated_make(Game::current->player1);
  else return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/**************** add_flag *************/
SCM_DEFINE (add_flag, "add-flag", 5, 0, 0,
			(SCM x,SCM y,SCM points,SCM visible, SCM radius),
			"Creates a flag giving points when captured.")
#define FUNC_NAME s_add_flag
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,s_add_flag);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,s_add_flag);
  SCM_ASSERT(SCM_NUMBERP(points),points,SCM_ARG3,s_add_flag);
  SCM_ASSERT(SCM_NUMBERP(visible),visible,SCM_ARG4,s_add_flag);
  SCM_ASSERT(SCM_REALP(radius),radius,SCM_ARG5,s_add_flag);
  Flag *flag = new Flag(scm_num2int(x,SCM_ARG1,s_add_flag),scm_num2int(y,SCM_ARG1,s_add_flag),
						scm_num2int(points,SCM_ARG1,s_add_flag),
						scm_num2int(visible,SCM_ARG1,s_add_flag),scm_num2dbl(radius,s_add_flag));
  return smobAnimated_make(flag);
}
#undef FUNC_NAME

/**************** add_colormodifier *************/
SCM_DEFINE (add_colormodifier, "add-colormodifier", 7, 0, 0,
			(SCM col,SCM x,SCM y,SCM min, SCM max,SCM freq, SCM phase),
			"Allow to modify the color(s) of a cell.")
#define FUNC_NAME s_add_colormodifier
{
  SCM_ASSERT(SCM_NUMBERP(col),col,SCM_ARG1,s_add_colormodifier);
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG2,s_add_colormodifier);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG3,s_add_colormodifier);
  SCM_ASSERT(SCM_REALP(min),min,SCM_ARG4,s_add_colormodifier);
  SCM_ASSERT(SCM_REALP(max),max,SCM_ARG5,s_add_colormodifier);
  SCM_ASSERT(SCM_REALP(freq),freq,SCM_ARG6,s_add_colormodifier);
  SCM_ASSERT(SCM_REALP(phase),phase,SCM_ARG7,s_add_colormodifier);
  ColorModifier *colormodifier = new ColorModifier(
     scm_num2int(col,SCM_ARG1,s_add_colormodifier),
     scm_num2int(x,SCM_ARG2,s_add_colormodifier),
     scm_num2int(y,SCM_ARG3,s_add_colormodifier),
     scm_num2dbl(min,s_add_colormodifier),
     scm_num2dbl(max,s_add_colormodifier),
     scm_num2dbl(freq,s_add_colormodifier),
     scm_num2dbl(phase,s_add_colormodifier));
  return smobAnimated_make(colormodifier);
}
#undef FUNC_NAME

/**************** add_heightmodifier *************/
SCM_DEFINE (add_heightmodifier, "add-heightmodifier", 7, 3, 0,
			(SCM corner,SCM x,SCM y,SCM min, SCM max,SCM freq, SCM phase, SCM n1, SCM n2, SCM n3),
			"Allow to modify the height of a cell's corner.")
#define FUNC_NAME s_add_colormodifier
{
  int not1 = -1, not2 = -1, not3 = -1;

  SCM_ASSERT(SCM_NUMBERP(corner),corner,SCM_ARG1,s_add_heightmodifier);
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG2,s_add_heightmodifier);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG3,s_add_heightmodifier);
  SCM_ASSERT(SCM_REALP(min),min,SCM_ARG4,s_add_heightmodifier);
  SCM_ASSERT(SCM_REALP(max),max,SCM_ARG5,s_add_heightmodifier);
  SCM_ASSERT(SCM_REALP(freq),freq,SCM_ARG6,s_add_heightmodifier);
  SCM_ASSERT(SCM_REALP(phase),phase,SCM_ARG7,s_add_heightmodifier);
  if (SCM_NUMBERP(n1))  not1 = scm_num2int(n1,SCM_ARG1,s_add_colormodifier);
  if (SCM_NUMBERP(n2))  not1 = scm_num2int(n2,SCM_ARG1,s_add_colormodifier);
  if (SCM_NUMBERP(n3))  not1 = scm_num2int(n3,SCM_ARG1,s_add_colormodifier);
  HeightModifier *heightmodifier = new HeightModifier(
     scm_num2int(corner,SCM_ARG1,s_add_heightmodifier),
     scm_num2int(x,SCM_ARG2,s_add_heightmodifier),
     scm_num2int(y,SCM_ARG3,s_add_heightmodifier),
     scm_num2dbl(min,s_add_heightmodifier),
     scm_num2dbl(max,s_add_heightmodifier),
     scm_num2dbl(freq,s_add_heightmodifier),
     scm_num2dbl(phase,s_add_heightmodifier),
     not1, not2, not3);
  return smobAnimated_make(heightmodifier);
}
#undef FUNC_NAME

/**************** add_cactus *************/
SCM_DEFINE (add_cactus, "add-cactus", 3, 0, 0,
			(SCM x,SCM y,SCM radius),
			"Creates a cactus at given position.")
#define FUNC_NAME s_add_cactus
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,s_add_flag);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,s_add_flag);
  SCM_ASSERT(SCM_REALP(radius),radius,SCM_ARG5,s_add_cactus);
  Cactus *cactus = new Cactus(scm_num2int(x,SCM_ARG1,s_add_cactus),
                              scm_num2int(y,SCM_ARG1,s_add_cactus),
			      scm_num2dbl(radius,s_add_cactus));
  return smobAnimated_make(cactus);
}
#undef FUNC_NAME


/**************** add_spike *************/
SCM_DEFINE (add_spike, "add-spike", 4, 0, 0,
			(SCM x,SCM y,SCM speed,SCM phase),
			"Creates a lethal spike.")
#define FUNC_NAME s_add_spike
{
  SCM_ASSERT(SCM_REALP(x),x,SCM_ARG1,s_add_spike);
  SCM_ASSERT(SCM_REALP(y),y,SCM_ARG2,s_add_spike);
  SCM_ASSERT(SCM_REALP(speed),speed,SCM_ARG3,s_add_spike);
  SCM_ASSERT(SCM_REALP(phase),phase,SCM_ARG4,s_add_spike);
  Coord3d pos;
  pos[0] = scm_num2dbl(x,s_add_spike);
  pos[1] = scm_num2dbl(y,s_add_spike);
  Spike *spike = new Spike(pos,scm_num2dbl(speed,s_add_spike),scm_num2dbl(phase,s_add_spike));
  return smobAnimated_make(spike);
}
#undef FUNC_NAME

/**************** add_spike *************/
SCM_DEFINE (add_sidespike, "add-sidespike", 5, 0, 0,
			(SCM x,SCM y,SCM speed,SCM phase,SCM side),
			"Creates a lethal spike (comming from side).")
#define FUNC_NAME s_add_sidespike
{
  SCM_ASSERT(SCM_REALP(x),x,SCM_ARG1,s_add_sidespike);
  SCM_ASSERT(SCM_REALP(y),y,SCM_ARG2,s_add_sidespike);
  SCM_ASSERT(SCM_REALP(speed),speed,SCM_ARG3,s_add_sidespike);
  SCM_ASSERT(SCM_REALP(phase),phase,SCM_ARG4,s_add_sidespike);
  SCM_ASSERT(SCM_NUMBERP(side),side,SCM_ARG5,s_add_sidespike);
  Coord3d pos;
  pos[0] = scm_num2dbl(x,s_add_sidespike);
  pos[1] = scm_num2dbl(y,s_add_sidespike);
  SideSpike *sidespike = new SideSpike(pos,scm_num2dbl(speed,s_add_sidespike),
      scm_num2dbl(phase,s_add_sidespike),scm_num2int(side,SCM_ARG5,s_add_sidespike));
  return smobAnimated_make(sidespike);
}
#undef FUNC_NAME

/**************** add_goal **************/
SCM_DEFINE (add_goal, "add-goal", 4, 0, 0,
			(SCM x,SCM y,SCM rotate,SCM nextLevel),
			"Adds a new goal to the map.")
#define FUNC_NAME s_add_goal
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,s_add_goal);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,s_add_goal);  
  SCM_ASSERT(SCM_BOOLP(rotate),rotate,SCM_ARG3,s_add_goal);
  SCM_ASSERT(SCM_STRINGP(nextLevel),nextLevel,SCM_ARG4,s_add_goal);
  char *sname = SCM_STRING_CHARS(nextLevel);
  if(sname) {
    Animated *a = (Animated*) new Goal(scm_num2int(x,SCM_ARG1,s_add_goal),scm_num2int(y,SCM_ARG1,s_add_goal),SCM_NFALSEP(rotate),sname);
    return smobAnimated_make(a);
  }
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/**************** set_onoff ************/
SCM_DEFINE (set_onoff, "set-onoff", 2, 0, 0,
			(SCM obj, SCM state),
			"Turns an object on/off")
#define FUNC_NAME s_set_onoff
{
  SCM_ASSERT(IS_GAMEHOOK(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_BOOLP(state),state,SCM_ARG2,FUNC_NAME);
  GameHook *h = (GameHook*) SCM_CDR(obj);
  h->is_on = SCM_NFALSEP(state);
  return obj;
}
#undef FUNC_NAME


/************* day / night **************/
SCM_DEFINE(day,"day",0,0,0,(),"Turns on the global light for this level.")
{ Game::current->isNight=0; Game::current->wantedFogThickness=0; return SCM_UNSPECIFIED; }
SCM_DEFINE(night,"night",0,0,0,(),"Turns off the global light for this level.")
{ Game::current->isNight=1; Game::current->wantedFogThickness=0; return SCM_UNSPECIFIED; }
SCM_DEFINE(fog,"fog",0,1,0,(SCM v),"Turns on a fog.")
{
  if(!(SCM_NUMBERP(v) && scm_num2dbl(v,s_fog) == 0.0))
	Game::current->isNight=0; 
  if(SCM_NUMBERP(v))
	Game::current->wantedFogThickness=scm_num2dbl(v,s_fog); 
  else
	Game::current->wantedFogThickness=1.0;
  return SCM_UNSPECIFIED; 
}
SCM_DEFINE(thick_fog,"thick-fog",0,0,0,(),"Turns on a thick fog.")
{ Game::current->isNight=0; Game::current->wantedFogThickness=2.0; return SCM_UNSPECIFIED; }

/************* fog_color **************/
SCM_DEFINE(fog_color,"fog-color",3,0,0,(SCM r,SCM g,SCM b),"Specifies color of fog")
#define FUNC_NAME s_fog_color
{
  SCM_ASSERT(SCM_NUMBERP(r),r,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(g),g,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(b),b,SCM_ARG3,FUNC_NAME);
  Game::current->fogColor[0] = scm_num2dbl(r,FUNC_NAME);
  Game::current->fogColor[1] = scm_num2dbl(g,FUNC_NAME);
  Game::current->fogColor[2] = scm_num2dbl(b,FUNC_NAME);
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************* set_bonus_level ************/
SCM_DEFINE(set_bonus_level,"set-bonus-level",1,0,0,(SCM name),
		   "Makes level a bonus level and specified return target.")
{
  SCM_ASSERT(SCM_STRINGP(name),name,SCM_ARG1,"set-bonus-level");  
  if(Game::current) {
	char *sname = SCM_STRING_CHARS(name);
	if(strlen(sname) > 0) {
	  snprintf(Game::current->returnLevel,sizeof(Game::current->returnLevel),"%s",sname);
	  Game::current->map->isBonus=1;
	} else
	  Game::current->map->isBonus=0;
  }
  return SCM_UNSPECIFIED;
}

/************* sign ************/
SCM_DEFINE(sign,"sign",6,1,0,(SCM text, SCM scale, SCM rotation, SCM duration, SCM x, SCM y, SCM z),
		   "Creates a new sign. duration<0 lasts forever.")
#define FUNC_NAME s_sign
{
  SCM_ASSERT(SCM_STRINGP(text),text,SCM_ARG1,FUNC_NAME);
  if(Game::current) {
	char *sname = SCM_STRING_CHARS(text);
	if(strlen(sname) > 0) {
	  Coord3d pos;
	  pos[0] = scm_num2dbl(x,FUNC_NAME);
	  pos[1] = scm_num2dbl(y,FUNC_NAME);
	  if(SCM_NUMBERP(z))
		pos[2] = scm_num2dbl(z,FUNC_NAME);
	  else
		pos[2] = Game::current->map->getHeight(pos[0],pos[1]) + 2.0;
	  Sign *s = new Sign(sname,scm_num2dbl(duration,FUNC_NAME),scm_num2dbl(scale,FUNC_NAME),scm_num2dbl(rotation,FUNC_NAME),pos);
	  return smobAnimated_make(s);
	}
  }
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/**************** set_track_name **************/
SCM_DEFINE (set_track_name, "set-track-name", 1, 0, 0,
			(SCM name),
			"Sets the name of this track.")
#define FUNC_NAME s_set_track_name
{
  SCM_ASSERT(SCM_STRINGP(name),name,SCM_ARG1,s_set_track_name);
  if(Game::current) {
	char *sname = SCM_STRING_CHARS(name);
	strcpy(Game::current->map->mapname,sname);
  }
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/***************** set_author ************/
SCM_DEFINE (set_author, "set-author", 1, 0, 0,
			(SCM name),
			"Sets the name of the author for the current track.")
#define FUNC_NAME s_set_author
{
  SCM_ASSERT(SCM_STRINGP(name),name,SCM_ARG1,s_set_author);
  if(Game::current) {
	char *sname = SCM_STRING_CHARS(name);
	strcpy(Game::current->map->author,sname);
  }
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/*********** start_time ************/
SCM_DEFINE (start_time, "start-time", 1, 0, 0,
			(SCM t),
			"Sets the starting time for this track.")
#define FUNC_NAME s_start_time
{
  int it=scm_num2int(t,SCM_ARG1,FUNC_NAME);
  if(Game::current)
    Game::current->startTime = it;
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/*********** get_time ************/
SCM_DEFINE (get_time, "get-time", 0, 0, 0,(), "Returns how much time the player has.left.")
{ return scm_int2num(Game::current->player1->timeLeft);}

/*********** get_score ************/
SCM_DEFINE (get_score, "get-score", 0, 0, 0,(), "Returns the players score.")
{ return scm_int2num(Game::current->player1->score);}

/*********** set_time ************/
SCM_DEFINE (set_time, "set-time", 1, 0, 0,(SCM t), "Sets the time left for player.")
#define FUNC_NAME s_set_time
{ 
  int it=scm_num2int(t,SCM_ARG1,FUNC_NAME);
  if(Game::current && Game::current->player1)
	Game::current->player1->timeLeft = it;
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/*********** set_score ************/
SCM_DEFINE (set_score, "set-score", 1, 0, 0,(SCM t), "Sets the score for player.")
#define FUNC_NAME s_set_score
{ 
  int it=scm_num2int(t,SCM_ARG1,FUNC_NAME);
  if(Game::current && Game::current->player1)
	Game::current->player1->score = it;
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/*********** add time ************/
SCM_DEFINE (add_time, "add-time", 1, 0, 0,
			(SCM t),
			"Adds time for the user.")
#define FUNC_NAME s_add_time
{
  int it=scm_num2int(t,SCM_ARG1,FUNC_NAME);
  if(Game::current && Game::current->player1)
	Game::current->player1->timeLeft += it;
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/*********** trigger ************/
SCM_DEFINE (trigger,"trigger", 4, 0, 0,
			(SCM x,SCM y,SCM r,SCM expr),
			"Call expr when player is within given radius.")
#define FUNC_NAME s_trigger
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(r),r,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NFALSEP(scm_procedure_p(expr)),expr,SCM_ARG4,FUNC_NAME);
  new Trigger(scm_num2dbl(x,FUNC_NAME),scm_num2dbl(y,FUNC_NAME),scm_num2dbl(r,FUNC_NAME),expr);
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/*********** smart-trigger ************/
SCM_DEFINE (smart_trigger,"smart-trigger", 5, 0, 0,
			(SCM x,SCM y,SCM r,SCM entering,SCM leaving),
			"Call expr when player is within given radius.")
#define FUNC_NAME s_smart_trigger
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(r),r,SCM_ARG3,FUNC_NAME);
  if(SCM_FALSEP(entering)) entering=NULL;
  else SCM_ASSERT(SCM_NFALSEP(scm_procedure_p(entering)),entering,SCM_ARG4,FUNC_NAME);
  if(SCM_FALSEP(leaving)) leaving=NULL;
  else SCM_ASSERT(SCM_NFALSEP(scm_procedure_p(leaving)),leaving,SCM_ARG5,FUNC_NAME);
  new SmartTrigger(scm_num2dbl(x,FUNC_NAME),scm_num2dbl(y,FUNC_NAME),scm_num2dbl(r,FUNC_NAME),entering,leaving);
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/*********** set_start_position *************/
SCM_DEFINE (set_start_position, "set-start-position", 2, 0, 0,
			(SCM x, SCM y),
			"Sets the players start position on this level.")
#define FUNC_NAME s_set_start_position
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  if(Game::current) {
	Game::current->map->startPosition[0] = scm_num2dbl(x,FUNC_NAME);
	Game::current->map->startPosition[1] = scm_num2dbl(y,FUNC_NAME);
	Game::current->map->startPosition[2] = Game::current->map->getHeight(Game::current->map->startPosition[0],
																		 Game::current->map->startPosition[1]);
  }
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/*********** add_modpill ***********/
SCM_DEFINE (add_modpill, "add-modpill", 5, 0, 0,
			(SCM x, SCM y, SCM kind, SCM length, SCM resurrecting),
			"Adds a new modpill to level at x,y with given kind and resurrection state.")
#define FUNC_NAME s_add_modpill
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(kind),kind,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(length),length,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(resurrecting),resurrecting,SCM_ARG5,FUNC_NAME);
  Coord3d position;
  position[0] = scm_num2dbl(x,FUNC_NAME);
  position[1] = scm_num2dbl(y,FUNC_NAME);
  position[2] = 0.0;
  new ModPill(position,scm_num2int(kind,SCM_ARG3,FUNC_NAME),scm_num2int(length,SCM_ARG4,FUNC_NAME),
			  scm_num2int(resurrecting,SCM_ARG5,FUNC_NAME));
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/*********** forcefield ***********/
SCM_DEFINE (forcefield, "forcefield", 8, 0, 0,
			(SCM x, SCM y, SCM z, SCM dx, SCM dy, SCM dz, SCM height, SCM allow),
			"Creats a forcefield")
#define FUNC_NAME s_forcefield
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(z),z,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(dx),dx,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(dy),dy,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(dz),dz,SCM_ARG6,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(height),height,SCM_ARG7,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(allow),allow,SCM_ARG7,FUNC_NAME);

  Coord3d pos, dir;
  pos[0] = scm_num2dbl(x,FUNC_NAME);
  pos[1] = scm_num2dbl(y,FUNC_NAME);
  pos[2] = Game::current->map->getHeight(pos[0],pos[1]) + scm_num2dbl(z,FUNC_NAME);
  dir[0] = scm_num2dbl(dx,FUNC_NAME);
  dir[1] = scm_num2dbl(dy,FUNC_NAME);
  dir[2] = scm_num2dbl(dz,FUNC_NAME);
  ForceField *ff = new ForceField(pos,dir,scm_num2dbl(height,FUNC_NAME),scm_num2int(allow,SCM_ARG7,FUNC_NAME));
  return smobAnimated_make(ff);
}
#undef FUNC_NAME

/*********** switch ***********/
SCM_DEFINE (fun_switch, "switch", 4, 0, 0,
			(SCM x, SCM y, SCM on,SCM off),
			"Creats a switch")
#define FUNC_NAME s_fun_switch
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NFALSEP(scm_procedure_p(on)),on,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NFALSEP(scm_procedure_p(off)),off,SCM_ARG3,FUNC_NAME);
  CSwitch *sw = new CSwitch(scm_num2dbl(x,FUNC_NAME),scm_num2dbl(y,FUNC_NAME),on,off);
  return smobAnimated_make(sw);
}
#undef FUNC_NAME

/*********** pipe **********/
SCM_DEFINE (new_pipe, "pipe", 7, 0, 0,
			(SCM x0,SCM y0,SCM z0, SCM x1,SCM y1,SCM z1,SCM radius),
			"Creates a new pipe")
#define FUNC_NAME s_new_pipe
{
  SCM_ASSERT(SCM_NUMBERP(x0),x0,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y0),y0,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(z0),z0,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(x1),x1,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y1),y1,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(z1),z1,SCM_ARG6,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(radius),radius,SCM_ARG6,FUNC_NAME);
  Coord3d from,to;
  from[0] = scm_num2dbl(x0,FUNC_NAME);
  from[1] = scm_num2dbl(y0,FUNC_NAME);
  from[2] = scm_num2dbl(z0,FUNC_NAME);
  to[0] = scm_num2dbl(x1,FUNC_NAME);
  to[1] = scm_num2dbl(y1,FUNC_NAME);
  to[2] = scm_num2dbl(z1,FUNC_NAME);
  Pipe *pipe = new Pipe(from,to,scm_num2dbl(radius,FUNC_NAME));
  return smobAnimated_make(pipe);
}
#undef FUNC_NAME

/************** set-flag **********/
SCM_DEFINE (set_flag,"set-flag", 3, 0, 0,
			(SCM anim, SCM flag, SCM state),
			"Adds or removes status flag to object")
#define FUNC_NAME s_set_flag
{
  SCM_ASSERT(IS_ANIMATED(anim),anim,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(flag),flag,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_BOOLP(state),state,SCM_ARG3,FUNC_NAME);
  int iflag=scm_num2int(flag,SCM_ARG2,FUNC_NAME);
  Animated *a = (Animated*) SCM_CDR(anim);
  if(SCM_FALSEP(state))
	a->flags = a->flags & (~iflag);
  else
	a->flags = a->flags | iflag;
  return anim;
}
#undef FUNC_NAME

/*********** pipe-connector **********/
SCM_DEFINE (pipe_connector, "pipe-connector", 4, 0, 0,
			(SCM x0,SCM y0,SCM z0, SCM radius),
			"Creates a new pipe connector")
#define FUNC_NAME s_pipe_connector
{
  SCM_ASSERT(SCM_NUMBERP(x0),x0,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y0),y0,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(z0),z0,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(radius),radius,SCM_ARG4,FUNC_NAME);
  Coord3d pos;
  pos[0] = scm_num2dbl(x0,FUNC_NAME);
  pos[1] = scm_num2dbl(y0,FUNC_NAME);
  pos[2] = scm_num2dbl(z0,FUNC_NAME);
  PipeConnector *pipeConnector = new PipeConnector(pos,scm_num2dbl(radius,FUNC_NAME));
  return smobAnimated_make(pipeConnector); 
}
#undef FUNC_NAME

/************* diamond ************/
SCM_DEFINE (diamond,"diamond", 2, 1, 0,
			(SCM x,SCM y,SCM z),
			"Creates a new diamond 'save point'")
#define FUNC_NAME s_diamond
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  Coord3d pos;
  pos[0] = scm_num2dbl(x,FUNC_NAME);
  pos[1] = scm_num2dbl(y,FUNC_NAME);
  pos[2] = Game::current->map->getHeight(pos[0],pos[1])+0.4;
  if(SCM_NUMBERP(z))
	pos[2] = scm_num2dbl(z,FUNC_NAME);
  Diamond *d=new Diamond(pos);
  return smobAnimated_make(d);
}
#undef FUNC_NAME

/*********** set_wind **********/
SCM_DEFINE (set_wind, "set-wind", 3, 0, 0,
			(SCM pipe,SCM forward,SCM backward),
			"Determines forward/backward wind of pipe")
#define FUNC_NAME s_set_wind
{
  SCM_ASSERT(IS_ANIMATED(pipe),pipe,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(forward),forward,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(backward),backward,SCM_ARG3,FUNC_NAME);
  Pipe *p = dynamic_cast<Pipe*>((Animated*)SCM_CDR(pipe));
  if(p) {
	p->windForward = scm_num2dbl(forward,FUNC_NAME);
	p->windBackward = scm_num2dbl(backward,FUNC_NAME);
  }
  return pipe;
}
#undef FUNC_NAME

/**************** snow *************/
SCM_DEFINE (snow, "snow", 1, 0, 0,
	    (SCM strength),
	    "Turns on snow, 0 <= strength <= 1.0 ")
#define FUNC_NAME s_snow
{
  SCM_ASSERT(SCM_NUMBERP(strength),strength,SCM_ARG1,FUNC_NAME);
  Game::current->weather->snow(scm_num2dbl(strength,FUNC_NAME));
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/**************** rain *************/
SCM_DEFINE (rain, "rain", 1, 0, 0,
	    (SCM strength),
	    "Turns on rain, 0 <= strength <= 1.0 ")
#define FUNC_NAME s_rain
{
  SCM_ASSERT(SCM_NUMBERP(strength),strength,SCM_ARG1,FUNC_NAME);
  Game::current->weather->rain(scm_num2dbl(strength,FUNC_NAME));
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/*********** difficulty ***********/
SCM_DEFINE (difficulty, "difficulty", 0, 0, 0,(),"Returns the difficulty we are currently playing on.")
{ return scm_long2num(Settings::settings->difficulty); }

/*********** use-grid ************/
SCM_DEFINE (use_grid, "use-grid", 1, 0, 0, (SCM v), "Turns the grid on/off")
{ Game::current->useGrid=SCM_FALSEP(v)?0:1; return 0;}

/*********** map-is-transparent ************/
SCM_DEFINE (map_is_transparent, "map-is-transparent", 1, 0, 0, (SCM v), "Turns on/off transparency rendering of map.")
{ Game::current->map->isTransparent=SCM_FALSEP(v)?0:1; return 0; }

/*********** jump ************/
SCM_DEFINE (jump, "jump", 1, 0, 0, (SCM v), "Scales maximum jump height of player.")
#define FUNC_NAME s_jump
{ 
  SCM_ASSERT(SCM_NUMBERP(v),v,SCM_ARG1,FUNC_NAME);
  if(Game::current)
	Game::current->jumpFactor=scm_num2dbl(v,FUNC_NAME);
  return 0;
}
#undef FUNC_NAME

/*********** scale-oxygen ************/
SCM_DEFINE (scale_oxygen, "scale-oxygen", 1, 0, 0, (SCM v), "Scales how long player can be under water")
#define FUNC_NAME s_jump
{ 
  SCM_ASSERT(SCM_NUMBERP(v),v,SCM_ARG1,FUNC_NAME);
  if(Game::current)
	Game::current->oxygenFactor=scm_num2dbl(v,FUNC_NAME);
  return 0;
}
#undef FUNC_NAME


/************ set-speed ************/
SCM_DEFINE (set_speed, "set-speed", 2, 0, 0, (SCM obj,SCM speed), "Alters the speed of platforms or spikes")
#define FUNC_NAME s_set_speed
{
  SCM_ASSERT(IS_GAMEHOOK(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(speed),speed,SCM_ARG2,FUNC_NAME);
  Spike *spike = dynamic_cast<Spike*>((Animated*)SCM_CDR(obj));
  if(spike) {
	spike->speed = scm_num2dbl(speed,FUNC_NAME);
  } else {
	CyclicPlatform *platform = dynamic_cast<CyclicPlatform*>((GameHook*)SCM_CDR(obj));
	if(platform) {
	  platform->speed = scm_num2dbl(speed,FUNC_NAME);
	}
  }
  return obj;
}
#undef FUNC_NAME

/************** set-cell-flag ************/
SCM_DEFINE (set_cell_flag, "set-cell-flag", 6, 0, 0, (SCM x0, SCM y0, SCM x1, SCM y1,SCM flag, SCM state), "Modifies the flags in a cell")
#define FUNC_NAME s_set_cell_flag
{
  SCM_ASSERT(SCM_NUMBERP(x0),x0,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y0),y0,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(x1),x1,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y1),y1,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(flag),flag,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_BOOLP(state),state,SCM_ARG6,FUNC_NAME);
  int ix0=scm_num2int(x0,SCM_ARG1,FUNC_NAME),iy0=scm_num2int(y0,SCM_ARG2,FUNC_NAME),
	ix1=scm_num2int(x1,SCM_ARG3,FUNC_NAME),iy1=scm_num2int(y1,SCM_ARG4,FUNC_NAME),
	iflag=scm_num2int(flag,SCM_ARG5,FUNC_NAME);
  for(int x=min(ix0,ix1);x<=max(ix0,ix1);x++)
	for(int y=min(iy0,iy1);y<=max(iy0,iy1);y++) {
	  Cell& c=Game::current->map->cell(x,y);
	  c.displayListDirty=1;
	  if(SCM_FALSEP(state))
		c.flags = c.flags & (~iflag);
	  else
		c.flags = c.flags | iflag;
	}
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************** set-cell-velocity ************/
SCM_DEFINE (set_cell_velocity, "set-cell-velocity", 6, 0, 0, 
			(SCM x0, SCM y0, SCM x1,SCM y1, SCM vx, SCM vy), "Modifies the velocity of a cell")
#define FUNC_NAME s_set_cell_velocity
{
  SCM_ASSERT(SCM_NUMBERP(x0),x0,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y0),y0,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(x1),x1,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y1),y1,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(vx),vx,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(vy),vy,SCM_ARG6,FUNC_NAME);
  int ix0=scm_num2int(x0,SCM_ARG1,FUNC_NAME),iy0=scm_num2int(y0,SCM_ARG2,FUNC_NAME),
	ix1=scm_num2int(x1,SCM_ARG3,FUNC_NAME),iy1=scm_num2int(y1,SCM_ARG4,FUNC_NAME);
  for(int x=min(ix0,ix1);x<=max(ix0,ix1);x++)
	for(int y=min(iy0,iy1);y<=max(iy0,iy1);y++) {
	  Cell& c=Game::current->map->cell(x,y);
	  c.displayListDirty=1;
	  c.velocity[0] = scm_num2dbl(vx,FUNC_NAME);
	  c.velocity[1] = scm_num2dbl(vy,FUNC_NAME);
	}
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************** set-cell-heights ************/
SCM_DEFINE (set_cell_heights, "set-cell-heights", 8, 1, 0, (SCM x0, SCM y0, SCM x1, SCM y1, SCM h0, SCM h1,SCM h2,SCM h3,SCM h4), 
			"Modifies the heights of a cell")
#define FUNC_NAME s_set_cell_heights
{
  SCM_ASSERT(SCM_NUMBERP(x0),x0,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y0),y0,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(x1),x1,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y1),y1,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(h0),h0,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(h1),h1,SCM_ARG6,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(h2),h2,SCM_ARG7,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(h3),h3,SCM_ARG7,FUNC_NAME);
  
  int ix0=scm_num2int(x0,SCM_ARG1,FUNC_NAME),iy0=scm_num2int(y0,SCM_ARG2,FUNC_NAME),
	ix1=scm_num2int(x1,SCM_ARG3,FUNC_NAME),iy1=scm_num2int(y1,SCM_ARG4,FUNC_NAME);
  for(int x=min(ix0,ix1);x<=max(ix0,ix1);x++)
	for(int y=min(iy0,iy1);y<=max(iy0,iy1);y++) {
	  Cell& c=Game::current->map->cell(x,y);
	  c.heights[0] = scm_num2dbl(h0,FUNC_NAME);
	  c.heights[1] = scm_num2dbl(h1,FUNC_NAME);
	  c.heights[2] = scm_num2dbl(h2,FUNC_NAME);
	  c.heights[3] = scm_num2dbl(h3,FUNC_NAME);
	  c.displayListDirty=1;
	  if(SCM_NUMBERP(h4))
		c.heights[4] = scm_num2dbl(h4,FUNC_NAME);
	  else c.heights[4] = (c.heights[0] + c.heights[1] + c.heights[2] + c.heights[3]) / 4.;
	}
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME


/************** set-cell-water-heights ************/
SCM_DEFINE (set_cell_water_heights, "set-cell-water-heights", 8, 1, 0, (SCM x0, SCM y0, SCM x1, SCM y1, SCM h0, SCM h1,SCM h2,SCM h3,SCM h4), 
			"Modifies the water heights of a cell")
#define FUNC_NAME s_set_cell_water_heights
{
  SCM_ASSERT(SCM_NUMBERP(x0),x0,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y0),y0,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(x1),x1,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y1),y1,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(h0),h0,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(h1),h1,SCM_ARG6,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(h2),h2,SCM_ARG7,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(h3),h3,SCM_ARG7,FUNC_NAME);
  
  int ix0=scm_num2int(x0,SCM_ARG1,FUNC_NAME),iy0=scm_num2int(y0,SCM_ARG2,FUNC_NAME),
	ix1=scm_num2int(x1,SCM_ARG3,FUNC_NAME),iy1=scm_num2int(y1,SCM_ARG4,FUNC_NAME);
  for(int x=min(ix0,ix1);x<=max(ix0,ix1);x++)
	for(int y=min(iy0,iy1);y<=max(iy0,iy1);y++) {
	  Cell& c=Game::current->map->cell(x,y);
	  c.waterHeights[0] = scm_num2dbl(h0,FUNC_NAME);
	  c.waterHeights[1] = scm_num2dbl(h1,FUNC_NAME);
	  c.waterHeights[2] = scm_num2dbl(h2,FUNC_NAME);
	  c.waterHeights[3] = scm_num2dbl(h3,FUNC_NAME);
	  c.displayListDirty=1;
	  if(SCM_NUMBERP(h4))
		c.waterHeights[4] = scm_num2dbl(h4,FUNC_NAME);
	  else c.waterHeights[4] = (c.waterHeights[0] + c.waterHeights[1] + c.waterHeights[2] + c.waterHeights[3]) / 4.;
	}
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************** set-cell-colors ************/
SCM_DEFINE (set_cell_colors, "set-cell-colors", 8, 1, 0, (SCM x0, SCM y0, SCM x1, SCM y1, SCM corner, SCM r,SCM g,SCM b, SCM a),
			"Modifies the colors of a cell")
#define FUNC_NAME s_set_cell_colors
{
  SCM_ASSERT(SCM_NUMBERP(x0),x0,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y0),y0,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(x1),x1,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y1),y1,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(corner),corner,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(r),r,SCM_ARG6,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(g),g,SCM_ARG7,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(b),b,SCM_ARG7,FUNC_NAME);
  int i=scm_num2int(corner,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(i>=0&&i<=5,corner,SCM_ARG5,FUNC_NAME);

  int ix0=scm_num2int(x0,SCM_ARG1,FUNC_NAME),iy0=scm_num2int(y0,SCM_ARG2,FUNC_NAME),
	ix1=scm_num2int(x1,SCM_ARG3,FUNC_NAME),iy1=scm_num2int(y1,SCM_ARG4,FUNC_NAME);
  for(int x=min(ix0,ix1);x<=max(ix0,ix1);x++)
	for(int y=min(iy0,iy1);y<=max(iy0,iy1);y++) {
	  Cell& c=Game::current->map->cell(x,y);
	  c.colors[i][0] = (GLfloat) scm_num2dbl(r,FUNC_NAME);
	  c.colors[i][1] = (GLfloat) scm_num2dbl(g,FUNC_NAME);
	  c.colors[i][2] = (GLfloat) scm_num2dbl(b,FUNC_NAME);
	  c.displayListDirty=1;
	  if(SCM_NUMBERP(a))	  
		c.colors[i][3] = (GLfloat) scm_num2dbl(a,FUNC_NAME);
	  else
		c.colors[i][3] = 1.0;
	}
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************** set-cell-wall-colors ************/
SCM_DEFINE (set_cell_wall_colors, "set-cell-wall-colors", 8, 1, 0, (SCM x0, SCM y0, SCM x1, SCM y1, SCM corner, SCM r,SCM g,SCM b, SCM a),
			"Modifies the wall colors of cells")
#define FUNC_NAME s_set_cell_wall_colors
{
  SCM_ASSERT(SCM_NUMBERP(x0),x0,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y0),y0,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(x1),x1,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y1),y1,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(corner),corner,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(r),r,SCM_ARG6,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(g),g,SCM_ARG7,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(b),b,SCM_ARG7,FUNC_NAME);
  int i=scm_num2int(corner,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(i>=0&&i<=4,corner,SCM_ARG5,FUNC_NAME);

  int ix0=scm_num2int(x0,SCM_ARG1,FUNC_NAME),iy0=scm_num2int(y0,SCM_ARG2,FUNC_NAME),
	ix1=scm_num2int(x1,SCM_ARG3,FUNC_NAME),iy1=scm_num2int(y1,SCM_ARG4,FUNC_NAME);
  for(int x=min(ix0,ix1);x<=max(ix0,ix1);x++)
	for(int y=min(iy0,iy1);y<=max(iy0,iy1);y++) {
	  Cell& c=Game::current->map->cell(x,y);
	  c.wallColors[i][0] = (GLfloat) scm_num2dbl(r,FUNC_NAME);
	  c.wallColors[i][1] = (GLfloat) scm_num2dbl(g,FUNC_NAME);
	  c.wallColors[i][2] = (GLfloat) scm_num2dbl(b,FUNC_NAME);
	  c.displayListDirty=1;
	  if(SCM_NUMBERP(a))	  
		c.wallColors[i][3] = (GLfloat) scm_num2dbl(a,FUNC_NAME);
	  else
		c.wallColors[i][3] = 1.0;
	}
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************ play_effect ************/
SCM_DEFINE (play_effect, "play-effect", 1, 0, 0, (SCM name), "Attempts to play a soundeffect")
#define FUNC_NAME s_play_effect
{
  SCM_ASSERT(SCM_STRINGP(name),name,SCM_ARG1,FUNC_NAME);
  playEffect(SCM_STRING_CHARS(name));
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/************* animator **************/
SCM_DEFINE (animator, "animator", 7, 0, 0, (SCM length, SCM position, SCM direction, SCM v0, SCM v1, SCM repeat, SCM fun),
			"Creates an animator object") 
#define FUNC_NAME s_animator
{
  SCM_ASSERT(SCM_NUMBERP(length),length,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(position),position,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(direction),direction,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(v0),v0,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(v1),v1,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(repeat),repeat,SCM_ARG6,FUNC_NAME);
  SCM_ASSERT(SCM_BOOLP(fun)|SCM_NFALSEP(scm_procedure_p(fun)),fun,SCM_ARG7,FUNC_NAME);
  Animator *a = new Animator(scm_num2dbl(length,FUNC_NAME), 
							 scm_num2dbl(position,FUNC_NAME), 
							 scm_num2dbl(direction,FUNC_NAME), 
							 scm_num2dbl(v0,FUNC_NAME), scm_num2dbl(v1,FUNC_NAME), 
							 scm_num2int(repeat,SCM_ARG5,FUNC_NAME), 
							 SCM_BOOLP(fun) ? NULL : fun);
  return smobGameHook_make(a);
}
#undef FUNC_NAME

/************* animator-value **********/
SCM_DEFINE (animator_value, "animator-value", 1, 0, 0, (SCM animator), "Gets the value from an animator object")
#define FUNC_NAME s_animator_value
{
  SCM_ASSERT(IS_GAMEHOOK(animator),animator,SCM_ARG1,FUNC_NAME);
  Animator *a = dynamic_cast<Animator*>((GameHook*)SCM_CDR(animator));
  SCM_ASSERT(a,animator,SCM_ARG1,FUNC_NAME);
  return scm_double2num(a->getValue());
}
#undef FUNC_NAME
			
/************* set-animator-direction **********/
SCM_DEFINE (set_animator_direction, "set-animator-direction", 2, 0, 0, 
			(SCM animator,SCM direction), "Sets the direction of an animator object")
#define FUNC_NAME s_set_animator_direction
{
  SCM_ASSERT(IS_GAMEHOOK(animator),animator,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(direction),direction,SCM_ARG2,FUNC_NAME);
  Animator *a = dynamic_cast<Animator*>((GameHook*)SCM_CDR(animator));
  SCM_ASSERT(a,animator,SCM_ARG1,FUNC_NAME);
  a->direction = scm_num2dbl(direction,FUNC_NAME);
  return animator;
}
#undef FUNC_NAME

/************* set-animator-position **********/
SCM_DEFINE (set_animator_position, "set-animator-position", 2, 0, 0, 
			(SCM animator,SCM position), "Sets the position of an animator object")
#define FUNC_NAME s_set_animator_position
{
  SCM_ASSERT(IS_GAMEHOOK(animator),animator,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(position),position,SCM_ARG2,FUNC_NAME);
  Animator *a = dynamic_cast<Animator*>((GameHook*)SCM_CDR(animator));
  SCM_ASSERT(a,animator,SCM_ARG1,FUNC_NAME);
  a->position = fmod(scm_num2dbl(position,FUNC_NAME),a->length);
  return animator;
}
#undef FUNC_NAME

/************* camera-angle ***********/
SCM_DEFINE (camera_angle,"camera-angle", 2, 0, 0, (SCM xy,SCM z), "Sets camera xy and z-angle")
#define FUNC_NAME s_camera_angle
{ 
  SCM_ASSERT(SCM_NUMBERP(xy),xy,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(z),z,SCM_ARG2,FUNC_NAME);
  ((MainMode*)GameMode::current)->wantedXYAngle = scm_num2dbl(xy,FUNC_NAME);
  ((MainMode*)GameMode::current)->wantedZAngle = scm_num2dbl(z,FUNC_NAME);
  return 0;
}
#undef FUNC_NAME

/************* restart-time ***********/
SCM_DEFINE (restart_time,"restart-time", 1, 0, 0, (SCM t), "Sets the timebonus after death.")
#define FUNC_NAME s_restart_time
{ 
  SCM_ASSERT(SCM_NUMBERP(t),t,SCM_ARG1,FUNC_NAME);
  Game::current->restartBonusTime=scm_num2dbl(t,FUNC_NAME);
  return 0;
}
#undef FUNC_NAME

/************* set-texture ************/
SCM_DEFINE (set_texture,"set-texture", 2, 0, 0, (SCM obj,SCM tname), "Attempts to set the texture of object")
#define FUNC_NAME s_set_texture
{
  SCM_ASSERT(IS_ANIMATED(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_STRINGP(tname),tname,SCM_ARG2,FUNC_NAME);
  char *name=SCM_STRING_CHARS(tname);
  for(int i=0;i<numTextures;i++)
	if(strcmp(name,textureNames[i])==0) {
	  Animated *anim = (Animated*) SCM_CDR(obj);
	  anim->texture=textures[i];
	  return obj;
	}
  return SCM_BOOL(false);
}
#undef FUNC_NAME

/************ fountain **************/
SCM_DEFINE (fountain,"fountain", 6, 0, 0,
			(SCM x, SCM y, SCM z, SCM randomSpeed,SCM radius,SCM strength),
			"Creates a new fountain object")
#define FUNC_NAME s_fountain
{
  SCM_ASSERT(SCM_NUMBERP(x),x,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(y),y,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(z),z,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(randomSpeed),randomSpeed,SCM_ARG4,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(radius),radius,SCM_ARG5,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(strength),strength,SCM_ARG6,FUNC_NAME);
  Fountain *fountain = new Fountain(scm_num2dbl(randomSpeed,FUNC_NAME),scm_num2dbl(radius,FUNC_NAME),scm_num2dbl(strength,FUNC_NAME));
  fountain->position[0]=scm_num2dbl(x,FUNC_NAME);
  fountain->position[1]=scm_num2dbl(y,FUNC_NAME);
  fountain->position[2]=scm_num2dbl(z,FUNC_NAME);
  return smobAnimated_make(fountain);
}
#undef FUNC_NAME

/************* set-fountain-strength *************/
SCM_DEFINE (set_fountain_strength,"set-fountain-strength", 2, 0, 0, (SCM obj, SCM str),
			"Sets the strength of a fountain object")
#define FUNC_NAME s_set_fountain_strength
{
  SCM_ASSERT(IS_ANIMATED(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(str),str,SCM_ARG2,FUNC_NAME);
  Fountain *fountain=dynamic_cast<Fountain*>((Animated*)SCM_CDR(obj));
  SCM_ASSERT(fountain,obj,SCM_ARG1,FUNC_NAME);
  fountain->strength=scm_num2dbl(str,FUNC_NAME);
  return obj;
}
#undef FUNC_NAME

/************* fountain-velocity *************/
SCM_DEFINE (set_fountain_velocity,"set-fountain-velocity", 4, 0, 0, (SCM obj, SCM vx,SCM vy,SCM vz),
			"Sets the velcity of ougoing droplets from a fountain object")
#define FUNC_NAME s_set_fountain_velocity
{
  SCM_ASSERT(IS_ANIMATED(obj),obj,SCM_ARG1,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(vx),vx,SCM_ARG2,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(vy),vy,SCM_ARG3,FUNC_NAME);
  SCM_ASSERT(SCM_NUMBERP(vz),vz,SCM_ARG4,FUNC_NAME);
  Fountain *fountain=dynamic_cast<Fountain*>((Animated*)SCM_CDR(obj));
  SCM_ASSERT(fountain,obj,SCM_ARG1,FUNC_NAME);
  fountain->velocity[0]=scm_num2dbl(vx,FUNC_NAME);
  fountain->velocity[1]=scm_num2dbl(vy,FUNC_NAME);
  fountain->velocity[2]=scm_num2dbl(vz,FUNC_NAME);
  return obj;
}
#undef FUNC_NAME


/******** Constants *******/
SCM_GLOBAL_VARIABLE_INIT(s_mod_speed,"*mod-speed*",scm_long2num(MOD_SPEED));
SCM_GLOBAL_VARIABLE_INIT(s_mod_jump,"*mod-jump*",scm_long2num(MOD_JUMP));
SCM_GLOBAL_VARIABLE_INIT(s_mod_spike,"*mod-spike*",scm_long2num(MOD_SPIKE));
SCM_GLOBAL_VARIABLE_INIT(s_mod_glass,"*mod-glass*",scm_long2num(MOD_GLASS));
SCM_GLOBAL_VARIABLE_INIT(s_mod_dizzy,"*mod-dizzy*",scm_long2num(MOD_DIZZY));
SCM_GLOBAL_VARIABLE_INIT(s_mod_frozen,"*mod-frozen*",scm_long2num(MOD_FROZEN));
SCM_GLOBAL_VARIABLE_INIT(s_mod_float,"*mod-float*",scm_long2num(MOD_FLOAT));
SCM_GLOBAL_VARIABLE_INIT(s_mod_extra_life,"*mod-extra-life*",scm_long2num(MOD_EXTRA_LIFE));
SCM_GLOBAL_VARIABLE_INIT(s_mod_large,"*mod-large*",scm_long2num(MOD_LARGE));
SCM_GLOBAL_VARIABLE_INIT(s_mod_small,"*mod-small*",scm_long2num(MOD_SMALL));
SCM_GLOBAL_VARIABLE_INIT(s_mod_nitro,"*mod-nitro*",scm_long2num(MOD_NITRO));

SCM_GLOBAL_VARIABLE_INIT(s_easy,"*easy*",scm_long2num(0));
SCM_GLOBAL_VARIABLE_INIT(s_normal,"*normal*",scm_long2num(1));
SCM_GLOBAL_VARIABLE_INIT(s_hard,"*hard*",scm_long2num(2));

SCM_GLOBAL_VARIABLE_INIT(s_ff_nothing,"*ff-nothing*",scm_long2num(0));
SCM_GLOBAL_VARIABLE_INIT(s_ff_kill1,"*ff-kill1*",scm_long2num(FF_KILL1));
SCM_GLOBAL_VARIABLE_INIT(s_ff_bounce1,"*ff-bounce1*",scm_long2num(FF_BOUNCE1));
SCM_GLOBAL_VARIABLE_INIT(s_ff_kill2,"*ff-kill2*",scm_long2num(FF_KILL2));
SCM_GLOBAL_VARIABLE_INIT(s_ff_bounce2,"*ff-bounce2*",scm_long2num(FF_BOUNCE2));
SCM_GLOBAL_VARIABLE_INIT(s_ff_bounce,"*ff-bounce*",scm_long2num(FF_BOUNCE1+FF_BOUNCE2));
SCM_GLOBAL_VARIABLE_INIT(s_ff_kill,"*ff-kill*",scm_long2num(FF_KILL1+FF_KILL2));

SCM_GLOBAL_VARIABLE_INIT(s_soft_enter,"*soft-enter*",scm_long2num(PIPE_SOFT_ENTER));
SCM_GLOBAL_VARIABLE_INIT(s_soft_exit,"*soft-exit*",scm_long2num(PIPE_SOFT_EXIT));

SCM_GLOBAL_VARIABLE_INIT(s_cell_ice,"*cell-ice*",scm_long2num(CELL_ICE));
SCM_GLOBAL_VARIABLE_INIT(s_cell_acid,"*cell-acid*",scm_long2num(CELL_ACID));
SCM_GLOBAL_VARIABLE_INIT(s_cell_sand,"*cell-sand*",scm_long2num(CELL_SAND));
SCM_GLOBAL_VARIABLE_INIT(s_cell_kill,"*cell-kill*",scm_long2num(CELL_KILL));
SCM_GLOBAL_VARIABLE_INIT(s_cell_trampoline,"*cell-trampoline*",scm_long2num(CELL_TRAMPOLINE));
SCM_GLOBAL_VARIABLE_INIT(s_cell_nogrid,"*cell-nogrid*",scm_long2num(CELL_NOGRID));
SCM_GLOBAL_VARIABLE_INIT(s_cell_track,"*cell-track*",scm_long2num(CELL_TRACK));

SCM_GLOBAL_VARIABLE_INIT(s_animator_0_remove,"*animator-0-remove*",scm_long2num(ANIMATOR_0_REMOVE));
SCM_GLOBAL_VARIABLE_INIT(s_animator_0_stop,"*animator-0-stop*",scm_long2num(ANIMATOR_0_STOP));
SCM_GLOBAL_VARIABLE_INIT(s_animator_0_bounce,"*animator-0-bounce*",scm_long2num(ANIMATOR_0_BOUNCE));
SCM_GLOBAL_VARIABLE_INIT(s_animator_0_wrap,"*animator-0-wrap*",scm_long2num(ANIMATOR_0_WRAP));
SCM_GLOBAL_VARIABLE_INIT(s_animator_1_remove,"*animator-1-remove*",scm_long2num(ANIMATOR_1_REMOVE));
SCM_GLOBAL_VARIABLE_INIT(s_animator_1_stop,"*animator-1-stop*",scm_long2num(ANIMATOR_1_STOP));
SCM_GLOBAL_VARIABLE_INIT(s_animator_1_bounce,"*animator-1-bounce*",scm_long2num(ANIMATOR_1_BOUNCE));
SCM_GLOBAL_VARIABLE_INIT(s_animator_1_wrap,"*animator-1-wrap*",scm_long2num(ANIMATOR_1_WRAP));
SCM_GLOBAL_VARIABLE_INIT(s_animator_remove,"*animator-remove*",scm_long2num(ANIMATOR_0_REMOVE+ANIMATOR_1_REMOVE));
SCM_GLOBAL_VARIABLE_INIT(s_animator_stop,"*animator-stop*",scm_long2num(ANIMATOR_0_STOP+ANIMATOR_1_STOP));
SCM_GLOBAL_VARIABLE_INIT(s_animator_wrap,"*animator-bounce*",scm_long2num(ANIMATOR_0_BOUNCE+ANIMATOR_1_BOUNCE));
SCM_GLOBAL_VARIABLE_INIT(s_animator_bounce,"*animator-wrap*",scm_long2num(ANIMATOR_0_WRAP+ANIMATOR_1_WRAP));

SCM_GLOBAL_VARIABLE_INIT(s_cell_ne,"*cell-ne*",scm_long2num(Cell::NORTH+Cell::EAST));
SCM_GLOBAL_VARIABLE_INIT(s_cell_nw,"*cell-nw*",scm_long2num(Cell::NORTH+Cell::WEST));
SCM_GLOBAL_VARIABLE_INIT(s_cell_se,"*cell-se*",scm_long2num(Cell::SOUTH+Cell::EAST));
SCM_GLOBAL_VARIABLE_INIT(s_cell_sw,"*cell-sw*",scm_long2num(Cell::SOUTH+Cell::WEST));
SCM_GLOBAL_VARIABLE_INIT(s_cell_center,"*cell-center*",scm_long2num(Cell::CENTER));

/** flags for animated objects **/
SCM_GLOBAL_VARIABLE_INIT(s_bird_ch,"*bird-constant-height*",scm_long2num(BIRD_CONSTANT_HEIGHT));


void initGuileInterface() {

  smobAnimated_tag = scm_make_smob_type("Animated", 0);
  scm_set_smob_free(smobAnimated_tag,smobAnimated_free);

  smobGameHook_tag = scm_make_smob_type("GameHook", 0);
  scm_set_smob_free(smobGameHook_tag,smobGameHook_free);

#include "guile.cc.x"  
}
