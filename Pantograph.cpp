#include "Pantograph.h"

Pantograph::Pantograph()
{
	xMin = -1;
	xMax = -1;
	yMin = -1;
	yMax = -1;
	zMin = -1;
	zMax = -1;
	xRange = xMax - xMin;
	yRange = yMax - yMin;
	zRange = zMax - zMin;
	
	screenWidth = -1;
	screenHeight = -1;

	finger1X = -1;
	finger1Y = -1;

	finger2X = -1;
	finger2Y = -1;

	lastDistanceBetween = -1;

	//look and feel variables:
	distanceOutwards = 75;
	minDistanceBetween  = 100;
	maxDistanceBetween  = 400;

	swapChecked = false;
	drawReticle = false;
	leftHanded = false;
}

Pantograph::~Pantograph()
{

}

void Pantograph::setBounds( float xmin, float xmax, float ymin, float ymax, float zmin, float zmax )
{
	xMin = xmin;
	xMax = xmax;
	yMin = ymin;
	yMax = ymax;
	zMin = zmin;
	zMax = zmax;
	xRange = xMax - xMin;
	yRange = yMax - yMin;
	zRange = zMax - zMin;
}

void Pantograph::setScreenSize( float width, float height )
{
	screenWidth = width;
	screenHeight = height;
}


void Pantograph::setLeftHanded( bool useLeftHand )
{
	leftHanded = useLeftHand;
}

void Pantograph::setFinger1( float x, float y )
{
	if (x != finger1X || y != finger1Y) //if changed, store and recalc
	{
		finger1X = x;
		finger1Y = y;
		recalcSelection();
	}
}

void Pantograph::setFinger2( float x, float y )
{
	if ( x != finger2X || y != finger2Y ) //if changed, store and recalc
	{
		finger2X = x;
		finger2Y = y;
		recalcSelection();
	}
}

void Pantograph::recalcSelection()
{
	//find selection point in screen coords
	float midpointX = ( finger1X + finger2X ) / 2;
	float midpointY = ( finger1Y + finger2Y ) / 2;
	float vectorX = finger2X - finger1X;
	float vectorY = finger2Y - finger1Y;
	float distanceBetween = sqrt( vectorX * vectorX + vectorY * vectorY );
	//normalize vector
	vectorX /= distanceBetween;
	vectorY /= distanceBetween;
	lastVectorX = vectorX;
	lastVectorY = vectorY;

	glm::vec3 between;
	between.x = vectorX;
	between.y = vectorY;
	between.z = 0;
	glm::normalize( between );
	
	glm::vec3 intoScreen;
	intoScreen.x = 0;
	intoScreen.y = 0;
	intoScreen.z = -1; //coordinate handedness switch

	glm::vec3 toRight = glm::cross( between, intoScreen );
	glm::normalize( toRight );

	intoScreen.z = 1;//coordinate handedness switch
	glm::vec3 toLeft = glm::cross( between, intoScreen );
	glm::normalize( toLeft );

	//selection coordinate is perpendicular out a fixed distance from midpoint
	float selectionScreenX, selectionScreenY;
	if ( leftHanded )
	{
		selectionScreenX = midpointX + ( toLeft.x * distanceOutwards );
		selectionScreenY = midpointY + ( toLeft.y * distanceOutwards );
	}
	else
	{
		selectionScreenX = midpointX + ( toRight.x * distanceOutwards );
		selectionScreenY = midpointY + ( toRight.y * distanceOutwards );
	}
	//figure out where to put the accept button
	glm::vec3 acceptVector = toLeft + between;
	glm::normalize( acceptVector );
	acceptBubbleX = finger2X + ( ( DRAG_BUBBLE_RADIUS + BUBBLE_PADDING + BUTTON_RADIUS ) * acceptVector.x );
	acceptBubbleY = finger2Y + ( ( DRAG_BUBBLE_RADIUS + BUBBLE_PADDING + BUTTON_RADIUS ) * acceptVector.y );


	//find depth based on distanceBetween
	lastDistanceBetween = distanceBetween;
	if ( distanceBetween < minDistanceBetween )
		distanceBetween = minDistanceBetween;
	else if ( distanceBetween > maxDistanceBetween )
		distanceBetween = maxDistanceBetween;
	
	float distanceBetweenFactor = ( distanceBetween - minDistanceBetween ) / ( maxDistanceBetween - minDistanceBetween );
	float zCoord = zMin + ( zRange * distanceBetweenFactor );

	//convert screen coords to bounds coords
	float xFactor = selectionScreenX / screenWidth;
	float yFactor = selectionScreenY / screenHeight;
	float xCoord = xMin + ( xRange * xFactor );
	float yCoord = yMin + ( yRange * yFactor );
	
	selection[ 0 ] = xCoord;
	selection[ 1 ] = yCoord;
	selection[ 2 ] = zCoord;

	lastInterfingerAngle = atan2( vectorY, vectorX );
}

void Pantograph::getSelection( float* x, float* y, float* z )
{
	*x = selection[ 0 ];
	*y = selection[ 1 ];
	*z = selection[ 2 ];
}

void Pantograph::getFinger1( float* x, float* y, float* z )
{
	//convert screen coords to bounds coords
	float xFactor = finger1X / screenWidth;
	float yFactor = finger1Y / screenHeight;
	float xCoord = xMin + ( xRange * xFactor );
	float yCoord = yMin + ( yRange * yFactor );
	float zCoord = zMin;	

	*x = xCoord;
	*y = yCoord;
	*z = zCoord;
}

void Pantograph::getFinger2( float* x, float* y, float* z )
{
	//convert screen coords to bounds coords
	float xFactor = finger2X / screenWidth;
	float yFactor = finger2Y / screenHeight;
	float xCoord = xMin + ( xRange * xFactor );
	float yCoord = yMin + ( yRange * yFactor );
	float zCoord = zMin;	

	*x = xCoord;
	*y = yCoord;
	*z = zCoord;
}

bool Pantograph::getSelectPoint( float *x, float *y )
{
	if ( firstFingerSet() && secondFingerSet() )
	{
		//find selection point in screen coords
		float midpointX = ( finger1X + finger2X ) / 2;
		float midpointY = ( finger1Y + finger2Y ) / 2;
		float vectorX = finger2X - finger1X;
		float vectorY = finger2Y - finger1Y;
		float distanceBetween = sqrt( vectorX * vectorX + vectorY * vectorY );
		//normalize vector
		vectorX /= distanceBetween;
		vectorY /= distanceBetween;
		lastVectorX = vectorX;
		lastVectorY = vectorY;

		glm::vec3 between;
		between.x = vectorX;
		between.y = vectorY;
		between.z = 0;
		glm::normalize(between);
		
		glm::vec3 intoScreen;
		intoScreen.x = 0;
		intoScreen.y = 0;
		intoScreen.z = -1;//coordinate handedness switch

		glm::vec3 toRight = glm::cross( between, intoScreen );
		glm::normalize( toRight );

		intoScreen.z = 1;//coordinate handedness switch
		glm::vec3 toLeft = glm::cross( between, intoScreen );
		glm::normalize( toLeft );

		//selection coordinate is perpendicular out a fixed distance from midpoint
		float selectionScreenX, selectionScreenY;
		if ( leftHanded )
		{
			selectionScreenX = midpointX + ( toLeft.x * distanceOutwards );
			selectionScreenY = midpointY + ( toLeft.y * distanceOutwards );
		}
		else
		{
			selectionScreenX = midpointX + ( toRight.x * distanceOutwards );
			selectionScreenY = midpointY + ( toRight.y * distanceOutwards );
		}
		
		*x = selectionScreenX;
		*y = selectionScreenY;

		return true;
	}
	else
		return false;
}

bool Pantograph::getDepthFactor( float *depthFactor )
{
	if ( firstFingerSet() && secondFingerSet() )
	{
		float vectorX = finger2X - finger1X;
		float vectorY = finger2Y - finger1Y;
		float distanceBetween = sqrt( vectorX * vectorX + vectorY * vectorY );

		//find depth based on distanceBetween
		lastDistanceBetween = distanceBetween;
		if ( distanceBetween < minDistanceBetween )
			distanceBetween = minDistanceBetween;
		else if ( distanceBetween > maxDistanceBetween )
			distanceBetween = maxDistanceBetween;
		
		float distanceBetweenFactor = ( distanceBetween - minDistanceBetween ) / ( maxDistanceBetween - minDistanceBetween );
		//float zCoord = zMin + (zRange * distanceBetweenFactor);
		*depthFactor = distanceBetweenFactor;
		
		return true;
	}
	else
		return false;
}

void Pantograph::setDrawReticle( bool reticleDrawn )
{
	drawReticle = reticleDrawn;
}

void Pantograph::draw3D()
{
	//if both fingers valid, draw pantograph
	if ( finger1X != -1 && finger1Y != -1 && finger2X != -1 && finger2Y != -1 )
	{
		float f1X, f1Y, f1Z;
		getFinger1( &f1X, &f1Y, &f1Z );
		float f2X, f2Y, f2Z;
		getFinger2( &f2X, &f2Y, &f2Z );
		
		glColor3f( 0.5, 0.5, 0.5 );
		glLineWidth( 1.5 );
		glBegin( GL_LINES );
		
			glVertex3f( settings->finger1modelCoords[ 0 ],
				settings->finger1modelCoords[ 1 ],
				settings->finger1modelCoords[ 2 ] );
			glVertex3f( settings->currentlySelectedPoint[ 0 ],
				settings->currentlySelectedPoint[ 1 ],
				settings->currentlySelectedPoint[ 2 ] );		
			
			glVertex3f( settings->finger2modelCoords[ 0 ],
				settings->finger2modelCoords[ 1 ],
				settings->finger2modelCoords[ 2 ] );
			glVertex3f( settings->currentlySelectedPoint[ 0 ],
				settings->currentlySelectedPoint[ 1 ],
				settings->currentlySelectedPoint[ 2 ] );			

		glEnd();

	}//end if both fingers valid draw pantograph
}

void Pantograph::draw2D()
{
	//draw fingers
	if ( finger1X != -1 && finger1Y != -1 )
	{
		if ( finger2X != -1 && finger2Y != -1 )
			drawFingerIndicator( finger1X, finger1Y, DRAG_BUBBLE_RADIUS, 1 );
		else
			drawFingerIndicator( finger1X, finger1Y, DRAG_BUBBLE_RADIUS, 0 );
	}
	if ( finger2X != -1 && finger2Y != -1 )
	{
		if ( finger1X != -1 && finger1Y != -1 )
			drawFingerIndicator( finger2X, finger2Y, DRAG_BUBBLE_RADIUS, 1 );
		else
			drawFingerIndicator( finger2X, finger2Y, DRAG_BUBBLE_RADIUS, 0 );
	}


	//if both fingers valid
	if ( finger1X != -1 && finger1Y != -1 && finger2X != -1 && finger2Y != -1 )
	{
		//draw accept button
		//drawFingerIndicator(acceptBubbleX, acceptBubbleY, BUTTON_RADIUS, 2);
		//////////////NEW ACCEPT BUTTON/////////////////
		glColor3f(1.0,0.85,0.0);
		drawArcButton( finger2X, finger2Y, DRAG_BUBBLE_RADIUS + SELECT_INNER_RADIUS, DRAG_BUBBLE_RADIUS + SELECT_OUTER_RADIUS,
					  SELECT_ANGLE_START + lastInterfingerAngle - 1.570795, SELECT_ANGLE_END + lastInterfingerAngle - 1.570795, "HIGHLIGHT", false );

		//draw distance indicator 
		if ( lastDistanceBetween < minDistanceBetween )
		{
			glColor3f( 0.65, 0.25, 0.25 );
			glLineWidth( 1.5 );
			glBegin( GL_LINES );
				if ( lastDistanceBetween < minDistanceBetween )
					glVertex2f( finger1X + ( lastVectorX * minDistanceBetween ), finger1Y + ( lastVectorY * minDistanceBetween ) );
				else
					glVertex2f( finger2X, finger2Y );
				glVertex2f( finger1X, finger1Y );
			glEnd();
		}
		else if ( lastDistanceBetween >= minDistanceBetween && lastDistanceBetween <= maxDistanceBetween )
		{
			glColor3f( 0.55, 0.55, 0.55 );
			glLineWidth( 1.5 );
			glBegin( GL_LINES );
				glVertex2f( finger1X, finger1Y );
				glVertex2f( finger1X + ( lastVectorX * minDistanceBetween ), finger1Y + ( lastVectorY * minDistanceBetween ) );
			glEnd();
			glColor3f( 0.45, 0.45, 0.55 );
			glLineWidth( 1 );
			glBegin( GL_LINES );
				glVertex2f( finger1X + ( lastVectorX * minDistanceBetween ), finger1Y + ( lastVectorY * minDistanceBetween ) );
				glVertex2f( finger2X, finger2Y );
			glEnd();
		}
		else
		{
			glColor3f( 0.55, 0.55, 0.55 );
			glLineWidth( 1.5 );
			glBegin( GL_LINES );
				glVertex2f( finger1X, finger1Y );
				glVertex2f( finger1X + ( lastVectorX * minDistanceBetween ), finger1Y + ( lastVectorY * minDistanceBetween ) );
			glEnd();
			glColor3f( 0.45, 0.45, 0.55);
			glLineWidth( 1 );
			glBegin( GL_LINES );
				glVertex2f( finger1X + ( lastVectorX * minDistanceBetween ), finger1Y + (lastVectorY * minDistanceBetween ) );
				glVertex2f( finger1X + ( lastVectorX * maxDistanceBetween ), finger1Y + (lastVectorY * maxDistanceBetween ) );
			glEnd();
			glColor3f( 0.65, 0.25, 0.25 );
			glLineWidth( 1.5 );
			glBegin( GL_LINES );
				glVertex2f( finger1X + ( lastVectorX * maxDistanceBetween ), finger1Y + ( lastVectorY * maxDistanceBetween ) );
				glVertex2f( finger2X, finger2Y );
			glEnd();
		}
	}//end if both fingers valid
}

bool Pantograph::firstFingerSet()
{
	if ( finger1X == -1 && finger1Y == -1 )
		return false;
	else
		return true;
}

bool Pantograph::secondFingerSet()
{
	if ( finger2X == -1 && finger2Y == -1 )
		return false;
	else
		return true;
}

void Pantograph::resetFingers()
{
	finger1X = -1;
	finger1Y = -1;

	finger2X = -1;
	finger2Y = -1;

	selection[0] = -1;
	selection[1] = -1;
	selection[2] = -1;

	swapChecked = false;
}

bool Pantograph::isOnDragBubble( float X, float Y )
{
	float dist = sqrt( ( X - finger2X ) * ( X - finger2X ) + ( Y - finger2Y ) * ( Y - finger2Y ) );
	if ( dist < DRAG_BUBBLE_RADIUS )
		return true;
	else
		return false;
}

bool Pantograph::swapNeeded()
{
	if ( !swapChecked )
	{
		swapChecked = true;
		if ( finger1Y > finger2Y )
			return true;
		else
			return false;			
	}
	return false;
}

bool Pantograph::isOnAcceptBubble( float X, float Y )
{
	return isOnArcButton( X, Y, finger2X, finger2Y, DRAG_BUBBLE_RADIUS + SELECT_INNER_RADIUS, DRAG_BUBBLE_RADIUS + SELECT_OUTER_RADIUS,
						  SELECT_ANGLE_START + lastInterfingerAngle - 1.570795, SELECT_ANGLE_END + lastInterfingerAngle - 1.570795 );
}
