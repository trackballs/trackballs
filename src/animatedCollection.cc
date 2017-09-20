/* animatedCollection.cc
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

#include "animatedCollection.h"

#include <vector>
/* For std::sort */
#include <algorithm>

static int AC_DEBUG = 0;

template<unsigned int D>
struct Rectangle {
  Real lower[D];
  Real upper[D];
  void* tag;
};

typedef struct {
  Real r;
  void* tag;
} PtAscending;
typedef struct {
  Real r;
  void* tag;
} PtDescending;
bool operator<(const PtAscending& l, const PtAscending& r) { return l.r < r.r; }
bool operator<(const PtDescending& l, const PtDescending& r) { return l.r > r.r; }

class SingleCycleAllocator {
 public:
  SingleCycleAllocator(size_t maxbytes) {
    data = new char[maxbytes];
    index = 0;
    maxlen = maxbytes;
  }
  ~SingleCycleAllocator() { delete[] data; }
  void* allocate(size_t bytes) {
    char* ret = &data[index];
    index += bytes;
    if (index >= maxlen) { error("Tried to allocate block %d over cap %d", bytes, maxlen); }
    return ret;
  }
  template<class T>
  T* allocateFor() {
    return (T*)allocate(sizeof(T));
  }
  size_t used() { return index; }

 private:
  char* data;
  size_t index;
  size_t maxlen;
};

/* The auxiliary tree for D=1 */
class OverlapTree1D {
 public:
  OverlapTree1D() {
    N = 0;
    lendpoints = NULL;
    rendpoints = NULL;
  }
  template<unsigned int K>
  void init(const std::vector<struct Rectangle<K> >& rectangles, int* input, int iN,
            SingleCycleAllocator* alloc) {
    N = iN;
    /* Since this is never modified, we use a sorted list rather than a tree */
    lendpoints = (PtAscending*)alloc->allocate(sizeof(PtAscending) * N);
    rendpoints = (PtDescending*)alloc->allocate(sizeof(PtDescending) * N);
    for (int i = 0; i < N; i++) {
      PtAscending lpt;
      lpt.tag = rectangles[input[i]].tag;
      lpt.r = rectangles[input[i]].lower[D - 1];
      lendpoints[i] = lpt;
      PtDescending rpt;
      rpt.tag = rectangles[input[i]].tag;
      rpt.r = rectangles[input[i]].upper[D - 1];
      rendpoints[i] = rpt;
    }
    std::sort(lendpoints, lendpoints + N);
    std::sort(rendpoints, rendpoints + N);
  }
  ~OverlapTree1D() {
    delete[] lendpoints;
    delete[] rendpoints;
  }
  operator bool() const { return N > 0; }

  int allTags(std::vector<void*>& acc) const {
    for (int i = 0; i < N; i++) { acc.push_back(lendpoints[i].tag); }
    return N;
  }
  int numTags() const { return N; }
  int leftEndsLeftOf(std::vector<void*>& acc, Real ubound) const {
    int nfound = 0;
    for (int i = 0; i < N; i++) {
      if (lendpoints[i].r <= ubound) {
        acc.push_back(lendpoints[i].tag);
        nfound++;
      } else {
        break;
      }
    }
    return nfound;
  }
  int rightEndsRightOf(std::vector<void*>& acc, Real lbound) const {
    int nfound = 0;
    for (int i = 0; i < N; i++) {
      if (rendpoints[i].r >= lbound) {
        acc.push_back(rendpoints[i].tag);
      } else {
        break;
      }
    }
    return nfound;
  }
  void dumpTree(int depth = 0) const {
    char* lsp = new char[2 * depth + 2];
    memset(lsp, ' ', 2 * depth + 2);
    lsp[2 * depth + 1] = '\0';
    warning("%s>A %d", lsp, N);
    for (int i = 0; i < N; i++) {
      warning("%s - | %f %d | %f %d |", lsp, lendpoints[i].r, lendpoints[i].tag,
              rendpoints[i].r, rendpoints[i].tag);
    }
    delete[] lsp;
  }

  static size_t maxAllocatedMemoryUse(size_t Nrects) {
    return sizeof(OverlapTree1D) + sizeof(PtAscending) * Nrects +
           sizeof(PtDescending) * Nrects;
  }

 private:
  static const int D = 1;

  int N;
  PtAscending* lendpoints;
  PtDescending* rendpoints;
};

template<unsigned int D>
class DFoldRectangleTree;

template<unsigned int D>
class OverlapTree {
 public:
  /* Constructor and destructor are delayed */
  OverlapTree() {
    N = 0;
    lendpoints = NULL;
    rendpoints = NULL;
    ltrees = NULL;
    rtrees = NULL;
    atree = NULL;
  }
  template<unsigned int K>
  void init(const std::vector<struct Rectangle<K> >& rectangles, int* input, int N,
            SingleCycleAllocator* alloc);
  ~OverlapTree();
  operator bool() const { return N > 0; }

  DFoldRectangleTree<D - 1>* allTags() const { return atree; }
  std::vector<DFoldRectangleTree<D - 1>*> leftEndsLeftOf(Real rbound) const;
  std::vector<DFoldRectangleTree<D - 1>*> rightEndsRightOf(Real lbound) const;

  void dumpTree(int depth = 0) const {
    char* lsp = new char[2 * depth + 2];
    memset(lsp, ' ', 2 * depth + 2);
    lsp[2 * depth + 1] = '\0';
    warning("%s>A %d", lsp, N);
    for (int i = 0; i < N; i++) {
      warning("%s - | %f | %f |", lsp, lendpoints[i].r, rendpoints[i].r);
    }
    atree->dumpTree(depth + 1);
  }

  static size_t maxAllocatedMemoryUse(size_t Nrects) {
    size_t total = sizeof(OverlapTree<D>);
    total += (sizeof(PtAscending) + sizeof(PtDescending)) * Nrects;
    if (Nrects <= 1) {
      total += DFoldRectangleTree<D - 1>::maxMemoryUse(1);
      return total;
    }

    size_t j = 1;
    while (j < Nrects) {
      int ofsizej = 2 * Nrects / j;
      total += ofsizej * DFoldRectangleTree<D - 1>::maxMemoryUse(j);
      j *= 2;
    }
    total += sizeof(DFoldRectangleTree<D - 1>*) * j * 2;
    return total;
  }

 private:
  int N;
  PtAscending* lendpoints;
  PtDescending* rendpoints;
  DFoldRectangleTree<D - 1>** ltrees;
  DFoldRectangleTree<D - 1>** rtrees;
  DFoldRectangleTree<D - 1>* atree;
};

template<unsigned int D>
OverlapTree<D>::~OverlapTree() {
  if (N > 1) {
    int k = 1;
    while (k < N) { k *= 2; }
    for (int i = 0; i < k; i++) {
      if (ltrees[i]) delete ltrees[i];
      if (rtrees[i]) delete rtrees[i];
    }
    delete[] ltrees;
    delete[] rtrees;
  }
  delete[] lendpoints;
  delete[] rendpoints;
  delete atree;
}

typedef enum {
  NoContainedAncestors = 0,
  RightOfFirstContainment = 1,
  LeftOfFirstContainment = -1
} AncTestValues;

static int appendTo(std::vector<void*>& l, const std::vector<void*>& r) {
  l.insert(l.end(), r.begin(), r.end());
  return r.size();
}
template<unsigned int D, unsigned int K>
Real computeSplitpoint(const std::vector<struct Rectangle<K> >& rectangles, int* input,
                       int N) {
  /* sort the 2d points and pick the center */
  Real* dco = new Real[2 * N];
  for (int i = 0; i < N; i++) {
    dco[2 * i] = rectangles[input[i]].lower[D - 1];
    dco[2 * i + 1] = rectangles[input[i]].upper[D - 1];
  }
  std::sort(dco, dco + 2 * N);
  Real splitpoint = 0.5 * (dco[N - 1] + dco[N]);
  delete[] dco;
  return splitpoint;
}

template<unsigned int D, unsigned int K>
Real splitRectangles(const std::vector<struct Rectangle<K> >& rectangles, int* input, int N,
                     int& nleft, int& noverlap, int& nright) {
  if (!N) { error("Must have rectangles to split"); }
  nleft = 0;
  nright = 0;
  noverlap = 0;
  if (N == 1) {
    noverlap = 1;
    return 0.5 * (rectangles[input[0]].lower[D - 1] + rectangles[input[0]].upper[D - 1]);
  }

  Real splitpoint = computeSplitpoint<D, K>(rectangles, input, N);

  /* Divide list into three segments */
  for (int i = 0; i < N; i++) {
    int p = nleft + noverlap;
    Real lbound = rectangles[input[p]].lower[D - 1];
    Real ubound = rectangles[input[p]].upper[D - 1];
    if (splitpoint > ubound) {
      /* Rectangle entirely below the split point */
      std::swap(input[p], input[nleft]);
      nleft++;
    } else if (splitpoint < lbound) {
      /* Rectangle entirely above the split point */
      std::swap(input[p], input[N - 1 - nright]);
      nright++;
    } else {
      /* Rectangle overlaps the split point */
      noverlap++;
    }
  }

  if (nleft == N) { error("Left size fail: %d %d %d (%d)", nleft, noverlap, nright, N); }
  if (nright == N) { error("Right size fail: %d %d %d (%d)", nleft, noverlap, nright, N); }
  return splitpoint;
}

template<unsigned int D>
class DFoldRectangleTree {
 public:
  template<unsigned int K>
  DFoldRectangleTree(const std::vector<struct Rectangle<K> >& rectangles, int* input, int N,
                     SingleCycleAllocator* alloc)
      : auxtree(), lefttree(NULL), righttree(NULL) {
    int nleft, noverlap, nright;
    splitpoint = splitRectangles<D, K>(rectangles, input, N, nleft, noverlap, nright);

    /* Construct left and right trees */
    if (nleft) {
      lefttree = alloc->allocateFor<DFoldRectangleTree<D> >();
      new (lefttree) DFoldRectangleTree<D>(rectangles, &input[0], nleft, alloc);
    }
    if (nright) {
      righttree = alloc->allocateFor<DFoldRectangleTree<D> >();
      new (righttree)
          DFoldRectangleTree<D>(rectangles, &input[nleft + noverlap], nright, alloc);
    }

    /* Project middle subtree down a dimension */
    if (noverlap) { auxtree.init(rectangles, &input[nleft], noverlap, alloc); }
  }
  ~DFoldRectangleTree() {
    if (lefttree) delete lefttree;
    if (righttree) delete righttree;
  }

  /* This coordinate passes all tests, so drop down to the next coordinate */
  int allsub(std::vector<void*>& acc, Real low[D], Real high[D]) const {
    int ncount = 0;
    if (auxtree) { ncount += auxtree.allTags()->intersect(acc, low, high); }
    if (lefttree) { ncount += lefttree->allsub(acc, low, high); }
    if (righttree) { ncount += righttree->allsub(acc, low, high); }
    return ncount;
  }

  /* Returns the number of intersections found, and adds indices corresponding
   * to the order of addition to the index accumulator. */
  int intersect(std::vector<void*>& acc, Real low[D], Real high[D],
                AncTestValues _anctest = NoContainedAncestors) const {
    /* Return the 'tag' list for the rectangles */
    int nfound = 0;
    Real lbound = low[D - 1];
    Real rbound = high[D - 1];
    if (AC_DEBUG) {
      warning("intersect %d %f [%f,%f] %d %x", D, splitpoint, lbound, rbound, _anctest, this);
    }

    if (_anctest == LeftOfFirstContainment) {
      /* An ancestor of this node is in [lbound,ubound],
       * and the current splitpoint on the left branch relative to that node */
      if (splitpoint >= lbound) {
        if (lefttree) nfound += lefttree->intersect(acc, low, high, LeftOfFirstContainment);
        if (auxtree) nfound += auxtree.allTags()->intersect(acc, low, high);
        if (righttree) nfound += righttree->allsub(acc, low, high);
      } else {
        if (auxtree) {
          std::vector<DFoldRectangleTree<D - 1>*> kds = auxtree.rightEndsRightOf(lbound);
          for (int i = 0; i < kds.size(); i++) { nfound += kds[i]->intersect(acc, low, high); }
        }
        if (righttree) nfound += righttree->intersect(acc, low, high, LeftOfFirstContainment);
      }
    } else if (_anctest == RightOfFirstContainment) {
      /* An ancestor of this node is in [lbound,ubound],
       * and the current splitpoint on the right branch relative to that node */
      if (splitpoint <= rbound) {
        if (righttree) nfound += righttree->intersect(acc, low, high, RightOfFirstContainment);
        if (auxtree) nfound += auxtree.allTags()->intersect(acc, low, high);
        if (lefttree) nfound += lefttree->allsub(acc, low, high);
      } else {
        if (auxtree) {
          std::vector<DFoldRectangleTree<D - 1>*> kds = auxtree.leftEndsLeftOf(rbound);
          for (int i = 0; i < kds.size(); i++) { nfound += kds[i]->intersect(acc, low, high); }
        }
        if (lefttree) nfound += lefttree->intersect(acc, low, high, RightOfFirstContainment);
      }
    } else {
      /* No ancestors contained in [lbound,ubound]. */
      if (lbound <= splitpoint && splitpoint <= rbound) {
        /* [lbound, splitpoint, rbound] */
        if (lefttree) nfound += lefttree->intersect(acc, low, high, LeftOfFirstContainment);
        if (auxtree) nfound += auxtree.allTags()->intersect(acc, low, high);
        if (righttree) nfound += righttree->intersect(acc, low, high, RightOfFirstContainment);
      } else if (splitpoint <= lbound) {
        /* splitpoint, [lbound,rbound] */
        if (auxtree) {
          std::vector<DFoldRectangleTree<D - 1>*> kds = auxtree.rightEndsRightOf(lbound);
          for (int i = 0; i < kds.size(); i++) { nfound += kds[i]->intersect(acc, low, high); }
        }
        if (righttree) nfound += righttree->intersect(acc, low, high, NoContainedAncestors);
      } else if (splitpoint >= rbound) {
        /* [lbound,rbound], splitpoint*/
        if (auxtree) {
          std::vector<DFoldRectangleTree<D - 1>*> kds = auxtree.leftEndsLeftOf(rbound);
          for (int i = 0; i < kds.size(); i++) { nfound += kds[i]->intersect(acc, low, high); }
        }
        if (lefttree) nfound += lefttree->intersect(acc, low, high, NoContainedAncestors);
      } else {
        error("branch failure");
      }
    }

    if (AC_DEBUG && nfound) {
      warning("yield %d %f -> %d %x", D, splitpoint, nfound, this);
      for (int i = acc.size() - nfound; i < acc.size(); i++) { warning("   yres %d", acc[i]); }
    }
    return nfound;
  }

  void dumpTree(int depth = 0) const {
    char* lsp = new char[2 * depth + 2];
    memset(lsp, ' ', 2 * depth + 2);
    lsp[2 * depth + 1] = '\0';
    warning("%s>%d %f %c %c %c", lsp, D, splitpoint, lefttree != NULL ? 'L' : '_',
            auxtree != NULL ? 'A' : '_', righttree != NULL ? 'R' : '_');
    if (lefttree) lefttree->dumpTree(depth + 1);
    if (auxtree) auxtree.dumpTree(depth + 1);
    if (righttree) righttree->dumpTree(depth + 1);
    delete[] lsp;
  }

  int rectcount() const {
    int s = 0;
    if (lefttree) s += lefttree->rectcount();
    if (righttree) s += righttree->rectcount();
    if (auxtree) s += auxtree.allTags()->rectcount();
    return s;
  }

  Real getSplitpoint() const { return splitpoint; }

  static size_t maxMemoryUse(size_t Nrects) {
    if (Nrects <= 1) {
      return sizeof(DFoldRectangleTree<D>) + OverlapTree<D>::maxAllocatedMemoryUse(1);
    }

    /* We assume total is bounded by the two extreme cases */
    size_t fully_split =
        Nrects * (DFoldRectangleTree<D>::maxMemoryUse(1) + sizeof(DFoldRectangleTree<D>));
    size_t fully_consolidated =
        sizeof(DFoldRectangleTree<D>) + OverlapTree<D>::maxAllocatedMemoryUse(Nrects);

    return std::max(fully_split, fully_consolidated);
  }

 private:
  Real splitpoint;
  OverlapTree<D> auxtree;
  DFoldRectangleTree<D>* lefttree;
  DFoldRectangleTree<D>* righttree;
};

template<>
class DFoldRectangleTree<1> {
 public:
  template<unsigned int K>
  DFoldRectangleTree<1>(const std::vector<struct Rectangle<K> >& rectangles, int* input, int N,
                        SingleCycleAllocator* alloc)
      : auxtree1d(), lefttree(NULL), righttree(NULL) {
    int nleft, noverlap, nright;
    splitpoint = splitRectangles<1, K>(rectangles, input, N, nleft, noverlap, nright);

    /* Construct left and right trees */
    if (nleft) {
      lefttree = alloc->allocateFor<DFoldRectangleTree<1> >();
      new (lefttree) DFoldRectangleTree<1>(rectangles, &input[0], nleft, alloc);
    }
    if (nright) {
      righttree = alloc->allocateFor<DFoldRectangleTree<1> >();
      new (righttree)
          DFoldRectangleTree<1>(rectangles, &input[nleft + noverlap], nright, alloc);
    }

    /* Project middle subtree down a dimension */
    if (noverlap) { auxtree1d.init(rectangles, &input[nleft], noverlap, alloc); }
  }
  ~DFoldRectangleTree<1>() {
    if (lefttree) delete lefttree;
    if (righttree) delete righttree;
  }

  /* Returns the number of intersections found, and adds indices corresponding
   * to the order of addition to the index accumulator. */
  int intersect(std::vector<void*>& acc, Real low[1], Real high[1],
                AncTestValues _anctest = NoContainedAncestors) const {
    /* Return the 'tag' list for the rectangles */
    int nfound = 0;
    Real lbound = low[D - 1];
    Real rbound = high[D - 1];
    if (AC_DEBUG) {
      warning("intersect %d %f [%f,%f] %d %x", D, splitpoint, lbound, rbound, _anctest, this);
    }

    /* Q: meta pass the ancestor info as `anctest` */
    if (_anctest == LeftOfFirstContainment) {
      /* An ancestor of this node is in [lbound,ubound],
       * and the current splitpoint on the left branch relative to that node */
      if (splitpoint >= lbound) {
        /* [lbound, splitpoint, alpha, rbound] */
        if (lefttree) nfound += lefttree->intersect(acc, low, high, LeftOfFirstContainment);
        if (auxtree1d) nfound += auxtree1d.allTags(acc);
        if (righttree) nfound += righttree->allTags(acc);
      } else {
        /* splitpoint, [lbound, alpha, rbound] */
        if (auxtree1d) nfound += auxtree1d.rightEndsRightOf(acc, lbound);
        if (righttree) nfound += righttree->intersect(acc, low, high, LeftOfFirstContainment);
      }
    } else if (_anctest == RightOfFirstContainment) {
      /* An ancestor of this node is in [lbound,ubound],
       * and the current splitpoint on the right branch relative to that node */
      if (splitpoint <= rbound) {
        /* [lbound, alpha, splitpoint, rbound] */
        if (righttree) nfound += righttree->intersect(acc, low, high, RightOfFirstContainment);
        if (auxtree1d) nfound += auxtree1d.allTags(acc);
        if (lefttree) nfound += lefttree->allTags(acc);
      } else {
        /* [lbound, alpha, rbound], splitpoint */
        if (auxtree1d) nfound += auxtree1d.leftEndsLeftOf(acc, rbound);
        if (lefttree) nfound += lefttree->intersect(acc, low, high, RightOfFirstContainment);
      }
    } else {
      /* No ancestors contained in [lbound,ubound]. */
      if (lbound <= splitpoint && splitpoint <= rbound) {
        /* [lbound, splitpoint, rbound] */
        if (auxtree1d) nfound += auxtree1d.allTags(acc);
        if (lefttree) nfound += lefttree->intersect(acc, low, high, LeftOfFirstContainment);
        if (righttree) nfound += righttree->intersect(acc, low, high, RightOfFirstContainment);
      } else if (splitpoint <= lbound) {
        /* splitpoint, [lbound,rbound] */
        if (auxtree1d) nfound += auxtree1d.rightEndsRightOf(acc, lbound);
        if (righttree) nfound += righttree->intersect(acc, low, high, NoContainedAncestors);
      } else if (rbound <= splitpoint) {
        /* [lbound,rbound], splitpoint*/
        if (auxtree1d) nfound += auxtree1d.leftEndsLeftOf(acc, rbound);
        if (lefttree) nfound += lefttree->intersect(acc, low, high, NoContainedAncestors);
      } else {
        error("branch failure");
      }
    }

    if (AC_DEBUG && nfound) {
      warning("yield %d %f -> %d %x", D, splitpoint, nfound, this);
      for (int i = acc.size() - nfound; i < acc.size(); i++) { warning("   yres %d", acc[i]); }
    }
    return nfound;
  }

  void dumpTree(int depth = 0) const {
    char* lsp = new char[2 * depth + 2];
    memset(lsp, ' ', 2 * depth + 2);
    lsp[2 * depth + 1] = '\0';
    warning("%s>%d %f %c %c %c", lsp, D, splitpoint, lefttree != NULL ? 'L' : '_',
            (bool)auxtree1d ? 'A' : '_', righttree != NULL ? 'R' : '_');
    if (lefttree) lefttree->dumpTree(depth + 1);
    if (auxtree1d) auxtree1d.dumpTree(depth + 1);
    if (righttree) righttree->dumpTree(depth + 1);
    delete[] lsp;
  }

  int allTags(std::vector<void*>& acc) const {
    int nfound = 0;
    if (lefttree) nfound += lefttree->allTags(acc);
    if (righttree) nfound += righttree->allTags(acc);
    if (auxtree1d) nfound += auxtree1d.allTags(acc);
    return nfound;
  }

  int rectcount() const {
    int s = 0;
    if (lefttree) s += lefttree->rectcount();
    if (righttree) s += righttree->rectcount();
    if (auxtree1d) s += auxtree1d.numTags();
    if (s < 1) error("Need at least one rectangle");
    return s;
  }

  Real getSplitpoint() const { return splitpoint; }

  static size_t maxMemoryUse(size_t Nrects) {
    if (Nrects <= 1) {
      return sizeof(DFoldRectangleTree<D>) + OverlapTree1D::maxAllocatedMemoryUse(1);
    }

    /* We assume total is bounded by the two extreme cases */
    size_t fully_split =
        Nrects * (DFoldRectangleTree<D>::maxMemoryUse(1) + sizeof(DFoldRectangleTree<D>));
    size_t fully_consolidated =
        sizeof(DFoldRectangleTree<D>) + OverlapTree1D::maxAllocatedMemoryUse(Nrects);

    return std::max(fully_split, fully_consolidated);
  }

 private:
  static const int D = 1;

  Real splitpoint;
  OverlapTree1D auxtree1d;
  DFoldRectangleTree<1>* lefttree;
  DFoldRectangleTree<1>* righttree;
};

template<unsigned int D>
template<unsigned int K>
void OverlapTree<D>::init(const std::vector<struct Rectangle<K> >& rectangles, int* input,
                          int iN, SingleCycleAllocator* alloc) {
  N = iN;
  lendpoints = (PtAscending*)alloc->allocate(sizeof(PtAscending) * N);
  rendpoints = (PtDescending*)alloc->allocate(sizeof(PtDescending) * N);

  for (int i = 0; i < N; i++) {
    PtAscending lpt;
    lpt.tag = (void*)(ptrdiff_t)input[i];
    lpt.r = rectangles[input[i]].lower[D - 1];
    lendpoints[i] = lpt;
    PtDescending rpt;
    rpt.tag = (void*)(ptrdiff_t)input[i];
    rpt.r = rectangles[input[i]].upper[D - 1];
    rendpoints[i] = rpt;
  }
  std::sort(lendpoints, lendpoints + N);
  std::sort(rendpoints, rendpoints + N);

  if (N > 1) {
    int k = 1;
    while (k < N) { k *= 2; }

    ltrees =
        (DFoldRectangleTree<D - 1>**)alloc->allocate(sizeof(DFoldRectangleTree<D - 1>*) * k);
    rtrees =
        (DFoldRectangleTree<D - 1>**)alloc->allocate(sizeof(DFoldRectangleTree<D - 1>*) * k);
    for (int i = 0; i < k; i++) {
      ltrees[i] = NULL;
      rtrees[i] = NULL;
    }

    int* iscratch = new int[k / 2];

    /* Construct all the trees */
    int j = 1;
    while (j < N) {
      int S = k / (2 * j);

      for (int p = 0; S * (2 * p + 1) < N; p++) {
        /* Create left subtrees of size S */
        for (int i = 0; i < S; i++) { iscratch[i] = (ptrdiff_t)lendpoints[S * 2 * p + i].tag; }
        ltrees[j - 1 + p] = alloc->allocateFor<DFoldRectangleTree<D - 1> >();
        new (ltrees[j - 1 + p]) DFoldRectangleTree<D - 1>(rectangles, iscratch, S, alloc);

        /* Create right subtrees of size S */
        for (int i = 0; i < S; i++) { iscratch[i] = (ptrdiff_t)rendpoints[S * 2 * p + i].tag; }
        rtrees[j - 1 + p] = alloc->allocateFor<DFoldRectangleTree<D - 1> >();
        new (rtrees[j - 1 + p]) DFoldRectangleTree<D - 1>(rectangles, iscratch, S, alloc);
      }

      j *= 2;
    }

    delete[] iscratch;
  } else {
    ltrees = NULL;
    rtrees = NULL;
  }
  atree = alloc->allocateFor<DFoldRectangleTree<D - 1> >();
  new (atree) DFoldRectangleTree<D - 1>(rectangles, input, N, alloc);
}

template<unsigned int D>
std::vector<DFoldRectangleTree<D - 1>*> OverlapTree<D>::leftEndsLeftOf(Real rbound) const {
  /* lendpoints in ascending order */
  std::vector<DFoldRectangleTree<D - 1>*> ret;
  /* fast cases, all and nothing */
  if (lendpoints[N - 1].r <= rbound) {
    if (AC_DEBUG) {
      warning("triv lelo %f <= %f | %d %d", lendpoints[N - 1].r, rbound, D,
              atree->rectcount());
    }
    ret.push_back(atree);
    return ret;
  }
  if (rbound < lendpoints[0].r) {
    if (AC_DEBUG) { warning("triv lelo %f < %f | %d", rbound, lendpoints[0].r, D); }
    return ret;
  }

  /* simulateneously binary search and build tree list  */
  int k = 1;
  while (2 * k < N) { k *= 2; }

  int j = 0;
  int v = 1;
  while (k > 0) {
    if (j + k - 1 >= N) {
      v = v * 2;
    } else if (lendpoints[j + k - 1].r <= rbound) {
      if (!ltrees[v - 1]) error("Null tree");
      ret.push_back(ltrees[v - 1]);
      j = j + k;
      v = v * 2 + 1;
    } else {
      v = v * 2;
    }
    k = k / 2;
  }
  if (AC_DEBUG) { warning("nontriv lelo %f D=%d sz=%d", rbound, D, ret.size()); }
  return ret;
}
template<unsigned int D>
std::vector<DFoldRectangleTree<D - 1>*> OverlapTree<D>::rightEndsRightOf(Real lbound) const {
  /* rendpoints in descending order */
  std::vector<DFoldRectangleTree<D - 1>*> ret;
  /* fast cases, all and nothing */
  if (rendpoints[N - 1].r >= lbound) {
    if (AC_DEBUG) {
      warning("triv rero %f >= %f | %d %f %d", rendpoints[N - 1].r, lbound, D,
              atree->getSplitpoint(), atree->rectcount());
    }
    ret.push_back(atree);
    return ret;
  }
  if (lbound > rendpoints[0].r) {
    if (AC_DEBUG) { warning("triv rero %f > %f | %d", lbound, rendpoints[0].r, D); }
    return ret;
  }

  /* simulateneously binary search and build tree list  */
  int k = 1;
  while (2 * k < N) { k *= 2; }

  int j = 0;
  int v = 1;
  while (k > 0) {
    if (j + k - 1 >= N) {
      v = v * 2;
    } else if (rendpoints[j + k - 1].r >= lbound) {
      if (!rtrees[v - 1]) error("Null tree");
      ret.push_back(rtrees[v - 1]);
      j = j + k;
      v = v * 2 + 1;
    } else {
      v = v * 2;
    }
    k = k / 2;
  }
  if (AC_DEBUG) { warning("nontriv rero %f D=%d sz=%d", lbound, D, ret.size()); }
  return ret;
}

AnimatedCollection::AnimatedCollection() {
  map = NULL;
  alloc = NULL;
}

AnimatedCollection::~AnimatedCollection() {
  if (alloc) delete (SingleCycleAllocator*)alloc;
}

void AnimatedCollection::insert(Animated* a) { store.insert(a); }

void AnimatedCollection::remove(Animated* a) { store.erase(a); }

static std::vector<struct Rectangle<3> > staticinput;

static struct Rectangle<3> rectFromAnim(const Animated* a) {
  struct Rectangle<3> r;
  r.tag = (void*)a;
  /* Reverse dimension order to avoid Z-clustering */
  r.lower[2] = a->position[0] + a->boundingBox[0][0];
  r.upper[2] = a->position[0] + a->boundingBox[1][0];
  r.lower[1] = a->position[1] + a->boundingBox[0][1];
  r.upper[1] = a->position[1] + a->boundingBox[1][1];
  r.lower[0] = a->position[2] + a->boundingBox[0][2];
  r.upper[0] = a->position[2] + a->boundingBox[1][2];

  for (int i = 0; i < 3; i++) {
    if (r.lower[i] > r.upper[i]) { error("Impossible rectangle"); }
  }
  return r;
}

void AnimatedCollection::recalculateBboxMap() {
  if (alloc) {
    delete (SingleCycleAllocator*)alloc;
    alloc = NULL;
    map = NULL;
  }

  std::vector<struct Rectangle<3> > input;
  std::vector<int> rect_indices;
  std::set<Animated*>::iterator iter = store.begin();
  std::set<Animated*>::iterator end = store.end();
  for (; iter != end; iter++) {
    struct Rectangle<3> r = rectFromAnim(*iter);
    input.push_back(r);
    rect_indices.push_back(rect_indices.size());
  }
  staticinput = input;

  /* For < 200 entities, maxmem < 5MB, and in practice ~200Kb is used */
  size_t maxmem = DFoldRectangleTree<3>::maxMemoryUse(store.size());
  alloc = new SingleCycleAllocator(maxmem);
  map = ((SingleCycleAllocator*)alloc)->allocate(sizeof(DFoldRectangleTree<3>));
  new (map) DFoldRectangleTree<3>(input, rect_indices.data(), rect_indices.size(),
                                  (SingleCycleAllocator*)alloc);
  size_t usage = ((SingleCycleAllocator*)alloc)->used();
  (void)usage;
}

std::set<Animated*> AnimatedCollection::bboxOverlapsWith(const Animated* a) const {
  if (!map) {
    warning("Map not yet created");
    return std::set<Animated*>();
  }
  DFoldRectangleTree<3>* dmap = (DFoldRectangleTree<3>*)map;
  struct Rectangle<3> r = rectFromAnim(a);

  std::vector<void*> uco;
  dmap->intersect(uco, r.lower, r.upper);
  std::set<Animated*> ret;
  for (int i = 0; i < uco.size(); i++) { ret.insert((Animated*)uco[i]); }
  if (0) {
    /* Sanity check via O(n^2) algorithm */

    /* As the Animateds move during the step, we use their initial positions */
    std::set<Animated*> coll;
    int nco = 0;
    for (int l = 0; l < staticinput.size(); l++) {
      struct Rectangle<3> s = staticinput[l];

      int nover = 0;
      for (int i = 0; i < 3; i++) {
        if (r.lower[i] <= s.upper[i] && r.upper[i] >= s.lower[i]) { nover++; }
      }
      if (nover == 3) {
        coll.insert((Animated*)s.tag);
        if (ret.count((Animated*)s.tag)) { nco++; }
      }
    }

    static int psu = 0;
    if (coll.size() != nco || coll.size() != ret.size() | ret.size() != uco.size()) {
      warning("Interval tree error %d -> %d (ideal %d (%d present)), %d prior passed",
              uco.size(), ret.size(), coll.size(), nco, psu);
      psu = 0;
    } else {
      psu++;
    }
    return coll;
  }

  return ret;
}

const std::set<Animated*>& AnimatedCollection::asSet() const { return store; }
