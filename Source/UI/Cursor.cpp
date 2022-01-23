#include "Cursor.h"

#include <SDL2/SDL.h>

#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"
#include "Texture.h"

Cursor::Cursor(const std::string& name, char* data, int dataLength) : Asset(name)
{
    // Texture used is always the same as the name of the cursor.
    Texture* texture = Services::GetAssets()->LoadTexture(GetNameNoExtension() + ".BMP");
    if(texture == nullptr)
    {
        std::cout << "Couldn't load texture for cursor " << mName << std::endl;
        return;
    }

    // Parse data from ini format.
    Vector2 hotspot;
    bool hotspotIsPercent = false;
    int frameCount = 1;

    IniParser parser(data, dataLength);
    parser.SetMultipleKeyValuePairsPerLine(false);
    while(parser.ReadLine())
    {
        while(parser.ReadKeyValuePair())
        {
            const IniKeyValue& keyValue = parser.GetKeyValue();
            if(StringUtil::EqualsIgnoreCase(keyValue.key, "hotspot"))
            {
                if(StringUtil::EqualsIgnoreCase(keyValue.value, "center"))
                {
                    hotspot.x = 0.5f;
                    hotspot.y = 0.5f;
                    hotspotIsPercent = true;
                }
                else
                {
                    hotspot = keyValue.GetValueAsVector2();
                }
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "frame count"))
            {
                frameCount = keyValue.GetValueAsInt();
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "frame rate"))
            {
                mFramesPerSecond = keyValue.GetValueAsInt();
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "allow fading"))
            {
                //TODO
                //keyValue.GetValueAsBool();
            }
            // Function
            // Source Opacity
            // Destination Opacity
        }
    }

    // Determine width/height of each cursor animation frame.
    // If cursor has multiple frames, they are laid out horizontally and all equal size.
    int frameWidth = texture->GetWidth() / frameCount;
    int frameHeight = texture->GetHeight();

    // If hotspot was a percent, convert to pixel position, now that we know the frame width/height.
    if(hotspotIsPercent)
    {
        hotspot.x = frameWidth * hotspot.x;
        hotspot.y = frameHeight * hotspot.y;
    }

    // Generate RGB masks (taken straight from SDL docs).
    unsigned int rmask, gmask, bmask, amask;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
    #else // little endian, like x86
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
    #endif

    // Create a surface from the texture.
    SDL_Surface* srcSurface = SDL_CreateRGBSurfaceFrom((void*)texture->GetPixelData(), texture->GetWidth(), texture->GetHeight(),
                                                       32, 4 * texture->GetWidth(), rmask, gmask, bmask, amask);
    if(srcSurface == nullptr)
    {
        SDL_Log("Creating surface failed: %s", SDL_GetError());
        return;
    }

    // Create cursors for each frame.
    for(int i = 0; i < frameCount; i++)
    {
        SDL_Rect srcRect;
        srcRect.x = i * frameWidth;
        srcRect.y = 0;
        srcRect.w = frameWidth;
        srcRect.h = frameHeight;

        // Copy frame from texture into a destination surface.
        SDL_Surface* dstSurface = SDL_CreateRGBSurface(0, frameWidth, frameHeight, 32, rmask, gmask, bmask, amask);
        SDL_BlitSurface(srcSurface, &srcRect, dstSurface, NULL);

        // Use destination surface to create cursor.
        SDL_Cursor* cursor = SDL_CreateColorCursor(dstSurface, (int)hotspot.x, (int)hotspot.y);
        if(cursor == nullptr)
        {
            std::cout << "Create cursor failed: " << SDL_GetError() << std::endl;
        }
        mCursorFrames.push_back(cursor);
    }
}

Cursor::~Cursor()
{
    for(auto& frame : mCursorFrames)
    {
        SDL_FreeCursor(frame);
    }
}

void Cursor::Activate(bool animate)
{
    // Set to first frame.
    if(mCursorFrames.size() > 0)
    {
        SDL_SetCursor(mCursorFrames[0]);
    }
    mFrameIndex = 0.0f;

    // Save animation pref.
    mAnimate = animate;
}

void Cursor::Update(float deltaTime)
{
    // Only need to update if there are multiple frames to animate.
    if(!mAnimate || mCursorFrames.size() < 2) { return; }

    // Increase timer, but keep in bounds.
    mFrameIndex += mFramesPerSecond * deltaTime;
    while(mFrameIndex >= mCursorFrames.size())
    {
        mFrameIndex -= mCursorFrames.size();
    }
    
    // Set frame.
    SDL_SetCursor(mCursorFrames[static_cast<int>(mFrameIndex)]);
}
