#ifndef BAZUU_BITBOARD_OPS_H_
#define BAZUU_BITBOARD_OPS_H_

#include "defs.hpp"

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

namespace BazuuBitBoardOps {

// constants
inline constexpr U64 A_FILE = 0x0101010101010101;
inline constexpr U64 H_FILE = 0x8080808080808080;
inline constexpr U64 RANK_1 = 0x00000000000000FF;
inline constexpr U64 RANK_8 = 0xFF00000000000000;
inline constexpr U64 A1_H8_DIAG = 0x8040201008040201;
inline constexpr U64 H1_A8_DIAG = 0x0102040810204080;
inline constexpr U64 LIGHT_SQUARE = 0x55AA55AA55AA55AA;
inline constexpr U64 DARK_SQUARE = 0xAA55AA55AA55AA55;
inline constexpr U64 NOT_A_FILE = 0xfefefefefefefefe; // ~0x0101010101010101
inline constexpr U64 NOT_H_FILE = 0x7f7f7f7f7f7f7f7f; // ~0x8080808080808080
inline constexpr U64 NOT_AB_FILES = 0xfcfcfcfcfcfcfcfc;
inline constexpr U64 NOT_GH_FILES = 0x3f3f3f3f3f3f3f3f;

// North Operations
inline BitBoard shiftNorth(BitBoard board) { return board << 8; }
inline BitBoard shiftNorthWest(BitBoard board) { return board << 7 & NOT_H_FILE; }
inline BitBoard shiftNorthEast(BitBoard board) { return board << 9 & NOT_A_FILE; }

// South Operations
inline BitBoard shiftSouth(BitBoard board) { return board >> 8; }
inline BitBoard shiftSouthWest(BitBoard board) { return board >> 9 & NOT_H_FILE; }
inline BitBoard shiftSouthEast(BitBoard board) { return board >> 7 & NOT_A_FILE; }

// White Operations
inline BitBoard WhiteSinglePushTargets(BitBoard whitePawns, BitBoard empty) { return shiftNorth(whitePawns) & empty; }
inline BitBoard WhiteDoublePushTargets(BitBoard whitePawns, BitBoard empty) {
  const U64 rank4 = 0x00000000FF000000ULL;
  U64 singlePushs = WhiteSinglePushTargets(whitePawns, empty);
  return shiftNorth(singlePushs) & empty & rank4;
}
inline BitBoard WhitePromotionTargets(BitBoard whitePawns, BitBoard empty) {
  return shiftNorth(whitePawns) & 0xFF00000000000000ULL & empty;
}
inline BitBoard WhitePawnAttacksTargets(BitBoard whitePawns, BitBoard occupancy) {
  return (shiftNorthEast(whitePawns) | shiftNorthWest(whitePawns)) & occupancy;
}
inline BitBoard WhitePawnPossibleAttacksTargets(BitBoard whitePawns) {
  return shiftNorthEast(whitePawns) | shiftNorthWest(whitePawns);
}
inline BitBoard WhitePawnAttacksWithPromotionTargets(BitBoard whitePawns, BitBoard occupancy) {
  return WhitePawnAttacksTargets(whitePawns, occupancy) & 0xFF00000000000000ULL;
}

// Black Operations
inline BitBoard BlackSinglePushTargets(BitBoard blackPawns, BitBoard empty) { return shiftSouth(blackPawns) & empty; }
inline BitBoard BlackDoublePushTargets(BitBoard blackPawns, BitBoard empty) {
  const U64 rank5 = 0x000000FF00000000ULL;
  U64 singlePushs = BlackSinglePushTargets(blackPawns, empty);
  return shiftSouth(singlePushs) & empty & rank5;
}
inline BitBoard BlackPromotionTargets(BitBoard blackPawns, BitBoard empty) {
  return shiftSouth(blackPawns) & 0x00000000000000FFULL & empty;
}
inline BitBoard BlackPawnAttacksTargets(BitBoard blackPawns, BitBoard occupancy) {
  return (shiftSouthEast(blackPawns) | shiftSouthWest(blackPawns)) & occupancy;
}
inline BitBoard BlackPawnPossibleAttacksTargets(BitBoard blackPawns) {
  return shiftSouthEast(blackPawns) | shiftSouthWest(blackPawns);
}
inline BitBoard BlackPawnAttacksWithPromotionTargets(BitBoard blackPawns, BitBoard occupancy) {
  return BlackPawnAttacksTargets(blackPawns, occupancy) & 0x00000000000000FFULL;
}
} // namespace BazuuBitBoardOps

#endif
