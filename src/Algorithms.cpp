#include <algorithm>
#include <cmath>
#include <limits>
#include <cassert>

#include "Algorithms.hpp"

#define LOWEST_SCORE (std::numeric_limits<ScoreType>::lowest())
#define GREATEST_SCORE (std::numeric_limits<ScoreType>::max())

struct MinimaxData
{
  MoveType move;
  ScoreType score;
};

MinimaxData minimax_aux(Board &board, size_t depth, size_t &expanded)
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

    expanded++;
    auto score = minimax_aux(board, depth - 1, expanded).score;

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
  Board &board, ScoreType alpha, ScoreType beta, size_t depth, size_t &expanded
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

    expanded++;
    auto score = alpha_beta_aux(board, alpha, beta, depth - 1, expanded).score;

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

SearchResult minimax(Board board, size_t max_depth)
{
  size_t expanded = 0;
  auto move = minimax_aux(board, max_depth, expanded).move;

  return { move, expanded, Duration() };
}

SearchResult alpha_beta(Board board, size_t max_depth)
{
  size_t expanded = 0;
  auto move = alpha_beta_aux(
      board, LOWEST_SCORE, GREATEST_SCORE, max_depth, expanded
    ).move;

  return { move, expanded, Duration() };
}

SearchResult monte_carlo_tree_search(Board const &board, size_t max_iters)
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
    size_t expanded;

    MonteCarloTree(Board const &board)
      : root({ board, &root, nullptr, INVALID_MOVE, 0, 0, 0 }), expanded(0)
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

        double const score = (double)child.wins / (2 * child.visits) +
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
        win == Board::DRAW ? 1 : 2 * (root.board.player ?
          win == Board::X_WIN : win == Board::O_WIN)
        );

      return true;
    }

    void backpropagate(Node *leaf, uint32_t won)
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
    Node *append_leaves(Node *leaf)
    {
      assert(leaf->count == 0 && leaf->children == nullptr);

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
        expanded++;
      }

      return leaf->children;
    }

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

  auto node = tree.choose_best_child(&tree.root);

  return node == nullptr ?
    SearchResult{ INVALID_MOVE, 0, Duration() } :
    SearchResult{ node->move, tree.expanded, Duration() };
}
