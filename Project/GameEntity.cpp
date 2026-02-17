#include "GameEntity.h"


GameEntity::GameEntity(std::shared_ptr<Mesh> mesh) : GameEntity(mesh, mesh.get()->GetName()){}

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, const char* name)
{
	GameEntity::mesh = mesh;
	transform = std::make_shared<Transform>();

	GameEntity::name = name;
}

std::shared_ptr<Transform> GameEntity::GetTransform()
{
	return transform;
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

const char* GameEntity::GetName()
{
	return name;
}

void GameEntity::Draw()
{
	mesh.get()->Draw();
}
