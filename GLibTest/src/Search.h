#pragma once

#include <vector>
#include <stdexcept>
#include <cassert>
#include <array>
#include <random>
#include <sstream> 
#include <map>

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

enum ValueType
{
	Exact, Lower, Upper
};

#define MaxScore 800

#define XSIZE 21
#define YSIZE 21

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
	
	int hasPossibleStraithP1 = 0;
	int hasPossibleStraithP2 = 0;

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
		calcBounds();
		calcLinearIndex();
		calcRows();
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
	};
	void calcBounds()
	{
		for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				inBounds[i][j] = true;

				if (j == 0 || j == YSIZE - 1) // top and bottom row
				{
					inBounds[i][j] = false;
				}

				if ((j % 2 == 0 && i + j / 2 < 6) || (j % 2 == 1 && i + j / 2 < 5)) // top left
				{
					inBounds[i][j] = false;
				}

				if (i - j / 2 < -4) // bottom left
				{
					inBounds[i][j] = false;
				}

				if (j / 2 - i < -14) // top right
				{
					inBounds[i][j] = false;
				}

				if ((j % 2 == 0 && i + j / 2 > 24) || (j % 2 == 1 && i + j / 2 > 23)) // bottom right
				{
					inBounds[i][j] = false;
				}
			}
		}		
	}
	void calcLinearIndex()
	{
		// 271
		for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				if (inBounds[i][j])
				{
					linearIndex[i][j] = amount;
					reverseLinearIndex.push_back({ i, j });
					amount++;
				}
			}
		}
	}
	void calcRows()
	{
		// 585
		for (auto location : reverseLinearIndex)
		{
			for (auto dir : { left, bottomLeft, bottomRight })
			{
				std::vector<Location> row;
				row.push_back(location);
				while (row.size() < 5)
				{
					Location next = neighbours[row.back().x][row.back().y][dir];
					if (!inBounds[next.x][next.y])
					{
						break;
					}
					row.push_back(next);
				}
				if (row.size() == 5)
				{
					allRows.push_back(row);
				}
			}
		}
	}

	std::array<Location, 6> neighbours[XSIZE][YSIZE];
	int linearIndex[XSIZE][YSIZE];
	std::vector<Location> reverseLinearIndex;

	bool inBounds[XSIZE][YSIZE];
	int amount = 0;

	std::vector<std::vector<Location>> allRows;
	std::map<int,int> moveRowReduction[XSIZE][YSIZE];
};

class StateHash
{
public:
	StateHash() : data({0,0,0,0,0,0,0,0,0,0})
	{
	}

	void set(Player p, int bit)
	{
		int index = bit / 64;
		bit -= index * 64;
		if (p == Player::P2)
		{
			index += 5;
		}

		data[index] |= 1ull << bit;
	}

	void unset(Player p, int bit)
	{
		int index = bit / 64;
		bit -= index * 64;
		if (p == Player::P2)
		{
			index += 5;
		}

		data[index] &= ~(1ull << bit);
	}

	bool get(Player p, int bit)
	{
		int index = bit / 64;
		bit -= index * 64;
		if (p == Player::P2)
		{
			index += 5;
		}

		return data[index] & (1ull << bit);
	}

	bool operator==(const StateHash& other)const
	{
		bool equal = true;
		for (int i = 0; i < 10; i++)
		{
			if (data[i] != other.data[i])
			{
				equal = false;
				break;
			}
		}
		return equal;
	}

	std::array<unsigned long long, 10> data;
};

bool operator<(const StateHash& left, const StateHash& right)
{
	return left.data < right.data;
}

class Hasher {
public:
	unsigned long long operator()(const StateHash& other) const
	{
		unsigned long long hash = 0;
		for (int i = 0; i < 10; i++)
		{
			hash ^= other.data[i];
		}
		return hash;
	}
};

struct StateTreeResult
{
	StateTreeResult(int _value, int _nodesVisited = 1, int _move = -1)
	{
		value = _value;
		nodesVisited = _nodesVisited;
		move = _move;
	}

	int value;
	ValueType type = ValueType::Exact;
	int move = -1;
	Location moveLocation = {0, 0};
	int nodesVisited = 1;
	int depth = 0;
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

		freeSpots.push_back({ {10, 10}, 0 });
		staticMoves[10][10].freeSpotsIndex = 0;
	}

	State(const State& other) = delete;

	void makeMove(int x, int y)
	{
		Move move = { {x, y}, player };
		moves.push_back(move);
		staticMoves[move.location.x][move.location.y].player = move.player;
		staticMoves[move.location.x][move.location.y].moveIndex = moves.size() - 1;
		stateHash.set(player, board.linearIndex[x][y]);

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
		if (scores.size() > 0)
		{
			newScore = scores.back();
		}
		else
		{
			newScore.hasPossibleStraithP1 = board.allRows.size();
			newScore.hasPossibleStraithP2 = board.allRows.size();
		}

		int straight = partOfStraight(move.location);
		if (move.player == Player::P1 && straight > newScore.straithP1) newScore.straithP1 = straight;
		if (move.player == Player::P2 && straight > newScore.straithP2) newScore.straithP2 = straight;

		int blocking = locationBlockingStraith(move.location);
		if (move.player == Player::P1) newScore.hasPossibleStraithP2 -= blocking;
		if (move.player == Player::P2) newScore.hasPossibleStraithP1 -= blocking;

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

		stateHash.unset(player, board.linearIndex[moves.back().location.x][moves.back().location.y]);
		staticMoves[moves.back().location.x][moves.back().location.y].player = Player::Empty;
		staticMoves[moves.back().location.x][moves.back().location.y].moveIndex = -1;
		moves.pop_back();
	}

	bool isFreeSpot(int x, int y) const
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

	int partOfStraight(Location location) const
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
	int expandPartOfStraight(Location location, Side side, Player player) const
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

	int locationBlockingStraith(Location location) const
	{
		int blocking = 0;
		int straight = 0;

		straight = 1
			+ expandPartOfStraightInverse(board.neighbours[location.x][location.y][left], left, staticMoves[location.x][location.y].player, 4)
			+ expandPartOfStraightInverse(board.neighbours[location.x][location.y][right], right, staticMoves[location.x][location.y].player, 4)
			- 4;

		if (straight > 0) blocking += straight;

		straight = 1
			+ expandPartOfStraightInverse(board.neighbours[location.x][location.y][topLeft], topLeft, staticMoves[location.x][location.y].player, 4)
			+ expandPartOfStraightInverse(board.neighbours[location.x][location.y][bottomRight], bottomRight, staticMoves[location.x][location.y].player, 4)
			- 4;

		if (straight > 0) blocking += straight;

		straight = 1
			+ expandPartOfStraightInverse(board.neighbours[location.x][location.y][topRight], topRight, staticMoves[location.x][location.y].player, 4)
			+ expandPartOfStraightInverse(board.neighbours[location.x][location.y][bottomLeft], bottomLeft, staticMoves[location.x][location.y].player, 4)
			- 4;

		if (straight > 0) blocking += straight;

		return blocking;
	}
	int expandPartOfStraightInverse(Location location, Side side, Player player, int limit) const
	{
		if (board.inBounds[location.x][location.y] && staticMoves[location.x][location.y].player != player && limit > 0)
		{
			return expandPartOfStraightInverse(board.neighbours[location.x][location.y][side], side, player, limit - 1) + 1;
		}
		else
		{
			return 0;
		}
	}

	bool makesCircle(Location location) const
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
	bool flowFillSearch(Location location, Player player) const
	{
		std::vector<Location> visited;
		return flowFillSearchExpand(visited, location, player);
	}
	bool flowFillSearchExpand(std::vector<Location>& visited, Location location, Player player) const
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

	int evaluate() const
	{
		if (scores.size() > 0)
		{
			if (scores.back().hasCircleP1 || scores.back().straithP1 >= 5)
			{
				if (player == Player::P1)
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
				if (player == Player::P1)
				{
					return -MaxScore;
				}
				else
				{
					return MaxScore;
				}
			}

			if (player == Player::P1)
			{
				return 10 * (scores.back().straithP1 - scores.back().straithP2) + scores.back().hasPossibleStraithP1 - scores.back().hasPossibleStraithP2;
			}
			else
			{
				return 10 * (scores.back().straithP2 - scores.back().straithP1) + scores.back().hasPossibleStraithP2 - scores.back().hasPossibleStraithP1;
			}
		}
		else
		{
			return 0;
		}
	}
	bool isEndGame() const
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

	StateHash stateHash;
};

StateTreeResult alphaBeta(State& state, std::map<StateHash, StateTreeResult>& transpositionTable, int depth, int alpha, int beta, bool& stop)
{
	long nodesVisited = 0;

	if (depth <= 0 || state.isEndGame())
	{
		return StateTreeResult(state.evaluate());
	}

	int olda = alpha;
	int bestMove = -1;
	bool cacheFound = false;
	auto cache = transpositionTable.find(state.stateHash);
	if (cache != transpositionTable.end())
	{
		cacheFound = true;
		StateTreeResult result = cache->second;
		if (result.depth >= depth)
		{
			if (result.type == ValueType::Exact)
			{
				return result;
			}
			else if (result.type == ValueType::Lower)
			{
				alpha = max(alpha, result.value);
			}
			else if (result.type == ValueType::Upper)
			{
				beta = min(beta, result.value);
			}
			if (alpha >= beta)
			{
				return result;
			}
		}
		else
		{
			for (int i = 0; i < state.freeSpots.size(); i++)
			{
				if (state.freeSpots[i].moveIndex > 0 && state.freeSpots[i].location == result.moveLocation)
				{
					bestMove = i;
				}
			}
			assert(bestMove != -1);
		}
	}

	bool localStop = false;
	int score = -999;
	int index = -1;
	if (bestMove != -1)
	{
		state.makeMove(state.freeSpots[bestMove].location.x, state.freeSpots[bestMove].location.y);
		auto result = alphaBeta(state, transpositionTable, depth - 1, -beta, -alpha, stop);
		state.undoMove();

		result.value = -result.value;
		nodesVisited += result.nodesVisited;

		if (result.value > score)
		{
			score = result.value;
			index = bestMove;
		}
		if (score > alpha) alpha = score;
		if (score >= beta) localStop = true;
	}

	if(!localStop)
	{
		for (int i = 0; i < state.freeSpots.size() && !stop; i++)
		{
			if (state.freeSpots[i].moveIndex > 0 && i != bestMove)
			{
				state.makeMove(state.freeSpots[i].location.x, state.freeSpots[i].location.y);
				auto result = alphaBeta(state, transpositionTable, depth - 1, -beta, -alpha, stop);
				state.undoMove();

				result.value = -result.value;
				nodesVisited += result.nodesVisited;

				if (result.value > score)
				{
					score = result.value;
					index = i;
				}
				if (score > alpha) alpha = score;
				if (score >= beta) break;
			}
		}
	}

	if (cacheFound || (depth >= 2 && transpositionTable.size() <= 100000000))
	{
		StateTreeResult result(score, nodesVisited, index);
		result.move = index;
		result.moveLocation = state.freeSpots[index].location;
		result.depth = depth;

		if (score <= olda)
		{
			result.type = ValueType::Upper;
		}
		else if (score >= beta)
		{
			result.type = ValueType::Lower;
		}
		else
		{
			result.type = ValueType::Exact;
		}

		transpositionTable.emplace(state.stateHash, result);
	}

	return StateTreeResult(score, nodesVisited, index);
}
