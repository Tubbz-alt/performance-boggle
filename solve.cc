// Simple Boggle Solver
//
// Given a dictionary and letters on the board, print the score.

#include <cstdio>
#include <string>
#include <iostream>
#include "boggler.h"
#include "gflags/gflags.h"
#include "trie.h"

const char kUsage[] =
"%s <dictionary file> [abcdefghijklmnop [qrstuvwxyzabcdef [...]]]\n"
"A 'q' is treated as 'qu'.\n";

DEFINE_string(dictionary, "words", "Dictionary file");

void HandleBoard(Boggler* b, const char* bd);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  FILE* f = fopen(FLAGS_dictionary.c_str(), "r");
  if (f == NULL) {
    fprintf(stderr, "Couldn't open dictionary file %s\n", FLAGS_dictionary.c_str());
    exit(1);
  }
  fclose(f);

  Trie* t = Boggler::DictionaryFromFile(FLAGS_dictionary.c_str());
  Boggler b(t);

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      HandleBoard(&b, argv[i]);
    }
  } else {
    std::string s;
    while (std::cin >> s) {
      HandleBoard(&b, s.c_str());
    }
  }
}

void HandleBoard(Boggler* b, const char* bd) {
  // Parse the board.
  if (strlen(bd) != 16) {
    fprintf(stderr, "Board strings must contain sixteen characters, got %zu\n",
            strlen(bd));
    return;
  }

  for (int i = 0; i < 16; i++) {
    if (bd[i] >= 'A' && bd[i] <= 'Z') {
      fprintf(stderr, "Expect lowercase letters, found '%c'\n", bd[i]);
      return;
    } else if (bd[i] < 'a' || bd[i] > 'z') {
      fprintf(stderr, "Unexpected board character: '%c'\n", bd[i]);
      return;
    }
  }

  if (!b->ParseBoard(bd)) {
    fprintf(stderr, "Couldn't parse board string '%s'\n", bd);
    return;
  }
  int score = b->Score();
  fprintf(stdout, "%s: %d\n", b->ToString().c_str(), score);
}
