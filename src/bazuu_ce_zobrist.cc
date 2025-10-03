#include "defs.hpp"
#include <bazuu_ce_zobrist.hpp>
#include <random>
#include <utility>

BazuuZobrist::BazuuZobrist() {}
/*
 * Initializes the Zobristkey hash and generates the keys.
 */
void BazuuZobrist::init() {
  std::mt19937_64 side_hash(1023310525UL);

  // Let us hash the side_to_move_hash_key.
  for (int i = std::to_underlying(Colours::White); i < std::to_underlying(Colours::Both); i++) {
    this->side_to_move_hash_key[i] = side_hash();
  }

  // Let us hash the castling_hash_key.
  for (int i = 0; i < 16; i++) {
    this->castling_hash_key[i] = side_hash();
  }

  // Let us hash the pieces and squares.
  for (int colour = std::to_underlying(Colours::White); colour < std::to_underlying(Colours::Both); colour++) {
    for (int piece = std::to_underlying(PieceType::P); piece < std::to_underlying(PieceType::Empty); piece++) {
      for (int square = std::to_underlying(BoardSquares::A1) - 21; square < std::to_underlying(BoardSquares::NO_SQ);
           square++) {
        this->pieces_hash_key[colour][piece][square] = side_hash();
      }
    }
  }

  // Let us hash the enpassant positions.
  for (int f = std::to_underlying(File::A); f < std::to_underlying(File::NONE); f++) {
    this->enpassant_hash_key[f] = side_hash();
  }
}

/*
 * Get the hash key of specific piece type of a given colour and square.
 * @param colour Side/colour piece.
 * @param piece - A given piece type.
 * @param square - Square the piece is on.
 * @return the hash key of the piece type.
 */
U64 BazuuZobrist::piece_hash(Colours colour, PieceType piece, BoardSquares square) {
  return this->pieces_hash_key[std::to_underlying(colour)][std::to_underlying(piece)][std::to_underlying(square)];
}

/*
 * Get the hash key of a given side/colour.
 * @param colour the side to get the hash key.
 * @return hash key of the side/colour.
 */
U64 BazuuZobrist::side_hash(Colours colour) { return this->side_to_move_hash_key[std::to_underlying(colour)]; }

/*
 * Get the hash key of the castling permission.
 * @param the castling permission to get the hash key.
 * @return hash key of the castling permission.
 */
U64 BazuuZobrist::castling_hash(CastlePermissions permissions) { return this->castling_hash_key[permissions]; }

/*
 * Get the hash key of the enpassant square
 * @param square - the enpassant square on the 120 square board.
 * @return the hash key of the enpassant square.
 */
U64 BazuuZobrist::enpassant_hash(BoardSquares square) { return this->enpassant_hash_key[std::to_underlying(square)]; }
