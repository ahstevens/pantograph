#include "DrawFunctions.h"


void drawStrokeLabel3D( float x, float y, float z, float scale, char *text )
{
	//glDisable(GL_LINE_SMOOTH);
	glLineWidth( 1.f );
	glPointSize( 1.f );
    char *p;
    glPushMatrix();
    glTranslatef( x, y, z );
	glScalef( scale, scale, scale );
    for ( p = text; *p; p++ )
	{
        glutStrokeCharacter( GLUT_STROKE_ROMAN, *p );
		glTranslatef( 13.f, 0, 0 );
	}
    glPopMatrix();
}



void drawHalfEllipse( float x, float y, float width, float height, bool archUp, int numSegs )
{
	float centerPointX = x + width / 2.f;
	float centerPointY = y;
	float vertRadius = height;
	float horzRadius = width / 2.f;
	float increment = M_PI / numSegs;

	if ( archUp )
	{
		glBegin( GL_POLYGON );
			for ( float i = 0.f; i < M_PI; i += increment )
				glVertex2f( centerPointX + cos( i ) * horzRadius, centerPointY + sin( i ) * vertRadius );
			glVertex2f( centerPointX + cos(M_PI) * horzRadius, centerPointY + sin(M_PI) * vertRadius );
		glEnd();
	}
	else
	{
		glBegin( GL_POLYGON );
			for ( float i = M_PI; i < ( 2.f * M_PI); i += increment )
				glVertex2f( centerPointX + cos( i ) * horzRadius, centerPointY + sin( i ) * vertRadius );
			glVertex2f( centerPointX + cos( 2.f * M_PI) * horzRadius, centerPointY + sin( 2.f * M_PI) * vertRadius );
		glEnd();
	}
}//end drawHalfEllipse

void drawBox( float x, float y, float sizeX, float sizeY, bool solid )
{
	if ( solid )
	{
		glBegin( GL_QUADS );
			//bottom left
			glVertex2f( x, y );
			//top left
			glVertex2f( x, y + sizeY );
			//top right
			glVertex2f( x + sizeX, y + sizeY );
			//bottom right
			glVertex2f( x + sizeX, y );
		glEnd();
	}
	else
	{
		glBegin( GL_LINES );
			//top right to top left
			glVertex2f( x, y + sizeY );
			glVertex2f( x + sizeX, y + sizeY );
			//top left to bottom left
			glVertex2f( x + sizeX, y + sizeY );
			glVertex2f( x + sizeX, y );
			//bottom left to bottom right
			glVertex2f( x + sizeX, y );
			glVertex2f( x, y );
			//bottom right to top left
			glVertex2f( x, y );
			glVertex2f( x, y + sizeY );			
		glEnd();
	}
}//end drawBox

void drawLeftArrowBox( float x, float y, float width, float height )
{	
	glColor3f( BUTTON_OUTLINE_COLOR );
	drawBox( x, y, width, height, true );

	glColor3f( BUTTON_COLOR );
	drawBox( x + 1.f, y + 1.f, width - 2.f, height - 2.f, true );

	glColor3f( BUTTON_LINE_COLOR );

	glBegin( GL_TRIANGLES );
		glVertex2f( x + width * 0.80f, y + height * 0.20f );		
		glVertex2f( x + width * 0.20f, y + height * 0.50f );
		glVertex2f( x + width * 0.80f, y + height * 0.80f );
	glEnd();
}//end drawLeftArrowBox

void drawRightArrowBox( float x, float y, float width, float height )
{
	glColor3f( BUTTON_OUTLINE_COLOR );
	drawBox( x, y, width, height, true );

	glColor3f( BUTTON_COLOR );
	drawBox( x + 1.f, y + 1.f, width - 2.f, height - 2.f, true );

	glColor3f( BUTTON_LINE_COLOR );

	glBegin( GL_TRIANGLES );
		glVertex2f( x + width * 0.20f, y + height * 0.80f );	
		glVertex2f( x + width * 0.80f, y + height * 0.50f );
		glVertex2f( x + width * 0.20f, y + height * 0.20f );		
	glEnd();
}//end drawRightArrowBox

void drawPercentageBox( float x, float y, float width, float height, float amount, float c1r, float c1g, float c1b, float c2r, float c2g, float c2b )
{
	glColor3f( BUTTON_OUTLINE_COLOR );
	drawBox( x, y, width, height, true );

	float width1 = amount * ( width - 2.f );
	float width2 = ( 1.f - amount ) * ( width - 2.f );

	glColor3f( c1r, c1g, c1b );
	drawBox( x + 1.f, y + 1.f, width1, height - 2.f, true );

	glColor3f( c2r, c2g, c2b );
	drawBox( x + 1.f + width1, y + 1.f, width2, height - 2.f, true );

}

void drawNukeSymbol3D( float x, float y, float z, float radius )
{
	float radFactor = radius / 11.f;
	glColor3f( 0.80f, 0.80f, 0.f );
	//draw yellow background circle
	glBegin( GL_POLYGON );
	for ( float i = 0.f; i < ( 2.f * M_PI); i += 0.15f )
		glVertex3f( x + cos( i ) * radius, y + sin( i ) * radius, z - 1 );
	glEnd();

	glColor3f( 0.65f, 0.65f, 0.25f );
	glBegin( GL_TRIANGLE_STRIP );
	for ( float i = 0.f; i < ( 2.f * M_PI); i += 0.15f )
	{
		glVertex3f( x + cos( i ) * radius, y + sin( i ) * radius, z - 1.f );
		glVertex3f( x + cos( i ) * radius, y + sin( i ) * radius, z - 10.f );
	}
	glVertex3f( x + cos( 0.f ) * radius, y + sin( 0.f ) * radius, z - 1.f );
	glVertex3f( x + cos( 0.f ) * radius, y + sin( 0.f ) * radius, z - 10.f );
	glEnd();
	


	glColor3f( 0.f, 0.f, 0.f );
	//draw inner black ring
	float innerRadius = radFactor*2;
	glBegin( GL_POLYGON );
	for ( float i = 0.f; i < 2.f * M_PI; i += 0.10f )
		glVertex3f( x + cos( i ) * innerRadius, y + sin( i ) * innerRadius, z );
	glEnd();
	//draw trefoils
	float outerRadius = radFactor * 10.f;
	innerRadius = radFactor * 3.f;
	glBegin( GL_TRIANGLE_STRIP );
	for ( float i = 0.f; i < M_PI / 3.f; i += 0.1f )
	{
		glVertex3f( x + cos( i ) * innerRadius, y + sin( i ) * innerRadius, z );
		glVertex3f( x + cos( i ) * outerRadius, y + sin( i ) * outerRadius, z );
	}
	glEnd();

	glBegin( GL_TRIANGLE_STRIP );
	for ( float i =  2.f * M_PI / 3.f ; i < M_PI; i += 0.1f )
	{
		glVertex3f( x + cos( i ) * innerRadius, y + sin( i ) * innerRadius, z );
		glVertex3f( x + cos( i ) * outerRadius, y + sin( i ) * outerRadius, z );
	}
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
	for ( float i = 4.f * M_PI / 3.f; i < 5.f * M_PI / 3.f; i += 0.1f )
	{
		glVertex3f( x + cos( i ) * innerRadius, y + sin( i ) * innerRadius, z );
		glVertex3f( x + cos( i ) * outerRadius, y + sin( i ) * outerRadius, z );
	}
	glEnd();
}//end draw nuke

void drawMapPoint3D( float x, float y, float z, float radius )
{
	float radFactor = radius / 3.f;
	glColor3f( 0.2f, 0.2f, 0.2f );
	//draw background circle
	glBegin( GL_POLYGON );
	for ( float i = 0.f; i < 2.f * M_PI; i += 0.15f )
		glVertex3f( x + cos( i ) * radius, y + sin( i ) * radius, z - 1.f);
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
	for ( float i = 0.f; i < 2.f * M_PI; i += 0.15f )
	{
		glVertex3f( x + cos( i ) * radius, y + sin( i ) * radius, z - 1.f);
		glVertex3f( x + cos( i ) * radius, y + sin( i ) * radius, z - 10.f);
	}
	glVertex3f( x + cos( 0.f ) * radius, y + sin( 0.f ) * radius, z - 1.f);
	glVertex3f( x + cos( 0.f ) * radius, y + sin( 0.f ) * radius, z - 10.f);
	glEnd();


	radius = radFactor * 2.f;
	glColor3f( 0.65f, 0.15f, 0.15f );
	glBegin( GL_POLYGON );
	for ( float i = 0.f; i < 2.f * M_PI; i += 0.15f )
		glVertex3f( x + cos( i ) * radius, y + sin( i ) * radius, z - 1.f );
	glEnd();
	
}//end draw map point

float getDigitsOffset( float value ) //value for half each digit, used to center labels, double for total offset per digit
{
	if ( value >= 1000000.f )
		return 24.5f; 
	else if ( value >= 100000.f )
		return 21.f; 
	else if ( value >= 10000.f )
		return 17.5f;
	else if ( value >= 1000.f )
		return 14.f;
	else if ( value >= 100.f )
		return 10.5f;
	else if ( value >= 10.f )
		return 7.f;
	else
		return 3.5f;
}

void drawCircle(float centerX, float centerY, float radius, float lineWidth, float numSegs)
{
	float increment = 6.28318 / numSegs;
	if (lineWidth <= 0) //draw filled is lineWidth is negative
	{
		glBegin(GL_POLYGON);
	}
	else
	{
		glLineWidth(lineWidth);
		glBegin(GL_LINE_LOOP);
	}

	for (float i=0; i < 2 * M_PI; i+=increment)
	{
		glVertex2f(centerX+cos(i)*radius, centerY+sin(i)*radius);
	}
	glEnd();
}

void drawInsertButton(float x, float y, float width, float height)
{
	glEnable(GL_BLEND);
	//draw faint circle
	glColor4f(0.75,0.75,0.75, 0.25);
		drawCircle(x+width/2, y+height/2, width/2, -1, 32);
	glColor4f(1,1,1,1);

	//draw plus sign inside
	x = x+width/4;
	y = y+height/4;
	width /= 2;
	height /= 2;

	glBegin(GL_QUADS);

		//outside
		glColor3f(0,0.75,0);

		//horz bar
		glVertex2f(x, y + height/2 - 2.5);
		glVertex2f(x, y + height/2 + 2.5);
		glVertex2f(x+width, y + height/2 + 2.5);
		glVertex2f(x+width, y + height/2 - 2.5);
		
		//vert bar
		glVertex2f(x + width/2 - 2.5, y);
		glVertex2f(x + width/2 - 2.5, y+height);
		glVertex2f(x + width/2 + 2.5, y+height);
		glVertex2f(x + width/2 + 2.5, y);
		
		//inside
		glColor3f(0,0.85,0);
		
		//horz bar
		glVertex2f(x+1, y + height/2 - 1.5);
		glVertex2f(x+1, y + height/2 + 1.5);
		glVertex2f(x+width-1, y + height/2 + 1.5);
		glVertex2f(x+width-1, y + height/2 - 1.5);

		//vert bar
		glVertex2f(x + width/2 - 1.5, y+1);
		glVertex2f(x + width/2 - 1.5, y+height-1);
		glVertex2f(x + width/2 + 1.5, y+height-1);
		glVertex2f(x + width/2 + 1.5, y+1);

	glEnd();

	glDisable(GL_BLEND);
	//glEnable(GL_LINE_SMOOTH);
}

void drawMoveUpButton(float x, float y, float width, float height)
{
	glEnable(GL_BLEND);
	//draw faint triangle
	glColor4f(0.75,0.75,0.75, 0.25);
		glBegin(GL_TRIANGLES);
			glVertex2f(x,y);
			glVertex2f(x+width/2,y+height);
			glVertex2f(x+width,y);
		glEnd();
	glColor4f(1,1,1,1);

	//draw up symbol inside
	x += 9;
	y += 6;
	width -= 18;
	height -= 17;

	glBegin(GL_TRIANGLES);

		//outside
		glColor3f(0,0.75,0);

		//outer triangle
		glVertex2f(x,y);
		glVertex2f(x+width/2,y+height);
		glVertex2f(x+width,y);
	
		//inside
		glColor3f(0,0.85,0);

		x+=1;
		y+=1;
		width-=2;
		height-=2;

		//inner triangle
		glVertex2f(x,y);
		glVertex2f(x+width/2,y+height);
		glVertex2f(x+width,y);
	
	glEnd();

	glDisable(GL_BLEND);
}

void drawMoveDownButton(float x, float y, float width, float height)
{
	glEnable(GL_BLEND);
	//draw faint triangle
	glColor4f(0.75,0.75,0.75, 0.25);
		glBegin(GL_TRIANGLES);
			glVertex2f(x,y+height);
			glVertex2f(x+width/2,y);
			glVertex2f(x+width,y+height);
		glEnd();
	glColor4f(1,1,1,1);

	//draw up symbol inside
	x += 9;
	y += 9;
	width -= 18;
	height -= 17;

	glBegin(GL_TRIANGLES);

		//outside
		glColor3f(0,0.75,0);

		//outer triangle
		glVertex2f(x,y+height);
		glVertex2f(x+width/2,y);
		glVertex2f(x+width,y+height);
	
		//inside
		glColor3f(0,0.85,0);

		x+=1;
		y+=1;
		width-=2;
		height-=2;

		//inner triangle
		glVertex2f(x,y+height);
		glVertex2f(x+width/2,y);
		glVertex2f(x+width,y+height);
	
	glEnd();

	glDisable(GL_BLEND);
}


void drawLabeled2LineButton(float x, float y, float width, float height, bool active, char* line1, char* line2)
{
	glColor3f(BUTTON_OUTLINE_COLOR);
	drawBox(x, y, width, height, true);

	if (active)
		glColor3f(BUTTON_COLOR);
	else
		glColor3f(BUTTON_COLOR_INACTIVE);
	drawBox(x+1, y+1, width-2, height-2, true);

	if (active)
		glColor3f(1,1,1);
	else
		glColor3f(0.6,0.6,0.6);
	
	glLineWidth(BUTTON_LABEL_LINE_WIDTH);
	float offset = strlen(line1) * 3.5;
	drawStrokeLabel3D(x+width/2-offset, y+height/2+3, 0, 0.09, line1);
	offset = strlen(line2) * 3.5;
	drawStrokeLabel3D(x+width/2-offset, y+height/2-13, 0, 0.09, line2);
}//end draw labeled 2 line button

void drawLabeledButton(float x, float y, float width, float height, bool active, char* label)
{
	
	glColor3f(BUTTON_OUTLINE_COLOR);
	drawBox(x, y, width, height, true);

	if (active)
		glColor3f(BUTTON_COLOR);
	else
		glColor3f(BUTTON_COLOR_INACTIVE);
	drawBox(x+1, y+1, width-2, height-2, true);

	if (active)
		glColor3f(1,1,1);
	else
		glColor3f(0.6,0.6,0.6);
	
	glLineWidth(BUTTON_LABEL_LINE_WIDTH);
	float offset = strlen(label) * 3.5;
	drawStrokeLabel3D(x+width/2-offset, y+height/2-5, 0, 0.09, label);
}//end draw labeled 2 line button

void drawReticle(float centerX, float centerY, float radius, float radian)
{
	drawCircle(centerX, centerY, radius, 2, 64);
	float innermostRadius = radius * 0.20;
	float middleRadius = radius * 0.6;
	
	float radian1 = radian;
	float radian2 = radian1 + M_PI / 2.f;
	float radian3 = radian2 + M_PI / 2.f;
	float radian4 = radian3 + M_PI / 2.f;

	glVertex2f(centerX+cos(radian1)*radius, centerY+sin(radian1)*radius);

	glLineWidth(1);
	glBegin(GL_LINES);
		glVertex2f(centerX+cos(radian1)*innermostRadius, centerY+sin(radian1)*innermostRadius);
		glVertex2f(centerX+cos(radian1)*radius, centerY+sin(radian1)*radius);

		glVertex2f(centerX+cos(radian2)*innermostRadius, centerY+sin(radian2)*innermostRadius);
		glVertex2f(centerX+cos(radian2)*radius, centerY+sin(radian2)*radius);

		glVertex2f(centerX+cos(radian3)*innermostRadius, centerY+sin(radian3)*innermostRadius);
		glVertex2f(centerX+cos(radian3)*radius, centerY+sin(radian3)*radius);

		glVertex2f(centerX+cos(radian4)*innermostRadius, centerY+sin(radian4)*innermostRadius);
		glVertex2f(centerX+cos(radian4)*radius, centerY+sin(radian4)*radius);
	glEnd();

	glLineWidth(3);
	glBegin(GL_LINES);
		glVertex2f(centerX+cos(radian1)*middleRadius, centerY+sin(radian1)*middleRadius);
		glVertex2f(centerX+cos(radian1)*radius, centerY+sin(radian1)*radius);

		glVertex2f(centerX+cos(radian2)*middleRadius, centerY+sin(radian2)*middleRadius);
		glVertex2f(centerX+cos(radian2)*radius, centerY+sin(radian2)*radius);

		glVertex2f(centerX+cos(radian3)*middleRadius, centerY+sin(radian3)*middleRadius);
		glVertex2f(centerX+cos(radian3)*radius, centerY+sin(radian3)*radius);

		glVertex2f(centerX+cos(radian4)*middleRadius, centerY+sin(radian4)*middleRadius);
		glVertex2f(centerX+cos(radian4)*radius, centerY+sin(radian4)*radius);
	glEnd();
}

void drawFingerIndicator(float centerX, float centerY, float radius, float color)
{
	if (color == 0)
		glColor3f(0.85,0.85,0.85);
	else if (color == 1)
		glColor3f(0.0,0.85,1.0);
	else if (color == 2)
		glColor3f(1.0,0.85,0.0);

	drawCircle(centerX, centerY, radius, 1.5, 64);
}

void draw3DReticle(float centerX, float centerY, float centerZ, float radius, float radian)
{
	float sideRadius = radius * 0.707106781; //cos(45deg)

	glPushMatrix();
		glTranslatef(centerX, centerY, centerZ);
		glRotatef(radian*180.f/M_PI, 0.25, 0.65, 0.15);

		glBegin(GL_LINES);
			glVertex3f(-radius, 0, 0);
			glVertex3f(radius, 0, 0);

			glVertex3f(0, -radius, 0);
			glVertex3f(0, radius, 0);

			glVertex3f(0, 0, -radius);
			glVertex3f(0, 0, radius);

			glVertex3f(-sideRadius, -sideRadius, -sideRadius);
			glVertex3f(sideRadius, sideRadius, sideRadius);

			glVertex3f(sideRadius, -sideRadius, -sideRadius);
			glVertex3f(-sideRadius, sideRadius, sideRadius);

			glVertex3f(sideRadius, sideRadius, -sideRadius);
			glVertex3f(-sideRadius, -sideRadius, sideRadius);

			glVertex3f(-sideRadius, sideRadius, -sideRadius);
			glVertex3f(sideRadius, -sideRadius, sideRadius);
		glEnd();

	glPopMatrix();
}

void drawArcButton(float centerX, float centerY, float innerRadius, float outerRadius, float startAngle, float endAngle, char* label, bool filled)
{

	if (filled)
	{
		float currentColor[4];
		glGetFloatv(GL_CURRENT_COLOR, currentColor);

		glEnable(GL_BLEND);
		glColor4f(currentColor[0],currentColor[1],currentColor[2],0.4);
		glBegin(GL_TRIANGLE_STRIP);
		for (float i=startAngle;i<=endAngle;i+=0.05)
		{
			glVertex2f(centerX+cos(i)*innerRadius, centerY+sin(i)*innerRadius);
			glVertex2f(centerX+cos(i)*outerRadius, centerY+sin(i)*outerRadius);
		}
		glEnd();
		glDisable(GL_BLEND);

		glColor3f(currentColor[0],currentColor[1],currentColor[2]);
	}

	glLineWidth(1);
	glBegin(GL_LINE_LOOP);
	for (float i=startAngle;i<=endAngle;i+=0.05)
	{
		glVertex2f(centerX+cos(i)*outerRadius, centerY+sin(i)*outerRadius);
	}
	for (float i=endAngle;i>=startAngle;i-=0.05)
	{
		glVertex2f(centerX+cos(i)*innerRadius, centerY+sin(i)*innerRadius);
	}
	glEnd();
	float midAngle = (startAngle+endAngle)/2;
	float midRadius = (innerRadius+outerRadius)/2;
	float offset = strlen(label) * 3.5;
	drawStrokeLabel3D(centerX+cos(midAngle)*(midRadius)-offset,
					  centerY+sin(midAngle)*(midRadius), 0, 0.07, label);
	
}

void drawArcButton(float centerX, float centerY, float innerRadius, float outerRadius, float startAngle, float endAngle, char* label1, char* label2, bool filled)
{

	if (filled)
	{
		float currentColor[4];
		glGetFloatv(GL_CURRENT_COLOR, currentColor);

		glEnable(GL_BLEND);
		glColor4f(currentColor[0],currentColor[1],currentColor[2],0.4);
		glBegin(GL_TRIANGLE_STRIP);
		for (float i=startAngle;i<=endAngle;i+=0.05)
		{
			glVertex2f(centerX+cos(i)*innerRadius, centerY+sin(i)*innerRadius);
			glVertex2f(centerX+cos(i)*outerRadius, centerY+sin(i)*outerRadius);
		}
		glEnd();
		glDisable(GL_BLEND);

		glColor3f(currentColor[0],currentColor[1],currentColor[2]);
	}

	glLineWidth(1);
	glBegin(GL_LINE_LOOP);
	for (float i=startAngle;i<=endAngle;i+=0.05)
	{
		glVertex2f(centerX+cos(i)*outerRadius, centerY+sin(i)*outerRadius);
	}
	for (float i=endAngle;i>=startAngle;i-=0.05)
	{
		glVertex2f(centerX+cos(i)*innerRadius, centerY+sin(i)*innerRadius);
	}
	glEnd();
	float midAngle = (startAngle+endAngle)/2;
	float midRadius = (innerRadius+outerRadius)/2;
	float offset1 = strlen(label1) * 3.5;
	float offset2 = strlen(label2) * 3.5;
	drawStrokeLabel3D(centerX+cos(midAngle)*(midRadius)-offset1+2,
					  centerY+sin(midAngle)*(midRadius)+4, 0, 0.06, label1);
	drawStrokeLabel3D(centerX+cos(midAngle)*(midRadius)-offset2+2,
					  centerY+sin(midAngle)*(midRadius)-8, 0, 0.06, label2);
	
}



bool isOnArcButton(float x, float y, float centerX, float centerY, float innerRadius, float outerRadius, float minAngle, float maxAngle)
{
	float vectorX = centerX - x;
	float vectorY = centerY - y;
	float dist = sqrt(  pow((vectorX),2) + pow((vectorY),2) );
	float angle = atan2(vectorY, vectorX) + M_PI;//atan(vectorY/vectorX);
	printf("dist = %f, angle = %f\n", dist, angle);
	if (angle >= minAngle && angle <= maxAngle)
	{
		if (dist > innerRadius && dist < outerRadius)
			return true;
	}
	return false;
}

void drawArcScale(float centerX, float centerY, float radius, float startAngle, float endAngle, float indicatorAngle, char* startLabel, char* endLabel, char* scaleLabel)
{
	//draw indicator
	glColor3f(1.0,0.85,0.0);

	if (indicatorAngle > endAngle)
		indicatorAngle = endAngle;
	if (indicatorAngle < startAngle)
		indicatorAngle = startAngle;
	float indicatorRadius = radius + 15;
	glBegin(GL_TRIANGLES);
		glVertex2f(centerX+cos(indicatorAngle)*radius, centerY+sin(indicatorAngle)*radius);
		glVertex2f(centerX+cos(indicatorAngle+0.03)*indicatorRadius, centerY+sin(indicatorAngle+0.03)*indicatorRadius);
		glVertex2f(centerX+cos(indicatorAngle-0.03)*indicatorRadius, centerY+sin(indicatorAngle-0.03)*indicatorRadius);
	glEnd();

	//draw arc
	glColor3f(1.0,0.65,0.0);
	glLineWidth(1);
	glBegin(GL_LINE_STRIP);
	for (float i=startAngle-0.01;i<=endAngle+0.01;i+=0.05)
		glVertex2f(centerX+cos(i)*radius, centerY+sin(i)*radius);
	glEnd();

	//draw ticks on arc
	float angleRange = endAngle - startAngle;
	float numTicks = 15;
	float anglesPerTick = angleRange / numTicks;
	glBegin(GL_LINES);
	float tickRadius = radius + 6;
	for (float i=startAngle-0.01;i<=endAngle+0.01;i+=anglesPerTick)
	{
		glVertex2f(centerX+cos(i)*radius, centerY+sin(i)*radius);
		glVertex2f(centerX+cos(i)*tickRadius, centerY+sin(i)*tickRadius);
	}
	glEnd();

	//draw labels
	float labelRadius = radius + 20;
	float offset = strlen(startLabel) * 3.5;
	drawStrokeLabel3D(centerX+cos(startAngle)*(labelRadius)-offset,
					  centerY+sin(startAngle)*(labelRadius), 0, 0.08, startLabel);
	offset = strlen(endLabel) * 3.5;
	drawStrokeLabel3D(centerX+cos(endAngle)*(labelRadius)-offset,
					  centerY+sin(endAngle)*(labelRadius), 0, 0.08, endLabel);
	offset = strlen(scaleLabel) * 3.5;
	float midAngle = (startAngle + endAngle)/2;
	drawStrokeLabel3D(centerX+cos(midAngle)*(labelRadius)-offset,
					  centerY+sin(midAngle)*(labelRadius), 0, 0.08, scaleLabel);
}


void drawVolumeCursor(float x, float y, float z, float radius)
{
	GLUquadric* sphere;
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluQuadricTexture(sphere, GL_TRUE);
	gluQuadricOrientation(sphere, GLU_OUTSIDE);

	boolean blendAlreadyOn;
	glGetBooleanv(GL_BLEND, &blendAlreadyOn);

	//draw 3d volumetric cursor
	if(!blendAlreadyOn) glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix();
		glTranslatef(x, y, z);
		glColor4f(0.35,0.35,1.0, 0.1);
		glDepthMask(GL_FALSE);
		gluSphere(sphere, radius, 32, 32);
		glDepthMask(GL_TRUE);

		glColor4f(0.35,0.35,1.0, 0.75);

		//draw one circle around it perpendicular to the camera to maintain screen size/area
		glLineWidth(1);
		glBegin(GL_LINE_LOOP);
		for (float i = 0; i < 2.f*M_PI; i += 0.15)
		{
			glVertex3f(cos(i)*radius, sin(i)*radius, 0);
		}
		glEnd();

		int msPerRotation = 3000;
		float spinAngle = ((float)((int)GetTickCount64()%msPerRotation)/msPerRotation)*360;

		//rotate other circles to attract attention
		glRotatef(spinAngle,0.25,0.75,0.4);
		glBegin(GL_LINE_LOOP);
		for (float i = 0; i < 2.f*M_PI; i += 0.15)
			glVertex3f(cos(i)*radius, sin(i)*radius, 0);
		glEnd();
		glBegin(GL_LINE_LOOP);
		for (float i = 0; i < 2.f*M_PI; i += 0.15)
			glVertex3f(cos(i)*radius, 0, sin(i)*radius);
		glEnd();
		glBegin(GL_LINE_LOOP);
		for (float i = 0; i < 2.f*M_PI; i += 0.15)
			glVertex3f(0, cos(i)*radius, sin(i)*radius);
		glEnd();

	glPopMatrix();
	if(!blendAlreadyOn) glDisable(GL_BLEND);
}

void drawAxes(float scale)
{
	glBegin(GL_LINES);
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(scale, 0.f, 0.f);
	glColor3f(0.f, 1.f, 0.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, scale, 0.f);
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, scale);
	glEnd();
}