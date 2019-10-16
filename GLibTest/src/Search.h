#pragma once

#include <vector>
#include <stdexcept>
#include <cassert>
#include <array>
#include <random>

#include "GLib.h"

#define CheckStatePersistence

#ifdef CheckStatePersistence
#define Check(x) x
#else
#define Check(x)
#endif

std::random_device dev;
std::mt19937 rng(dev());

enum Player
{
	P1, P2, Empty
};

enum Side
{
	right, topRight, topLeft, left, bottomLeft, bottomRight
};

#define MaxScore 100

#define XSIZE 12
#define YSIZE 12

Player getOtherPlayer(Player player)
{
	if (player == Player::P1)
	{
		return Player::P2;
	}
	else
	{
		return Player::P1;
	}
}

struct Location
{
	int x;
	int y;

	bool operator==(const Location& other) const
	{
		return x == other.x && y == other.y;
	};
};

struct StaticLocation
{
	Player player;
	int moveIndex;
	int freeSpotsIndex;
	int amountNeighbours;

	bool operator==(const StaticLocation& other) const
	{
		return player == other.player 
			&& moveIndex == other.moveIndex
			&& freeSpotsIndex == other.freeSpotsIndex
			&& amountNeighbours == other.amountNeighbours;
	};
};

struct Move
{
	Location location;
	Player player;

	bool operator==(const Move& other) const
	{
		return location == other.location && player == other.player;
	};
};

struct FreeSpot
{
	Location location;
	int moveIndex;

	bool operator==(const FreeSpot& other) const
	{
		return location == other.location && moveIndex == other.moveIndex;
	};
};

struct Score
{
	int straithP1 = 0;
	int straithP2 = 0;

	bool hasCircleP1 = false;
	bool hasCircleP2 = false;

	bool operator==(const Score& other) const
	{
		return straithP1 == other.straithP1
			&& straithP2 == other.straithP2
			&& hasCircleP1 == other.hasCircleP1
			&& hasCircleP2 == other.hasCircleP2;
	};
};

class Board
{
public:
	Board()
	{
		calcNeighbours();
	}

	void calcNeighbours()
	{
		for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				if (i > 0) neighbours[i][j][Side::left] = { i - 1, j };
				if (i < XSIZE-1) neighbours[i][j][Side::right] = { i + 1, j };

				if (j > 0)
				{
					if (j % 2 == 0)
					{
						if (i > 0) neighbours[i][j][Side::topLeft] = { i - 1, j - 1 };
						neighbours[i][j][Side::topRight] = { i, j - 1 };
					}
					else
					{
						neighbours[i][j][Side::topLeft] = { i, j - 1 };
						if (i < XSIZE-1) neighbours[i][j][Side::topRight] = { i + 1, j - 1 };
					}
				}

				if (j < YSIZE-1)
				{
					if (j % 2 == 0)
					{
						if (i > 0) neighbours[i][j][Side::bottomLeft] = { i - 1, j + 1 };
						neighbours[i][j][Side::bottomRight] = { i, j + 1 };
					}
					else
					{
						neighbours[i][j][Side::bottomLeft] = { i, j + 1 };
						if (i < XSIZE-1) neighbours[i][j][Side::bottomRight] = { i + 1, j + 1 };
					}
				}
			}
		}
		for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				if (i == 0 || j == 0 || i == XSIZE - 1 || j == YSIZE - 1)
				{
					inBounds[i][j] = false;
				}
				else
				{
					inBounds[i][j] = true;
				}
			}
		}
	};

	std::array<Location, 6> neighbours[XSIZE][YSIZE];
	bool inBounds[XSIZE][YSIZE];
};

class State
{
public:
	State()
	{
		for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				staticMoves[i][j] = { Player::Empty, -1, -1, 0};
			}
		}

		freeSpots.push_back({ {5, 5}, 0 });
		staticMoves[5][5].freeSpotsIndex = 0;
	}

	void makeMove(int x, int y)
	{
		Move move = { {x, y}, player };
		moves.push_back(move);
		staticMoves[move.location.x][move.location.y].player = move.player;
		staticMoves[move.location.x][move.location.y].moveIndex = moves.size() - 1;

		const int freeSpotIndex = staticMoves[move.location.x][move.location.y].freeSpotsIndex;
		assert(freeSpotIndex >= 0);
		freeSpots[freeSpotIndex].moveIndex = -freeSpots[freeSpotIndex].moveIndex;

		moveIndex++;
		player = getOtherPlayer(player);

		if (moves.size() == 2)
		{
			while (freeSpots.size() > 1)
			{
				staticMoves[freeSpots.back().location.x][freeSpots.back().location.y].freeSpotsIndex = -1;
				freeSpots.pop_back();
			}
		}

		const auto& neighbours = board.neighbours[move.location.x][move.location.y];
		for (const auto& location : neighbours)
		{
			if (board.inBounds[location.x][location.y])
			{
				staticMoves[location.x][location.y].amountNeighbours++;

				if (staticMoves[location.x][location.y].player == Player::Empty)
				{
					if (moves.size() == 1 || staticMoves[location.x][location.y].amountNeighbours == 2)
					{
						freeSpots.push_back({ location, moveIndex });
						staticMoves[location.x][location.y].freeSpotsIndex = freeSpots.size() - 1;
					}
				}
			}
		}

		Score newScore;
		if (scores.size() > 0) newScore = scores.back();

		int straight = partOfStraight(move.location);
		if (move.player == Player::P1 && straight > newScore.straithP1) newScore.straithP1 = straight;
		if (move.player == Player::P2 && straight > newScore.straithP2) newScore.straithP2 = straight;

		if (move.player == Player::P1 && makesCircle(move.location)) newScore.hasCircleP1 = true;
		if (move.player == Player::P2 && makesCircle(move.location)) newScore.hasCircleP2 = true;

		scores.push_back(newScore);
	}

	void undoMove()
	{
		scores.pop_back();

		if (moves.size() == 2)
		{
			while (freeSpots.size() > 1)
			{
				staticMoves[freeSpots.back().location.x][freeSpots.back().location.y].freeSpotsIndex = -1;
				freeSpots.pop_back();
			}

			{
				const auto& neighbours = board.neighbours[moves.back().location.x][moves.back().location.y];
				for (const auto& location : neighbours)
				{
					if (board.inBounds[location.x][location.y])
					{
						staticMoves[location.x][location.y].amountNeighbours--;
					}
				}
			}

			{
				const auto& neighbours = board.neighbours[moves[0].location.x][moves[0].location.y];
				for (const auto& location : neighbours)
				{
					if (board.inBounds[location.x][location.y])
					{
						freeSpots.push_back({ location, 1});
						staticMoves[location.x][location.y].freeSpotsIndex = freeSpots.size() - 1;
					}
				}
			}
		}
		else
		{
			while (freeSpots.size() > 0 && freeSpots.back().moveIndex == moveIndex)
			{
				staticMoves[freeSpots.back().location.x][freeSpots.back().location.y].freeSpotsIndex = -1;
				freeSpots.pop_back();
			}

			const int freeSpotIndex = staticMoves[moves.back().location.x][moves.back().location.y].freeSpotsIndex;
			assert(freeSpotIndex >= 0);
			freeSpots[freeSpotIndex].moveIndex = -freeSpots[freeSpotIndex].moveIndex;

			const auto& neighbours = board.neighbours[moves.back().location.x][moves.back().location.y];
			for (const auto& location : neighbours)
			{
				if (board.inBounds[location.x][location.y])
				{
					staticMoves[location.x][location.y].amountNeighbours--;
				}
			}
		}

		moveIndex--;
		player = getOtherPlayer(player);

		staticMoves[moves.back().location.x][moves.back().location.y].player = Player::Empty;
		staticMoves[moves.back().location.x][moves.back().location.y].moveIndex = -1;
		moves.pop_back();
	}

	bool isFreeSpot(int x, int y)
	{
		const int freeSpotIndex = staticMoves[x][y].freeSpotsIndex;
		if (freeSpotIndex >= 0)
		{
			if (freeSpots[freeSpotIndex].moveIndex > 0)
			{
				return true;
			}
		}

		return false;
	}

	int partOfStraight(Location location)
	{
		int maxStraithLocation = 0;
		int straight = 0;

		straight = 1
			+ expandPartOfStraight(board.neighbours[location.x][location.y][left], left, staticMoves[location.x][location.y].player)
			+ expandPartOfStraight(board.neighbours[location.x][location.y][right], right, staticMoves[location.x][location.y].player);

		if (straight > maxStraithLocation) maxStraithLocation = straight;

		straight = 1
			+ expandPartOfStraight(board.neighbours[location.x][location.y][topLeft], topLeft, staticMoves[location.x][location.y].player)
			+ expandPartOfStraight(board.neighbours[location.x][location.y][bottomRight], bottomRight, staticMoves[location.x][location.y].player);

		if (straight > maxStraithLocation) maxStraithLocation = straight;

		straight = 1
			+ expandPartOfStraight(board.neighbours[location.x][location.y][topRight], topRight, staticMoves[location.x][location.y].player)
			+ expandPartOfStraight(board.neighbours[location.x][location.y][bottomLeft], bottomLeft, staticMoves[location.x][location.y].player);

		if (straight > maxStraithLocation) maxStraithLocation = straight;

		return maxStraithLocation;
	}
	int expandPartOfStraight(Location location, Side side, Player player)
	{
		if (board.inBounds[location.x][location.y] && staticMoves[location.x][location.y].player == player)
		{
			return expandPartOfStraight(board.neighbours[location.x][location.y][side], side, player) + 1;
		}
		else
		{
			return 0;
		}
	}

	bool makesCircle(Location location)
	{
		std::array<bool, 6> covered = {false, false, false, false, false, false};
		
		for (int i = 0; i < 6; i++)
		{
			if (!covered[i])
			{
				Location current = location;
				Side side = (Side)i;
				int sum = 0;

				if (staticMoves[current.x][current.y].player != staticMoves[board.neighbours[current.x][current.y][side].x][board.neighbours[current.x][current.y][side].y].player)
				{
					do
					{
						if (board.inBounds[board.neighbours[current.x][current.y][(side + 1) % 6].x][board.neighbours[current.x][current.y][(side + 1) % 6].y]
							&& staticMoves[current.x][current.y].player == staticMoves[board.neighbours[current.x][current.y][(side+1) % 6].x][board.neighbours[current.x][current.y][(side + 1) % 6].y].player)
						{
							sum += 1;
							current = { board.neighbours[current.x][current.y][(side + 1) % 6].x, board.neighbours[current.x][current.y][(side + 1) % 6].y };
							side = (Side)((side + 5) % 6);
						}
						else
						{
							sum -= 1;
							side = (Side)((side + 1) % 6);
						}
						if (current == location)
						{
							covered[side] = true;
						}

					} while (!(current == location) || side != i);

					if (sum == 6)
					{
						bool containsOther = flowFillSearch({ board.neighbours[location.x][location.y][i].x, board.neighbours[location.x][location.y][i].y }, getOtherPlayer(staticMoves[location.x][location.y].player));
						if (containsOther)
						{
							return true;
						}
					}
				}
				covered[i] = true;
			}
		}

		return false;
	}
	bool flowFillSearch(Location location, Player player)
	{
		std::vector<Location> visited;
		return flowFillSearchExpand(visited, location, player);
	}
	bool flowFillSearchExpand(std::vector<Location>& visited, Location location, Player player)
	{
		Player current = staticMoves[location.x][location.y].player;
		if (current == player)
		{
			return true;
		}
		else if (current == Player::Empty)
		{
			visited.push_back(location);
			for (auto neighbour : board.neighbours[location.x][location.y])
			{
				if (board.inBounds[location.x][location.y])
				{
					if (std::find(visited.begin(), visited.end(), neighbour) == visited.end())
					{
						bool result = flowFillSearchExpand(visited, neighbour, player);
						if (result)
						{
							return true;
						}
					}
				}
			}
			return false;
		}
		else
		{
			return false;
		}
	}

	int evaluate(Player asPlayer)
	{
		if (scores.size() > 0)
		{
			if (scores.back().hasCircleP1 || scores.back().straithP1 >= 5)
			{
				if (asPlayer == Player::P1)
				{
					return MaxScore;
				}
				else
				{
					return -MaxScore;
				}
			}
			if (scores.back().hasCircleP2 || scores.back().straithP2 >= 5)
			{
				if (asPlayer == Player::P1)
				{
					return -MaxScore;
				}
				else
				{
					return MaxScore;
				}
			}
			if (asPlayer == Player::P1)
			{
				return scores.back().straithP1;
			}
			else
			{
				return scores.back().straithP2;
			}
		}
		else
		{
			return 0;
		}
	}
	bool isEndGame()
	{
		return scores.back().hasCircleP1 || scores.back().straithP1 >= 5
			|| scores.back().hasCircleP2 || scores.back().straithP2 >= 5;
	}

	bool operator==(const State& other) const
	{
		bool staticMovesEqual = true;

		for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				if (!(staticMoves[i][j] == other.staticMoves[i][j]))
				{
					staticMovesEqual = false;
				}
			}
		}

		return player == other.player 
			&& moveIndex == other.moveIndex 
			&& moves == other.moves 
			&& freeSpots == other.freeSpots 
			&& staticMovesEqual;
	}

	Player player = Player::P1;
	int moveIndex = 0;

	std::vector<Move> moves;
	std::vector<FreeSpot> freeSpots;

	StaticLocation staticMoves[XSIZE][YSIZE];

	Board board;

	std::vector<Score> scores;
};

std::pair<int, int> alphaBetaBranch(State& state, long& nodesVisited, int depth, Player asPlayer, int alpha, int beta)
{
	nodesVisited++;

	if (depth == 0 || state.isEndGame())
	{
		return { state.evaluate(asPlayer), 0 };
	}

	/*int score = -999;
	int index = -1;
	for (int i = 0; i < state.freeSpots.size(); i++)
	{
		if (state.freeSpots[i].moveIndex > 0)
		{
			state.makeMove(state.freeSpots[i].location.x, state.freeSpots[i].location.y);
			auto[value, move] = alphaBetaBranch(state, nodesVisited, depth - 1, asPlayer, -beta, -alpha);
			state.undoMove();

			value = -value;

			if (value > score)
			{
				score = value;
				index = i;
			}
			if (score > alpha) alpha = score;
			if (score >= beta) break;
		}
	}*/

	int index = -1;
	int score;
	if (state.player == asPlayer)
	{
		score = -999;
		for (int i = 0; i < state.freeSpots.size(); i++)
		{
			if (state.freeSpots[i].moveIndex > 0)
			{
				state.makeMove(state.freeSpots[i].location.x, state.freeSpots[i].location.y);
				auto [value, move] = alphaBetaBranch(state, nodesVisited, depth - 1, asPlayer, alpha, beta);
				state.undoMove();

				if (value > score)
				{
					score = value;
					index = i;
				}
				if (score > alpha) alpha = score;
				if (alpha >= beta) break;
			}
		}
	}
	else
	{
		score = 999;
		for (int i = 0; i < state.freeSpots.size(); i++)
		{
			if (state.freeSpots[i].moveIndex > 0)
			{
				state.makeMove(state.freeSpots[i].location.x, state.freeSpots[i].location.y);
				auto [value, move] = alphaBetaBranch(state, nodesVisited, depth - 1, asPlayer, alpha, beta);
				state.undoMove();

				if (value < score)
				{
					score = value;
					index = i;
				}
				if (score < beta) beta = score;
				if (alpha >= beta) break;
			}
		}
	}

	return { score, index };
}
std::pair<Location, int> alphaBeta(State& state, long& nodesVisited, int depth)
{
	/*int maxIndex = -1;
	int maxWins = -1;
	Player asPlayer = state.player;

	Check(auto stateCopy = state);

	for (int i = 0; i < state.freeSpots.size(); i++)
	{
		if (state.freeSpots[i].moveIndex > 0)
		{
			state.makeMove(state.freeSpots[i].location.x, state.freeSpots[i].location.y);
			auto [value, move] = alphaBetaBranch(state, nodesVisited, depth - 1, asPlayer, -999, 999);
			state.undoMove();

			GLib::Out << score << " ";

			Check(assert(state == stateCopy));
			
			if (maxIndex == -1 || score > maxWins)
			{
				maxIndex = i;
				maxWins = score;
			}
		}
	}

	GLib::Out << "\n";*/

	auto [value, move] = alphaBetaBranch(state, nodesVisited, depth - 1, state.player, -999, 999);
	return { state.freeSpots[move].location, value };
}

std::pair<Location, int> alphaBetaIterative(State& state, long& nodesVisited, int depth, int time)
{
	for (int i = 1; i <= depth; i++)
	{
		auto[location, score] = alphaBeta(state, nodesVisited, i);

		if (score == MaxScore || i == depth)
		{
			return { location, score };
		}
	}
}

int randomPlayout(State state, Player asPlayer)
{
	int score = state.evaluate(asPlayer);

	while (score != -MaxScore && score != MaxScore)
	{
		std::uniform_int_distribution<std::mt19937::result_type> dist6(0, state.freeSpots.size() - 1);

		int i = dist6(rng);
		int fails = 0;

		while (state.freeSpots[i].moveIndex <= 0)
		{
			i = dist6(rng);
			fails++;

			if (fails > 50)
			{
				return 0;
			}
		}

		state.makeMove(state.freeSpots[i].location.x, state.freeSpots[i].location.y);
		score = state.evaluate(asPlayer);
	}

	return state.evaluate(asPlayer);
}
std::pair<Location, int> monteCarloPlayer(State& state)
{
	int maxIndex = -1;
	int maxWins = -1;

	Check(auto stateCopy = state);

	for (int i = 0; i < state.freeSpots.size(); i++)
	{
		if (state.freeSpots[i].moveIndex > 0)
		{
			int sum = 0;

			state.makeMove(state.freeSpots[i].location.x, state.freeSpots[i].location.y);

			for (int i = 0; i < 1000; i++)
			{
				if (randomPlayout(state, state.player) == MaxScore)
				{
					sum++;
				}
			}

			state.undoMove();

			Check(assert(state == stateCopy));
			
			if (maxIndex == -1 || sum > maxWins)
			{
				maxIndex = i;
				maxWins = sum;
			}
		}
	}

	return { state.freeSpots[maxIndex].location, maxWins };
}
