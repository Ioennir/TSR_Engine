struct MaterialMapNames
{
	eastl::string diffuse;
	eastl::string metallic;
	eastl::string roughness;
	eastl::string normal;
	eastl::string emissive;
	eastl::string opacity;
};

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
		LOGCHECK(LOGSYSTEM_ASSIMP, MESSAGE("Submesh of " + model->name + " doesnt have vertex colors, setting to white instead."), scene->mMeshes[i]->HasVertexColors(i));
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
			LOGASSERT(LOGSYSTEM_ASSIMP, MESSAGE("One or more model " + model->name + " faces are not triangles.") , face.mNumIndices == 3);
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
		//foreach material fetch it and store its information
		// For now, we only fetch texture type and paths.
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
			eastl::string tex;
			model->materials.push_back({});
			
			if (!mapNames[i].diffuse.empty())
			{
				tex = path + mapNames[i].diffuse;
				model->materials[i].diffuse = TSR_DX11_LoadTextureFromPath(tex);
			}

			if (!mapNames[i].metallic.empty())
			{
				tex = path + mapNames[i].metallic;
				model->materials[i].metallic = TSR_DX11_LoadTextureFromPath(tex);
			}

			if (!mapNames[i].normal.empty())
			{
				tex = path + mapNames[i].normal;
				model->materials[i].normal = TSR_DX11_LoadTextureFromPath(tex);
			}

			if (!mapNames[i].roughness.empty())
			{
				tex = path + mapNames[i].roughness;
				model->materials[i].roughness = TSR_DX11_LoadTextureFromPath(tex);
			}

			if (!mapNames[i].emissive.empty())
			{
				tex = path + mapNames[i].emissive;
				model->materials[i].roughness = TSR_DX11_LoadTextureFromPath(tex);
			}

			if (!mapNames[i].opacity.empty())
			{
				tex = path + mapNames[i].opacity;
				model->materials[i].roughness = TSR_DX11_LoadTextureFromPath(tex);
			}
		}

	}

	//Apply some scale to the vertices(fbx scales are quite huge)
	//TODO(Fran): take a deep look at scaled models with different extensions
	// coming from different 3D editors ( blender, maya etc)
	r32 scale = 1.0f / 100.0f;
	for (ui32 w = 0; w < model->vertexCount; ++w)
	{
		model->totalVertices[w] = (scale * model->totalVertices[w]);
	}

}