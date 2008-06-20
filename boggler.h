// An interface for solving Boggle boards given a Trie.
// This is designed to be extremely efficient.

#ifndef SSBOGGLER_H
#define SSBOGGLER_H

#include "trie.h"

class Boggler {
 public:
  // Does not assume ownership of the Trie, though it must remain live for the
  // lifetime of the Boggler. The trie will be modified by board scoring, and
  // must not be modified by any other Boggler.
  Boggler(Trie* t);

  // Parses a 16 character boards string like "abcdefghijklmnop"
  bool ParseBoard(const char* lets);

  // Scores the current board
  int Score();

  // Shortcut for ParseBoard() + Score()
  int Score(const char* bd);

  // Set a cell on the current board. Must have 0 <= x, y < 4 and 0 <= c < 26.
  // These constraints are NOT checked.
  void SetCell(int x, int y, int c) { bd_[(x << 2) + y] = c; }
  int Cell(int x, int y) const { return bd_[(x << 2) + y]; }

  // Returns the total number of boards this Boggler has evaluated.
  int NumBoards() { return num_boards_; }

  // Is this a valid boggle word? e.g. only has 'q' followed by 'u'.
  static bool IsBoggleWord(const char* word);

  // Returns true if it's a valid boggle word and converts "qu" -> 'q'
  static bool BogglifyWord(char* word);
  
  // Load a dictionary file, removing all non-Boggle words and converting "qu"
  // to 'q'.
  static Trie* DictionaryFromFile(const char* dict_filename);

 private:
  void DoDFS(int i, int len, Trie* t);

  Trie* dict_;
  mutable unsigned int runs_;
  mutable int bd_[16];
  int num_boards_;
  int score_;
};

#endif
