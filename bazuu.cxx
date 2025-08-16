#include "defs.hpp"
#include <bazuu_ce_board.hpp>
#include <print>
#include <utility>

int main() {
  BazuuBoard board;
  board.setup_fen();
  board.print_board();
  std::println();
  board.setup_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
  board.print_board();
  std::println();
  board.setup_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
  board.print_board();
  std::println();
  board.setup_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
  board.print_board();
  for (int square = std::to_underlying(BoardSquares::A1) - 21; square < 120; square++) {
    auto file_rank = board.get_file_and_rank(static_cast<BoardSquares>(square));
    if (square % 10 == 0 and square != 0)
      std::println();
    if (file_rank.first == File::NONE) {
      std::print("\e[0;32m(-|-)\x1b[0m ");

    } else {
      std::print("({},{}) ", std::to_underlying(file_rank.first), std::to_underlying(file_rank.second));
    }
  }
  return 0;
}
