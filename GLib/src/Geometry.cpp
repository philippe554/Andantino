#include "../include/GLib.h"

namespace GLib
{
	ID2D1Factory* Geometry::Direct2dFactory = nullptr;

	Geometry::Geometry(std::vector<std::pair<float, float>> data)
	{
		if (Direct2dFactory)
		{
			ID2D1GeometrySink* pSink = NULL;
			Direct2dFactory->CreatePathGeometry(&geometry);
			geometry->Open(&pSink);
			
			pSink->BeginFigure(D2D1::Point2F(data.at(0).first, data.at(0).second), D2D1_FIGURE_BEGIN_FILLED);

			for (int i = 0; i < data.size(); i++)
			{
				pSink->AddLine(D2D1::Point2F(data.at(i).first, data.at(i).second));
			}

			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
			pSink->Close();

			pSink->Release();
		}
		else
		{
			throw std::runtime_error("Factory not set, can not make geometry");
		}
	}

	Geometry::~Geometry()
	{
		geometry->Release();
	}
	void Geometry::setFactory(ID2D1Factory* factory)
	{
		Direct2dFactory = factory;
	}
	void Geometry::draw(RT* rt, ID2D1SolidColorBrush* brush, int width, float x, float y)
	{
		D2D1::Matrix3x2F start;
		rt->GetTransform(&start);

		rt->SetTransform(start * D2D1::Matrix3x2F::Translation(x, y));

		rt->DrawGeometry(geometry, brush, width);

		rt->SetTransform(start);
	}
	void Geometry::fill(RT* rt, ID2D1SolidColorBrush* brush, float x, float y)
	{
		D2D1_MATRIX_3X2_F start;
		rt->GetTransform(&start);

		rt->SetTransform(start * D2D1::Matrix3x2F::Translation(x,y));

		rt->FillGeometry(geometry, brush);

		rt->SetTransform(start);
	}
}