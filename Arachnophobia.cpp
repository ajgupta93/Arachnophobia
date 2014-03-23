#include <iostream>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include <math.h>
#include <GL/glut.h>
using namespace std;

#define epsilon 0.0001
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)

// Function Declarations
void drawSpider(float x,float y,char color);
void drawScene();
void drawCannon();
void update(int value);
void drawBase(float len);
void drawArrow(float x,char color);
void drawBasket(float x,char color);
void initRendering();
void handleResize(int w, int h);
void handleKeypress1(unsigned char key, int x, int y);
void handleKeypress2(int key, int x, int y);
void handleMouseclick(int button, int state, int x, int y);
void dragwithmouse(int x,int y);
void printscore(float x, float y, string String);

// Global Variables
float ball_x = 1.0f;
float ball_y = 0.0f;
float ball_velx = 0.01f;
float ball_vely = 0.02f;
float ball_rad = 0.2f;
float len = 8.0f;
float tri_x = 0.0f;
float tri_y = 0.0f;
float * theta; 
float * xp;
time_t prev,curr,lprev,lcurr;
int selected=0;
bool start=0,fire=0,p=0;
int spidercount=0,lasercount=0;
long long int points=0;
int score = 0;
char myscore[100];
bool gameover=0;

typedef struct baskets {
	char color;
	float x_pos;
	float maxr;
	float maxl;
} baskets;
baskets basket[2];

typedef struct cannons {
	float angle;
	float x_pos;
	float tangle;
	float maxr;
	float maxl;
} cannons;
cannons cannon;

typedef struct beams {
	float top[2];
	float bottom[2];
	float angle;
	bool destroyed;
} beams;
beams laser[100000];
void drawLaser(beams beam);

typedef struct spiders {
	float x;
	float y;
	char color;
	float speed;
	bool destroyed;
} spiders;
spiders spider[100000];

void setup(){
	glClearColor(0.439216f, 0.576471f, 0.858824f, 0.0f);    // backgrouning
}

int main(int argc, char **argv) {

	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	int w = glutGet(GLUT_SCREEN_WIDTH);
	int h = glutGet(GLUT_SCREEN_HEIGHT);
	int windowWidth = w * 2 / 3;
	int windowHeight = h * 2 / 3;

	basket[0].x_pos=3.25f;
	basket[0].color='g';
	basket[0].maxr = basket[1].maxr = cannon.maxr = 3.25;
	basket[0].maxl = basket[1].maxl = cannon.maxl = -3.25;
	basket[1].x_pos=-3.25f;
	basket[1].color='r';

	cannon.x_pos=0;
	cannon.angle=0;
	cannon.tangle=90;

	start=1;
	srand(time(0));
	float temp=0;
	theta=&temp;

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition((w - windowWidth) / 2, (h - windowHeight) / 2);

	glutCreateWindow("Arachnophobia");  // Setup the window
	initRendering();

	// Register callbacks
	glutDisplayFunc(drawScene);
	glutIdleFunc(drawScene);
	glutKeyboardFunc(handleKeypress1);
	glutSpecialFunc(handleKeypress2);
	glutMouseFunc(handleMouseclick);
	glutMotionFunc(dragwithmouse);
	glutReshapeFunc(handleResize);
	glutTimerFunc(10, update, 0);

	glutMainLoop();
	return 0;
}

// Function to draw objects on the screen
void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -5.0f);

	if(gameover==1)
	{
		sprintf(myscore,"Game Over");
		printscore(400.0,200.0,string(myscore));

		sprintf(myscore,"Score: %d",score);
		printscore(780.0f,490.0f,string(myscore));
		glPopMatrix();
		glutSwapBuffers();
		return;
	}

	// Draw Base
	glPushMatrix();
	drawBase(len);
	glPopMatrix();

	//Draw Green Basket
	glPushMatrix();
	glTranslatef(basket[0].x_pos,-1.5f,0.0f);
	drawBasket(basket[0].x_pos,basket[0].color);
	glPopMatrix();
	if(selected==3)
		drawArrow(basket[0].x_pos,'g');

	//Draw Red Basket
	glPushMatrix();
	glTranslatef(basket[1].x_pos,-1.5f,0.0f);
	drawBasket(basket[1].x_pos,basket[1].color);
	glPopMatrix();
	if(selected==2)
		drawArrow(basket[1].x_pos,'r');

	//DrawCannon
	glPushMatrix();
	glTranslatef(cannon.x_pos,-1.5f,0.0f);
	glRotatef(cannon.angle, 0.0f, 0.0f, 1.0f);
	drawCannon();
	glPopMatrix();
	if(selected==1)
		drawArrow(cannon.x_pos,'b');

	time(&curr);
	if(difftime(curr,prev)>=2.0 && !p) {

		spider[spidercount].x=(float)((float)(rand()%100)/100)*6.5-3.25;
		spider[spidercount].y=2.0f;
		int temp=rand()%3;
		if(temp==0)
			spider[spidercount].color='r';
		else if(temp==1)
			spider[spidercount].color='g';
		else
			spider[spidercount].color='b';
		spider[spidercount].speed=(float)((float)(rand()%10)/10)*0.02;
		spider[spidercount].destroyed=0;
		if(spider[spidercount].speed==0)
			spider[spidercount].speed=0.005;
		spidercount++;
		prev=curr;
	}
	for(int i=0;i<spidercount;i++)
	{
		if(spider[i].destroyed==0)
			drawSpider(spider[i].x,spider[i].y,spider[i].color);
	}

	for(int i=0;i<lasercount;i++)
		if(laser[i].destroyed==0)
			drawLaser(laser[i]);

	sprintf(myscore,"Score: %d",score);
	printscore(780.0f,490.0f,string(myscore));
	//printf("%s\n",myscore);
	glutSwapBuffers();
}

// Function to handle all calculations in the scene
// updated evry 10 milliseconds
void update(int value) {

	if(!p) {
		// Handle ball collisions with box
		int i;
		for(i=0;i<spidercount;i++) {

			if(spider[i].y>=-1.6 && spider[i].destroyed==0) {

				spider[i].y-=spider[i].speed;
				float dist=sqrt(pow((cannon.x_pos-spider[i].x),2)+pow((-1.5f-spider[i].y),2));
				if(dist<0.3) {
					gameover=1;
				}
				if(spider[i].y<-1.6)
					score-=5;
				if(spider[i].destroyed==0) {
					if(fabs(basket[0].x_pos-spider[i].x)<=0.4 && spider[i].y+1.5f<0.4f) 
					{
						spider[i].destroyed=1;
						if(spider[i].color=='g')
							score++;
						else
							score--;


					}
					if(fabs(basket[1].x_pos-spider[i].x)<=0.4 && spider[i].y+1.5f<0.4f)
					{
						spider[i].destroyed=1;
						if(spider[i].color=='r')
							score++;
						else
							score--;
					}
				}
			}
			else if(spider[i].destroyed==0)	{

				float dist1 = spider[i].x-cannon.x_pos;
				float dist2 = spider[i].x-basket[0].x_pos;
				float dist3 = spider[i].x-basket[1].x_pos;
				if(dist1<0) { 
					if(fabs(dist1)<fabs(cannon.maxl-cannon.x_pos))
						cannon.maxl=spider[i].x+0.3;
				}
				else if(dist1<cannon.maxr-cannon.x_pos)
					cannon.maxr=spider[i].x-0.3;
				if(dist2<0) { 
					if(fabs(dist2)<fabs(basket[0].maxl-basket[0].x_pos))
						basket[0].maxl=spider[i].x+0.3;
				}
				else if(dist2<basket[0].maxr-basket[0].x_pos)
					basket[0].maxr=spider[i].x-0.3;
				if(dist3<0) {
					if(fabs(dist3)<fabs(basket[1].maxl-basket[1].x_pos))
						basket[1].maxl=spider[i].x+0.3;
				}
				else if(dist3<basket[1].maxr-basket[1].x_pos)
					basket[1].maxr=spider[i].x-0.3;

			}
		}

		for(i=0;i<lasercount;i++) {

			laser[i].top[0]+=0.025*(cos(DEG2RAD(laser[i].angle)));
			laser[i].top[1]+=0.025*(sin(DEG2RAD(laser[i].angle)));
			laser[i].bottom[0]+=0.025*(cos(DEG2RAD(laser[i].angle)));
			laser[i].bottom[1]+=0.025*(sin(DEG2RAD(laser[i].angle)));
		}

		//Handle Laser hitting the spiders and also laser reflecting due to the sidewalls
		float dist;
		for(i=0;i<lasercount;i++)
		{
			if(laser[i].destroyed==0) {
				if(laser[i].top[0]>3.7 || laser[i].top[0]<-3.7)
				{
					glPushMatrix();
					laser[i].angle=180-laser[i].angle;
					laser[i].bottom[0]=laser[i].top[0];
					laser[i].bottom[1]=laser[i].top[1];
					laser[i].top[0]=laser[i].bottom[0]+0.4*cos(DEG2RAD(laser[i].angle));
					laser[i].top[1]=laser[i].bottom[1]+0.4*sin(DEG2RAD(laser[i].angle));
					glPopMatrix();
				}
				for(int j=0;j<spidercount;j++)
				{
					if(spider[j].destroyed==0)
					{

						dist=sqrt(pow((laser[i].top[0]-spider[j].x),2)+pow((laser[i].top[1]-spider[j].y),2));
						if(dist<0.3)
						{
							system("aplay Laser2.wav &");
							if(spider[j].color=='b')
								score++;
							spider[j].destroyed=1;
							laser[i].destroyed=1;
						}

					}
				}
			}
		}

	}
	glutTimerFunc(10, update, 0);
}

void printscore(float x, float y, string String)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	int w = glutGet(GLUT_SCREEN_WIDTH);
	int h = glutGet(GLUT_SCREEN_HEIGHT);
	int windowWidth = w * 2 / 3;
	int windowHeight = h * 2 / 3;
	glOrtho(0, windowWidth, 0,windowHeight, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	glColor3f(0.0,0.0,0.0);
	glRasterPos2f(x,y);
	for (size_t i=0; i<String.size(); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, String[i]);
	}
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void drawArrow(float x,char color) {

	glPushMatrix();
	glTranslatef(x,-4.0f,-5.0f);
	if(color=='g')
		glColor3f(0.0,0.65,0.0);
	else if(color=='r')
		glColor3f(0.65,0.0,0.0);
	else
		glColor3f(0.137255f,0.137255f,0.556863f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_TRIANGLES);
	glVertex3f(x,0.2f,0.0f);
	glVertex3f(x-0.25f,0.0f,0.0f);
	glVertex3f(x+0.25f,0.0f,0.0f);
	glEnd();
	glBegin(GL_QUADS);
	glVertex3f(x-0.15f,0.0f,0.0f);
	glVertex3f(x-0.15f,-0.35f,0.0f);
	glVertex3f(x+0.15f,-0.35f,0.0f);
	glVertex3f(x+0.15f,0.0f,0.0f);
	glEnd();
	glPopMatrix();

}

void drawLaser(beams beam) {

	glPushMatrix();
	//glTranslatef(beam.bottom[0],beam.bottom[1],0.0f);
	glLineWidth(10);
	glColor3f(1.0, 0.1, 0.0);
	glBegin(GL_LINES);
	glVertex3f(beam.bottom[0], beam.bottom[1], 0.0f);
	glVertex3f(beam.top[0], beam.top[1], 0.0f);
	glEnd();
	glLineWidth(1);
	glPopMatrix();

}

void drawSpider(float x,float y,char color) {

	glLineWidth(2);
	glPushMatrix();
	if(color=='r')
		glColor3f(0.9f,0.0f,0.0f);
	else if(color=='g')
		glColor3f(0.0f,0.8f,0.0f);
	else
		glColor3f(0.0f,0.0f,0.0f);
	glTranslatef(x,y,0.0f);

	glPushMatrix();
	glTranslatef(0.0f,0.18f,0.0f);
	glBegin(GL_LINES);

	glVertex3f(0.26f,0.0f,0.0f);
	glVertex3f(-0.26f,0.0f,0.0f);
	glVertex3f(-0.26f,0.0f,0.0f);
	glVertex3f(-0.34f,0.10f,0.0f);
	glVertex3f(0.26f,0.0f,0.0f);
	glVertex3f(0.34f,0.10f,0.0f);

	glVertex3f(0.24f,-0.07f,0.0f);
	glVertex3f(-0.24f,-0.07f,0.0f);
	glVertex3f(-0.24f,-0.07f,0.0f);
	glVertex3f(-0.28f,-0.18f,0.0f);
	glVertex3f(0.24f,-0.07f,0.0f);
	glVertex3f(0.28f,-0.18f,0.0f);

	glVertex3f(0.26f,0.1f,0.0f);
	glVertex3f(-0.26f,0.1f,0.0f);
	glVertex3f(-0.26f,0.1f,0.0f);
	glVertex3f(-0.30f,0.24f,0.0f);
	glVertex3f(0.26f,0.1f,0.0f);
	glVertex3f(0.30f,0.24f,0.0f);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	for(int i=0 ; i<360 ; i++) {
		glVertex2f(0.2f * cos(DEG2RAD(i)), 0.2f * sin(DEG2RAD(i)));
	}
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f,-0.08f,0.0f);
	glBegin(GL_LINES);
	glVertex3f(-0.16f,0.0f,0.0f);
	glVertex3f(0.16f,0.0f,0.0f);
	glVertex3f(-0.16f,0.0f,0.0f);
	glVertex3f(-0.18f,-0.18f,0.0f);
	glVertex3f(0.16f,0.0f,0.0f);
	glVertex3f(0.18f,-0.18f,0.0f);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	for(int i=0 ; i<360 ; i++) {
		glVertex2f(0.1f * cos(DEG2RAD(i)), 0.1f * sin(DEG2RAD(i)));
	}
	glEnd();
	glColor3f(1.0f,1.0f,1.0f);
	glPointSize(4);
	glBegin(GL_POINTS);
	glVertex2f(-0.03f,-0.03f);
	glVertex2f(0.03f,-0.03f);
	glEnd();
	glPointSize(2);
	glBegin(GL_POINTS);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex2f(-0.03f,-0.03f);
	glVertex2f(0.03f,-0.03f);
	glEnd();
	glPopMatrix();

	glEnd();
	glPopMatrix();
	glLineWidth(1);

}

void drawBase(float len) {

	glLineWidth(3.5);
	glColor3f(0.0f, 0.0f, 0.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glVertex2f(-len / 2, 4.0f);
	glVertex2f(len / 2, 4.0f);
	glVertex2f(len / 2, 4.0f);
	glVertex2f(-len / 2, 4.0f);
	glEnd();
	glColor3f(0.36f, 0.20f, 0.09f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glTranslatef(0.0f,-1.8f,0.0f);
	glBegin(GL_QUADS);
	glVertex3f(-len/2,-0.3f,0.0f);
	glVertex3f(len/2,-0.3f,0.0f);
	glVertex3f(len/2,0.3f,0.0f);
	glVertex3f(-len/2,0.3f,0.0f);
	glEnd();

}

void drawCannon() {

	glLineWidth(10);
	glColor3f(0.1f,0.1f,0.1f);
	glBegin(GL_LINES);
	glVertex3f(-0.3f,0.0f,0.0f);
	glVertex3f(0.3f,0.0f,0.0f);

	glVertex3f(-0.3f,0.25f,0.0f);
	glVertex3f(-0.3f,-0.25f,0.0f);

	glVertex3f(0.3f,0.25f,0.0f);
	glVertex3f(0.3f,-0.25f,0.0f);

	glEnd();
	glColor3f(0.137255f,0.137255f,0.556863f);
	glBegin(GL_TRIANGLE_FAN);
	for(int i=0 ; i<360 ; i++) {
		glVertex2f(0.2f * cos(DEG2RAD(i)), 0.2f * sin(DEG2RAD(i)));
	}
	glEnd();
	glColor3f(0.1f,0.1f,0.1f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.4f, 0.0f);
	glEnd();
	glLineWidth(1);
	glBegin(GL_TRIANGLE_FAN);
	for(int i=0 ; i<360 ; i++) {
		glVertex2f(0.08f * cos(DEG2RAD(i)), 0.08f * sin(DEG2RAD(i)));
	}
	glEnd();
}

void drawBasket(float x,char color) {

	if(color=='g')
		glColor3f(0.0,0.65,0.0);
	else if(color=='r')
		glColor3f(0.65,0.0,0.0);

	glPushMatrix();
	glTranslatef(0.0f,-0.35f,0.0f);
	GLUquadricObj * quadricObj;             //Creates a quadratic object
	quadricObj = gluNewQuadric();           //Sets a pointer to a new quadratic object
	gluQuadricDrawStyle(quadricObj, GLU_FILL);
	glScalef(0.5, 0.1, -5.0);                       //Scales the ellipse
	gluDisk(quadricObj, 0.0, 0.68, 100, 100);        //Sets the ellipse properties
	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);
	glVertex3f(-0.35f,-0.35f,0.0f);
	glVertex3f(0.35f,-0.35f,0.0f);
	glVertex3f(0.35f,0.35f,0.0f);
	glVertex3f(-0.35f,0.35f,0.0f);
	glEnd();

	glPushMatrix();
	glTranslatef(0.0f,0.35f,0.0f);
	glColor3f(0.4f,0.4f,0.4f);
	gluQuadricDrawStyle(quadricObj, GLU_FILL);
	glScalef(0.5, 0.1, -5.0);                       //Scales the ellipse
	gluDisk(quadricObj, 0.0, 0.68, 100, 100);        //Sets the ellipse properties
	glPopMatrix();

}
// Initializing some openGL 3D rendering options
void initRendering() {

	glEnable(GL_DEPTH_TEST);        // Enable objects to be drawn ahead/behind one another
	glEnable(GL_COLOR_MATERIAL);    // Enable coloring
	glClearColor(0.137255f, 0.419608f, 0.556863f, 1.0f);   // Setting a background color
}

// Function called when the window is resized
void handleResize(int w, int h) {

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (float)w / (float)h, 0.1f, 200.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

float distance(float x1,float y1,float x2,float y2) {

	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

float mousex,mousey;
int togcan,togred,toggreen,rotcan;
void handleKeypress1(unsigned char key, int x1, int y1) {

	time(&lcurr);
	if (key == 27 || key == 113 || key == 81) {
		exit(0);     // escape key is pressed
	}
	else if (key == 112 || key == 80) {
		p=(p)?0:1;
	}
	if(!p) {
		if (key == 114 || key == 82) {
			xp=&(basket[1].x_pos);
			selected=2;
		}
		else if (key == 103 || key == 71) {
			xp=&(basket[0].x_pos);
			selected=3;
		}
		else if (key == 98 || key == 66) {
			selected=1;
			xp=&(cannon.x_pos);
			theta=&(cannon.angle);
		}
		else if (key == 32 && selected == 1) {
			if(difftime(lcurr,lprev)>=1.0) {

				system("aplay LASER1.WAV &");
				laser[lasercount].angle=cannon.tangle;
				laser[lasercount].destroyed=0;
				laser[lasercount].top[0]=cannon.x_pos+0.8*cos(DEG2RAD(cannon.tangle));
				laser[lasercount].top[1]=-1.5f+0.8*sin(DEG2RAD(cannon.tangle));
				laser[lasercount].bottom[0]=cannon.x_pos+0.4*cos(DEG2RAD(cannon.tangle));
				laser[lasercount].bottom[1]=-1.5f+0.4*sin(DEG2RAD(cannon.tangle));
				lasercount++;
				lprev=lcurr;
			}
		}
	}
}

void handleKeypress2(int key, int x1, int y1) {

	if(!p) {
		if(selected==3)
		{

			if (key == GLUT_KEY_LEFT && basket[0].x_pos>-3.24f && basket[0].x_pos-0.1>basket[0].maxl)
				*xp -= 0.1;
			if (key == GLUT_KEY_RIGHT && basket[0].x_pos<3.24f && basket[0].x_pos+0.1<basket[0].maxr)
				*xp += 0.1;
		}
		if(selected==2)
		{

			if (key == GLUT_KEY_LEFT && basket[1].x_pos>-3.24f && basket[1].x_pos-0.1>basket[1].maxl)
				*xp -= 0.1;
			if (key == GLUT_KEY_RIGHT && basket[1].x_pos<3.24f && basket[1].x_pos+0.1<basket[1].maxr)
				*xp += 0.1;
		}
		if(selected==1)
		{
			if (key == GLUT_KEY_LEFT && cannon.x_pos>-3.24f && cannon.x_pos-0.1>cannon.maxl)
				*xp -= 0.1;
			if (key == GLUT_KEY_RIGHT && cannon.x_pos<3.24f && cannon.x_pos+0.1<cannon.maxr)
				*xp += 0.1;
		}
		if (key == GLUT_KEY_UP && selected == 1 && *theta<=65)
		{
			*theta += 7.5;
			cannon.tangle+=7.5;
		}
		if (key == GLUT_KEY_DOWN && selected == 1 && *theta>=-65)
		{
			*theta -= 7.5;
			cannon.tangle-=7.5;
		}
	}
}

void handleMouseclick(int button, int state, int x, int y) {

	if(!p) {
		if (state == GLUT_DOWN)
		{
			GLint viewport[4];
			GLdouble modelview[16];
			GLdouble projection[16];
			GLfloat winX, winY, winZ;
			GLdouble posX, posY, posZ;

			glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
			glGetDoublev( GL_PROJECTION_MATRIX, projection );
			glGetIntegerv( GL_VIEWPORT, viewport );

			winX = (float)x;
			winY = (float)viewport[3] - (float)y;
			glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

			gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);


			mousex=posX*50.37;
			mousey=posY*50;
			if (button == GLUT_LEFT_BUTTON)
			{
				if(distance(mousex,mousey,cannon.x_pos,-1.5f)<0.5)
				{
					togcan=1;
					selected=1;
					xp=&(cannon.x_pos);
					theta=&(cannon.angle);
					toggreen=0;
					togred=0;
					rotcan=0;

				}
				else if(distance(mousex,mousey,basket[0].x_pos,-1.5f)<0.5)
				{
					xp=&(basket[0].x_pos);
					selected=3;
					togcan=0;
					toggreen=1;
					togred=0;
					rotcan=0;
				}
				else if(distance(mousex,mousey,basket[1].x_pos,-1.5f)<0.5)
				{
					xp=&(basket[1].x_pos);
					selected=2;
					togcan=0;
					toggreen=0;
					togred=1;
					rotcan=0;
				}
				else
				{
					togcan=0;
					toggreen=0;
					togred=0;
					rotcan=0;
				}
			}
			if(button == GLUT_RIGHT_BUTTON)
			{
				togcan=0;
				toggreen=0;
				togred=0;
				if(distance(mousex,mousey,cannon.x_pos,-1.5f)<1.5)
				{
					selected=1;
					rotcan=1;
				}
				else
					rotcan=0;
			}
		}

		glutPostRedisplay();
	}
}

float drag(float position,float lefy,float rify)
{
	if(!p) 
	{
		float  move;
		if(distance(mousex,mousey,position,-1.5f)<0.7)
		{   
			move=(mousex-position);
			if(move>0)
			{   
				if(position+move< rify)
					position+=move;
			}   
			else if(move<0)
			{   
				if(position+move>lefy)
					position+=move;
			}   
		} 
	}  
	return position;
	
}

void dragwithmouse(int x,int y)
{
	if(!p) {

		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX, winY, winZ;
		GLdouble posX, posY, posZ;

		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );

		winX = (float)x;
		winY = (float)viewport[3] - (float)y;

		glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

		gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);


		mousex=posX*50.37;
		mousey=posY*50;
		if(togred)
		{
			basket[1].x_pos=drag(basket[1].x_pos,basket[1].maxl,basket[1].maxr);
		}
		else if(toggreen)
		{
			basket[0].x_pos=drag(basket[0].x_pos,basket[0].maxl,basket[0].maxr);
		}
		else if(togcan)
		{
			cannon.x_pos=drag(cannon.x_pos,cannon.maxl,cannon.maxr);

		}
		if(rotcan && distance(mousex,mousey,cannon.x_pos,-1.5f)<3)
		{
			float hy=distance(cannon.x_pos,-1.5,mousex,mousey);
			float b=mousex-cannon.x_pos;
			float c1= b/hy;
			float ang = acos(c1);
			ang=ang * 180/(3.142);

			if(ang>cannon.tangle && *theta<=65) {
				*theta += 1.875;
				cannon.tangle+=1.875;
			}
			else if(*theta>=-65){
				*theta -= 1.875;
				cannon.tangle-=1.875;
			}

		}
	}

}
