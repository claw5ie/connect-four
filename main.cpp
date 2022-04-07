#include <iostream>
#include <cassert>

struct Board
{
  int8_t player;
  uint8_t free[7];
  int8_t data[7][6];

  void init()
  {
    for (size_t i = 0; i < 7 * 6; i++)
      ((int8_t *)data)[i] = -1;
  }

  void place_at(size_t column)
  {
    assert(column < 7);

    if (free[column] < 6)
    {
      data[column][free[column]++] = player;
      player = !player;
    }
    else
    {
      std::cerr << "error: invalid move: the column "
                << column
                << " is already full.\n";
    }
  }

  int32_t score() const
  {
    static int32_t const values[] = { 0, 1, 10, 50 };

    static int32_t const dirs[8][2] = {
      {  1,  0 },
      {  1,  1 },
      {  0,  1 },
      { -1,  1 },
      { -1,  0 },
      { -1, -1 },
      {  0, -1 },
      {  1, -1 }
    };

    int32_t score = 0;

    for (size_t i = 0; i < 7; i++)
    {
      for (size_t j = 0; j < 6; j++)
      {
        for (int32_t k = 0; k < 8; k++)
        {
          size_t zeroes = data[i][j] == 0,
            ones = data[i][j] == 1;

          int32_t x = i,
            y = j;

          for (int32_t s = 0; s < 3; s++)
          {
            x += dirs[k][0];
            y += dirs[k][1];

            if (x >= 0 && x < 7 && y >= 0 && y < free[x])
            {
              zeroes += (data[x][y] == 0);
              ones += (data[x][y] == 1);
            }
          }

          if (x >= 0 && x < 7 && y >= 0 && y < 6)
          {
            if (zeroes == 0)
              score += values[ones];
            else if (ones == 0)
              score -= values[zeroes];
          }

          if (zeroes == 4)
            return -512;
          else if (ones == 4)
            return 512;
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
        char ch = data[i][5 - j];
        std::cout << (ch != -1 ? (ch ? 'X' : 'O') : '-')
                  << (i + 1 < 7 ? ' ' : '\n');
      }
    }
  }
};


int main()
{
  Board board = { };

  board.init();
  board.print();

  auto score = 0;

  do
  {
    size_t column = 0;
    std::cout << "where to place? ";
    std::cin >> column;

    board.place_at(column);
    board.print();
    score = board.score();

    std::cout << "Score: " << score << '\n';
  } while (score != 512 && score != -512 && !std::cin.eof());

  return 0;
}
