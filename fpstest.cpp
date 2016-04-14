// Edited by: Nicholas Gardner
// Program: FPS framework
// Purpose: This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#include <stdio.h>
extern "C" {
    #include "fonts.h"
}


#define RAND ((float)rand() / RAND_MAX)
#define PI 3.14159

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int mousex = 0;
int mousey = 0;
float rotx = 0;
float roty = PI / 2.0;

#define MAX_PARTICLES 50000
#define GRAVITY 0.1
#define BOUNCE 0.1
#define MAX_BOXES 5
#define MAX_CIRC 2


//X Windows variables
Display *dpy;
Window root;
Window win;
GLXContext glc;

//Structures
struct Vec {
    float x, y, z;
};




struct Game {
    
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);
void ShowCursor(const int onoff);

Vec normal(Vec A, Vec B, Vec C);
Vec cross(Vec A, Vec B);
Vec normalize(Vec A);


int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;
    
    //start animation
    while(!done) {
        while(XPending(dpy)) {
            XEvent e;
            XNextEvent(dpy, &e);
            check_mouse(&e, &game);
            done = check_keys(&e, &game);
            
            // Deal with window resize.
            if (e.type == ConfigureNotify) {
                XConfigureEvent xce = e.xconfigure;
                if (xce.width != WINDOW_WIDTH || xce.height != WINDOW_HEIGHT) {
                    WINDOW_WIDTH = xce.width;
                    WINDOW_HEIGHT = xce.height;
                    init_opengl();
                }
            }
            
        }
        movement(&game);
        render(&game);
        glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Test");
}

void cleanupXWindows(void)
{
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void)
{
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    root = DefaultRootWindow(dpy);
    
    XWindowAttributes getWinAttr;
    XGetWindowAttributes(dpy, root, &getWinAttr);
    
    XGrabKeyboard(dpy, root,
                  False, GrabModeAsync, GrabModeAsync, CurrentTime);
    
    WINDOW_WIDTH = getWinAttr.width;
    WINDOW_HEIGHT = getWinAttr.height;
    
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
    ButtonPress | ButtonReleaseMask |
    PointerMotionMask |
    StructureNotifyMask | SubstructureNotifyMask;
    swa.override_redirect = True;
    //win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
    //                    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    
    win = XCreateWindow(dpy, root, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, vi->depth,
                        InputOutput, vi->visual, CWBorderPixel|CWColormap|CWEventMask|CWOverrideRedirect, &swa);
    
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
   
    
}
/*
void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(1.0, 1.0, 1.0, 1.0);
    ///////////////////////////////////
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}
*/
void ShowCursor(const int onoff)
{
    // See: asteroids.cpp
    if (onoff) {
        //this removes our own blank cursor.
        XUndefineCursor(dpy, win);
        return;
    }
    //vars to make blank cursor
    Pixmap blank;
    XColor dummy;
    char data[1] = {0};
    Cursor cursor;
    //make a blank cursor
    blank = XCreateBitmapFromData (dpy, win, data, 1, 1);
    if (blank == None)
		std::cout << "error: out of memory." << std::endl;
    cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
    XFreePixmap(dpy, blank);
    //this makes you the cursor. then set it using this function
    XDefineCursor(dpy, win, cursor);
    //after you do not need the cursor anymore use this function.
    //it will undo the last change done by XDefineCursor
    //(thus do only use ONCE XDefineCursor and then XUndefineCursor):
}

void init_opengl(void)
{
    float w = WINDOW_WIDTH;
    float h = WINDOW_HEIGHT;
    //OpenGL initialization
    //glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    
    glFrustum(-1.0/h*w, 1.0/h*w, -1, 1, 1.5, 200.0);
    //Set the screen background color
    glClearColor(1.0, 1.0, 1.0, 1.0);
    ///////////////////////////////////
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();


   glEnable(GL_COLOR_MATERIAL);
   
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);
   ShowCursor(0);

    
   GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat mat_shininess[] = { 50.0 };
   GLfloat light_position[] = { 0.0, 0.0, 0.0, 0.0 };
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_SMOOTH);

   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}
void check_mouse(XEvent *e, Game *game)
{
    
    
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button was pressed
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed
            return;
        }
    }
    //Did the mouse move?
    /*
    if (mousex != e->xbutton.x || mousey != e->xbutton.y) {
        mousex = e->xbutton.x;
        mousey = e->xbutton.y;
    }
    */
    int movex = e->xbutton.x - (WINDOW_WIDTH / 2);
    int movey = e->xbutton.y - (WINDOW_HEIGHT / 2);
    rotx +=(float) movex / 4000000.0 * WINDOW_HEIGHT; // <--Not a mistake. Use WH for scale!
    roty +=(float) movey / 4000000.0 * WINDOW_HEIGHT;
    rotx = fmod(rotx,2.0*PI);
    if (rotx < 0.0)
	rotx += PI * 2.0;
    if (roty > PI)
	roty = PI;
    if (roty < 0)
	roty = 0;
    if (movex != 0 || movey != 0)
        XWarpPointer(dpy,root,root,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,WINDOW_WIDTH/2,WINDOW_HEIGHT/2);
    

}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }
        //You may check other keys here.
        
    }
    return 0;
}

void movement(Game *game)
{
    
}

Vec normal(Vec A, Vec B, Vec C)
{
    Vec X;
    Vec Y;
    X.x = A.x - B.x;
    X.y = A.y - B.y;
    X.z = A.z - B.z;
    Y.x = A.x - C.x;
    Y.y = A.y - C.y;
    Y.z = A.z - C.z;
    return normalize(cross(X,Y));
}

Vec cross(Vec A, Vec B)
{
    Vec X;
    X.x = A.y*B.z-A.z*B.y;
    X.y = A.z*B.x-A.x*B.z;
    X.z = A.x*B.y-A.y*B.x;
    return X;
}

Vec normalize(Vec A)
{
    float mag = -1.0 * sqrt(pow(A.x,2)+pow(A.y,2)+pow(A.z,2));
    A.x /= mag;
    A.y /= mag;
    A.z /= mag;
    return A;
}


void render(Game *game)
{
    static float val = 0.0;
    val += 0.01;
    float ra = 10.0;
    //Rect r;
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float top = 2.0 * (cos(val) + 1.0) + 1.0;
    float bot = -1.0;
    
    
    Vec a,b,c,d;

    glColor3ub(200,200,200);
    glPushMatrix();
    glBegin(GL_POLYGON);
    a.x = ra * cos(-rotx);
    a.y = bot;
    a.z = ra * sin(-rotx);
    b.x = ra * cos(-rotx+PI/2.0);
    b.y = bot;
    b.z = ra * sin(-rotx+PI/2.0);
    c.x = ra * cos(-rotx+PI);
    c.y = bot;
    c.z = ra * sin(-rotx+PI);
    d.x = ra * cos(-rotx+PI/2.0*3.0);
    d.y = bot;
    d.z = ra * sin(-rotx+PI/2.0*3.0);
    Vec N = normal(a,b,c);
    glNormal3f(N.x,N.y,N.z);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glVertex3f(c.x, c.y, c.z);
    glVertex3f(d.x, d.y, d.z);
    glEnd();
    glPopMatrix();
    
    
    glColor3ub(200,200,255);
    glPushMatrix();
    glBegin(GL_POLYGON);
    a.x = ra * cos(-rotx);
    a.y = top;
    a.z = ra * sin(-rotx);
    b.x = ra * cos(-rotx+PI/2.0);
    b.y = top;
    b.z = ra * sin(-rotx+PI/2.0);
    c.x = ra * cos(-rotx+PI);
    c.y = top;
    c.z = ra * sin(-rotx+PI);
    d.x = ra * cos(-rotx+PI/2.0*3.0);
    d.y = top;
    d.z = ra * sin(-rotx+PI/2.0*3.0);
    N = normal(a,b,c);
    glNormal3f(N.x,N.y,N.z);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glVertex3f(c.x, c.y, c.z);
    glVertex3f(d.x, d.y, d.z);
    glEnd();
    glPopMatrix();
    
    
    glColor3ub(255,200,200);
    glPushMatrix();
    glBegin(GL_POLYGON);
    a.x = ra * cos(-rotx);
    a.y = bot;
    a.z = ra * sin(-rotx);
    b.x = ra * cos(-rotx+PI/2.0);
    b.y = bot;
    b.z = ra * sin(-rotx+PI/2.0);
    c.x = ra * cos(-rotx+PI/2.0);
    c.y = top;
    c.z = ra * sin(-rotx+PI/2.0);
    d.x = ra * cos(-rotx);
    d.y = top;
    d.z = ra * sin(-rotx);
    N = normal(a,b,c);
    glNormal3f(N.x,N.y,N.z);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glVertex3f(c.x, c.y, c.z);
    glVertex3f(d.x, d.y, d.z);
    glEnd();
    glPopMatrix();
    
    

    glColor3ub(255,255,255);
    glPushMatrix();
    glBegin(GL_POLYGON);
    a.x = ra * cos(-rotx+PI/2.0);
    a.y = bot;
    a.z = ra * sin(-rotx+PI/2.0);
    b.x = ra * cos(-rotx+PI);
    b.y = bot;
    b.z = ra * sin(-rotx+PI);
    c.x = ra * cos(-rotx+PI);
    c.y = top;
    c.z = ra * sin(-rotx+PI);
    d.x = ra * cos(-rotx+PI/2.0);
    d.y = top;
    d.z = ra * sin(-rotx+PI/2.0);
    N = normal(a,b,c);
    glNormal3f(N.x,N.y,N.z);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glVertex3f(c.x, c.y, c.z);
    glVertex3f(d.x, d.y, d.z);
    glEnd();
    glPopMatrix();
    
    
    glColor3ub(200,255,255);
    glPushMatrix();
    glBegin(GL_POLYGON);
    a.x = ra * cos(-rotx+PI);
    a.y = bot;
    a.z = ra * sin(-rotx+PI);
    b.x = ra * cos(-rotx+PI/2.0*3.0);
    b.y = bot;
    b.z = ra * sin(-rotx+PI/2.0*3.0);
    c.x = ra * cos(-rotx+PI/2.0*3.0);
    c.y = top;
    c.z = ra * sin(-rotx+PI/2.0*3.0);
    d.x = ra * cos(-rotx+PI);
    d.y = top;
    d.z = ra * sin(-rotx+PI);
    N = normal(a,b,c);
    glNormal3f(N.x,N.y,N.z);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glVertex3f(c.x, c.y, c.z);
    glVertex3f(d.x, d.y, d.z);
    glEnd();
    glPopMatrix();
    
    
    glColor3ub(255,200,255);
    glPushMatrix();
    glBegin(GL_POLYGON);
    a.x = ra * cos(-rotx);
    a.y = bot;
    a.z = ra * sin(-rotx);
    b.x = ra * cos(-rotx+PI/2.0*3.0);
    b.y = bot;
    b.z = ra * sin(-rotx+PI/2.0*3.0);
    c.x = ra * cos(-rotx+PI/2.0*3.0);
    c.y = top;
    c.z = ra * sin(-rotx+PI/2.0*3.0);
    d.x = ra * cos(-rotx);
    d.y = top;
    d.z = ra * sin(-rotx);
    N = normal(a,b,c);
    glNormal3f(N.x,N.y,N.z);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glVertex3f(c.x, c.y, c.z);
    glVertex3f(d.x, d.y, d.z);
    glEnd();
    glPopMatrix();
    
  /*
    glPushMatrix();
    glTranslatef(5.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(ra * cos(roty), ra * sin(roty), z);
    glVertex3f(ra * cos(roty+PI/2.0), ra * sin(roty+PI/2.0), z);
    glVertex3f(ra * cos(roty+PI), ra * sin(roty+PI), z);
    glVertex3f(ra * cos(roty+PI/2.0*3.0), ra * sin(roty+PI/2.0*3.0), z);
    glEnd();
    glPopMatrix();
    */
    /*
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    r.bot = WINDOW_HEIGHT / 2;
    r.left = WINDOW_WIDTH / 2;
    r.center = 0;
    //16 12 13 10 08 07 06 8b
    char buff [50];
    sprintf(buff,"%f",rotx);
    ggprint16(&r, 16, 0x00222222, buff);
    sprintf(buff,"%f",roty);
    ggprint16(&r, 16, 0x00222222, buff);
    */
    
    glFlush ();
    
}



