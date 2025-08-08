#ifndef BAZUU_CE_ZOBRIST_H_
#define BAZUU_CE_ZOBRIST_H_
#include <defs.hpp>
#include <utility>

class BazuuZobrist {
public:
  BazuuZobrist();
  void init();
  U64 piece_hash(Colours colour, PieceType piece, BoardSquares square);
  U64 side_hash(Colours colour);
  U64 castling_hash(CastlePermissions permissions);
  U64 enpassant_hash(BoardSquares square);

private:
  U64 pieces_hash_key[std::to_underlying(Colours::Both)][std::to_underlying(PieceType::Empty)]
                     [std::to_underlying(BoardSquares::NO_SQ)];
  U64 side_to_move_hash_key[std::to_underlying(Colours::Both)];
  U64 castling_hash_key[16];
  U64 enpassant_hash_key[std::to_underlying(File::NONE)];
};
#endif
