#ifndef MODEL_H
#define MODEL_H

#include "material.h"
#include "texture.h"
#include "triangle.h"

#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using std::vector;
using std::string;

class model : public hittable_list {
public:
  // constructor, expects a filepath to a 3D model.
  model(const char* path) {
    auto checker = make_shared<checker_texture>(0.32, color(.2,  .3, .1), color(.9, .9, .9));
    checker_mat = make_shared<lambertian>(checker);
 
    loadModel(path);
  }

private:
  vector<shared_ptr<material>> materials_loaded;
  vector<shared_ptr<image_texture>> textures_loaded;   // stores all the textures loaded so far,
                                                       // optimization to make sure textures aren't loaded more than once.
  string directory;
  
  shared_ptr<material> checker_mat;
  
  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
  void loadModel(string const &path) {
    Assimp::Importer import;
    import.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    const aiScene* scene = import.ReadFile(path,
                     aiProcess_Triangulate            |
                     aiProcess_JoinIdenticalVertices  |
                     aiProcess_SortByPType);
                     // | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace

    // check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      std::clog << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
      return;
    }
 
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
  }
 
  void processNode(aiNode *node, const aiScene *scene) {
    // process all node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      processMesh(mesh, scene);
    }
    
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      processNode(node->mChildren[i], scene);
    }
  }
 
  void processMesh(aiMesh* _mesh, const aiScene* scene) {
    std::clog << "Model has: " << _mesh->mNumVertices << " vertices;" << std::endl;
    std::clog << "Model has: " << _mesh->mNumFaces << " faces; At most: " << _mesh->mNumFaces * 3 << " vertices;" << std::endl;
    std::clog << "Model has normals: " << std::boolalpha << _mesh->HasNormals() << std::endl;
 
    vector<shared_ptr<triangle>> triangles;
    
    // each face is a sigle triangle
    // should have nNumFaces of trinagles in total
    point3 vs[3];
    point3 ns[3];
    point2 uvs[3];

    for (unsigned int i = 0; i < _mesh->mNumFaces; i++) {
      const aiFace& face = _mesh->mFaces[i];
      for (unsigned int j = 0; j < 3; j++) {
        const int idx = face.mIndices[j];
        vs[j][0] = _mesh->mVertices[idx].x;
        vs[j][1] = _mesh->mVertices[idx].y;
        vs[j][2] = _mesh->mVertices[idx].z;
 
        if (_mesh->HasNormals()) {
          ns[j][0] = _mesh->mNormals[idx].x;
          ns[j][1] = _mesh->mNormals[idx].y;
          ns[j][2] = _mesh->mNormals[idx].z;
        }
              
        if (_mesh->HasTextureCoords(0)) {  // Check if there are UV coordinates
          uvs[j][0] = _mesh->mTextureCoords[0][idx].x;
          uvs[j][1] = _mesh->mTextureCoords[0][idx].y;
        }
      }
      triangles.push_back(make_shared<triangle>(vs[0], vs[1], vs[2], ns[0], ns[1], ns[2], uvs[0], uvs[1], uvs[2]));
    }
    // process materials
    aiMaterial* material = scene->mMaterials[_mesh->mMaterialIndex];
    // 1. diffuse material
    auto diffuse_mat = loadMaterial(material, aiTextureType_DIFFUSE, "texture_diffuse");

    for (auto p_triangle = triangles.begin(); p_triangle != triangles.end(); p_triangle++) {
      (*p_triangle)->mat = diffuse_mat;
      add(*p_triangle);
    }
  }
 
  shared_ptr<material> loadMaterial(aiMaterial *mat, aiTextureType type, string typeName) {
    // todo: handle the case of multiple textures for a single material
    if (mat->GetTextureCount(type) == 0) {
      return checker_mat;
    }
    for (int i = 0; i < mat->GetTextureCount(type); i++) {
      aiString str;
      mat->GetTexture(type, i, &str);
      std::string filename = string(str.C_Str());
      filename = directory + '/' + filename;
      for (unsigned int j = 0; j < textures_loaded.size(); j++) {
        if (textures_loaded[j]->path == filename) {
          return materials_loaded[j];
        }
      }
      std::clog << "Loading texture at: " << filename << std::endl;
      auto tex = make_shared<image_texture>(filename.c_str());
      textures_loaded.push_back(tex);
      auto mat = make_shared<lambertian>(tex);
      materials_loaded.push_back(mat);
      return mat;
    }
  }
};

#endif
