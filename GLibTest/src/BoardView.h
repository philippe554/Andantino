#pragma once

#include "GLib.h"

#define PI 3.14159265358979323846

#include "Search.h"
#include "SearchControl.h"

class BoardView : public GLib::View
{
public:
	using View::View;
	void init()
	{
		hexRadius = 20;
		hexHeight = hexRadius + sin(PI / 6) * hexRadius;
		hexWidth = cos(PI / 6) * hexRadius * 2;

		state.makeMove(10, 10);

		std::vector<std::pair<float, float>> hexagonPoints;

		for (int k = 0; k < 6; k++)
		{
			float a1 = (k * 60 + 90) * (PI / 180);
			float a2 = (k * 60 + 150) * (PI / 180);

			float x = hexWidth / 2 + cos(a1) * hexRadius;
			float y = hexRadius + sin(a1) * hexRadius;

			hexagonPoints.emplace_back(x, y);
		}

		hexagon = std::make_unique<GLib::Geometry>(hexagonPoints);

		addMouseListener(WM_MOUSEMOVE, [&](int x, int y)
		{
			//CODE : https://stackoverflow.com/a/7714148
			x -= hexXOffset;
			y -= hexYOffset;

			// Find the row and column of the box that the point falls in.
			int row = (int)(y / hexHeight);
			int column;

			boolean rowIsOdd = row % 2 == 1;

			// Is the row an odd number?
			if (rowIsOdd)// Yes: Offset x to match the indent of the row
				column = (int)((x - hexWidth / 2) / hexWidth);
			else// No: Calculate normally
				column = (int)(x / hexWidth);

			// Work out the position of the point relative to the box it is in
			double relY = y - (row * hexHeight);
			double relX;

			if (rowIsOdd)
				relX = (x - (column * hexWidth)) - hexWidth / 2;
			else
				relX = x - (column * hexWidth);

			double c = hexRadius - sin(PI / 6) * hexRadius;
			double m = c / (hexWidth / 2);

			// Work out if the point is above either of the hexagon's top edges
			if (relY < (-m * relX) + c) // LEFT edge
			{
				row--;
				if (!rowIsOdd)
					column--;
			}
			else if (relY < (m * relX) - c) // RIGHT edge
			{
				row--;
				if (rowIsOdd)
					column++;
			}

			selected = { column, row };
			return true;
		});

		addMouseListener(WM_LBUTTONDOWN, [&](int x, int y)
		{
			if (!state.isEndGame() && !searchControl)
			{
				if (0 <= selected.first && selected.first < XSIZE && 0 <= selected.second && selected.second < YSIZE)
				{
					if (state.isFreeSpot(selected.first, selected.second))
					{
						state.makeMove(selected.first, selected.second);
					}
				}
			}
			return true;
		});

		addView<GLib::Button>(800,50,150,50, [&]()
		{
			if (state.moves.size() > 1 && !searchControl)
			{
				state.undoMove();
			}
		}, " Undo");

		addView<GLib::Button>(800, 120, 150, 50, [&]()
		{
				if (!state.isEndGame())
				{
					if (!searchControl)
					{
						searchControl = std::make_unique<SearchControl>(state, 6000, 20);
					}
				}
		}, " Search");

		addView<GLib::Button>(800, 190, 150, 50, [&]()
			{
				if (searchControl)
				{
					searchControl.reset();
				}
			}, " Stop");
	}

	void render(GLib::RT* rt, GLib::Writer* w, GLib::Color* c, D2D1_RECT_F& visibleRect)
	{
		for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				if (state.staticMoves[i][j].player == Player::Empty && state.board.inBounds[i][j])
				{
					hexagon->fill(rt, c->get(193, 154, 107), hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
				}
				if (state.staticMoves[i][j].player == Player::P1)
				{
					hexagon->fill(rt, c->get(50, 50, 50), hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
				}
				if (state.staticMoves[i][j].player == Player::P2)
				{
					hexagon->fill(rt, c->get(200, 200, 200), hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
				}
				if (state.isFreeSpot(i, j))
				{
					hexagon->fill(rt, c->get(193, 255, 107), hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
				}
			}
		}

		if (state.staticMoves[state.moves.back().location.x][state.moves.back().location.y].player == Player::P1)
		{
			hexagon->fill(rt, c->get(0,0,0), 
				hexXOffset + state.moves.back().location.x * hexWidth 
				+ (state.moves.back().location.y % 2 * hexWidth / 2), 
				hexYOffset + state.moves.back().location.y * hexHeight);
		}
		if (state.staticMoves[state.moves.back().location.x][state.moves.back().location.y].player == Player::P2)
		{
			hexagon->fill(rt, c->get(255,255,255), 
				hexXOffset + state.moves.back().location.x * hexWidth 
				+ (state.moves.back().location.y % 2 * hexWidth / 2), 
				hexYOffset + state.moves.back().location.y * hexHeight);
		}

		/*for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				if (!state.board.inBounds[i][j])
				{
					hexagon->draw(rt, c->get(150, 150, 150), 2, hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
				}
			}
		}*/

		for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				if (state.board.inBounds[i][j])
				{
					hexagon->draw(rt, c->get(0, 0, 0), 2, hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
				}
			}
		}

		if (state.scores.size() > 0)
		{
			w->print("Black straigth: " + std::to_string(state.scores.back().straithP1), c->get(0, 0, 0), GLib::WriterFactory::getFont(14), { 800,300,1000,320 });
			w->print("White straigth: " + std::to_string(state.scores.back().straithP2), c->get(0, 0, 0), GLib::WriterFactory::getFont(14), { 800,320,1000,340 });
			w->print("Black circle: " + std::to_string(state.scores.back().hasCircleP1), c->get(0, 0, 0), GLib::WriterFactory::getFont(14), { 800,340,1000,360 });
			w->print("White circle: " + std::to_string(state.scores.back().hasCircleP2), c->get(0, 0, 0), GLib::WriterFactory::getFont(14), { 800,360,1000,380 });
			w->print("Black possible rows: " + std::to_string(state.scores.back().hasPossibleStraithP1), c->get(0, 0, 0), GLib::WriterFactory::getFont(14), { 800,380,1000,400 });
			w->print("White possible rows: " + std::to_string(state.scores.back().hasPossibleStraithP2), c->get(0, 0, 0), GLib::WriterFactory::getFont(14), { 800,400,1000,420 });
		}

		if (searchControl)
		{
			w->print("Calculating...", c->get(0, 0, 0), GLib::WriterFactory::getFont(14), { 800,500,1000,520 });
		}
		else
		{
			if (state.player == Player::P1)
			{
				w->print("Black's turn", c->get(0, 0, 0), GLib::WriterFactory::getFont(14), { 800,500,1000,520 });
			}
			else
			{
				w->print("White's turn", c->get(0, 0, 0), GLib::WriterFactory::getFont(14), { 800,500,1000,520 });
			}
		}

		w->print("Calc time: " + std::to_string((int)(totalCalculationTime / 1000)), c->get(0, 0, 0), GLib::WriterFactory::getFont(14), { 800,600,1000,620 });
	}

	void update() override
	{
		if (searchControl)
		{
			if (searchControl->isFinished())
			{
				auto result = searchControl->getResult();
				totalCalculationTime += searchControl->getTotalTime();
				GLib::Out << "Nodes visited: " << result.nodesVisited
					<< ",   Predicted score: " << result.value
					<< ",   Total time: " << searchControl->getTotalTime() << "ms"
					<< ",   Level reached: " << searchControl->getLevelReached() << "\n";

				auto location = state.freeSpots[result.move].location;
				state.makeMove(location.x, location.y);

				searchControl.reset();
			}
			else
			{
				searchControl->tick();
			}
		}
	}

private:
	D2D1_RECT_F background;

	float hexRadius;
	float hexHeight;
	float hexWidth;
	float hexXOffset = 0;
	float hexYOffset = 0;

	std::unique_ptr<GLib::Geometry> hexagon;

	std::pair<int, int> selected;

	State state;

	std::unique_ptr<SearchControl> searchControl;

	int totalCalculationTime = 0;
};