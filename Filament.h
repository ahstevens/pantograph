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
	float getLength();
	float getRadius();
	void setBrightness(float brightness);
	unsigned int getHighlightedCount();
	unsigned int getTargetCount();
	void render();

private:
	void generate(unsigned int nPoints, float spreadFactor);
	void renderPath();
	void renderControlPoints();

	SplinePath splinePath;
	std::vector< glm::vec3 > path;
	float len, radius, deltas, brightness;

	glm::vec4 color, highlightColor, completeColor, primaryTargetColor;

	bool showPath, showControlPoints, done;

	unsigned int nHighlighted;
};

