#include <ostream>
#include <iostream>
#include "log.h"
#include "iheuristic.h"
#include "state.h"


static bool goal_eval(const char* val, int x, int y, unsigned dist, void* data)
{
        return *val == *(int*) data;
}

static bool is_goal_for(const char* board, unsigned w, unsigned h, const Move& move, int who, unsigned k)
{
        return (::scan_on(board, w, h, k, move.x, move.y, 0, ::goal_eval, &who) +
                ::scan_on(board, w, h, k, move.x, move.y, 4, ::goal_eval, &who)) > k ||

               (::scan_on(board, w, h, k, move.x, move.y, 1, ::goal_eval, &who) +
                ::scan_on(board, w, h, k, move.x, move.y, 5, ::goal_eval, &who)) > k ||

               (::scan_on(board, w, h, k, move.x, move.y, 2, ::goal_eval, &who) +
                ::scan_on(board, w, h, k, move.x, move.y, 6, ::goal_eval, &who)) > k ||

               (::scan_on(board, w, h, k, move.x, move.y, 3, ::goal_eval, &who) +
                ::scan_on(board, w, h, k, move.x, move.y, 7, ::goal_eval, &who)) > k;
}

// APIs
State::State(const unsigned num_cols,
             const unsigned num_rows,
             const bool gravity_on,
             const Move& last_move,
             const unsigned k,
             const unsigned deadline):
        num_rows(num_rows),
        num_cols(num_cols),
        num_left(num_rows*num_cols),
        gravity_on(gravity_on),
        last_move(last_move),
        deadline(deadline),
        k(k)
{
        m_board = new char [num_cols*num_rows];
        for (unsigned i = 0; i < num_cols*num_rows; i ++)
                m_board[i] = State::NO_PIECE;
}

State::State(const State& s):
        num_rows(s.num_rows),
        num_cols(s.num_cols),
        num_left(num_rows*num_cols),
        gravity_on(s.gravity_on),
        last_move(s.last_move),
        deadline(s.deadline),
        k(s.k),
        m_goal_for(s.m_goal_for),
        m_stack(s.m_stack)
{
        m_board = new char [num_cols*num_rows];
        for (unsigned y = 0; y < num_rows; y ++) {
                for (unsigned x = 0; x < num_cols; x ++) {
                        m_board[x + y*num_cols] = s.m_board[x + y*s.num_cols];
                }
        }
}

State::~State()
{
    delete [] m_board;
}

int State::is(unsigned x, unsigned y) const
{
        return m_board[x + y*num_cols];
}

void State::set_move(unsigned x, unsigned y, int who)
{
        num_left -= (int) (m_board[x + y*num_cols] == NO_PIECE) - (int) (who == NO_PIECE);
        m_board[x + y*num_cols] = who;
}

bool State::is_goal() const
{
        return m_goal_for != State::NO_PIECE;
}

bool State::is_goal_for(const Move& m, int who) const
{
        return ::is_goal_for(m_board, num_cols, num_rows, m, who, k);
}

bool State::is_goal_for(int who) const
{
        return m_goal_for == who;
}

bool State::collides_edges(int x, int y, unsigned d, int dist) const
{
        switch (d) {
        case 0:
                return x + dist >= (int) num_cols;
        case 1:
                return x + dist >= (int) num_cols || y - dist < 0;
        case 2:
                return y - dist < 0;
        case 3:
                return x - dist < 0 || y - dist < 0;
        case 4:
                return x - dist < 0;
        case 5:
                return x - dist < 0 || y + dist >= (int) num_rows;
        case 6:
                return y + dist >= (int) num_rows;
        case 7:
                return x + dist >= (int) num_cols || y + dist >= (int) num_rows;
        default:
                return false;
        }
}

bool State::collides_edges(int x, int y, unsigned d) const
{
        switch (d) {
        case 0:
                return x >= (int) num_cols;
        case 1:
                return x >= (int) num_cols || y < 0;
        case 2:
                return y < 0;
        case 3:
                return x < 0 || y < 0;
        case 4:
                return x < 0;
        case 5:
                return x < 0 || y >= (int) num_rows;
        case 6:
                return y >= (int) num_rows;
        case 7:
                return x >= (int) num_cols || y >= (int) num_rows;
        default:
                return false;
        }
}

const std::vector<State::MiniNode>& State::path() const
{
        return m_stack;
}

void State::push_move(unsigned x, unsigned y, int who)
{
        m_stack.push_back(State::MiniNode(x, y));
        m_board[x + y*num_cols] = who;

        if (::is_goal_for(m_board, num_cols, num_rows, Move(x, y), who, k))
                m_goal_for = who;
}

void State::prev_move(Move& move) const
{
        const State::MiniNode& node = m_stack.back();
        move.x = node.x;
        move.y = node.y;
}

void State::pop_move()
{
        const State::MiniNode& node = m_stack.back();
        m_board[node.x + node.y*num_cols] = State::NO_PIECE;
        m_goal_for = State::NO_PIECE;
        m_stack.pop_back();
}

void State::reset_all_moves()
{
        while (!m_stack.empty())
                pop_move();
}

std::ostream& operator<<(std::ostream& os, const State& s)
{
        os << "State = [";
        // Gameboard:
        os << "board:" << std::endl;
        for (unsigned y = 0; y < s.num_rows; y ++) {
                os << "\t";
                for (unsigned x = 0; x < s.num_cols; x ++) {
                        os << (int) s.m_board[x + y*s.num_cols] << "\t";
                }
                os << std::endl;
        }
        // Path:
        os << ", path:\t" << std::endl;
        for (unsigned i = 0; i < s.m_stack.size(); i ++) {
                s.m_stack[i].print(os);
                if (i != s.m_stack.size() - 1)
                        os << ",";
        }
        // Goal for:
        os << ", goal_for:" << s.m_goal_for << "]";

        return os;
}
