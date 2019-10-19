#include "GLibMain.h"

#include "BoardView.h"

void GLibMain(GLib::Frame* frame)
{
	frame->init("Test", 1000, 800);

	frame->addView<GLib::MainBar>(0, 0, -1, 50, "Andantino");

	auto tabs = frame->addView<GLib::TabView>(0, 50);

	auto game = tabs->getNewTab(" Game");
	game->addView<BoardView>(0, 50);

	tabs->getNewTab(" Console")
		->addView<GLib::MovingView>(0, 0, -1, -1, false, true)
		->getMovingView()
		->addView<GLib::OutputView>()
		->setDefault();
}

/*#include <iostream>
#include <chrono>

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
	state.makeMove(4, 5);

	printState(state);
	state.makeMove(5, 4);
	state.undoMove();
	printState(state);


	//long nodesVisited = 0;

	//auto start = std::chrono::steady_clock::now();
	//std::cout << "result: " << alphaBeta(state, nodesVisited, 16, -999, 999) << "\n";
	//auto end = std::chrono::steady_clock::now();

	//std::cout << "Ready: " << nodesVisited << " nodes visited in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
	std::cin.get();
}*/



