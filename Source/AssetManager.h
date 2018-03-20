//
//  AssetManager.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/17/17.
//
#pragma once
#include <string>
#include <vector>
#include "BarnFile.h"
#include "Audio.h"
#include "Model.h"
#include "Texture.h"
#include "BSP.h"
#include "SIF.h"
#include "Scene.h"
#include "NVC.h"
#include "Soundtrack.h"

using namespace std;

class AssetManager
{
public:
    AssetManager();
    ~AssetManager();
    
    void AddSearchPath(string searchPath);
    
    void LoadBarn(string barnName);
    void UnloadBarn(string barnName);
    BarnFile* GetBarn(string barnName);
    
    Audio* LoadAudio(string name);
    Soundtrack* LoadSoundtrack(string name);
    
    Model* LoadModel(string name);
    Texture* LoadTexture(string name);
    
    SIF* LoadSIF(string name);
    Scene* LoadScene(string name);
    NVC* LoadNVC(string name);
    BSP* LoadBSP(string name);
    
private:
    // A list of paths to search for assets.
    // In priority order, since we'll search in order, and stop when we find the item.
    vector<string> mSearchPaths;
    
    // A map of loaded barn files. If an asset isn't found on any search path,
    // we then search each loaded barn file for the asset.
    unordered_map<string, BarnFile*> mLoadedBarns;
    
    // A list of loaded assets, so we can just return existing assets if already loaded.
    unordered_map<string, Texture*> mLoadedTextures;
    unordered_map<string, Audio*> mLoadedAudios;
    
    
    string GetAssetPath(string fileName);
    BarnFile* GetContainingBarn(string fileName);
    
    template<class T> T* LoadAsset(string assetName, unordered_map<string, T*>* cache);
};
