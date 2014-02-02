#if defined ( WIN32 ) || defined( _WINDOWS )
#include "GLFW/glfw.h"
#else
#include <GL/glfw.h>
#include <stdlib.h>
#endif

#include "DemoApp.h"
#include <iostream>
#include <cstring>

//FOR PARSING CONFIG XML
#include <XMLParser/utXMLParser.h>

// Configuration
std::string caption = "GameEngine Demo Application";
int width = 1024;
int height = 768;
bool fullScreen = false;
std::string sceneFile;


bool setupWindow( int, int, bool );
static bool running;
static double t0;
static DemoApp *app;

#ifdef MEMORYCHECK
#include <vld.h>
#endif

void parseConfigXml()
{
	XMLResults xmlResults;
	XMLNode xmlFile = XMLNode::parseFile( "config.xml", "config", &xmlResults );
	if( ( xmlResults.error == eXMLErrorNone ) && !xmlFile.isEmpty() )
	{
		XMLNode config = xmlFile.getChildNodeWithAttribute("application", "name", "DominoApp");
		XMLNode graphicsconfig = config.getChildNode("graphics");
		if( !graphicsconfig.isEmpty() )
		{
			caption = config.getAttribute("name", "Horde3D GameEngine App");

			const char* widthC = graphicsconfig.getChildNodeWithAttribute( "cfg", "name", "resolutionX").getText();
			if (widthC)
				width = atoi(widthC);
			const char* heightC = graphicsconfig.getChildNodeWithAttribute( "cfg", "name", "resolutionY").getText();
			if (heightC)
				height = atoi(heightC);
			const char* fullC = graphicsconfig.getChildNodeWithAttribute( "cfg", "name", "fullscreen").getText();
			if (fullC)
				fullScreen = strcmp(fullC, "0") && strcmp(fullC, "false");
		}

		XMLNode path = config.getChildNode("path");
		const char* sceneC = path.getChildNodeWithAttribute("cfg", "name", "sceneFile").getText();
		if (sceneC != 0)
			sceneFile = sceneC;
	}
	else
	{
	    std::cerr << "Found no config file for the application settings, trying to use the command line argument." << std::endl;
	}
}

int windowCloseListener()
{
	running = false;
	return 0;
}


void keyPressListener( int key, int action )
{
	if( !running ) return;

	if( action == GLFW_PRESS )
	{
		switch (key)
		{
		case GLFW_KEY_ESC:
			running = false;
			break;

		case GLFW_KEY_F1:
			app->release();
			glfwCloseWindow();

			// Toggle fullscreen mode
			fullScreen = !fullScreen;

			if( fullScreen )
			{
				GLFWvidmode mode;
				glfwGetDesktopMode( &mode );
				// Use desktop resolution
				width = mode.Width; height = mode.Height;
			}

			if( !setupWindow( width, height, fullScreen ) )
			{
				glfwTerminate();
				exit( -1 );
			}

			app->init(sceneFile.c_str());
			app->resize( width, height );
			t0 = glfwGetTime();
			break;
		default:
			app->keyPress( key, true );
		}
	}

	if( key >= 0 ) app->keyPress( key, action == GLFW_PRESS );
}


void mouseMoveListener( int x, int y )
{
	if( !running )
	{
		return;
	}

	app->mouseMoved( (float)x, (float)y );
}

void  mouseButtonListener(int button, int action)
{
	if( !running )	return;

	app->mouseClick(button, action);
}


bool setupWindow( int width, int height, bool fullscreen )
{
	// Create OpenGL window
	if( !glfwOpenWindow( width, height, 8, 8, 8, 8, 24, 8, fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW ) )
	{
		glfwTerminate();
		return false;
	}

	if( !fullscreen ) glfwSetWindowTitle( caption.c_str() );

	// Disable vertical synchronization
	glfwSwapInterval( 0 );

	// Set listeners
	glfwSetWindowCloseCallback( windowCloseListener );
	glfwSetKeyCallback( keyPressListener );
	glfwSetMousePosCallback( mouseMoveListener );
	glfwSetMouseButtonCallback( mouseButtonListener );

	return true;
}


int main(int argc, char** argv)
{
	parseConfigXml();

	if (sceneFile.empty() && argc >= 2)
	{
		sceneFile = argv[1];
	}

	if (sceneFile.empty())
	{
		std::cerr << "No Scenefile given. Please configure config.xml or pass an argument!" << std::endl;
		glfwOpenWindow( 640, 16, 8, 8, 8, 8, 24, 8, GLFW_WINDOW );
		glfwSetWindowTitle( "No Scenefile given. Please configure config.xml or pass an argument!" );
		double startTime = glfwGetTime();
		while( glfwGetTime() - startTime < 5.0 ) {}
		glfwTerminate();
		return -1;
	}

	// Initialize GLFW
	glfwInit();
	if( !setupWindow( width, height, fullScreen ) )
		return -1;

	GLFWvidmode desktopMode;
	glfwGetDesktopMode( &desktopMode );

	// Initalize application and engine
	app = new DemoApp();
	if ( !app->init(sceneFile.c_str()) )
	{
		std::cout << "Unable to initalize engine" << std::endl;
		std::cout << "Make sure you have an OpenGL 2.0 compatible graphics card";

		// Fake message box
		glfwCloseWindow();
		glfwOpenWindow( 800, 16, 8, 8, 8, 8, 24, 8, GLFW_WINDOW );
		glfwSetWindowTitle( "Unable to initalize engine - Make sure you have an OpenGL 2.0 compatible "
			"graphics card and have specified a valid GameEngine scene" );
		double startTime = glfwGetTime();
		while( glfwGetTime() - startTime < 5.0 ) {}  // Sleep

		glfwTerminate();
		return -1;
	}
	app->resize( width, height );

	//glfwDisable( GLFW_MOUSE_CURSOR );

	int frames = 0;
	float fps = 30.0f;
	t0 = glfwGetTime();
	running = true;

	// Game loop
	while( running )
	{
		// Calc FPS
		++frames;
		if( frames >= 3 )
		{
			double t = glfwGetTime();
			fps = frames / (float)(t - t0);
			frames = 0;
			t0 = t;
		}

		// Render
		app->render();
		glfwSwapBuffers();
	}

	glfwEnable( GLFW_MOUSE_CURSOR );

	// Quit
	app->release();
	delete app;
	glfwTerminate();

	return 0;
}
