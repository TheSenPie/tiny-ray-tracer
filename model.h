#ifndef MODEL_H
#define MODEL_H

#include "material.h"
#include "texture.h"
#include "triangle.h"

#include <vector>
#include <string>
#include <cstdlib>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using std::vector;
using std::string;

class model {
public:
  triangle* primitives;
  int primitives_count = 0;

  // constructor, expects a filepath to a 3D model.
  model(const char* path)
    : model{
        path,
        make_shared<lambertian>(make_shared<checker_texture>(0.32, color(.2,  .3, .1), color(.9, .9, .9)))
    } {}
  
  // ... and material
  model(const char* path, shared_ptr<material> material) {
    default_mat = material;
 
    load_model(path);
  }

  ~model() {
    delete [] primitives;
  }

private:
  vector<shared_ptr<material>> materials_loaded;
  vector<shared_ptr<image_texture>> textures_loaded;   // stores all the textures loaded so far,
                                                       // optimization to make sure textures aren't loaded more than once.
  string directory;

  shared_ptr<material> default_mat;

  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
  void load_model(string const &path) {
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
    
    // allocate the faces array
    uint primitives_count = calculate_number_of_faces(scene->mRootNode, scene);
    primitives = new triangle[primitives_count];
    
    process_node(scene->mRootNode, scene);
  }
  
  uint calculate_number_of_faces (const aiNode *node, const aiScene* scene) {
    uint sum = 0;
    
    // process all node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      sum += calculate_number_of_faces(mesh);
    }
    
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      sum += calculate_number_of_faces(node->mChildren[i], scene);
    }
    
    return sum;
  }
  
  uint calculate_number_of_faces (const aiMesh* _mesh) {
    return _mesh->mNumFaces;
  }
 
  void process_node(aiNode *node, const aiScene *scene) {
    // process all node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      process_mesh(mesh, scene);
    }
    
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      process_node(node->mChildren[i], scene);
    }
  }
 
  void process_mesh(aiMesh* _mesh, const aiScene* scene) {
    std::clog << "Mesh has: " << _mesh->mNumVertices << " vertices;" << std::endl;
    std::clog << "Mesh has: " << _mesh->mNumFaces << " faces; At most: " << _mesh->mNumFaces * 3 << " vertices;" << std::endl;
    std::clog << "Mesh has normals: " << std::boolalpha << _mesh->HasNormals() << std::endl;
 
    uint primitives_count_old = primitives_count;
    
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
      primitives[primitives_count].v1 = vs[0];
      primitives[primitives_count].v2 = vs[1];
      primitives[primitives_count].v3 = vs[2];
      primitives[primitives_count].n1 = ns[0];
      primitives[primitives_count].n2 = ns[1];
      primitives[primitives_count].n3 = ns[2];
      primitives[primitives_count].uv1 = uvs[0];
      primitives[primitives_count].uv2 = uvs[1];
      primitives[primitives_count].uv3 = uvs[2];
      primitives[primitives_count].update_bounds();
      primitives_count++;
    }
    
    shared_ptr<material> final_mat;
    // process materials
    aiMaterial* material = scene->mMaterials[_mesh->mMaterialIndex];
 
    // 1. diffuse material
    final_mat = loadMaterial(material, aiTextureType_DIFFUSE, "texture_diffuse");
    if (!final_mat) {
      // 2. emmisive material
      aiColor4D emission;
      if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emission) && !emission.IsBlack()) {
        color c{emission.r, emission.g, emission.b};
        auto difflight = make_shared<diffuse_light>(c);
        final_mat = difflight;
      } else {
        // 3. default material
        final_mat = default_mat;
      }
    }

    for (uint primitive_idx = primitives_count_old; primitive_idx < primitives_count; primitive_idx++) {
      primitives[primitive_idx].mat = final_mat;
    }
  }
 
  shared_ptr<material> loadMaterial(aiMaterial *mat, aiTextureType type, string typeName) {
    // todo: handle the case of multiple textures for a single material
    if (mat->GetTextureCount(type) == 0) {
      return nullptr;
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
