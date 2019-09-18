#include "GLibMain.h"

#include "BoardView.h"

void GLibMain(GLib::Frame* frame)
{
	frame->init("Test", 1000, 800);

	frame->addView<GLib::MainBar>(0, 0, -1, 50, "Test Page");

	frame->addView<BoardView>(0, 50);
}

/*#include <iostream>

#include "search.h"

void printState(State& state)
{
	std::cout << "STATE: next player: " << (state.player == Player::P1 ? "P1" : "P2") << "\n";

	std::cout << "Moves: ";
	for (auto move : state.moves)
	{
		std::cout << "[" << move.location.x << "," << move.location.y << "," << (move.player == Player::P1 ? "P1" : "P2") << "] ";
	}
	std::cout << "\n";

	std::cout << "Free: ";
	for (auto spot : state.freeSpots)
	{
		std::cout << "[" << spot.location.x << "," << spot.location.y << "," << spot.moveIndex << "] ";
	}
	std::cout << "\n\n";
}

int main()
{
	State state;

	state.makeMove(5, 5);

	printState(state);

	state.makeMove(4, 5);

	printState(state);

	state.makeMove(5, 6);

	printState(state);

	state.undoMove();

	printState(state);

	state.undoMove();

	printState(state);

	std::cin.get();
}*/



