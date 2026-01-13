#include "defs.hpp"
#include <bazuu_ce_board.hpp>
#include <memory>
#include <print>

int main() {
  std::unique_ptr<BazuuBoard> board = std::make_unique<BazuuBoard>();
  board->setup_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  board->verify_all_magics();
  board->print_board();
  std::println("{}", board->is_square_attacked(BoardSquares::F6, Colours::White));
  board->print_attacked_squares(Colours::Black);
  return 0;
}
