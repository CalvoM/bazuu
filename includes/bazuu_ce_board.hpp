#ifndef BAZUU_CE_H_
#define BAZUU_CE_H_

#include <bazuu_ce_game_state.hpp>
#include <bazuu_ce_zobrist.hpp>
#include <cstdint>
#include <defs.hpp>
#include <memory>
#include <print>
#include <string>
#include <utility>

class BazuuBoard {
public:
  BazuuBoard();
  static constexpr std::string NAME = "Bazuu";
  static constexpr std::string VERSION = "1.0.0";
  static constexpr std::uint8_t BRD_SQ_NUM = 120;
  static constexpr std::uint16_t MAX_PLY = 2048;
  static constexpr std::uint8_t MAX_NUM_OF_PIECES_PER_TYPE = 10;
  static constexpr std::uint8_t BOARD_64_OFFSET = 21;
  static constexpr std::uint8_t INVALID_SQUARE_ON_64 = 65;
  static const std::string STARTING_FEN;
  std::uint16_t current_king_square[2];
  std::uint16_t pieces_on_board[13];
  std::uint16_t non_pawn_pieces[3]; // White, Black and Both Colors.
  std::uint16_t major_pieces[3];    // White, Black and Both Colors.
  std::uint16_t minor_pieces[3];    // White, Black and Both Colors.
  BazuuGameState history[MAX_PLY];
  void init_board_squares();
  void update_piece_list();
  void print_square_layout();
  void print_bit_board(BitBoard bit_board);
  void print_board();
  void setup_fen(const std::string fen_position = STARTING_FEN);
  ZobristKey generate_hash_keys();
  std::uint8_t to_64_board_square(BoardSquares square_on_120_board) const;
  BoardSquares to_120_board_square(std::uint8_t square_on_64_board) const;
  BoardSquares file_rank_to_120_board(File file, Rank rank) const;
  BitBoard get_bitboard_of_piece(PieceType piece, Colours colour);
  BitBoard occupancy() const;
  BoardSquares king_square(Colours colour) const;
  bool has_bishop_pair(Colours colour);
  std::pair<File, Rank> get_file_and_rank(BoardSquares square_on_120_board) const;
  void reset();

private:
  std::uint8_t sq_120_to_sq_64[BRD_SQ_NUM];
  BoardSquares sq_64_to_sq_120[64];
  std::shared_ptr<BazuuZobrist> zobrist;
  std::shared_ptr<BazuuGameState> game_state;
  BitBoard bitboards_for_pieces[std::to_underlying(Colours::Both)][std::to_underlying(PieceType::Empty)];
  BitBoard bitboards_for_sides[std::to_underlying(Colours::Both)];
  BoardSquares piece_list[std::to_underlying(Colours::Both)][std::to_underlying(PieceType::Empty)]
                         [MAX_NUM_OF_PIECES_PER_TYPE];
  std::uint8_t piece_count[std::to_underlying(Colours::Both)][std::to_underlying(PieceType::Empty)];
  std::pair<File, Rank> file_rank_to_board_mapper[BRD_SQ_NUM];
  void print_bits(U64 n) {
    unsigned long long i;
    std::string buf;
    i = 1UL << (sizeof(n) * CHAR_BIT - 1);
    while (i > 0) {
      if (n & i)
        buf += '1';
      else
        buf += '0';
      i >>= 1;
    }
    std::println("{}\n", buf);
  }
};
#endif
