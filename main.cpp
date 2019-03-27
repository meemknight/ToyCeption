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

#include "Camera.h"
#include "GameObject.h"
#include "Light.h"
#include "customBulletdebuggClass.h"
#include "shapesGenerator.h"
#include "GameObjectPool.h"
#include "CharacterMouseController.h"

#include "tools.h"

extern "C"
{
	//Enable dedicated graphics
	//__declspec(dllexport) DWORD NvOptimusEnablement = true;
	//__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = true;
}

float width = 1280;
float height = 720;

using glm::mat4;



int main()
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


	btTransform t;
	t.setIdentity();
	t.setOrigin({ 0, -1, 0 });

	btStaticPlaneShape *plane = new btStaticPlaneShape({ 0,1,0 }, 0);
	btMotionState *motion = new btDefaultMotionState(t);
	btRigidBody::btRigidBodyConstructionInfo info(0.0f, motion, plane);
	btRigidBody body(info);

	world->addRigidBody(&body);
#pragma endregion


	sf::ContextSettings contextSettings;
	contextSettings.depthBits = 24;
	contextSettings.stencilBits = 8;
	contextSettings.antialiasingLevel = 2;

	sf::RenderWindow window(sf::VideoMode(width, height), "glEngine", sf::Style::Default, contextSettings);

	auto windoHandle = window.getSystemHandle();

	//window.setVerticalSyncEnabled(1);

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

	Camera camera(85.f, &width, &height, 0.01f, 1500.f);
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
	ShaderProgram normalProgram(VertexShader("vertn.vert"), FragmentShader("fragn.frag"));
	ShaderProgram textureProgram(VertexShader("vertt.vert"), FragmentShader("fragt.frag"));
	ShaderProgram debugShader(VertexShader("debugShader.vert"), FragmentShader("debugShader.frag"));
	
	customBulletdebuggClass debugDrawer(&debugShader ,&camera);

	world->setDebugDrawer(&debugDrawer);
	world->getDebugDrawer()->setDebugMode(btIDebugDraw::DebugDrawModes::DBG_DrawWireframe);

	GameObjectPool gameObjectPool;
	gameObjectPool.initialize(&textureProgram, &camera, &light, world, &textureManager, &modelManager);
	gameObjectPool.load("maps//map1.txt");

	float *planVertexes = 0;
	float *planVertexes2 = 0;

	unsigned int *planIndices = 0;
	unsigned int *planIndices2 = 0;

	int plansize = 0;
	int plansize2 = 0;
	int planIndicessize = 0;
	int planIndicessize2 = 0;


	shapeGenerator::generatePlane(&planVertexes, &planIndices, 512, plansize, planIndicessize);
	std::cout << glGetString(GL_VERSION);


	

	GameObject plan(vertexBuffer(planVertexes, plansize * 4), indexBuffer(planIndices, planIndicessize * 4), vertexAttribute({ 3,3,3 }), &normalProgram, &camera);
	plan.pushElement(glm::mat4(0));
	plan.getInstance(0).setPosition(0, -1, 0);
	plan.getInstance(0).setRotation(0, 0, 0);
	plan.setMaterial(Material::greyMaterial(1, 0.5f, 0.01f, 1));

	sf::Clock c;
	sf::Clock fpsClock;
	bool updatemouse = 0;
	int frames = 0;
	
	PhisicalObject playerObject(&camera, &textureProgram, &light, world, nullptr/*new btSphereShape(1)*/, 10);
	playerObject.loadCollisionBox(modelManager.getData("objects//sphere.obj"), nullptr);
	playerObject.loadPtn323(modelManager.getData("objects//sphere.obj"), textureManager);
	playerObject.pushElement({ 0, 3 ,0 });

	//playerObject.getIndtance(0)->setFriction(0.5f);
	
	bool canJump = true;
	bool canJump2 = true;
	float jumpingCharge = 0;

	window.setTitle((char*)glGetString(GL_RENDERER));
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
			window.setTitle(c);
			frames = 0;
		}

		//glClearColor(0.1, 0.5, 1.0, 1.0);
		glViewport(0, 0, width, height);

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		mouseScroll *= 0.4f;

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
			}

		}

#pragma region keys
		if (sf::Keyboard::isKeyPressed((sf::Keyboard::Escape)))
		{
			exit(0);
			//ShowWindow((HWND)(windoHandle), SW_SHOWNOACTIVATE);
			//SendMessage((HWND)windoHandle, WM_KILLFOCUS, 0, 0);
			//updatemouse = 0;
		}

		float maxSpeed = 35000 * deltatime;
		float jumpImpulse = 7.5;
	
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			camera.moveFront(deltatime);

			playerObject.getInstance(0)->applyCentralForce({ 0,0,-maxSpeed * cos(playerAngle) });
			playerObject.getInstance(0)->applyCentralForce({ -maxSpeed * sin(playerAngle), 0, 0 });
			playerObject.getInstance(0)->activate(1);
		}
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			camera.moveBack(deltatime);

			playerObject.getInstance(0)->applyCentralForce({ 0,0,maxSpeed * cos(playerAngle) });
			playerObject.getInstance(0)->applyCentralForce({ maxSpeed * sin(playerAngle), 0, 0 });
			playerObject.getInstance(0)->activate(1);
		}
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			camera.moveLeft(deltatime);
		
			playerObject.getInstance(0)->applyCentralForce({ -maxSpeed * cos(playerAngle),0,0 });
			playerObject.getInstance(0)->applyCentralForce({ 0,0, maxSpeed * sin(playerAngle) });
			playerObject.getInstance(0)->activate(1);
		}
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			camera.moveRight(deltatime);
		
			playerObject.getInstance(0)->applyCentralForce({ maxSpeed * cos(playerAngle),0,0 });
			playerObject.getInstance(0)->applyCentralForce({ 0,0,maxSpeed * -sin(playerAngle) });
			playerObject.getInstance(0)->activate(1);
		}
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		{
			camera.moveUp(deltatime);
		}
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
		{
			camera.moveDown(deltatime);
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

		if(canJump == false)
		{
			if(jumpingCharge >= 0.2)
			{
				canJump = true;
				jumpingCharge = 0;
			}
		}
		btVector3 playerPos;
		btTransform playerTransform;
		playerObject.getInstance(0)->getMotionState()->getWorldTransform(playerTransform);
		//playerObject.getInstance(0)->setWorldTransform(playerTransform);
		playerPos = { playerTransform.getOrigin().x(), playerTransform.getOrigin().y(), playerTransform.getOrigin().z() };

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			/*
			btVector3 direction = { 0,-1,0 };
			btCollisionWorld::ClosestRayResultCallback RayCallback(playerPos + (direction * 1.01), playerPos + (direction * 1.11));
			//RayCallback.

			// Perform raycast
			world->rayTest(playerPos + (direction * 1.01), playerPos + (direction * 1.11), RayCallback);
			if (RayCallback.hasHit() && canJump) 
			{
				ilog("jumped");
				jumpingCharge = 0;
				canJump = false;
				//End = RayCallback.m_hitPointWorld;
				//Normal = RayCallback.m_hitNormalWorld;
				playerObject.getInstance(0)->applyCentralForce({ 0,jumpImpulse,0 });
				playerObject.getInstance(0)->activate(1);
			}
			*/
			/*
			if(abs(playerObject.getInstance(0)->getLinearVelocity().getY()) < 0.01)
			{
				canJump = false;
				jumpingCharge = 0;
				playerObject.getInstance(0)->applyCentralForce({ 0,jumpImpulse,0 });
				playerObject.getInstance(0)->activate(1);
			}
			*/
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
						if (pt.getDistance() < 0.012f)
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

							if (normal.y() > 0.3f /*put the threshold here */)
							{
								//The character controller is on the ground
								canJump2 = true;
								break;
							}
						}
					}
				}
			}

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

		world->stepSimulation(deltatime);

		if(gameObjectPool.phisicalObjectVector.getPositionById(250) != -1)
		{

			if (gameObjectPool.phisicalObjectVector.getElementById(250).colidesWith(0, playerObject, 0))
			{
				gameObjectPool.phisicalObjectVector.getElementById(250).objectData[0].material = Material::cyanRubber();
			}
			else
			{
				gameObjectPool.phisicalObjectVector.getElementById(250).objectData[0].material = Material::gold();
			}

		}

		playerObject.getInstance(0)->getMotionState()->getWorldTransform(playerTransform);
		//playerObject.getInstance(0)->setWorldTransform(playerTransform);
		playerPos = { playerTransform.getOrigin().x(), playerTransform.getOrigin().y(), playerTransform.getOrigin().z() };
		//ilog(playerPos.x(), playerPos.y(), playerPos.z());

		camera.playerPosition = { playerPos.x(), playerPos.y(), playerPos.z() };
		camera.topDownAngle = playerAngle;
		playerObject.draw();
		
		gameObjectPool.drawAll();

		//world->debugDrawWorld();

		plan.draw();

		window.pushGLStates();
		

		for(int i=0; i<2; i++)
		{
			glDisableVertexAttribArray(i);
		}
	

		
		sf::RectangleShape s({ 100,100 });
		s.setPosition({ 100, 100 });

		s.setFillColor(sf::Color::Green);
	
		window.setView(sf::View({ 0, 0, width, height }));
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


