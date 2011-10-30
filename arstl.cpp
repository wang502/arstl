/*
 * Based on the aruco_test_board_gl example available under a BSD license from
 * the ArUco library by Rafael Munoz-Salinas: rmsalinas@uco.es
 */

#include <iostream>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include <GL/gl.h>
#include <GL/glut.h>
#include <aruco/aruco.h>
#include <aruco/boarddetector.h>
#include "common.h"
#include "stlparser.h"
using namespace cv;
using namespace aruco;

string TheInputVideo,TheIntrinsicFile,TheBoardConfigFile;
bool isIntrinsicFileYAML=false;
bool The3DInfoAvailable=false;
float TheMarkerSize=-1;
MarkerDetector MDetector;
BoardDetector BDetector;
VideoCapture TheVideoCapturer;
vector<Marker> TheMarkers;
//board
BoardDetector TheBoardDetector;
pair<Board,float> TheBoardDetected; //the board and its probabilit
BoardConfiguration TheBoardConfig;
Mat TheInputImage,TheUndInputImage,TheResizedImage;
CameraParameters TheCameraParams;
Size TheGlWindowSize;
GLuint STLDisplayList;
bool TheCaptureFlag=true;
void readArguments ( int argc,char **argv );
void usage();
void vDrawScene();
void vIdle();
void vResize( GLsizei iWidth, GLsizei iHeight );
void vMouse(int b,int s,int x,int y);
static GLuint loadSTL(const char *filename);
/************************************
 *
 *
 *
 *
 ************************************/

int main(int argc,char **argv)
{
	try
	{
		if(argc==1) usage();
		readArguments (argc,argv);
		if (TheIntrinsicFile==""){cerr<<"-f or -y option required"<<endl;return -1;}
		if (TheMarkerSize==-1){cerr<<"-s option required"<<endl;return -1;}
		if(TheBoardConfigFile==""){
		  cerr<<"The board configuration info must be provided (-b option)"<<endl;
		  return -1;
		}
		//read board configuration
		TheBoardConfig.readFromFile(TheBoardConfigFile);

		//Open video input source
		if (TheInputVideo=="")  //read from camera
		  TheVideoCapturer.open(0);
		else TheVideoCapturer.open(TheInputVideo);
		if (!TheVideoCapturer.isOpened())
		{
			cerr<<"Could not open video"<<endl;
			return -1;

		}

		//read first image
		TheVideoCapturer>>TheInputImage;
		//read camera paramters if passed
		if (isIntrinsicFileYAML)
		  TheCameraParams.readFromXMLFile(TheIntrinsicFile);
		else 
		  TheCameraParams.readFromFile(TheIntrinsicFile);
		TheCameraParams.resize( TheInputImage.size());

		glutInit(&argc, argv);
		glutInitWindowPosition( 0, 0);
		glutInitWindowSize(TheInputImage.size().width,TheInputImage.size().height);
		glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE );
		glutCreateWindow( "arstl" );
		glutDisplayFunc( vDrawScene );
		glutIdleFunc( vIdle );
		glutReshapeFunc( vResize );
		glutMouseFunc(vMouse);
		glClearColor( 0.0, 0.0, 0.0, 1.0 );
		glClearDepth( 1.0 );
		TheGlWindowSize=TheInputImage.size();
		STLDisplayList = loadSTL("will.stl");
		vResize(TheGlWindowSize.width,TheGlWindowSize.height);
		glutMainLoop();

	}catch(std::exception &ex)

	{
		cout<<"Exception :"<<ex.what()<<endl;
	}

}

static void drawSTL(float *faces, int numFaces)
{
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < numFaces; i++) {
        // Switching y and z since OpenGL uses a Y-up coordinate system
        glVertex3f(faces[i*12+3]/1000.0,faces[i*12+5]/1000.0,-faces[i*12+4]/1000.0);
        glVertex3f(faces[i*12+6]/1000.0,faces[i*12+8]/1000.0,-faces[i*12+7]/1000.0);
        glVertex3f(faces[i*12+9]/1000.0,faces[i*12+11]/1000.0,-faces[i*12+10]/1000.0);
    }
    glEnd();
}

static GLuint loadSTL(const char *filename)
{
    GLuint displayList = glGenLists(1);
    int numFaces = 0;
    float *faces = load_stl(filename, &numFaces);
    if (faces) {
        glNewList(displayList,GL_COMPILE);
        drawSTL(faces, numFaces);
        glEndList();
        free(faces);
    }
    return displayList;   
}


/************************************
 *
 *
 *
 *
 ************************************/

void vMouse(int b,int s,int x,int y)
{
    if (b==GLUT_LEFT_BUTTON && s==GLUT_DOWN) {
      TheCaptureFlag=!TheCaptureFlag;
    }

}

/************************************
 *
 *
 *
 *
 ************************************/
void axis(float size)
{
    glColor3f (1,0,0 );
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f(size,0.0f, 0.0f); // ending point of the line
    glEnd( );

    glColor3f ( 0,1,0 );
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f( 0.0f,size, 0.0f); // ending point of the line
    glEnd( );


    glColor3f (0,0,1 );
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f(0.0f, 0.0f, size); // ending point of the line
    glEnd( );


}
/************************************
 *
 *
 *
 *
 ************************************/
void vDrawScene()
{
	if (TheResizedImage.rows==0) //prevent from going on until the image is initialized
	  return;
	///clear
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	///draw image in the buffer
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, TheGlWindowSize.width, 0, TheGlWindowSize.height, -1.0, 1.0);
	glViewport(0, 0, TheGlWindowSize.width , TheGlWindowSize.height);
	glDisable(GL_TEXTURE_2D);
	glPixelZoom( 1, -1);
	glRasterPos3f( 0, TheGlWindowSize.height  - 0.5, -1.0 );
	glDrawPixels ( TheGlWindowSize.width , TheGlWindowSize.height , GL_RGB , GL_UNSIGNED_BYTE , TheResizedImage.ptr(0) );
	///Set the appropriate projection matrix so that rendering is done in a enrvironment
	//like the real camera (without distorsion)
	glMatrixMode(GL_PROJECTION);
	double proj_matrix[16];
	MarkerDetector::glGetProjectionMatrix(TheCameraParams,TheInputImage.size(),TheGlWindowSize,proj_matrix,0.05,10);
	glLoadIdentity();
	glLoadMatrixd(proj_matrix);
	glLineWidth(2);
	//now, for each marker,
	double modelview_matrix[16];

	for(unsigned int m=0;m<TheMarkers.size();m++)
	{
		TheMarkers[m].glGetModelViewMatrix(modelview_matrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glLoadMatrixd(modelview_matrix);


// 		axis(TheMarkerSize);

		glColor3f(1,0.4,0.4);
		glTranslatef(0, TheMarkerSize/2,0);
		glPushMatrix();
 		glutWireCube( TheMarkerSize );

		glPopMatrix();
	}
	//If the board is detected with enough probability
	if (TheBoardDetected.second>0.3){
	  TheBoardDetected.first.glGetModelViewMatrix(modelview_matrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glLoadMatrixd(modelview_matrix);
		glColor3f(0,1,0);
		glTranslatef(0, TheMarkerSize/2,0);
		glPushMatrix();
 		glutWireCube( TheMarkerSize );
 		glCallList(STLDisplayList);
		axis(TheMarkerSize);
		glPopMatrix();
	}

	glutSwapBuffers();

}


/************************************
 *
 *
 *
 *
 ************************************/
void vIdle()
{
  if(TheCaptureFlag){
	//capture image
	TheVideoCapturer.grab();
	TheVideoCapturer.retrieve( TheInputImage);
	TheUndInputImage.create(TheInputImage.size(),CV_8UC3);
	//by deafult, opencv works in BGR, so we must convert to RGB because OpenGL in windows preffer
	cv::cvtColor(TheInputImage,TheInputImage,CV_BGR2RGB);
	//remove distorion in image
	cv::undistort(TheInputImage,TheUndInputImage, TheCameraParams.CameraMatrix,TheCameraParams.Distorsion);
	//detect markers
	MDetector.detect(TheUndInputImage,TheMarkers,TheCameraParams.CameraMatrix,Mat(),TheMarkerSize);
	//Detection of the board
	TheBoardDetected.second=TheBoardDetector.detect( TheMarkers, TheBoardConfig,TheBoardDetected.first, TheCameraParams,TheMarkerSize);
	//chekc the speed by calculating the mean speed of all iterations
	//resize the image to the size of the GL window
	cv::resize(TheUndInputImage,TheResizedImage,TheGlWindowSize);
  }
  glutPostRedisplay();
}


/************************************
 *
 *
 *
 *
 ************************************/
void vResize( GLsizei iWidth, GLsizei iHeight )
{
	TheGlWindowSize=Size(iWidth,iHeight);
	//not all sizes are allowed. OpenCv images have padding at the end of each line in these that are not aligned to 4 bytes
	if (iWidth*3%4!=0){
	  iWidth+=iWidth*3%4;//resize to avoid padding
	  vResize(iWidth,TheGlWindowSize.height);
	}
	else{
	  //resize the image to the size of the GL window
	  if (TheUndInputImage.rows!=0)
	    cv::resize(TheUndInputImage,TheResizedImage,TheGlWindowSize);
	}
}


/************************************
 *
 *
 *
 *
 ************************************/
void usage()
{
	cout<<"This program test the ArUco Library \n\n";
	cout<<"-i <video.avi>: specifies a input video file. If not, images from camera are captures"<<endl;
	cout<<"-f <file.int>: if you have calibrated your camera, pass calibration information here so as to be able to get 3D marker info"<<endl;
	cout<<"-y <file.yml>: if you have calibrated your camera in yml format as provided by the calibration.cpp aplication in OpenCv >= 2.2"<<endl;
	cout<<"-b <boardConfiguration.abc>: file with the board configuration"<<endl;
	cout<<"-s <size>: size of the marker's sides (expressed in meters!)"<<endl;
}


/************************************
 *
 *
 *
 *
 ************************************/
static const char short_options [] = "hi:f:s:b:y:";

static const struct option
long_options [] =
{
	{ "help",           no_argument,   NULL,                 'h' },
	{ "input",     required_argument,   NULL,           'i' },
	{ "intFile",     required_argument,   NULL,           'f' },
	{ "YAMLFile",     required_argument,   NULL,           'y' },
	{ "size",     required_argument,   NULL,           's' },
	{ "boardFile",     required_argument,   NULL,           'b' },

	{ 0, 0, 0, 0 }
};

/************************************
 *
 *
 *
 *
 ************************************/
void readArguments ( int argc,char **argv )
{
	for ( ;; )
	{
		int index;
		int c;
		c = getopt_long ( argc, argv,
			short_options, long_options,
			&index );

		if ( -1 == c )
			break;
		switch ( c )
		{
			case 0:
				break;
			case 'h':
				usage ();
				exit ( EXIT_SUCCESS );
				break;
			case 'i':
				TheInputVideo=optarg;
				break;
			case 'f':
				TheIntrinsicFile=optarg;
				isIntrinsicFileYAML=false;
				break;
			case 'y':
				TheIntrinsicFile=optarg;
				isIntrinsicFileYAML=true;
				break;
			case 's':
				TheMarkerSize=atof(optarg);
				break;
			case 'b':
				TheBoardConfigFile=optarg;
				break;
			default:
				usage ();
				exit ( EXIT_FAILURE );
		};
	}

}

