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
			if (0 <= selected.first && selected.first < 10 && 0 <= selected.second && selected.second)
			{
				if (state.isFreeSpot(selected.first, selected.second))
				{
					state.makeMove(selected.first, selected.second);
				}
			}
			return true;
		});

		addView<GLib::Button>(400,50,100,50, [&]()
		{
			if (state.moves.size() > 1)
			{
				state.undoMove();
			}
		}, "Undo");
	}

	void render(GLib::RT* rt, GLib::Writer* w, GLib::Color* c, D2D1_RECT_F& visibleRect)
	{
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				if (state.staticMoves[i][j].player == Player::Empty)
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

				D2D1_RECT_F rect1 = { hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight,
									 hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2) + hexWidth, hexYOffset + j * hexHeight + hexHeight };
				w->print(std::to_string(state.staticMoves[i][j].amountNeighbours), c->get(0, 0, 0), GLib::WriterFactory::getFont(12), rect1);

				D2D1_RECT_F rect2 = { hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2) + hexWidth * 0.5, hexYOffset + j * hexHeight,
									 hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2) + hexWidth, hexYOffset + j * hexHeight + hexHeight };
				w->print(std::to_string(state.staticMoves[i][j].freeSpotsIndex), c->get(0, 0, 0), GLib::WriterFactory::getFont(12), rect2);
			}
		}
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				hexagon->draw(rt, c->get(0, 0, 0), 2, hexXOffset + i * hexWidth + (j % 2 * hexWidth / 2), hexYOffset + j * hexHeight);
			}
		}

		if (state.scores.size() > 0)
		{
			w->print("Red straigth: " + std::to_string(state.scores.back().straithP1), c->get(0, 0, 0), GLib::WriterFactory::getFont(14, 500, "Courier New"), { 400,100,600,120 });
			w->print("Blue straigth: " + std::to_string(state.scores.back().straithP2), c->get(0, 0, 0), GLib::WriterFactory::getFont(14, 500, "Courier New"), { 400,120,600,140 });

			w->print("Red circle: " + std::to_string(state.scores.back().hasCircleP1), c->get(0, 0, 0), GLib::WriterFactory::getFont(14, 500, "Courier New"), { 400,140,600,160 });
			w->print("Blue circle: " + std::to_string(state.scores.back().hasCircleP2), c->get(0, 0, 0), GLib::WriterFactory::getFont(14, 500, "Courier New"), { 400,160,600,180 });
		}
	}

private:
	/*bool checkEncircled(Player playerInside)
	{
		bool placeChecked[10][10];

		for (int i = 1; i < 9; i++)
		{
			for (int j = 1; j < 9; j++)
			{
				if (state[i][j] == playerInside)
				{
					for (int i = 0; i < 10; i++)
					{
						for (int j = 0; j < 10; j++)
						{
							placeChecked[i][j] = false;
						}
					}

					bool v = searchWall(placeChecked, i, j, playerInside);

					if (!v)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	bool searchWall(bool placeChecked[][10], int x, int y, Player player)
	{
		placeChecked[x][y] = true;

		if (x == 0 || x == 9 || y == 0 || y == 9)
		{
			return true;
		}

		const auto& n = neighbours[x][y];
		
		for (auto place : n)
		{
			if (!placeChecked[place.first][place.second])
			{
				if (state[place.first][place.second] == Player::Empty || state[place.first][place.second] == player)
				{
					bool v = searchWall(placeChecked, place.first, place.second, player);
					if (v)
					{
						return true;
					}
				}
			}
		}

		return false;
	}*/

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