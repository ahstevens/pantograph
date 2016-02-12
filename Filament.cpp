#include "Filament.h"



Filament::Filament()
{
	showPath = showControlPoints = true;
	done = false;

	deltas = 0.1f;

	nHighlighted = 0;

	color = glm::vec4(0.8f, 0.1f, 0.1f, 1.f);
	highlightColor = glm::vec4(0.75f, 0.75f, 0.f, 1.f);
	completeColor = glm::vec4(0.f, 1.f, 0.f, 1.f);

	splinePath.addPoint(glm::vec3(-50.f, 0.f, 0.f), deltas);
	splinePath.addPoint(glm::vec3(-25.f, 25.f, -25.f), deltas);
	splinePath.addPoint(glm::vec3(25.f, -25.f, 25.f), deltas);
	splinePath.addLastPoint(glm::vec3(50.f, 0.f, 0.f));

	while (!splinePath.isEnd())
		path.push_back(splinePath.advanceAlongSpline());

	generate(500, 10.f);
}


Filament::Filament(float length)
{
	showPath = showControlPoints = true;
	done = false;

	deltas = 0.1f;

	nHighlighted = 0;

	color = glm::vec4(0.8f, 0.1f, 0.1f, 1.f);
	highlightColor = glm::vec4(0.75f, 0.75f, 0.f, 1.f);
	completeColor = glm::vec4(0.f, 1.f, 0.f, 1.f);

	std::random_device rd;
	std::mt19937 generator(rd());
	std::normal_distribution<float> unit_distribution(0.00001f, 1.f);
	std::uniform_real_distribution<float> signed_unit_distribution(-1.f, 1.f);
	std::uniform_real_distribution<float> midCP(0.2f, 0.4f);

	glm::vec3 cp0, cp1, cp2, cp3;
	cp0.x = unit_distribution(generator);
	cp0.y = unit_distribution(generator);
	cp0.z = unit_distribution(generator);
	cp0 = glm::normalize(cp0) * (length / 4.f); // length/4 is sphere radius
	cp3 = -cp0;

	glm::vec3 vecAB = cp3 - cp0;

	//glm::vec3 cp1LinePos = cp0 + midCP( generator ) * vecAB;
	//glm::vec3 cp2LinePos = cp0 + ( midCP( generator ) + 0.5f ) * vecAB;

	glm::vec3 cp1LinePos = cp0 + 0.3333333333f * vecAB;
	glm::vec3 cp2LinePos = cp0 + 0.6666666666f * vecAB;

	glm::vec3 randVec1;
	randVec1.x = signed_unit_distribution(generator);
	randVec1.y = signed_unit_distribution(generator);
	randVec1.z = signed_unit_distribution(generator);
	randVec1 = glm::normalize(randVec1);

	glm::vec3 randVec2;
	randVec2.x = signed_unit_distribution(generator);
	randVec2.y = signed_unit_distribution(generator);
	randVec2.z = signed_unit_distribution(generator);
	randVec2 = glm::normalize(randVec2);

	cp1 = cp1LinePos + (length / 6.f) * randVec1;
	cp2 = cp2LinePos + (length / 6.f) * randVec2;

	splinePath.addPoint(cp0, deltas);
	splinePath.addPoint(cp1, deltas);
	splinePath.addPoint(cp2, deltas);
	splinePath.addLastPoint(cp3);

	while (!splinePath.isEnd())
		path.push_back(splinePath.advanceAlongSpline());

	generate(500, 10.f);
}

Filament::~Filament()
{

}

bool Filament::highlight(glm::vec3 lensPos, float radius_sq)
{
	if (!done)
	{
		if (nHighlighted == vSample.size() && !done)
		{
			for (auto& p : vSample)
				p.col = completeColor;

			done = true;
		}
		else
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
					std::cout << "Highlighted " << nHighlighted << " of " << vSample.size() << " target particles (" << ((float)nHighlighted / (float)vSample.size()) * 100.f << "%)" << std::endl;
				}
			}
	}

	return done;
}

void Filament::generate(unsigned int nPoints, float spreadFactor)
{
	float pointSpacing = splinePath.length() / ( nPoints - 1 );
	float leftover, offset, traversedLength;
	leftover = offset = traversedLength = 0.f;

	// generate set of random numbers for displacing points along filament spline
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_real_distribution <float> distribution(-pointSpacing * spreadFactor, pointSpacing * spreadFactor);

	for (std::vector<glm::vec3>::iterator it = path.begin(); it != (path.end() - 1); ++it)
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

void Filament::renderControlPoints()
{
	glPointSize(6.f);
	glColor4f(0.4f, 0.2f, 1.f, 1.f);

	glBegin(GL_POINTS);
		for (int i = 0; i < 4; ++i)
			glVertex3fv(glm::value_ptr(splinePath.getPoint(i)));
	glEnd();
}

void Filament::renderPath()
{
	std::vector< glm::vec3 >::iterator it;
	glColor4f(0.f, 0.75f, 0.75f, 1.f);
	glBegin(GL_LINE_STRIP);
		for (it = path.begin(); it != path.end(); ++it)
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

		if (showControlPoints) renderControlPoints();

		glPointSize(done?3.f:2.f);
		glBegin(GL_POINTS);
			for (auto& p : vSample)
			{
				glColor4fv(glm::value_ptr(p.col));
				glVertex3fv(glm::value_ptr(p.pos));
			}
		glEnd();
	//--------------------------------------------------
	glPopMatrix();
}