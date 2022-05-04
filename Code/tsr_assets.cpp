#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void LoadSimpleMesh(eastl::string path, RenderData* renderData) {
	Assimp::Importer imp;
	// This loads the supplied model into the aiScene structure
	const aiScene* model = imp.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	if (!model) {
		printf("%s", imp.GetErrorString());
		return;
	}

	// Fetch total number of meshes in the scene and reserve it
	const ui32 meshCount = model->mNumMeshes;
	renderData->meshes.reserve(meshCount);
	// Iterate over each mesh
	ui32 indexOffset = 0;
	for (ui32 i = 0; i < meshCount; ++i) {
		Mesh m{};
		const auto mesh = model->mMeshes[i];
		// Get vertices and insert them
		DirectX::XMFLOAT3* vertices = reinterpret_cast<DirectX::XMFLOAT3*>(mesh->mVertices);
		m.vertices.reserve(mesh->mNumVertices);
		m.vertices.insert(m.vertices.end(), vertices, vertices + mesh->mNumVertices);

		renderData->totalVertices.insert(renderData->totalVertices.end(), vertices, vertices + mesh->mNumVertices);

		// Fetch total number of faces and reserve index amount
		const eastl_size_t faceCount = static_cast<eastl_size_t>(mesh->mNumFaces);
		m.indices.reserve(faceCount * 3);
		// Get indices and insert them
		//TODO(Fran): Add the correct offset to the indices.
		for (ui32 j = 0; j < faceCount; ++j)
		{
			const auto& face = mesh->mFaces[j];
			assert(face.mNumIndices == 3);
			m.indices.push_back(face.mIndices[0]);
			m.indices.push_back(face.mIndices[1]);
			m.indices.push_back(face.mIndices[2]);
			renderData->totalIndices.insert(renderData->totalIndices.end(), face.mIndices[0] + indexOffset);
			renderData->totalIndices.insert(renderData->totalIndices.end(), face.mIndices[1] + indexOffset);
			renderData->totalIndices.insert(renderData->totalIndices.end(), face.mIndices[2] + indexOffset);
		}
		//renderData->totalIndices.insert(renderData->totalIndices.end(), m.indices.data(), m.indices.data() + m.indices.size());
		renderData->meshes.push_back(m);
		indexOffset += static_cast<ui32>(m.vertices.size());
	}
}
