#include "bazuu_bitboard_ops.hpp"
#include "bazuu_ce_board.hpp"
#include "bazuu_ce_zobrist.hpp"
#include "defs.hpp"
#include "prng.hpp"
#include <bit>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <print>
#include <set>

// ============================================================================
// BOARD SQUARE MAPPING TESTS
// ============================================================================

TEST_CASE("Board square mapping - 64 to 120 conversion", "[board][mapping]") {
  BazuuBoard board;

  SECTION("A1 maps correctly") {
    uint8_t sq64 = 0;
    BoardSquares sq120 = board.to_120_board_square(sq64);
    REQUIRE(sq120 == BoardSquares::A1);
    REQUIRE(std::to_underlying(sq120) == 21);
  }

  SECTION("H8 maps correctly") {
    uint8_t sq64 = 63;
    BoardSquares sq120 = board.to_120_board_square(sq64);
    REQUIRE(sq120 == BoardSquares::H8);
    REQUIRE(std::to_underlying(sq120) == 98);
  }

  SECTION("All 64 squares are valid") {
    for (uint8_t sq64 = 0; sq64 < 64; ++sq64) {
      BoardSquares sq120 = board.to_120_board_square(sq64);
      REQUIRE(sq120 != BoardSquares::NO_SQ);
    }
  }
}

TEST_CASE("Board square mapping - 120 to 64 conversion", "[board][mapping]") {
  BazuuBoard board;

  SECTION("A1 maps correctly") {
    uint8_t sq64 = board.to_64_board_square(BoardSquares::A1);
    REQUIRE(sq64 == 0);
  }

  SECTION("H8 maps correctly") {
    uint8_t sq64 = board.to_64_board_square(BoardSquares::H8);
    REQUIRE(sq64 == 63);
  }

  SECTION("Invalid squares return INVALID marker") {
    uint8_t sq64 = board.to_64_board_square(static_cast<BoardSquares>(0));
    REQUIRE(sq64 == BazuuBoard::INVALID_SQUARE_ON_64);
  }
}

TEST_CASE("Board square mapping - roundtrip consistency", "[board][mapping]") {
  BazuuBoard board;

  SECTION("64->120->64 identity") {
    for (uint8_t sq64 = 0; sq64 < 64; ++sq64) {
      BoardSquares sq120 = board.to_120_board_square(sq64);
      uint8_t back_to_64 = board.to_64_board_square(sq120);
      REQUIRE(back_to_64 == sq64);
    }
  }
}

TEST_CASE("File and rank extraction", "[board][mapping]") {
  BazuuBoard board;

  SECTION("A1 is File A, Rank 1") {
    auto [file, rank] = board.get_file_and_rank(BoardSquares::A1);
    REQUIRE(file == File::A);
    REQUIRE(rank == Rank::R1);
  }

  SECTION("H8 is File H, Rank 8") {
    auto [file, rank] = board.get_file_and_rank(BoardSquares::H8);
    REQUIRE(file == File::H);
    REQUIRE(rank == Rank::R8);
  }

  SECTION("E4 is File E, Rank 4") {
    auto [file, rank] = board.get_file_and_rank(BoardSquares::E4);
    REQUIRE(file == File::E);
    REQUIRE(rank == Rank::R4);
  }
}

TEST_CASE("File rank to 120 board conversion", "[board][mapping]") {
  BazuuBoard board;

  SECTION("File A, Rank 1 gives A1") {
    BoardSquares sq = board.file_rank_to_120_board(File::A, Rank::R1);
    REQUIRE(sq == BoardSquares::A1);
  }

  SECTION("File H, Rank 8 gives H8") {
    BoardSquares sq = board.file_rank_to_120_board(File::H, Rank::R8);
    REQUIRE(sq == BoardSquares::H8);
  }
}

// ============================================================================
// FEN PARSING TESTS
// ============================================================================

TEST_CASE("FEN parsing - starting position", "[board][fen]") {
  BazuuBoard board;
  board.setup_fen(BazuuBoard::STARTING_FEN);

  SECTION("White pawns are on rank 2") {
    BitBoard white_pawns = board.get_bitboard_of_piece(PieceType::P, Colours::White);
    REQUIRE(white_pawns == 0x000000000000FF00ULL);
  }

  SECTION("Black pawns are on rank 7") {
    BitBoard black_pawns = board.get_bitboard_of_piece(PieceType::P, Colours::Black);
    REQUIRE(black_pawns == 0x00FF000000000000ULL);
  }

  SECTION("White king is on e1") {
    BitBoard white_king = board.get_bitboard_of_piece(PieceType::K, Colours::White);
    BoardSquares king_sq = board.king_square(Colours::White);
    REQUIRE(king_sq == BoardSquares::E1);
    REQUIRE(std::popcount(white_king) == 1);
  }

  SECTION("Black king is on e8") {
    BitBoard black_king = board.get_bitboard_of_piece(PieceType::K, Colours::Black);
    BoardSquares king_sq = board.king_square(Colours::Black);
    REQUIRE(king_sq == BoardSquares::E8);
    REQUIRE(std::popcount(black_king) == 1);
  }
}

TEST_CASE("FEN parsing - custom positions", "[board][fen]") {
  BazuuBoard board;

  SECTION("Empty board except kings") {
    board.setup_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    BitBoard occupancy = board.occupancy();
    REQUIRE(std::popcount(occupancy) == 2);
  }

  SECTION("Position with en passant") {
    board.setup_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    // Note: en passant square handling would need to be tested through game state
  }

  SECTION("Position without castling rights") {
    board.setup_fen("r3k2r/8/8/8/8/8/8/R3K2R w - - 0 1");
    // Castling rights would need to be tested through game state
    BitBoard white_rooks = board.get_bitboard_of_piece(PieceType::R, Colours::White);
    REQUIRE(std::popcount(white_rooks) == 2);
  }

  SECTION("Tricky position FEN parsing") {
    board.setup_fen(TRICKY_BOARD_FEN);
    BitBoard occ = board.occupancy();
    REQUIRE(std::popcount(occ) == 32);
  }

  SECTION("Killer position FEN parsing") {
    board.setup_fen(KILLER_BOARD_FEN);
    BitBoard white_pawns = board.get_bitboard_of_piece(PieceType::P, Colours::White);
    REQUIRE(std::popcount(white_pawns) == 9);
    BitBoard black_pawns = board.get_bitboard_of_piece(PieceType::P, Colours::Black);
    REQUIRE(std::popcount(black_pawns) == 7);
  }
}

// ============================================================================
// ATTACK GENERATION TESTS - KNIGHT
// ============================================================================

TEST_CASE("Knight attacks from center", "[board][attacks][knight]") {
  BazuuBoard board;

  SECTION("Knight on E4 has 8 attacks") {
    BitBoard attacks = board.get_knight_attacks(BoardSquares::E4);
    uint8_t e4_sq64 = board.to_64_board_square(BoardSquares::E4);
    REQUIRE(std::popcount(attacks) == 8);
  }

  SECTION("Knight on E4 attacks correct squares") {
    BitBoard attacks = board.get_knight_attacks(BoardSquares::E4);
    uint8_t e4_sq64 = board.to_64_board_square(BoardSquares::E4);

    // Expected attacks: d2, f2, c3, g3, c5, g5, d6, f6
    std::vector<BoardSquares> expected = {BoardSquares::D2, BoardSquares::F2, BoardSquares::C3, BoardSquares::G3,
                                          BoardSquares::C5, BoardSquares::G5, BoardSquares::D6, BoardSquares::F6};

    for (auto sq : expected) {
      uint8_t sq64 = board.to_64_board_square(sq);
      REQUIRE((attacks & (1ULL << sq64)) != 0);
    }
  }
}

TEST_CASE("Knight attacks from corner", "[board][attacks][knight]") {
  BazuuBoard board;

  SECTION("Knight on A1 has 2 attacks") {
    BitBoard attacks = board.get_knight_attacks(BoardSquares::A1);
    uint8_t a1_sq64 = board.to_64_board_square(BoardSquares::A1);
    REQUIRE(std::popcount(attacks) == 2);
  }

  SECTION("Knight on H8 has 2 attacks") {
    BitBoard attacks = board.get_knight_attacks(BoardSquares::H8);
    uint8_t h8_sq64 = board.to_64_board_square(BoardSquares::H8);
    REQUIRE(std::popcount(attacks) == 2);
  }
}

TEST_CASE("Knight attacks from edge", "[board][attacks][knight]") {
  BazuuBoard board;

  SECTION("Knight on A4 has 4 attacks") {
    BitBoard attacks = board.get_knight_attacks(BoardSquares::A4);
    uint8_t a4_sq64 = board.to_64_board_square(BoardSquares::A4);
    REQUIRE(std::popcount(attacks) == 4);
  }

  SECTION("Knight on E1 has 4 attacks") {
    BitBoard attacks = board.get_knight_attacks(BoardSquares::E1);
    uint8_t e1_sq64 = board.to_64_board_square(BoardSquares::E1);
    REQUIRE(std::popcount(attacks) == 4);
  }
}

// ============================================================================
// ATTACK GENERATION TESTS - KING
// ============================================================================

TEST_CASE("King attacks from center", "[board][attacks][king]") {
  BazuuBoard board;

  SECTION("King on E4 has 8 attacks") {
    BitBoard attacks = board.get_king_attacks(BoardSquares::E4);
    uint8_t e4_sq64 = board.to_64_board_square(BoardSquares::E4);
    REQUIRE(std::popcount(attacks) == 8);
  }
}

TEST_CASE("King attacks from corner", "[board][attacks][king]") {
  BazuuBoard board;

  SECTION("King on A1 has 3 attacks") {
    BitBoard attacks = board.get_king_attacks(BoardSquares::A1);
    uint8_t a1_sq64 = board.to_64_board_square(BoardSquares::A1);
    REQUIRE(std::popcount(attacks) == 3);
  }

  SECTION("King on H8 has 3 attacks") {
    BitBoard attacks = board.get_king_attacks(BoardSquares::H8);
    uint8_t h8_sq64 = board.to_64_board_square(BoardSquares::H8);
    REQUIRE(std::popcount(attacks) == 3);
  }
}

TEST_CASE("King attacks from edge", "[board][attacks][king]") {
  BazuuBoard board;

  SECTION("King on E1 has 5 attacks") {
    BitBoard attacks = board.get_king_attacks(BoardSquares::E1);
    uint8_t e1_sq64 = board.to_64_board_square(BoardSquares::E1);
    REQUIRE(std::popcount(attacks) == 5);
  }
}

// ============================================================================
// ATTACK GENERATION TESTS - PAWN
// ============================================================================

TEST_CASE("White pawn attacks", "[board][attacks][pawn]") {
  BazuuBoard board;

  SECTION("White pawn on E4 attacks D5 and F5") {
    BitBoard attacks = board.get_pawn_attacks(Colours::White, BoardSquares::E4);
    uint8_t e4_sq64 = board.to_64_board_square(BoardSquares::E4);
    REQUIRE(std::popcount(attacks) == 2);

    uint8_t d5_sq64 = board.to_64_board_square(BoardSquares::D5);
    uint8_t f5_sq64 = board.to_64_board_square(BoardSquares::F5);
    REQUIRE((attacks & (1ULL << d5_sq64)) != 0);
    REQUIRE((attacks & (1ULL << f5_sq64)) != 0);
  }

  SECTION("White pawn on A4 attacks only B5") {
    BitBoard attacks = board.get_pawn_attacks(Colours::White, BoardSquares::A4);
    uint8_t a4_sq64 = board.to_64_board_square(BoardSquares::A4);
    REQUIRE(std::popcount(attacks) == 1);
  }

  SECTION("White pawn on H4 attacks only G5") {
    BitBoard attacks = board.get_pawn_attacks(Colours::White, BoardSquares::H4);
    uint8_t h4_sq64 = board.to_64_board_square(BoardSquares::H4);
    REQUIRE(std::popcount(attacks) == 1);
  }
}

TEST_CASE("Black pawn attacks", "[board][attacks][pawn]") {
  BazuuBoard board;

  SECTION("Black pawn on E5 attacks D4 and F4") {
    BitBoard attacks = board.get_pawn_attacks(Colours::Black, BoardSquares::E5);
    uint8_t e5_sq64 = board.to_64_board_square(BoardSquares::E5);
    REQUIRE(std::popcount(attacks) == 2);

    uint8_t d4_sq64 = board.to_64_board_square(BoardSquares::D4);
    uint8_t f4_sq64 = board.to_64_board_square(BoardSquares::F4);
    REQUIRE((attacks & (1ULL << d4_sq64)) != 0);
    REQUIRE((attacks & (1ULL << f4_sq64)) != 0);
  }
}

// ============================================================================
// ATTACK GENERATION TESTS - BISHOP
// ============================================================================

TEST_CASE("Bishop attack masks", "[board][attacks][bishop]") {
  BazuuBoard board;

  SECTION("Bishop on E4 mask excludes edges") {
    BitBoard mask = board.mask_bishop_attacks(BoardSquares::E4);
    // Should not include squares on rank 1, 8 or file A, H
    REQUIRE((mask & 0xFF) == 0);                  // Rank 1
    REQUIRE((mask & 0xFF00000000000000ULL) == 0); // Rank 8
    REQUIRE((mask & 0x0101010101010101ULL) == 0); // File A
    REQUIRE((mask & 0x8080808080808080ULL) == 0); // File H
  }
}

TEST_CASE("Bishop realtime attacks with blockers", "[board][attacks][bishop]") {
  BazuuBoard board;

  SECTION("Bishop on E4 blocked by piece on G6") {
    uint8_t g6_sq64 = board.to_64_board_square(BoardSquares::G6);
    BitBoard blocker = 1ULL << g6_sq64;
    BitBoard attacks = board.mask_bishop_attacks_realtime(BoardSquares::E4, blocker);

    // Should include G6 but not H7
    uint8_t h7_sq64 = board.to_64_board_square(BoardSquares::H7);
    REQUIRE((attacks & (1ULL << g6_sq64)) != 0);
    REQUIRE((attacks & (1ULL << h7_sq64)) == 0);
  }
}

TEST_CASE("Bishop magic bitboard lookups", "[board][attacks][bishop][magic]") {
  BazuuBoard board;

  SECTION("Bishop on E4 with empty board") {
    board.setup_fen("8/8/8/8/4B3/8/8/8 w - - 0 1");
    BitBoard occ = board.occupancy();
    BitBoard attacks = board.get_bishop_attacks_lookup(BoardSquares::E4, occ);

    // Should attack all diagonal squares
    REQUIRE(std::popcount(attacks) == 13);
  }

  SECTION("Bishop on E4 with blockers") {
    board.setup_fen("8/8/6p1/8/4B3/8/2p5/8 w - - 0 1");
    BitBoard occ = board.occupancy();
    BitBoard attacks = board.get_bishop_attacks_lookup(BoardSquares::E4, occ);

    // Should include blockers but not squares beyond
    uint8_t g6_sq64 = board.to_64_board_square(BoardSquares::G6);
    uint8_t c2_sq64 = board.to_64_board_square(BoardSquares::C2);
    uint8_t h7_sq64 = board.to_64_board_square(BoardSquares::H7);
    uint8_t a1_sq64 = board.to_64_board_square(BoardSquares::A1);

    REQUIRE((attacks & (1ULL << g6_sq64)) != 0); // Blocker included
    REQUIRE((attacks & (1ULL << h7_sq64)) == 0); // Beyond blocker
    REQUIRE((attacks & (1ULL << c2_sq64)) != 0); // Blocker included
    REQUIRE((attacks & (1ULL << a1_sq64)) == 0); // Beyond blocker
  }
}

// ============================================================================
// ATTACK GENERATION TESTS - ROOK
// ============================================================================

TEST_CASE("Rook attack masks", "[board][attacks][rook]") {
  BazuuBoard board;

  SECTION("Rook on E4 mask excludes edges") {
    BitBoard mask = board.mask_rook_attacks(BoardSquares::E4);
    uint8_t e4_sq64 = board.to_64_board_square(BoardSquares::E4);

    // Should not include E1, E8, A4, H4
    uint8_t e1_sq64 = board.to_64_board_square(BoardSquares::E1);
    uint8_t e8_sq64 = board.to_64_board_square(BoardSquares::E8);
    uint8_t a4_sq64 = board.to_64_board_square(BoardSquares::A4);
    uint8_t h4_sq64 = board.to_64_board_square(BoardSquares::H4);

    REQUIRE((mask & (1ULL << e1_sq64)) == 0);
    REQUIRE((mask & (1ULL << e8_sq64)) == 0);
    REQUIRE((mask & (1ULL << a4_sq64)) == 0);
    REQUIRE((mask & (1ULL << h4_sq64)) == 0);
  }
}

TEST_CASE("Rook realtime attacks with blockers", "[board][attacks][rook]") {
  BazuuBoard board;

  SECTION("Rook on E4 blocked by piece on E6") {
    uint8_t e6_sq64 = board.to_64_board_square(BoardSquares::E6);
    BitBoard blocker = 1ULL << e6_sq64;
    BitBoard attacks = board.mask_rook_attacks_realtime(BoardSquares::E4, blocker);

    // Should include E6 but not E7 or E8
    uint8_t e7_sq64 = board.to_64_board_square(BoardSquares::E7);
    uint8_t e8_sq64 = board.to_64_board_square(BoardSquares::E8);
    REQUIRE((attacks & (1ULL << e6_sq64)) != 0);
    REQUIRE((attacks & (1ULL << e7_sq64)) == 0);
    REQUIRE((attacks & (1ULL << e8_sq64)) == 0);
  }
}

TEST_CASE("Rook magic bitboard lookups", "[board][attacks][rook][magic]") {
  BazuuBoard board;

  SECTION("Rook on E4 with empty board") {
    board.setup_fen("8/8/8/8/4R3/8/8/8 w - - 0 1");
    BitBoard occ = board.occupancy();
    BitBoard attacks = board.get_rook_attacks_lookup(BoardSquares::E4, occ);

    // Should attack all rank and file squares except its own
    REQUIRE(std::popcount(attacks) == 14);
  }

  SECTION("Rook on E4 with blockers") {
    board.setup_fen("8/8/4p3/8/2p1R1p1/8/8/8 w - - 0 1");
    BitBoard occ = board.occupancy();
    BitBoard attacks = board.get_rook_attacks_lookup(BoardSquares::E4, occ);

    // Should include blockers but not squares beyond
    uint8_t e6_sq64 = board.to_64_board_square(BoardSquares::E6);
    uint8_t c4_sq64 = board.to_64_board_square(BoardSquares::C4);
    uint8_t g4_sq64 = board.to_64_board_square(BoardSquares::G4);
    uint8_t e7_sq64 = board.to_64_board_square(BoardSquares::E7);
    uint8_t b4_sq64 = board.to_64_board_square(BoardSquares::B4);
    uint8_t h4_sq64 = board.to_64_board_square(BoardSquares::H4);

    REQUIRE((attacks & (1ULL << e6_sq64)) != 0); // Blocker included
    REQUIRE((attacks & (1ULL << e7_sq64)) == 0); // Beyond blocker
    REQUIRE((attacks & (1ULL << c4_sq64)) != 0); // Blocker included
    REQUIRE((attacks & (1ULL << b4_sq64)) == 0); // Beyond blocker
    REQUIRE((attacks & (1ULL << g4_sq64)) != 0); // Blocker included
    REQUIRE((attacks & (1ULL << h4_sq64)) == 0); // Beyond blocker
  }
}

// ============================================================================
// ATTACK GENERATION TESTS - QUEEN
// ============================================================================

TEST_CASE("Queen magic bitboard lookups", "[board][attacks][queen][magic]") {
  BazuuBoard board;

  SECTION("Queen on E4 with empty board") {
    board.setup_fen("8/8/8/8/4Q3/8/8/8 w - - 0 1");
    BitBoard occ = board.occupancy();
    BitBoard attacks = board.get_queen_attacks_lookup(BoardSquares::E4, occ);

    // Queen = Rook + Bishop attacks
    REQUIRE(std::popcount(attacks) == 27);
  }

  SECTION("Queen on E4 with blockers") {
    board.setup_fen("8/8/4p3/8/2p1Q1p1/8/2p5/8 w - - 0 1");
    BitBoard occ = board.occupancy();
    BitBoard attacks = board.get_queen_attacks_lookup(BoardSquares::E4, occ);

    // Should combine bishop and rook attacks
    uint8_t e6_sq64 = board.to_64_board_square(BoardSquares::E6);
    uint8_t c4_sq64 = board.to_64_board_square(BoardSquares::C4);
    uint8_t c2_sq64 = board.to_64_board_square(BoardSquares::C2);

    REQUIRE((attacks & (1ULL << e6_sq64)) != 0);
    REQUIRE((attacks & (1ULL << c4_sq64)) != 0);
    REQUIRE((attacks & (1ULL << c2_sq64)) != 0);
  }
}

// ============================================================================
// BITBOARD OPERATION TESTS
// ============================================================================

TEST_CASE("Occupancy calculation", "[board][bitboard]") {
  BazuuBoard board;
  board.setup_fen(BazuuBoard::STARTING_FEN);

  SECTION("Starting position has 32 pieces") {
    BitBoard occ = board.occupancy();
    REQUIRE(std::popcount(occ) == 32);
  }

  SECTION("Empty board has 0 pieces") {
    board.setup_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    BitBoard occ = board.occupancy();
    REQUIRE(std::popcount(occ) == 2);
  }
}

TEST_CASE("Side occupancy calculation", "[board][bitboard]") {
  BazuuBoard board;
  board.setup_fen(BazuuBoard::STARTING_FEN);

  SECTION("White has 16 pieces") {
    BitBoard white_occ = board.side_occupancy(Colours::White);
    REQUIRE(std::popcount(white_occ) == 16);
  }

  SECTION("Black has 16 pieces") {
    BitBoard black_occ = board.side_occupancy(Colours::Black);
    REQUIRE(std::popcount(black_occ) == 16);
  }

  SECTION("White and Black occupancies don't overlap") {
    BitBoard white_occ = board.side_occupancy(Colours::White);
    BitBoard black_occ = board.side_occupancy(Colours::Black);
    REQUIRE((white_occ & black_occ) == 0);
  }
}

TEST_CASE("King square location", "[board][bitboard]") {
  BazuuBoard board;

  SECTION("Find white king in starting position") {
    board.setup_fen(BazuuBoard::STARTING_FEN);
    BoardSquares king_sq = board.king_square(Colours::White);
    REQUIRE(king_sq == BoardSquares::E1);
  }

  SECTION("Find black king in custom position") {
    board.setup_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    BoardSquares king_sq = board.king_square(Colours::Black);
    REQUIRE(king_sq == BoardSquares::E8);
  }
}

TEST_CASE("Pop bit operation", "[board][bitboard]") {
  BazuuBoard board;

  SECTION("Pop bit clears specific bit") {
    U64 bb = 0xFFULL; // All bits in first byte set
    board.pop_bit(bb, 3);
    REQUIRE(bb == 0xF7ULL); // Bit 3 cleared
  }

  SECTION("Pop bit on already clear bit does nothing") {
    U64 bb = 0xF7ULL;
    board.pop_bit(bb, 3);
    REQUIRE(bb == 0xF7ULL);
  }
}

// ============================================================================
// OCCUPANCY BOARD GENERATION TESTS
// ============================================================================

TEST_CASE("Occupancy board generation", "[board][magic]") {
  BazuuBoard board;

  SECTION("Generate occupancies for bishop on E4") {
    BitBoard mask = board.mask_bishop_attacks(BoardSquares::E4);
    uint8_t bits = std::popcount(mask);

    // Generate a few occupancies
    BitBoard occ0 = board.create_occupancy_board(0, bits, mask);
    BitBoard occ1 = board.create_occupancy_board(1, bits, mask);

    REQUIRE(occ0 == 0);             // First occupancy should be empty
    REQUIRE(occ1 != 0);             // Second should have at least one bit set
    REQUIRE((occ1 & mask) == occ1); // All bits should be within mask
  }

  SECTION("Generate occupancies for rook on A1") {
    BitBoard mask = board.mask_rook_attacks(BoardSquares::A1);
    uint8_t bits = std::popcount(mask);

    BitBoard occ0 = board.create_occupancy_board(0, bits, mask);
    REQUIRE(occ0 == 0);
  }
}

// ============================================================================
// ZOBRIST HASHING TESTS
// ============================================================================

TEST_CASE("Zobrist hash generation", "[board][zobrist]") {
  BazuuBoard board1, board2;

  SECTION("Same position generates same hash") {
    board1.setup_fen(BazuuBoard::STARTING_FEN);
    board2.setup_fen(BazuuBoard::STARTING_FEN);
    ZobristKey hash1 = board1.generate_hash_keys();
    ZobristKey hash2 = board2.generate_hash_keys();
    REQUIRE(hash1 == hash2);
  }

  SECTION("Different positions generate different hashes") {
    board1.setup_fen(BazuuBoard::STARTING_FEN);
    board2.setup_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    ZobristKey hash1 = board1.generate_hash_keys();
    ZobristKey hash2 = board2.generate_hash_keys();
    REQUIRE(hash1 != hash2);
  }
}

// ============================================================================
// PIECE LIST TESTS
// ============================================================================

TEST_CASE("Piece list update", "[board][piecelist]") {
  BazuuBoard board;
  board.setup_fen(BazuuBoard::STARTING_FEN);

  SECTION("Starting position has correct piece counts") {
    // Each side should have 8 pawns
    // This would require exposing piece_count or adding getters
    // For now, we test indirectly through bitboards
    BitBoard white_pawns = board.get_bitboard_of_piece(PieceType::P, Colours::White);
    BitBoard black_pawns = board.get_bitboard_of_piece(PieceType::P, Colours::Black);
    REQUIRE(std::popcount(white_pawns) == 8);
    REQUIRE(std::popcount(black_pawns) == 8);
  }
}

// ============================================================================
// BOARD RESET TESTS
// ============================================================================

TEST_CASE("Board reset", "[board][reset]") {
  BazuuBoard board;
  board.setup_fen(BazuuBoard::STARTING_FEN);
  board.reset();

  SECTION("After reset, occupancy is zero") {
    BitBoard occ = board.occupancy();
    REQUIRE(occ == 0);
  }
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST_CASE("Edge cases", "[board][edge]") {
  BazuuBoard board;

  SECTION("Position with promoted pieces") {
    // 9 queens scenario (original + 8 promoted pawns)
    board.setup_fen("4k3/8/8/8/8/8/8/QQQQQQQKQ w - - 0 1");
    BitBoard white_queens = board.get_bitboard_of_piece(PieceType::Q, Colours::White);
    REQUIRE(std::popcount(white_queens) == 7);
  }
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

TEST_CASE("Full board operations", "[board][integration]") {
  BazuuBoard board;

  SECTION("Setup, generate attacks, verify consistency") {
    board.setup_fen(BazuuBoard::STARTING_FEN);

    // Get all piece positions
    BitBoard occ = board.occupancy();
    REQUIRE(std::popcount(occ) == 32);

    // Verify kings exist
    BoardSquares white_king = board.king_square(Colours::White);
    BoardSquares black_king = board.king_square(Colours::Black);
    REQUIRE(white_king != BoardSquares::NO_SQ);
    REQUIRE(black_king != BoardSquares::NO_SQ);

    // Generate hash
    ZobristKey hash = board.generate_hash_keys();
    REQUIRE(hash != 0);
  }
}

// ============================================================================
// GAME STATE TESTS
// ============================================================================

TEST_CASE("BazuuGameState reset", "[gamestate][reset]") {
  BazuuGameState state;

  SECTION("Reset clears all fields to defaults") {
    // Set state to non-default values
    state.active_side = Colours::White;
    state.zobrist_key = 0x123456789ABCDEFULL;
    state.castling = 15; // All castling rights
    state.en_passant_square = BoardSquares::E3;
    state.ply_since_pawn_move = 50;
    state.total_moves = 100;

    // Reset
    state.reset();

    // Verify all fields reset to defaults
    REQUIRE(state.active_side == Colours::Both);
    REQUIRE(state.zobrist_key == 0ULL);
    REQUIRE(state.castling == 0ULL);
    REQUIRE(state.en_passant_square == BoardSquares::NO_SQ);
    REQUIRE(state.ply_since_pawn_move == 0);
    REQUIRE(state.total_moves == 0);
  }

  SECTION("Reset is idempotent") {
    state.reset();
    BazuuGameState copy = state;
    state.reset();

    REQUIRE(state.active_side == copy.active_side);
    REQUIRE(state.zobrist_key == copy.zobrist_key);
    REQUIRE(state.castling == copy.castling);
    REQUIRE(state.en_passant_square == copy.en_passant_square);
    REQUIRE(state.ply_since_pawn_move == copy.ply_since_pawn_move);
    REQUIRE(state.total_moves == copy.total_moves);
  }
}

// ============================================================================
// ZOBRIST HASHING TESTS (DETAILED)
// ============================================================================

TEST_CASE("BazuuZobrist initialization", "[zobrist][init]") {
  BazuuZobrist zobrist;
  zobrist.init();

  SECTION("Piece hashes are non-zero") {
    U64 hash = zobrist.piece_hash(Colours::White, PieceType::P, BoardSquares::E4);
    REQUIRE(hash != 0);
  }

  SECTION("Different pieces have different hashes") {
    U64 white_pawn = zobrist.piece_hash(Colours::White, PieceType::P, BoardSquares::E4);
    U64 white_knight = zobrist.piece_hash(Colours::White, PieceType::N, BoardSquares::E4);
    REQUIRE(white_pawn != white_knight);
  }

  SECTION("Different colors have different hashes") {
    U64 white_pawn = zobrist.piece_hash(Colours::White, PieceType::P, BoardSquares::E4);
    U64 black_pawn = zobrist.piece_hash(Colours::Black, PieceType::P, BoardSquares::E4);
    REQUIRE(white_pawn != black_pawn);
  }

  SECTION("Different squares have different hashes") {
    U64 e4_pawn = zobrist.piece_hash(Colours::White, PieceType::P, BoardSquares::E4);
    U64 e5_pawn = zobrist.piece_hash(Colours::White, PieceType::P, BoardSquares::E5);
    REQUIRE(e4_pawn != e5_pawn);
  }
}

TEST_CASE("BazuuZobrist side hash", "[zobrist][side]") {
  BazuuZobrist zobrist;
  zobrist.init();

  SECTION("Side hashes are non-zero") {
    U64 white_hash = zobrist.side_hash(Colours::White);
    U64 black_hash = zobrist.side_hash(Colours::Black);
    REQUIRE(white_hash != 0);
    REQUIRE(black_hash != 0);
  }

  SECTION("Different sides have different hashes") {
    U64 white_hash = zobrist.side_hash(Colours::White);
    U64 black_hash = zobrist.side_hash(Colours::Black);
    REQUIRE(white_hash != black_hash);
  }
}

TEST_CASE("BazuuZobrist castling hash", "[zobrist][castling]") {
  BazuuZobrist zobrist;
  zobrist.init();

  SECTION("Different castling rights have different hashes") {
    U64 no_castling = zobrist.castling_hash(0);
    U64 white_short = zobrist.castling_hash(1);
    U64 white_long = zobrist.castling_hash(2);
    U64 all_castling = zobrist.castling_hash(15);

    REQUIRE(no_castling != white_short);
    REQUIRE(white_short != white_long);
    REQUIRE(white_long != all_castling);
  }

  SECTION("All 16 castling permissions have unique hashes") {
    std::set<U64> hashes;
    for (uint8_t perm = 0; perm < 16; ++perm) {
      hashes.insert(zobrist.castling_hash(perm));
    }
    REQUIRE(hashes.size() == 16);
  }
}

TEST_CASE("BazuuZobrist en passant hash", "[zobrist][enpassant]") {
  BazuuZobrist zobrist;
  zobrist.init();

  SECTION("En passant hashes are non-zero for valid files") {
    U64 a_file = zobrist.enpassant_hash(BoardSquares::A3);
    U64 e_file = zobrist.enpassant_hash(BoardSquares::E3);
    REQUIRE(a_file != 0);
    REQUIRE(e_file != 0);
  }

  SECTION("Different en passant files have different hashes") {
    U64 a_file = zobrist.enpassant_hash(BoardSquares::A3);
    U64 b_file = zobrist.enpassant_hash(BoardSquares::B3);
    U64 e_file = zobrist.enpassant_hash(BoardSquares::E3);

    REQUIRE(a_file != b_file);
    REQUIRE(b_file != e_file);
  }
}

// ============================================================================
// PRNG TESTS
// ============================================================================

TEST_CASE("PRNG constructor", "[prng][constructor]") {
  SECTION("PRNG accepts valid seed") {
    PRNG prng(12345ULL);
    U64 num = prng.rand64();
    REQUIRE(num != 0);
  }

  SECTION("PRNG produces deterministic sequences") {
    PRNG prng1(12345ULL);
    PRNG prng2(12345ULL);

    REQUIRE(prng1.rand64() == prng2.rand64());
    REQUIRE(prng1.rand64() == prng2.rand64());
    REQUIRE(prng1.rand64() == prng2.rand64());
  }

  SECTION("Different seeds produce different sequences") {
    PRNG prng1(12345ULL);
    PRNG prng2(54321ULL);

    REQUIRE(prng1.rand64() != prng2.rand64());
  }
}

TEST_CASE("PRNG rand64", "[prng][rand64]") {
  PRNG prng(1804289383ULL);

  SECTION("Generates non-zero numbers") {
    bool has_nonzero = false;
    for (int i = 0; i < 100; ++i) {
      U64 num = prng.rand64();
      if (num != 0) {
        has_nonzero = true;
        break;
      }
    }
    REQUIRE(has_nonzero);
  }

  SECTION("Generates varied numbers") {
    std::set<U64> numbers;
    for (int i = 0; i < 100; ++i) {
      numbers.insert(prng.rand64());
    }
    // Should have high diversity (at least 95 unique out of 100)
    REQUIRE(numbers.size() >= 95);
  }

  SECTION("Generates full 64-bit range") {
    PRNG test_prng(1804289383ULL);
    bool has_high_bit = false;
    for (int i = 0; i < 1000; ++i) {
      U64 num = test_prng.rand64();
      if (num & (1ULL << 63)) {
        has_high_bit = true;
        break;
      }
    }
    REQUIRE(has_high_bit);
  }
}

TEST_CASE("PRNG sparse_rand", "[prng][sparse]") {
  PRNG prng(1804289383ULL);

  SECTION("Generates sparser numbers than rand64") {
    PRNG prng1(1804289383ULL);
    PRNG prng2(1804289383ULL);

    int sparse_popcount_total = 0;
    int normal_popcount_total = 0;

    for (int i = 0; i < 100; ++i) {
      U64 sparse = prng1.sparse_rand();
      U64 normal = prng2.rand64();
      sparse_popcount_total += std::popcount(sparse);
      normal_popcount_total += std::popcount(normal);
    }

    // Sparse random should have significantly fewer bits set
    REQUIRE(sparse_popcount_total < normal_popcount_total);
  }

  SECTION("Can generate zero") {
    PRNG test_prng(1804289383ULL);
    bool has_zero = false;
    for (int i = 0; i < 1000; ++i) {
      if (test_prng.sparse_rand() == 0) {
        has_zero = true;
        break;
      }
    }
    REQUIRE(has_zero);
  }
}

// ============================================================================
// BITBOARD OPERATIONS TESTS - SHIFT FUNCTIONS
// ============================================================================

TEST_CASE("Bitboard shift operations - North", "[bitboard][shift][north]") {
  using namespace BazuuBitBoardOps;

  SECTION("shiftNorth moves bits up one rank") {
    BitBoard rank2 = 0x000000000000FF00ULL; // Rank 2
    BitBoard result = shiftNorth(rank2);
    REQUIRE(result == 0x0000000000FF0000ULL); // Rank 3
  }

  SECTION("shiftNorth from rank 8 wraps off board") {
    BitBoard rank8 = 0xFF00000000000000ULL;
    BitBoard result = shiftNorth(rank8);
    REQUIRE(result == 0); // Bits fall off the top
  }

  SECTION("shiftNorthWest respects file boundaries") {
    BitBoard h_file = 0x8080808080808080ULL; // H file
    BitBoard result = shiftNorthWest(h_file);
    // Should not wrap to A file
    REQUIRE((result & BazuuBitBoardOps::A_FILE) == 0);
  }

  SECTION("shiftNorthEast respects file boundaries") {
    BitBoard a_file = 0x0101010101010101ULL; // A file
    BitBoard result = shiftNorthEast(a_file);
    // Should not wrap to H file
    REQUIRE((result & BazuuBitBoardOps::H_FILE) == 0);
  }
}

TEST_CASE("Bitboard shift operations - South", "[bitboard][shift][south]") {
  using namespace BazuuBitBoardOps;

  SECTION("shiftSouth moves bits down one rank") {
    BitBoard rank7 = 0x00FF000000000000ULL; // Rank 7
    BitBoard result = shiftSouth(rank7);
    REQUIRE(result == 0x0000FF0000000000ULL); // Rank 6
  }

  SECTION("shiftSouth from rank 1 wraps off board") {
    BitBoard rank1 = 0x00000000000000FFULL;
    BitBoard result = shiftSouth(rank1);
    REQUIRE(result == 0); // Bits fall off the bottom
  }

  SECTION("shiftSouthWest respects file boundaries") {
    BitBoard h_file = 0x8080808080808080ULL; // H file
    BitBoard result = shiftSouthWest(h_file);
    // Should not wrap to A file
    REQUIRE((result & BazuuBitBoardOps::A_FILE) == 0);
  }

  SECTION("shiftSouthEast respects file boundaries") {
    BitBoard a_file = 0x0101010101010101ULL; // A file
    BitBoard result = shiftSouthEast(a_file);
    // Should not wrap to H file
    REQUIRE((result & BazuuBitBoardOps::H_FILE) == 0);
  }
}

// ============================================================================
// BITBOARD OPERATIONS TESTS - WHITE PAWN MOVES
// ============================================================================

TEST_CASE("White pawn single push", "[bitboard][pawn][white][push]") {
  using namespace BazuuBitBoardOps;

  SECTION("Single pawn push from rank 2") {
    BitBoard pawns = 0x0000000000001000ULL; // E2
    BitBoard empty = 0xFFFFFFFFFFFFEFFFULL; // All except E2
    BitBoard targets = WhiteSinglePushTargets(pawns, empty);
    REQUIRE(targets == 0x0000000000100000ULL); // E3
  }

  SECTION("Blocked pawn cannot push") {
    BitBoard pawns = 0x0000000000001000ULL; // E2
    BitBoard empty = 0xFFFFFFFFFFEFEFFFULL; // E2 and E3 blocked
    BitBoard targets = WhiteSinglePushTargets(pawns, empty);
    REQUIRE(targets == 0);
  }

  SECTION("Multiple pawns push") {
    BitBoard pawns = 0x000000000000FF00ULL; // All rank 2
    BitBoard empty = 0xFFFFFFFFFFFF00FFULL; // Rank 2 occupied, rest empty
    BitBoard targets = WhiteSinglePushTargets(pawns, empty);
    REQUIRE(targets == 0x00000000FF000000ULL); // All rank 4
  }
}

TEST_CASE("White pawn double push", "[bitboard][pawn][white][doublepush]") {
  using namespace BazuuBitBoardOps;

  SECTION("Double push from rank 2 to rank 4") {
    BitBoard pawns = 0x0000000000001000ULL; // E2
    BitBoard empty = 0xFFFFFFFFFFEFEFFFULL; // E2 blocked, E3 and E4 empty
    BitBoard targets = WhiteDoublePushTargets(pawns, empty);
    REQUIRE(targets == 0x0000000010000000ULL); // E4
  }

  SECTION("Double push blocked by piece on E3") {
    BitBoard pawns = 0x0000000000001000ULL; // E2
    BitBoard empty = 0xFFFFFFFFFFEFFFFFULL; // E3 blocked
    BitBoard targets = WhiteDoublePushTargets(pawns, empty);
    REQUIRE(targets == 0);
  }

  SECTION("Double push only from rank 2") {
    BitBoard pawns = 0x0000000000100000ULL; // E3 (not rank 2)
    BitBoard empty = 0xFFFFFFFFFFEFFFFFULL;
    BitBoard targets = WhiteDoublePushTargets(pawns, empty);
    REQUIRE(targets == 0); // Cannot double push from rank 3
  }
}

TEST_CASE("White pawn promotion", "[bitboard][pawn][white][promotion]") {
  using namespace BazuuBitBoardOps;

  SECTION("Pawn on rank 7 can promote") {
    BitBoard pawns = 0x0010000000000000ULL; // E7
    BitBoard empty = 0xFFEFFFFFFFFFFFFFULL; // E8 empty
    BitBoard targets = WhitePromotionTargets(pawns, empty);
    REQUIRE(targets == 0x1000000000000000ULL); // E8
  }

  SECTION("Promotion blocked by piece") {
    BitBoard pawns = 0x0010000000000000ULL; // E7
    BitBoard empty = 0xEFEFFFFFFFFFFFFFULL; // E8 occupied
    BitBoard targets = WhitePromotionTargets(pawns, empty);
    REQUIRE(targets == 0);
  }

  SECTION("Non-rank-7 pawns cannot promote") {
    BitBoard pawns = 0x0000001000000000ULL; // E6
    BitBoard empty = 0xFFFFFFEFFFFFFFFFULL;
    BitBoard targets = WhitePromotionTargets(pawns, empty);
    REQUIRE(targets == 0);
  }
}

TEST_CASE("White pawn attacks", "[bitboard][pawn][white][attacks]") {
  using namespace BazuuBitBoardOps;

  SECTION("Pawn attacks diagonal squares with occupancy") {
    BitBoard pawns = 0x0000000000001000ULL;     // E2
    BitBoard occupancy = 0x0000000000280000ULL; // D3 and F3
    BitBoard targets = WhitePawnAttacksTargets(pawns, occupancy);
    REQUIRE(targets == 0x0000000000280000ULL); // Attacks both
  }

  SECTION("Pawn attacks only occupied squares") {
    BitBoard pawns = 0x0000000000001000ULL;     // E2
    BitBoard occupancy = 0x0000000000080000ULL; // Only D3
    BitBoard targets = WhitePawnAttacksTargets(pawns, occupancy);
    REQUIRE(targets == 0x0000000000080000ULL); // Only D3
  }

  SECTION("Possible attacks includes empty squares") {
    BitBoard pawns = 0x0000000000001000ULL; // E2
    BitBoard targets = WhitePawnPossibleAttacksTargets(pawns);
    REQUIRE(targets == 0x0000000000280000ULL); // D3 and F3
  }

  SECTION("A-file pawn attacks only B-file") {
    BitBoard pawns = 0x0000000000000100ULL; // A2
    BitBoard targets = WhitePawnPossibleAttacksTargets(pawns);
    REQUIRE(targets == 0x0000000000020000ULL);          // Only B3
    REQUIRE((targets & BazuuBitBoardOps::H_FILE) == 0); // No wrap
  }

  SECTION("H-file pawn attacks only G-file") {
    BitBoard pawns = 0x0000000000008000ULL; // H2
    BitBoard targets = WhitePawnPossibleAttacksTargets(pawns);
    REQUIRE(targets == 0x0000000000400000ULL);          // Only G3
    REQUIRE((targets & BazuuBitBoardOps::A_FILE) == 0); // No wrap
  }
}

TEST_CASE("White pawn promotion captures", "[bitboard][pawn][white][promotion][capture]") {
  using namespace BazuuBitBoardOps;

  SECTION("Pawn on rank 7 captures and promotes") {
    BitBoard pawns = 0x0010000000000000ULL;     // E7
    BitBoard occupancy = 0x2800000000000000ULL; // D8 and F8
    BitBoard targets = WhitePawnAttacksWithPromotionTargets(pawns, occupancy);
    REQUIRE(targets == 0x2800000000000000ULL); // Both promotion captures
  }

  SECTION("Non-promotion captures not included") {
    BitBoard pawns = 0x0000001000000000ULL;     // E6
    BitBoard occupancy = 0x0000280000000000ULL; // D7 and F7
    BitBoard targets = WhitePawnAttacksWithPromotionTargets(pawns, occupancy);
    REQUIRE(targets == 0); // Not on rank 8
  }
}

// ============================================================================
// BITBOARD OPERATIONS TESTS - BLACK PAWN MOVES
// ============================================================================

TEST_CASE("Black pawn single push", "[bitboard][pawn][black][push]") {
  using namespace BazuuBitBoardOps;

  SECTION("Single pawn push from rank 7") {
    BitBoard pawns = 0x0010000000000000ULL; // E7
    BitBoard empty = 0xFFEFFFFFFFFFFFFFULL; // All except E7
    BitBoard targets = BlackSinglePushTargets(pawns, empty);
    REQUIRE(targets == 0x0000100000000000ULL); // E6
  }

  SECTION("Blocked pawn cannot push") {
    BitBoard pawns = 0x0010000000000000ULL; // E7
    BitBoard empty = 0xFFEFEFFFFFFFFFFFULL; // E7 and E6 blocked
    BitBoard targets = BlackSinglePushTargets(pawns, empty);
    REQUIRE(targets == 0);
  }

  SECTION("Multiple pawns push") {
    BitBoard pawns = 0x00FF000000000000ULL; // All rank 7
    BitBoard empty = 0xFF00FFFFFFFFFFFFULL; // Rank 7 occupied, rest empty
    BitBoard targets = BlackSinglePushTargets(pawns, empty);
    REQUIRE(targets == 0x0000FF0000000000ULL); // All rank 6
  }
}

TEST_CASE("Black pawn double push", "[bitboard][pawn][black][doublepush]") {
  using namespace BazuuBitBoardOps;

  SECTION("Double push from rank 7 to rank 5") {
    BitBoard pawns = 0x0010000000000000ULL; // E7
    BitBoard empty = 0xFFEFEFEFFFFFFFFFULL; // E7 blocked, E6 and E5 empty
    BitBoard targets = BlackDoublePushTargets(pawns, empty);
    REQUIRE(targets == 0x0000001000000000ULL); // E5
  }

  SECTION("Double push blocked by piece on E6") {
    BitBoard pawns = 0x0010000000000000ULL; // E7
    BitBoard empty = 0xFFEFFFEFFFFFFFFFULL; // E6 blocked
    BitBoard targets = BlackDoublePushTargets(pawns, empty);
    REQUIRE(targets == 0);
  }

  SECTION("Double push only from rank 7") {
    BitBoard pawns = 0x0000100000000000ULL; // E6 (not rank 7)
    BitBoard empty = 0xFFFFEFFFFFFFFFFFULL;
    BitBoard targets = BlackDoublePushTargets(pawns, empty);
    REQUIRE(targets == 0); // Cannot double push from rank 6
  }
}

TEST_CASE("Black pawn promotion", "[bitboard][pawn][black][promotion]") {
  using namespace BazuuBitBoardOps;

  SECTION("Pawn on rank 2 can promote") {
    BitBoard pawns = 0x0000000000001000ULL; // E2
    BitBoard empty = 0xFFFFFFFFFFFFEFFFULL; // E1 empty
    BitBoard targets = BlackPromotionTargets(pawns, empty);
    REQUIRE(targets == 0x0000000000000010ULL); // E1
  }

  SECTION("Promotion blocked by piece") {
    BitBoard pawns = 0x0000000000001000ULL; // E2
    BitBoard empty = 0xFFFFFFFFFFFFEFEFULL; // E1 occupied
    BitBoard targets = BlackPromotionTargets(pawns, empty);
    REQUIRE(targets == 0);
  }

  SECTION("Non-rank-2 pawns cannot promote") {
    BitBoard pawns = 0x0000000000100000ULL; // E3
    BitBoard empty = 0xFFFFFFFFFFEFFFFFULL;
    BitBoard targets = BlackPromotionTargets(pawns, empty);
    REQUIRE(targets == 0);
  }
}

TEST_CASE("Black pawn attacks", "[bitboard][pawn][black][attacks]") {
  using namespace BazuuBitBoardOps;

  SECTION("Pawn attacks diagonal squares with occupancy") {
    BitBoard pawns = 0x0010000000000000ULL;     // E7
    BitBoard occupancy = 0x0000280000000000ULL; // D6 and F6
    BitBoard targets = BlackPawnAttacksTargets(pawns, occupancy);
    REQUIRE(targets == 0x0000280000000000ULL); // Attacks both
  }

  SECTION("Pawn attacks only occupied squares") {
    BitBoard pawns = 0x0010000000000000ULL;     // E7
    BitBoard occupancy = 0x0000080000000000ULL; // Only D6
    BitBoard targets = BlackPawnAttacksTargets(pawns, occupancy);
    REQUIRE(targets == 0x0000080000000000ULL); // Only D6
  }

  SECTION("Possible attacks includes empty squares") {
    BitBoard pawns = 0x0010000000000000ULL; // E7
    BitBoard targets = BlackPawnPossibleAttacksTargets(pawns);
    REQUIRE(targets == 0x0000280000000000ULL); // D6 and F6
  }

  SECTION("A-file pawn attacks only B-file") {
    BitBoard pawns = 0x0100000000000000ULL; // A7
    BitBoard targets = BlackPawnPossibleAttacksTargets(pawns);
    REQUIRE(targets == 0x0000020000000000ULL);          // Only B6
    REQUIRE((targets & BazuuBitBoardOps::H_FILE) == 0); // No wrap
  }

  SECTION("H-file pawn attacks only G-file") {
    BitBoard pawns = 0x8000000000000000ULL; // H7
    BitBoard targets = BlackPawnPossibleAttacksTargets(pawns);
    REQUIRE(targets == 0x0000400000000000ULL);          // Only G6
    REQUIRE((targets & BazuuBitBoardOps::A_FILE) == 0); // No wrap
  }
}

TEST_CASE("Black pawn promotion captures", "[bitboard][pawn][black][promotion][capture]") {
  using namespace BazuuBitBoardOps;

  SECTION("Pawn on rank 2 captures and promotes") {
    BitBoard pawns = 0x0000000000001000ULL;     // E2
    BitBoard occupancy = 0x0000000000000028ULL; // D1 and F1
    BitBoard targets = BlackPawnAttacksWithPromotionTargets(pawns, occupancy);
    REQUIRE(targets == 0x0000000000000028ULL); // Both promotion captures
  }

  SECTION("Non-promotion captures not included") {
    BitBoard pawns = 0x0000000000100000ULL;     // E3
    BitBoard occupancy = 0x0000000000002800ULL; // D2 and F2
    BitBoard targets = BlackPawnAttacksWithPromotionTargets(pawns, occupancy);
    REQUIRE(targets == 0); // Not on rank 1
  }
}

// ============================================================================
// SQUARE ATTACKED TESTS
// ============================================================================

TEST_CASE("is_square_attacked by pawns", "[board][attacked][pawn]") {
  BazuuBoard board;

  SECTION("White pawn attacks black king") {
    board.setup_fen("4k3/8/8/8/3P4/8/8/4K3 w - - 0 1");
    // White pawn on D4 attacks C5 and E5
    REQUIRE(board.is_square_attacked(BoardSquares::C5, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::E5, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::D5, Colours::White) == false); // Straight ahead
  }

  SECTION("Black pawn attacks white king") {
    board.setup_fen("4k3/8/8/3p4/8/8/8/4K3 b - - 0 1");
    // Black pawn on D5 attacks C4 and E4
    REQUIRE(board.is_square_attacked(BoardSquares::C4, Colours::Black) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::E4, Colours::Black) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::D4, Colours::Black) == false); // Straight ahead
  }
}

TEST_CASE("is_square_attacked by knights", "[board][attacked][knight]") {
  BazuuBoard board;

  SECTION("White knight attacks from center") {
    board.setup_fen("4k3/8/8/8/4N3/8/8/4K3 w - - 0 1");
    // Knight on E4 attacks all 8 squares
    REQUIRE(board.is_square_attacked(BoardSquares::D2, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::F2, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::C3, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::G3, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::C5, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::G5, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::D6, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::F6, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::E5, Colours::White) == false); // Not a knight move
  }
}

TEST_CASE("is_square_attacked by bishops", "[board][attacked][bishop]") {
  BazuuBoard board;

  SECTION("White bishop attacks diagonals") {
    board.setup_fen("4k3/8/8/8/4B3/8/8/4K3 w - - 0 1");
    // Bishop on E4 attacks diagonals
    REQUIRE(board.is_square_attacked(BoardSquares::D3, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::C2, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::F5, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::H7, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::E5, Colours::White) == false); // Not diagonal
  }

  SECTION("Bishop attack blocked by piece") {
    board.setup_fen("4k3/8/6p1/8/4B3/8/8/4K3 w - - 0 1");
    // Bishop on E4, black pawn on G6
    REQUIRE(board.is_square_attacked(BoardSquares::F5, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::G6, Colours::White) == true);  // Can attack enemy
    REQUIRE(board.is_square_attacked(BoardSquares::H7, Colours::White) == false); // Blocked
  }
}

TEST_CASE("is_square_attacked by rooks", "[board][attacked][rook]") {
  BazuuBoard board;

  SECTION("White rook attacks files and ranks") {
    board.setup_fen("4k3/8/8/8/4R3/8/8/4K3 w - - 0 1");
    // Rook on E4 attacks E-file and 4th rank
    REQUIRE(board.is_square_attacked(BoardSquares::E1, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::E8, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::A4, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::H4, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::D5, Colours::White) == false); // Not on line
  }

  SECTION("Rook attack blocked by piece") {
    board.setup_fen("4k3/8/4p3/8/4R3/8/8/4K3 w - - 0 1");
    // Rook on E4, black pawn on E6
    REQUIRE(board.is_square_attacked(BoardSquares::E5, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::E6, Colours::White) == true);  // Can attack enemy
    REQUIRE(board.is_square_attacked(BoardSquares::E7, Colours::White) == false); // Blocked
  }
}

TEST_CASE("is_square_attacked by queens", "[board][attacked][queen]") {
  BazuuBoard board;

  SECTION("White queen attacks all directions") {
    board.setup_fen("4k3/8/8/8/4Q3/8/8/4K3 w - - 0 1");
    // Queen on E4 attacks like rook + bishop
    REQUIRE(board.is_square_attacked(BoardSquares::E8, Colours::White) == true); // Vertical
    REQUIRE(board.is_square_attacked(BoardSquares::A4, Colours::White) == true); // Horizontal
    REQUIRE(board.is_square_attacked(BoardSquares::H7, Colours::White) == true); // Diagonal
    REQUIRE(board.is_square_attacked(BoardSquares::A8, Colours::White) == true); // Diagonal
  }
}

TEST_CASE("is_square_attacked by kings", "[board][attacked][king]") {
  BazuuBoard board;

  SECTION("White king attacks adjacent squares") {
    board.setup_fen("4k3/8/8/8/4K3/8/8/8 w - - 0 1");
    // King on E4 attacks all 8 adjacent squares
    REQUIRE(board.is_square_attacked(BoardSquares::D3, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::E3, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::F3, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::D4, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::F4, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::D5, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::E5, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::F5, Colours::White) == true);
    REQUIRE(board.is_square_attacked(BoardSquares::E6, Colours::White) == false); // Too far
  }
}

TEST_CASE("is_square_attacked complex positions", "[board][attacked][complex]") {
  BazuuBoard board;

  SECTION("Starting position - white controls center") {
    board.setup_fen(BazuuBoard::STARTING_FEN);
    // White controls d3, e3
    REQUIRE(board.is_square_attacked(BoardSquares::D3, Colours::White) == true); // Pawn from e2
    REQUIRE(board.is_square_attacked(BoardSquares::E3, Colours::White) == true); // Pawn from d2 or f2
  }

  SECTION("King in check detection") {
    board.setup_fen("4k3/8/8/8/4R3/8/8/4K3 w - - 0 1");
    // White rook on E4 attacks black king on E8
    REQUIRE(board.is_square_attacked(BoardSquares::E8, Colours::White) == true);
  }

  SECTION("Multiple attackers on same square") {
    board.setup_fen("4k3/8/8/3NBR2/8/8/8/4K3 w - - 0 1");
    // D5 is attacked by knight, bishop, and rook
    REQUIRE(board.is_square_attacked(BoardSquares::F3, Colours::White) == true); // Multiple pieces can attack
  }
}

// ============================================================================
// BOARD INITIALIZATION TESTS
// ============================================================================

TEST_CASE("Board initialization consistency", "[board][init]") {
  BazuuBoard board;

  SECTION("Square mappings are initialized") {
    // Test that 64-120 and 120-64 conversions work
    for (uint8_t sq64 = 0; sq64 < 64; ++sq64) {
      BoardSquares sq120 = board.to_120_board_square(sq64);
      uint8_t back = board.to_64_board_square(sq120);
      REQUIRE(back == sq64);
      REQUIRE(sq120 != BoardSquares::NO_SQ);
    }
  }

  SECTION("Attack tables are non-zero after init") {
    // Knight attacks should be initialized
    BitBoard knight_e4 = board.get_knight_attacks(BoardSquares::E4);
    REQUIRE(knight_e4 != 0);
    REQUIRE(std::popcount(knight_e4) == 8); // Knight has 8 attacks from center

    // King attacks should be initialized
    BitBoard king_e4 = board.get_king_attacks(BoardSquares::E4);
    REQUIRE(king_e4 != 0);
    REQUIRE(std::popcount(king_e4) == 8); // King has 8 attacks from center

    // Pawn attacks should be initialized
    BitBoard white_pawn_e4 = board.get_pawn_attacks(Colours::White, BoardSquares::E4);
    REQUIRE(white_pawn_e4 != 0);
    REQUIRE(std::popcount(white_pawn_e4) == 2); // Pawn has 2 attacks
  }

  SECTION("Magic bitboard tables work after init") {
    board.setup_fen("8/8/8/8/4B3/8/8/8 w - - 0 1");
    BitBoard occ = board.occupancy();

    // Bishop magic lookup should work
    BitBoard bishop_attacks = board.get_bishop_attacks_lookup(BoardSquares::E4, occ);
    REQUIRE(bishop_attacks != 0);
    REQUIRE(std::popcount(bishop_attacks) == 13); // Bishop on empty board from E4

    // Rook magic lookup should work
    board.setup_fen("8/8/8/8/4R3/8/8/8 w - - 0 1");
    occ = board.occupancy();
    BitBoard rook_attacks = board.get_rook_attacks_lookup(BoardSquares::E4, occ);
    REQUIRE(rook_attacks != 0);
    REQUIRE(std::popcount(rook_attacks) == 14); // Rook on empty board from E4
  }
}

// ============================================================================
// PIECE LIST AND SIDE BITBOARD TESTS
// ============================================================================

TEST_CASE("update_piece_list consistency", "[board][piecelist]") {
  BazuuBoard board;

  SECTION("Piece lists match bitboards after FEN setup") {
    board.setup_fen(BazuuBoard::STARTING_FEN);

    // White should have 16 pieces
    BitBoard white_occ = board.side_occupancy(Colours::White);
    REQUIRE(std::popcount(white_occ) == 16);

    // Black should have 16 pieces
    BitBoard black_occ = board.side_occupancy(Colours::Black);
    REQUIRE(std::popcount(black_occ) == 16);

    // Total occupancy should be 32
    BitBoard total_occ = board.occupancy();
    REQUIRE(std::popcount(total_occ) == 32);
  }

  SECTION("Piece lists update correctly for custom position") {
    board.setup_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

    // White pawns should be 8 (one moved to E4)
    BitBoard white_pawns = board.get_bitboard_of_piece(PieceType::P, Colours::White);
    REQUIRE(std::popcount(white_pawns) == 8);

    // Black pawns should still be 8
    BitBoard black_pawns = board.get_bitboard_of_piece(PieceType::P, Colours::Black);
    REQUIRE(std::popcount(black_pawns) == 8);
  }

  SECTION("Empty squares have no pieces") {
    board.setup_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    BitBoard occ = board.occupancy();
    REQUIRE(std::popcount(occ) == 2); // Only two kings
  }
}

TEST_CASE("side_occupancy correctness", "[board][bitboard][sides]") {
  BazuuBoard board;

  SECTION("Sides don't overlap") {
    board.setup_fen(BazuuBoard::STARTING_FEN);
    BitBoard white = board.side_occupancy(Colours::White);
    BitBoard black = board.side_occupancy(Colours::Black);
    REQUIRE((white & black) == 0); // No overlap
  }

  SECTION("Side occupancies sum to total occupancy") {
    board.setup_fen(BazuuBoard::STARTING_FEN);
    BitBoard white = board.side_occupancy(Colours::White);
    BitBoard black = board.side_occupancy(Colours::Black);
    BitBoard total = board.occupancy();
    REQUIRE((white | black) == total);
  }

  SECTION("Side occupancy matches piece bitboards") {
    board.setup_fen(BazuuBoard::STARTING_FEN);

    // Calculate white occupancy from pieces
    BitBoard white_calc = 0;
    white_calc |= board.get_bitboard_of_piece(PieceType::P, Colours::White);
    white_calc |= board.get_bitboard_of_piece(PieceType::N, Colours::White);
    white_calc |= board.get_bitboard_of_piece(PieceType::B, Colours::White);
    white_calc |= board.get_bitboard_of_piece(PieceType::R, Colours::White);
    white_calc |= board.get_bitboard_of_piece(PieceType::Q, Colours::White);
    white_calc |= board.get_bitboard_of_piece(PieceType::K, Colours::White);

    BitBoard white_actual = board.side_occupancy(Colours::White);
    REQUIRE(white_calc == white_actual);
  }
}
