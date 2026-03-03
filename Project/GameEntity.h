#pragma once

#include <wrl/client.h>
#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"

class GameEntity
{
private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
	const char* name;
public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, const char* name);

	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	const char* GetName();

	void SetMaterial(std::shared_ptr<Material> material);

	void Draw();
};

