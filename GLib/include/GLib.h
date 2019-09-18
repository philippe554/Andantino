#pragma once

#include <vector>
#include <map>
#include <string>
#include <functional>
#include <stdexcept>
#include <chrono>
#include <algorithm>
#include <thread>

#include <Windows.h>
#include <Windowsx.h>
#include <d2d1.h>
#include <dwrite.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace GLib
{
	typedef ID2D1HwndRenderTarget RT;
	typedef D2D1::ColorF C;

	class View;
	class Frame;
	class Color;
	class Writer;
	class MovingBar;
	class MainBar;
	class OutputView;
	struct OutputForward;

	class View
	{
	public:
		View(View* parent, int x, int y, int width, int height);
		virtual void init() {};
		virtual ~View();

		template <typename R, typename... Ts> R* addView(Ts&&... args)
		{
			R* e = new R(this, 0, 0, -1, -1);
			e->parentView = this;
			e->init(std::forward<Ts>(args)...);
			subViews.push_back(e);
			return e;
		}

		template <typename R, typename... Ts> R* addView(int x, int y, Ts&&... args)
		{
			R* e = new R(this, x, y, -1, -1);
			e->parentView = this;
			e->init(std::forward<Ts>(args)...);
			subViews.push_back(e);
			return e;
		}

		template <typename R, typename... Ts> R* addView(int x, int y, int width, int height, Ts&&... args)
		{
			R* e = new R(this, x, y, width, height);
			e->parentView = this;
			e->init(std::forward<Ts>(args)...);
			subViews.push_back(e);
			return e;
		}

		void addMouseListener(int type, std::function<bool(int, int)> f);

		std::pair<int, int> getMousePosition();

		virtual void parentResized(D2D1_RECT_F p) {};
		View* getParentView();
		virtual Frame* getFrame();

		void move(float x, float y);

	public:

		D2D1_RECT_F place;

		bool activateFlag = true;
		bool renderFlag = true;
		bool updateFlag = true;
		bool winEventFlag = true;
		bool mouseEventFlag = true;

		std::vector<View*> subViews;

	protected:
		void renderControl(RT* rt, Writer* w, Color* c, int x, int y, D2D1_RECT_F& visibleRect);
		void updateControl();
		void winEventControl(Frame* frame, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		bool mouseEventControl(Frame* frame, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		virtual void render(RT* rt, Writer* w, Color* c, D2D1_RECT_F& visibleRect) {};
		virtual void update() {};
		virtual void winEvent(Frame* frame, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {};
		virtual void worker() {};

		bool forwardMouseEvent(int type, int x, int y);

	private:
		View* parentView = nullptr;

		std::map<int, std::function<bool(int, int)>> mouseFunctions;

		int mouseX = 0;
		int mouseY = 0;
	};

	class Writer
	{
	public:
		void init(RT* _rt);
		void print(std::string text, ID2D1SolidColorBrush*color, IDWriteTextFormat*font, D2D1_RECT_F place);
		void printCharacter(char text, ID2D1SolidColorBrush* color, IDWriteTextFormat* font, D2D1_RECT_F place);
		
	private:
		RT* rt;
	};

	class WriterFactory
	{
	public:
		static void setup();

		static IDWriteTextFormat* getFont(int size, int weight = DWRITE_FONT_WEIGHT_LIGHT, std::string font = "Verdana");
		static DWRITE_TEXT_METRICS getMetric(char letter, IDWriteTextFormat* font);

	private:
		static IDWriteFactory* writeFactory;
		static std::map<std::tuple<int, int, std::string>, IDWriteTextFormat*> fonts;
		static std::map<std::tuple<char, IDWriteTextFormat*>, DWRITE_TEXT_METRICS> charMetrics;
	};

	class Color
	{
	public:
		void init(ID2D1HwndRenderTarget* rt);

		ID2D1SolidColorBrush* get(int i);
		ID2D1SolidColorBrush* get(int r, int g, int b);

	private:
		ID2D1HwndRenderTarget* renderTarget;
		std::map<int, ID2D1SolidColorBrush*> data;
	};

	class Geometry
	{
	public:
		Geometry(std::vector<std::pair<float, float>> data);
		~Geometry();

		static void setFactory(ID2D1Factory* factory);

		void draw(RT* rt, ID2D1SolidColorBrush* brush, int width, float x, float y);
		void fill(RT* rt, ID2D1SolidColorBrush* brush, float x, float y);

	private:
		static ID2D1Factory* Direct2dFactory;

		ID2D1PathGeometry* geometry;
	};

	class Frame : public View
	{
	public:
		using View::View;
		//Frame();
		void init(std::string name, int width, int height);
		void runMessageLoop();

		virtual Frame* getFrame();

		float getLastPaintTime();
		float getLastUpdateTime();

		static void askRepaint();
		static void showWindow(int mode);
		static void closeWindow();

		static HWND getHWND();

	private:
		static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		HRESULT createDeviceIndependentResources();
		HRESULT createDeviceResources();

		static bool repaint;
		static bool close;

		bool continues = true;
		bool mouseTracked = false;
		TRACKMOUSEEVENT tme;

		static HWND hwnd;
		RT* rt;
		ID2D1Factory* Direct2dFactory;
		Writer writer;
		Color color;

		float paintTime = 0;
		float updateTime = 0;
		int fps = 0;
	};

	class Button : public View
	{
	public:
		using View::View;
		void init(std::function<void()> _onClick, std::string _title = "");

		void render(RT * rt, Writer * w, Color * c, D2D1_RECT_F& visibleRect) override;

		void setHorizontalDragable(int _maxLeft, int _maxRight, std::function<void(float)> _onVerticalDrag);
		void moveHorizontalPlace(int i);
		float getHorizontalRatio();

		void setVerticalDragable(int _maxTop, int _maxBotton, std::function<void(float)> _onVerticalDrag);
		void moveVerticalPlace(int i);
		float getVerticalRatio();

		bool activated = true;

		int maxLeft;
		int maxRight;
		int maxTop;
		int maxBottom;

		D2D1_RECT_F box;

	private:
		std::string title;
		D2D1_RECT_F titleBox;
		std::function<void()> onClick;
		int state = 0;

		bool isDragging = false;

		bool horizontalDrag = false;
		std::function<void(float)> onHorizontalDrag;
		int horizontalStart;

		bool verticalDrag = false;
		std::function<void(float)> onVerticalDrag;
		int verticalStart;
	};

	class CheckBox : public View
	{
	public:
		using View::View;
		void init(std::function<void(bool state)> _onClick, bool default);

		void render(RT * rt, Writer * w, Color * c, D2D1_RECT_F& visibleRect) override;

		bool getState();

	private:
		std::function<void(bool state)> onClick;
		int state = 0;
		bool activated = true;
		D2D1_RECT_F box1;
		D2D1_RECT_F box2;
		D2D1_RECT_F box3;
	};

	class Slider : public View
	{
	public:
		using View::View;	
		void init(std::function<void(float ratio)> _onClick, float default);

		void render(RT * rt, Writer * w, Color * c, D2D1_RECT_F& visibleRect) override;

		float getRatio();
	private:
		Button* left;
		Button* right;
		Button* bar;

		D2D1_RECT_F box1;
		D2D1_RECT_F box2;

		std::function<void(float ratio)> onClick;
	};

	class MovingView : public View
	{
	public:
		using View::View;

		void init(bool _horizontal, bool _vertical);

		void update() override;
		void winEvent(Frame * frame, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

		View* getMovingView();

		void setScrollZoom(bool _horizontal, bool _vertical);

		void makeVissible(D2D1_RECT_F box);

	protected:
		void resize(float horizontalSize, float verticalSize);

	private:
		void setup(int xSize, int ySize);

		View* staticView;
		View* movingView;

		bool vertical;
		bool horizontal;

		bool verticalScrollZoom;
		bool horizontalScrollZoom;

		Button* left;
		Button* right;
		Button* horizontalBar;

		Button* up;
		Button* down;
		Button* verticalBar;

		int buttonSize = 20;
		int spaceSize = 5;
	};

	class TextEditLine : public View
	{
	public:
		using View::View;
		void init(boolean _fixedSize = true);

		void render(RT* rt, Writer* w, Color* c, D2D1_RECT_F& visibleRect) override;
		void update() override;
		void winEvent(Frame* frame, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

		float getCursorLocation();
		void addUpdateCallback(std::function<void(std::string)> _updateCallback);
		void addEnterCallback(std::function<void(std::string)> _enterCallback);
		std::string getText();
		void giveFocus();
		void removeFocus();

		bool textUpdated = false;

	private:
		void updateCursor();
		void updateLocations();
		void removeSelected();

	private:
		std::string text;

		long msLastMoved = 0;
		int cursorIndex = 0;
		std::vector<float> characterLocations;

		bool mouseDown = false;
		bool selecting = false;
		int from = 0;
		int to = 0;

		bool fixedSize;
		int startSize;

		std::function<void(std::string)> updateCallback;
		std::function<void(std::string)> enterCallback;

		static TextEditLine* hasFocus;
	};

	class TextBox : public MovingView
	{
	public:
		using MovingView::MovingView;
		void init(std::string start = "");
		void update() override;

		void addUpdateCallback(std::function<void(std::string)> _updateCallback);
		void addEnterCallback(std::function<void(std::string)> _enterCallback);
		std::string getText();
		void giveFocus();
		void removeFocus();

	private:
		TextEditLine* textEditLine;
	};

	class TabView : public View
	{
	public:
		using View::View;
		View* getNewTab(std::string name);

		void render(RT * rt, Writer * w, Color * c, D2D1_RECT_F& visibleRect) override;

	private:
		std::vector<View*> tabs;
		int currentTab = -1;
	};

	class MainBar : public View
	{
	public:
		using View::View;
		void init(std::string _title = "");

		void render(RT* rt, Writer* w, Color* c, D2D1_RECT_F& visibleRect) override;

	private:
		D2D1_RECT_F background;
		std::string title;
		D2D1_RECT_F titleBox;
		Button* closeButton;
		bool move = false;
		POINT movePoint;
	};

	class FrameStats : public View
	{
	public:
		using View::View;
		void init();

		void render(RT* rt, Writer* w, Color* c, D2D1_RECT_F& visibleRect) override;
	private:
		D2D1_RECT_F background;
		Frame* frame;
	};

	class OutputView : public View
	{
	public:
		using View::View;
		void init();

		void render(RT* rt, Writer* w, Color* c, D2D1_RECT_F& visibleRect) override;
		void update() override;
		void winEvent(Frame* frame, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

		void setDefault();
		void write(std::string s);

	private:
		std::vector<std::string> text;

		int xOffset = 10;
		int yOffset = 10;
		int fontSize = 14;
	};

	struct OutputForward
	{
	public:
		OutputView* outputView = nullptr;
	};

	extern OutputForward Out;

	template<class T> OutputForward& operator<<(OutputForward& out, T t)
	{
		if (out.outputView != nullptr)
		{
			out.outputView->write(std::to_string(t));
		}
		return out;
	}
	template<> OutputForward& operator<< <const char*>(OutputForward& out, const char* s);
	template<> OutputForward& operator<< <std::string>(OutputForward& out, std::string s);

}