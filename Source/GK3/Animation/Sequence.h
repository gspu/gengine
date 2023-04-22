//
// Clark Kromenaker
//
// An animation sequence specifies a list of things to play in sequence:
// 1) A list of animations (.ANM), to be played in sequence.
// 2) A list of textures (.BMP), to be played in sequence.
// 3) A single texture (.BMP) containing multiple animation frames (like cursors).
// 
// GK3 doesn't have very many of these, and they aren't referenced in Sheep logic, SIFs, NVCs, etc.
// In the original game, animated cursors use these under the hood, as well as the animated text on timeblock screens.
// An "animation sequencer" was used to determine which frame to use.
// 
// In G-Engine, I'll use the data in these, but otherwise they don't do much!
// 
// On disk, these assets have a .SEQ extension.
//
#pragma once
#include "Asset.h"

#include <vector>

class Texture;

class Sequence : public Asset
{
public:
    Sequence(const std::string& name, AssetScope scope, char* data, int dataLength);

    int GetFramesPerSecond() const { return mFramesPerSecond; }

    int GetTextureCount() { return mTextures.size(); }
    Texture* GetTexture(int index) { return (index >= 0 && index < mTextures.size()) ? mTextures[index] : nullptr; }

private:
    int mFramesPerSecond = 15;
    std::vector<Texture*> mTextures;
};