#pragma once
#include "Includes/CppIncludes.h"

class Graphics;
class CommandList;

class RenderStep
{
protected:
	RenderStep();
	RenderStep(const std::string& name);

public:
	const std::string& GetName() const;

private:
	std::string m_name;
};