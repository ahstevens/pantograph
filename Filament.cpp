#include "Filament.h"



Filament::Filament()
{
	showPath = true;

	color = glm::vec4(0.8f, 0.1f, 0.1f, 1.f);
	highlightColor = glm::vec4(0.75f, 0.75f, 0.f, 1.f);

	spline.addPoint(glm::vec3(-50.f, 0.f, 0.f), 1.f);
	spline.addPoint(glm::vec3(-25.f, 25.f, -25.f), 1.f);
	spline.addPoint(glm::vec3(25.f, -25.f, 25.f), 1.f);
	spline.addLastPoint(glm::vec3(50.f, 0.f, 0.f));

	while (!spline.isEnd())
		splinePath.push_back(spline.advanceAlongSpline());

	generate(10);
}


Filament::~Filament()
{

}

void Filament::generate(unsigned int nPoints)
{
	float pointSpacing = spline.length() / nPoints;
	float leftover, offset, traversedLength;
	leftover = offset = traversedLength = 0.f;	

	for (std::vector<glm::vec3>::iterator it = splinePath.begin(); it != (splinePath.end() - 1); ++it)
	{
		glm::vec3 curVec = *(it + 1) - *it;
		float curVecLen = glm::length(curVec);
		leftover = fmodf(curVecLen, pointSpacing);
		int i;

		if (((it==splinePath.begin()?0:leftover) + curVecLen) >= pointSpacing)
		{
			int nSteps = int(((it == splinePath.begin() ? 0 : leftover) + curVecLen) / pointSpacing);
			for (i = 0; i <= nSteps; ++i)
			{
				float progress = (float)i / (float)nSteps;
				Particle p;
				p.pos = *it + progress * (curVec - offset);
				p.col = color;
				p.highlighted = false;
				vSample.push_back(p);

				traversedLength += pointSpacing;
			}
			offset = pointSpacing - leftover;
		}
		else
			leftover += curVecLen;		
	}
}

void Filament::renderPath()
{
	std::vector< glm::vec3 >::iterator it;
	glColor4f(0.f, 0.75f, 0.75f, 1.f);
	glBegin(GL_LINE_STRIP);
		for (it = splinePath.begin(); it != splinePath.end(); ++it)
			glVertex3f(it->x, it->y, it->z);
	glEnd();
}

void Filament::render()
{
	glPushMatrix();
		glTranslatef(position.x, position.y, position.z);
		glRotatef(rotation, rotationAxis.x, rotationAxis.y, rotationAxis.z);
		glScalef(scale.x, scale.y, scale.z);

		if(showPath) renderPath();

		glPointSize(6.f);
		glBegin(GL_POINTS);
			for (std::vector<Particle>::iterator it = vSample.begin(); it != vSample.end(); ++it)
			{
				glColor4fv(glm::value_ptr(it->col));
				glVertex3fv(glm::value_ptr(it->pos));
			}
		glEnd();
	//--------------------------------------------------
	glPopMatrix();
}