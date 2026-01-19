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

public:
	void AddTechnique(RenderTechnique&& technique);

	RenderTechnique& GetTechnique(RenderJob::JobType type);
	bool HasTechnique(RenderJob::JobType type);

	std::vector<RenderTechnique>& GetTechniques();

public:
	void SubmitJobs(Renderer& renderer);

private:
	void CreateImplicitTechniques(Graphics& graphics, Pipeline& pipeline);

	void CreateDepthTechnique(Graphics& graphics, Pipeline& pipeline);

	static bool StepHasOpacity(const RenderGraphicsStep& geometryStep);

	RenderTechnique* m_GetTechnique(RenderJob::JobType type);

protected:
	std::vector<RenderTechnique> m_techniques;
};