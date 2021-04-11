#pragma once

class GalaxyApplication;

namespace UI
{

class GalaxyUI
{
public:
	GalaxyUI(GalaxyApplication& app)
		: app_(app)
	{
	}

	void Build();

private:
	GalaxyApplication& app_;
};

}
