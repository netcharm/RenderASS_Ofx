#pragma once

#include "stdafx.h"
#include "Grid.h"

using namespace OpenFX;

EXPORT OfxPlugin * OfxGetPlugin(int nth)
{
	if (nth == 0) {
		Grid* grid = new Grid();
		return grid->GetPlugin();
	}
	return 0;
}

EXPORT int OfxGetNumberOfPlugins(void)
{
	return 1;
}


