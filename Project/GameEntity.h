#pragma once

#include <wrl/client.h>
#include <memory>
#include "Transform.h"
#include "Mesh.h"

class GameEntity
{
private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
	const char* name;
public:
	GameEntity(std::shared_ptr<Mesh> mesh);
	GameEntity(std::shared_ptr<Mesh> mesh, const char* name);

	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Mesh> GetMesh();
	const char* GetName();

	void Draw();
};

