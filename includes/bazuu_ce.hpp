#ifndef BAZUU_CE_H_
#define BAZUU_CE_H_

#include <cstdint>
#define NAME "Bazuu"
#define VERSION "1.0.0"
#define BRD_SQ_NUM 120
#define MAX_PLY 2048

using U64 = unsigned long long;

enum class ChessPieces : std::uint8_t {
  Empty,
  wP,
  wN,
  wB,
  wR,
  wQ,
  wK,
  bP,
  bN,
  bB,
  bR,
  bQ,
  bK
};
enum class File : std::uint8_t { A, B, C, D, E, F, G, H, NONE };
enum class Rank : std::uint8_t { R1, R2, R3, R4, R5, R6, R7, R8, NONE };
enum class Colours : std::uint8_t { White, Black, Both };
enum class BoardSquares : std::uint8_t {
  A1 = 21,
  B1,
  C1,
  D1,
  E1,
  F1,
  G1,
  H1,
  A2 = 31,
  B2,
  C2,
  D2,
  E2,
  F2,
  G2,
  H2,
  A3 = 41,
  B3,
  C3,
  D3,
  E3,
  F3,
  G3,
  H3,
  A4 = 51,
  B4,
  C4,
  D4,
  E4,
  F4,
  G4,
  H4,
  A5 = 61,
  B5,
  C5,
  D5,
  E5,
  F5,
  G5,
  H5,
  A6 = 71,
  B6,
  C6,
  D6,
  E6,
  F6,
  G6,
  H6,
  A7 = 81,
  B7,
  C7,
  D7,
  E7,
  F7,
  G7,
  H7,
  A8 = 91,
  B8,
  C8,
  D8,
  E8,
  F8,
  G8,
  H8,
  NO_SQ
};
enum class Turn : std::uint8_t { White, Black };
enum class Castling : std::uint8_t {
  WhiteShort = 1,
  WhiteLong = 2,
  BlackShort = 4,
  BlackLong = 8
};
typedef struct {
  std::uint16_t move;
  Castling castle_perms;
  std::uint16_t enPassant_square;
  std::uint16_t fifty_move_counter;
  U64 pos_key;
} Undo;

class BazuuBoard {
public:
  BazuuBoard();
  std::uint16_t pieces[BRD_SQ_NUM];
  U64 pawn_bits[3]; // White, Black and Both Color pawns.
  Turn current_turn;
  std::uint16_t current_king_square[2];
  std::uint16_t enPassant_square;
  std::uint16_t fifty_move_counter;
  std::uint16_t ply;
  std::uint16_t ply_counter;
  U64 pos_key; // Unique Key generated for a position.
  std::uint16_t pieces_on_board[13];
  std::uint16_t non_pawn_pieces[3]; // White, Black and Both Colors.
  std::uint16_t major_pieces[3];    // White, Black and Both Colors.
  std::uint16_t minor_pieces[3];    // White, Black and Both Colors.
  Undo ply_history[MAX_PLY];
  BoardSquares file_rank_to_120_board(File file, Rank rank) const;
  void init_board_squares();
  void print_square_layout();

private:
  std::uint8_t sq_120_to_sq_64[BRD_SQ_NUM];
  BoardSquares sq_64_to_sq_120[64];
};
#endif
