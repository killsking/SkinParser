// Author: Daren Cheng
// Class:  CS 4392
// Date:   10/18/2015

// Desc: 
// This program renders a posable model given a skel and skin file

//include dependancies
#include <AntTweakBar.h>
#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

//include local header files
#include "Balljoint.h"
#include "Skin.h"
#include "Functions.h"

//function prototypes
void display();
void reshape(int width, int height);
void mouseMotion(int mouseX, int mouseY);

//variables that determine rendering setings
TwBar *bar;                                             //pointer to tweakbar
GLfloat ambientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };    //ambient light color
GLfloat lightPosition0[] = { 2.5f, 2.5f, 2.5f, 1.0f };              //position of sunlight0
GLfloat lightColor0[] = { 0.913725f, 0.513725f, 0.0f, 1.0f };       //color of sunlight0
GLfloat lightPosition1[] = { -2.5f, 2.5f, -2.5f, 1.0f };            //position of sunlight1
GLfloat lightColor1[] = { 0.411764f, 0.745098f, 0.156862f, 1.0f };  //color of sunlight1

//variables defined globally so they aren't redefined each display call
Skin skin;             //skin of the model
Balljoint base;        //base joint to hold the tree of joints
string joint;          //joint currently being manipulated
float pose[3];         //pose of given joint  

//lookat and viewport parameters
int width = 640;
int height = 480;
double eye_x = 0.0;
double eye_y = 0.0;
double eye_z = 0.0;
double center_x = 0.0;
double center_y = 0.0;
double center_z = 0.0;

using namespace std;

int main(int argc, char** argv) {
    //print out skin file name and create file stream
    cout << "Filename: " << argv[1] << endl;
    ifstream skinFile;
    skinFile.open(argv[1]);

    //print out skel file name and create file stream
    cout << "Filename: " << argv[2] << endl;
    ifstream skelFile;
    skelFile.open(argv[2]);

    //set camera position as specified
    eye_x = stod(argv[3]);
    eye_y = stod(argv[4]);
    eye_z = stod(argv[5]);
    center_x = stod(argv[6]);
    center_y = stod(argv[7]);
    center_z = stod(argv[8]);

    //check if skin file opened correctly, then parse file into the skin
    if (skinFile.is_open()) {
        //attempt to get the skin
        skin.getSkin(skinFile);
    }
    skinFile.close();

    //check if skel file opened correctly, then parse file into balljoints
    if (skelFile.is_open()) {
        //check to see if the first token in the skel file is a root joint
        string token;
        skelFile >> token;

        if (token == "balljoint") {
            int base_id = 0;

            //get the entire skeleton by getting the base and its children
            base.getBalljoint(skelFile);
        }
        else {
            cout << "Error, .skel file must start with a root balljoint." << endl;
        }
    }
    skelFile.close();

    //test joint string
    base.getID(joint, 1);
    base.getPose(pose, 1);

    //initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    glutCreateWindow("SkelParser.cpp");
    glutCreateMenu(NULL);

    //get GLUT callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    //initialize AntTweakBar
    TwInit(TW_OPENGL, NULL);

    //control keyboard inputs and redirect mouse events to AntTweakBar
    //glutIgnoreKeyRepeat(true);
    glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);
    glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
    glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
    glutPassiveMotionFunc((GLUTmousemotionfun)mouseMotion);
    TwGLUTModifiersFunc(glutGetModifiers);

    //create a tweak bar
    bar = TwNewBar("Translations");

    //add camera motions
    TwAddVarRW(bar, "Camera_X", TW_TYPE_DOUBLE, &eye_x,
        " min=-5 max=5 step=0.1 keyIncr=d keyDecr=a ");
    TwAddVarRW(bar, "Camera_Y", TW_TYPE_DOUBLE, &eye_y,
        " min=-5 max=5 step=0.1 keyIncr=e keyDecr=q ");
    TwAddVarRW(bar, "Camera_Z", TW_TYPE_DOUBLE, &eye_z,
        " min=-5 max=5 step=0.1 keyIncr=s keyDecr=w ");

    //add pose controllers
    TwAddVarRW(bar, "Joint", TW_TYPE_STDSTRING, &joint, "");
    TwAddVarRW(bar, "Pose_X", TW_TYPE_FLOAT, &pose[0],
        "step=0.1 keyIncr=1 keyDecr=2 ");
    TwAddVarRW(bar, "Pose_Y", TW_TYPE_FLOAT, &pose[1],
        "step=0.1 keyIncr=3 keyDecr=4 ");
    TwAddVarRW(bar, "Pose_Z", TW_TYPE_FLOAT, &pose[2],
        "step=0.1 keyIncr=5 keyDecr=6 ");

    //call the GLUT main loop
    glutMainLoop();

    cin.get();
    return 0;
}

//callback function passed to glutDisplayFunc
void display() {
    //clear frame buffer to avoid rendering issues
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //rendering settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //set camera angle and position, along with viewport
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (double)width / height, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye_x, eye_y, eye_z, center_x, center_y, center_z, 0, 1, 0);

    //draw WCS axis
    //drawWCSAxis();

    //Enable lighting and create a light
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    //Add ambient light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

    //Add two 3/4 spotlights
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition0);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor1);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);

    //update pose of indicated joint
    base.setPose(pose, 1);

    //perform smoothing based on current pose of the joints
    skin.smooth(base.getJointsLcs());

    //draw model by drawing the skin
    skin.draw();
    
    //Draw and refresh AntTweakBar
    TwDraw();
    TwRefreshBar(bar);

    //Swap Buffers for double buffering
    glutSwapBuffers();

    //Indicate need for new display() call
    glutPostRedisplay();

}

//callback function called by GLUT when window size changes
void reshape(int width_, int height_)
{
    width = width_;
    height = height_;
    // Set OpenGL viewport and camera
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (double)width / height, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye_x, eye_y, eye_z, center_x, center_y, center_z, 0, 1, 0);

    //send the new window size to AntTweakBar
    TwWindowSize(width, height);
}

//callback function which handles mouseover events
void mouseMotion(int mouseX, int mouseY)
{
    //test
    //if event was not handled by AntTweakBar, handle it manually
    if (!TwEventMouseMotionGLUT(mouseX, mouseY)) { 
        cout << "x: " << mouseX << ", y: " << mouseY << endl;
    }
}