enum class Primitive : ui32 {
	Triangle,
	Plane,
	Cube,
	FlatCube,
	Sphere,
	Icosphere,
	Pyramid,
	Cilinder,
	Cone,
	Torus,
	Capsule,
	Count
};

// stacks: vertical quads, slices: horizontal quads ex: stacks: 1 = cilinder 
void GenerateSphereGeometry(eastl::vector<Vertex>& vertices, eastl::vector<ui32>& indices, r32 radius, ui32 slices, ui32 stacks)
{
	const r32 Pi = DirectX::XM_PI;
	r32 sliceStep = 2.0f * Pi / static_cast<r32>(slices);
	r32 stackStep = Pi / static_cast<r32>(stacks);
	ui32 vertexCount = slices * (stacks - 1) + 2;
	ui32 trisCount = slices * (stacks - 1) * 2;
	ui32 indexCount = trisCount * 3;

	vertices.reserve(vertexCount);
	indices.reserve(indexCount);
	
	//Vertices
	DirectX::XMFLOAT3 position{ 0.0f, -radius, 0.0f };
	DirectX::XMFLOAT3 normal = TSR_DX_NormalizeFLOAT3(position);
	vertices.push_back({ position, white, normal});
	r32 stackAngle = Pi - stackStep;
	for (ui32 v = 0; v < stacks - 1; ++v)
	{
		r32 sliceAngle = 0.0f;
		for (ui32 h = 0; h < slices; ++h) {
			r32 x = radius * sin(stackAngle) * cos(sliceAngle);
			r32 y = radius * cos(stackAngle);
			r32 z = radius * sin(stackAngle) * sin(sliceAngle);
			position = { x, y, z };
			normal = TSR_DX_NormalizeFLOAT3(position);
			vertices.push_back({ position, white, normal});
			sliceAngle += sliceStep;
		}
		stackAngle -= stackStep;
	}
	position = { 0.0f, radius, 0.0f };
	normal = TSR_DX_NormalizeFLOAT3(position);
	vertices.push_back({ position, white, normal });

	//TODO(Fran): check this out
	//Indices
	for (ui32 i = 1; i <= slices; ++i)
	{
		indices.push_back(i);
		indices.push_back(0);
		indices.push_back((i - 1 == 0 ? i + slices - 1 : i - 1));
	}

	for (ui32 i = 1; i < vertexCount - slices - 1; ++i)
	{
		indices.push_back(i + slices);
		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices + slices - 1 : i + slices - 1));

		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices - 1 : i - 1));
		indices.push_back(((i - 1) % slices == 0 ? i + slices + slices - 1 : i + slices - 1));
	}

	for (ui32 i = vertexCount - slices - 1; i < vertexCount - 1; ++i)
	{
		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices - 1 : i - 1));
		indices.push_back(vertexCount - 1);
	}

}

//TODO(Fran): It does weird stuff sometimes, check this out.
void GenerateCilinderGeometry(eastl::vector<Vertex>& vertices, eastl::vector<ui32>& indices, r32 radius, ui32 slices)
{
	const r32 Pi = DirectX::XM_PI;
	r32 sliceStep = 2.0f * Pi / static_cast<r32>(slices);
	ui32 vertexCount = (slices * 2) + 2;
	ui32 trisCount = (slices * 4);
	ui32 indexCount = trisCount * 3;

	vertices.reserve(vertexCount);
	indices.reserve(indexCount);

	//bot
	vertices.push_back({ {0.0f, -radius, 0.0f}, white });

	//middle
	r32 sliceAngle = 0.0f;
	for (ui32 i = 0; i < slices; ++i)
	{
		r32 x = 0.0f;
		r32 z = 0.0f;
		vertices.push_back({ {x, -radius, z},white });
		sliceAngle += sliceStep;
	}

	sliceAngle = 0.0f;
	for (ui32 i = 0; i < slices; ++i)
	{
		r32 x = radius * cos(sliceAngle);
		r32 z = radius * sin(sliceAngle);
		vertices.push_back({ {x, radius, z},white });
		sliceAngle += sliceStep;
	}

	//top
	vertices.push_back({ { 0.0f, radius, 0.0f }, white });


	//Indices
	for (ui32 i = 1; i <= slices; ++i)
	{
		indices.push_back(i);
		indices.push_back(0);
		indices.push_back((i - 1 == 0 ? i + slices - 1 : i - 1));
	}

	for (ui32 i = 1; i < vertexCount - slices - 1; ++i)
	{
		indices.push_back(i + slices);
		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices + slices - 1 : i + slices - 1));

		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices - 1 : i - 1));
		indices.push_back(((i - 1) % slices == 0 ? i + slices + slices - 1 : i + slices - 1));
	}

	for (ui32 i = vertexCount - slices - 1; i < vertexCount - 1; ++i)
	{
		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices - 1 : i - 1));
		indices.push_back(vertexCount - 1);
	}

}

//TODO(Fran): Add normals
void TSR_DX11_BuildPrimitiveBuffers(Primitive primitive, ID3D11Device* device, ModelBuffers * buffers)
{
	eastl::vector<Vertex> vertices;
	eastl::vector<ui32> indices;
	switch (primitive)
	{
	case Primitive::Triangle:
	{
		// Vertices of a triangle with center in 0,0,0
		vertices.insert(vertices.end(),
			{
				{DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), white},
				{DirectX::XMFLOAT3(0.5f, 0.0f, 0.0f), white},
				{DirectX::XMFLOAT3(-0.5f, 0.0f, 0.0f), white}
			});
		indices.insert(indices.end(),
			{
				0, 1, 2
			});
	}break;
	case Primitive::Plane:
	{
		// Vertices of a plane with center in 0,0,0
		vertices.insert(vertices.end(),
			{
				{DirectX::XMFLOAT3(-0.5f, 0.0f, 0.5f), white},
				{DirectX::XMFLOAT3(0.5f, 0.0f, 0.5f), white},
				{DirectX::XMFLOAT3(-0.5f, 0.0f, -0.5f), white},
				{DirectX::XMFLOAT3(0.5f, 0.0f, -0.5f), white}
			});
		indices.insert(indices.end(),
			{
				0, 1, 2,
				1, 2, 3
			});
	}break;
	case Primitive::Cube:
	{
		// Vertices of a cube with center in 0,0,0
		vertices.insert(vertices.end(),
			{
				{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), white},
				{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), white},
				{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), white},
				{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), white},

				{DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), white},
				{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), white},
				{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), white},
				{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), white},
			});
		indices.insert(indices.end(),
			{
				2, 0, 3, 3, 0, 1,
				0, 4, 1, 1, 4, 6,
				0, 2, 4, 4, 2, 5,
				4, 5, 6, 6, 5, 7,
				5, 2, 7, 7, 2, 3
			});
	}break;
	case Primitive::FlatCube: {}break;
	case Primitive::Sphere:
	{
		// has 1080 indices
		GenerateSphereGeometry(vertices, indices, 0.5f, 20, 10);
	}break;
	case Primitive::Icosphere: {}break;
	case Primitive::Pyramid:
	{
		// vertices for a pyramid with center 0,0,0
		vertices.insert(vertices.end(),
			{
				// base
				{{-0.5f, 0.0f, -0.5f}, white},
				{{0.5f, 0.0f, -0.5f}, white},
				{{-0.5f, 0.0f, 0.5f}, white},
				{{0.5f, 0.0f, 0.5f}, white},
				// pinacle
				{{0.0f, 1.0f, 0.0f}, white}
			});
		indices.insert(indices.end(),
			{
				//base
				0, 1, 2,
				1, 3, 2,
				//sides
				1, 4, 3,
				1, 0, 4,
				0, 2, 4,
				2, 3, 4
			});
	}break;
	case Primitive::Cilinder:
	{
		GenerateCilinderGeometry(vertices, indices, 0.5f, 20);
	}break;
	case Primitive::Cone: {}break;
	case Primitive::Torus: {}break;
	case Primitive::Capsule: {}break;
	default:
	{
		printf("Wrong primitive value.\n");
	}
	}

	//Build Vertex Buffer
	TSR_DX11_BuildBuffer(device, 
						sizeof(Vertex), 
						0, 
						static_cast<ui32>(vertices.size()), 
						D3D11_USAGE_IMMUTABLE, 
						D3D11_BIND_VERTEX_BUFFER, 
						vertices.data(), 
						&buffers->vertexBuffer
						);
	
	//Build Index Buffer
	TSR_DX11_BuildBuffer(device,
						sizeof(ui32),
						0,
						static_cast<ui32>(indices.size()),
						D3D11_USAGE_IMMUTABLE,
						D3D11_BIND_INDEX_BUFFER,
						indices.data(),
						&buffers->indexBuffer
						);
}
