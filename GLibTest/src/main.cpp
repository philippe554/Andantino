#include "GLibMain.h"

void GLibMain(GLib::Frame* frame)
{
	frame->init("Test", 1000, 800);

	frame->addView<GLib::MainBar>(0, 0, -1, 50, "Test Page");

	auto mainPage = frame->addView<GLib::MovingView>(0, 50, false, true)->getMovingView();

	mainPage->addView<GLib::OutputView>()->setDefault();

	std::string idk = " IDK works";

	GLib::Out << "test" << idk << " " << 9 << " " << "test\n";
}

