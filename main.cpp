#include <iostream>
#include <cassert>
#include <limits>
#include <cmath>
#include <cstring>

using MoveType = uint32_t;
using ScoreType = int32_t;

#define INVALID_MOVE (std::numeric_limits<MoveType>::max())
#define LOWEST_SCORE (std::numeric_limits<ScoreType>::lowest())
#define GREATEST_SCORE (std::numeric_limits<ScoreType>::max())

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

  void remove_at(MoveType column)
  {
    assert(column < COLUMNS);

    if (top[column] > 0)
    {
      top[column]--;
      player = !player;
    }
  }

  bool insert_at(MoveType column)
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

  bool is_over() const
  {
    for (size_t i = 0; i < COLUMNS; i++)
    {
      if (top[i] < ROWS)
        return false;
    }

    return true;
  }

  MoveType choose_random_move() const
  {
    static MoveType moves[COLUMNS];

    size_t count = 0;
    for (MoveType i = 0; i < COLUMNS; i++)
    {
      moves[count] = i;
      count += (top[i] < ROWS);
    }

    return count == 0 ? INVALID_MOVE : moves[rand() % count];
  }

  Status score() const
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

    return { is_over() ? DRAW : NOT_OVER, score };
  }

  void read_board(char const *board)
  {
    assert(std::strlen(board) == COLUMNS * ROWS);

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

  void print(size_t offset = 0) const
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
};

char const *to_string(Board::GameState status)
{
  static char const *const lookup[] = { "Not over", "Draw", "O won", "X won" };

  return status < Board::GAME_STATE_COUNT ? lookup[status] : nullptr;
}

struct MinimaxData
{
  MoveType move;
  ScoreType score;
};

MinimaxData minimax_aux(Board &board, size_t depth)
{
  if (depth == 0 || board.is_over())
    return { INVALID_MOVE, board.score().score };

  MinimaxData result = {
    INVALID_MOVE, board.player ? LOWEST_SCORE : GREATEST_SCORE
  };

  for (MoveType i = 0; i < COLUMNS; i++)
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

MinimaxData alpha_beta_aux(
  Board &board, ScoreType alpha, ScoreType beta, size_t depth
  )
{
  if (depth == 0 || board.is_over())
    return { INVALID_MOVE, board.score().score };

  MinimaxData result = {
    INVALID_MOVE, board.player ? LOWEST_SCORE : GREATEST_SCORE
  };

  for (MoveType i = 0; i < COLUMNS; i++)
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

MoveType minimax(Board board, size_t max_depth)
{
  return minimax_aux(board, max_depth).move;
}

MoveType alpha_beta(Board board, size_t max_depth)
{
  return alpha_beta_aux(
    board, LOWEST_SCORE, GREATEST_SCORE, max_depth
    ).move;
}

MoveType monte_carlo_tree_search(Board const &board, size_t max_iters)
{
  struct MonteCarloTree
  {
    struct Node
    {
      Board board;
      Node *parent;
      Node *children;
      MoveType move;
      size_t count;
      uint32_t wins;
      uint32_t visits;
    };

    Node root;

    MonteCarloTree(Board const &board)
      : root({ board, &root, nullptr, INVALID_MOVE, 0, 0, 0 })
    {
    }

    Node *find_best_leaf()
    {
      Node *current = &root;

      while (current->count > 0)
        current = choose_best_child(current);

      return current;
    }

    Node *choose_best_child(Node *parent)
    {
      assert(parent->count > 0);

      double best_ucb = std::numeric_limits<double>::lowest();
      Node *best_child = nullptr;

      for (size_t i = 0; i < parent->count; i++)
      {
        Node &child = parent->children[i];

        if (child.visits == 0)
          return &child;

        double const score = (double)child.wins / child.visits +
          std::sqrt(2 * std::log(parent->visits) / child.visits);

        if (best_ucb < score)
        {
          best_ucb = score;
          best_child = &child;
        }
      }

      return best_child;
    }

    bool rollout(Node *leaf)
    {
      assert(leaf->count == 0);

      if (leaf->visits > 0)
      {
        leaf = append_leaves(leaf);

        if (leaf == nullptr)
          return false;
      }

      Board board = leaf->board;

      while (!board.is_over())
        board.insert_at(board.choose_random_move());

      auto win = board.score().state;

      backpropagate(
        leaf,
        root.board.player ?
          win == Board::X_WIN : win == Board::O_WIN
        );

      return true;
    }

    Node *append_leaves(Node *leaf)
    {
      static MoveType moves[COLUMNS];

      size_t count = 0;
      for (MoveType i = 0; i < COLUMNS; i++)
      {
        if (leaf->board.top[i] < ROWS)
          moves[count++] = i;
      }

      if (count == 0)
        return nullptr;

      leaf->children = new Node[count]{ };
      leaf->count = count;

      for (size_t i = 0; i < count; i++)
      {
        Node &node = leaf->children[i];
        node.board = leaf->board;
        node.board.insert_at(moves[i]);
        node.parent = leaf;
        node.move = moves[i];
      }

      return leaf->children;
    }

    void backpropagate(Node *leaf, bool won)
    {
      while (leaf->parent != leaf)
      {
        leaf->wins += won;
        leaf->visits++;
        leaf = leaf->parent;
      }

      leaf->wins += won;
      leaf->visits++;
    }

    ~MonteCarloTree()
    {
      clean(root);
    }

  private:
    void clean(Node const &node) const
    {
      for (size_t i = 0; i < node.count; i++)
        clean(node.children[i]);

      delete[] node.children;
    }
  };

  MonteCarloTree tree{ board };

  while (max_iters-- > 0)
  {
    if (!tree.rollout(tree.find_best_leaf()))
      break;
  }

  return tree.choose_best_child(&tree.root)->move;
}

#define INVALID_OPTION (std::numeric_limits<size_t>::max())

int main(int argc, char **argv)
{
  struct
  {
    char short_opt;
    char const *long_opt;
    bool has_arg;
  } const option_list[] = {
    { 'o', "o-algorithm", true },
    { 'x', "x-algorithm", true },
    { '\0', "o-depth", true },
    { '\0', "x-depth", true },
    { '\0', "o-max-iter", true },
    { '\0', "x-max-iter", true },
    { 'b', "board", true },
    { 's', "show", false },
    { 'p', "player", true }
  };

  char const *const algorithms[] = {
    "minimax", "alpha-beta", "monte-carlo"
  };

  auto const find_option =
    [option_list](char const *arg) -> size_t
    {
      if (
        !(arg[0] == '-' &&
          ((std::isalpha(arg[1]) && arg[2] == '\0') ||
           (arg[1] == '-' && std::isalpha(arg[2]))))
        )
      {
        return INVALID_OPTION;
      }

      for (size_t i = 0; i < sizeof(option_list) / sizeof(*option_list); i++)
      {
        auto &option = option_list[i];

        if (
          arg[1] == option.short_opt ||
          (option.long_opt != nullptr &&
           !std::strcmp(arg + 2, option.long_opt))
          )
        {
          return i;
        }
      }

      return INVALID_OPTION;
    };

  enum Algorithm
    {
      MINIMAX,
      ALPHA_BETA,
      MONTE_CARLO,
      ALGORITHM_COUNT
    };

  auto const what_algorithm =
    [algorithms](char const *arg) -> Algorithm
    {
      for (size_t i = 0; i < ALGORITHM_COUNT; i++)
      {
        if (!strcmp(arg, algorithms[i]))
          return (Algorithm)i;
      }

      return ALGORITHM_COUNT;
    };

  auto const alg_to_string =
    [algorithms](Algorithm alg) -> char const *
    {
      return alg < ALGORITHM_COUNT ? algorithms[alg] : nullptr;
    };

  Board board = { };
  Algorithm o_alg = MINIMAX,
    x_alg = ALPHA_BETA;
  size_t o_depth = 4,
    x_depth = 8;
  size_t o_max_iter = 50000,
    x_max_iter = 200000;
  bool should_show = false;

  for (size_t i = 1; i < (size_t)argc; i++)
  {
    size_t const option = find_option(argv[i]);

    if (option == INVALID_OPTION)
    {
      std::cerr << "error: unrecognized option.\n";

      return EXIT_FAILURE;
    }
    else if (option_list[option].has_arg && ++i >= (size_t)argc)
    {
      std::cerr << "error: excepted argument for the option.\n";

      return EXIT_FAILURE;
    }

    switch (option)
    {
    case 0:
    case 1:
    {
      auto alg = what_algorithm(argv[i]);

      if (alg < ALGORITHM_COUNT)
      {
        if (option == 0)
          o_alg = alg;
        else
          x_alg = alg;
      }
      else
      {
        std::cerr << "error: unrecognized algorithm.\n";

        return EXIT_FAILURE;
      }

      break;
    }
    case 2:
    case 3:
    {
      uint32_t depth = std::strtoul(argv[i], nullptr, 10);

      if (option == 2)
        o_depth = depth;
      else
        x_depth = depth;

      break;
    }
    case 4:
    case 5:
    {
      uint32_t iter = std::strtoul(argv[i], nullptr, 10);

      if (option == 4)
        o_max_iter = iter;
      else
        x_max_iter = iter;

      break;
    }
    case 6:
      board.read_board(argv[i]);
      break;
    case 7:
      should_show = true;
      break;
    case 8:
    {
      char ch = argv[i][0];

      if (argv[i][1] != '\0' || (ch != 'x' && ch != 'X' && ch != 'o' && ch != 'O'))
      {
        std::cerr << "error: invalid player.\n";

        return EXIT_FAILURE;
      }

      board.player = (ch == 'x' || ch == 'X');
      break;
    }
    default:
      std::cerr << "error: invalid option\n";
      return EXIT_FAILURE;
    }
  }

  auto const choose_algorithm =
    [&board](Algorithm alg, size_t depth, size_t iter) -> MoveType
    {
      switch (alg)
      {
      case MINIMAX:
        return minimax(board, depth);
      case ALPHA_BETA:
        return alpha_beta(board, depth);
      case MONTE_CARLO:
        return monte_carlo_tree_search(board, iter);
      default:
        return INVALID_MOVE;
      }

      return INVALID_MOVE;
    };

  if (should_show)
  {
    std::cout << "Current config:\n  - X's:\n    * Algorithm: "
              << alg_to_string(x_alg)
              << "\n    * Depth: "
              << x_depth
              << "\n    * Maximum iterations: "
              << x_max_iter
              << "\n  - O's:\n    * O algorithm: "
              << alg_to_string(o_alg)
              << "\n    * Depth: "
              << o_depth
              << "\n    * Maximum iterations: "
              << o_max_iter
              << "\n  - Board:\n";

    board.print(6);

    std::cout << "  - Current player: "
              << (board.player ? 'X' : 'O')
              << "\n\n";
  }

  do
  {
    MoveType move = board.player ?
      choose_algorithm(x_alg, x_depth, x_max_iter) :
      choose_algorithm(o_alg, o_depth, o_max_iter);

    if (move >= COLUMNS)
    {
      std::cerr << "error: invalid column number."
        " It should be no greater than 6.\n";

      return EXIT_FAILURE;
    }

    board.insert_at(move);
    auto status = board.score().state;

    if (status != Board::NOT_OVER)
    {
      board.print();
      std::cout << "Status: " << to_string(status) << '\n';
      break;
    }

  } while (true);

  return EXIT_SUCCESS;
}
