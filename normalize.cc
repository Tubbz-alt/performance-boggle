// Rotate and reflect boards into a "normalized" position.
//
// Designed as a filter: replaces each the first 16 letter sequence on each
// line with a normalized version.

#include <iostream>
#include <stdio.h>
#include <string>
#include "board-utils.h"
#include "gflags/gflags.h"
#include "init.h"

DEFINE_bool(all_expansions, false,
            "Print all expansions, not just the canonical");
DEFINE_int32(width, 4, "Width of boards");
DEFINE_int32(height, 4, "Height of boards");

int main(int argc, char** argv) {
  Init(&argc, &argv);

  std::string s;
  std::vector<std::string> rots;
  BoardUtils bu(FLAGS_width, FLAGS_height);
  while (std::getline(std::cin, s)) {
    if (s.size() != FLAGS_width * FLAGS_height) continue;
    if (FLAGS_all_expansions) {
      if (bu.GenerateAnalogues(s, &rots)) {
        std::cout << s << std::endl;
        for (int i = 0; i < rots.size(); i++) {
          std::cout << rots[i] << std::endl;
        }
      }
    } else {
      std::cout << bu.Canonicalize(s) << std::endl;
    }
  }
}
