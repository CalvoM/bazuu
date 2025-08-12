#include <bazuu_ce_board.hpp>

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
  return 0;
}
