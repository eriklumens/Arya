#include <iostream>

#include <GL/glew.h>
#include <GL/glfw.h>

#include "Root.h"
#include "Models.h"
#include "Shaders.h"
#include "Fonts.h"
#include "Textures.h"
#include "Scene.h"
#include "Decals.h"
#include "Console.h"
#include "Config.h"
#include "Commands.h"
#include "Files.h"
#include "Overlay.h"
#include "Camera.h"
#include "Sounds.h"
#include "common/Logger.h"
#include "Interface.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace Arya
{
    template<> Root* Singleton<Root>::singleton = 0;

    //glfw callback functions
    void GLFWCALL windowSizeCallback(int width, int height);
    void GLFWCALL keyCallback(int key, int action);
    void GLFWCALL mouseButtonCallback(int button, int action);
    void GLFWCALL mousePosCallback(int x, int y);
    void GLFWCALL mouseWheelCallback(int pos);

    Root::Root()
    {
        scene = 0;
        oldTime = 0;
        interface = 0;
		settingsManager = 0;

        FileSystem::create();
        CommandHandler::create();
        Config::create();
        TextureManager::create();
        MaterialManager::create();
        ModelManager::create();
        FontManager::create();
        SoundManager::create();
        Console::create();
        Decals::create();

        //Some classes should be initialized
        //before the graphics, like Config.
        //Other graphic related classes are
        //initialized in Root::initialize
        //when the graphics are initialized
        if(!Config::shared().init()) LOG_WARNING("Unable to init config");
    }

    Root::~Root()
    {
        //Console deconstructor uses overlay
        //so it must be deleted first
        Console::destroy();

        if(scene) delete scene;
		if(settingsManager) delete settingsManager;
        if(interface) delete interface;

        SoundManager::destroy();
        FontManager::destroy();
        ModelManager::destroy();
        MaterialManager::destroy();
        TextureManager::destroy();
        Config::destroy();
        CommandHandler::destroy();
        FileSystem::destroy();
		Decals::destroy();

        //TODO: Check if GLEW, GLFW, Shaders, Objects were still initated
        //Only clean them up if needed
        glfwTerminate();
    }

    bool Root::init(bool fullscr, int w, int h)
    {
        LOG_INFO("loading root");

        windowWidth = w;
        windowHeight = h;
        fullscreen = fullscr;

        if(!initGLFW()) return false;
        if(!initGLEW()) return false;

        checkForErrors("start of root init");

        // set GL stuff
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Call these in the right order: Models need Textures
        TextureManager::shared().initialize();
		//MaterialManager::shared().initialize();
		ModelManager::shared().initialize();
		if(!SoundManager::shared().init())
		{
			LOG_WARNING("Could not initialize SoundManager, files not found!");
		}

		if(!interface) interface = new Interface;
		if(!interface->init())
		{
			LOG_INFO("Could not initialize interface");
			return false;
		}
		if(!settingsManager)
		{
			settingsManager = new SettingsManager;
		}
		if(!Config::shared().getSettingsManager()->init())
		{
			LOG_INFO("Could not initialize the settings menu");
			return false;
		}
		addFrameListener(interface);

		if(!Console::shared().init()) 
		{
			LOG_INFO("Could not initialize console");
			return false;
		}
		addFrameListener(&Console::shared());
		addInputListener(&Console::shared());

		checkForErrors("end of root init");

		Decals::shared().init();

		LOG_INFO("Root initialized");

		return true;
	}

	Scene* Root::makeDefaultScene()
	{
		if(!scene) delete scene;

		scene = new Scene;
		if( !scene->isInitialized() )
		{
			LOG_ERROR("Unable to initialize scene");
			delete scene;
			scene = 0;
			return 0;
		}
		else
			addFrameListener(scene);

		LOG_INFO("Made scene");

		return scene;
	}

	void Root::removeScene()
	{
		removeFrameListener(scene);
		if(scene) delete scene;
		scene = 0;
	}

	void Root::startRendering()
	{
		LOG_INFO("start rendering");
		running = true;
		while(running)
		{
			if(!oldTime)
				oldTime = glfwGetTime();
			else {
				double pollTime = glfwGetTime();
				double elapsed = pollTime - oldTime;

				//Note: it can happen that the list is modified
				//during a call to onFrame()
				//Some framelisteners (network update) add other framelisteners
				for(std::list<FrameListener*>::iterator it = frameListeners.begin(); it != frameListeners.end();++it)
					(*it)->onFrame((float)elapsed);

				oldTime = pollTime;
			}

			render();

			glfwPollEvents();
			if( glfwGetWindowParam(GLFW_OPENED) == 0 ) running = false;
		}
	}

	void Root::stopRendering()
	{
		running = false;
	}

	void Root::setFullscreen(bool fullscr)
	{
		if( fullscreen == fullscr ) return; //no difference
		fullscreen = fullscr;

		glfwCloseWindow();

		if(!glfwOpenWindow(windowWidth, windowHeight, 0, 0, 0, 0, 32, 0, (fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)))
		{
			LOG_ERROR("Could not re-create window. Closing now.");
			stopRendering();
			return;
		}

		glfwSetWindowTitle("Arya");
		glfwEnable(GLFW_MOUSE_CURSOR);
		glfwSetWindowSizeCallback(windowSizeCallback);
		glfwSetKeyCallback(keyCallback);
		glfwSetMouseButtonCallback(mouseButtonCallback);
		glfwSetMousePosCallback(mousePosCallback);
		glfwSetMouseWheelCallback(mouseWheelCallback);
	}

	bool Root::initGLFW()
	{
		if(!glfwInit())
		{
			LOG_ERROR("Could not init glfw!");
			return false;
		}

		GLFWvidmode mode;
		glfwGetDesktopMode(&mode);
		desktopWidth = mode.Width;
		desktopHeight = mode.Height;
		if(fullscreen)
		{
			windowWidth = desktopWidth;
			windowHeight = desktopHeight;
		}

		glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3); // Use OpenGL Core v3.2
		glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);

		if(!glfwOpenWindow(windowWidth, windowHeight, 0, 0, 0, 0, 32, 0, (fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)))
		{
			LOG_ERROR("Could not open glfw window!");
			return false;
		}

		glfwSetWindowTitle("Arya");
		glfwEnable(GLFW_MOUSE_CURSOR);
		glfwSetWindowSizeCallback(windowSizeCallback);
		glfwSetKeyCallback(keyCallback);
		glfwSetMouseButtonCallback(mouseButtonCallback);
		glfwSetMousePosCallback(mousePosCallback);
		glfwSetMouseWheelCallback(mouseWheelCallback);

		return true;
	}

	bool Root::initGLEW()
	{
		glewExperimental = GL_TRUE; 
		glewInit();

		if (!GLEW_VERSION_3_1)
		{
			LOG_WARNING("No OpenGL 3.1 support! Continuing");
		}

		checkForErrors("glew initalization");

		return true;
	}

	void Root::render()
	{
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, getWindowWidth(), getWindowHeight());
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		checkForErrors("root render start");

		if(scene)
		{
			scene->render();

			checkForErrors("scene render");

			for(std::list<FrameListener*>::iterator it = frameListeners.begin(); it != frameListeners.end();)
			{
				//This construction allows the callback to erase itself from the frameListeners list
				std::list<FrameListener*>::iterator iter = it++;
				(*iter)->onRender();
			}

			checkForErrors("callback onRender");

			GLfloat depth;
			glReadPixels(mouseX, mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

			vec4 screenPos(2.0f * mouseX /((float)windowWidth) - 1.0f, 2.0f * mouseY/((float)windowHeight) - 1.0f, 2.0f*depth-1.0f, 1.0);

			screenPos = scene->getCamera()->getInverseVPMatrix() * screenPos;
			screenPos /= screenPos.w; 

			clickScreenLocation.x = screenPos.x;
			clickScreenLocation.y = screenPos.y;
			clickScreenLocation.z = screenPos.z;
		}

		if(interface)
			interface->render();

		checkForErrors("root render end");

		glfwSwapBuffers();
	}

	Overlay* Root::getOverlay() const
	{
		return interface->getOverlay();
	}

	mat4 Root::getPixelToScreenTransform() const
	{
		return glm::scale(mat4(1.0), vec3(2.0/windowWidth, 2.0/windowHeight, 1.0));
	}

	bool Root::checkForErrors(const char* stateInfo)
	{
		GLenum err = glGetError();
		if(err != GL_NO_ERROR)
		{
			if(!stateInfo || stateInfo[0] == 0)
				AryaLogger << Logger::L_ERROR << "OpenGL error: ";
			else
				AryaLogger << Logger::L_ERROR << "OpenGL error at " << stateInfo << ". Error: ";
			switch(err)
			{
				case GL_INVALID_ENUM:
					AryaLogger << "Invalid enum";
					break;
				case GL_INVALID_VALUE:
					AryaLogger << "Invalid numerical value";
					break;
				case GL_INVALID_OPERATION:
					AryaLogger << "Invalid operation in current state";
					break;
				case GL_STACK_OVERFLOW:
					AryaLogger << "Stack overflow";
					break;
				case GL_STACK_UNDERFLOW:
					AryaLogger << "Stack underflow";
					break;
				case GL_OUT_OF_MEMORY:
					AryaLogger << "Out of memory";
					break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:
					AryaLogger << "Invalid framebuffer operation. Additional information: ";
					{
						GLenum fberr = glCheckFramebufferStatus(GL_FRAMEBUFFER);
						switch(fberr){
							case GL_FRAMEBUFFER_COMPLETE:
								AryaLogger << "framebuffer is complete";
								break;
							case GL_FRAMEBUFFER_UNSUPPORTED:
								AryaLogger << "framebuffer is unsupported";
								break;
							case GL_FRAMEBUFFER_UNDEFINED:
								AryaLogger << "framebuffer undefined";
								break;
							case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
								AryaLogger << "framebuffer has incomplete attachment";
								break;
							case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
								AryaLogger << "framebuffer is missing attachment";
								break;
							default:
								AryaLogger << "unkown fbo error code: " << fberr;
								break;
						}
					}
					break;
				default:
					AryaLogger << "Unkown error. Code: " << err;
					break;
			}
			AryaLogger << endLog;
			return true;
		}
		return false;
	}

	void Root::addInputListener(InputListener* listener)
	{
		inputListeners.push_back(listener);
	}

	void Root::removeInputListener(InputListener* listener)
	{
		for( std::list<InputListener*>::iterator it = inputListeners.begin(); it != inputListeners.end(); ){
			if( *it == listener ) it = inputListeners.erase(it);
			else ++it;
		}
	}

	void Root::addFrameListener(FrameListener* listener)
	{
		frameListeners.push_back(listener);
	}

	void Root::removeFrameListener(FrameListener* listener)
	{
		for( std::list<FrameListener*>::iterator it = frameListeners.begin(); it != frameListeners.end(); ){
			if( *it == listener ) it = frameListeners.erase(it);
			else ++it;
		}
	}

	void Root::windowSizeChanged(int width, int height)
	{
		windowWidth = width;
		windowHeight = height;
		if(scene)
		{
			Camera* cam = scene->getCamera();
			if(cam)
				cam->setProjectionMatrix(45.0f, getAspectRatio(), 0.1f, 2000.0f);
		}
        if(interface) interface->recalculatePositions();
	}

	void Root::keyDown(int key, int action)
	{
		for( std::list<InputListener*>::iterator it = inputListeners.begin(); it != inputListeners.end(); ++it )
			if( (*it)->keyDown(key, action == GLFW_PRESS) == true ) break;

	}

	void Root::mouseDown(int button, int action)
	{
		for( std::list<InputListener*>::iterator it = inputListeners.begin(); it != inputListeners.end(); ++it )
			if( (*it)->mouseDown((MOUSEBUTTON)button, action == GLFW_PRESS, mouseX, mouseY) == true ) break;
	}

	void Root::mouseWheelMoved(int pos)
	{
		int delta = pos - mouseWheelPos;
		mouseWheelPos = pos;
		for( std::list<InputListener*>::iterator it = inputListeners.begin(); it != inputListeners.end(); ++it )
			if( (*it)->mouseWheelMoved(delta) == true ) break;
	}

	void Root::mouseMoved(int x, int y)
	{
		y = windowHeight - y;
		int dx = x - mouseX, dy = y - mouseY;
		mouseX = x; mouseY = y;

		for( std::list<InputListener*>::iterator it = inputListeners.begin(); it != inputListeners.end(); ++it )
			if( (*it)->mouseMoved(x, y, dx, dy) == true ) break;
	}

	void GLFWCALL windowSizeCallback(int width, int height)
	{
		Root::shared().windowSizeChanged(width, height);
	}

	void GLFWCALL keyCallback(int key, int action)
	{
		Root::shared().keyDown(key, action);
	}

	void GLFWCALL mouseButtonCallback(int button, int action)
	{
		Root::shared().mouseDown(button, action);
	}

	void GLFWCALL mousePosCallback(int x, int y)
	{
		Root::shared().mouseMoved(x, y);
	}

	void GLFWCALL mouseWheelCallback(int pos)
	{
		Root::shared().mouseWheelMoved(pos);
	}
}
