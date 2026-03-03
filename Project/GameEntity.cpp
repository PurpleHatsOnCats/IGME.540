#include "GameEntity.h"


GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) : GameEntity(mesh, material, mesh.get()->GetName()){}

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, const char* name)
{
	GameEntity::mesh = mesh;
	transform = std::make_shared<Transform>();
	GameEntity::material = material;

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

std::shared_ptr<Material> GameEntity::GetMaterial()
{
	return material;
}

const char* GameEntity::GetName()
{
	return name;
}

void GameEntity::SetMaterial(std::shared_ptr<Material> material)
{
	GameEntity::material = material;
}

void GameEntity::Draw()
{
	// Set the active vertex and pixel shaders
	Graphics::Context->VSSetShader(material->GetVertexShader().Get(), 0, 0);
	Graphics::Context->PSSetShader(material->GetPixelShader().Get(), 0, 0);
	mesh.get()->Draw();
}
