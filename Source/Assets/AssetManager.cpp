#include "AssetManager.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "FileSystem.h"
#include "imstream.h"
#include "Services.h"

AssetManager::~AssetManager()
{
	// All the loaded stuff has to be unloaded!
    UnloadAssets(mLoadedTexts);
	UnloadAssets(mLoadedShaders);
	
	UnloadAssets(mLoadedSheeps);
	UnloadAssets(mLoadedBSPs);
    UnloadAssets(mLoadedBSPLightmaps);
	UnloadAssets(mLoadedActionSets);
	UnloadAssets(mLoadedSceneAssets);
	UnloadAssets(mLoadedSIFs);
	
	UnloadAssets(mLoadedVertexAnimations);
	UnloadAssets(mLoadedAnimations);
	UnloadAssets(mLoadedGases);
	
	UnloadAssets(mLoadedTextures);
	UnloadAssets(mLoadedModels);
	
	UnloadAssets(mLoadedYaks);
	UnloadAssets(mLoadedSoundtracks);
	UnloadAssets(mLoadedAudios);
	
	UnloadAssets(mLoadedBarns);
}

void AssetManager::AddSearchPath(const std::string& searchPath)
{
    // If the search path already exists in the list, don't add it again.
    if(std::find(mSearchPaths.begin(), mSearchPaths.end(), searchPath) != mSearchPaths.end())
    {
        return;
    }
    mSearchPaths.push_back(searchPath);
}

std::string AssetManager::GetAssetPath(const std::string& fileName)
{
    // We have a set of paths, at which we should search for the specified filename.
    // So, iterate each search path and see if the file is in that folder.
    std::string assetPath;
    for(const std::string& searchPath : mSearchPaths)
    {
        if(Path::FindFullPath(fileName, searchPath, assetPath))
        {
            return assetPath;
        }
    }
    return std::string();
}

std::string AssetManager::GetAssetPath(const std::string& fileName, std::initializer_list<std::string> extensions)
{
    // If already has an extension, just use the normal path find function.
    if(File::HasExtension(fileName))
    {
        return GetAssetPath(fileName);
    }
    
    // Otherwise, we have a filename, but multiple valid extensions.
    // A good example is a movie file. The file might be called "intro", but the extension could be "avi" or "bik".
    for(const std::string& extension : extensions)
    {
        std::string assetPath = GetAssetPath(fileName + "." + extension);
        if(!assetPath.empty())
        {
            return assetPath;
        }
    }
    return std::string();
}

bool AssetManager::LoadBarn(const std::string& barnName)
{
    // If the barn is already in the map, then we don't need to load it again.
    if(mLoadedBarns.find(barnName) != mLoadedBarns.end()) { return true; }
    
    // Find path to barn file.
    std::string assetPath = GetAssetPath(barnName);
    if(assetPath.empty())
    {
        std::cout << "Barn doesn't exist at any search path." << std::endl;
		return false;
    }
    
    // Load barn file.
    mLoadedBarns[barnName] = new BarnFile(assetPath);
	return true;
}

void AssetManager::UnloadBarn(const std::string& barnName)
{
    // If the barn isn't in the map, we can't unload it!
    auto iter = mLoadedBarns.find(barnName);
    if(iter == mLoadedBarns.end()) { return; }
    
    // Delete barn.
    BarnFile* barn = iter->second;
    delete barn;
    
    // Remove from map.
    mLoadedBarns.erase(iter);
}

void AssetManager::WriteBarnAssetToFile(const std::string& assetName)
{
	WriteBarnAssetToFile(assetName, "");
}

void AssetManager::WriteBarnAssetToFile(const std::string& assetName, const std::string& outputDir)
{
	BarnFile* barn = GetBarnContainingAsset(assetName);
	if(barn != nullptr)
	{
		barn->WriteToFile(assetName, outputDir);
	}
}

void AssetManager::WriteAllBarnAssetsToFile(const std::string& search)
{
	WriteAllBarnAssetsToFile(search, "");
}

void AssetManager::WriteAllBarnAssetsToFile(const std::string& search, const std::string& outputDir)
{
	// Pass the buck to all loaded barn files.
	for(auto& entry : mLoadedBarns)
	{
		entry.second->WriteAllToFile(search, outputDir);
	}
}

Audio* AssetManager::LoadAudio(const std::string& name)
{
    return LoadAsset<Audio>(SanitizeAssetName(name, ".WAV"), &mLoadedAudios);
}

Soundtrack* AssetManager::LoadSoundtrack(const std::string& name)
{
    return LoadAsset<Soundtrack>(SanitizeAssetName(name, ".STK"), &mLoadedSoundtracks);
}

Animation* AssetManager::LoadYak(const std::string& name)
{
    return LoadAsset<Animation>(SanitizeAssetName(name, ".YAK"), &mLoadedYaks);
}

Model* AssetManager::LoadModel(const std::string& name)
{
    return LoadAsset<Model>(SanitizeAssetName(name, ".MOD"), &mLoadedModels);
}

Texture* AssetManager::LoadTexture(const std::string& name)
{
    return LoadAsset<Texture>(SanitizeAssetName(name, ".BMP"), &mLoadedTextures);
}

Texture* AssetManager::LoadSceneTexture(const std::string& name)
{
    // Load texture per usual.
    Texture* texture = LoadTexture(name);
    
    // A "scene" texture means it is rendered as part of the 3D game scene (as opposed to a 2D UI texture).
    // These textures look better if you apply bilinear filtering.
    if(texture != nullptr && texture->GetRenderType() != Texture::RenderType::AlphaTest)
    {
        texture->SetFilterMode(Texture::FilterMode::Bilinear);
    }
    return texture;
}

GAS* AssetManager::LoadGAS(const std::string& name)
{
    return LoadAsset<GAS>(SanitizeAssetName(name, ".GAS"), &mLoadedGases);
}

Animation* AssetManager::LoadAnimation(const std::string& name)
{
    return LoadAsset<Animation>(SanitizeAssetName(name, ".ANM"), &mLoadedAnimations);
}

Animation* AssetManager::LoadMomAnimation(const std::string& name)
{
    // GK3 has this notion of a "mother-of-all-animations" file. Thing is, it's nearly identical to a normal .ANM file...
    // Only difference I could find is MOM files support a few more keywords.
    // Anyway, it's all the same thing in my eyes!
    return LoadAsset<Animation>(SanitizeAssetName(name, ".MOM"), &mLoadedMomAnimations);
}

VertexAnimation* AssetManager::LoadVertexAnimation(const std::string& name)
{
    return LoadAsset<VertexAnimation>(SanitizeAssetName(name, ".ACT"), &mLoadedVertexAnimations);
}

SceneInitFile* AssetManager::LoadSIF(const std::string& name)
{
    return LoadAsset<SceneInitFile>(SanitizeAssetName(name, ".SIF"), &mLoadedSIFs);
}

SceneAsset* AssetManager::LoadSceneAsset(const std::string& name)
{
    return LoadAsset<SceneAsset>(SanitizeAssetName(name, ".SCN"), &mLoadedSceneAssets);
}

NVC* AssetManager::LoadNVC(const std::string& name)
{
    return LoadAsset<NVC>(SanitizeAssetName(name, ".NVC"), &mLoadedActionSets);
}

BSP* AssetManager::LoadBSP(const std::string& name)
{
    return LoadAsset<BSP>(SanitizeAssetName(name, ".BSP"), &mLoadedBSPs);
}

BSPLightmap* AssetManager::LoadBSPLightmap(const std::string& name)
{
    return LoadAsset<BSPLightmap>(SanitizeAssetName(name, ".MUL"), &mLoadedBSPLightmaps);
}

SheepScript* AssetManager::LoadSheep(const std::string& name)
{
    // Sheep assets need more complex/custom creation login, provided in the create callback.
    return LoadAsset<SheepScript>(SanitizeAssetName(name, ".SHP"), &mLoadedSheeps, [](const std::string& name, char* buffer, unsigned int bufferSize) {

        // Determine whether this is a binary sheep asset.
        if(bufferSize >= 8 && memcmp(buffer, "GK3Sheep", 8) == 0)
        {
            return new SheepScript(name, buffer, bufferSize);
        }

        // This doesn't appear to be a binary sheep file, so it might be a text sheep file.
        // Let's try compiling it on-the-fly!
        imstream stream(buffer, bufferSize);
        return Services::GetSheep()->Compile(name, stream);
    });
}

Cursor* AssetManager::LoadCursor(const std::string& name)
{
    return LoadAsset<Cursor>(SanitizeAssetName(name, ".CUR"), nullptr);
}

Font* AssetManager::LoadFont(const std::string& name)
{
	return LoadAsset<Font>(SanitizeAssetName(name, ".FON"), nullptr);
}

Shader* AssetManager::LoadShader(const std::string& name)
{
    // Assumes vert/frag shaders have the same name.
    return LoadShader(name, name);
}

Shader* AssetManager::LoadShader(const std::string& vertName, const std::string& fragName)
{
    // Return existing shader if already loaded.
	std::string key = vertName + fragName;
	auto it = mLoadedShaders.find(key);
	if(it != mLoadedShaders.end())
	{
		return it->second;
	}

    // Get paths for vert/frag shaders.
	std::string vertFilePath = GetAssetPath(vertName + ".vert");
	std::string fragFilePath = GetAssetPath(fragName + ".frag");

    // Try to load shader.
	Shader* shader = new Shader(vertFilePath.c_str(), fragFilePath.c_str());
	if(shader == nullptr || !shader->IsGood())
	{
		return nullptr;
	}
	
	// Cache and return.
	mLoadedShaders[key] = shader;
	return shader;
}

TextAsset* AssetManager::LoadText(const std::string& name)
{
    // Specifically DO NOT delete the asset buffer when creating TextAssets, since they take direct ownership of it.
    return LoadAsset<TextAsset>(SanitizeAssetName(name, ""), &mLoadedTexts, nullptr, false);
}

void AssetManager::UnloadText(TextAsset* text)
{
    UnloadAsset<TextAsset>(text, mLoadedTexts);
}

BarnFile* AssetManager::GetBarn(const std::string& barnName)
{
	// If we find it, return it.
	auto iter = mLoadedBarns.find(barnName);
	if(iter != mLoadedBarns.end())
	{
		return iter->second;
	}
	
	//TODO: Maybe load barn if not loaded?
	return nullptr;
}

BarnFile* AssetManager::GetBarnContainingAsset(const std::string& fileName)
{
	// Iterate over all loaded barn files to find the asset.
	for(const auto& entry : mLoadedBarns)
	{
		BarnAsset* asset = entry.second->GetAsset(fileName);
		if(asset != nullptr)
		{
			// If the asset is a pointer, we need to redirect to the correct BarnFile.
			// If the correct Barn isn't available, spit out an error and fail.
			if(asset->IsPointer())
			{
                BarnFile* barn = GetBarn(asset->barnFileName);
                if(barn == nullptr)
                {
                    std::cout << "Asset " << fileName << " exists in Barn " << asset->barnFileName << ", but that Barn is not loaded!" << std::endl;
                }
                return barn;
			}
			else
			{
				return entry.second;
			}
		}
	}
	
	// Didn't find the Barn containing this asset.
	return nullptr;
}

std::string AssetManager::SanitizeAssetName(const std::string& assetName, const std::string& expectedExtension)
{
    // We want to add the expected extension if no extension already exists on the name.
    if(!File::HasExtension(assetName))
    {
        return assetName + expectedExtension;
    }
    return assetName;
}

template<class T>
T* AssetManager::LoadAsset(const std::string& assetName, std::unordered_map_ci<std::string, T*>* cache, std::function<T* (const std::string&, char*, unsigned int)> createFunc, bool deleteBuffer)
{
    // See if this asset is already loaded in the cache
    // If so, we can just return it right away.
    if(cache != nullptr)
    {
        auto it = cache->find(assetName);
        if(it != cache->end())
        {
            return it->second;
        }
    }
	
    // Create the asset.
    T* asset = CreateAsset(assetName, createFunc, deleteBuffer);
    
	// Add entry in cache, if we have a cache.
	if(cache != nullptr)
	{
		(*cache)[assetName] = asset;
	}

	//std::cout << "Loaded asset " << upperName << std::endl;
	return asset;
}

template<class T>
T* AssetManager::CreateAsset(const std::string& assetName, std::function<T* (const std::string&, char*, unsigned int)> createFunc, bool deleteBuffer)
{
    // Retrieve the buffer, from which we'll create the asset.
    unsigned int bufferSize = 0;
    char* buffer = CreateAssetBuffer(assetName, bufferSize);
    
    // If no buffer could be found, we're in trouble!
    if(buffer == nullptr)
    {
        //std::cout << "Asset " << upperName << " could not be loaded!" << std::endl;
        return nullptr;
    }
    
    // Generate asset from the data buffer.
    // For now, always use uppercase version of asset name for the asset, as this matches the original game.
    std::string upperName = StringUtil::ToUpperCopy(assetName);
    T* asset = createFunc != nullptr ? createFunc(upperName, buffer, bufferSize) : new T(upperName, buffer, bufferSize);
    
    // Delete the buffer after use (or it'll leak).
    if(deleteBuffer)
    {
        delete[] buffer;
    }
    
    // Aaaand there it is.
    return asset;
}

char* AssetManager::CreateAssetBuffer(const std::string& assetName, unsigned int& outBufferSize)
{
	// First, see if the asset exists at any asset search path.
	// If so, we load the asset directly from file.
	// Loose files take precedence over packaged barn assets.
	std::string assetPath = GetAssetPath(assetName);
	if(!assetPath.empty())
	{
		// Open the file, or error if failed.
		std::ifstream file(assetPath);
		if(!file.good())
		{
			std::cout << "Found asset path, but could not open file for " << assetName << std::endl;
			return nullptr;
		}
		
		// Read the file contents into a char buffer.
		// Important to read to intermediate std::string first!
		// Doesn't read correctly without that bit.
		std::stringstream bufferStream;
		bufferStream << file.rdbuf();
		std::string fileContentsStr = bufferStream.str();
		
		// Copy file contents to a char array.
		outBufferSize = static_cast<int>(fileContentsStr.length()) + 1;
		char* buffer = new char[fileContentsStr.length() + 1];
		std::strcpy(buffer, fileContentsStr.c_str());
		return buffer;
	}
	
	// If no file to load, we'll get the asset from a barn.
	BarnFile* barn = GetBarnContainingAsset(assetName);
	if(barn != nullptr)
	{
		// Find asset entry in this barn, or fail.
		BarnAsset* barnAsset = barn->GetAsset(assetName);
		if(barnAsset == nullptr)
		{
			std::cout << "Asset " << assetName << " appears to be in barn " << barn->GetName() << " (based on asset manifest), but could not retrieve the asset!" << std::endl;
			return nullptr;
		}
		
		// Create a buffer of the correct size.
		outBufferSize = barnAsset->uncompressedSize;
		char* buffer = new char[outBufferSize];
		
		// Extract the asset to that buffer.
        // If the asset is compressed, this will perform decompression.
		barn->Extract(barnAsset, buffer, outBufferSize);
		return buffer;
	}
	
	// Couldn't find this asset!
	return nullptr;
}

template<class T>
void AssetManager::UnloadAsset(T* asset, std::unordered_map_ci<std::string, T*>& cache)
{
    // Remove from cache.
    auto it = cache.find(asset->GetName());
    if(it != cache.end())
    {
        cache.erase(it);
    }

    // Delete asset.
    delete asset;
}

template<class T>
void AssetManager::UnloadAssets(std::unordered_map_ci<std::string, T*>& cache)
{
	// Delete all assets in the cache.
	for(auto& entry : cache)
	{
		delete entry.second;
	}
	
	// Clear the cache.
	cache.clear();
}
