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

static int AC_DEBUG = 0;

template<unsigned int D>
struct Rectangle {
  Real lower[D];
  Real upper[D];
  void* tag;
};

int realcmp(const void* l, const void* r) {
  Real rl = *(Real*)(l);
  Real rr = *(Real*)(r);
  return (rl >= rr ? 1 : 0) - (rl <= rr ? 1 : 0);
}

/* The auxiliary tree for D=1 */
class OverlapTree1D {
 public:
  typedef struct {
    /* Real goes first so realcmp still works */
    Real r;
    void* tag;
  } Pt;
  OverlapTree1D(const std::vector<struct Rectangle<1> >& input) {
    /* Since this is never modified, we use a sorted list rather than a tree */
    N = input.size();
    lendpoints = new Pt[N];
    rendpoints = new Pt[N];
    tags = new void*[N];
    for (int i = 0; i < N; i++) {
      Pt lpt;
      lpt.tag = input[i].tag;
      lpt.r = input[i].lower[D - 1];
      lendpoints[i] = lpt;
      Pt rpt;
      rpt.tag = input[i].tag;
      rpt.r = input[i].upper[D - 1];
      rendpoints[i] = rpt;
      tags[i] = input[i].tag;
    }
    qsort(lendpoints, N, sizeof(Pt), realcmp);
    qsort(rendpoints, N, sizeof(Pt), realcmp);
  }
  ~OverlapTree1D() {
    delete[] lendpoints;
    delete[] rendpoints;
    delete[] tags;
  }

  int allTags(std::vector<void*>& acc) const {
    for (int i = 0; i < N; i++) { acc.push_back(tags[i]); }
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
    for (int i = N - 1; i >= 0; i--) {
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

 private:
  static const int D = 1;

  int N;
  Pt* lendpoints;
  Pt* rendpoints;
  void** tags;
};

template<unsigned int D>
class DFoldRectangleTree;

template<unsigned int D>
class OverlapTree {
 public:
  /* Constructor and destructor are delayed */
  OverlapTree(const std::vector<struct Rectangle<D> >& input);
  ~OverlapTree();

  DFoldRectangleTree<D - 1>* allTags() const { return atree; }
  std::vector<DFoldRectangleTree<D - 1>*> leftEndsLeftOf(Real rbound) const;
  std::vector<DFoldRectangleTree<D - 1>*> rightEndsRightOf(Real lbound) const;

  void dumpTree(int depth = 0) const {
    char* lsp = new char[2 * depth + 2];
    memset(lsp, ' ', 2 * depth + 2);
    lsp[2 * depth + 1] = '\0';
    warning("%s>A %d", lsp, N);
    for (int i = 0; i < N; i++) {
      warning("%s - | %f | %f |", lsp, lendpoints[i], rendpoints[i]);
    }
    atree->dumpTree(depth + 1);
  }

 private:
  int N;
  Real* lendpoints;
  Real* rendpoints;
  DFoldRectangleTree<D - 1>** ltrees;
  DFoldRectangleTree<D - 1>** rtrees;
  DFoldRectangleTree<D - 1>* atree;
};

typedef struct {
  Real r;
  int idx;
} AdjPt;

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

template<unsigned int D>
Real splitRectangles(const std::vector<struct Rectangle<D> >& input,
                     std::vector<struct Rectangle<D> >& left,
                     std::vector<struct Rectangle<D> >& right,
                     std::vector<struct Rectangle<D> >& overlapping) {
  int N = input.size();
  if (!N) { error("Must have rectangles to split"); }
  if (N == 1) {
    overlapping.push_back(input[0]);
    return 0.5 * (input[0].lower[D - 1] + input[0].upper[D - 1]);
  }
  /* sort the 2d points and pick the center */
  std::vector<Real> dco;
  for (int i = 0; i < N; i++) {
    dco.push_back(input[i].lower[D - 1]);
    dco.push_back(input[i].upper[D - 1]);
  }
  qsort(dco.data(), dco.size(), sizeof(Real), realcmp);

  Real splitpoint = 0.5 * (dco[N - 1] + dco[N]);

  /* Divide rectangles into three lists. Note a point
   * == splitpoint is left if splitpoint iff (i < N/2) */
  for (int i = 0; i < N; i++) {
    Real lbound = input[i].lower[D - 1];
    Real ubound = input[i].upper[D - 1];
    if (ubound < splitpoint || (i > N / 2 && ubound == splitpoint)) {
      /* Rectangle entirely below the split point */
      left.push_back(input[i]);
    } else if (splitpoint < lbound || (i <= N / 2 && lbound == splitpoint)) {
      /* Rectangle entirely above the split point */
      right.push_back(input[i]);
    } else {
      /* Rectangle overlaps the split point */
      overlapping.push_back(input[i]);
    }
  }

  if (left.size() == input.size()) {
    error("Left size fail: %d %d %d (%d)", left.size(), overlapping.size(), right.size(),
          input.size());
  }
  if (right.size() == input.size()) {
    error("Right size fail: %d %d %d (%d)", left.size(), overlapping.size(), right.size(),
          input.size());
  }
  return splitpoint;
}

template<unsigned int D>
class DFoldRectangleTree {
 public:
  /* Note: the dfold tree shall stay out of headers, and instead
   * a wrapper class will handle the Animated tracking */

  template<unsigned int K>
  DFoldRectangleTree(const std::vector<struct Rectangle<K> >& input)
      : auxtree(NULL), lefttree(NULL), righttree(NULL) {
    std::vector<struct Rectangle<D> > left;
    std::vector<struct Rectangle<D> > right;
    std::vector<struct Rectangle<D> > overlapping;
    splitpoint = splitRectangles<D>(input, left, right, overlapping);

    /* Construct left and right trees */
    if (left.size()) { lefttree = new DFoldRectangleTree<D>(left); }
    if (right.size()) { righttree = new DFoldRectangleTree<D>(right); }

    /* Project middle subtree down a dimension */
    if (overlapping.size()) { auxtree = new OverlapTree<D>(overlapping); }
  }
  ~DFoldRectangleTree() {
    if (lefttree) delete lefttree;
    if (righttree) delete righttree;
    if (auxtree) delete auxtree;
  }

  /* This coordinate passes all tests, so drop down to the next coordinate */
  int allsub(std::vector<void*>& acc, Real low[D], Real high[D]) const {
    int ncount = 0;
    if (auxtree) { ncount += auxtree->allTags()->intersect(acc, low, high); }
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
        if (auxtree) nfound += auxtree->allTags()->intersect(acc, low, high);
        if (righttree) nfound += righttree->allsub(acc, low, high);
      } else {
        if (auxtree) {
          std::vector<DFoldRectangleTree<D - 1>*> kds = auxtree->rightEndsRightOf(lbound);
          for (int i = 0; i < kds.size(); i++) { nfound += kds[i]->intersect(acc, low, high); }
        }
        if (righttree) nfound += righttree->intersect(acc, low, high, LeftOfFirstContainment);
      }
    } else if (_anctest == RightOfFirstContainment) {
      /* An ancestor of this node is in [lbound,ubound],
       * and the current splitpoint on the right branch relative to that node */
      if (splitpoint <= rbound) {
        if (righttree) nfound += righttree->intersect(acc, low, high, RightOfFirstContainment);
        if (auxtree) nfound += auxtree->allTags()->intersect(acc, low, high);
        if (lefttree) nfound += lefttree->allsub(acc, low, high);
      } else {
        if (auxtree) {
          std::vector<DFoldRectangleTree<D - 1>*> kds = auxtree->leftEndsLeftOf(rbound);
          for (int i = 0; i < kds.size(); i++) { nfound += kds[i]->intersect(acc, low, high); }
        }
        if (lefttree) nfound += lefttree->intersect(acc, low, high, RightOfFirstContainment);
      }
    } else {
      /* No ancestors contained in [lbound,ubound]. */
      if (lbound <= splitpoint && splitpoint <= rbound) {
        /* [lbound, splitpoint, rbound] */
        if (lefttree) nfound += lefttree->intersect(acc, low, high, LeftOfFirstContainment);
        if (auxtree) nfound += auxtree->allTags()->intersect(acc, low, high);
        if (righttree) nfound += righttree->intersect(acc, low, high, RightOfFirstContainment);
      } else if (splitpoint <= lbound) {
        /* splitpoint, [lbound,rbound] */
        if (auxtree) {
          std::vector<DFoldRectangleTree<D - 1>*> kds = auxtree->rightEndsRightOf(lbound);
          for (int i = 0; i < kds.size(); i++) { nfound += kds[i]->intersect(acc, low, high); }
        }
        if (righttree) nfound += righttree->intersect(acc, low, high, NoContainedAncestors);
      } else if (splitpoint >= rbound) {
        /* [lbound,rbound], splitpoint*/
        if (auxtree) {
          std::vector<DFoldRectangleTree<D - 1>*> kds = auxtree->leftEndsLeftOf(rbound);
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
    if (auxtree) auxtree->dumpTree(depth + 1);
    if (righttree) righttree->dumpTree(depth + 1);
    delete[] lsp;
  }

  int rectcount() const {
    int s = 0;
    if (lefttree) s += lefttree->rectcount();
    if (righttree) s += righttree->rectcount();
    if (auxtree) s += auxtree->allTags()->rectcount();
    return s;
  }

  Real getSplitpoint() const { return splitpoint; }

 private:
  Real splitpoint;
  OverlapTree<D>* auxtree;
  DFoldRectangleTree<D>* lefttree;
  DFoldRectangleTree<D>* righttree;
};

template<>
class DFoldRectangleTree<1> {
 public:
  DFoldRectangleTree<1>(const std::vector<struct Rectangle<1> >& input)
      : auxtree1d(NULL), lefttree(NULL), righttree(NULL) {
    std::vector<struct Rectangle<1> > left;
    std::vector<struct Rectangle<1> > right;
    std::vector<struct Rectangle<1> > overlapping;
    splitpoint = splitRectangles<1>(input, left, right, overlapping);

    /* Construct left and right trees */
    if (left.size()) { lefttree = new DFoldRectangleTree<1>(left); }
    if (right.size()) { righttree = new DFoldRectangleTree<1>(right); }

    /* Project middle subtree down a dimension */
    if (overlapping.size()) { auxtree1d = new OverlapTree1D(overlapping); }
  }
  ~DFoldRectangleTree<1>() {
    if (lefttree) delete lefttree;
    if (righttree) delete righttree;
    if (auxtree1d) delete auxtree1d;
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
        if (auxtree1d) nfound += auxtree1d->allTags(acc);
        if (righttree) nfound += righttree->allTags(acc);
      } else {
        /* splitpoint, [lbound, alpha, rbound] */
        if (auxtree1d) nfound += auxtree1d->rightEndsRightOf(acc, lbound);
        if (righttree) nfound += righttree->intersect(acc, low, high, LeftOfFirstContainment);
      }
    } else if (_anctest == RightOfFirstContainment) {
      /* An ancestor of this node is in [lbound,ubound],
       * and the current splitpoint on the right branch relative to that node */
      if (splitpoint <= rbound) {
        /* [lbound, alpha, splitpoint, rbound] */
        if (righttree) nfound += righttree->intersect(acc, low, high, RightOfFirstContainment);
        if (auxtree1d) nfound += auxtree1d->allTags(acc);
        if (lefttree) nfound += lefttree->allTags(acc);
      } else {
        /* [lbound, alpha, rbound], splitpoint */
        if (auxtree1d) nfound += auxtree1d->leftEndsLeftOf(acc, rbound);
        if (lefttree) nfound += lefttree->intersect(acc, low, high, RightOfFirstContainment);
      }
    } else {
      /* No ancestors contained in [lbound,ubound]. */
      if (lbound <= splitpoint && splitpoint <= rbound) {
        /* [lbound, splitpoint, rbound] */
        if (auxtree1d) nfound += auxtree1d->allTags(acc);
        if (lefttree) nfound += lefttree->intersect(acc, low, high, LeftOfFirstContainment);
        if (righttree) nfound += righttree->intersect(acc, low, high, RightOfFirstContainment);
      } else if (splitpoint <= lbound) {
        /* splitpoint, [lbound,rbound] */
        if (auxtree1d) nfound += auxtree1d->rightEndsRightOf(acc, lbound);
        if (righttree) nfound += righttree->intersect(acc, low, high, NoContainedAncestors);
      } else if (rbound <= splitpoint) {
        /* [lbound,rbound], splitpoint*/
        if (auxtree1d) nfound += auxtree1d->leftEndsLeftOf(acc, rbound);
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
            auxtree1d != NULL ? 'A' : '_', righttree != NULL ? 'R' : '_');
    if (lefttree) lefttree->dumpTree(depth + 1);
    if (auxtree1d) auxtree1d->dumpTree(depth + 1);
    if (righttree) righttree->dumpTree(depth + 1);
    delete[] lsp;
  }

  int allTags(std::vector<void*>& acc) const {
    int nfound = 0;
    if (lefttree) nfound += lefttree->allTags(acc);
    if (righttree) nfound += righttree->allTags(acc);
    if (auxtree1d) nfound += auxtree1d->allTags(acc);
    return nfound;
  }

  int rectcount() const {
    int s = 0;
    if (lefttree) s += lefttree->rectcount();
    if (righttree) s += righttree->rectcount();
    if (auxtree1d) s += auxtree1d->numTags();
    if (s < 1) error("Need at least one rectangle");
    return s;
  }

  Real getSplitpoint() const { return splitpoint; }

 private:
  static const int D = 1;

  Real splitpoint;
  OverlapTree1D* auxtree1d;
  DFoldRectangleTree<1>* lefttree;
  DFoldRectangleTree<1>* righttree;
};

template<unsigned int D>
OverlapTree<D>::OverlapTree(const std::vector<struct Rectangle<D> >& input) {
  N = input.size();

  AdjPt* mlepts = new AdjPt[N];
  AdjPt* mrepts = new AdjPt[N];
  for (int i = 0; i < N; i++) {
    AdjPt lpt;
    lpt.idx = i;
    lpt.r = input[i].lower[D - 1];
    mlepts[i] = lpt;
    AdjPt rpt;
    rpt.idx = i;
    rpt.r = input[i].upper[D - 1];
    mrepts[i] = rpt;
  }
  qsort(mlepts, N, sizeof(AdjPt), realcmp);
  qsort(mrepts, N, sizeof(AdjPt), realcmp);

  /* Lookup index into `input` vector */
  int* luk = new int[N];
  int* ruk = new int[N];
  rendpoints = new Real[N];
  lendpoints = new Real[N];
  for (int i = 0; i < N; i++) {
    luk[i] = mlepts[i].idx;
    lendpoints[i] = mlepts[i].r;
    ruk[N - i - 1] = mrepts[i].idx;
    rendpoints[N - 1 - i] = mrepts[i].r;
  }
  delete[] mlepts;
  delete[] mrepts;

  if (N > 1) {
    int k = 1;
    while (k < N) { k *= 2; }

    ltrees = new DFoldRectangleTree<D - 1>*[k];
    rtrees = new DFoldRectangleTree<D - 1>*[k];
    for (int i = 0; i < k; i++) {
      ltrees[i] = NULL;
      rtrees[i] = NULL;
    }

    /* Construct all the trees */
    int j = 1;
    while (j < N) {
      int S = k / (2 * j);

      for (int p = 0; S * (2 * p + 1) < N; p++) {
        std::vector<struct Rectangle<D - 1> > rslist;
        std::vector<struct Rectangle<D - 1> > lslist;
        for (int i = S * 2 * p; i < S * (2 * p + 1); i++) {
          struct Rectangle<D - 1> lret;
          struct Rectangle<D - 1> rret;
          for (int z = 0; z < D - 1; z++) {
            lret.upper[z] = input[luk[i]].upper[z];
            lret.lower[z] = input[luk[i]].lower[z];
            rret.upper[z] = input[ruk[i]].upper[z];
            rret.lower[z] = input[ruk[i]].lower[z];
          }
          lret.tag = input[luk[i]].tag;
          rret.tag = input[ruk[i]].tag;
          rslist.push_back(rret);
          lslist.push_back(lret);
        }
        ltrees[j - 1 + p] = new DFoldRectangleTree<D - 1>(lslist);
        rtrees[j - 1 + p] = new DFoldRectangleTree<D - 1>(rslist);
      }

      j *= 2;
    }
  } else {
    ltrees = NULL;
    rtrees = NULL;
  }
  delete[] luk;
  delete[] ruk;
  std::vector<struct Rectangle<D - 1> > dinput;
  for (int i = 0; i < N; i++) {
    struct Rectangle<D - 1> ret;
    for (int j = 0; j < D - 1; j++) {
      ret.upper[j] = input[i].upper[j];
      ret.lower[j] = input[i].lower[j];
    }
    ret.tag = input[i].tag;
    dinput.push_back(ret);
  }
  atree = new DFoldRectangleTree<D - 1>(dinput);
}

template<unsigned int D>
std::vector<DFoldRectangleTree<D - 1>*> OverlapTree<D>::leftEndsLeftOf(Real rbound) const {
  /* lendpoints in ascending order */
  std::vector<DFoldRectangleTree<D - 1>*> ret;
  /* fast cases, all and nothing */
  if (lendpoints[N - 1] <= rbound) {
    if (AC_DEBUG) {
      warning("triv lelo %f <= %f | %d %d", lendpoints[N - 1], rbound, D, atree->rectcount());
    }
    ret.push_back(atree);
    return ret;
  }
  if (rbound < lendpoints[0]) {
    if (AC_DEBUG) { warning("triv lelo %f < %f | %d", rbound, lendpoints[0], D); }
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
    } else if (lendpoints[j + k - 1] <= rbound) {
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
  if (rendpoints[N - 1] >= lbound) {
    if (AC_DEBUG) {
      warning("triv rero %f >= %f | %d %f %d", rendpoints[N - 1], lbound, D,
              atree->getSplitpoint(), atree->rectcount());
    }
    ret.push_back(atree);
    return ret;
  }
  if (lbound > rendpoints[0]) {
    if (AC_DEBUG) { warning("triv rero %f > %f | %d", lbound, rendpoints[0], D); }
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
    } else if (rendpoints[j + k - 1] >= lbound) {
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

AnimatedCollection::AnimatedCollection() { map = NULL; }

AnimatedCollection::~AnimatedCollection() {}

void AnimatedCollection::insert(Animated* a) { store.insert(a); }

void AnimatedCollection::remove(Animated* a) { store.erase(a); }

static std::vector<struct Rectangle<3> > staticinput;

void AnimatedCollection::recalculateBboxMap() {
  if (map) {
    delete (DFoldRectangleTree<3>*)map;
    map = NULL;
  }

  std::vector<struct Rectangle<3> > input;
  std::set<Animated*>::iterator iter = store.begin();
  std::set<Animated*>::iterator end = store.end();
  for (; iter != end; iter++) {
    Animated* a = *iter;
    struct Rectangle<3> r;
    r.tag = (void*)a;
    for (int i = 0; i < 3; i++) {
      r.lower[i] = a->position[i] + a->boundingBox[0][i];
      r.upper[i] = a->position[i] + a->boundingBox[1][i];
      if (r.lower[i] > r.upper[i]) { error("Impossible rectangle"); }
    }
    input.push_back(r);
  }
  staticinput = input;
  map = (void*)new DFoldRectangleTree<3>(input);
}

std::set<Animated*> AnimatedCollection::bboxOverlapsWith(const Animated* a) const {
  if (!map) {
    warning("Map not yet created");
    return std::set<Animated*>();
  }
  DFoldRectangleTree<3>* dmap = (DFoldRectangleTree<3>*)map;
  struct Rectangle<3> r;
  r.tag = (void*)a;
  for (int i = 0; i < 3; i++) {
    r.lower[i] = a->position[i] + a->boundingBox[0][i];
    r.upper[i] = a->position[i] + a->boundingBox[1][i];
  }

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

    if (coll.size() != nco || coll.size() != ret.size() | ret.size() != uco.size()) {
      warning("Interval tree error %d -> %d (ideal %d (%d present)),", uco.size(), ret.size(),
              coll.size(), nco);
    }
    return coll;
  }

  return ret;
}

const std::set<Animated*>& AnimatedCollection::asSet() const { return store; }
