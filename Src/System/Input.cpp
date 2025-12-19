#include "Input.h"
#include "Macros/ErrorMacros.h"

#include <imgui.h>

/*
			Keyboard
*/

void Input::Keys::DrawImguiWindow() const
{
	ImGui::Text("Keybord:");

	static constexpr const char* statusNames[4] = { "None", "Pressed", "Held", "Released" };

	for (const auto& keyEvent : m_events)
	{
		THROW_INTERNAL_ERROR_IF("Invalid input was passed", keyEvent.first == 0);

		UINT keyScanCode = MapVirtualKeyA(keyEvent.first, MAPVK_VK_TO_VSC);

		if(keyScanCode != 0 && keyEvent.first != VK_LWIN && keyEvent.first != VK_RWIN) // skipping buttons that don't belong to scan code list
		{
			std::string keyName(32, '\0');

			LONG lParam = static_cast<LONG>(keyScanCode) << 16;

			int result = GetKeyNameTextA(lParam, keyName.data(), 32);

			if (result == 0)
				THROW_LAST_ERROR;

			keyName.resize(result);

			ImGui::Text(("key: " + keyName + " is " + statusNames[short(keyEvent.second)]).c_str());
		}
		else if (keyEvent.first <= VK_XBUTTON2 && keyEvent.first != VK_CANCEL) // handling only mouse buttons here
		{
			static constexpr const char* mouseButtonNames[6] = { "Left mouse button", "Right mouse button", "", "Middle mouse button", "1 Thumb mouse button", "2 Thumb mouse button"};

			ImGui::Text((std::string("key: ") + mouseButtonNames[keyEvent.first - 1] + " is " + statusNames[short(keyEvent.second)]).c_str());
		}
	}
}

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

void Input::Mouse::DrawImguiWindow() const
{
	ImGui::Text("Mouse:");

	std::string mousePositionStr = "Mouse position: { x=";
	mousePositionStr += std::to_string(m_position.x);
	mousePositionStr += ", y=";
	mousePositionStr += std::to_string(m_position.y);
	mousePositionStr += " }";

	std::string mouseDeltaStr = "Mouse delta { x=";
	mouseDeltaStr += std::to_string(m_positionDelta.x);
	mouseDeltaStr += ", y=";
	mouseDeltaStr += std::to_string(m_positionDelta.y);
	mouseDeltaStr += " }";

	ImGui::Text(mousePositionStr.c_str());
	ImGui::Text(mouseDeltaStr.c_str());
}

void Input::Mouse::SetPosition(POINTS position)
{
	m_position = position;
}

void Input::Mouse::SetDelta(POINTS delta)
{
	m_positionDelta = delta;
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

void Input::DrawImguiWindow(bool isLayerVisible) const
{
	if (!isLayerVisible)
		return;

	if(ImGui::Begin("Input"))
	{
		mouse.DrawImguiWindow();
		keyInput.DrawImguiWindow();
	}

	ImGui::End();
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

void Input::SetMouseDelta(POINTS delta)
{
	mouse.SetDelta(delta);
}

void Input::PushKeyEvent(unsigned short key, KeyState state)
{
	keyInput.PushEvent(key, state);
}