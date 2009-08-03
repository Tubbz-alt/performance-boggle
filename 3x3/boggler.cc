#include "3x3/boggler.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
const int BogglerBase::kWordScores[] =
      //0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10
      { 0, 0, 0, 1, 1, 2, 3, 5, 11, 11, 11 };

bool BogglerBase::IsBoggleWord(const char* wd) {
  int size = strlen(wd);
  if (size < 3 || size > 10) return false;
  for (int i=0; i<size; ++i) {
    int c = wd[i];
    if (c<'a' || c>'z') return false;
    if (c=='q' && (i+1 >= size || wd[1+i] != 'u')) return false;
  }
  return true;
}

bool BogglerBase::BogglifyWord(char* word) {
  if (!IsBoggleWord(word)) return false;
  int src, dst;
  for (src=0, dst=0; word[src]; src++, dst++) {
    word[dst] = word[src];
    if (word[src] == 'q') src += 1;
  }
  word[dst] = word[src];
  return true;
}

Trie* Boggler::DictionaryFromFile(const char* dict_filename) {
  SimpleTrie* st = GenericBoggler<SimpleTrie>::DictionaryFromFile(dict_filename);
  Trie* t = Trie::CompactTrie(*st);
  delete st;
  return t;
}
