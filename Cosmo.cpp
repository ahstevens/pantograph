#include "Cosmo.h"

#include <stdint.h>
#include <stack>

//----------------------------------------------------------------------------
Cosmo::Cosmo()
{
	lensMode = velocityMode = showTrails = false;
	curLens = SPHERE_POINTS;
	particleCount = samples = 0;
	sphereLensRadius = axisLensRadius = maxDistance = 0.f;
	oscAxis.scale = 1.f;
	oscAxis.show = false;
	lensInnerBrightness = 1.f;
	lensOuterBrightness = 0.f;
	normalBrightness = 1.f;
	brightnessRatio = lensInnerBrightness;

	filament = nullptr;
}

Cosmo::~Cosmo()
{
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

void Cosmo::generateFilament()
{
	filament = new Filament(maxDistance / 2.f);
	setLensSize(filament->getRadius() * 2.f);
}

unsigned int Cosmo::getRemainingTargets()
{
	return filament->getTargetCount() - filament->getHighlightedCount();
}

// returns distance between origin and farthest (Eucl. distance) particle
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

float Cosmo::getLensSize()
{
	return this->sphereLensRadius * scale.x;
}

void Cosmo::setBrightness(float brightness)
{
	this->normalBrightness = brightness;
}

void Cosmo::setLensBrightnessRange(float inner, float outer)
{
	this->lensInnerBrightness = inner;
	this->lensOuterBrightness = outer;
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

void Cosmo::setLensType(Cosmo::Lens l)
{
	this->curLens = l;
}

Cosmo::Lens Cosmo::getLensType()
{
	return this->curLens;
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
	if (dimTimer >= 0)
	{
		float t = (float)dimTimer / (float)DIM_FRAMES;
		brightnessRatio = t;
		--dimTimer;
	}
}

void Cosmo::requestDimming()
{
	dimTimer = DIM_FRAMES;
}

float Cosmo::lensBrightnessRange() { return lensInnerBrightness - lensOuterBrightness;  }

//-------------------------------------------------------------------------------



void Cosmo::renderCursorTrails()
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

void Cosmo::renderOscillationAxis()
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

void Cosmo::renderLens()
{
	glm::vec3 axisTop, axisBottom, axis;
	float length_sq, radius;

	// calc parameters to be passed to the appropriate point-in-volume method
	switch (curLens) {
	case SPHERE_POINTS:
	case SPHERE_VELOCITY:
		radius = sphereLensRadius;
		break;
	case CYLINDER_POINTS:
	case CYLINDER_VELOCITY:
		axisTop = oscAxis.center + oscAxis.axis * oscAxis.scale;
		axisBottom = oscAxis.center - oscAxis.axis * oscAxis.scale;
		axis = axisTop - axisBottom;
		length_sq = axis.x * axis.x + axis.y * axis.y + axis.z * axis.z;
		radius = axisLensRadius;
		break;
	}

	float radius_sq = radius * radius;

	filament->highlight(lensPos, radius_sq);

	// render the data
	glPointSize(2.f);
	glBegin(GL_POINTS);
		std::vector<Particle>::iterator it;
		for (it = vSample.begin(); it != vSample.end(); ++it)
		{
			float dist_sq;
			glm::vec3 end;

			// calc a couple more parameters to be passed to the appropriate point-in-volume method
			switch (curLens) {
			case SPHERE_VELOCITY:
				end = it->pos + it->vel * 0.0025f;
			case SPHERE_POINTS:
				dist_sq = sphereTest(lensPos, radius_sq, it->pos);
				break;
			case CYLINDER_VELOCITY:
				end = it->pos + it->vel * 0.0025f;
			case CYLINDER_POINTS:
				dist_sq = cylTest(axisTop, axisBottom, length_sq, radius_sq, it->pos);
				break;
			}

			// if the point is within the sphere
			if(dist_sq >= 0.f)
				// if the current lens renders particle velocities
				if (curLens == SPHERE_VELOCITY || curLens == CYLINDER_VELOCITY)
				{
					glEnd(); // end GL_POINTS mode

					// draw particle velocity streak
					glBegin(GL_LINES);
						glColor4f(it->col.r, it->col.g, it->col.b, 0.0f);
						glVertex3f(it->pos.x, it->pos.y, it->pos.z);

						glColor4f(it->col.r, it->col.g, it->col.b, 1.0f * (1.f - sqrtf(dist_sq) / radius));
						glVertex3f(end.x, end.y, end.z);
					glEnd();

					glBegin(GL_POINTS); // resume GL_POINTS mode
					continue; // nothing else to do, so short-circuit to next for loop iteration
				}
				else // current lens renders particle points
					glColor4f(1.f, 1.f, 1.f, lensInnerBrightness * (1.f - sqrtf(dist_sq) / radius));
			else // point is not in sphere
				glColor4f(1.f, 1.f, 1.f, lensOuterBrightness + ( lensBrightnessRange() * brightnessRatio ));

			// draw particle
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

void Cosmo::renderPoints()
{
	glPointSize(2.f);
	glBindVertexArray(vaoID);
	glColor4f(1.f, 1.f, 1.f, normalBrightness);
	glDrawArrays(GL_POINTS, 0, numberOfPoints);
	glBindVertexArray(0);
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
		if (glm::length(oscAxis.axis) > 0.001f)
		{
			glTranslatef(oscAxis.center.x, oscAxis.center.y, oscAxis.center.z);

			// draw oscillation pole
			if (oscAxis.show)
				renderOscillationAxis();

			glRotatef(oscAxis.angle, oscAxis.axis.x, oscAxis.axis.y, oscAxis.axis.z);
			glTranslatef(-oscAxis.center.x, -oscAxis.center.y, -oscAxis.center.z);
		}

		// cursor trails
		if (showTrails)
			renderCursorTrails();
				
		// cosmos
		if (lensMode)
		{
			renderLens();
			filament->setBrightness((lensOuterBrightness + (lensBrightnessRange() * brightnessRatio))*2.f);
		}
		else
		{
			if (velocityMode)
				renderVelocities();
			else
				renderPoints();

			if (filament->getHighlightedCount() == filament->getTargetCount()) generateFilament();
			filament->setBrightness(normalBrightness*2.f);
		}
		
			filament->render();
		//--------------------------------------------------
	glPopMatrix();
}