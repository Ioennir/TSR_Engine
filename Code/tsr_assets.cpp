#include <WICTextureLoader.h>

struct MaterialMapNames
{
	eastl::string diffuse;
	eastl::string metallic;
	eastl::string roughness;
	eastl::string normal;
	eastl::string emissive;
	eastl::string opacity;
};

//TODO(Fran): doing this at the start might be useful
void TSR_LoadMeshesToMemory()
{

}

void TSR_LoadMeshFromPath(ModelData * model, eastl::string path)
{
	Assimp::Importer Importer;
	//NOTE(Fran): this flags might change soon, I.E: left handedness etc.
	ui32 importFlags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
	const aiScene* scene = Importer.ReadFile(path.c_str(), importFlags);
	LOGCHECK(LOGSYSTEM_ASSIMP, "Invalid path.", scene != nullptr);
	// this just crashes if scene is nullptr
	model->name = scene->GetShortFilename(path.c_str());
	// Set the number of sub meshes and reserve start and end indexes
	model->submeshCount = scene->mNumMeshes;
	model->submeshStartIndex.reserve(model->submeshCount);
	model->submeshEndIndex.reserve(model->submeshCount);
	model->submeshTexcoordStart.reserve(model->submeshCount);
	model->submeshTexcoordEnd.reserve(model->submeshCount);

	//Reserve total amount of vertex in the model
	ui32 totalVertexCount = 0;
	ui32 totalIndexCount = 0;
	for (ui32 i = 0; i < scene->mNumMeshes; ++i)
	{
		LOGCHECK(LOGSYSTEM_ASSIMP, eastl::string("Submesh of " + model->name + " doesnt have vertex colors, setting to white instead.").c_str(), scene->mMeshes[i]->HasVertexColors(i));
		totalVertexCount += scene->mMeshes[i]->mNumVertices;
		totalIndexCount += (scene->mMeshes[i]->mNumFaces * 3);
	}
	model->vertexCount = totalVertexCount;
	model->indexCount = totalIndexCount;
	model->totalVertices.reserve(TYPECAST(eastl_size_t, totalVertexCount));
	model->texCoords.reserve(TYPECAST(eastl_size_t, totalVertexCount));
	model->normals.reserve(TYPECAST(eastl_size_t, totalVertexCount));
	model->totalIndices.reserve(TYPECAST(eastl_size_t, totalIndexCount));
	ui32 indexOffset = 0;
	ui32 texCoordIndex = 0;

	for (ui32 i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		// first index of the submesh texcoord
		model->submeshTexcoordStart.push_back(texCoordIndex);
		//bool hasem = mesh->HasTangentsAndBitangents();
		//texture coordinates; check if this can be done better
		// im assuming things here as I dont fully know the assimp texturecoords member,
		eastl::vector<DirectX::XMFLOAT2> meshTexCoords;
		meshTexCoords.reserve(mesh->mNumVertices);
		for (ui32 j = 0; j < mesh->mNumVertices; ++j)
		{
			aiVector3D tc = mesh->mTextureCoords[0][j];
			DirectX::XMFLOAT2 tcdx{tc.x, tc.y};
			model->texCoords.push_back(tcdx);
		}

		texCoordIndex += mesh->mNumVertices;
		model->submeshTexcoordEnd.push_back(texCoordIndex);

		//bulk insert vertices
		model->totalVertices.insert(
			model->totalVertices.end(),
			PTRCAST(DirectX::XMFLOAT3*, mesh->mVertices),
			PTRCAST(DirectX::XMFLOAT3 *, mesh->mVertices + mesh->mNumVertices)
		);

		//bulk insert normals
		model->normals.insert(
			model->normals.end(),
			PTRCAST(DirectX::XMFLOAT3*, mesh->mNormals),
			PTRCAST(DirectX::XMFLOAT3*, mesh->mNormals + mesh->mNumVertices)
		);
		const eastl_size_t trisCount = TYPECAST(eastl_size_t, mesh->mNumFaces);
		model->submeshStartIndex.push_back(TYPECAST(ui32, model->totalIndices.size()));
		for (ui32 j = 0; j < trisCount; ++j)
		{
			const aiFace face = mesh->mFaces[j];
			LOGASSERT(LOGSYSTEM_ASSIMP, eastl::string("One or more model " + model->name + " faces are not triangles.").c_str() , face.mNumIndices == 3);
			model->totalIndices.insert(model->totalIndices.end(), face.mIndices[0] + indexOffset);
			model->totalIndices.insert(model->totalIndices.end(), face.mIndices[1] + indexOffset);
			model->totalIndices.insert(model->totalIndices.end(), face.mIndices[2] + indexOffset);
		}
		model->submeshEndIndex.push_back(TYPECAST(ui32, model->totalIndices.size()));
		indexOffset += TYPECAST(ui32, mesh->mNumVertices);
	}
	eastl::vector<MaterialMapNames> mapNames;
	model->materials.reserve(TYPECAST(eastl_size_t, scene->mNumMaterials));
	if (scene->HasMaterials())
	{
		mapNames.reserve(TYPECAST(eastl_size_t, scene->mNumMaterials));

		aiMaterial** const mats = scene->mMaterials;
		model->submeshMaterialIndex.reserve(model->submeshCount);
		for (ui32 i = 0; i < model->submeshCount; ++i)
		{
			//store the material index each mesh uses.
			model->submeshMaterialIndex.push_back(scene->mMeshes[i]->mMaterialIndex);
		}
		//foreach material fetch it and store its information somewhere.
		for (ui32 i = 0; i < scene->mNumMaterials; ++i)
		{
			mapNames.push_back({});
			const aiMaterial* mat = mats[i];
			
			aiString texName;
			//get Diffuse
			mat->GetTexture(aiTextureType_DIFFUSE, 0, &texName);
			mapNames[i].diffuse.append(texName.C_Str());
			
			texName.Clear();

			mat->GetTexture(aiTextureType_METALNESS, 0, &texName);
			mapNames[i].metallic.append(texName.C_Str());

			texName.Clear();

			mat->GetTexture(aiTextureType_SHININESS, 0, &texName);
			mapNames[i].roughness.append(texName.C_Str());

			texName.Clear();

			mat->GetTexture(aiTextureType_EMISSIVE, 0, &texName);
			mapNames[i].emissive.append(texName.C_Str());

			texName.Clear();

			mat->GetTexture(aiTextureType_NORMALS, 0, &texName);
			mapNames[i].normal.append(texName.C_Str());

			texName.Clear();

			mat->GetTexture(aiTextureType_OPACITY, 0, &texName);
			mapNames[i].opacity.append(texName.C_Str());
			
		}

		eastl::string path = "..\\..\\..\\MODELS\\";
		for (ui32 i = 0; i < scene->mNumMaterials; ++i)
		{
			HRESULT hr; 
			eastl::string tex;
			eastl::wstring wide;
			model->materials.push_back({});
			if (!mapNames[i].diffuse.empty())
			{
				ID3D11ShaderResourceView* diffuse = nullptr;
				tex = path + mapNames[i].diffuse;
				wide.append_convert(tex.data(), tex.size());
				hr = DirectX::CreateWICTextureFromFile(DX11::dxData.device, wide.c_str(), nullptr, &diffuse);
				model->materials[i].diffuse = diffuse;
				wide.clear();
			}
			if (!mapNames[i].metallic.empty())
			{
				ID3D11ShaderResourceView* metallic = nullptr;
				tex = path + mapNames[i].metallic;
				wide.append_convert(tex.data(), tex.size());
				hr = DirectX::CreateWICTextureFromFile(DX11::dxData.device, wide.c_str(), nullptr, &metallic);
				model->materials[i].metallic = metallic;
				wide.clear();
			}
			if (!mapNames[i].normal.empty())
			{
				ID3D11ShaderResourceView* normal = nullptr;
				tex = path + mapNames[i].normal;
				wide.append_convert(tex.data(), tex.size());
				hr = DirectX::CreateWICTextureFromFile(DX11::dxData.device, wide.c_str(), nullptr, &normal);
				model->materials[i].normal = normal;
				wide.clear();
			}
			if (!mapNames[i].roughness.empty())
			{
				ID3D11ShaderResourceView* roughness = nullptr;
				tex = path + mapNames[i].roughness;
				wide.append_convert(tex.data(), tex.size());
				hr = DirectX::CreateWICTextureFromFile(DX11::dxData.device, wide.c_str(), nullptr, &roughness);
				model->materials[i].roughness = roughness;
				wide.clear();
			}
			if (!mapNames[i].emissive.empty())
			{
				ID3D11ShaderResourceView* emissive = nullptr;
				tex = path + mapNames[i].emissive;
				wide.append_convert(tex.data(), tex.size());
				hr = DirectX::CreateWICTextureFromFile(DX11::dxData.device, wide.c_str(), nullptr, &emissive);
				model->materials[i].emissive = emissive;
				wide.clear();
			}
			if (!mapNames[i].opacity.empty())
			{
				ID3D11ShaderResourceView* opacity = nullptr;
				tex = path + mapNames[i].opacity;
				wide.append_convert(tex.data(), tex.size());
				hr = DirectX::CreateWICTextureFromFile(DX11::dxData.device, wide.c_str(), nullptr, &opacity);
				model->materials[i].opacity = opacity;
				wide.clear();
			}

		}

	}

	//Apply some scale to the vertices(fbx scales are quite huge)
	r32 scale = 1.0f / 100.0f;
	for (ui32 w = 0; w < model->vertexCount; ++w)
	{
		model->totalVertices[w] = (scale * model->totalVertices[w]);
	}

}

//Note(Fran): This will be outdated soon as I'm refactoring the whole renderizables data layout and 
// loading
void LoadMeshToVertex(eastl::string path, eastl::vector<Vertex>& vertexData, eastl::vector<ui32>& indices) {
	Assimp::Importer imp;
	// This loads the supplied model into the aiScene structure
	const aiScene* model = imp.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
	if (!model) {
		printf("%s", imp.GetErrorString());
		return;
	}

	// Get total amount of vertices so we can reserve
	ui32 numVertex = 0;
	for(ui32 i = 0; i < model->mNumMeshes; ++i)
	{
		const auto& mesh = model->mMeshes[i];
		for (ui32 j = 0; j < mesh->mNumVertices; ++j) 
		{
			numVertex += 1;
		}
	}
	vertexData.reserve(numVertex);

	ui32 indexOffset = 0;
	for (ui32 i = 0; i < model->mNumMeshes; ++i) {
		const auto mesh = model->mMeshes[i];
		for (ui32 j = 0; j < mesh->mNumVertices; ++j)
		{
			// Get vertices and insert them
			DirectX::XMFLOAT3 * vertices = PTRCAST(DirectX::XMFLOAT3 *, &mesh->mVertices[j]);
			DirectX::XMFLOAT3 * normal = PTRCAST(DirectX::XMFLOAT3*, &mesh->mNormals[j]);
			Vertex v = { *vertices, white, *normal };
			vertexData.push_back(v);
		}

		DirectX::XMFLOAT3* vertices = PTRCAST(DirectX::XMFLOAT3*, mesh->mVertices);

		// Fetch total number of faces and reserve index amount
		const eastl_size_t faceCount = TYPECAST(eastl_size_t, mesh->mNumFaces);
		indices.reserve(faceCount * 3);
		// Get indices and insert them
		//TODO(Fran): Add the correct offset to the indices.
		for (ui32 j = 0; j < faceCount; ++j)
		{
			const auto& face = mesh->mFaces[j];
			assert(face.mNumIndices == 3);
			indices.insert(indices.end(), face.mIndices[0] + indexOffset);
			indices.insert(indices.end(), face.mIndices[1] + indexOffset);
			indices.insert(indices.end(), face.mIndices[2] + indexOffset);
		}
		indexOffset += TYPECAST(ui32, mesh->mNumVertices);
	}
}