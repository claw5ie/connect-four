#include <iostream>
#include <cstring>
#include <cassert>

#include "Board.hpp"

void Board::remove_at(MoveType column)
{
  assert(column < COLUMNS);

  if (top[column] > 0)
  {
    top[column]--;
    player = !player;
  }
}

bool Board::insert_at(MoveType column)
{
  assert(column < COLUMNS);

  if (top[column] < ROWS)
  {
    data[column][top[column]++] = player;
    player = !player;

    return true;
  }
  else
  {
    return false;
  }
}

bool Board::is_over() const
{
  for (size_t i = 0; i < COLUMNS; i++)
  {
    if (top[i] < ROWS)
      return false;
  }

  return true;
}

MoveType Board::choose_random_move() const
{
  static MoveType moves[COLUMNS];

  size_t count = 0;
  for (MoveType i = 0; i < COLUMNS; i++)
  {
    if (top[i] < ROWS)
      moves[count++] = i;
  }

  return count == 0 ? INVALID_MOVE : moves[rand() % count];
}

Board::Status Board::score() const
{
  static ScoreType const values[] = { 0, 1, 10, 50 };

  static int32_t const dirs[4][2] = {
    { 1, -1 },
    { 1,  0 },
    { 1,  1 },
    { 0,  1 }
  };

  ScoreType score = 0;

  for (size_t i = 0; i < COLUMNS; i++)
  {
    for (size_t j = 0; j < ROWS; j++)
    {
      for (size_t k = 0; k < 4; k++)
      {
        size_t zeroes = !data[i][j] && j < top[i],
          ones = (data[i][j] != 0) && j < top[i];

        int32_t x = i,
          y = j;

        for (size_t s = 0; s < 3; s++)
        {
          x += dirs[k][0];
          y += dirs[k][1];

          if (x >= 0 && x < COLUMNS && y >= 0 && y < top[x])
          {
            zeroes += !data[x][y];
            ones += (data[x][y] != 0);
          }
        }

        if (zeroes == 4)
          return { O_WIN, -512 };
        else if (ones == 4)
          return { X_WIN, 512 };
        else if (x >= 0 && x < COLUMNS && y >= 0 && y < ROWS)
        {
          if (zeroes == 0)
            score += values[ones];
          else if (ones == 0)
            score -= values[zeroes];
        }
      }
    }
  }

  if (is_over())
    return { DRAW, 0 };
  else
    return { NOT_OVER, score + (player ? 16 : -16) };
}

void Board::read_board(char const *board)
{
  if (std::strlen(board) != COLUMNS * ROWS)
  {
    std::cerr << "error: incorrect dimension of the board.\n";
    exit(EXIT_FAILURE);
  }

  for (size_t j = 0; j < COLUMNS; j++)
  {
    top[j] = ROWS;

    for (size_t i = ROWS; i-- > 0; )
    {
      char ch = board[i * COLUMNS + j];

      data[j][ROWS - 1 - i] = (ch == 'x' || ch == 'X');

      if (ch == 'b' || ch == 'B')
      {
        top[j] = ROWS - 1 - i;
        break;
      }
    }
  }
}

void Board::print(size_t offset) const
{
  for (size_t j = 0; j < ROWS; j++)
  {
    for (size_t k = 0; k < offset; k++)
      std::cout << ' ';

    for (size_t i = 0; i < COLUMNS; i++)
    {
      size_t row = ROWS - 1 - j;

      std::cout << (row < top[i] ? (data[i][row] ? 'X' : 'O') : '-')
                << (i + 1 < COLUMNS ? ' ' : '\n');
    }
  }
}

char const *to_string(Board::GameState status)
{
  static char const *const lookup[] = { "Not over", "Draw", "O won", "X won" };

  return status < Board::GAME_STATE_COUNT ? lookup[status] : nullptr;
}
