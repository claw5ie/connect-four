#include <iostream>
#include <cassert>
#include <limits>

enum GameState
{
  NOT_OVER,
  DRAW,
  O_WIN,
  X_WIN,
  GAME_STATE_COUNT
};

struct Board
{
  struct Status
  {
    GameState state;
    int32_t score;
  };

  int8_t player;
  uint8_t free[7];
  int8_t data[7][6];

  void remove_at(size_t column)
  {
    assert(column < 7);

    if (free[column] > 0)
    {
      free[column]--;
      player = !player;
    }
  }

  bool insert_at(size_t column)
  {
    assert(column < 7);

    if (free[column] < 6)
    {
      data[column][free[column]++] = player;
      player = !player;

      return true;
    }
    else
    {
      return false;
    }
  }

  Status score() const
  {
    static int32_t const values[] = { 0, 1, 10, 50 };

    static int32_t const dirs[4][2] = {
      { 1, -1 },
      { 1,  0 },
      { 1,  1 },
      { 0,  1 }
    };

    int32_t score = 0;

    for (size_t i = 0; i < 7; i++)
    {
      for (size_t j = 0; j < 6; j++)
      {
        for (size_t k = 0; k < 4; k++)
        {
          size_t zeroes = ((data[i][j] == 0) && j < free[i]),
            ones = ((data[i][j] == 1) && j < free[i]);

          int32_t x = i,
            y = j;

          for (size_t s = 0; s < 3; s++)
          {
            x += dirs[k][0];
            y += dirs[k][1];

            if (x >= 0 && x < 7 && y >= 0 && y < free[x])
            {
              zeroes += (data[x][y] == 0);
              ones += (data[x][y] == 1);
            }
          }

          if (zeroes == 4)
            return { O_WIN, -512 };
          else if (ones == 4)
            return { X_WIN, 512 };
          else if (x >= 0 && x < 7 && y >= 0 && y < 6)
          {
            if (zeroes == 0)
              score += values[ones];
            else if (ones == 0)
              score -= values[zeroes];
          }
        }
      }
    }

    for (size_t i = 0; i < 7; i++)
    {
      if (free[i] < 7)
        return { NOT_OVER, score };
    }

    return { DRAW, score };
  }

  void print() const
  {
    for (size_t j = 0; j < 6; j++)
    {
      for (size_t i = 0; i < 7; i++)
      {
        size_t row = 5 - j;
        char ch = data[i][row];

        std::cout << (row < free[i] ? (ch ? 'X' : 'O') : '-')
                  << (i + 1 < 7 ? ' ' : '\n');
      }
    }
  }
};

char const *to_string(GameState status)
{
  static char const *const lookup[] = { "Not over", "Draw", "O won", "X won" };

  return status < GAME_STATE_COUNT ? lookup[status] : nullptr;
}

#define INVALID_MOVE (std::numeric_limits<int32_t>::max())

struct Move
{
  uint32_t move;
  Board::Status status;
};

Move max_aux(Board &, size_t);

Move min_aux(Board &board, size_t depth)
{
  Move min = { INVALID_MOVE, { DRAW, std::numeric_limits<int32_t>::max() } };

  if (depth == 0)
    return min;

  for (uint32_t i = 0; i < 7; i++)
  {
    if (!board.insert_at(i))
      continue;

    auto score = board.score();
    if (min.status.score > score.score)
      min = { i, score };

    auto max = max_aux(board, depth - 1);
    if (min.status.score > max.status.score)
      min = { i, { score.state, max.status.score } };

    board.remove_at(i);
  }

  return min;
}

Move max_aux(Board &board, size_t depth)
{
  Move max = {
    INVALID_MOVE, { DRAW, std::numeric_limits<int32_t>::lowest() }
  };

  if (depth == 0)
    return max;

  for (uint32_t i = 0; i < 7; i++)
  {
    if (!board.insert_at(i))
      continue;

    auto score = board.score();
    if (max.status.score < score.score)
      max = { i, score };

    auto min = max_aux(board, depth - 1);
    if (max.status.score < min.status.score)
      max = { i, { score.state, min.status.score } };

    board.remove_at(i);
  }

  return max;
}

Move minimax(Board board, size_t max_depth)
{
  return min_aux(board, max_depth);
}

Move maximin(Board board, size_t max_depth)
{
  return max_aux(board, max_depth);
}

int main()
{
  Board board = { };

  board.print();

  do
  {
    auto move = board.player ? minimax(board, 8) : maximin(board, 8);

    if (move.move >= 7)
    {
      std::cout << "Invalid column number. It should be no greater than 6.\n";
      break;
    }

    board.insert_at(move.move);
    board.print();

    if (move.status.state != NOT_OVER)
    {
      std::cout << "Status: " << to_string(move.status.state) << '\n';
      break;
    }

  } while (true);

  return 0;
}
