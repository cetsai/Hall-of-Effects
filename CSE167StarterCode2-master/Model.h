#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <string>

#include "Mesh.h"


class Model
{
    public:
        Model(const char *path);
		
        void draw(GLuint shaderProgram);	
    private:

        std::vector<Mesh*> meshes;
		std::vector<Material> textures_loaded;
		
        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh* processMesh(aiMesh *mesh, const aiScene *scene);
        
		Material loadMaterial(aiMaterial *mat);
		GLuint TextureFromFile(const char *path);
};