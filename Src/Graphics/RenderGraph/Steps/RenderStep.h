#pragma once
#include "Includes/CppIncludes.h"

class Graphics;
class Pipeline;
class CommandList;

class RenderStep
{
protected:
	RenderStep();
	RenderStep(const std::string& name);

public:
	const std::string& GetName() const;

	bool IsEnabled() const;
	void SetEnabled(bool enabled);

	virtual void Initialize(Graphics& graphics, Pipeline& pipeline);
	virtual void Update();

private:
	std::string m_name;
	bool m_enabled = true;
};