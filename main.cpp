#include <iostream>
#include <cassert>

struct Board
{
  int8_t player;
  uint8_t free[7];
  int8_t data[7][6];

  int8_t place_at(size_t column)
  {
    assert(column < 7);

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

    if (free[column] < 6)
    {
      size_t row = free[column];

      data[column][free[column]] = player;
      free[column]++;
      player = !player;

      char state = 0;

      for (int32_t s = 1; s <= 3; s++)
      {
        for (int32_t i = 0; i < 8; i++)
        {
          int32_t const y = (int32_t)row + s * dirs[i][0],
            x = (int32_t)column + s * dirs[i][1];

          state = state & (
            y >= 0 && y < 7 &&
            x >= 0 && x < 6 &&
            data[x][y] != data[column][row]
            ) >> i;
        }
      }

      return ~state ? data[column][row] : -1;
    }
    else
    {
      std::cerr << "error: invalid move: the column "
                << column
                << " is already full.\n";
    }
  }

  size_t score() const
  {
    return 0;
  }
};


int main()
{
  std::cout << sizeof(Board) << std::endl;

  return 0;
}
