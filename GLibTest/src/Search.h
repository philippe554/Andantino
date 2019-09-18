#pragma once

#include <vector>
#include <stdexcept>
#include <cassert>
#include <array>

enum Player
{
	P1, P2, Empty
};

enum Side
{
	right, topRight, topLeft, left, bottomLeft, bottomRight
};

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
};

struct Move
{
	Location location;
	Player player;
};

struct FreeSpot
{
	Location location;
	int moveIndex;
};

struct Score
{
	int straithP1 = 0;
	int straithP2 = 0;

	bool hasCircleP1 = false;
	bool hasCircleP2 = false;
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
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				if (i > 0) neighbours[i][j][Side::left] = { i - 1, j };
				if (i < 9) neighbours[i][j][Side::right] = { i + 1, j };

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
						if (i < 9) neighbours[i][j][Side::topRight] = { i + 1, j - 1 };
					}
				}

				if (j < 9)
				{
					if (j % 2 == 0)
					{
						if (i > 0) neighbours[i][j][Side::bottomLeft] = { i - 1, j + 1 };
						neighbours[i][j][Side::bottomRight] = { i, j + 1 };
					}
					else
					{
						neighbours[i][j][Side::bottomLeft] = { i, j + 1 };
						if (i < 9) neighbours[i][j][Side::bottomRight] = { i + 1, j + 1 };
					}
				}
			}
		}
	};

	std::array<Location, 6> neighbours[10][10];
};

class State
{
public:
	State()
	{
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
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
					staticMoves[location.x][location.y].amountNeighbours--;
				}
			}

			{
				const auto& neighbours = board.neighbours[moves[0].location.x][moves[0].location.y];
				for (const auto& location : neighbours)
				{

					freeSpots.push_back({ location, moveIndex });
					staticMoves[location.x][location.y].freeSpotsIndex = freeSpots.size() - 1;
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
				staticMoves[location.x][location.y].amountNeighbours--;
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
		if (staticMoves[location.x][location.y].player == player)
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
		return false;
	}

	int evaluate()
	{
		return 0;
	}

	Player player = Player::P1;
	int moveIndex = 0;

	std::vector<Move> moves;
	std::vector<FreeSpot> freeSpots;

	StaticLocation staticMoves[10][10];

	Board board;

	std::vector<Score> scores;
};

int search(State& state, int depth)
{
	int score = state.evaluate();

	if (score != 0)
	{
		return score;
	}

	if (state.player == Player::P1)
	{
		int bestScore = -999;
		for (int i = 0; i < state.freeSpots.size(); i++)
		{
			if (state.freeSpots[i].moveIndex > 0)
			{
				state.makeMove(state.freeSpots[i].location.x, state.freeSpots[i].location.y);

				int t = search(state, depth + 1);
				if (t > bestScore) bestScore = t;

				state.undoMove();
			}
		}
		return bestScore;
	}
	else
	{
		int bestScore = 999;
		for (int i = 0; i < state.freeSpots.size(); i++)
		{
			if (state.freeSpots[i].moveIndex > 0)
			{
				state.makeMove(state.freeSpots[i].location.x, state.freeSpots[i].location.y);

				int t = search(state, depth + 1);
				if (t < bestScore) bestScore = t;

				state.undoMove();
			}
		}
		return bestScore;
	}
}