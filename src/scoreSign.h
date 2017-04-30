/* template.h
   Used to create new header files

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

#ifndef SCORESIGN_H
#define SCORESIGN_H

#ifndef SIGN_H
#include "sign.h"
#endif


class ScoreSign : public Sign {
 public:
  ScoreSign(int points,Coord3d position);
  void tick(Real);

 private:
  Real pointsLeft,points,pointsPerSecond;
  Real lastLife;
};

#endif
