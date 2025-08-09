#include <bazuu_ce_board.hpp>
#include <catch2/catch_test_macros.hpp>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>

TEST_CASE("BazuuBoard can convert file/rank to square on 120 board", "[file_rank_to_120_board]") {
  BazuuBoard board;
  REQUIRE(board.file_rank_to_120_board(File::A, Rank::R1) == BoardSquares::A1);
  REQUIRE(board.file_rank_to_120_board(File::H, Rank::R6) == BoardSquares::H6);
  REQUIRE(board.file_rank_to_120_board(File::B, Rank::R4) == BoardSquares::B4);
  REQUIRE(board.file_rank_to_120_board(File::D, Rank::R7) == BoardSquares::D7);
}
