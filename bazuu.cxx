#include "bazuu_bitboard_ops.hpp"
#include "defs.hpp"
#include <bazuu_ce_board.hpp>
#include <memory>
#include <print>

int main() {
  std::unique_ptr<BazuuBoard> board = std::make_unique<BazuuBoard>();
  board->setup_fen("rnbqkbnr/pp2p1p1/2p5/3pPp2/3P2Pp/2N2N2/PPP2P1P/R1BQKB1R b KQkq g3 0 6");
  board->verify_all_magics();
  board->print_board();
  board->generate_moves();
  return 0;
}
