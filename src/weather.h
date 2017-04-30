/* weather.h
   Adds purely decorative snow and rain

   Copyright (C) 2003-2004  Mathias Broxvall

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

#ifndef WEATHER_H
#define WEATHER_H

#define WEATHER_SNOW 0
#define WEATHER_RAIN 1

class Weather {
 public:
  Weather();

  void clear();
  void draw2();
  void tick(Real t);
  void snow(double strength);
  void rain(double strength);

 private:
  typedef struct sParticle {
	double position[3];
	double velocity[3];
	double size;
	double corners[3][3]; /* For snow */
  } Particle;
  Particle particles[3000];
  int next,nextSnowDrift;
  int max_weather_particles;

  int kind;
  double strength;
};

#endif
