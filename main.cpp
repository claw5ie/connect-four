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

  bool is_over() const
  {
    for (size_t i = 0; i < 7; i++)
    {
      if (free[i] < 6)
        return false;
    }

    return true;
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

    return { is_over() ? DRAW : NOT_OVER, score };
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
  int32_t score;
};

Move minimax_aux(Board &board, size_t depth)
{
  if (depth == 0 || board.is_over())
    return { INVALID_MOVE, board.score().score };

  Move result = { INVALID_MOVE,
                  board.player ? std::numeric_limits<int32_t>::lowest() :
                    std::numeric_limits<int32_t>::max() };

  for (uint32_t i = 0; i < 7; i++)
  {
    if (!board.insert_at(i))
      continue;

    auto score = minimax_aux(board, depth - 1).score;

    if (
      (board.player && result.score > score) ||
      (!board.player && result.score < score)
      )
    {
      result = { i, score };
    }

    board.remove_at(i);
  }

  return result;
}

Move alpha_beta_aux(Board &board, int32_t alpha, int32_t beta, size_t depth)
{
  if (depth == 0 || board.is_over())
    return { INVALID_MOVE, board.score().score };

  Move result = { INVALID_MOVE,
                  board.player ? std::numeric_limits<int32_t>::lowest() :
                    std::numeric_limits<int32_t>::max() };

  for (uint32_t i = 0; i < 7; i++)
  {
    if (!board.insert_at(i))
      continue;

    auto score = alpha_beta_aux(board, alpha, beta, depth - 1).score;

    if (board.player)
    {
      if (result.score > score)
        result = { i, score };

      beta = std::min(beta, score);
    }
    else
    {
      if (result.score < score)
        result = { i, score };

      alpha = std::max(alpha, score);
    }

    board.remove_at(i);

    if (beta <= alpha)
      break;
  }

  return result;
}

int32_t minimax(Board board, size_t max_depth)
{
  return minimax_aux(board, max_depth).move;
}

int32_t alpha_beta(Board board, size_t max_depth)
{
  return alpha_beta_aux(
    board,
    std::numeric_limits<int32_t>::lowest(),
    std::numeric_limits<int32_t>::max(),
    max_depth
    ).move;
}

int main()
{
  Board board = { };

  board.print();

  do
  {
    auto move = minimax(board, board.player ? 8 : 8);

    if (move >= 7)
    {
      std::cout << "Invalid column number. It should be no greater than 6.\n";
      break;
    }

    board.insert_at(move);
    board.print();
    auto status = board.score().state;

    if (status != NOT_OVER)
    {
      std::cout << "Status: " << to_string(status) << '\n';
      break;
    }

  } while (true);

  return 0;
}
