#include "defs.hpp"
#include <bazuu_ce_board.hpp>
#include <memory>

int main() {
  std::unique_ptr<BazuuBoard> board = std::make_unique<BazuuBoard>();
  board->setup_fen(KILLER_BOARD_FEN);
  board->verify_all_magics();
  auto bb = board->occupancy();
  board->print_board();
  board->print_bit_board(board->get_bishop_attacks_lookup(BoardSquares::E2, bb) &
                         board->side_occupancy(Colours::Black));
  board->print_bit_board(board->get_rook_attacks_lookup(BoardSquares::F1, bb));
  board->print_bit_board(board->get_queen_attacks_lookup(BoardSquares::F3, bb) & board->side_occupancy(Colours::Black));
  return 0;
}
