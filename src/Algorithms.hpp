#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include <chrono>

#include "Board.hpp"

using Duration = std::chrono::duration<double>;

struct SearchResult
{
  MoveType move;
  size_t expanded;
  Duration time_spent;
};

SearchResult minimax(Board board, size_t max_depth);

SearchResult alpha_beta(Board board, size_t max_depth);

SearchResult monte_carlo_tree_search(Board const &board, size_t max_iters);

#endif // ALGORITHMS_HPP
