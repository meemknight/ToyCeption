//////////////////////////////////////////////////////////////////
//MenuApi.cpp
//(c) Luta Vlad - 2019
// this library is under MIT license, do not remove this notice
//https://github.com/meemknight/MenuApi
//////////////////////////////////////////////////////////////////
#include "MenuApi.h"


namespace ma
{

#pragma region MenuElement
	// -2 if nothing was pressed
	// -1 if a normal butoon was pressed
	// 0, 1, 3, ... if a button that leads to a menu was pressed

	int MenuElement::checkInput(sf::RenderWindow * window, bool mouseReleased)
	{
		if (mouseReleased)
		{
			sf::IntRect rect(getPositionX(), getPositionY(), getSize().x, getSize().y);
			if (rect.contains(sf::Mouse::getPosition(*window)))
			{
				additionalFunctonality();

				if (actionType != nullptr)
				{
					if (actionType->getType() == type::function)
					{
						actionType->execute();
						return -1;
					}
					else if (actionType->getType() == type::menuHolder)
					{
						return 0;
					}
				}
				else
				{
					return -1;
				}
			}
		}

		return -2;
	}

#pragma endregion

	void Menu::updateElementsPosition(MenuHolder *h)
	{ //todo
		if (h == nullptr)
		{
			h = mainMenu;
		}

		h->updateElementsPosition();
	}

	void Menu::updateBackgrounsPosition()
	{
		if (background.getTexture() != nullptr)
		{
			int sparex = window->getSize().x - background.getTexture()->getSize().x;
			int sparey = window->getSize().y - background.getTexture()->getSize().y;
			sparex /= 2;
			sparey /= 2;

			backgroundPositionx = sparex;
			backgroundPositiony = sparey;
		}
	}

	int Menu::update(bool mouseReleased, bool escapeReleased)
	{
		MenuHolder *holder;
		holder = mainMenu;

		if (stack.size() != 0)
		{
			for (int i = 0; i < stack.size(); i++)
			{


				if (holder->elements[stack[i].first]->actionType != nullptr && holder->elements[stack[i].first]->actionType->getType() == type::menuHolder)
				{
					holder = (MenuHolder*)holder->elements[stack[i].first]->actionType;
				}
				else
					if (holder->elements[stack[i].first]->getType() == type::buttonGroup)
					{
						auto temp = ((ButtonGroup*)holder->elements[stack[i].first])->buttons[stack[i].second].first;
						if (temp->actionType->getType() == type::menuHolder)
						{
							holder = (MenuHolder*)temp->actionType;
						}
					}
			}

		}

		if (checkForResize)
		{
			updateElementsPosition(holder);
			updateBackgrounsPosition();
		}

		if (background.getTexture() != nullptr)
		{
			background.setPosition(backgroundPositionx, backgroundPositiony);
			window->draw(background);
		}

		int input = -1;
		int secondary = 0;
		for (int i = holder->elements.size() - 1; i >= 0; i--)
		{

			holder->elements[i]->draw(window);
			int temp = holder->elements[i]->checkInput(window, mouseReleased);
			if (temp >= 0)
			{
				input = i;
				secondary = temp;
			}
		}

		if (input != -1)
		{
			stack.push_back({ input , secondary });
		}

		if (escapeReleased)
		{
			if (stack.size() == 0)
			{
				return 0;
			}

			stack.pop_back();
		}

		if (backButton != nullptr)
		{
			//todo make it set it's position
			backButton->setPositionX(backgroundPositionx + backButtonPaddingx);
			backButton->setPositionY(backgroundPositiony + backButtonPaddingy);
			backButton->draw(window);
			if (backButton->checkInput(window, mouseReleased) != -2)
			{
				if (stack.size() == 0)
				{
					return 0;
				}

				stack.pop_back();
			}

		}

		return 1;
	}


#pragma region MenuHolder



	void MenuHolder::appendElement(MenuElement * e)
	{
		elements.push_back(e);
		updateElementsPosition();
	}

	void MenuHolder::updateElementsPosition()
	{
		int spareSpace = menu->window->getSize().y;

		for (int i = elements.size() - 1; i >= 0; i--)
		{
			spareSpace -= elements[i]->getSize().y;
		}

		int padding;
		if (spareSpace > 0)
		{
			padding = spareSpace / (elements.size() + 1);
		}
		else
		{
			padding = 0;
		}

		int pos = 0;
		for (int i = 0; i < elements.size(); i++)
		{
			pos += padding;
			elements[i]->setPositionY(pos);
			elements[i]->setPositionX(((menu->window->getSize().x) / 2) - (elements[i]->getSize().x / 2));
			pos += elements[i]->getSize().y;
		}

	}
#pragma endregion

#pragma region TextButton

	void TextButton::draw(sf::RenderWindow *window)
	{
		window->draw(s);
		textContent.setPosition(s.getPosition());
		auto startingPos = s.getPosition();
		int spareX = getSize().x;
		int spareY = getSize().y;

		spareY -= textContent.getLocalBounds().height;
		spareY /= 2;
		startingPos.y += spareY;

		spareX -= textContent.getLocalBounds().width;
		spareX /= 2;
		startingPos.x += spareX;

		textContent.setPosition(startingPos);
		window->draw(textContent);

	}

	Point TextButton::getSize()
	{
		if (s.getTexture() == nullptr)
		{
			return { 0,0 };
		}
		auto size = s.getTexture()->getSize();
		return Point({ (int)size.x, (int)size.y });
	}

	void TextButton::setPositionX(int x)
	{
		s.setPosition({ (float)x, (float)s.getPosition().y });
	}

	void TextButton::setPositionY(int y)
	{
		s.setPosition({ (float)s.getPosition().x, (float)y });
	}

	int TextButton::getPositionX()
	{
		return s.getPosition().x;
	}

	int TextButton::getPositionY()
	{
		return s.getPosition().y;
	}
#pragma endregion

#pragma region Function
	void Function::execute()
	{
		if (functionPointer != nullptr)
		{
			functionPointer();
		}
	}
#pragma endregion

#pragma region IconButton

	void IconButton::draw(sf::RenderWindow * window)
	{
		window->draw(backgroundSprite);
		window->draw(foregroundSprite);
	}


	Point IconButton::getSize()
	{
		auto sizeF = sf::Vector2u();
		if (foregroundSprite.getTexture() == nullptr)
		{
			sizeF = { 0,0 };
		}
		else
		{
			sizeF = foregroundSprite.getTexture()->getSize();
		}

		auto sizeB = sf::Vector2u();
		if (backgroundSprite.getTexture() == nullptr)
		{
			sizeB = { 0,0 };
		}
		else
		{
			sizeB = backgroundSprite.getTexture()->getSize();
		}



		if (sizeB.x > sizeF.x) { sizeF.x = sizeB.x; }
		if (sizeB.y > sizeF.y) { sizeF.y = sizeB.y; }

		return Point({ (int)sizeF.x, (int)sizeF.y });
	}

	void IconButton::setPositionX(int x)
	{
		backgroundSprite.setPosition({ (float)x, (float)backgroundSprite.getPosition().y });
		foregroundSprite.setPosition({ (float)x, (float)foregroundSprite.getPosition().y });
	}

	void IconButton::setPositionY(int y)
	{
		backgroundSprite.setPosition({ (float)backgroundSprite.getPosition().x, (float)y });
		foregroundSprite.setPosition({ (float)foregroundSprite.getPosition().x, (float)y });
	}

	int IconButton::getPositionX()
	{
		//todo ?
		return backgroundSprite.getPosition().x;
	}

	int IconButton::getPositionY()
	{
		return backgroundSprite.getPosition().y;
	}
#pragma endregion

#pragma region ButtonGroup

	void ButtonGroup::appendElement(MenuElement * element)
	{
		buttons.push_back({ element, 0 });
		updateElementsPosition();
	}

	void ButtonGroup::updateElementsPosition()
	{
		int spareSpace = menu->window->getSize().x;

		for (auto &i : buttons)
		{
			spareSpace -= i.first->getSize().x;
		}

		int gaps = buttons.size() - 1;
		if (gaps <= 0) { gaps = 1; }
		spareSpace /= gaps * 4;

		buttons[0].second = spareSpace;
		int currentPos = buttons[0].first->getSize().x + spareSpace * 2;
		for (int i = 1; i < buttons.size(); i++)
		{
			buttons[i].second = currentPos;
			currentPos += buttons[i].first->getSize().x;
			currentPos += spareSpace;

		}

	}

	void ButtonGroup::draw(sf::RenderWindow * window)
	{
		for (auto &i : buttons) //todo ???
		{
			i.first->draw(window);
		}
	}

	Point ButtonGroup::getSize()
	{
		Point size = { 0,0 };
		for (auto &i : buttons)
		{
			if (i.first->getSize().y > size.y)
			{
				size.y = i.first->getSize().y;
			}
		}
		int spareSpace = buttons.begin()->second;
		size.x = (buttons.end() - 1)->second + (buttons.end() - 1)->first->getSize().x + spareSpace;
		return size;
	}

	void ButtonGroup::setPositionX(int x)
	{
		for (auto &i : buttons)
		{
			i.first->setPositionX(x + i.second);
		}
	}

	void ButtonGroup::setPositionY(int y)
	{
		for (auto &i : buttons)
		{
			i.first->setPositionY(y);
		}
	}

	int ButtonGroup::getPositionX()
	{
		//todo make other checks like this
		if (buttons.size() == 0)
		{
			throw;
		}

		return buttons[0].first->getPositionX();
	}

	int ButtonGroup::getPositionY()
	{
		//todo make other checks like this
		if (buttons.size() == 0)
		{
			throw;
		}

		return buttons[0].first->getPositionY();
	}

	int ButtonGroup::checkInput(sf::RenderWindow * window, bool mouseReleased)
	{
		additionalFunctonality();

		int valueReturned = -2;
		if (mouseReleased)
		{
			for (int i = 0; i < buttons.size(); i++)
			{
				sf::IntRect rect(buttons[i].first->getPositionX(), buttons[i].first->getPositionY(), buttons[i].first->getSize().x, buttons[i].first->getSize().y);
				if (rect.contains(sf::Mouse::getPosition(*window)))
				{
					buttons[i].first->additionalFunctonality();

					if (buttons[i].first->actionType != nullptr)
					{

						if (buttons[i].first->actionType->getType() == type::function)
						{
							buttons[i].first->actionType->execute();
							if (valueReturned < 0)
							{
								valueReturned = -1;
							}
						}
						else if (buttons[i].first->actionType->getType() == type::menuHolder)
						{
							//return 1;
							valueReturned = i;
						}
					}

				}
			}

		}

		return valueReturned;
	}

#pragma endregion

	void OnOffButton::draw(sf::RenderWindow * window)
	{
		if (backgroundSprite.getTexture() != nullptr)
		{
			window->draw(backgroundSprite);
		}

		if (data != nullptr)
		{
			if (*data == true)
			{
				if (onStateSprite.getTexture() != nullptr)
				{
					window->draw(onStateSprite);
				}
			}
			else
			{
				if (offStateSprite.getTexture() != nullptr)
				{
					window->draw(offStateSprite);
				}
			}
		}
	}

	Point OnOffButton::getSize()
	{

		auto sizeB = sf::Vector2u();
		if (backgroundSprite.getTexture() == nullptr)
		{
			sizeB = { 0,0 };
		}
		else
		{
			sizeB = backgroundSprite.getTexture()->getSize();
		}

		auto sizeO = sf::Vector2u();
		if (onStateSprite.getTexture() == nullptr)
		{
			sizeO = { 0,0 };
		}
		else
		{
			sizeO = onStateSprite.getTexture()->getSize();
		}

		auto sizeF = sf::Vector2u();
		if (offStateSprite.getTexture() == nullptr)
		{
			sizeF = { 0,0 };
		}
		else
		{
			sizeF = offStateSprite.getTexture()->getSize();
		}



		if (sizeB.x > sizeO.x) { sizeO.x = sizeB.x; }
		if (sizeB.y > sizeO.y) { sizeO.y = sizeB.y; }

		if (sizeF.x > sizeO.x) { sizeO.x = sizeF.x; }
		if (sizeF.y > sizeO.y) { sizeO.y = sizeF.y; }



		return Point({ (int)sizeO.x, (int)sizeO.y });
	}

	void OnOffButton::setPositionX(int x)
	{
		backgroundSprite.setPosition({ (float)x, (float)backgroundSprite.getPosition().y });
		onStateSprite.setPosition({ (float)x, (float)onStateSprite.getPosition().y });
		offStateSprite.setPosition({ (float)x, (float)offStateSprite.getPosition().y });
	}

	void OnOffButton::setPositionY(int y)
	{
		backgroundSprite.setPosition({ (float)backgroundSprite.getPosition().x, (float)y });
		onStateSprite.setPosition({ (float)onStateSprite.getPosition().x, (float)y });
		offStateSprite.setPosition({ (float)offStateSprite.getPosition().x, (float)y });
	}

	int OnOffButton::getPositionX()
	{
		return backgroundSprite.getPosition().x;
	}

	int OnOffButton::getPositionY()
	{
		return backgroundSprite.getPosition().y;
	}

	void OnOffButton::additionalFunctonality()
	{
		if (data != nullptr)
		{
			*data = !(*data);
		}
	}

}

