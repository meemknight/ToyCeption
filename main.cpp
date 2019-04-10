/////////////////////////////////////////////
//main.cpp
//Copyright(c) 2019 Luta Vlad
//https://github.com/meemknight/OpenGLEngine
/////////////////////////////////////////////
#include <iostream>
#include <vector>
#include <Windows.h>
#include <cmath>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
//#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletDynamics/Character/btCharacterControllerInterface.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Camera.h"
#include "GameObject.h"
#include "Light.h"
#include "customBulletdebuggClass.h"
#include "shapesGenerator.h"
#include "GameObjectPool.h"
#include "CharacterMouseController.h"
#include "MenuApi.h"

#include "tools.h"
#include "declarations.h"
#include "escapi.h"
#include "Drawer2D.h"

extern "C"
{
	//Enable dedicated graphics
	//__declspec(dllexport) DWORD NvOptimusEnablement = true;
	//__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = true;
}

float width = 1280;
float height = 720;

using glm::mat4;

States gameState = States::mainMenu;
GameObjectPool gameObjectPool;
PhisicalObject *playerPointer = nullptr;
sf::RenderWindow *windowPointer = nullptr;

extern ma::Menu mainMenu;
extern ma::Menu gameMenu;
extern bool levelShouldLoad;
extern bool debugDraw;
extern bool showFramerate;


glm::vec3 exitPosition = { 0,0,0 };
glm::vec3 pickupPosition = { 0,0,0 };
bool pickupped = false;
sf::Music mainMusic;

Drawer2D drawer2D;
SimpleCapParams capture;
int nCameras = 0;

void setupCamera()
{
	gameState = States::extras;

	if(drawer2D.initialised)
	{
		drawer2D.Cleanup();
	}

	drawer2D.initialize(width, height);
	
	nCameras = setupESCAPI();
	if(nCameras == 0)
	{
		elog("no camera detedted");
	}

	capture.mHeight = drawer2D.height;
	capture.mWidth = drawer2D.width;
	capture.mTargetBuf = new int[drawer2D.height*drawer2D.width];

	if (initCapture(0, &capture) == 0)
	{
		elog("couldn't load camera drivers");
	}

}

void closeCamera()
{
	drawer2D.Cleanup();
	delete[] capture.mTargetBuf;
	deinitCapture(0);
}


int MAIN
{
#pragma region phisicalWorld
	btDynamicsWorld *world;
	btDispatcher *dispatcher;
	btBroadphaseInterface *broadPhase;
	btConstraintSolver *solver;
	btCollisionConfiguration *collisionConfiguration;

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	broadPhase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();

	world = new btDiscreteDynamicsWorld(dispatcher, broadPhase, solver, collisionConfiguration);
	world->setGravity({ 0, -9.81f, 0 });

#pragma endregion


	sf::ContextSettings contextSettings;
	contextSettings.depthBits = 24;
	contextSettings.stencilBits = 8;
	contextSettings.antialiasingLevel = 2;

	sf::RenderWindow window(sf::VideoMode(width, height), ((char*)glGetString(GL_RENDERER)), sf::Style::Default, contextSettings);
	windowPointer = &window;
	
	initializeMenu(&window);

	auto windoHandle = window.getSystemHandle();

	window.setVerticalSyncEnabled(true);
	
	if(!mainMusic.openFromFile("musiq.ogg"))
	{
		elog("Error loading the music file");
	}else
	{
		mainMusic.play();
		mainMusic.setLoop(true);
	}
	
	glewInit();
	glewExperimental = GL_TRUE;
	glEnable(GL_DEPTH_TEST);
	//glBlendFunc(GL_DST_COLOR, GL_ZERO);

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
#endif

	glEnable(GL_CULL_FACE);


	AssetManager<Texture> textureManager;
	AssetManager<LoadedIndexModel> modelManager;
	LightContext light;

	Camera camera(85.f, &width, &height, 0.1f, 1000.f);
	camera.mSpeed = 16.0f;
	
	camera.position = { 0, 3, -4 };
	//camera.viewDirection = { 0, 0, 1 };
	///
	camera.firstPersonCamera = true;
	camera.distanceFromPlayer = 8;
	camera.cameraAngle = glm::radians(25.f);
	camera.topDownAngle = 3.141f;

	float playerAngle = 0;
	float playerRotationSpeed = glm::radians(120.f);
	float mouseScroll = 0;

	CharacterMouseController characterController;
	characterController.topDownRotation = &playerAngle;
	characterController.cameraAngle = &camera.cameraAngle;
	characterController.downMax = glm::radians(0.f);
	characterController.mouseScroll = &camera.distanceFromPlayer;

	//ShaderProgram program(VertexShader("vertex.vert"), FragmentShader("fragment.frag"));
	//ShaderProgram normalProgram(VertexShader("vertn.vert"), FragmentShader("fragn.frag"));
	ShaderProgram textureProgram(VertexShader("vertt.vert"), FragmentShader("fragt.frag"));
	ShaderProgram textureProgramEffect(VertexShader("vertt.vert"), FragmentShader("fragtEffect.frag"));
	ShaderProgram debugShader(VertexShader("debugShader.vert"), FragmentShader("debugShader.frag"));
	ShaderProgram cameraShader(VertexShader("camera.vert"), FragmentShader("camera.frag"));

	customBulletdebuggClass debugDrawer(&debugShader, &camera);

	world->setDebugDrawer(&debugDrawer);
	world->getDebugDrawer()->setDebugMode(btIDebugDraw::DebugDrawModes::DBG_DrawWireframe);

	gameObjectPool.initialize(&textureProgram, &camera, &light, world, &textureManager, &modelManager);
	

	ilog(glGetString(GL_VERSION));


	glClearColor(0.08, 0.08, 0.1, 1.0);

	sf::Clock c;
	sf::Clock fpsClock;
	bool updatemouse = 0;
	int frames = 0;

	PhisicalObject playerObject(&camera, &textureProgram, &light, world, nullptr/*new btSphereShape(1)*/, 10);
	playerObject.loadCollisionBox(modelManager.getData("objects//sphere.obj"), nullptr);
	playerObject.loadPtn323(modelManager.getData("objects//sphere.obj"), textureManager);
	//playerObject.pushElement({ -37, 3 ,45 });
	playerObject.pushElement({ 40, 3 , -40 });
	playerObject.objectData[0].material = Material::greyMaterial(0.6, 0.7, 1, 40);
	playerObject.objectData[0].material.ka *= 1.1;
	playerPointer = &playerObject;

	//playerObject.getIndtance(0)->setFriction(0.5f);
	
	bool canJump = true;
	bool canJump2 = true;
	float jumpingCharge = 0;

	
	while (window.isOpen())
	{

		float deltatime = c.restart().asSeconds();
		if(deltatime > (1.f/4.f))
		{
			deltatime = 1.f / 4.f;
		}

		frames++;
		if (fpsClock.getElapsedTime().asSeconds() >= 1)
		{
			float time = fpsClock.restart().asSeconds();
			sf::String t;
			time = frames / time;
			char c[12];
			sprintf(c, "%f", time);
			if(showFramerate)
			{
				window.setTitle(c);
			}else
			{
				window.setTitle("ToyCeption");
			}
			frames = 0;
		}

		glViewport(0, 0, width, height);

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		mouseScroll *= 0.4f;

		bool escapeReleased = false;
		bool mouseButtonReleased = false;
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
				//std::cin.get();
				exit(0);
			}
			else
			if (event.type == sf::Event::Resized)
			{
				width = window.getSize().x;
				height = window.getSize().y;

				if(gameState == States::extras)
				{
					setupCamera();
				}

			}
			else
			if (event.type == sf::Event::MouseLeft)
			{
				updatemouse = 0;
			}
			else
			if (event.type == sf::Event::MouseEntered)
			{
				//updatemouse = 1;
				//camera.oldMousePosition = { sf::Mouse::getPosition(window).x, (float)sf::Mouse::getPosition(window).y };
			}
			else
			if (event.type == sf::Event::MouseWheelScrolled)
			{
				mouseScroll = event.mouseWheelScroll.delta;
			}else
			if(event.type == sf::Event::GainedFocus)
			{
				updatemouse = 1;
				camera.oldMousePosition = { sf::Mouse::getPosition(window).x, (float)sf::Mouse::getPosition(window).y };
			}else
			if(event.type == sf::Event::KeyReleased)
			{
				if(event.key.code == sf::Keyboard::Escape)
				{
					escapeReleased = true;
				}
			}else
			if(event.type == sf::Event::MouseButtonReleased)
			{
				if(event.mouseButton.button == sf::Mouse::Button::Left)
				{
					mouseButtonReleased = true;
				}
			}

		}

		switch (gameState)
		{
		case States::none:
			break;
		case States::mainMenu:
		{

			break; 
		}
		case States::inGameMenu: 
		{
		
			break;
		}
		case States::inGame:
		{

			if (escapeReleased == true)
			{
				gameState = States::inGameMenu;
				escapeReleased = false;

			}

#pragma region keys
			if (sf::Keyboard::isKeyPressed((sf::Keyboard::Escape)))
			{

				//exit(0);
				//ShowWindow((HWND)(windoHandle), SW_SHOWNOACTIVATE);
				//SendMessage((HWND)windoHandle, WM_KILLFOCUS, 0, 0);
				//updatemouse = 0;
			}

			float maxSpeed = 35000 * deltatime;
			float jumpImpulse = 7.5;

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
				camera.moveFront(deltatime);

				playerObject.getInstance(0)->applyCentralForce({ 0,0,-maxSpeed * cos(playerAngle) });
				playerObject.getInstance(0)->applyCentralForce({ -maxSpeed * sin(playerAngle), 0, 0 });
				playerObject.getInstance(0)->activate(1);
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				camera.moveBack(deltatime);

				playerObject.getInstance(0)->applyCentralForce({ 0,0,maxSpeed * cos(playerAngle) });
				playerObject.getInstance(0)->applyCentralForce({ maxSpeed * sin(playerAngle), 0, 0 });
				playerObject.getInstance(0)->activate(1);
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				camera.moveLeft(deltatime);

				playerObject.getInstance(0)->applyCentralForce({ -maxSpeed * cos(playerAngle),0,0 });
				playerObject.getInstance(0)->applyCentralForce({ 0,0, maxSpeed * sin(playerAngle) });
				playerObject.getInstance(0)->activate(1);
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				camera.moveRight(deltatime);

				playerObject.getInstance(0)->applyCentralForce({ maxSpeed * cos(playerAngle),0,0 });
				playerObject.getInstance(0)->applyCentralForce({ 0,0,maxSpeed * -sin(playerAngle) });
				playerObject.getInstance(0)->activate(1);
			}


			if (updatemouse)
			{
				if (!camera.firstPersonCamera)
				{
					camera.mouseUpdate({ (float)sf::Mouse::getPosition(window).x, (float)sf::Mouse::getPosition(window).y }, window);
				}
				else
				{
					characterController.update({ (float)sf::Mouse::getPosition(window).x, (float)sf::Mouse::getPosition(window).y }, true, mouseScroll, window);
				}

			}


			if (window.hasFocus())
			{
				window.setMouseCursorVisible(0);
				updatemouse = 1;
			}
			else
			{
				window.setMouseCursorVisible(1);
				updatemouse = 0;
			}

			jumpingCharge += deltatime;

			if (canJump == false)
			{
				if (jumpingCharge >= 0.2)
				{
					canJump = true;
					jumpingCharge = 0;
				}
			}
			btVector3 playerPos;
			btTransform playerTransform;
			playerObject.getInstance(0)->getMotionState()->getWorldTransform(playerTransform);
			playerPos = { playerTransform.getOrigin().x(), playerTransform.getOrigin().y(), playerTransform.getOrigin().z() };

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				canJump2 = false;
				int numManifolds = world->getDispatcher()->getNumManifolds();
				for (int i = 0; i < numManifolds; i++)
				{
					btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
					const btCollisionObject* obA = contactManifold->getBody0();
					const btCollisionObject* obB = contactManifold->getBody1();
					if (obA == playerObject.getInstance(0) || obB == playerObject.getInstance(0))
					{
						int numContacts = contactManifold->getNumContacts();
						for (int j = 0; j < numContacts; j++)
						{
							btManifoldPoint& pt = contactManifold->getContactPoint(j);
							if (pt.getDistance() < 0.014f)
							{
								btVector3 normal;
								if (obB == playerObject.getInstance(0)) //Check each object to see if it's the rigid body and determine the correct normal.
								{
									normal = -pt.m_normalWorldOnB;
								}
								else
								{
									normal = pt.m_normalWorldOnB;
								}

								if (normal.y() > 0.25f /*put the threshold here */)
								{
									//The character controller is on the ground
									canJump2 = true;
									goto endCanJump;
								}
							}
						}
					}
				}
			endCanJump:
				if (canJump2 && canJump)
				{
					playerObject.getInstance(0)->setLinearVelocity({ 0,jumpImpulse,0 });
					playerObject.getInstance(0)->activate(true);
					canJump = false;
					jumpingCharge = 0;
				}
			}

			auto v = playerObject.getInstance(0)->getLinearVelocity();
			const int maxVelocity = 6;
			if (v.getZ() > maxVelocity) { v.setZ(maxVelocity); }
			if (v.getZ() < -maxVelocity) { v.setZ(-maxVelocity); }
			if (v.getX() > maxVelocity) { v.setX(maxVelocity); }
			if (v.getX() < -maxVelocity) { v.setX(-maxVelocity); }
			playerObject.getInstance(0)->setLinearVelocity(v);
#pragma endregion
			//hover exit
			{
				float c = cos(GetTickCount() * 0.002) * 1.5f;
				int pos = gameObjectPool.gameObjectVector.getPositionById(420);
				if (pos != -1)
				{
					gameObjectPool.gameObjectVector.elements[pos].getInstance(0).setPosition(exitPosition.x, exitPosition.y + c, exitPosition.z);
				}
				pos = gameObjectPool.gameObjectVector.getPositionById(421);
				if (pos != -1)
				{
					gameObjectPool.gameObjectVector.elements[pos].getInstance(0).setPosition(pickupPosition.x, pickupPosition.y + c, pickupPosition.z);
				}
			}

			world->stepSimulation(deltatime);

			///remove objects
			int objects = gameObjectPool.phisicalObjectVector.elements.size();
			for (int y = 0; y < objects; y++)
			{
				int size = gameObjectPool.phisicalObjectVector.elements[y].rigidBodies.size();
				for (int i = 0; i < size; i++)
				{
					btTransform pos;
					gameObjectPool.phisicalObjectVector.elements[y].rigidBodies[i]->getMotionState()->getWorldTransform(pos);
					if (pos.getOrigin().y() < -60.f)
					{
						gameObjectPool.phisicalObjectVector.elements[y].deleteElement(i);
						i--;
						size--;
					}

				}

			}

			playerObject.getInstance(0)->getMotionState()->getWorldTransform(playerTransform);
			playerPos = { playerTransform.getOrigin().x(), playerTransform.getOrigin().y(), playerTransform.getOrigin().z() };
			//llog(playerPos.x(), playerPos.y(), playerPos.z());

			if (playerPos.y() < -10) { closeLevel(); }
			if (playerPos.distance({exitPosition.x, exitPosition.y, exitPosition.z }) < 2) { closeLevel(); }
			if (pickupped == false && playerPos.distance({ pickupPosition.x, pickupPosition.y, pickupPosition.z }) < 2) 
			{
				gameObjectPool.setShaderProgramToAllComponents(&textureProgramEffect);
				playerObject.sp = &textureProgramEffect;
				pickupped = true; 
				int pos = gameObjectPool.gameObjectVector.getPositionById(421);
				if (pos != -1)
				{
					gameObjectPool.gameObjectVector.RemoveElement(pos);
				}
				playerObject.objectData[0].material = Material::cyanPlastic();
			}


			camera.playerPosition = { playerPos.x(), playerPos.y(), playerPos.z() };
			camera.topDownAngle = playerAngle;
			playerObject.draw();

			gameObjectPool.drawAll();

			if (debugDraw) 
			{
				world->debugDrawWorld();
			}
			
			break;
		}
		case States::extras:

			if (!drawer2D.initialised)
			{
				setupCamera();
			}

			doCapture(0); while (isCaptureDone(0)) {};
			memcpy(drawer2D.buffer, capture.mTargetBuf, drawer2D.height*drawer2D.width * sizeof(int));
			cameraShader.bind();
			drawer2D.Render();

			if(escapeReleased)
			{
				closeCamera();
				gameState = States::mainMenu; 
				escapeReleased = false;
			}


			break;
		default:
			break;
		}

		window.pushGLStates();
		//glDisable(GL_DEPTH_TEST);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindVertexArray(0);
		//glBindTexture(GL_TEXTURE_2D, 0);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		//glUseProgram(0);
		for(int i=0; i<14; i++)
		{
			glDisableVertexAttribArray(i);
		}
		//window.resetGLStates();
		//sfml drawing
		if (gameState == States::mainMenu)
		{
			window.setMouseCursorVisible(1);
			if (!mainMenu.update(mouseButtonReleased, escapeReleased)) {exit(0);}
			
			if(levelShouldLoad)
			{
				gameObjectPool.clearAll();
				gameObjectPool.lights->clear();
				gameObjectPool.load("maps//map1.txt");

				int pos = gameObjectPool.gameObjectVector.getPositionById(420);
				if (pos != -1)
				{
					gameObjectPool.gameObjectVector.elements[pos].setMaterial(Material::whitePlastic());
					exitPosition = gameObjectPool.gameObjectVector.elements[pos].getInstance(0).getPosition();
					pickupped = false;
				}
				else
				{
					pickupped = true;
					wlog("exit not found, id:", 421);
				}
				pos = gameObjectPool.gameObjectVector.getPositionById(421);
				if (pos != -1)
				{
					gameObjectPool.gameObjectVector.elements[pos].setMaterial(Material::cyanPlastic());
					pickupPosition = gameObjectPool.gameObjectVector.elements[pos].getInstance(0).getPosition();
				}
				else
				{
					wlog("pickup not found, id:", 421);
				}
			}
			levelShouldLoad = false;
		}else
		if(gameState == States::inGameMenu)
		{
			window.setMouseCursorVisible(1);
			if (!gameMenu.update(mouseButtonReleased, escapeReleased))
			{
				gameState = States::inGame;
				escapeReleased = false;
			}
		}

		window.setView(sf::View({ 0, 0, width, height })); //todo check if necessary
		window.display();
		window.popGLStates();
	
	}

	/*
	playerObject.cleanup();

	world->removeCollisionObject(&body);
	delete body.getMotionState();
	delete body.getCollisionShape();

	world->removeCollisionObject(&sphere);
	delete sphere.getMotionState();
	delete body.getCollisionShape();

	delete dispatcher;
	delete collisionConfiguration;
	delete solver;
	delete broadPhase;
	delete world;

	textureManager.cleanUp();
	modelManager.cleanUp();
	*/
	return 0;
}

