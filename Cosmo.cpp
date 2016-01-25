#include "Cosmo.h"

#include <stdint.h>
#include <stack>

//----------------------------------------------------------------------------
Cosmo::Cosmo()
{
	lensMode = axisMode = velocityMode = showTrails = false;
	particleCount = samples = 0;
	sphereLensRadius = axisLensRadius = maxDistance = 0.f;
	oscAxis.scale = 1.f;
	oscAxis.show = false;
	maxBrightness = 1.f;
	minBrightness = 0.f;
	brightness = maxBrightness;
}

#define POSVEL_T    float
#define ID_T        int32_t

struct cosmoData {
    POSVEL_T px;         // X position for particles on this processor
    POSVEL_T vx;         // X velocity for particles on this processor
    POSVEL_T py;         // Y position for particles on this processor
    POSVEL_T vy;         // Y velocity for particles on this processor
    POSVEL_T pz;         // Z position for particles on this processor
    POSVEL_T vz;         // Z velocity for particles on this processor  
    POSVEL_T mass;       // Mass for particles on this processor 
    ID_T     tag;
};

//----------------------------------------------------------------------------
bool in_focal_area(glm::vec3 point, float r, glm::vec3 center )
{
    //vec3 center = vec3(0.0f, 0.0f, 0.0f);
    if (r <= 0.0f) r = 0.0001f;
	glm::vec3 d = point - center; //delta
    return ( d.x*d.x + d.y*d.y + d.z*d.z <= r*r );
}

void Cosmo::resample( int n )
{
    if (n > particleCount) {n = particleCount;} 
    vSample.clear();

    // generate set of random numbers
    std::random_device rd;
    std::mt19937 generator( rd() );
    std::uniform_int_distribution<int> distribution( 0, particleCount-1 ) ;

    // copy N random points to buffer
    for (int i = 0; i < n; ++i) {
        vSample.push_back( vParticles.at( distribution(generator)) );
    }

    push();
}

// ------------------------------------------------erw----------------------
void Cosmo::read( std::string fileName )
{
    // read entire binary file   
    // get length of the file:
    std::cout << "Reading file " << fileName << "...";
    streampos size;
    char *buffer;
    std::ifstream file (fileName, ios::in|ios::binary|ios::ate ); //binary
    
    // check the file
    if (!file.is_open()) {
       std::cout << "error: "<< fileName <<" cannot be read" << std::endl;
       return;
    }
    // read file into memory
    size = file.tellg();
    buffer = new char [size];
    file.seekg (0, ios::beg);
    file.read (buffer, size);
    file.close();

    std::cout << " file size: "<< size << "...";

    //------------------------------------------------
    // parse the data into related object vector
	glm::vec3 vmax     = glm::vec3(0.0f, 0.0f, 0.0f);    //max velocity
	glm::vec3 vmin     = glm::vec3(0.0f, 0.0f, 0.0f);    //min velocity
	glm::vec3 max      = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 min      = glm::vec3(0.0f, 0.0f, 0.0f);

    cosmoData *data = (cosmoData*) buffer; 
    
    vParticles.clear();  
    Particle p;
    int n = size/32;
    // 32 bytes for each data
    for (int i = 0; i < n; ++i)
    {     
        p.vel.x = data[i].vx;
        p.vel.y = data[i].vy;
        p.vel.z = data[i].vz;

        p.pos.x = data[i].px;
        p.pos.y = data[i].py;
        p.pos.z = data[i].pz;
        p.id    = data[i].tag;
        p.mass  = data[i].mass;

        // calculate vmin and vmax values
        if(p.vel.x < vmin.x) vmin.x = p.vel.x;
        if(p.vel.y < vmin.y) vmin.y = p.vel.y;
        if(p.vel.z < vmin.z) vmin.z = p.vel.z;

        if(p.vel.x > vmax.x) vmax.x = p.vel.x;
        if(p.vel.y > vmax.y) vmax.y = p.vel.y;
        if(p.vel.z > vmax.z) vmax.z = p.vel.z;

        // calculate min and max xyz values
        if(p.pos.x < min.x) min.x = p.pos.x;
        if(p.pos.y < min.y) min.y = p.pos.y;
        if(p.pos.z < min.z) min.z = p.pos.z;
        
        if(p.pos.x > max.x) max.x = p.pos.x;
        if(p.pos.y > max.y) max.y = p.pos.y;
        if(p.pos.z > max.z) max.z = p.pos.z;

        vParticles.push_back( p );
    }

    // calculate color for each particle
	glm::vec3 c;
    for (int i = 0; i < n; ++i)
    {  
        //vParticles.at(id).col
        // calculate color by using velocity for each particle
        c = glm::normalize( vParticles.at(i).vel );

        c.x = fabs( c.x ) ;
        c.y = fabs( c.y ) ;
        c.z = fabs( c.z ) ;
    
        vParticles.at(i).col = glm::vec4(c, 1.f);
    }
    
    // calculate center coordinate
	glm::vec3 center = glm::vec3( (max.x - min.x) / 2,
                        (max.y - min.y) / 2,
                        (max.z - min.z) / 2);

	float maxDist_sq = 0.f, pDist_sq = 0.f;
	glm::vec3 pos;
    // make centered and find distance of furthest particle from origin
    for (int i = 0; i < n; ++i) {
        vParticles.at(i).pos -= center;
        vParticles.at(i).vel -= center;
		pos = vParticles.at(i).pos;
		pDist_sq = pos.x*pos.x + pos.y*pos.y + pos.z*pos.z;
		if (pDist_sq > maxDist_sq) maxDist_sq = pDist_sq;
    }
	// calc sqrt here instead of for loop to save some CPU cycles
	maxDistance = sqrtf(maxDist_sq);
    
    //free unnecessary space
    file.close();
    delete[] buffer;

	//maxDimension = sqrtf(max.x * max.x + max.y * max.y + max.z * max.z) / 2.f;

    particleCount = vParticles.size();
	std::cout << " particle count: " << particleCount << "...";
    std::cout << " done! " << std::endl;

}

float Cosmo::getMaxDistance(){ return maxDistance; }

void Cosmo::setMovableRotationAxis(glm::vec3 axis)
{
	this->oscAxis.axis = axis;
}

void Cosmo::setMovableRotationAxis(float x, float y, float z)
{
	setMovableRotationAxis(glm::vec3(x, y, z));
}

glm::vec3 Cosmo::getMovableRotationAxis()
{
	return oscAxis.axis;
}

void Cosmo::setMovableRotationCenter(glm::vec3 ctr)
{
	if (centerPoints.size() > 500)
		centerPoints.pop_front();

	centerPoints.push_back(ctr);

	this->oscAxis.center = ctr;
}

void Cosmo::setMovableRotationCenter(float x, float y, float z)
{
	setMovableRotationCenter(glm::vec3(x, y, z));
}

glm::vec3 Cosmo::getMovableRotationCenter()
{
	return oscAxis.center;
}

void Cosmo::setMovableRotationAngle(float angle)
{
	this->oscAxis.angle = angle;
}

void Cosmo::setMovableRotationAxisScale(float scale)
{
	this->oscAxis.scale = scale;
}

void Cosmo::setLensPosition(glm::vec3 pos)
{
	this->lensPos = pos;
}

void Cosmo::setLensPosition(float x, float y, float z)
{
	setLensPosition(glm::vec3(x, y, z));
}

void Cosmo::setLensSize(float radius)
{
	this->sphereLensRadius = radius;
}

void Cosmo::setBrightnessRange(float min, float max)
{
	this->minBrightness = min;
	this->maxBrightness = max;
}

void Cosmo::setAxisLensSize(float radius)
{
	this->axisLensRadius = radius;
}

float Cosmo::getMovableRotationAxisScale()
{
	return oscAxis.scale;
}

void Cosmo::setLensMode(bool yesno)
{
	lensMode = yesno;
}

void Cosmo::setAxisMode(bool yesno)
{
	axisMode = yesno;
}

void Cosmo::setVelocityMode(bool yesno)
{
	velocityMode = yesno;
}

void Cosmo::toggleTrailsMode()
{
	this->showTrails = !this->showTrails;
}

void Cosmo::toggleShowOscillationAxis()
{
	this->oscAxis.show = !this->oscAxis.show;
}

void Cosmo::getMV(double *mv)
{
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(position.x, position.y, position.z);
		glRotatef(rotation, rotationAxis.x, rotationAxis.y, rotationAxis.z);
		glScalef(scale.x, scale.y, scale.z);

		glGetDoublev(GL_MODELVIEW_MATRIX, mv);
	glPopMatrix();
}

void Cosmo::dim()
{
	float t = (float)dimTimer / (float)DIM_FRAMES;

	if (dimTimer >= 0)
	{
		brightness = t;
		--dimTimer;
	}
}

void Cosmo::requestDimming()
{
	dimTimer = DIM_FRAMES;
}

float Cosmo::brightnessRange() { return maxBrightness - minBrightness;  }

//-------------------------------------------------------------------------------
void Cosmo::renderPoints()
{
	glColor4f(1.f, 1.f, 1.f, 0.2f);
	glBindVertexArray(vaoID);
	glDrawArrays(GL_POINTS, 0, numberOfPoints);
	glBindVertexArray(0);
}

float Cosmo::sphereTest(float radius_sq, const glm::vec3 & testPt)
{
	return 0.f;
}

void Cosmo::renderPointsWithinSphere()
{
	std::vector<Particle>::iterator it;

	glBegin(GL_POINTS);
		for (it = vSample.begin(); it != vSample.end(); ++it)
		{
			// check the hitbox first to quick-fail ad save some cycles
			if (it->pos.x <= (lensPos.x + sphereLensRadius) && it->pos.x >= (lensPos.x - sphereLensRadius) &&
				it->pos.y <= (lensPos.y + sphereLensRadius) && it->pos.y >= (lensPos.y - sphereLensRadius) &&
				it->pos.z <= (lensPos.z + sphereLensRadius) && it->pos.z >= (lensPos.z - sphereLensRadius))
			{
				float dist = sqrtf(pow(it->pos.x - lensPos.x, 2) + pow(it->pos.y - lensPos.y, 2) + pow(it->pos.z - lensPos.z, 2));
		
				// if within lens fade range
				if(dist <= sphereLensRadius && dist >= sphereLensRadius)
					glColor4f(1.f, 1.f, 1.f, maxBrightness * (1.f - (dist - sphereLensRadius) / sphereLensRadius));
				else if (dist <= sphereLensRadius) // within lens itself
					glColor4f(1.f, 1.f, 1.f, maxBrightness);
				else // not within range (corners of hitbox that envelopes lens)
					glColor4f(1.f, 1.f, 1.f, minBrightness + ( brightnessRange() * brightness ));
			}
			else
				glColor4f(1.f, 1.f, 1.f, minBrightness + ( brightnessRange() * brightness ));

			glVertex3f(it->pos.x, it->pos.y, it->pos.z);
		}
	glEnd();
}

// this function is adapted from http://www.flipcode.com/archives/Fast_Point-In-Cylinder_Test.shtml by Greg James @ NVIDIA
float Cosmo::cylTest(const glm::vec3 & pt1, const glm::vec3 & pt2, float length_sq, float radius_sq, const glm::vec3 & testPt)
{
	float dx, dy, dz;	    // vector d  from line segment point 1 to point 2
	float pdx, pdy, pdz;    // vector pd from point 1 to test point
	float dot, dsq;

	dx = pt2.x - pt1.x;	    // translate so pt1 is origin.  Make vector from
	dy = pt2.y - pt1.y;	    // pt1 to pt2.  Need for this is easily eliminated
	dz = pt2.z - pt1.z;

	pdx = testPt.x - pt1.x;	// vector from pt1 to test point.
	pdy = testPt.y - pt1.y;
	pdz = testPt.z - pt1.z;

	// Dot the d and pd vectors to see if point lies behind the 
	// cylinder cap at pt1.x, pt1.y, pt1.z
	dot = pdx * dx + pdy * dy + pdz * dz;

	// If dot is less than zero the point is behind the pt1 cap.
	// If greater than the cylinder axis line segment length squared
	// then the point is outside the other end cap at pt2.
	if (dot < 0.f || dot > length_sq)
		return(-1.f);
	else
	{
		// Point lies within the parallel caps, so find
		// distance squared from point to line, using the fact that sin^2 + cos^2 = 1
		// the dot = cos() * |d||pd|, and cross*cross = sin^2 * |d|^2 * |pd|^2
		// Carefull: '*' means mult for scalars and dotproduct for vectors
		// In short, where dist is pt distance to cyl axis: 
		// dist = sin( pd to d ) * |pd|
		// distsq = dsq = (1 - cos^2( pd to d)) * |pd|^2
		// dsq = ( 1 - (pd * d)^2 / (|pd|^2 * |d|^2) ) * |pd|^2
		// dsq = pd * pd - dot * dot / lengthsq
		//  where lengthsq is d*d or |d|^2 that is passed into this function 

		// distance squared to the cylinder axis:
		dsq = (pdx*pdx + pdy*pdy + pdz*pdz) - dot*dot / length_sq;

		if (dsq > radius_sq) 
			return(-1.f);		
		else 
			return(dsq);		// return distance squared to axis		
	}
}

void Cosmo::renderPointsWithinAxisCylinder()
{
	glm::vec3 axisTop = oscAxis.center + oscAxis.axis * oscAxis.scale;
	glm::vec3 axisBottom = oscAxis.center - oscAxis.axis * oscAxis.scale;
	float length_sq = powf((axisTop - axisBottom).x, 2) + powf((axisTop - axisBottom).y, 2) + powf((axisTop - axisBottom).z, 2);
	float radius_sq = axisLensRadius * axisLensRadius;

	std::vector<Particle>::iterator it;
	glBegin(GL_POINTS);
		for (it = vSample.begin(); it != vSample.end(); ++it)
		{
			float dist_sq = cylTest(axisTop, axisBottom, length_sq, radius_sq, it->pos);
			if(dist_sq >= 0.f)
				glColor4f(1.f, 1.f, 1.f, maxBrightness * (1.f - sqrtf(dist_sq)/axisLensRadius));
			else
				glColor4f(1.f, 1.f, 1.f, minBrightness + ( brightnessRange() * brightness ));

			glVertex3f(it->pos.x, it->pos.y, it->pos.z);
		}
	glEnd();
}

void Cosmo::renderStreaksWithin()
{
	std::vector<Particle>::iterator it;

	glBegin(GL_POINTS);
		for (it = vSample.begin(); it != vSample.end(); ++it)
		{
			if (it->pos.x <= (lensPos.x + sphereLensRadius) && it->pos.x >= (lensPos.x - sphereLensRadius) &&
				it->pos.y <= (lensPos.y + sphereLensRadius) && it->pos.y >= (lensPos.y - sphereLensRadius) &&
				it->pos.z <= (lensPos.z + sphereLensRadius) && it->pos.z >= (lensPos.z - sphereLensRadius) &&
				sqrtf(pow(it->pos.x - lensPos.x, 2) + pow(it->pos.y - lensPos.y, 2) + pow(it->pos.z - lensPos.z, 2)) <= sphereLensRadius)
			{
				glm::vec3 end = it->pos + it->vel * 0.0025f;

				glEnd();

				glBegin(GL_LINES);
					glColor4f(it->col.r, it->col.g, it->col.b, 0.0f);
					glVertex3f(it->pos.x, it->pos.y, it->pos.z);

					glColor4f(it->col.r, it->col.g, it->col.b, 1.0f);
					glVertex3f(end.x, end.y, end.z);
				glEnd();

				glBegin(GL_POINTS);
				continue;
			}
			else
				glColor4f(1.f, 1.f, 1.f, minBrightness + ( brightnessRange() * brightness) );

			glVertex3f(it->pos.x, it->pos.y, it->pos.z);
		}
	glEnd();
}

//-------------------------------------------------------------------------------
void Cosmo::renderVelocities()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glm::vec3 v = glm::vec3(0.0f, 0.0f, 0.0f); // 63.71757, 19.2537, 0.0488669 
	glm::vec4 c = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
	glm::vec3 p;
	
	glBegin(GL_LINES);
		for (int i = 0; i < (int)vSample.size() / 2; ++i)
		{
			p = vSample.at(i).pos;
			v = (vSample.at(i).vel) * 0.0035f / scale * vectorScale;
			c = vSample.at(i).col;

			// its done in CPU, implement geometry shader and move to GPU
			glColor4f(c.x, c.y, c.z, 1.0f);
			glVertex3f(p.x, p.y, p.z);
			p -= v;
			glColor4f(c.x, c.y, c.z, 0.0f);
			glVertex3f(p.x, p.y, p.z);
		}
	glEnd();
}
//----------------------------------------------------------------------------

void Cosmo::render()
{
	dim();

	glPushMatrix();
		glTranslatef(position.x, position.y, position.z);
		glRotatef(rotation, rotationAxis.x, rotationAxis.y, rotationAxis.z);
		glScalef(scale.x, scale.y, scale.z);

		// oscillation
		if (glm::length(oscAxis.axis) > 0.001)
		{
			glTranslatef(oscAxis.center.x, oscAxis.center.y, oscAxis.center.z);

			// draw oscillation pole
			if(oscAxis.show)
			{
				glm::vec3 scaledAxis = oscAxis.axis * oscAxis.scale;

				glColor4f(0.75f, 0.f, 0.75f, 1.f);
				glBegin(GL_LINES);
					glVertex3f(scaledAxis.x, scaledAxis.y, scaledAxis.z);
					glVertex3f(oscAxis.axis.x, oscAxis.axis.y, oscAxis.axis.z);
					glVertex3f(-oscAxis.axis.x, -oscAxis.axis.y, -oscAxis.axis.z);
					glVertex3f(-scaledAxis.x, -scaledAxis.y, -scaledAxis.z);
				glEnd();
			}

			glRotatef(oscAxis.angle, oscAxis.axis.x, oscAxis.axis.y, oscAxis.axis.z);
			glTranslatef(-oscAxis.center.x, -oscAxis.center.y, -oscAxis.center.z);
		}

		if(showTrails)
		{ 
			glPointSize(4.f);
			glBegin(GL_POINTS);
				for (int i = 0; i < centerPoints.size(); ++i)
				{
					float t = (float)i / ((float)centerPoints.size() - 1.f);
					glColor4f(1.f, 0.f, 1.f, 1.f * t);
					glVertex3f(centerPoints.at(i).x, centerPoints.at(i).y, centerPoints.at(i).z);
				}
			glEnd();
		}

		glPointSize(2.f);
		glColor4f(1.f, 1.f, 1.f, 0.2f);

		if (lensMode)
			if (velocityMode)
				renderStreaksWithin();
			else
				renderPointsWithinSphere();
		else if (axisMode)
			if (velocityMode)
				renderStreaksWithin();
			else
				renderPointsWithinAxisCylinder();
		else
			if (velocityMode)
				renderVelocities();
			else
				renderPoints();
		//--------------------------------------------------
	glPopMatrix();
}