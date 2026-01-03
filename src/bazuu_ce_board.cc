#include "bazuu_ce_zobrist.hpp"
#include "defs.hpp"
#include <algorithm>
#include <bazuu_ce_board.hpp>
#include <bit>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <print>
#include <string>
#include <utility>

const std::string BazuuBoard::STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const U64 A_FILE = 0x0101010101010101;
const U64 H_FILE = 0x8080808080808080;
const U64 RANK_1 = 0x00000000000000FF;
const U64 RANK_8 = 0xFF00000000000000;
const U64 A1_H8_DIAG = 0x8040201008040201;
const U64 H1_A8_DIAG = 0x0102040810204080;
const U64 LIGHT_SQUARE = 0x55AA55AA55AA55AA;
const U64 DARK_SQUARE = 0xAA55AA55AA55AA55;
const U64 NOT_A_FILE = 0xfefefefefefefefe; // ~0x0101010101010101
const U64 NOT_H_FILE = 0x7f7f7f7f7f7f7f7f; // ~0x8080808080808080

BazuuBoard::BazuuBoard() {
  this->zobrist = std::make_shared<BazuuZobrist>();
  this->zobrist->init();
  this->game_state = std::make_shared<BazuuGameState>();
  this->init_board_squares();
  this->game_state->zobrist_key = this->generate_hash_keys();
  this->init_non_sliding_attacks();
}

/*
 * Initializes the two 120 and 64 squares boards.
 * Creates the mapping between the two boards.
 */
void BazuuBoard::init_board_squares() {
  std::fill(std::begin(this->file_rank_to_board_mapper), std::end(this->file_rank_to_board_mapper),
            std::make_pair(File::NONE, Rank::NONE));
  BoardSquares square_on_120_board = BoardSquares::A1;
  uint8_t square_on_64_board = 0;
  std::memset(this->sq_120_to_sq_64, this->INVALID_SQUARE_ON_64, sizeof(this->sq_120_to_sq_64));
  std::memset(this->sq_64_to_sq_120, std::to_underlying(BoardSquares::NO_SQ), sizeof(this->sq_64_to_sq_120));
  for (int rank = std::to_underlying(Rank::R1); rank <= std::to_underlying(Rank::R8); ++rank) {
    for (int file = std::to_underlying(File::A); file <= std::to_underlying(File::H); ++file) {
      square_on_120_board = this->file_rank_to_120_board(static_cast<File>(file), static_cast<Rank>(rank));
      file_rank_to_board_mapper[std::to_underlying(square_on_120_board)] = {static_cast<File>(file),
                                                                            static_cast<Rank>(rank)};
      this->sq_64_to_sq_120[square_on_64_board] = square_on_120_board;
      this->sq_120_to_sq_64[std::to_underlying(square_on_120_board)] = square_on_64_board;
      square_on_64_board++;
    }
  }
}

/*
 * Initialize the attack bitboards and masks for non sliding pieces e.g. K, N and P
 */
void BazuuBoard::init_non_sliding_attacks() {
  for (std ::uint8_t square_on_64_board = 0; square_on_64_board < this->INVALID_SQUARE_ON_64; square_on_64_board++) {
    BoardSquares square_on_120_board = to_120_board_square(square_on_64_board);
    this->knight_attacks[std::to_underlying(square_on_120_board)] = this->mask_knight_attacks(square_on_120_board);
    this->king_attacks[std::to_underlying(square_on_120_board)] = this->mask_king_attacks(square_on_120_board);
  }
}
/*
 * Clears and updates the board piece list.
 */
void BazuuBoard::update_piece_list() {
  // Let us clear the piece counts.
  std::memset(this->piece_list, std::to_underlying(BoardSquares::NO_SQ), sizeof(this->piece_list));
  std::memset(this->piece_count, 0, sizeof(this->piece_count));
  for (int color = std::to_underlying(Colours::White); color < std::to_underlying(Colours::Both); color++) {
    for (int piece = std::to_underlying(PieceType::P); piece < std::to_underlying(PieceType::Empty); piece++) {
      BitBoard bb = this->bitboards_for_pieces[color][piece];
      while (bb) {
        // on the 120 square board the black side has lower index than white side.
        std::uint8_t square_on_64_board = std::countr_zero(bb);
        bb &= bb - 1; // clear the rightmost set bit.
        int idx = this->piece_count[color][piece]++;
        BoardSquares sq = this->to_120_board_square(square_on_64_board);
        this->piece_list[color][piece][idx] = sq;
      }
    }
  }
}

/*
 * Generate hash keys using mersenne twister.
 * @return zobrist hash key.
 */
ZobristKey BazuuBoard::generate_hash_keys() {
  ZobristKey key = 0ULL;
  for (int color = std::to_underlying(Colours::White); color < std::to_underlying(Colours::Both); color++) {
    for (int piece = std::to_underlying(PieceType::P); piece < std::to_underlying(PieceType::Empty); piece++) {
      BitBoard bb = this->bitboards_for_pieces[color][piece];
      while (bb) {
        std::uint8_t square_on_64_board = std::countr_zero(bb);
        bb &= bb - 1; // clear the rightmost set bit.
        BoardSquares square = this->to_120_board_square(square_on_64_board);
        key ^= zobrist->piece_hash(Colours(color), PieceType(piece), square);
      }
    }
  }
  // Update key with side_hash_key
  key ^= zobrist->side_hash(this->game_state->active_side);
  // Update key with enpassant_hash_key
  if (this->game_state->en_passant_square != BoardSquares::NO_SQ) {
    key ^= zobrist->enpassant_hash(this->game_state->en_passant_square);
  }
  // Update key with castling_hash_key
  assert(this->game_state->castling < 16);
  key ^= zobrist->castling_hash(this->game_state->castling);
  return key;
}

/*
 * Set up chess board from FEN position provided.
 * @param fen_position FEN position of the board.
 */
void BazuuBoard::setup_fen(const std::string fen_position) {
  std::memset(this->bitboards_for_pieces, 0, sizeof(this->bitboards_for_pieces));
  std::size_t pos = 0;
  std::uint8_t rank = 7;
  std::uint8_t file = 0;
  char token;
  std::uint8_t count;
  PieceType piece = PieceType::Empty;
  Colours active_side = Colours::Black;
  BoardSquares square_on_120_board = BoardSquares::NO_SQ;
  // handle position placement.
  while (fen_position[pos] != ' ') {
    token = fen_position[pos++];
    count = 1;
    if (token == '/') {
      rank -= 1;
      file = 0;
      count = 0;
      continue;
    } else if (std::isdigit(token)) {
      piece = PieceType::Empty;
      count = token - '0';
    } else {
      switch (token) {
      case 'p':
        piece = PieceType::P;
        active_side = Colours::Black;
        break;
      case 'r':
        piece = PieceType::R;
        active_side = Colours::Black;
        break;
      case 'n':
        piece = PieceType::N;
        active_side = Colours::Black;
        break;
      case 'b':
        piece = PieceType::B;
        active_side = Colours::Black;
        break;
      case 'q':
        piece = PieceType::Q;
        active_side = Colours::Black;
        break;
      case 'k':
        piece = PieceType::K;
        active_side = Colours::Black;
        break;
      case 'P':
        piece = PieceType::P;
        active_side = Colours::White;
        break;
      case 'R':
        piece = PieceType::R;
        active_side = Colours::White;
        break;
      case 'N':
        piece = PieceType::N;
        active_side = Colours::White;
        break;
      case 'B':
        piece = PieceType::B;
        active_side = Colours::White;
        break;
      case 'Q':
        piece = PieceType::Q;
        active_side = Colours::White;
        break;
      case 'K':
        piece = PieceType::K;
        active_side = Colours::White;
        break;
      default:
        std::println("Issue encountered: {}", token);
        // TODO: Better error handling.
      }
    }
    while (count > 0) {
      // If count is >0, then update the pieces according to the replacement number.
      if (piece != PieceType::Empty) {
        square_on_120_board = this->file_rank_to_120_board(static_cast<File>(file), static_cast<Rank>(rank));
        this->bitboards_for_pieces[std::to_underlying(active_side)][std::to_underlying(piece)] |=
            1ULL << this->to_64_board_square(square_on_120_board);
      }
      count--;
      file += 1;
    }
  }

  this->game_state->active_side = fen_position[++pos] == 'w' ? Colours::White : Colours::Black;
  pos += 2;
  while (fen_position[pos] != ' ') {
    token = fen_position[pos++];
    switch (token) {
    case '-':
      this->game_state->castling |= 0;
      break;
    case 'K':
      this->game_state->castling |= 1;
      break;
    case 'Q':
      this->game_state->castling |= 2;
      break;
    case 'k':
      this->game_state->castling |= 4;
      break;
    case 'q':
      this->game_state->castling |= 8;
      break;
    default:
      break;
    }
  }
  pos++;
  while (fen_position[pos] != ' ') {
    token = fen_position[pos++];
    if (token == '-') {
      this->game_state->en_passant_square = BoardSquares::NO_SQ;
    } else if (token > 'a' and token < 'h') {
      File file = static_cast<File>(token - 'a');
      Rank rank = static_cast<Rank>(fen_position[pos++] - '1');
      this->game_state->en_passant_square = this->file_rank_to_120_board(file, rank);
    }
  }
  pos++;
  std::string half_move = "";
  while (fen_position[pos] != ' ') {
    token = fen_position[pos++];
    half_move += token;
  }
  this->game_state->ply_since_pawn_move = std::stoi(half_move);
  pos += 1;
  std::string full_move = "";
  while (pos < fen_position.length()) {
    token = fen_position[pos++];
    full_move += token;
  }
  this->game_state->total_moves = std::stoi(full_move);
  this->update_piece_list();
  this->game_state->zobrist_key = this->generate_hash_keys();
  return;
}
/*
 * Maps the (file, rank) to square on the 120 square board.
 * @param chess File
 * @param chess Rank
 * @return BoardSquare on the 120 square board.
 */
BoardSquares BazuuBoard::file_rank_to_120_board(File file, Rank rank) const {
  return static_cast<BoardSquares>((this->BOARD_64_OFFSET + std::to_underlying(file)) +
                                   (std::to_underlying(rank) * 10));
}

/*
 * Maps the square on a 120 square board to index on the main 64 square chess board.
 * @param square_on_120_board square from the 120 square board.
 * @return index of square on the 64 square board.
 */
std::uint8_t BazuuBoard::to_64_board_square(BoardSquares square_on_120_board) const {
  return unsigned(this->sq_120_to_sq_64[std::to_underlying(square_on_120_board)]);
}

/*
 * Maps the index of the square on the 64 square board to 120 square board.
 * @param index of the square on the 64 square board.
 * @return square on the 120 square board.
 */
BoardSquares BazuuBoard::to_120_board_square(std::uint8_t square_on_64_board) const {
  return this->sq_64_to_sq_120[square_on_64_board];
}

/*
 * Prints the 2 boards - 120 square board and 64 square board.
 */
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

/*
 * Prints the bit board provided.
 * @param bit_board to be printed.
 */
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

/*
 * Prints the current state of the board.
 */
void BazuuBoard::print_board() {
  BoardSquares square_on_120_board = BoardSquares::A1;
  uint8_t square_on_64_board = 0;
  const char *piece_char = ".";
  bool piece_found = false;

  for (int rank = std::to_underlying(Rank::R8); rank >= std::to_underlying(Rank::R1); --rank) {
    std::print("\x1b[1;34m{}\x1b[0m  ", rank + 1);
    for (int file = std::to_underlying(File::A); file <= std::to_underlying(File::H); ++file) {
      square_on_120_board = this->file_rank_to_120_board(static_cast<File>(file), static_cast<Rank>(rank));
      square_on_64_board = this->sq_120_to_sq_64[std::to_underlying(square_on_120_board)];
      piece_char = ".";
      piece_found = false;
      for (int color = std::to_underlying(Colours::White); color < std::to_underlying(Colours::Both); color++) {
        for (int piece = std::to_underlying(PieceType::P); piece < std::to_underlying(PieceType::Empty); piece++) {
          if (this->bitboards_for_pieces[color][piece] & (1ULL << square_on_64_board)) {
            piece_char = PieceChars[color][piece];

            std::print("{} ", piece_char);
            piece_found = true;
          }
        }
      }
      if (!piece_found) {
        std::print("{} ", piece_char);
      }
    }
    std::println();
  }
  std::print("   ");
  for (int file = std::to_underlying(File::A); file <= std::to_underlying(File::H); ++file) {
    std::print("\x1b[1;31m{} ", char('a' + file));
  }
  std::println("\x1b[0m\n");
  std::println("\e[0;32m Side to play\x1b[0m: \e[4;32m{}\x1b[0m:",
               ActiveSideRep[std::to_underlying(this->game_state->active_side)]);
  std::println("\e[0;32m En-Passant Target:\x1b[0m: \e[4;32m{}\x1b[0m:",
               std::to_underlying(this->game_state->en_passant_square));
  std::println("\e[0;32m Hash Key of the position:\x1b[0m: \e[4;32m{}\x1b[0m:", this->game_state->zobrist_key);
}

/*
 * Get the bitboard of the a specific piece type of either color.
 * @param piece - The chess piece type.
 * @param colour - Specifies the colour of the piece type.
 * @return the bitboard.
 */
BitBoard BazuuBoard::get_bitboard_of_piece(PieceType piece, Colours colour) {
  return this->bitboards_for_pieces[std::to_underlying(colour)][std::to_underlying(piece)];
}

/*
 * Get the bitboard of all the pieces on the board, both sides/colors.
 */
BitBoard BazuuBoard::occupancy() const {
  return this->bitboards_for_sides[std::to_underlying(Colours::White)] |
         this->bitboards_for_sides[std::to_underlying(Colours::Black)];
}

/*
 * Get the board square on the king piece of a given side/colour.
 * @param colour - the side/colour of the king.
 */
BoardSquares BazuuBoard::king_square(Colours colour) const {
  BitBoard king_bb = this->bitboards_for_pieces[std::to_underlying(colour)][std::to_underlying(PieceType::K)];
  std::uint8_t square_on_64_board = std::countr_zero(king_bb);
  return this->to_120_board_square(square_on_64_board);
}

/*
 * Get the file and rank of a give board square on a 120 square board.
 * @param square_on_120_board board square on the 120 square board.
 * @return the file and rank of board square provided.
 */
std::pair<File, Rank> BazuuBoard::get_file_and_rank(BoardSquares square_on_120_board) const {
  return this->file_rank_to_board_mapper[std::to_underlying(square_on_120_board)];
}

/*
 * Compute the attack bitboards for a knight on a given square.
 * @param square_on_120_board board square on the 120 square board.
 * @return the bitboard of the knight attacks of a given board square provided.
 */
BitBoard BazuuBoard::mask_knight_attacks(BoardSquares square_on_120_board) {
  std::uint8_t square_on_64_board = to_64_board_square(square_on_120_board);
  BitBoard knight_pos = 1ULL << square_on_64_board;
  BitBoard attacks = knight_pos;
  // Front
  attacks |= knight_pos << 17 & this->NOT_A_FILE;
  attacks |= knight_pos << 15 & this->NOT_H_FILE;
  attacks |= knight_pos << 10 & this->NOT_AB_FILES;
  attacks |= knight_pos << 6 & this->NOT_GH_FILES;
  // Back
  attacks |= knight_pos >> 17 & this->NOT_H_FILE;
  attacks |= knight_pos >> 15 & this->NOT_A_FILE;
  attacks |= knight_pos >> 10 & this->NOT_GH_FILES;
  attacks |= knight_pos >> 6 & this->NOT_AB_FILES;
  return attacks;
}

/*
 * Compute the attack bitboards for a king on a given square.
 * @param square_on_120_board board square on the 120 square board.
 * @return the bitboard of the king attacks of a given board square provided.
 */
BitBoard BazuuBoard::mask_king_attacks(BoardSquares square_on_120_board) {
  std::uint8_t square_on_64_board = to_64_board_square(square_on_120_board);
  BitBoard king_pos = 1ULL << square_on_64_board;
  BitBoard attacks = king_pos;
  // Front
  attacks |= king_pos << 7 & this->NOT_H_FILE;
  attacks |= king_pos << 8;
  attacks |= king_pos << 9 & this->NOT_A_FILE;
  // Side
  attacks |= king_pos << 1 & this->NOT_A_FILE;
  attacks |= king_pos >> 1 & this->NOT_H_FILE;
  // Back
  attacks |= king_pos >> 7 & this->NOT_A_FILE;
  attacks |= king_pos >> 8;
  attacks |= king_pos >> 9 & this->NOT_H_FILE;
  return attacks;
}

/*
 * Get the knight attacks bit board for a given board square.
 * @param square_on_120_board board square on the 120 square board.
 * @return the bitboard of the knight attacks of a given board square provided.
 */
BitBoard BazuuBoard::get_knight_attacks(BoardSquares square_on_120_board) const {
  return this->knight_attacks[std::to_underlying(square_on_120_board)];
}

/*
 * Get the king attacks bit board for a given board square.
 * @param square_on_120_board board square on the 120 square board.
 * @return the bitboard of the king attacks of a given board square provided.
 */
BitBoard BazuuBoard::get_king_attacks(BoardSquares square_on_120_board) const {
  return this->king_attacks[std::to_underlying(square_on_120_board)];
}
/*
 * Reset the chess board.
 */
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
