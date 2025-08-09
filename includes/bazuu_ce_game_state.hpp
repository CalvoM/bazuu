#ifndef BAZUU_CE_GAME_STATE_H_
#define BAZUU_CE_GAME_STATE_H_
#include <cstdint>
#include <defs.hpp>
struct BazuuGameState {
  Colours active_side = Colours::Both;
  ZobristKey zobrist_key = 0ULL;
  CastlePermissions castling = 0ULL;
  BoardSquares en_passant_square = BoardSquares::NO_SQ;
  std::uint16_t ply_since_pawn_move = 0;
  std::uint16_t total_moves = 0;

  void reset() {
    this->active_side = Colours::Both;
    this->zobrist_key = 0ULL;
    this->castling = 0ULL;
    this->en_passant_square = BoardSquares::NO_SQ;
    this->ply_since_pawn_move = 0;
    this->total_moves = 0;
  }
};
#endif
