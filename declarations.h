#pragma once
#include <SFML/Graphics.hpp>

void initializeMenu(sf::RenderWindow *window);
void loadLevel();
void closeLevel();

enum class States
{
	none = 0,
	mainMenu,
	inGame,
	inGameMenu,
};