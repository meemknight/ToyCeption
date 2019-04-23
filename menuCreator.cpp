#include "MenuApi.h"
#include "tools.h"
#include "GameObjectPool.h"
#include "declarations.h"
#include <SFML/Audio.hpp>

extern States gameState;
extern GameObjectPool gameObjectPool;
extern PhisicalObject *playerPointer;
extern sf::RenderWindow *windowPointer;
extern sf::Music mainMusic;

bool musicPlaying = true;
bool vsynk = true;
bool showFramerate = false;
bool debugDraw = false;

void closeLevel()
{
	gameState = States::mainMenu;
	gameObjectPool.clearAll();
}

bool levelShouldLoad = false;
int levelToLoad = 0;

struct LoadLevel
{
	int index = 0;
	void operator()()
	{
		levelShouldLoad = true;
		levelToLoad = index;
	}
};

void loadLevel()
{
	levelShouldLoad = true;

}

void setVsynk()
{
	windowPointer->setVerticalSyncEnabled(vsynk);
}

void setMusic()
{
	if(musicPlaying)
	{
		mainMusic.play();
	}else
	{
		mainMusic.pause();
	}
}

void loadTexture(sf::Texture &t, const char* c)
{
	if (!t.loadFromFile(c))
	{
		elog("couldn't load the ", c, " texture");
	}
}

ma::Menu mainMenu;
ma::Menu gameMenu;
ma::MenuHolder mainHolder;
ma::MenuHolder gameHolder;
sf::Font font;

sf::Texture backgroundT;
sf::Texture bigTextHolderT;
sf::Texture mediumTextHolderT;
sf::Texture smallTextHolderT;
sf::Texture bigBrickT;
sf::Texture onTexture;
sf::Texture offTexture;
sf::Texture backButton;
sf::Texture textAreaT;

void initializeMenu(sf::RenderWindow *window)
{
	font.loadFromFile("ui//Blobtastics.ttf");

	loadTexture(backgroundT, "ui//background.png");
	loadTexture(bigTextHolderT, "ui//bigTextHolder.png");
	loadTexture(mediumTextHolderT, "ui//mediumTextHolder.png");
	loadTexture(bigBrickT, "ui//bigbrick.png");
	loadTexture(offTexture, "ui//noButton.png");
	loadTexture(onTexture, "ui//yesBUtton.png");
	loadTexture(smallTextHolderT, "ui//smallbrick.png");
	loadTexture(backButton, "ui//backButton.png");
	loadTexture(textAreaT, "ui//textArea.png");

	mainMenu.background.setTexture(backgroundT);
	mainMenu.window = window;
	mainMenu.checkForResize = true;
	mainMenu.mainMenu = &mainHolder;
	mainMenu.backButton = new ma::IconButton(nullptr, &backButton, nullptr);
	mainHolder.menu = &mainMenu;
	mainMenu.backButtonPaddingx = 400;
	mainMenu.backButtonPaddingy = 250;


	gameMenu.window = window;
	gameMenu.checkForResize = true;
	gameMenu.mainMenu = &gameHolder;
	gameMenu.backButton = new ma::IconButton(nullptr, &backButton, nullptr);
	gameMenu.backButtonPaddingx = 10;
	gameMenu.backButtonPaddingy = 10;
	gameHolder.menu = &gameMenu;


	///settings
	auto settings = new ma::MenuHolder(&mainMenu);
	auto soundGroup = new ma::ButtonGroup(&mainMenu);
	soundGroup->appendElement(new ma::OnOffButton(&offTexture, &onTexture, nullptr, &musicPlaying, new ma::Function(&setMusic)));
	soundGroup->appendElement(new ma::TextButton(&mediumTextHolderT, font, nullptr, "music", 50, sf::Color::White));
	auto vsynkGroup = new ma::ButtonGroup(&mainMenu);
	vsynkGroup->appendElement(new ma::OnOffButton(&offTexture, &onTexture, nullptr, &vsynk, new ma::Function(&setVsynk)));
	vsynkGroup->appendElement(new ma::TextButton(&mediumTextHolderT, font, nullptr, "vsynk", 50, sf::Color::White));
	auto framerateGroup = new ma::ButtonGroup(&mainMenu);
	framerateGroup->appendElement(new ma::OnOffButton(&offTexture, &onTexture, nullptr, &showFramerate));
	framerateGroup->appendElement(new ma::TextButton(&mediumTextHolderT, font, nullptr, "show framerate", 50, sf::Color::White));
	auto debugGroup = new ma::ButtonGroup(&mainMenu);
	debugGroup->appendElement(new ma::OnOffButton(&offTexture, &onTexture, nullptr, &debugDraw));
	debugGroup->appendElement(new ma::TextButton(&mediumTextHolderT, font, nullptr, "debug draw", 50, sf::Color::White));

	settings->appendElement(new ma::TextButton(&bigTextHolderT, font, nullptr, "Settings", 50, sf::Color::White));
	settings->appendElement(soundGroup);
	settings->appendElement(vsynkGroup);
	settings->appendElement(framerateGroup);
	settings->appendElement(debugGroup);
	///

	auto howToPlay = new ma::MenuHolder(&mainMenu);
	howToPlay->appendElement(new ma::TextButton(&textAreaT, font, nullptr, "In this game you play as a toy.\nAll you have to do is to get\nto the other side of the level.\nUse (WASD) or the arrows\nto move and feel free to interract\nwith any object that might help\nyou. Skins in this game also\nhave a visual effect.\nCollect them and use the\nvisual effects at the etras section.", 24));

	auto extrasButton = new ma::TextButton(&bigBrickT, font, new ma::Function([] {gameState = States::extras; }), "Extras", 50);
	extrasButton->s.setColor(sf::Color::Blue);

	auto playMenu = new ma::MenuHolder(&mainMenu);
	playMenu->appendElement(new ma::TextButton(&smallTextHolderT, font, new ma::Function(LoadLevel{ 1 }), "1", 74));
	playMenu->appendElement(new ma::TextButton(&smallTextHolderT, font, new ma::Function(LoadLevel{ 2 }), "2", 74));


	mainHolder.appendElement(new ma::TextButton(&bigTextHolderT, font, nullptr, "ToyCeption", 50));
	auto playButton = new ma::TextButton(&bigBrickT, font, playMenu, "Play", 50);
	auto settingsButton = new ma::TextButton(&bigBrickT, font, settings, "Settings", 50);
	auto infoButton = new ma::TextButton(&bigBrickT, font, howToPlay, "Info", 50);

	playButton->s.setColor(sf::Color::Green);
	settingsButton->s.setColor(sf::Color::Red);

	mainHolder.appendElement(playButton);
	mainHolder.appendElement(settingsButton);
	mainHolder.appendElement(extrasButton);
	mainHolder.appendElement(infoButton);

	//gameHolder.appendElement(new ma::TextButton(&bigTextHolderT, font, nullptr, "ToyCeption", 50));
	gameHolder.appendElement(settingsButton);
	gameHolder.appendElement(new ma::TextButton(&bigBrickT, font, new ma::Function(&closeLevel), "Exit", 50));
}