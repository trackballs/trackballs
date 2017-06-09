/* sound.h
   Handles all sound events / background music

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

#ifndef SOUND_H
#define SOUND_H

#include "general.h"

void soundInit();
void soundIdle(Real td);
void playEffect(int, float vol);
void playEffect(int);
void volumeChanged();
void playEffect(const char *name);
void playEffect(const char *name, float vol);
void clearMusicPreferences();
void setMusicPreference(char *name, int weight);
void playNextSong();

#define SFX_PLAYER_DIES "die.wav"  // 0
#define SFX_START "start.wav"      // 1
#define SFX_GOT_LIFE "start.wav"
#define SFX_GOT_FLAG "flag.wav"  // 2
#define SFX_GOT_GOODPILL "goodpill.wav"
#define SFX_GOT_BADPILL "badpill.wav"
#define SFX_GOT_SAVEPOINT "savepoint.wav"
#define SFX_LV_COMPLETE "levelComplete.wav"  // 3
#define SFX_TIMEOUT "timeout.wav"
#define SFX_BLACK_DIE "blackDie.wav"         // 4
#define SFX_TIME_WARNING "time_warning.wav"  // 5
#define SFX_FF_DEATH "ff_death.wav"          // 6
#define SFX_SAND_CRASH "sandcrash.wav"       // 7
#define SFX_SWITCH "switch.wav"              // 8
#define SFX_CACTUS_DIE "blackDie.wav"        // 9
#define SFX_BIRD_DIE "blackDie.wav"          // 10
#define SFX_SPIKE "switch.wav"               // 11
#define N_EFFECTS n_effects                  // 12

/* Needed sound effects
   SFX_CACTUS_DIE  - maybe some "squishy"
   SFX_BIRD_DIE    - a bird shriek?
   SFX_SPIKE       -

   sound when getting diamong (currently is SFX_GOT_FLAG)
   SFX_TELEPORT    -
   SFX_TELEPORT_ABIENT    -


   Maybe imrpove
   ff_death
   sandcrash
*/

#endif
