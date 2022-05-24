struct ETransforms 
{
	eastl::vector<DirectX::XMFLOAT3>	positions;
	eastl::vector<DirectX::XMFLOAT4>	rotations;
	eastl::vector<DirectX::XMFLOAT3>	scales;
};

struct ETransform
{
	DirectX::XMFLOAT3	position;
	DirectX::XMFLOAT4	rotation;
	DirectX::XMFLOAT3	scale;
};

struct ERenderizables
{

};

struct ERenderizable
{

};

struct Entities
{
	ETransforms		transforms;
	ERenderizables	renderizables;
};

struct Entity
{
	ui64	eID;
};

void TSR_UpdateEntityPositionData(Entity * entity, DirectX::XMFLOAT3 newPosition, Entities * entities)
{
	entities->transforms.positions[entity->eID] = newPosition;
}

void TSR_UpdateEntityRotationData(Entity* entity, DirectX::XMFLOAT4 newRotation, Entities* entities)
{
	entities->transforms.rotations[entity->eID] = newRotation;
}

void TSR_UpdateEntityScaleData(Entity* entity, DirectX::XMFLOAT3 newScale, Entities* entities)
{
	entities->transforms.scales[entity->eID] = newScale;
}

void TSR_InitializeEntities(Entities * entities) 
{
	// reserve memory for 1000 entities.
	entities->transforms.positions.reserve(1000);
	entities->transforms.rotations.reserve(1000);
	entities->transforms.scales.reserve(1000);
}
