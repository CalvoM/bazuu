# bazuu

[![Testing chess engine](https://github.com/CalvoM/bazuu/actions/workflows/ci-test.yml/badge.svg)](https://github.com/CalvoM/bazuu/actions/workflows/ci-test.yml)

Hobby chess engine, because why not?

## Roadmap from Oct 2025

I have noticed I am now drawn to many other projects, so I need to make sure I have a clear roadmap of tasks to be done.
I have used ChatGPT to generate the next steps.

## 📅 4-Week Chess Engine Plan (Weekend-Focused)

### Week 1 — Move Generation Core

- **Precompute attack tables**
  - Knight/king attacks (simple bit shifts + masks)
  - Sliding rays (rook/bishop/queen) using either magic bitboards or simpler occupancy-based lookup
  - Pawn moves: single push, double push, captures, en passant

- **Implement move encoding format**
  - Pack move into an integer: from, to, piece, promotion, flags

- **Generate pseudo-legal moves**
  - Pawns (with promotions & en passant)
  - Knights, bishops, rooks, queen, king
  - Castling moves (check castling rights, occupancy, attacks)

👉 **End of Week 1:** Be able to generate all pseudo-legal moves for any position and print them.

---

### Week 2 — Make & Unmake Moves

- **Implement make_move**
  - Update bitboards, side-to-move, castling rights, en passant, halfmove clock
  - Update Zobrist hash incrementally

- **Implement unmake_move**  
  - Critical for search

- **Add move legality check**  
  - Filter out moves that leave king in check

- **Write perft testing framework**
  - Compare against known results (e.g., perft(1) = 20, perft(2) = 400)

👉 **End of Week 2:** Pass perft tests at least to depth 4–5.

---

### Week 3 — Search

- **Implement negamax with alpha-beta pruning**

- **Add a simple evaluation function**
  - Material balance
  - Piece-square tables for positional bias

- **Add search limits**
  - Depth, time

- **Implement iterative deepening (optional, but very useful)**

👉 **End of Week 3:** Engine can play legal chess with basic strategy.

---

### Week 4 — UCI & Polishing

- **Implement UCI protocol**
  - Connect Bazuu to GUI (e.g., Arena, CuteChess)

- **Add transposition table**
  - Use Zobrist hash as key

- **Add quiescence search**
  - Helps reduce horizon effect

- **Test against other small engines**

- **Optimize**
  - Bitboards and hot paths

👉 **End of Week 4:** Bazuu is a working chess engine you can actually play against.
