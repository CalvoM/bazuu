#include "defs.hpp"
BoardSquares file_rank_to_120_board(File file, Rank rank) {
  return static_cast<BoardSquares>((BOARD_64_OFFSET + std::to_underlying(file)) + (std::to_underlying(rank) * 10));
}
