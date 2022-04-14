#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include "Board.hpp"

MoveType minimax(Board board, size_t max_depth);

MoveType alpha_beta(Board board, size_t max_depth);

MoveType monte_carlo_tree_search(Board const &board, size_t max_iters);

#endif // ALGORITHMS_HPP
