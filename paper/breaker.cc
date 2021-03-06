// Author: danvk@google.com (Dan Vanderkam)
//
// Play bucketed boggle w/o a bucketed trie. This could potentially be really
// slow, but will give better bounds and allow more flexible bucketing.

#include "breaker.h"

#include <algorithm>
#include <inttypes.h>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <vector>

using std::cout;
using std::endl;
using std::setprecision;

namespace paper {

Breaker::Breaker(BucketBoggler* bb, int best_score)
    : bb_(bb), best_score_(best_score) {
  debug_ = true;
}

// TODO(danvk): make these methods and stop passing around state.
double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}


// Use the board solving details to pick a bucket to kill and guess how many
// board representatives this will kill.
int Breaker::PickABucket(std::vector<std::string>* splits, int level) {
  int pick = -1;

  splits->clear();
  // Possible heuristics:
  //   - always pick the center when possible or a non-corner
  //   - pick the cell with the most letters
  //   - pick the cell with the least letters > 1
  // TODO(danvk): make this choice a parameter
  int order[] = { 4, 5, 3, 1, 7, 0, 2, 6, 8 };
  for (int i = 0; i < 9; i++) {
    if (strlen(bb_->Cell(order[i])) > 1) {
      pick = order[i];
      break;
    }
  }
  // int max_len = 1;
  // for (int i = 0; i < 9; i++) {
  //   int len = strlen(bb_->Cell(i));
  //   if (len > max_len) {
  //     max_len = len;
  //     pick = i;
  //   }
  // }

  // TODO(danvk): analyze this in more detail; it has a big impact on speed.
  int len = strlen(bb_->Cell(pick));
  if (len == 26) {
    splits->push_back("aeiou");
    splits->push_back("sy");
    splits->push_back("bdfgjkmpvwxzq");
    splits->push_back("chlnrt");
  } else if (len >= 9) {
    int num_splits = 4;  //(len < 13 ? 3 : 4);
    splits->push_back("");
    int split = 0;
    for (int i = 0; i < len; i++) {
      if (1.0 * num_splits * i >= (split+1) * len) {
        split += 1;
        splits->push_back("");
      }
      splits->back() += std::string(1, bb_->Cell(pick)[i]);
    }
  } else {
    for (int j=0; bb_->Cell(pick)[j]; j++) {
      splits->push_back(std::string(1, bb_->Cell(pick)[j]));
    }
  }

  int out_len = 0;
  for (unsigned int i = 0; i < splits->size(); i++)
    out_len += splits->at(i).size();
  if (out_len != len) {
    cout << bb_->Cell(pick) << " (" << len << ") => " << out_len << endl;
    exit(1);
  }

  return pick;
}

void Breaker::SplitBucket(int level) {
  char orig_bd[27 * 9];
  char orig_cell[27];
  std::vector<std::string> splits;
  int cell = PickABucket(&splits, level);
  if (cell == -1) {
    // should be a board at this point, so the spaces are unneeded.
    std::string bd;
    const char* bd_class = bb_->as_string();
    for (; *bd_class; bd_class++) {
      if (*bd_class != ' ') bd.append(1, *bd_class);
    }
    details_->failures.push_back(bd);
    if (debug_) {
      cout << "Unable to break board: " << bd << endl;
    }
    return;
  }

  if (debug_) cout << "split cell " << cell << endl;

  strcpy(orig_bd, bb_->as_string());
  strcpy(orig_cell, bb_->Cell(cell));

  if (debug_) {
    cout << std::string(level, ' ') << "Will evaluate "
         << splits.size() << " more boards..." << endl;
  }

  for (unsigned int i=0; i < splits.size(); i++) {
    if (!bb_->ParseBoard(orig_bd)) {
      fprintf(stderr, "bucket boggle couldn't parse '%s'\n", orig_bd);
      exit(1);
    }
    strcpy(bb_->Cell(cell), splits[i].c_str());
    AttackBoard(level + 1, 1+i, splits.size());
  }
}

// Shed/Split until finished
void Breaker::AttackBoard(int level, int num, int outof) {
  uint64_t reps = bb_->NumReps();
  if (debug_) {
    float frac = 100.0 * elim_ / orig_reps_;
    float est = (secs() - details_->start_time) * orig_reps_ / elim_;
    cout << "(" << setprecision(5) << frac << "%)" << std::string(level, ' ')
         << " (" << level << ";" << num << "/" << outof << ") "
         << bb_->as_string() << " (" << reps << ") est. " << est << " s"
         << endl;
  }

  if (bb_->UpperBound(best_score_) <= best_score_) {
    elim_ += bb_->NumReps();
    if (level > details_->max_depth) details_->max_depth = level;
    return;
  } else {
    SplitBucket(level);
  }
}


void Breaker::Break(BreakDetails* details) {
  std::string orig = bb_->as_string();
  details_ = details;
  details_->max_depth = 0;
  details_->num_reps = 0;
  details_->elapsed = 0.0;
  details_->failures.clear();

  elim_ = 0;
  orig_reps_ = bb_->NumReps();
  details_->start_time = secs();
    AttackBoard();
  double b = secs();
  double a = details_->start_time;
  if (debug_) {
    float pace = 1.0*elim_/(b-a);
    cout << elim_ << " reps in " << setprecision(3) << (b - a) << " s "
         << "@ depth " << details_->max_depth
         << " = " << pace << " bds/sec:\n  " << orig
         << endl;
  }

  details->elapsed = b - a;
  details->num_reps = orig_reps_;
}


// misc board parsing stuff

bool Breaker::ParseBoard(const std::string& board) {
  return bb_->ParseBoard(board.c_str());
}

bool Breaker::FromId(const std::vector<std::string>& classes, uint64_t idx) {
  char buf[26 * 9 + 1];
  buf[0] = '\0';

  int num_classes = classes.size();
  uint64_t left = idx;
  for (int i = 0; i < 9; i++) {
    strcat(buf, classes[left % num_classes].c_str());
    left /= num_classes;
    if (i < 8) strcat(buf, " ");
  }
  if (left) return false;
  return bb_->ParseBoard(buf);
}

// static
uint64_t Breaker::BoardId(const BdArray& bd, int num_classes) {
  uint64_t id = 0;
  for (int i = 8; i >= 0; i--) {
    id *= num_classes;
    id += bd[i/3][i%3];
  }
  return id;
}

// static
bool Breaker::IsCanonical(int num_classes, uint64_t idx) {
  if (idx < 0) return false;
  int bd[3][3];
  uint64_t left = idx;
  for (int i = 0; i < 9; i++) {
    bd[i/3][i%3] = left % num_classes;
    left /= num_classes;
  }
  if (left) return false;

  for (int rot = 0; rot < 2; rot++) {
    // ABC    CBA
    // DEF -> FED
    // GHI    IHG
    for (int i = 0; i < 3; i++) std::swap(bd[0][i], bd[2][i]);
    if (BoardId(bd, num_classes) < idx) return false;

    // CBA    IHG
    // FED -> FED
    // IHG    CBA
    for (int i = 0; i < 3; i++) std::swap(bd[i][0], bd[i][2]);
    if (BoardId(bd, num_classes) < idx) return false;

    // IHG    GHI
    // FED -> DEF
    // CBA    ABC
    for (int i = 0; i < 3; i++) std::swap(bd[0][i], bd[2][i]);
    if (BoardId(bd, num_classes) < idx) return false;

    if (rot == 1) break;
    // GHI    ABC    ADG
    // DEF -> DEF -> BEH
    // ABC    GHI    CFI
    for (int i = 0; i < 3; i++) std::swap(bd[i][0], bd[i][2]);
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < i; j++)
        std::swap(bd[i][j], bd[j][i]);
    if (BoardId(bd, num_classes) < idx) return false;
  }

  return true;
}

}  // namespace paper
