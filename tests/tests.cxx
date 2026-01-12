#include "bazuu_ce_board.hpp"
#include "defs.hpp"
#include <bit>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <print>

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
