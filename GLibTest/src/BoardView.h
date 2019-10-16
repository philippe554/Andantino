#pragma once

#include "GLib.h"

#define PI 3.14159265358979323846

#include "Search.h"

class BoardView : public GLib::View
{
public:
	using View::View;
	void init()
	{
		hexRadius = 20;
		hexHeight = hexRadius + sin(PI / 6) * hexRadius;
		hexWidth = cos(PI / 6) * hexRadius * 2;

		state.makeMove(5, 5);

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
			if (0 <= selected.first && selected.first < XSIZE && 0 <= selected.second && selected.second < YSIZE)
			{
				if (state.isFreeSpot(selected.first, selected.second))
				{
					state.makeMove(selected.first, selected.second);
				}
			}
			return true;
		});

		addView<GLib::Button>(500,50,100,50, [&]()
		{
			if (state.moves.size() > 1)
			{
				state.undoMove();
			}
		}, "Undo");

		/*addView<GLib::Button>(550, 50, 100, 50, [&]()
		{
			Location location = alphaBetaHelper(state, 10);
			state.makeMove(location.x, location.y);
		}, "AlphaBeta");*/

		addView<GLib::Button>(700, 50, 100, 50, [&]()
		{
			auto start = std::chrono::steady_clock::now();
			long nodesVisited = 0;
			auto [location, score] = alphaBeta(state, nodesVisited, 10);
			state.makeMove(location.x, location.y);
			auto end = std::chrono::steady_clock::now();

			GLib::Out << "Alpha-beta nodes visited: " << nodesVisited 
				<< " | " << score 
				<< " | " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
		}, "MonteCarlo");
	}

	void render(GLib::RT* rt, GLib::Writer* w, GLib::Color* c, D2D1_RECT_F& visibleRect)
	{
		for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				if (state.staticMoves[i][j].player == Player::Empty && state.board.inBounds[i][j])
				{
					hexagon->fill(rt, c->get(255, 255, 255), hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
				}
				if (state.staticMoves[i][j].player == Player::P1)
				{
					hexagon->fill(rt, c->get(255, 0, 0), hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
				}
				if (state.staticMoves[i][j].player == Player::P2)
				{
					hexagon->fill(rt, c->get(0, 0, 255), hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
				}
				if (state.isFreeSpot(i, j))
				{
					hexagon->fill(rt, c->get(128, 128, 128), hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
				}
			}
		}

		if (state.staticMoves[state.moves.back().location.x][state.moves.back().location.y].player == Player::P1)
		{
			hexagon->fill(rt, c->get(255, 150, 150), 
				hexXOffset + state.moves.back().location.x * hexWidth 
				+ (state.moves.back().location.y % 2 * hexWidth / 2), 
				hexYOffset + state.moves.back().location.y * hexHeight);
		}
		if (state.staticMoves[state.moves.back().location.x][state.moves.back().location.y].player == Player::P2)
		{
			hexagon->fill(rt, c->get(150, 150, 255), 
				hexXOffset + state.moves.back().location.x * hexWidth 
				+ (state.moves.back().location.y % 2 * hexWidth / 2), 
				hexYOffset + state.moves.back().location.y * hexHeight);
		}


		for (int i = 0; i < XSIZE; i++)
		{
			for (int j = 0; j < YSIZE; j++)
			{
				hexagon->draw(rt, c->get(0, 0, 0), 2, hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
			}
		}

		if (state.scores.size() > 0)
		{
			w->print("Red straigth: " + std::to_string(state.scores.back().straithP1), c->get(0, 0, 0), GLib::WriterFactory::getFont(14, 500, "Courier New"), { 500,100,700,120 });
			w->print("Blue straigth: " + std::to_string(state.scores.back().straithP2), c->get(0, 0, 0), GLib::WriterFactory::getFont(14, 500, "Courier New"), { 500,120,700,140 });

			w->print("Red circle: " + std::to_string(state.scores.back().hasCircleP1), c->get(0, 0, 0), GLib::WriterFactory::getFont(14, 500, "Courier New"), { 500,140,700,160 });
			w->print("Blue circle: " + std::to_string(state.scores.back().hasCircleP2), c->get(0, 0, 0), GLib::WriterFactory::getFont(14, 500, "Courier New"), { 500,160,700,180 });
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
};