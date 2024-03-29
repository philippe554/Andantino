#pragma once

#include "Search.h"
#include <thread>

class SearchControl
{
public:
	SearchControl(State& state, int _maxTime, int _maxDepth)
		: result(-9999, 0, -1)
	{
		maxTime = _maxTime;
		maxDepth = _maxDepth;
		start = std::chrono::steady_clock::now();

		for (int i = 0; i < state.freeSpots.size() && !stop; i++)
		{
			if (state.freeSpots[i].moveIndex > 0)
			{
				result.move = i;
				break;
			}
		}

		worker = std::make_unique<std::thread>(&SearchControl::workerFunction, this, std::ref(state));
	}
	~SearchControl()
	{
		forceStop();
		worker->join();
	}

	void tick()
	{
		auto end = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > maxTime)
		{
			forceStop();
		}
	}

	void forceStop()
	{
		stop = true;
	}

	bool isFinished()
	{
		return finished;
	}

	StateTreeResult getResult()
	{
		return result;
	}

	int getTotalTime()
	{
		return totalTime;
	}

	int getLevelReached()
	{
		return levelReached;
	}

private:
	void workerFunction(State& state)
	{
		std::map<StateHash, StateTreeResult> transpositionTable;
		long cacheHits = 0;

		for (int i = 1; i <= maxDepth; i++)
		{
			StateTreeResult newResult = alphaBeta(state, transpositionTable, i, -999, 999, stop, cacheHits);

			if (stop)
			{
				break;
			}
			else if (newResult.value == MaxScore)
			{
				result = newResult;
				levelReached = i;
				break;
			}
			else
			{
				result = newResult;
				levelReached = i;
			}
		}

		auto end = std::chrono::steady_clock::now();
		totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		//GLib::Out << "Cache hits: " << cacheHits << "\n";

		finished = true;
	}

private:
	bool stop = false;
	bool finished = false;
	int maxTime;
	int maxDepth;

	std::chrono::steady_clock::time_point start;

	std::unique_ptr<std::thread> worker;

	StateTreeResult result;
	int totalTime;
	int levelReached;
};