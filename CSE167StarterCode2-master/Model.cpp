#include "Model.h"
#include "Window.h"
#include "stb_image.h"

Model::Model(const char * path){
	textures_loaded = std::vector<Material>();
	loadModel(path);
}

void Model::loadModel(std::string path) {
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	processNode(scene->mRootNode, scene);
}


void Model::processNode(aiNode *node, const aiScene *scene)
{
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh* Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	Material mat;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// process vertex positions
		glm::vec3 p;
		p.x = mesh->mVertices[i].x;
		p.y = mesh->mVertices[i].y;
		p.z = mesh->mVertices[i].z;

		vertex.pos = p;

		if (mesh->HasNormals()) {
			glm::vec3 n;
			n.x = mesh->mNormals[i].x;
			n.y = mesh->mNormals[i].y;
			n.z = mesh->mNormals[i].z;
			vertex.norm = n;
		}

		if (mesh->mTextureCoords[0]) // texture coordinates
		{
			glm::vec2 texcoords;
			texcoords.x = mesh->mTextureCoords[0][i].x;
			texcoords.y = mesh->mTextureCoords[0][i].y;
			vertex.tc = texcoords;
		}
		else
			vertex.tc = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}
	
	// indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) 	
			indices.push_back(face.mIndices[j]);
		
	}

	// process material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		mat = loadMaterial(material);
		if (mat.name == std::string(""))
			std::cout << mat.name.c_str()<< mesh->mMaterialIndex << std::endl;
		
	}

	return new Mesh(vertices, indices, mat);
}


Material Model::loadMaterial(aiMaterial *aimat)
{
	Material mat = Material();
	
	aiString str;
	aimat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
	bool skip = false;
	for (unsigned int j = 0; j < textures_loaded.size(); j++)
	{
		if (std::strcmp(textures_loaded[j].name.data(), str.C_Str()) == 0)
		{
			mat.texture = textures_loaded[j].texture;
			mat.name = str.C_Str();
			skip = true;
			break;
		}
	}
	if (!skip)
	{   // if texture hasn't been loaded already, load it
		Material m;
		if (str.C_Str() == "") {
			std::cerr << "No material path " << std::endl;
			return mat;
		}
		m.texture = TextureFromFile(str.C_Str());
		m.name = str.C_Str();
		textures_loaded.push_back(m); // add to loaded textures
		mat = m;
	}
	
	return mat;
}

GLuint Model::TextureFromFile(const char *path)
{
	std::string name, ext;
	std::string pathstr = std::string(path);
	int dot = pathstr.find_first_of(".");
	name = pathstr.substr(0, dot);
	ext = pathstr.substr(dot);

	std::string filename = "png/" + name  + ".png";

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << filename << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void Model::draw(GLuint shaderProgram)
{
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i]->draw(shaderProgram);
}  