#pragma once

enum Axiss
{
	E_None,
	E_X,
	E_Y,
	E_Z,
	E_XY = E_X|E_Y,
	E_XZ = E_X|E_Z,
	E_YZ = E_Y|E_Z,
	E_XYZ = E_X|E_Y|E_Z,
	E_ALL = E_XYZ,

	E_ZROTATION = E_YZ,
};