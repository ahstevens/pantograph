#pragma once
#include "Object.h"
#include "SplinePath.h"

class Filament :
	public Object
{
public:
	Filament();
	Filament(float length);
	~Filament();

	bool highlight(glm::vec3 lensPos, float radius_sq);
	void render();

private:
	void generate(unsigned int nPoints, float spreadFactor);
	void renderPath();

	SplinePath spline;
	std::vector< glm::vec3 > splinePath;
	float len, deltas;

	glm::vec4 color, highlightColor, completeColor;

	bool showPath, done;

	unsigned int nHighlighted;
};

