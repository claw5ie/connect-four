#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstdint>
#include <cstddef>
#include <limits>

using MoveType = uint32_t;
using ScoreType = int32_t;

#define INVALID_MOVE (std::numeric_limits<MoveType>::max())
#define COLUMNS 7
#define ROWS 6

struct Board
{
  enum GameState
  {
    NOT_OVER,
    DRAW,
    O_WIN,
    X_WIN,
    GAME_STATE_COUNT
  };

  struct Status
  {
    GameState state;
    ScoreType score;
  };

  int8_t player;
  uint8_t top[COLUMNS];
  int8_t data[COLUMNS][ROWS];

  void remove_at(MoveType column);

  bool insert_at(MoveType column);

  bool is_over() const;

  MoveType choose_random_move() const;

  Status score() const;

  void read_board(char const *board);

  void print(size_t offset = 0) const;
};

char const *to_string(Board::GameState status);

#endif // BOARD_HPP
