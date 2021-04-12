#pragma once

class GalaxyEngine;

namespace UI
{

class GalaxyUI
{
public:
	GalaxyUI(GalaxyEngine& engine)
		: engine_(engine)
	{
	}

	void Build();

private:
	GalaxyEngine& engine_;
};

}
