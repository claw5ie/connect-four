#include <iostream>
#include <cstring>
#include <limits>
#include <cassert>

#include "src/Board.hpp"
#include "src/Algorithms.hpp"

#define MINIMAX_DEPTH 8
#define ALPHA_BETA_DEPTH 10
#define MONTE_CARLO_ITER 25000
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
    { 'b', "board", true },
    { 'c', "config", false },
    { 'p', "player", true },
    { 's', "no-stats", false },
    { '\0', "no-print-board", false }
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
      HUMAN,
      ALGORITHM_COUNT
    };

  char const *const algorithms[] = {
    "minimax", "alpha-beta", "monte-carlo", "human"
  };

  auto const what_algorithm =
    [algorithms](char const *arg) -> Algorithm
    {
      for (size_t i = 0; i < ALGORITHM_COUNT; i++)
      {
        if (!std::strcmp(arg, algorithms[i]))
          return (Algorithm)i;
      }

      return ALGORITHM_COUNT;
    };

  Board board = { };
  bool should_show = false;
  bool should_show_stats = true;
  bool should_print_board = true;

  struct
  {
    Algorithm algorithm;

    void print() const
    {
      std::cout << "  * algorithm: ";

      switch (algorithm)
      {
      case MINIMAX:
        std::cout << "minimax\n  * maximum depth: "
                  << MINIMAX_DEPTH
                  << '\n';
        break;
      case ALPHA_BETA:
        std::cout << "alpha-beta\n  * maximum depth: "
                  << ALPHA_BETA_DEPTH
                  << '\n';
        break;
      case MONTE_CARLO:
        std::cout << "monte carlo\n  * maximum iterations: "
                  << MONTE_CARLO_ITER
                  << '\n';
        break;
      case HUMAN:
        std::cout << "human\n";
        break;
      default:
        break;
      }
    }
  } player_info[2];

  player_info[0].algorithm = MINIMAX;
  player_info[1].algorithm = ALPHA_BETA;

  for (size_t i = 1; i < (size_t)argc; i++)
  {
    size_t const option = find_option(argv[i]);

    if (option == INVALID_OPTION)
    {
      std::cerr << "error: unrecognized option: \"" << argv[i] << "\".\n";

      return EXIT_FAILURE;
    }
    else if ((i += option_list[option].has_arg) >= (size_t)argc)
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
        player_info[option].algorithm = alg;
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
    case 6:
      should_print_board = false;
      break;
    default:
      std::cerr << "error: unrecognized option slipped through checks,"
        " this shouldn't happen.\n";
      return EXIT_FAILURE;
    }
  }

  auto const choose_algorithm =
    [&board, &player_info]() -> SearchResult
    {
      auto const start = std::chrono::steady_clock::now();

      SearchResult stats = { INVALID_MOVE, 0, Duration() };

      auto &player = player_info[(bool)board.player];

      switch (player.algorithm)
      {
      case MINIMAX:
        stats = minimax(board, MINIMAX_DEPTH);
        break;
      case ALPHA_BETA:
        stats = alpha_beta(board, ALPHA_BETA_DEPTH);
        break;
      case MONTE_CARLO:
        stats = monte_carlo_tree_search(board, MONTE_CARLO_ITER);
        break;
      case HUMAN:
        std::cout << "Your move [0-6]? ";
        std::cin >> stats.move;
        break;
      default:
        break;
      }

      stats.time_spent = std::chrono::steady_clock::now() - start;

      return stats;
    };

  if (should_show)
  {
    std::cout << "- player O:\n";
    player_info[0].print();
    std::cout << "- player X:\n";
    player_info[1].print();
    std::cout << "- board:\n";
    board.print(2);
    std::cout << "- current player: "
              << (board.player ? 'X' : 'O')
              << "\n\n";
  }

  if (should_print_board)
    board.print();

  do
  {
    SearchResult stats = choose_algorithm();

    if (stats.move >= COLUMNS)
    {
      std::cerr << "error: invalid column number."
        " It should be no greater than 6.\n";

      if (player_info[(bool)board.player].algorithm == HUMAN)
        continue;
      else
        return EXIT_FAILURE;
    }

    board.insert_at(stats.move);

    if (should_print_board)
      board.print();

    if (should_show_stats)
    {
      std::cout << "expanded " << stats.expanded
                << " nodes and spent "
                << stats.time_spent.count()
                << " seconds.\n";
    }

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
