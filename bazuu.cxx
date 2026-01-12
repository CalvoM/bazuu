#include "defs.hpp"
#include <bazuu_ce_board.hpp>
#include <bit>
#include <cstdio>
#include <memory>
#include <print>
#include <utility>

int main() {
  std::unique_ptr<BazuuBoard> board = std::make_unique<BazuuBoard>();
  board->setup_fen("8/4P1P1/8/2P3P1/8/8/1P2PP2/8 w - - 0 1");
  board->verify_all_magics();
  auto bb = board->get_bitboard_of_piece(PieceType::P, Colours::White) |
            board->get_bitboard_of_piece(PieceType::P, Colours::Black);
  board->print_bit_board(bb);
  board->print_bit_board(board->get_rook_attacks_lookup(BoardSquares::E5, bb));
  board->print_bit_board(bb);
  board->print_bit_board(board->get_bishop_attacks_lookup(BoardSquares::D4, bb));
  return 0;
}
