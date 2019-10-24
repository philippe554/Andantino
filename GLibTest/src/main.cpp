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
/*
#include <iostream>
#include <chrono>
#include <thread>
#include <random>

#include "search.h"
#include "SearchControl.h"

Player simulate(int t1, int t2, int& p1moves, int&p2moves, int& p1sumdepth, int& p2sumdepth)
{
	State state;

	state.makeMove(10, 10);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, 4);

	int counter = 0;
	while(counter < 4)
	{
		for (int i = 0; i < state.freeSpots.size(); i++)
		{
			if (state.freeSpots[i].moveIndex > 0)
			{
				if (dis(gen) == 1)
				{
					state.makeMove(state.freeSpots[i].location.x, state.freeSpots[i].location.y);
					counter++;
					break;
				}
			}
		}
	}

	counter = 0;
	while (!state.isEndGame() && counter < 200)
	{
		auto searchControl = std::make_unique<SearchControl>(state, state.player == Player::P1 ? t1 : t2, 20);

		while (!searchControl->isFinished())
		{
			searchControl->tick();
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}

		auto result = searchControl->getResult();

		if (state.player == Player::P1)
		{
			p1moves++;
			p1sumdepth += result.depth;
		}
		else
		{
			p2moves++;
			p2sumdepth += result.depth;
		}

		auto location = state.freeSpots[result.move].location;
		state.makeMove(location.x, location.y);
		counter++;
	}

	if (counter == 200)
	{
		std::cout << "Long game: " << counter;
	}

	return getOtherPlayer(state.player);
}

int main()
{
	int p1moves = 0;
	int p2moves = 0;
	int p1sumdepth = 0;
	int p2sumdepth = 0;

	int wins[2] = {0, 0};

	for (int i = 0; i < 100000; i++)
	{
		auto r = simulate(100, 100, p1moves, p2moves, p1sumdepth, p2sumdepth);
		wins[r]++;
		std::cout << i << " : " << wins[0] << " <-> " << wins[1] << " : " << float(wins[0]) / float(wins[0] + wins[1])
			<< " (" << float(p1sumdepth) / float(p1moves) << "|" << float(p2sumdepth) / float(p2moves) << ")\n";
	}


	std::cin.get();
}
*/

