#include "../includes/bazuu_ce.hpp"
#include <iomanip>
#include <iostream>
#include <utility>

BazuuBoard::BazuuBoard() { this->init_board_squares(); }

// Initializes the two 120 and 64 squares boards.
// Creates the mapping between the two boards.
void BazuuBoard::init_board_squares() {
  BoardSquares square_on_sq_120 = BoardSquares::A1;
  uint8_t square_on_sq_64 = 0;
  for (uint8_t i = 0; i < BRD_SQ_NUM; i++) {
    this->sq_120_to_sq_64[i] = 65;
  }
  for (uint8_t i = 0; i < 64; i++) {
    this->sq_64_to_sq_120[i] = BoardSquares::NO_SQ;
  }
  for (int rank = std::to_underlying(Rank::R1);
       rank <= std::to_underlying(Rank::R8); ++rank) {
    for (int file = std::to_underlying(File::A);
         file <= std::to_underlying(File::H); ++file) {
      square_on_sq_120 = this->file_rank_to_120_board(static_cast<File>(file),
                                                      static_cast<Rank>(rank));
      this->sq_64_to_sq_120[square_on_sq_64] = square_on_sq_120;
      this->sq_120_to_sq_64[std::to_underlying(square_on_sq_120)] =
          square_on_sq_64;
      square_on_sq_64++;
    }
  }
}
BoardSquares BazuuBoard::file_rank_to_120_board(File file, Rank rank) const {
  return static_cast<BoardSquares>((21 + std::to_underlying(file)) +
                                   (std::to_underlying(rank) * 10));
}
void BazuuBoard::print_square_layout() {
  for (int i = 0; i < BRD_SQ_NUM; i++) {
    if (i % 10 == 0) {
      std::cout << std::endl;
    }
    std::cout << std::setw(2) << unsigned(this->sq_120_to_sq_64[i]) << " ";
  }
  std::cout << std::endl;
  for (int i = 0; i < 64; i++) {
    if (i % 8 == 0) {
      std::cout << std::endl;
      std::cout << std::setw(2) << " ";
    }
    std::cout << std::setw(2)
              << unsigned(std::to_underlying(this->sq_64_to_sq_120[i])) << " ";
  }
}
