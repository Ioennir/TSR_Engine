struct ModelData
{
	eastl::vector<DirectX::XMFLOAT3>	totalVertices;
	eastl::vector<ui32>					totalIndices;
	eastl::vector<ui32>					submeshStartIndex;
	eastl::vector<ui32>					submeshEndIndex;
	ui32								submeshCount;
	eastl::string						name;
};

//TODO(Fran): doing this at the start might be useful
void TSR_LoadMeshesToMemory()
{

}

void TSR_LoadMeshFromPath(ModelData * model, eastl::string path)
{
	Assimp::Importer Importer;
	//NOTE(Fran): this flags might change soon, I.E: left handedness etc.
	ui32 importFlags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices;
	const aiScene* scene = Importer.ReadFile(path.c_str(), importFlags);
	//TODO(Fran): if the model loads incorrectly this just crashes, lets just make another log so it so it drops an error.
	LOGCHECK(LOGSYSTEM_ASSIMP, Importer.GetErrorString(), !scene);
	
	model->name = scene->GetShortFilename(path.c_str());
	// Set the number of sub meshes and reserve start and end indexes
	model->submeshCount = scene->mNumMeshes;
	model->submeshStartIndex.reserve(model->submeshCount);
	model->submeshEndIndex.reserve(model->submeshCount);

	//Reserve total amount of vertex in the model
	ui32 totalVertexCount = 0;
	for (ui32 i = 0; i < scene->mNumMeshes; ++i)
	{
		totalVertexCount += scene->mMeshes[i]->mNumVertices;
	}
	model->totalVertices.reserve(totalVertexCount);

	for (ui32 i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		//bulk insert vertices
		model->totalVertices.insert(
			model->totalVertices.end(),
			PTRCAST(DirectX::XMFLOAT3*, mesh->mVertices),
			PTRCAST(DirectX::XMFLOAT3 *, mesh->mVertices + mesh->mNumVertices)
		);

	}

}

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