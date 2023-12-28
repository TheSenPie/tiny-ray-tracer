#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"

#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using std::vector;
using std::string;

class model : public hittable{
public:
  // model data
  vector<shared_ptr<image_texture>> textures_loaded; // stores all the textures loaded so far,
                                                    // optimization to make sure textures aren't loaded more than once.
  vector<mesh> meshes;
  string directory;
 
  // constructor, expects a filepath to a 3D model.
  model(const char* path) { loadModel(path); }
  
  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    for (auto mesh: meshes) {
      if(mesh.hit(r, ray_t, rec)) {
        return true;
      }
    }
    return false;
  }
 
private:
  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
  void loadModel(string const &path) {
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
    // check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
      return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
  }
 
  void processNode(aiNode *node, const aiScene *scene) {
    // process all node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      meshes.push_back(processMesh(mesh, scene));
    }
    
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      processNode(node->mChildren[i], scene);
    }
  }
 
  mesh processMesh(aiMesh *_mesh, const aiScene *scene) {
    vector<double> vertices;
    vector<int> indices;
    vector<texture> textures;
     
    for (unsigned int i = 0; i < _mesh->mNumVertices; i++) {
      // process vertex positions, normals and texture coordinates
      vertices.push_back(_mesh->mVertices[i].x);
      vertices.push_back(_mesh->mVertices[i].y);
      vertices.push_back(_mesh->mVertices[i].z);
      
      // normals
      if (_mesh->HasNormals()) { // todo: attempt to generate normals, if missing
        vertices.push_back(_mesh->mNormals[i].x);
        vertices.push_back(_mesh->mNormals[i].y);
        vertices.push_back(_mesh->mNormals[i].z);
      }
      
      // texture coordinates
      if(_mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
        // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
        // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
        vertices.push_back(_mesh->mTextureCoords[0][i].x);
        vertices.push_back(_mesh->mTextureCoords[0][i].y);
//        // tangent
//        vector.x = _mesh->mTangents[i].x;
//        vector.y = _mesh->mTangents[i].y;
//        vector.z = _mesh->mTangents[i].z;
//        vertex.Tangent = vector;
//        // bitangent
//        vector.x = _mesh->mBitangents[i].x;
//        vector.y = _mesh->mBitangents[i].y;
//        vector.z = _mesh->mBitangents[i].z;
//        vertex.Bitangent = vector;
      } else {
        vertices.push_back(0);
        vertices.push_back(0);
      }
    }
 
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < _mesh->mNumFaces; i++) {
      aiFace face = _mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++)
          indices.push_back(face.mIndices[j]);
    }

//    // process materials
//    aiMaterial* material = scene->mMaterials[_mesh->mMaterialIndex];
//    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
//    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
//    // Same applies to other texture as the following list summarizes:
//    // diffuse: texture_diffuseN
//    // specular: texture_specularN
//    // normal: texture_normalN
//    
//    
//    // 1. diffuse maps
//    vector<texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
//    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//    // 2. specular maps
//    vector<texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
//    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//    // 3. normal maps
//    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
//    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
//    // 4. height maps
//    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    
    // give proper texture
    auto checker = make_shared<checker_texture>(0.32, color(.2,  .3, .1), color(.9, .9, .9));
    auto ground_material = make_shared<lambertian>(checker);
    
    return mesh{vertices, indices, ground_material};
  }
 
  vector<texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
//    vector<texture> textures;
//    for (int i = 0; i < mat->GetTextureCount(type); i++) {
//      aiString str;
//      mat->GetTexture(type, i, &str);
//      image_texture tex;
//      textures.push_back(tex);
//    }
//    return textures;
  }
};

#endif
