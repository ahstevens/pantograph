#include "Filament.h"

#include <random>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\type_ptr.hpp>



Filament::Filament()
{
	showPath = showControlPoints = true;
	done = false;

	deltas = 0.1f;

	brightness = 1.f;

	nHighlighted = 0;

	percentTillDone = 99.9f;

	color = glm::vec4(1.f, 0.1f, 0.1f, 1.f);
	highlightColor = glm::vec4(0.75f, 0.75f, 0.f, 1.f);
	primaryTargetColor = glm::vec4(1.f, 0.5f, 0.f, 1.f);
	completeColor = glm::vec4(0.f, 1.f, 0.f, 1.f);

	splinePath.addPoint(glm::vec3(-50.f, 0.f, 0.f), deltas);
	splinePath.addPoint(glm::vec3(-25.f, 25.f, -25.f), deltas);
	splinePath.addPoint(glm::vec3(25.f, -25.f, 25.f), deltas);
	splinePath.addLastPoint(glm::vec3(50.f, 0.f, 0.f));

	while (!splinePath.isEnd())
		path.push_back(splinePath.advanceAlongSpline());

	//generate(500, 5.f);
}


Filament::Filament(float length)
{
	showPath = false;
	showControlPoints = false;
	done = false;

	deltas = 0.1f;

	brightness = 1.f;

	nHighlighted = 0;

	percentTillDone = 95.f;

	color = glm::vec4(1.f, 0.1f, 0.1f, 1.f);
	highlightColor = glm::vec4(0.75f, 0.75f, 0.f, 1.f);
	primaryTargetColor = glm::vec4(1.f, 0.5f, 0.f, 1.f);
	completeColor = glm::vec4(0.f, 1.f, 0.f, 1.f);

	std::random_device rd;
	std::mt19937 generator(rd());
	std::normal_distribution<float> unit_distribution(0.00001f, 1.f);
	std::uniform_real_distribution<float> signed_unit_distribution(-1.f, 1.f);
	std::uniform_real_distribution<float> midCP(0.25f, 0.45f);
	std::uniform_real_distribution<float> circle(0.f, 2.f * (float)M_PI);
	std::uniform_real_distribution<float> splineFactor(2.f, 5.f);

		
	glm::vec3 cp0, cp1, cp2, cp3;
	//cp0.x = unit_distribution(generator);
	//cp0.y = unit_distribution(generator);
	//cp0.z = unit_distribution(generator);
	//cp0 = glm::normalize(cp0) * (length / 4.f); // length/4 is sphere radius

	cp0 = glm::normalize(glm::axis(glm::quat(signed_unit_distribution(generator), signed_unit_distribution(generator), signed_unit_distribution(generator), signed_unit_distribution(generator))));
	cp0 *= length / 2.f;
	cp3 = -cp0;

	glm::vec3 vecAB = cp3 - cp0;

	glm::vec3 unitVecAB = glm::normalize(vecAB);

	glm::vec3 v = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), unitVecAB));

	if(glm::length(v) < 0.00001f)
		v = glm::normalize(glm::cross(glm::vec3(1.f, 0.f, 0.f), unitVecAB));

	glm::vec3 u = glm::normalize(cross(v, unitVecAB));

	float radius = length / splineFactor(generator);

	glm::mat4 coordFrame = glm::mat4(glm::vec4(u, .0f) * radius,
		glm::vec4(v, 0.f) * radius,
		glm::vec4(unitVecAB, 0.f) * radius,
		glm::vec4(cp0, 1.f));
	
	float cp1CirclePt = circle(generator);
	float cp2CirclePt = circle(generator);
	glm::vec3 cp1OffsetRingPt = glm::vec3(cosf(cp1CirclePt), sinf(cp1CirclePt), 0.f);
	glm::vec3 cp2OffsetRingPt = glm::vec3(cosf(cp2CirclePt), sinf(cp2CirclePt), 0.f);

	cp1 = glm::vec3(glm::vec4(coordFrame * glm::vec4(cp1OffsetRingPt, 1.f))) + (midCP(generator)) * vecAB;
	cp2 = glm::vec3(glm::vec4(coordFrame * glm::vec4(cp2OffsetRingPt, 1.f))) + (1.f - midCP(generator)) * vecAB;

	splinePath.addPoint(cp0, deltas);
	splinePath.addPoint(cp1, deltas);
	splinePath.addPoint(cp2, deltas);
	splinePath.addLastPoint(cp3);

	while (!splinePath.isEnd())
		path.push_back(splinePath.advanceAlongSpline());

	generate(10000, 10.f);
	//generate(path.size(), 0.f);
}

Filament::~Filament()
{

}

void Filament::generate(unsigned int nPoints, float spreadFactor)
{
	// hack to just place points along path vertices
	if (spreadFactor < 0.001f)
	{
		Particle tempPart;
		tempPart.col = primaryTargetColor;
		tempPart.highlighted = false;

		for (auto& p : path)
		{
			tempPart.pos = p;
			vSample.push_back(tempPart);
		}

		return;
	}

	float pointSpacing = getLength() / ( nPoints - 1 );
	float leftover, offset, traversedLength;
	leftover = offset = traversedLength = 0.f;

	radius = sqrt(pow(spreadFactor/10.f, 2) * 3);
	
	// generate set of random numbers for displacing points along filament spline
	std::random_device rd;
	std::mt19937 generator(rd());
	//std::uniform_real_distribution <float> distribution(-radius, radius);

	//std::normal_distribution <float> radDist(0.f, radius);
	//std::exponential_distribution <float> radDist(2.f);
	std::uniform_real_distribution <float> radDist(0.f, radius);

	std::uniform_real_distribution <float> quatDist(-1.f, 1.f);	

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

				glm::vec3 randVec = glm::normalize(glm::axis(glm::quat(quatDist(generator), quatDist(generator), quatDist(generator), quatDist(generator))));
				p.pos += randVec * radDist(generator);

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

float Filament::getLength()
{
	len = 0.f;

	for (std::vector<glm::vec3>::iterator it = path.begin(); it != (path.end() - 1); ++it)
		len += glm::length(*(it + 1) - *it);
	
	return len;
}

float Filament::getRadius()
{
	return radius;
}

glm::vec3 Filament::getSplineControlPoint(int i)
{
	return splinePath.getPoint(i);
}

void Filament::setBrightness(float brightness)
{
	this->brightness = brightness;
}


unsigned int Filament::getHighlightedCount()
{
	return nHighlighted;
}

unsigned int Filament::getTargetCount()
{
	return vSample.size();
}

bool Filament::highlight(glm::vec3 lensPos, float radius_sq)
{
	bool firstHighlight = false;

	if (!done)
	{
		float percentHighlighted = ((float)nHighlighted / (float)vSample.size()) * 100.f;

		if ( percentHighlighted >= percentTillDone && !done)
		{
			for (auto& p : vSample)
				p.col = completeColor;

			nHighlighted = vSample.size();
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
					if (nHighlighted++ == 0) firstHighlight = true;
				}
			}
	}

	return firstHighlight;
}

float Filament::getMinDistTo(glm::vec3 *p)
{
	glm::vec3 minVec = path[0] - *p;
	float min = minVec.x * minVec.x + minVec.y * minVec.y + minVec.z * minVec.z;
	unsigned int minIndex = 0;

	for (unsigned int i = 0; i < path.size(); ++i)
	{
		glm::vec3 tempVec = path.at(i) - *p;
		float temp = tempVec.x * tempVec.x + tempVec.y * tempVec.y + tempVec.z * tempVec.z;
		if (temp < min)
		{
			min = temp;
			minVec = tempVec;
			minIndex = i;
		}
	}

	float dist = -1.f;

	if (minIndex > 0)
	{
		glm::vec3 ptToPrev = *p - path.at(minIndex - 1);
		glm::vec3 ptToClosest = *p - path.at(minIndex);
		glm::vec3 lineVec = path.at(minIndex) - path.at(minIndex - 1);

		dist = glm::length(glm::cross(ptToPrev, ptToClosest)) / glm::length(lineVec);
	}
	
	if (minIndex < path.size() - 1)
	{
		glm::vec3 ptToNext = *p - path.at(minIndex + 1);
		glm::vec3 ptToClosest = *p - path.at(minIndex);
		glm::vec3 lineVec = path.at(minIndex) - path.at(minIndex + 1);

		if(dist < 0.f)
			dist = glm::length(glm::cross(ptToClosest, ptToNext)) / glm::length(lineVec);
		else
		{
			float tempDist = glm::length(glm::cross(ptToClosest, ptToNext)) / glm::length(lineVec);
			if (tempDist < dist) dist = tempDist;
		}
	}

	return dist;
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
	glColor4f(0.f, 0.75f, 0.75f, 0.7f);
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
				glm::vec4 col = p.highlighted ? p.col : glm::vec4(glm::vec3(p.col), brightness);
				glColor4f(col.r, col.g, col.b, col.a);
				glVertex3fv(glm::value_ptr(p.pos));
			}
		glEnd();
	//--------------------------------------------------
	glPopMatrix();
}