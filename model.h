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
    vector<shared_ptr<image_texture>> textures;
     
    
    std::clog << "Model has: " << _mesh->mNumVertices << " vertices;" << std::endl;
    std::clog << "Model has normals: " << _mesh->HasNormals() << std::endl;
 
    for (unsigned int i = 0; i < _mesh->mNumVertices; i++) {
      // process vertex positions, normals and texture coordinates
      vertices.push_back(_mesh->mVertices[i].x);
      vertices.push_back(_mesh->mVertices[i].y);
      vertices.push_back(_mesh->mVertices[i].z);
      
      std::clog << "v" << i << " x: " << _mesh->mVertices[i].x << " y: " << _mesh->mVertices[i].y << " z: "<< _mesh->mVertices[i].z << std::endl;
      if ((i + 1) % 3 == 0) {
        std::clog << "\n\n" << std::endl;
      }
      
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
    
    std::cout << "Total vertices: " << vertices.size() << std::endl;
 
    std::cout << "Reading indicies, beep-boop!" << std::endl;
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < _mesh->mNumFaces; i++) {
      aiFace face = _mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++) {
          indices.push_back(face.mIndices[j]);
//          std::cout << face.mIndices[j] << std::endl;
        }
    }
    
    std::cout << "Total indicies: " << indices.size() << std::endl;

    // process materials
    aiMaterial* material = scene->mMaterials[_mesh->mMaterialIndex];
 
    // 1. diffuse maps
    auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//    // 2. specular maps
//    vector<texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
//    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//    // 3. normal maps
//    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
//    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
//    // 4. height maps
//    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    
    // give proper texture
    auto ground_material = make_shared<lambertian>(textures.front());
    
    return mesh{vertices, indices, ground_material};
  }
 
  vector<shared_ptr<image_texture>> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
    vector<shared_ptr<image_texture>> textures;
    for (int i = 0; i < mat->GetTextureCount(type); i++) {
      aiString str;
      mat->GetTexture(type, i, &str);
      std::string filename = string(str.C_Str());
      filename = directory + '/' + filename;
      std::clog << "Texture path is: " << filename << std::endl;
      auto tex = make_shared<image_texture>(filename.c_str());
      textures.push_back(tex);
    }
    return textures;
  }
};

#endif
