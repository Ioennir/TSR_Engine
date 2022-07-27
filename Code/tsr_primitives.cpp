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

// Stacks: vertical quads, Slices: horizontal quads ex: Stacks: 1 = cilinder 
void GenerateSphereGeometry(eastl::vector<Vertex>& Vertices, eastl::vector<ui32>& Indices, r32 Radius, ui32 Slices, ui32 Stacks)
{
	const r32 Pi = DirectX::XM_PI;
	r32 SliceStep = 2.0f * Pi / static_cast<r32>(Slices);
	r32 StackStep = Pi / static_cast<r32>(Stacks);
	ui32 VertexCount = Slices * (Stacks - 1) + 2;
	ui32 TrisCount = Slices * (Stacks - 1) * 2;
	ui32 IndexCount = TrisCount * 3;

	Vertices.reserve(VertexCount);
	Indices.reserve(IndexCount);
	
	//Vertices
	DirectX::XMFLOAT3 Position{ 0.0f, -Radius, 0.0f };
	DirectX::XMFLOAT3 Normal = TSR_DX_NormalizeFLOAT3(Position);
	Vertices.push_back({ Position, TSR_White, Normal});
	r32 StackAngle = Pi - StackStep;
	for (ui32 V = 0; V < Stacks - 1; ++V)
	{
		r32 SliceAngle = 0.0f;
		for (ui32 H = 0; H < Slices; ++H) {
			r32 X = Radius * sin(StackAngle) * cos(SliceAngle);
			r32 Y = Radius * cos(StackAngle);
			r32 Z = Radius * sin(StackAngle) * sin(SliceAngle);
			Position = { X, Y, Z };
			Normal = TSR_DX_NormalizeFLOAT3(Position);
			Vertices.push_back({ Position, TSR_White, Normal});
			SliceAngle += SliceStep;
		}
		StackAngle -= StackStep;
	}
	Position = { 0.0f, Radius, 0.0f };
	Normal = TSR_DX_NormalizeFLOAT3(Position);
	Vertices.push_back({ Position, TSR_White, Normal });

	//TODO(Fran): check this out
	//Indices
	for (ui32 I = 1; I <= Slices; ++I)
	{
		Indices.push_back(I);
		Indices.push_back(0);
		Indices.push_back((I - 1 == 0 ? I + Slices - 1 : I - 1));
	}

	for (ui32 I = 1; I < VertexCount - Slices - 1; ++I)
	{
		Indices.push_back(I + Slices);
		Indices.push_back(I);
		Indices.push_back(((I - 1) % Slices == 0 ? I + Slices + Slices - 1 : I + Slices - 1));

		Indices.push_back(I);
		Indices.push_back(((I - 1) % Slices == 0 ? I + Slices - 1 : I - 1));
		Indices.push_back(((I - 1) % Slices == 0 ? I + Slices + Slices - 1 : I + Slices - 1));
	}

	for (ui32 I = VertexCount - Slices - 1; I < VertexCount - 1; ++I)
	{
		Indices.push_back(I);
		Indices.push_back(((I - 1) % Slices == 0 ? I + Slices - 1 : I - 1));
		Indices.push_back(VertexCount - 1);
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
	vertices.push_back({ {0.0f, -radius, 0.0f}, TSR_White});

	//middle
	r32 sliceAngle = 0.0f;
	for (ui32 i = 0; i < slices; ++i)
	{
		r32 x = 0.0f;
		r32 z = 0.0f;
		vertices.push_back({ {x, -radius, z}, TSR_White});
		sliceAngle += sliceStep;
	}

	sliceAngle = 0.0f;
	for (ui32 i = 0; i < slices; ++i)
	{
		r32 x = radius * cos(sliceAngle);
		r32 z = radius * sin(sliceAngle);
		vertices.push_back({ {x, radius, z}, TSR_White});
		sliceAngle += sliceStep;
	}

	//top
	vertices.push_back({ { 0.0f, radius, 0.0f }, TSR_White });


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
				{DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), TSR_White},
				{DirectX::XMFLOAT3(0.5f, 0.0f, 0.0f), TSR_White},
				{DirectX::XMFLOAT3(-0.5f, 0.0f, 0.0f), TSR_White}
			});
		indices.insert(indices.end(),
			{
				0, 1, 2
			});
	}break;
	case Primitive::Plane:
	{
		// Vertices of a plane with center in 0,0,0
		DirectX::XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };
		vertices.insert(vertices.end(),
			{
				{DirectX::XMFLOAT3(-0.5f, 0.0f, 0.5f), TSR_White,normal},
				{DirectX::XMFLOAT3(0.5f, 0.0f, 0.5f), TSR_White,normal},
				{DirectX::XMFLOAT3(-0.5f, 0.0f, -0.5f), TSR_White,normal},
				{DirectX::XMFLOAT3(0.5f, 0.0f, -0.5f), TSR_White,normal}
			});
		indices.insert(indices.end(),
			{
				0, 1, 2,
				2, 1, 3
			});
	}break;
	case Primitive::Cube:
	{
		// Vertices of a cube with center in 0,0,0
		vertices.insert(vertices.end(),
			{
				{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), TSR_White},
				{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), TSR_White},
				{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), TSR_White},
				{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), TSR_White},

				{DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), TSR_White},
				{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), TSR_White},
				{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), TSR_White},
				{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), TSR_White},
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
		// has 1080 Indices
		GenerateSphereGeometry(vertices, indices, 0.5f, 20, 10);
	}break;
	case Primitive::Icosphere: {}break;
	case Primitive::Pyramid:
	{
		// Vertices for a pyramid with center 0,0,0
		vertices.insert(vertices.end(),
			{
				// base
				{{-0.5f, 0.0f, -0.5f}, TSR_White},
				{{0.5f, 0.0f, -0.5f}, TSR_White},
				{{-0.5f, 0.0f, 0.5f}, TSR_White},
				{{0.5f, 0.0f, 0.5f}, TSR_White},
				// pinacle
				{{0.0f, 1.0f, 0.0f}, TSR_White}
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
