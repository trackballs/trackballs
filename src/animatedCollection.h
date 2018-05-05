/* animatedCollection.h
   Routines for a collection of objects, esp. intersection finding

   Copyright (C) 2017  Manuel Stoeckl

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

#ifndef ANIMATED_COLLECTION_H
#define ANIMATED_COLLECTION_H

#include "animated.h"

class AnimatedCollection {
 public:
  AnimatedCollection();
  ~AnimatedCollection();
  void add(Animated*);
  void reserve(int N);
  void clear();

  void recalculateBboxMap();
  size_t bboxOverlapsWith(const Animated*, Animated*** ret) const;
  size_t bboxOverlapsWith(const double lower[3], const double upper[3], Animated*** ret) const;

 private:
  void* map;
  void* alloc;
  void** retlist;
  int ntot;
  int nreserved;
  int* rect_indices;
  void* input;
};

#endif /* ANIMATED_COLLECTION_H */
