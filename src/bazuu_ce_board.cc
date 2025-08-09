#include "bazuu_ce_zobrist.hpp"
#include "defs.hpp"
#include <bazuu_ce_board.hpp>
#include <bit>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <print>
#include <utility>

BazuuBoard::BazuuBoard() {
  this->zobrist = std::make_shared<BazuuZobrist>();
  this->zobrist->init();
  this->game_state = std::make_shared<BazuuGameState>();
  this->init_board_squares();
  this->init_bit_board();
  this->init_piece_list();
  this->generate_hash_keys();
}

// Initializes the two 120 and 64 squares boards.
// Creates the mapping between the two boards.
void BazuuBoard::init_board_squares() {
  BoardSquares square_on_120_board = BoardSquares::A1;
  uint8_t square_on_64_board = 0;
  std::memset(this->sq_120_to_sq_64, this->INVALID_SQUARE_ON_64, sizeof(this->sq_120_to_sq_64));
  std::memset(this->sq_64_to_sq_120, std::to_underlying(BoardSquares::NO_SQ), sizeof(this->sq_64_to_sq_120));
  for (int rank = std::to_underlying(Rank::R1); rank <= std::to_underlying(Rank::R8); ++rank) {
    for (int file = std::to_underlying(File::A); file <= std::to_underlying(File::H); ++file) {
      square_on_120_board = this->file_rank_to_120_board(static_cast<File>(file), static_cast<Rank>(rank));
      this->sq_64_to_sq_120[square_on_64_board] = square_on_120_board;
      this->sq_120_to_sq_64[std::to_underlying(square_on_120_board)] = square_on_64_board;
      square_on_64_board++;
    }
  }
}
void BazuuBoard::init_bit_board() {
  // White BitBoards
  BitBoard pieces_bb = 0ULL;
  for (BoardSquares sq : {BoardSquares::A2, BoardSquares::B2, BoardSquares::C2, BoardSquares::D2, BoardSquares::E2,
                          BoardSquares::F2, BoardSquares::G2, BoardSquares::H2}) {
    pieces_bb |= 1ULL << this->to_64_board_square(sq);
  }
  this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::P)] = pieces_bb;

  pieces_bb = 0ULL;
  for (BoardSquares sq : {BoardSquares::B1, BoardSquares::G1}) {
    pieces_bb |= 1ULL << this->to_64_board_square(sq);
  }
  this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::N)] = pieces_bb;

  pieces_bb = 0ULL;
  for (BoardSquares sq : {BoardSquares::C1, BoardSquares::F1}) {
    pieces_bb |= 1ULL << this->to_64_board_square(sq);
  }
  this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::B)] = pieces_bb;

  pieces_bb = 0ULL;
  for (BoardSquares sq : {BoardSquares::A1, BoardSquares::H1}) {
    pieces_bb |= 1ULL << this->to_64_board_square(sq);
  }
  this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::R)] = pieces_bb;

  this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::Q)] =
      1ULL << this->to_64_board_square(BoardSquares::D1);
  this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::K)] =
      1ULL << this->to_64_board_square(BoardSquares::E1);
  this->bitboards_for_sides[std::to_underlying(Colours::White)] =
      this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::P)] |
      this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::N)] |
      this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::B)] |
      this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::R)] |
      this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::Q)] |
      this->bitboards_for_pieces[std::to_underlying(Colours::White)][std::to_underlying(PieceType::K)];
  // Black
  pieces_bb = 0ULL;
  for (BoardSquares sq : {BoardSquares::A7, BoardSquares::B7, BoardSquares::C7, BoardSquares::D7, BoardSquares::E7,
                          BoardSquares::F7, BoardSquares::G7, BoardSquares::H7}) {
    pieces_bb |= 1ULL << this->to_64_board_square(sq);
  }
  this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::P)] = pieces_bb;

  pieces_bb = 0ULL;
  for (BoardSquares sq : {BoardSquares::B8, BoardSquares::G8}) {
    pieces_bb |= 1ULL << this->to_64_board_square(sq);
  }
  this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::N)] = pieces_bb;

  pieces_bb = 0ULL;
  for (BoardSquares sq : {BoardSquares::C8, BoardSquares::F8}) {
    pieces_bb |= 1ULL << this->to_64_board_square(sq);
  }
  this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::B)] = pieces_bb;

  pieces_bb = 0ULL;
  for (BoardSquares sq : {BoardSquares::A8, BoardSquares::H8}) {
    pieces_bb |= 1ULL << this->to_64_board_square(sq);
  }
  this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::R)] = pieces_bb;

  this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::Q)] =
      1ULL << this->to_64_board_square(BoardSquares::D8);
  this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::K)] =
      1ULL << this->to_64_board_square(BoardSquares::E8);
  this->bitboards_for_sides[std::to_underlying(Colours::Black)] =
      this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::P)] |
      this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::N)] |
      this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::B)] |
      this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::R)] |
      this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::Q)] |
      this->bitboards_for_pieces[std::to_underlying(Colours::Black)][std::to_underlying(PieceType::K)];
}

void BazuuBoard::init_piece_list() {
  // Let us clear the piece counts.
  std::memset(this->piece_list, std::to_underlying(BoardSquares::NO_SQ), sizeof(this->piece_list));
  std::memset(this->piece_count, 0, sizeof(this->piece_count));
  for (int color = std::to_underlying(Colours::White); color < std::to_underlying(Colours::Both); color++) {
    for (int piece = std::to_underlying(PieceType::P); piece < std::to_underlying(PieceType::Empty); piece++) {
      BitBoard bb = this->bitboards_for_pieces[color][piece];
      while (bb) {
        // on the 120 square board the black side has lower index than white side.
        std::uint8_t square_on_64_board = std::countl_zero(bb);
        bb &= bb - 1; // clear the rightmost set bit.
        int idx = this->piece_count[color][piece]++;
        BoardSquares sq = this->to_120_board_square(square_on_64_board);
        this->piece_list[color][piece][idx] = sq;
      }
    }
  }
}

ZobristKey BazuuBoard::generate_hash_keys() {
  ZobristKey key = 0ULL;
  for (int color = std::to_underlying(Colours::White); color < std::to_underlying(Colours::Both); color++) {
    for (int piece = std::to_underlying(PieceType::P); piece < std::to_underlying(PieceType::Empty); piece++) {
      BitBoard bb = this->bitboards_for_pieces[color][piece];
      while (bb) {
        std::uint8_t square_on_64_board = std::countl_zero(bb);
        bb &= bb - 1; // clear the rightmost set bit.
        BoardSquares square = this->to_120_board_square(square_on_64_board);
        key ^= zobrist->piece_hash(Colours(color), PieceType(piece), square);
      }
    }
  }
  // Update key with side_hash_key
  key ^= zobrist->side_hash(this->game_state->active_side);
  // Update key with enpassant_hash_key
  key ^= zobrist->enpassant_hash(this->game_state->en_passant_square);
  // Update key with castling_hash_key
  assert(this->game_state->castling < 16);
  key ^= zobrist->castling_hash(this->game_state->castling);
  // TODO: Add the initialization of the CASTLING, SIDE and ENPASSANT hashes.
  return key;
}
// Maps the (file, rank) to square on the 120 square board.
BoardSquares BazuuBoard::file_rank_to_120_board(File file, Rank rank) const {
  return static_cast<BoardSquares>((this->BOARD_64_OFFSET + std::to_underlying(file)) +
                                   (std::to_underlying(rank) * 10));
}

// Maps the square on a 120 square board to index on the main 64 square chess board.
std::uint8_t BazuuBoard::to_64_board_square(BoardSquares square_on_120_board) const {
  return unsigned(this->sq_120_to_sq_64[std::to_underlying(square_on_120_board)]);
}
BoardSquares BazuuBoard::to_120_board_square(std::uint8_t square_on_64_board) const {
  return this->sq_64_to_sq_120[square_on_64_board];
}

// Prints the 2 boards - 120 square board and 64 square board.
void BazuuBoard::print_square_layout() {
  for (int i = 0; i < BRD_SQ_NUM; i++) {
    if (i % 10 == 0) {
      std::println();
    }
    std::cout << std::setw(2) << unsigned(this->sq_120_to_sq_64[i]) << " ";
  }
  std::println();
  for (int i = 0; i < 64; i++) {
    if (i % 8 == 0) {
      std::println();
      std::cout << std::setw(2) << " ";
    }
    std::cout << std::setw(2) << unsigned(std::to_underlying(this->sq_64_to_sq_120[i])) << " ";
  }
  std::println("\n");
}
void BazuuBoard::print_bit_board(BitBoard bit_board) {
  BoardSquares square_on_120_board = BoardSquares::A1;
  uint8_t square_on_64_board = 0;
  std::println("\n");
  for (int rank = std::to_underlying(Rank::R8); rank >= std::to_underlying(Rank::R1); --rank) {
    for (int file = std::to_underlying(File::A); file <= std::to_underlying(File::H); ++file) {
      square_on_120_board = this->file_rank_to_120_board(static_cast<File>(file), static_cast<Rank>(rank));
      square_on_64_board = this->sq_120_to_sq_64[std::to_underlying(square_on_120_board)];
      if ((1ULL << square_on_64_board) & bit_board) {
        std::cout << std::setw(2) << "X";
      } else {
        std::cout << std::setw(2) << "-";
      }
    }
    std::println();
  }
  std::println("\n");
}
void BazuuBoard::reset() {
  // Possibly in reverse order of setting up/initializing.
  this->game_state->reset();
  std::memset(this->piece_list, std::to_underlying(BoardSquares::NO_SQ), sizeof(this->piece_list));
  std::memset(this->piece_count, 0, sizeof(this->piece_count));
  std::memset(this->bitboards_for_pieces, 0, sizeof(this->bitboards_for_pieces));
  std::memset(this->bitboards_for_sides, 0, sizeof(this->bitboards_for_sides));
  std::memset(this->sq_120_to_sq_64, this->INVALID_SQUARE_ON_64, sizeof(this->sq_120_to_sq_64));
  std::memset(this->sq_64_to_sq_120, std::to_underlying(BoardSquares::NO_SQ), sizeof(this->sq_64_to_sq_120));
}
