#ifndef DEFS_H_
#define DEFS_H_
#include <cstdint>
#include <utility>
using U64 = unsigned long long;
using BitBoard = U64;
using CastlePermissions = std::uint8_t;
using ZobristKey = U64;

enum class Pieces : std::uint8_t { Empty = 0, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };
enum class PieceType : std::uint8_t { P = 0, N, B, R, Q, K, Empty };
constexpr const char *PieceChars[2][std::to_underlying(PieceType::Empty)] = {{"♟", "♞", "♝", "♜", "♛", "♚"},
                                                                             {"♙", "♘", "♗", "♖", "♕", "♔"}};
constexpr const char *AsciiPieceChars[2][std::to_underlying(PieceType::Empty)] = {{"P", "N", "B", "R", "Q", "K"},
                                                                                  {"p", "n", "b", "r", "q", "k"}};
constexpr const char *square_to_coordinates[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", // rank 1
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", // rank 2
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", // rank 3
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", // rank 4
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", // rank 5
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", // rank 6
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", // rank 7
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", // rank 8
};
enum class File : std::uint8_t { A = 0, B, C, D, E, F, G, H, NONE };
enum class Rank : std::uint8_t { R1 = 0, R2, R3, R4, R5, R6, R7, R8, NONE };
enum class Colours : std::uint8_t { White, Black, Both };
constexpr char ActiveSideRep[4] = "wb-";
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
enum class Castling : std::uint8_t { WhiteShort = 1, WhiteLong = 2, BlackShort = 4, BlackLong = 8 };
constexpr const char *EMPTY_BOARD_FEN = "8/8/8/8/8/8/8/8 w - -";
constexpr const char *TRICKY_BOARD_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
constexpr const char *KILLER_BOARD_FEN = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
constexpr const char *CMK_BOARD_FEN = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9";
// LERF mapping
/*
 8  ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖
 7  ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙
 6  . . . . . . . .
 5  . . . . . . . .
 4  . . . . . . . .
 3  . . . . . . . .
 2  ♟ ♟ ♟ ♟ ♟ ♟ ♟ ♟
 1  ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜
    a b c d e f g h
*/

// LERF ROSE COMPASS
/*
 northwest    north   northeast
 noWe         nort         noEa
         +7    +8    +9
             \  |  /
 west    -1 <-  0 -> +1    east
             /  |  \
         -9    -8    -7
 soWe         sout         soEa
 southwest    south   southeast
*/

#endif
