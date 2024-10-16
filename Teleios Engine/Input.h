#pragma once
#include "includes/CppIncludes.h"
#include "Macros/KeyMacros.h"

class Input
{
public:
	enum class KeyState
	{
		None,
		Pressed,
		Held,
		Released
	};

	class Keys
	{
	public:
		void DrawImguiWindow() const;

		KeyState GetKeyState(unsigned short key) const;

		void UpdateEvents();

		void CleanupHeldKey(unsigned short key);

		void ReleaseAllKeys();

		void PushEvent(unsigned short key, KeyState state);

	private:
		std::vector<std::pair<unsigned short, KeyState>> m_events = {};
	};

	class Mouse
	{
	public:
		void DrawImguiWindow() const;
		
		POINTS GetDelta() const;

		void SetPosition(POINTS position);

		void SetDelta(POINTS delta);

		void CleanupDelta();

	private:
		POINTS m_positionDelta;
		POINTS m_position;
	};

public: // Keyboard interface
	bool GetKeyDown(unsigned short key) const;

	bool GetKeyUp(unsigned short key) const;

	bool GetKey(unsigned short key) const;

public: // Mouse interface
	POINTS GetMouseDelta() const;

public:	// Internal stuff
	void DrawImguiWindow() const;

	void Update();

	void ReleaseAllKeys();

	void SetMousePosition(POINTS position);

	void SetMouseDelta(POINTS delta);

	void PushKeyEvent(unsigned short key, KeyState state);

private:
	Mouse mouse;
	Keys keyInput;
};