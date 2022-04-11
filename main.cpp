#include <iostream>
#include <cassert>

struct Board
{
  int8_t player;
  uint8_t free[7];
  int8_t data[7][6];

  bool place_at(size_t column)
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

  int32_t score() const
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
            return -512;
          else if (ones == 4)
            return 512;
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

    return score;
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


int main()
{
  Board board = { };

  board.print();

  auto score = 0;

  do
  {
    size_t column = 0;
    std::cout << "where to place? ";
    std::cin >> column;

    if (column >= 7)
    {
      std::cout << "Invalid column number. It should be no greater than 6.\n";
    }
    else if (!board.place_at(column))
    {
      std::cout << "Column " << column << " is already full! Try again with different column.\n";
    }
    else
    {
      board.print();
      score = board.score();
      std::cout << "Score: " << score << '\n';
    }
  } while (score != 512 && score != -512 && !std::cin.eof());

  return 0;
}
