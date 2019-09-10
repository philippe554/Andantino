#include "../include/GLib.h"

namespace GLib
{
	void Writer::init(RT * _rt)
	{
		rt = _rt;
	}
	
	void Writer::print(std::string text, ID2D1SolidColorBrush * color, IDWriteTextFormat * font, D2D1_RECT_F place)
	{
		ID2D1Layer *pLayerText = NULL;
		rt->CreateLayer(NULL, &pLayerText);
		rt->PushLayer(D2D1::LayerParameters(place), pLayerText);

		auto t = std::wstring(text.begin(), text.end());
		rt->DrawText(t.c_str(), text.size(), font, place, color);

		rt->PopLayer();
		pLayerText->Release();
	}

	void Writer::printCharacter(char text, ID2D1SolidColorBrush* color, IDWriteTextFormat* font, D2D1_RECT_F place)
	{
		auto t = std::wstring(1, text);
		rt->DrawText(t.c_str(), 1, font, place, color);
	}

	IDWriteFactory* WriterFactory::writeFactory = nullptr;
	std::map<std::tuple<int, int, std::string>, IDWriteTextFormat*> WriterFactory::fonts;
	std::map<std::tuple<char, IDWriteTextFormat*>, DWRITE_TEXT_METRICS> WriterFactory::charMetrics;

	void WriterFactory::setup()
	{
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(writeFactory), reinterpret_cast<IUnknown * *>(&writeFactory));
	}

	IDWriteTextFormat* WriterFactory::getFont(int size, int weight, std::string font)
	{
		if (!writeFactory)
		{
			throw std::runtime_error("Writer not yet setup.");
		}

		auto tuple = std::make_tuple(size, weight, font);

		if (fonts.count(tuple) == 0)
		{
			IDWriteTextFormat* newFont;
			auto name = std::wstring(font.begin(), font.end());
			writeFactory->CreateTextFormat(name.c_str(), nullptr, DWRITE_FONT_WEIGHT(weight), DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, L"", &newFont);

			newFont->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
			newFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

			fonts[tuple] = newFont;

			GLib::Out << "New font: " << size << ", " << font << "\n";
		}

		return fonts.at(tuple);
	}
	DWRITE_TEXT_METRICS WriterFactory::getMetric(char letter, IDWriteTextFormat* font)
	{
		if (!writeFactory)
		{
			throw std::runtime_error("Writer not yet setup.");
		}

		auto tuple = std::make_tuple(letter, font);

		if (charMetrics.count(tuple) == 0)
		{
			auto t = std::wstring(1, letter);
			IDWriteTextLayout* layout;
			DWRITE_TEXT_METRICS metric;

			writeFactory->CreateTextLayout(t.c_str(), 1, font, 0, 0, &layout);
			layout->GetMetrics(&metric);
			layout->Release();

			charMetrics[tuple] = metric;
		}

		return charMetrics.at(tuple);
	}
}