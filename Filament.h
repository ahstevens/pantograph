#pragma once
#include "Object.h"
#include "SplinePath.h"

class Filament :
	public Object
{
public:
	Filament();
	~Filament();

	void render();

private:
	void generate(unsigned int nPoints);
	void renderPath();

	SplinePath spline;
	std::vector< glm::vec3 > splinePath;
	float len;

	glm::vec4 color, highlightColor;

	bool showPath;
};

