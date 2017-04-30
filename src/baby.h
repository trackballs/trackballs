/* baby.h
   Implements "baby-balls". Eg, balls which can killed and gives points

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

#ifndef BABY_H
#define BABY_H

class Baby : public Black {
 public:
  Baby(double x, double y);
  ~Baby();
  void die();

  Boolean crash(Real speed,
                Ball *);  // When we crash into another ball. Returns if we are still alive
};

#endif
