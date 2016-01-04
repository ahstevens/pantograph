#include "Cosmo.h"

#include <stdint.h>
#include <stack>

//----------------------------------------------------------------------------
Cosmo::Cosmo()
{
}

#define POSVEL_T    float
#define ID_T        int32_t

struct cosmoData {
    POSVEL_T xx;         // X location for particles on this processor
    POSVEL_T vx;         // X velocity for particles on this processor
    POSVEL_T yy;         // Y location for particles on this processor
    POSVEL_T vy;         // Y velocity for particles on this processor
    POSVEL_T zz;         // Z location for particles on this processor
    POSVEL_T vz;         // Z velocity for particles on this processor  
    POSVEL_T mass;       // Mass for particles on this processor 
    ID_T     tag;
};

//----------------------------------------------------------------------------
bool in_focal_area(vec3 point, float r, vec3 center )
{
    //vec3 center = vec3(0.0f, 0.0f, 0.0f);
    if (r <= 0.0f) r = 0.0001f;
    vec3 d = point - center; //delta
    return ( d.x*d.x + d.y*d.y + d.z*d.z <= r*r );
}

bool compare_vec3(vec3 v1, vec3 v2)
{
    return (v1 == v2);
}
void Cosmo::resample( int n )
{
    if (n > particleCount) {n = particleCount;} 
    vSample.clear();
    vFocal.clear();

    // generate set of random numbers
    std::random_device rd;
    std::mt19937 generator( rd() );
    std::uniform_int_distribution<int> distribution( 0, particleCount-1 ) ;

    // copy N random points to buffer
    for (int i = 0; i < n; ++i) {
        vSample.push_back( vParticles.at( distribution(generator)) );
    }

    // generate N2 random points within focal area
    // get random particles within limit
    float radius = 50.0f / pow(0.95, 10) / scale;// / std::sqrt(scale); //std::sqrt(
    Particle p;
    int count = 0;
    while(count < 200000 )  
    {
        // get a sample particle
        p = vParticles.at( distribution(generator) );
        // check if it is within the desired boundry
        if (!in_focal_area(p.pos, radius, focalCenter )) continue;   // check if in focal area
        //if (!compare_vec3(p, vParticles.at(id).tag)) continue; // check for dublicates
        vFocal.push_back(p);
        count++;
    }

    push();
}

// ------------------------------------------------erw----------------------
void Cosmo::read( std::string fileName )
{
    // read entire binary file   
    // get length of the file:
    std::cout << "Reading... ";
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

    std::cout << "size: "<< size;

    //------------------------------------------------
    // parse the data into related object vector
    vec3 vmax     = vec3(0.0f, 0.0f, 0.0f);    //max velocity
    vec3 vmin     = vec3(0.0f, 0.0f, 0.0f);    //min velocity
    vec3 max      = vec3(0.0f, 0.0f, 0.0f);
    vec3 min      = vec3(0.0f, 0.0f, 0.0f);

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

        p.pos.x = data[i].xx;
        p.pos.y = data[i].yy;
        p.pos.z = data[i].zz;
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
    vec3 v = vec3(0.0f, 0.0f, 0.0f);;
    vec4 c = vec4(0.0f, 0.0f, 0.0f, 0.85f);
    for (int i = 0; i < n; ++i)
    {  
        //vParticles.at(id).col
        // calculate color buy using velocity for each particle
        v = vParticles.at(i).vel;
        float l = sqrt( v.x * v.x +
                        v.y * v.y +
                        v.z * v.z );
        
        v.x /= l; v.y /= l; v.z /= l;

        c.x = fabs( v.x ) ;
        c.y = fabs( v.y ) ;
        c.z = fabs( v.z ) ;
    
        //vColor.push_back( color );
        vParticles.at(i).col = c;
    }
    

    // calculate center coordinate
    vec3 center = vec3( (max.x - min.x) / 2, 
                        (max.y - min.y) / 2,
                        (max.z - min.z) / 2);


    // make centered
    for (int i = 0; i < n; ++i) {
        vParticles.at(i).pos -= center;
        vParticles.at(i).vel -= center;
    }
    
    //free unnecessary space
    file.close();
    delete[] buffer;

	maxDepth = max.z;
	minDepth = min.z;

    particleCount = vParticles.size();
    std::cout << "done! " << std::endl;
    std::cout << particleCount << " particles..." << std::endl;
}

float Cosmo::getMaxDepth(){ return maxDepth; }

float Cosmo::getMinDepth() { return minDepth; }


//----------------------------------------------------------------------------

