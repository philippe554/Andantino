#include "..\include\GLib.h"
#include "..\include\GLib.h"
#include "..\include\GLib.h"
#include "../include/GLib.h"

namespace GLib
{
	TextEditLine* TextEditLine::hasFocus = nullptr;

	void TextEditLine::init(boolean _fixedSize)
	{
		fixedSize = _fixedSize;
		startSize = place.right - place.left;

		updateLocations();

		addMouseListener(WM_LBUTTONDOWN, [&](int x, int y)
		{
			if (place.left <= x && x <= place.right && place.top <= y && y <= place.bottom)
			{
				hasFocus = this;

				float closest = 0;
				float closestIndex = -1;
				for (int i = 0; i < characterLocations.size(); i++)
				{
					if (closestIndex == -1 || std::abs(characterLocations.at(i) - x + 10) < closest)
					{
						closestIndex = i;
						closest = std::abs(characterLocations.at(i) - x + 10);
					}
				}

				if (closestIndex != -1)
				{
					cursorIndex = closestIndex;
					auto now = std::chrono::high_resolution_clock::now();
					msLastMoved = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
					selecting = false;
					mouseDown = true;
					updateCursor();
				}
				return true;
			}
			else
			{
				if (hasFocus == this)
				{
					hasFocus = nullptr;
				}
			}
			return false;
		});

		addMouseListener(WM_LBUTTONDBLCLK, [&](int x, int y)
		{
			if (place.left <= x && x <= place.right && place.top <= y && y <= place.bottom)
			{
				selecting = true;
				from = 0;
				to = characterLocations.size() - 1;
				return true;
			}
			return false;
		});

		addMouseListener(WM_LBUTTONUP, [&](int x, int y)
		{
			if (hasFocus == this)
			{
				mouseDown = false;
				if (place.left <= x && x <= place.right && place.top <= y && y <= place.bottom)
				{
					float closest = 0;
					float closestIndex = -1;
					for (int i = 0; i < characterLocations.size(); i++)
					{
						if (closestIndex == -1 || std::abs(characterLocations.at(i) - x + 10) < closest)
						{
							closestIndex = i;
							closest = std::abs(characterLocations.at(i) - x + 10);
						}
					}

					if (closestIndex != -1)
					{
						if (!selecting)
						{
							cursorIndex = closestIndex;
							auto now = std::chrono::high_resolution_clock::now();
							msLastMoved = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
						}
						updateCursor();
					}
					return true;
				}
			}
			return false;
		});

		addMouseListener(WM_MOUSEMOVE, [&](int x, int y)
		{
			if (mouseDown && hasFocus == this && !selecting)
			{
				if (place.left <= x && x <= place.right && place.top <= y && y <= place.bottom)
				{
					float closest = 0;
					float closestIndex = -1;
					for (int i = 0; i < characterLocations.size(); i++)
					{
						if (closestIndex == -1 || std::abs(characterLocations.at(i) - x + 10) < closest)
						{
							closestIndex = i;
							closest = std::abs(characterLocations.at(i) - x + 10);
						}
					}

					if (closestIndex != -1)
					{
						if (closestIndex != cursorIndex)
						{
							selecting = true;
							from = cursorIndex;
							to = closestIndex;
							updateCursor();
						}
					}
					return true;
				}
			}
			return false;
		});

		addMouseListener(WM_SETCURSOR, [&](int x, int y)
		{
			auto mouse = getMousePosition();
			if (place.left <= mouse.first && mouse.first <= place.right && place.top <= mouse.second && mouse.second <= place.bottom)
			{
				HCURSOR cursor = LoadCursor(NULL, IDC_IBEAM);
				SetCursor(cursor);
				return true;
			}
			return false;
		});
	}
	void TextEditLine::render(RT* rt, Writer* w, Color* c, D2D1_RECT_F& visibleRect)
	{
		float xSize = place.right - place.left;
		float ySize = place.bottom - place.top;
		D2D1_RECT_F background = D2D1::RectF(0, 0, xSize, ySize);
		rt->FillRectangle(background, c->get(C::Black));

		if (selecting)
		{
			D2D1_RECT_F selectArea;
			selectArea.top = 10;
			selectArea.bottom = place.bottom - place.top - 10;
			selectArea.left = characterLocations.at(from) + 10;
			selectArea.right = characterLocations.at(to) + 10;
			rt->FillRectangle(selectArea, c->get(C::Gray));
		}

		for (int i = 0; i < text.length(); i++)
		{
			D2D1_RECT_F spot = background;
			spot.left += characterLocations.at(i) + 10;

			w->printCharacter(text.at(i), c->get(C::White), WriterFactory::getFont(14), spot);
		}

		if (hasFocus == this)
		{
			if (selecting)
			{
				rt->DrawLine({ (float)(int)characterLocations.at(from) + 10, 10.0 },
					{ (float)(int)characterLocations.at(from) + 10, place.bottom - place.top - 10 }, c->get(C::White));
				rt->DrawLine({ (float)(int)characterLocations.at(to) + 10, 10.0 },
					{ (float)(int)characterLocations.at(to) + 10, place.bottom - place.top - 10 }, c->get(C::White));
			}
			else
			{
				auto now = std::chrono::high_resolution_clock::now();
				auto msNow = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

				if ((msNow - msLastMoved) / 500 % 2 == 0)
				{
					rt->DrawLine({ (float)(int)characterLocations.at(cursorIndex) + 10, 10.0 },
						{ (float)(int)characterLocations.at(cursorIndex) + 10, place.bottom - place.top - 10 }, c->get(C::White));
				}
			}
		}
	}
	void TextEditLine::update()
	{
		if (mouseDown && hasFocus == this && selecting)
		{
			auto x = getMousePosition().first;
			auto y = getMousePosition().second;

			if (place.left <= x && x <= place.right && place.top <= y && y <= place.bottom)
			{
				float closest = 0;
				float closestIndex = -1;
				for (int i = 0; i < characterLocations.size(); i++)
				{
					if (closestIndex == -1 || std::abs(characterLocations.at(i) - x + 10) < closest)
					{
						closestIndex = i;
						closest = std::abs(characterLocations.at(i) - x + 10);
					}
				}

				if (closestIndex != -1)
				{
					to = closestIndex;
					updateCursor();
				}
			}
		}
	}
	void TextEditLine::winEvent(Frame* frame, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if(message == WM_KEYDOWN && hasFocus == this)
		{
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				if(wParam == 0x56) // V
				{
					HWND hWnd = 0;
					if (!IsClipboardFormatAvailable(CF_TEXT))
						return;
					if (!OpenClipboard(hWnd))
						return;

					HGLOBAL hglb = GetClipboardData(CF_TEXT);
					if (hglb != NULL)
					{
						LPTSTR lptstr;
						lptstr = (LPTSTR)GlobalLock(hglb);
						if (lptstr != NULL)
						{
							removeSelected();

							auto data = std::string(lptstr);
							data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
							data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
							text.insert(cursorIndex, data);
							cursorIndex += data.length();

							GlobalUnlock(hglb);
						}
					}
					CloseClipboard();
					updateLocations();
				}
				if (wParam == 0x43) // C
				{
					HWND hWnd = 0;
					if (!OpenClipboard(hWnd))
						return;
					EmptyClipboard();

					HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(TCHAR));
					if (hglb != NULL)
					{
						LPTSTR lptstr;
						lptstr = (LPTSTR)GlobalLock(hglb);
						if (lptstr != NULL)
						{
							if (selecting)
							{
								if (to < from)
								{
									memcpy(lptstr, text.substr(to, from - to).c_str(), text.length() * sizeof(TCHAR));
								}
								else
								{
									memcpy(lptstr, text.substr(from, to - from).c_str(), text.length() * sizeof(TCHAR));
								}
							}
							else
							{
								memcpy(lptstr, text.c_str(), text.length() * sizeof(TCHAR));
							}

							GlobalUnlock(lptstr);
						}
					}
					SetClipboardData(CF_TEXT, hglb);

					CloseClipboard();
					updateLocations();
				}
				if (wParam == 0x58) // X
				{
					HWND hWnd = 0;
					if (!OpenClipboard(hWnd))
						return;
					EmptyClipboard();

					HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(TCHAR));
					if (hglb != NULL)
					{
						LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
						if (lptstr != NULL)
						{
							if (selecting)
							{
								if (to < from)
								{
									std::swap(from, to);
								}
								memcpy(lptstr, text.substr(from, to - from).c_str(), text.length() * sizeof(TCHAR));
								text.erase(text.begin() + from, text.begin() + to);
								cursorIndex = from;
								selecting = false;
							}
							else
							{
								memcpy(lptstr, text.c_str(), text.length() * sizeof(TCHAR));
								text.clear();
								cursorIndex = 0;
								selecting = false;
							}

							GlobalUnlock(lptstr);
						}
					}
					SetClipboardData(CF_TEXT, hglb);

					CloseClipboard();
					updateLocations();
				}
				if (wParam == 0x41) // A
				{
					selecting = true;
					from = 0;
					to = characterLocations.size() - 1;
				}
			}
			else if (wParam == VK_LEFT)
			{
				bool shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
				if (selecting)
				{
					if (shiftPressed)
					{
						if (to > 0)
						{
							to--;
						}
					}
					else
					{
						if (to < from)
						{
							std::swap(from, to);
						}
						cursorIndex = from;
						selecting = false;
					}
				}
				else
				{
					if (cursorIndex > 0)
					{
						if (shiftPressed)
						{
							selecting = true;
							from = cursorIndex;
							to = cursorIndex - 1;
						}
						else
						{
							cursorIndex--;
						}
					}
				}
				updateCursor();
			}
			else if (wParam == VK_RIGHT)
			{
				bool shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
				if (selecting)
				{
					if (shiftPressed)
					{
						if (to < characterLocations.size() - 1)
						{
							to++;
						}
					}
					else
					{
						if (to < from)
						{
							std::swap(from, to);
						}
						cursorIndex = to;
						selecting = false;
					}
				}
				else
				{
					if (cursorIndex < characterLocations.size() - 1)
					{
						if (shiftPressed)
						{
							selecting = true;
							from = cursorIndex;
							to = cursorIndex + 1;
						}
						else
						{
							cursorIndex++;
						}
					}
				}
				updateCursor();
			}
			else if (wParam == VK_BACK)
			{
				if (selecting)
				{
					removeSelected();
				}
				else
				{
					if (text.length() > 0 && cursorIndex > 0)
					{
						text.erase(text.begin() + cursorIndex - 1);
						cursorIndex--;
					}
				}
				updateLocations();
			}
			else if (wParam == VK_DELETE)
			{
				if (selecting)
				{
					removeSelected();
				}
				else
				{
					if (cursorIndex < text.length())
					{
						text.erase(cursorIndex, 1);
					}
				}
				updateLocations();
			}
			else if (wParam == VK_SPACE)
			{
				removeSelected();
				text.insert(text.begin() + cursorIndex, ' ');
				cursorIndex++;
				updateLocations();
			}
			else if (wParam == VK_HOME)
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					if (selecting)
					{
						to = 0;
					}
					else
					{
						selecting = true;
						from = cursorIndex;
						to = 0;
					}
				}
				else
				{
					selecting = false;
					cursorIndex = 0;
				}
				updateCursor();
			}
			else if (wParam == VK_END)
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					if (selecting)
					{
						to = characterLocations.size() - 1;
					}
					else
					{
						selecting = true;
						from = cursorIndex;
						to = characterLocations.size() - 1;
					}
				}
				else
				{
					selecting = false;
					cursorIndex = characterLocations.size() - 1;
				}
				updateCursor();
			}
			else if (wParam == VK_RETURN)
			{
				if (enterCallback)
				{
					enterCallback(text);
				}
			}
			else if (wParam == VK_ESCAPE)
			{
			}
			else
			{
				if (selecting)
				{
					removeSelected();
				}
				BYTE kb[256];
				GetKeyboardState(kb);
				WCHAR uc[5] = {};
				if (ToUnicode(wParam, MapVirtualKey(wParam, MAPVK_VK_TO_VSC), kb, uc, 4, 0) == 1)
				{
					char letter = uc[0];
					text.insert(text.begin() + cursorIndex, letter);
					cursorIndex++;
				}
				updateLocations();
			}
		}
	}
	void TextEditLine::updateCursor()
	{
		MovingView* m = dynamic_cast<MovingView*>(this->getParentView()->getParentView()->getParentView());
		if (m)
		{
			m->makeVissible({ getCursorLocation() - 10, 0, getCursorLocation() + 10, place.bottom - place.top });
		}
		else
		{
			GLib::Out << "Cast failed\n";
		}

		auto now = std::chrono::high_resolution_clock::now();
		msLastMoved = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	}
	void TextEditLine::updateLocations()
	{
		characterLocations.clear();

		float location = 0;
		for (int i = 0; i < text.length(); i++)
		{
			characterLocations.push_back(location);
			location += WriterFactory::getMetric(text[i], WriterFactory::getFont(14)).widthIncludingTrailingWhitespace;
		}

		characterLocations.push_back(location);	
	
		if (fixedSize)
		{
			while (characterLocations.size() > 0 && characterLocations.back() + 20 > startSize)
			{
				text.pop_back();
				characterLocations.pop_back();
			}

			if (selecting)
			{
				from = min(from, characterLocations.size() - 1);
				to = min(to, characterLocations.size() - 1);
			}
			else
			{
				cursorIndex = min(cursorIndex, characterLocations.size() - 1);
			}
		}
		else
		{
			if (characterLocations.size() == 0)
			{
				place.right = startSize;
			}
			else
			{
				place.right = max(startSize, characterLocations.back() + 20);
			}
		}

		updateCursor();

		if (updateCallback)
		{
			updateCallback(text);
		}
	}

	float TextEditLine::getCursorLocation()
	{
		if (characterLocations.size() == 0)
		{
			return 10;
		}
		else
		{
			if (selecting)
			{
				return characterLocations.at(to) + 10;
			}
			else
			{
				return characterLocations.at(cursorIndex) + 10;
			}
		}
	}

	void TextEditLine::removeSelected()
	{
		if (selecting)
		{
			if (to < from)
			{
				std::swap(from, to);
			}

			text.erase(text.begin() + from, text.begin() + to);
			cursorIndex = from;
			selecting = false;
		}
	}

	void TextEditLine::addUpdateCallback(std::function<void(std::string)> _updateCallback)
	{
		updateCallback = _updateCallback;
	}
	void TextEditLine::addEnterCallback(std::function<void(std::string)> _enterCallback)
	{
		enterCallback = _enterCallback;
	}

	std::string TextEditLine::getText()
	{
		return text;
	}

	void TextEditLine::giveFocus()
	{
		hasFocus = this;
	}
	void TextEditLine::removeFocus()
	{
		if (hasFocus == this)
		{
			hasFocus = nullptr;
		}
	}

	void TextBox::init(std::string start)
	{
		MovingView::init(true, false);

		int textEditLineWidth = place.right - place.left;
		int textEditLineHeight = place.bottom - place.top - 20;
		textEditLine = getMovingView()->addView<TextEditLine>(0, 0, textEditLineWidth, textEditLineHeight, false);
	}

	void TextBox::update()
	{
		MovingView::update();

		/*bool doResize = false;

		float xSize = getMovingView()->place.right - getMovingView()->place.left;

		if (abs(textEditLine->place.right - xSize) > 0.001)
		{
			getMovingView()->place.right = getMovingView()->place.left + textEditLine->place.right;
			doResize = true;
		}

		if (textEditLine->textUpdated)
		{
			textEditLine->textUpdated = false;
			float cursorLocation = textEditLine->getCursorLocation() + getMovingView()->place.left;

			if (cursorLocation < 10)
			{
				getMovingView()->move(-cursorLocation + 10, 0);
				doResize = true;
			}

			if (place.right - place.left < cursorLocation + 10)
			{
				float dif = cursorLocation - place.right + place.left + 10;
				getMovingView()->move(-dif, 0);
				doResize = true;
			}

			if (getMovingView()->place.right < place.right - place.left)
			{
				float dif = place.right - place.left - getMovingView()->place.right;
				getMovingView()->move(dif, 0);
				doResize = true;
			}
		}

		if (doResize)
		{
			resize(textEditLine->place.right, textEditLine->place.bottom);
		}*/
	}

	void TextBox::addUpdateCallback(std::function<void(std::string)> _updateCallback)
	{
		textEditLine->addUpdateCallback(_updateCallback);
	}
	void TextBox::addEnterCallback(std::function<void(std::string)> _enterCallback)
	{
		textEditLine->addEnterCallback(_enterCallback);
	}

	std::string TextBox::getText()
	{
		return textEditLine->getText();
	}

	void TextBox::giveFocus()
	{
		textEditLine->giveFocus();
	}
	void TextBox::removeFocus()
	{
		textEditLine->removeFocus();
	}
}