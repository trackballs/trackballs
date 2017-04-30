/* sound.cc
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

#include "general.h"
#include "SDL/SDL_mixer.h"
#include "sound.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "settings.h"

using namespace std;

SDL_AudioSpec audioFormat;

#define MAX_EFFECTS 1024
const char *wavs[MAX_EFFECTS];
Mix_Chunk *effects[MAX_EFFECTS];
int n_effects = 0;  // YP: was not initialized !

#define MAX_SONGS 1024  // in case someone makes a symlink to his real music directory..
Mix_Music *music[MAX_SONGS];
int n_songs = 0;
int mute = 1;

void soundInit() {
  mute = 0;
  char str[512], i;
  DIR *dir;
  struct dirent *dirent;

  if (!silent) {
    printf("Attempting to open mixer...");
    fflush(stdout);
  }
  if (Mix_OpenAudio(22050, AUDIO_S16, 2, 4096) < 0) {
    printf("failed\n");
    fprintf(stderr,
            "Couldn't open audio: %s\nTry shutting down artsd/esd or run trackballs through "
            "artsdsp/esddsp\n",
            SDL_GetError());
    mute = 1;
    return;
  }
  if (!silent) printf("successfull\n");

  snprintf(str, sizeof(str), "%s/sfx", SHARE_DIR);
  dir = opendir(str);
  if (dir) {
    while ((dirent = readdir(dir))) {
      if (strlen(dirent->d_name) > 4 &&
          (strcmp(&dirent->d_name[strlen(dirent->d_name) - 4], ".wav") == 0)) {
        snprintf(str, sizeof(str), "%s/sfx/%s", SHARE_DIR, dirent->d_name);
        effects[n_effects] = Mix_LoadWAV(str);
        if (effects[n_effects]) {
          wavs[n_effects] = strdup(dirent->d_name);
          n_effects++;
        } else
          printf("Warning: Error when loading '%s'\n", str);
      }
    }
    closedir(dir);
  }

  n_songs = 0;
  snprintf(str, sizeof(str), "%s/music", SHARE_DIR);
  dir = opendir(str);
  if (dir) {
    while ((dirent = readdir(dir))) {
      if (strlen(dirent->d_name) > 4 &&
          (strcmp(&dirent->d_name[strlen(dirent->d_name) - 4], ".ogg") == 0 ||
           strcmp(&dirent->d_name[strlen(dirent->d_name) - 4], ".mp3") == 0)) {
        snprintf(str, sizeof(str), "%s/music/%s", SHARE_DIR, dirent->d_name);
        music[n_songs] = Mix_LoadMUS(str);
        if (!music[n_songs++]) printf("Warning: Error when loading '%s'\n", str);
      }
    }
    closedir(dir);
  }

  /*
  for(i=0;i<N_SONGS;i++) {
        sprintf(str,"%s/music/%s",SHARE_DIR,songs[i]);
        music[i] = Mix_LoadMUS(str);
        if(!music[i])
          printf("Warning: Failed to load '%s'\n",str);
          }*/
}

int doSpecialSfx = 0;
double musicFade = 1.0;

void soundIdle() {
  if (mute) return;
  int i;

  if (doSpecialSfx) {
    musicFade = max(0.0, musicFade - 0.3 / fps);
    if (musicFade == 0.0) {
      Mix_Volume(1, (int)(128 * Settings::settings->sfxVolume));
      Mix_PlayChannel(1, effects[doSpecialSfx], 0);
      doSpecialSfx = 0;
    }
  } else if (!Mix_Playing(1))
    musicFade = min(1.0, musicFade + 0.3 / fps);
  Mix_VolumeMusic((int)(musicFade * 127.0 * Settings::settings->musicVolume));

  /* TODO. Alternative songs... */
  if (!Mix_PlayingMusic() && n_songs) {
    i = (rand() >> 7) % n_songs;
    if (music[i]) Mix_FadeInMusic(music[i], 1, 2000);
  }
}

void playEffect(int e, float vol) {
  if (mute) return;
  vol = vol * Settings::settings->sfxVolume;
  if (e >= 0 && e < N_EFFECTS && effects[e]) {
    if (strcmp(wavs[e], SFX_LV_COMPLETE) == 0) {
      doSpecialSfx = e;
      musicFade = 1.0;
    } else if (!Mix_Playing(0)) {
      Mix_Volume(0, (int)(vol * 128));
      Mix_PlayChannel(0, effects[e], 0);
    } else if (!Mix_Playing(2)) {
      Mix_Volume(2, (int)(vol * 128));
      Mix_PlayChannel(2, effects[e], 0);
    } else if (!Mix_Playing(3)) {
      Mix_Volume(3, (int)(vol * 128));
      Mix_PlayChannel(3, effects[e], 0);
    } else if (!Mix_Playing(4)) {
      Mix_Volume(4, (int)(vol * 128));
      Mix_PlayChannel(4, effects[e], 0);
    }
  }
}
void playEffect(int e) { playEffect(e, 1.); }

void playEffect(const char *name) {
  if (mute) return;
  for (int e = 0; e < N_EFFECTS; e++)
    if (effects[e] && strcmp(wavs[e], name) == 0) {
      playEffect(e, 1.);
      return;
    }
}
void playEffect(const char *name, float vol) {
  if (mute) return;
  for (int e = 0; e < N_EFFECTS; e++)
    if (effects[e] && strcmp(wavs[e], name) == 0) {
      playEffect(e, vol);
      return;
    }
}

void volumeChanged() {
  if (mute) return;
  Mix_Volume(0, (int)(127.0 * Settings::settings->sfxVolume));
  Mix_VolumeMusic((int)(127.0 * Settings::settings->musicVolume));
}
