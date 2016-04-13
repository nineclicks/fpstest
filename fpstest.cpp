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
extern "C" {
    #include "fonts.h"
}


#define RAND ((float)rand() / RAND_MAX)
#define PI 3.14159

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int mousex = 0;
int mousey = 0;

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
    mousex += movex / 4;
    mousey += movey / 4;
    if (mousey < 0)
        mousey = 0;
    if (mousex < 0)
        mousex = 0;
    if (mousey > WINDOW_HEIGHT)
        mousey = WINDOW_HEIGHT;
    if (mousex > WINDOW_WIDTH)
        mousex = WINDOW_WIDTH;
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
    //int w = WINDOW_WIDTH;
    int h = WINDOW_HEIGHT;
    Rect r;
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glColor3ub(200,200,200);
    
    glPushMatrix();
    glTranslatef(mousex, WINDOW_HEIGHT - mousey, 0);
    glBegin(GL_QUADS);
    glVertex2i(h * -.1, h * -.1);
    glVertex2i(h * -.1, h * .1);
    glVertex2i(h * .1, h * .1);
    glVertex2i(h * .1, h * -.1);
    glEnd();
    glPopMatrix();
    
    
    r.bot = WINDOW_HEIGHT - 20;
    r.left = 10;
    r.center = 0;
    //16 12 13 10 08 07 06 8b
    ggprint8b(&r, 16, 0x00222222, "Test");
    
    
}



