#include <iostream>
#include <cassert>
#include <limits>
#include <cmath>

using MoveType = uint32_t;
using ScoreType = int32_t;

#define INVALID_MOVE (std::numeric_limits<MoveType>::max())
#define LOWEST_SCORE (std::numeric_limits<ScoreType>::lowest())
#define GREATEST_SCORE (std::numeric_limits<ScoreType>::max())

enum GameState
{
  NOT_OVER,
  DRAW,
  O_WIN,
  X_WIN,
  GAME_STATE_COUNT
};

#define COLUMNS 7
#define ROWS 6

struct Board
{
  struct Status
  {
    GameState state;
    ScoreType score;
  };

  int8_t player;
  uint8_t free[COLUMNS];
  int8_t data[COLUMNS][ROWS];

  void remove_at(MoveType column)
  {
    assert(column < COLUMNS);

    if (free[column] > 0)
    {
      free[column]--;
      player = !player;
    }
  }

  bool insert_at(MoveType column)
  {
    assert(column < COLUMNS);

    if (free[column] < ROWS)
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

  bool is_over() const
  {
    for (size_t i = 0; i < COLUMNS; i++)
    {
      if (free[i] < ROWS)
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
      count += (free[i] < ROWS);
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
          size_t zeroes = !data[i][j] && j < free[i],
            ones = (data[i][j] != 0) && j < free[i];

          int32_t x = i,
            y = j;

          for (size_t s = 0; s < 3; s++)
          {
            x += dirs[k][0];
            y += dirs[k][1];

            if (x >= 0 && x < COLUMNS && y >= 0 && y < free[x])
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

  void print() const
  {
    for (size_t j = 0; j < ROWS; j++)
    {
      for (size_t i = 0; i < COLUMNS; i++)
      {
        size_t row = ROWS - 1 - j;

        std::cout << (row < free[i] ? (data[i][row] ? 'X' : 'O') : '-')
                  << (i + 1 < COLUMNS ? ' ' : '\n');
      }
    }
  }
};

char const *to_string(GameState status)
{
  static char const *const lookup[] = { "Not over", "Draw", "O won", "X won" };

  return status < GAME_STATE_COUNT ? lookup[status] : nullptr;
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

      while (current->children != nullptr)
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
          win == X_WIN : win == O_WIN
        );

      return true;
    }

    Node *append_leaves(Node *leaf)
    {
      static MoveType moves[COLUMNS];

      size_t count = 0;
      for (MoveType i = 0; i < COLUMNS; i++)
      {
        if (leaf->board.free[i] < ROWS)
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

int main()
{
  Board board = { };

  board.print();

  do
  {
    auto move = monte_carlo_tree_search(board, board.player ? 50000 : 100000);

    if (move >= COLUMNS)
    {
      std::cout << "Invalid column number. It should be no greater than 6.\n";
      break;
    }

    board.insert_at(move);
    board.print();
    auto status = board.score().state;

    if (status != NOT_OVER)
    {
      std::cout << "Status: " << to_string(status) << '\n';
      break;
    }

  } while (true);

  return 0;
}
