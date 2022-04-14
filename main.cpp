#include <iostream>
#include <cstring>
#include <limits>

#include "src/Board.hpp"
#include "src/Algorithms.hpp"

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

  char const *const algorithms[] = {
    "minimax", "alpha-beta", "monte-carlo"
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
      std::cerr << "error: unrecognized option: \"" << argv[i] << "\".\n";

      return EXIT_FAILURE;
    }
    else if (option_list[option].has_arg && ++i >= (size_t)argc)
    {
      std::cerr << "error: expected argument for the option \""
                << argv[i - 1]
                << "\".\n";

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
        std::cerr << "error: unrecognized algorithm \""
                  << argv[i]
                  << "\".\n";

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

      if ((ch != 'x' && ch != 'X' && ch != 'o' && ch != 'O') || argv[i][1] != '\0')
      {
        std::cerr << "error: invalid player: \""
                  << argv[i]
                  << "\".\n";

        return EXIT_FAILURE;
      }

      board.player = (ch == 'x' || ch == 'X');
      break;
    }
    default:
      std::cerr << "error: unrecognized option slipped through checks,"
        " this shouldn't happen.\n";
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
              << "\n  - O's:\n    * Algorithm: "
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
