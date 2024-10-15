#include "Input.h"
#include "Macros/ErrorMacros.h"

/*
			Keyboard
*/

Input::KeyState Input::Keys::GetKeyState(unsigned short key) const
{
	for (const auto& keyEvent : m_events)
		if (keyEvent.first == key)
			return keyEvent.second;

	return KeyState::None;
};

void Input::Keys::UpdateEvents()
{
	size_t eventListSize = m_events.size();

	for(size_t eventIndex = 0; eventIndex < eventListSize;)
	{
		Input::KeyState keyState = m_events.at(eventIndex).second;
		if(keyState == Input::KeyState::Pressed || keyState == Input::KeyState::Released)
		{
			if (keyState == KeyState::Pressed)
				m_events.push_back({ m_events.at(eventIndex).first, Input::KeyState::Held }); // if key was pressed in previous frame, now its held

			m_events.erase(m_events.begin() + eventIndex); // removing event for key that was pressed or released in previous frame
			eventListSize--;
		}
		else
			eventIndex++;
	}
}

void Input::Keys::CleanupHeldKey(unsigned short key)
{
	size_t eventListSize = m_events.size();

	for (size_t eventIndex = 0; eventIndex < eventListSize;)
	{
		if (m_events.at(eventIndex).first == key)
		{
			m_events.erase(m_events.begin() + eventIndex); // removing event for held key
			eventListSize--;
		}
		else
			eventIndex++;
	}
}

void Input::Keys::ReleaseAllKeys()
{
	size_t eventListSize = m_events.size(); // this size will be counted without newly added events since we don't need to iterate over them

	for (size_t eventIndex = 0; eventIndex < eventListSize;)
	{
		Input::KeyState keyState = m_events.at(eventIndex).second;
		if (keyState == Input::KeyState::Held || keyState == KeyState::Pressed)
		{
			m_events.push_back({ m_events.at(eventIndex).first, Input::KeyState::Released });
			m_events.erase(m_events.begin() + eventIndex);
			eventListSize--;
		}
		else
			eventIndex++;
	}
}

void Input::Keys::PushEvent(unsigned short key, Input::KeyState state)
{
	if (state == KeyState::Released)
		CleanupHeldKey(key);

	m_events.push_back({ key, state });
}

/*
			Mouse
*/

void Input::Mouse::SetPosition(POINTS position)
{
	if(m_positionInitialized)
	{
		m_positionDelta.x = position.x - m_position.x;
		m_positionDelta.y = position.y - m_position.y;
	}

	m_position = position;

	m_positionInitialized = true;
}

POINTS Input::Mouse::GetDelta() const
{
	return m_positionDelta;
}

void Input::Mouse::CleanupDelta()
{
	m_positionDelta.x = 0;
	m_positionDelta.y = 0;
}

/*
			Input
*/

bool Input::GetKeyDown(unsigned short key) const
{
	return keyInput.GetKeyState(key) == KeyState::Pressed;
}

bool Input::GetKeyUp(unsigned short key) const
{
	return keyInput.GetKeyState(key) == KeyState::Released;
}

bool Input::GetKey(unsigned short key) const
{
	KeyState keyState = keyInput.GetKeyState(key);

	return keyState == KeyState::Pressed || keyState == KeyState::Held;
}

POINTS Input::GetMouseDelta() const
{
	return mouse.GetDelta();
}

void Input::Update()
{
	keyInput.UpdateEvents();
	mouse.CleanupDelta();
}

void Input::ReleaseAllKeys()
{
	keyInput.ReleaseAllKeys();
}

void Input::SetMousePosition(POINTS position)
{
	mouse.SetPosition(position);
}

void Input::PushKeyEvent(unsigned short key, KeyState state)
{
	keyInput.PushEvent(key, state);
}