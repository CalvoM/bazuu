#include "defs.hpp"
#include <bazuu_ce_board.hpp>
#include <bit>
#include <cstdio>
#include <memory>
#include <print>
#include <utility>

int main() {
  std::unique_ptr<BazuuBoard> board = std::make_unique<BazuuBoard>();
  board->init_magic_numbers();
  return 0;
}
