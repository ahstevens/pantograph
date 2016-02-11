#include "Filament.h"



Filament::Filament()
{
	showPath = true;
	done = false;

	nHighlighted = 0;

	color = glm::vec4(0.8f, 0.1f, 0.1f, 1.f);
	highlightColor = glm::vec4(0.75f, 0.75f, 0.f, 1.f);
	completeColor = glm::vec4(0.f, 1.f, 0.f, 1.f);

	spline.addPoint(glm::vec3(-50.f, 0.f, 0.f), 0.1f);
	spline.addPoint(glm::vec3(-25.f, 25.f, -25.f), 0.1f);
	spline.addPoint(glm::vec3(25.f, -25.f, 25.f), 0.1f);
	spline.addLastPoint(glm::vec3(50.f, 0.f, 0.f));

	while (!spline.isEnd())
		splinePath.push_back(spline.advanceAlongSpline());

	generate(1000, 10.f);
}


Filament::~Filament()
{

}

void Filament::highlight(glm::vec3 lensPos, float radius_sq)
{
	if (done) return;
	
	if (nHighlighted == vSample.size() && !done)
	{
		for (auto& p : vSample)
			p.col = completeColor;

		done = true;
		return;
	}

	for (auto& p : vSample)
	{
		float dist_sq;
		glm::vec3 end;

		dist_sq = sphereTest(lensPos, radius_sq, p.pos);

		// if the point is within the sphere
		if (dist_sq >= 0.f && !p.highlighted)
		{
			p.col = highlightColor;
			p.highlighted = true;
			nHighlighted++;
			std::cout << "Highlighted " << nHighlighted << " of " << vSample.size() << " target particles (" << ((float)nHighlighted/(float)vSample.size()) * 100.f << "%)" << std::endl;
		}
	}
}

void Filament::generate(unsigned int nPoints, float spreadFactor)
{
	float pointSpacing = spline.length() / ( nPoints - 1 );
	float leftover, offset, traversedLength;
	leftover = offset = traversedLength = 0.f;

	// generate set of random numbers for displacing points along filament spline
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_real_distribution <float> distribution(-pointSpacing * spreadFactor, pointSpacing * spreadFactor);

	for (std::vector<glm::vec3>::iterator it = splinePath.begin(); it != (splinePath.end() - 1); ++it)
	{
		glm::vec3 curVec = *(it + 1) - *it;
		float curVecLen = glm::length(curVec);
		leftover = fmod(curVecLen - offset, pointSpacing);
		int nSteps = (curVecLen - offset) / pointSpacing + 1;
		int i;

		if (nSteps > 0)
		{
			for (i = 0; i < nSteps; ++i)
			{
				float progress = (offset + (float)i * pointSpacing) / curVecLen;
				Particle p;
				p.pos = *it + progress * curVec;
				p.pos.x += distribution( generator );
				p.pos.y += distribution( generator );
				p.pos.z += distribution( generator );
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

		glPointSize(2.f);
		glBegin(GL_POINTS);
			for (auto p : vSample)
			{
				glColor4fv(glm::value_ptr(p.col));
				glVertex3fv(glm::value_ptr(p.pos));
			}
		glEnd();
	//--------------------------------------------------
	glPopMatrix();
}