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

/* For std::sort, std::nth_element */
#include <algorithm>
#include <set>

#include <cstring>

static int AC_DEBUG = 0;
static int SANITY_CHECK = 0;

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
  SingleCycleAllocator(size_t maxbytes, size_t istacklen) {
    size_t nints = istacklen / sizeof(int) + 1;
    data = new char[maxbytes];
    istack = new int[nints];
    index = 0;
    stackdex = 0;
    maxlen = maxbytes;
    stacklen = istacklen;
  }
  ~SingleCycleAllocator() {
    delete[] data;
    delete[] istack;
  }
  void reset() {
    index = 0;
    stackdex = 0;
  }
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
  size_t used() const { return index; }

  void* stackPush(size_t bytes) {
    size_t nints = bytes / sizeof(int) + 1;
    void* r = (void*)&istack[stackdex];
    stackdex += nints + 1;
    if (index >= maxlen) {
      error("Tried to acquire %d more stack bytes (limit %d)", nints, stackdex);
    }
    istack[stackdex - 1] = nints;
    return r;
  }
  void stackPop() { stackdex -= istack[stackdex - 1] + 1; }
  size_t maxStackSize() const { return stacklen; }
  size_t maxSize() const { return maxlen; }

 private:
  char* data;
  int* istack;
  size_t index;
  size_t maxlen;
  size_t stackdex;
  size_t stacklen;
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
  void init(const struct Rectangle<K>* rectangles, int* input, int iN,
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

  int allTags(void** acc) const {
    for (int i = 0; i < N; i++) { acc[i] = lendpoints[i].tag; }
    return N;
  }
  int numTags() const { return N; }
  int leftEndsLeftOf(void** acc, Real ubound) const {
    int nfound = 0;
    for (int i = 0; i < N; i++) {
      if (lendpoints[i].r <= ubound) {
        acc[nfound] = lendpoints[i].tag;
        nfound++;
      } else {
        break;
      }
    }
    return nfound;
  }
  int rightEndsRightOf(void** acc, Real lbound) const {
    int nfound = 0;
    for (int i = 0; i < N; i++) {
      if (rendpoints[i].r >= lbound) {
        acc[nfound] = rendpoints[i].tag;
        nfound++;
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
  void init(const struct Rectangle<K>* rectangles, int* input, int N,
            SingleCycleAllocator* alloc);
  ~OverlapTree();
  operator bool() const { return N > 0; }

  DFoldRectangleTree<D - 1>* allTags() const { return atree; }
  int leftEndsLeftOf(Real rbound, void** acc, Real low[D], Real high[D]) const;
  int rightEndsRightOf(Real lbound, void** acc, Real low[D], Real high[D]) const;

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

template<unsigned int D, unsigned int K>
Real computeSplitpoint(const struct Rectangle<K>* rectangles, int* input, int N,
                       SingleCycleAllocator* alloc) {
  /* sort the 2d points and pick the center. */
  Real* dco = (Real*)alloc->stackPush(sizeof(Real) * 2 * N);
  for (int i = 0; i < N; i++) {
    dco[2 * i] = rectangles[input[i]].lower[D - 1];
    dco[2 * i + 1] = rectangles[input[i]].upper[D - 1];
  }
  /* Typically N<=4, in which case this is slower than sorting, but
   * this is much faster for the few large N calls */
  std::nth_element(dco, dco + N, dco + 2 * N);
  Real upper = dco[N];
  Real lower = dco[0];
  for (int i = 1; i < N; i++) { lower = std::max(dco[i], lower); }
  Real splitpoint = 0.5 * (lower + upper);
  alloc->stackPop();
  return splitpoint;
}

template<unsigned int D, unsigned int K>
Real splitRectangles(const struct Rectangle<K>* rectangles, int* input, int N, int& nleft,
                     int& noverlap, int& nright, SingleCycleAllocator* alloc) {
  if (!N) { error("Must have rectangles to split"); }
  nleft = 0;
  nright = 0;
  noverlap = 0;
  if (N == 1) {
    noverlap = 1;
    return 0.5 * (rectangles[input[0]].lower[D - 1] + rectangles[input[0]].upper[D - 1]);
  }

  Real splitpoint = computeSplitpoint<D, K>(rectangles, input, N, alloc);

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
  DFoldRectangleTree(const struct Rectangle<K>* rectangles, int* input, int N,
                     SingleCycleAllocator* alloc)
      : auxtree(), lefttree(NULL), righttree(NULL) {
    int nleft, noverlap, nright;
    splitpoint = splitRectangles<D, K>(rectangles, input, N, nleft, noverlap, nright, alloc);

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
  int allsub(void** acc, Real low[D], Real high[D]) const {
    int nfound = 0;
    if (auxtree) { nfound += auxtree.allTags()->intersect(&acc[nfound], low, high); }
    if (lefttree) { nfound += lefttree->allsub(&acc[nfound], low, high); }
    if (righttree) { nfound += righttree->allsub(&acc[nfound], low, high); }
    return nfound;
  }

  /* Returns the number of intersections found, and adds indices corresponding
   * to the order of addition to the index accumulator. */
  int intersect(void** acc, Real low[D], Real high[D],
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
        if (lefttree)
          nfound += lefttree->intersect(&acc[nfound], low, high, LeftOfFirstContainment);
        if (auxtree) nfound += auxtree.allTags()->intersect(&acc[nfound], low, high);
        if (righttree) nfound += righttree->allsub(&acc[nfound], low, high);
      } else {
        if (auxtree) nfound += auxtree.rightEndsRightOf(lbound, &acc[nfound], low, high);
        if (righttree)
          nfound += righttree->intersect(&acc[nfound], low, high, LeftOfFirstContainment);
      }
    } else if (_anctest == RightOfFirstContainment) {
      /* An ancestor of this node is in [lbound,ubound],
       * and the current splitpoint on the right branch relative to that node */
      if (splitpoint <= rbound) {
        if (righttree)
          nfound += righttree->intersect(&acc[nfound], low, high, RightOfFirstContainment);
        if (auxtree) nfound += auxtree.allTags()->intersect(&acc[nfound], low, high);
        if (lefttree) nfound += lefttree->allsub(&acc[nfound], low, high);
      } else {
        if (auxtree) nfound += auxtree.leftEndsLeftOf(rbound, &acc[nfound], low, high);
        if (lefttree)
          nfound += lefttree->intersect(&acc[nfound], low, high, RightOfFirstContainment);
      }
    } else {
      /* No ancestors contained in [lbound,ubound]. */
      if (lbound <= splitpoint && splitpoint <= rbound) {
        /* [lbound, splitpoint, rbound] */
        if (lefttree)
          nfound += lefttree->intersect(&acc[nfound], low, high, LeftOfFirstContainment);
        if (auxtree) nfound += auxtree.allTags()->intersect(&acc[nfound], low, high);
        if (righttree)
          nfound += righttree->intersect(&acc[nfound], low, high, RightOfFirstContainment);
      } else if (splitpoint <= lbound) {
        /* splitpoint, [lbound,rbound] */
        if (auxtree) nfound += auxtree.rightEndsRightOf(lbound, &acc[nfound], low, high);
        if (righttree)
          nfound += righttree->intersect(&acc[nfound], low, high, NoContainedAncestors);
      } else if (splitpoint >= rbound) {
        /* [lbound,rbound], splitpoint*/
        if (auxtree) nfound += auxtree.leftEndsLeftOf(rbound, &acc[nfound], low, high);
        if (lefttree)
          nfound += lefttree->intersect(&acc[nfound], low, high, NoContainedAncestors);
      } else {
        error("branch failure");
      }
    }

    if (AC_DEBUG && nfound) {
      warning("yield %d %f -> %d %x", D, splitpoint, nfound, this);
      for (int i = 0; i < nfound; i++) { warning("   yres %d", acc[i]); }
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
  DFoldRectangleTree<1>(const struct Rectangle<K>* rectangles, int* input, int N,
                        SingleCycleAllocator* alloc)
      : auxtree1d(), lefttree(NULL), righttree(NULL) {
    int nleft, noverlap, nright;
    splitpoint = splitRectangles<1, K>(rectangles, input, N, nleft, noverlap, nright, alloc);

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
  int intersect(void** acc, Real low[1], Real high[1],
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
        if (lefttree)
          nfound += lefttree->intersect(&acc[nfound], low, high, LeftOfFirstContainment);
        if (auxtree1d) nfound += auxtree1d.allTags(&acc[nfound]);
        if (righttree) nfound += righttree->allTags(&acc[nfound]);
      } else {
        /* splitpoint, [lbound, alpha, rbound] */
        if (auxtree1d) nfound += auxtree1d.rightEndsRightOf(&acc[nfound], lbound);
        if (righttree)
          nfound += righttree->intersect(&acc[nfound], low, high, LeftOfFirstContainment);
      }
    } else if (_anctest == RightOfFirstContainment) {
      /* An ancestor of this node is in [lbound,ubound],
       * and the current splitpoint on the right branch relative to that node */
      if (splitpoint <= rbound) {
        /* [lbound, alpha, splitpoint, rbound] */
        if (righttree)
          nfound += righttree->intersect(&acc[nfound], low, high, RightOfFirstContainment);
        if (auxtree1d) nfound += auxtree1d.allTags(&acc[nfound]);
        if (lefttree) nfound += lefttree->allTags(&acc[nfound]);
      } else {
        /* [lbound, alpha, rbound], splitpoint */
        if (auxtree1d) nfound += auxtree1d.leftEndsLeftOf(&acc[nfound], rbound);
        if (lefttree)
          nfound += lefttree->intersect(&acc[nfound], low, high, RightOfFirstContainment);
      }
    } else {
      /* No ancestors contained in [lbound,ubound]. */
      if (lbound <= splitpoint && splitpoint <= rbound) {
        /* [lbound, splitpoint, rbound] */
        if (auxtree1d) nfound += auxtree1d.allTags(&acc[nfound]);
        if (lefttree)
          nfound += lefttree->intersect(&acc[nfound], low, high, LeftOfFirstContainment);
        if (righttree)
          nfound += righttree->intersect(&acc[nfound], low, high, RightOfFirstContainment);
      } else if (splitpoint <= lbound) {
        /* splitpoint, [lbound,rbound] */
        if (auxtree1d) nfound += auxtree1d.rightEndsRightOf(&acc[nfound], lbound);
        if (righttree)
          nfound += righttree->intersect(&acc[nfound], low, high, NoContainedAncestors);
      } else if (rbound <= splitpoint) {
        /* [lbound,rbound], splitpoint*/
        if (auxtree1d) nfound += auxtree1d.leftEndsLeftOf(&acc[nfound], rbound);
        if (lefttree)
          nfound += lefttree->intersect(&acc[nfound], low, high, NoContainedAncestors);
      } else {
        error("branch failure");
      }
    }

    if (AC_DEBUG && nfound) {
      warning("yield %d %f -> %d %x", D, splitpoint, nfound, this);
      for (int i = 0; i < nfound; i++) { warning("   yres %d", acc[i]); }
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

  int allTags(void** acc) const {
    int nfound = 0;
    if (lefttree) nfound += lefttree->allTags(&acc[nfound]);
    if (righttree) nfound += righttree->allTags(&acc[nfound]);
    if (auxtree1d) nfound += auxtree1d.allTags(&acc[nfound]);
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
void OverlapTree<D>::init(const struct Rectangle<K>* rectangles, int* input, int iN,
                          SingleCycleAllocator* alloc) {
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

    /* Construct all the trees */
    int j = 1;
    while (j < N) {
      int S = k / (2 * j);

      int* iscratch = (int*)alloc->stackPush(sizeof(int) * S);
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
      alloc->stackPop();

      j *= 2;
    }

  } else {
    ltrees = NULL;
    rtrees = NULL;
  }
  atree = alloc->allocateFor<DFoldRectangleTree<D - 1> >();
  new (atree) DFoldRectangleTree<D - 1>(rectangles, input, N, alloc);
}

template<unsigned int D>
int OverlapTree<D>::leftEndsLeftOf(Real rbound, void** acc, Real low[D], Real high[D]) const {
  /* lendpoints in ascending order */
  /* fast cases, all and nothing */
  if (lendpoints[N - 1].r <= rbound) {
    if (AC_DEBUG) {
      warning("triv lelo %f <= %f | %d %d", lendpoints[N - 1].r, rbound, D,
              atree->rectcount());
    }
    return atree->intersect(&acc[0], low, high);
  }
  if (rbound < lendpoints[0].r) {
    if (AC_DEBUG) { warning("triv lelo %f < %f | %d", rbound, lendpoints[0].r, D); }
    return 0;
  }

  /* simulateneously binary search and build tree list  */
  int k = 1;
  while (2 * k < N) { k *= 2; }

  int j = 0;
  int v = 1;
  int nfound = 0;
  while (k > 0) {
    if (j + k - 1 >= N) {
      v = v * 2;
    } else if (lendpoints[j + k - 1].r <= rbound) {
      if (!ltrees[v - 1]) error("Null tree");
      nfound += ltrees[v - 1]->intersect(&acc[nfound], low, high);
      j = j + k;
      v = v * 2 + 1;
    } else {
      v = v * 2;
    }
    k = k / 2;
  }
  if (AC_DEBUG) { warning("nontriv lelo %f D=%d nfo=%d", rbound, D, nfound); }
  return nfound;
}
template<unsigned int D>
int OverlapTree<D>::rightEndsRightOf(Real lbound, void** acc, Real low[D],
                                     Real high[D]) const {
  /* rendpoints in descending order */
  /* fast cases, all and nothing */
  if (rendpoints[N - 1].r >= lbound) {
    if (AC_DEBUG) {
      warning("triv rero %f >= %f | %d %f %d", rendpoints[N - 1].r, lbound, D,
              atree->getSplitpoint(), atree->rectcount());
    }
    return atree->intersect(&acc[0], low, high);
  }
  if (lbound > rendpoints[0].r) {
    if (AC_DEBUG) { warning("triv rero %f > %f | %d", lbound, rendpoints[0].r, D); }
    return 0;
  }

  /* simulateneously binary search and build tree list  */
  int k = 1;
  while (2 * k < N) { k *= 2; }

  int j = 0;
  int v = 1;
  int nfound = 0;
  while (k > 0) {
    if (j + k - 1 >= N) {
      v = v * 2;
    } else if (rendpoints[j + k - 1].r >= lbound) {
      if (!rtrees[v - 1]) error("Null tree");
      nfound += rtrees[v - 1]->intersect(&acc[nfound], low, high);
      j = j + k;
      v = v * 2 + 1;
    } else {
      v = v * 2;
    }
    k = k / 2;
  }
  if (AC_DEBUG) { warning("nontriv rero %f D=%d nfo=%d", lbound, D, nfound); }
  return nfound;
}

static struct Rectangle<3> rectFromAnim(const Animated* a) {
  struct Rectangle<3> r;
  r.tag = (void*)a;
  for (int k = 0; k < 3; k++) {
    r.lower[k] = a->position[k] + a->boundingBox[0][k];
    r.upper[k] = a->position[k] + a->boundingBox[1][k];
  }
  for (int i = 0; i < 3; i++) {
    if (r.lower[i] > r.upper[i] || !std::isfinite(r.lower[i]) || !std::isfinite(r.upper[i])) {
      warning("Invalid bounding box");
      for (int k = 0; k < 3; k++) {
        r.lower[k] = 0.;
        r.upper[k] = 0.;
      }
      return r;
    }
  }
  return r;
}

static struct Rectangle<3>
rectFromBounds(const double lower[3], const double upper[3]) {
  struct Rectangle<3> r;
  r.tag = NULL;
  for (int i = 0; i < 3; i++) {
    r.lower[i] = lower[i];
    r.upper[i] = upper[i];
  }
  for (int i = 0; i < 3; i++) {
    if (r.lower[i] > r.upper[i] || !std::isfinite(r.lower[i]) || !std::isfinite(r.upper[i])) {
      warning("Invalid bounding box");
      for (int k = 0; k < 3; k++) {
        r.lower[k] = 0.;
        r.upper[k] = 0.;
      }
      return r;
    }
  }
  return r;
}

static struct Rectangle<3>
reverseAxisOrder(const struct Rectangle<3>& s) {
  struct Rectangle<3> r;
  r.tag = s.tag;
  for (int i = 0; i < 3; i++) {
    r.lower[i] = s.lower[2 - i];
    r.upper[i] = s.upper[2 - i];
  }
  return r;
}

AnimatedCollection::AnimatedCollection()
    : map(NULL),
      alloc(NULL),
      retlist(NULL),
      ntot(0),
      nreserved(0),
      rect_indices(NULL),
      input(NULL) {}

AnimatedCollection::~AnimatedCollection() {
  if (alloc) delete (SingleCycleAllocator*)alloc;
  if (retlist) delete[] retlist;
  if (rect_indices) delete[] rect_indices;
  if (input) delete[] (struct Rectangle<3>*)input;
}

void AnimatedCollection::add(Animated* a) {
  struct Rectangle<3> r = rectFromAnim(a);
  r = reverseAxisOrder(r);
  if (ntot >= nreserved) { reserve(std::max(2 * ntot, 16)); }
  ((struct Rectangle<3>*)input)[ntot] = r;
  ntot++;
}

void AnimatedCollection::reserve(int N) {
  if (N <= nreserved) return;
  nreserved = N;

  struct Rectangle<3>* ninput = new struct Rectangle<3>[nreserved];
  int* nrect_indices = new int[nreserved];
  for (int i = 0; i < nreserved; i++) { nrect_indices[i] = i; }
  for (int i = 0; i < ntot; i++) { ninput[i] = ((struct Rectangle<3>*)input)[i]; }

  if (input) delete[] (struct Rectangle<3>*)input;
  if (rect_indices) delete[] rect_indices;
  if (retlist) delete[] retlist;
  input = (void*)ninput;
  rect_indices = nrect_indices;
  retlist = new void*[nreserved];
}

void AnimatedCollection::clear() {
  ntot = 0;
  nreserved = 0;
  if (input) delete[] (struct Rectangle<3>*)input;
  if (rect_indices) delete[] rect_indices;
  if (retlist) delete[] retlist;
  retlist = NULL;
  input = NULL;
  rect_indices = NULL;
}

void AnimatedCollection::recalculateBboxMap() {
  if (ntot == 0) {
    map = NULL;
    return;
  }

  /* For < 200 entities, maxmem < 5MB, and in practice ~200Kb is used */
  size_t maxmem = DFoldRectangleTree<3>::maxMemoryUse(ntot);
  size_t nstack = (ntot + 1) * (sizeof(int) * 3 + sizeof(Real) * 2);

  /* Reset memory */
  if (alloc) {
    size_t am = ((SingleCycleAllocator*)alloc)->maxSize();
    size_t ns = ((SingleCycleAllocator*)alloc)->maxStackSize();
    if (maxmem > am || am > 2 * maxmem || nstack > ns || ns > 2 * nstack) {
      delete (SingleCycleAllocator*)alloc;
      alloc = new SingleCycleAllocator(3 * maxmem / 2, 3 * nstack / 2);
    } else {
      ((SingleCycleAllocator*)alloc)->reset();
    }
  } else {
    alloc = new SingleCycleAllocator(3 * maxmem / 2, 3 * nstack / 2);
  }

  /* Build map */
  map = ((SingleCycleAllocator*)alloc)->allocate(sizeof(DFoldRectangleTree<3>));
  new (map) DFoldRectangleTree<3>((struct Rectangle<3>*)input, rect_indices, ntot,
                                  (SingleCycleAllocator*)alloc);
  size_t usage = ((SingleCycleAllocator*)alloc)->used();
  (void)usage;
}

size_t AnimatedCollection::bboxOverlapsWith(const Animated* a, Animated*** ret) const {
  struct Rectangle<3> r = rectFromAnim(a);
  return bboxOverlapsWith(r.lower, r.upper, ret);
}
size_t AnimatedCollection::bboxOverlapsWith(const double lower[3], const double upper[3],
                                            Animated*** ret) const {
  if (!map) {
    /* Either map has not been created or there are no rectangles */
    return 0;
  }
  DFoldRectangleTree<3>* dmap = (DFoldRectangleTree<3>*)map;
  struct Rectangle<3> r = rectFromBounds(lower, upper);
  r = reverseAxisOrder(r);

  int nfound = dmap->intersect(retlist, r.lower, r.upper);

  if (SANITY_CHECK) {
    /* Sanity check via O(n^2) algorithm */

    /* As the Animateds move during the step, we use their initial positions */
    std::set<Animated*> coll;
    int nco = 0;
    for (int l = 0; l < ntot; l++) {
      struct Rectangle<3> s = ((struct Rectangle<3>*)input)[l];

      int nover = 0;
      for (int i = 0; i < 3; i++) {
        if (r.lower[i] <= s.upper[i] && r.upper[i] >= s.lower[i]) { nover++; }
      }
      if (nover == 3) {
        coll.insert((Animated*)s.tag);
        for (int k = 0; k < nfound; k++) {
          if (retlist[k] == (Animated*)s.tag) {
            nco++;
            break;
          }
        }
      }
    }

    static int psu = 0;
    if (coll.size() != nco || coll.size() != nfound) {
      warning("Interval tree %d error %d (ideal %d (%d present)), %d prior passed", ntot,
              nfound, coll.size(), nco, psu);
      psu = 0;
    } else {
      psu++;
    }
  }

  *ret = (Animated**)retlist;
  return nfound;
}
