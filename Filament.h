#pragma once
#include "Object.h"
#include "SplinePath.h"
#include "DrawFunctions.h"

class Filament :
	public Object
{
public:
	Filament();
	~Filament();

	void highlight(glm::vec3 lensPos, float radius_sq);
	void render();

private:
	void generate(unsigned int nPoints, float spreadFactor);
	void renderPath();

	SplinePath spline;
	std::vector< glm::vec3 > splinePath;
	float len;

	glm::vec4 color, highlightColor;

	bool showPath;
};

