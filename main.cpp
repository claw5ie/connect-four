#include <iostream>
#include <cstring>
#include <limits>
#include <cassert>

#include "src/Board.hpp"
#include "src/Algorithms.hpp"

#define INVALID_OPTION (std::numeric_limits<size_t>::max())

char const *find_next_elem(char const *str)
{
  while (*str != ',' && *str != '\0')
    str++;

  return str;
}

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
    { 'b', "board", true },
    { 'c', "config", false },
    { 'p', "player", true },
    { 's', "no-stats", false }
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
    [algorithms](char const *arg, size_t count) -> Algorithm
    {
      for (size_t i = 0; i < ALGORITHM_COUNT; i++)
      {
        if (!std::strncmp(arg, algorithms[i], count))
          return (Algorithm)i;
      }

      return ALGORITHM_COUNT;
    };

  Board board = { };
  bool should_show = false;
  bool should_show_stats = true;

  struct
  {
    Algorithm algorithm;

    union
    {
      struct
      {
        size_t depth;
      } minimax;

      struct
      {
        size_t depth;
      } alpha_beta;

      struct
      {
        size_t max_iter;
      } monte_carlo;
    } as;

    void set(size_t val)
    {
      switch (algorithm)
      {
      case MINIMAX:
        as.minimax.depth = val;
        break;
      case ALPHA_BETA:
        as.alpha_beta.depth = val;
        break;
      case MONTE_CARLO:
        as.monte_carlo.max_iter = val;
        break;
      default:
        assert(false);
      }
    }

    void set_default()
    {
      switch (algorithm)
      {
      case MINIMAX:
        as.minimax.depth = 8;
        break;
      case ALPHA_BETA:
        as.alpha_beta.depth = 10;
        break;
      case MONTE_CARLO:
        as.monte_carlo.max_iter = 25000;
        break;
      default:
        assert(false);
      }
    }

    void print() const
    {
      std::cout << "- algorithm: ";

      switch (algorithm)
      {
      case MINIMAX:
        std::cout << "minimax\n- maximum depth: " << as.minimax.depth << '\n';
        break;
      case ALPHA_BETA:
        std::cout << "alpha-beta\n- maximum depth: " << as.alpha_beta.depth << '\n';
        break;
      case MONTE_CARLO:
        std::cout << "monte carlo\n- maximum iterations: " << as.monte_carlo.max_iter << '\n';
        break;
      default:
        assert(false);
      }
    }
  } data[2];

  data[0].algorithm = MINIMAX;
  data[0].set_default();
  data[1].algorithm = ALPHA_BETA;
  data[1].set_default();

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
      auto next_arg = find_next_elem(argv[i]);
      auto alg = what_algorithm(argv[i], next_arg - argv[i]);

      if (alg < ALGORITHM_COUNT)
      {
        data[option].algorithm = alg;

        if (*next_arg == '\0')
          data[option].set_default();
        else
          data[option].set(strtoul(next_arg + 1, nullptr, 10));
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
      board.read_board(argv[i]);
      break;
    case 3:
      should_show = true;
      break;
    case 4:
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
    case 5:
      should_show_stats = false;
      break;
    default:
      std::cerr << "error: unrecognized option slipped through checks,"
        " this shouldn't happen.\n";
      return EXIT_FAILURE;
    }
  }

  auto const choose_algorithm =
    [&board, &data]() -> SearchResult
    {
      auto const start = std::chrono::steady_clock::now();

      SearchResult stats = { INVALID_MOVE, 0, Duration() };

      auto &player = data[(bool)board.player];

      switch (player.algorithm)
      {
      case MINIMAX:
        stats = minimax(board, player.as.minimax.depth);
        break;
      case ALPHA_BETA:
        stats = alpha_beta(board, player.as.alpha_beta.depth);
        break;
      case MONTE_CARLO:
        stats = monte_carlo_tree_search(board, player.as.monte_carlo.max_iter);
        break;
      default:
        break;
      }

      stats.time_spent = std::chrono::steady_clock::now() - start;

      return stats;
    };

  if (should_show)
  {
    std::cout << "player O:\n";
    data[0].print();
    std::cout << "player X:\n";
    data[1].print();
    std::cout << "board:\n";
    board.print();

    std::cout << "- current player: "
              << (board.player ? 'X' : 'O')
              << "\n\n";
  }

  do
  {
    SearchResult stats = board.player ?
      choose_algorithm() :
      choose_algorithm();

    if (stats.move >= COLUMNS)
    {
      std::cerr << "error: invalid column number."
        " It should be no greater than 6.\n";

      return EXIT_FAILURE;
    }

    if (should_show_stats)
    {
      std::cout << (board.player ? 'X' : 'O')
                << " expanded " << stats.expanded
                << " nodes and spent "
                << stats.time_spent.count()
                << " seconds.\n";
    }

    board.insert_at(stats.move);
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
