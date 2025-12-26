#pragma once
#include "Includes/CppIncludes.h"
#include "RenderTechnique.h"

class Renderer;

class Mesh
{
public:
	Mesh() = default;
	Mesh(Mesh&&) noexcept = default;
	Mesh(const Mesh&) = delete;

	virtual ~Mesh() = default;

public:
	void Initialize(Graphics& graphics, Pipeline& pipeline);

	void Update(Graphics& graphics, Pipeline& pipeline);

public:
	void AddTechnique(RenderTechnique&& technique);

	RenderTechnique& GetTechnique(const std::string& name);

	std::vector<RenderTechnique>& GetTechniques();

public:
	void SubmitJobs(Renderer& renderer);

protected:
	std::vector<RenderTechnique> m_techniques;
};