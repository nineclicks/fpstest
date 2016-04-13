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


void render(Game *game)
{
    float w = WINDOW_WIDTH;
    float h = WINDOW_HEIGHT;
    float ra = h / 5.0;
    Rect r;
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glColor3ub(200,200,200);
    
    glPushMatrix();
    glTranslatef(w / 3.0, h / 2.0, 0);
    glBegin(GL_POLYGON);
    glVertex2f(ra * cos(-rotx), ra * sin(-rotx));
    glVertex2f(ra * cos(-rotx+PI/2.0), ra * sin(-rotx+PI/2.0));
    glVertex2f(ra * cos(-rotx+PI), ra * sin(-rotx+PI));
    glVertex2f(ra * cos(-rotx+PI/2.0*3.0), ra * sin(-rotx+PI/2.0*3.0));
    glEnd();
    glPopMatrix();
  
    glPushMatrix();
    glTranslatef(w / 3.0 * 2.0, h / 2.0, 0);
    glBegin(GL_POLYGON);
    glVertex2f(ra * cos(roty), ra * sin(roty));
    glVertex2f(ra * cos(roty+PI/2.0), ra * sin(roty+PI/2.0));
    glVertex2f(ra * cos(roty+PI), ra * sin(roty+PI));
    glVertex2f(ra * cos(roty+PI/2.0*3.0), ra * sin(roty+PI/2.0*3.0));
    glEnd();
    glPopMatrix();
    
    r.bot = WINDOW_HEIGHT / 2;
    r.left = WINDOW_WIDTH / 2;
    r.center = 0;
    //16 12 13 10 08 07 06 8b
    char buff [50];
    sprintf(buff,"%f",rotx);
    ggprint16(&r, 16, 0x00222222, buff);
    sprintf(buff,"%f",roty);
    ggprint16(&r, 16, 0x00222222, buff);
    
    
}



