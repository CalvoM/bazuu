#include "defs.hpp"
#include <bazuu_ce_board.hpp>
#include <print>
#include <utility>

int main() {
  BazuuBoard board;
  board.setup_fen("3q4/3q4/3q4/8/qqq2qq1/8/3q4/3q4 w - - 0 1");
  board.print_bit_board(
      board.mask_rook_attacks_realtime(BoardSquares::D4, board.get_bitboard_of_piece(PieceType::Q, Colours::Black)));
  return 0;
}
