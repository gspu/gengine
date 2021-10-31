//
// AudioManager.h
//
// Clark Kromenaker
//
// Handles playback of all types of audio.
//
#pragma once
#include <vector>

#include <fmod.hpp>
#include <fmod_errors.h>

class Audio;
class Vector3;

// A "handle" for a single playing sound. Basically a wrapper around a FMOD channel.
// Returned by audio manager functions that start sound playback to allow the caller to query and modify the playing sound.
// After the sound stops (either plays all the way or is stopped prematurely), the handle is no longer valid.
// Even if the handle is not valid, it can still be passed around and used, but just will no longer have any effect.
class PlayingSoundHandle
{
public:
    PlayingSoundHandle() = default;
    PlayingSoundHandle(FMOD::Channel* channel);
    
    void Pause();
    void Resume();
    
    void SetVolume(float volume);
    bool IsPlaying() const;
    
private:
    // Allow audio manager to access channel directly, but not others.
    friend class AudioManager;
    
    // An FMOD "channel" represents a single playing sound. Channel* is returned by PlaySound.
    // Once returned, this pointer is always valid, even if the sound stops playing or the channel is reused.
    // In one of those scenarios, calls to channel functions start to return FMOD_ERR_INVALID_HANDLE or similar result codes.
    FMOD::Channel* channel = nullptr;
};

struct AudioSaveState
{
    std::vector<PlayingSoundHandle> playingSounds;
};

enum class AudioType
{
    SFX,
    VO,
    Ambient
};

class AudioManager
{
public:
    // If calling code doesn't provide specific min/max 3D dists, we'll use these.
    //TODO: values are pulled from GAME.CFG; maybe we should read in that file.
    static constexpr float kDefault3DMinDist = 200.0f;
    static constexpr float kDefault3DMaxDist = 2000.0f;
    
    bool Initialize();
    void Shutdown();
    
    void Update(float deltaTime);
    void UpdateListener(const Vector3& position, const Vector3& velocity, const Vector3& forward, const Vector3& up);
    
    PlayingSoundHandle PlaySFX(Audio* audio);
    PlayingSoundHandle PlaySFX3D(Audio* audio, const Vector3& position, float minDist = kDefault3DMinDist, float maxDist = kDefault3DMaxDist);
    
    PlayingSoundHandle PlayVO(Audio* audio);
    PlayingSoundHandle PlayVO3D(Audio* audio, const Vector3& position, float minDist = kDefault3DMinDist, float maxDist = kDefault3DMaxDist);
    
    PlayingSoundHandle PlayAmbient(Audio* audio, float fadeInTime = 0.0f);
    PlayingSoundHandle PlayAmbient3D(Audio* audio, const Vector3& position, float minDist = kDefault3DMinDist, float maxDist = kDefault3DMaxDist);

    void SwapAmbient();

    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    
    void SetVolume(AudioType audioType, float volume);
    float GetVolume(AudioType audioType) const;
    
    void SetMuted(bool mute);
    bool GetMuted();
    
    void SetMuted(AudioType audioType, bool mute);
    bool GetMuted(AudioType audioType);
    
    AudioSaveState SaveAudioState(bool includeAmbient);
    void RestoreAudioState(AudioSaveState& audioState);
    
private:
    // Underlying FMOD system - portal to audio greatness.
    FMOD::System* mSystem = nullptr;

    // Master channel group is the final output destination for all other channel groups.
    // Changes to this group affect all other groups.
    FMOD::ChannelGroup* mMasterChannelGroup = nullptr;

    // Channel group for SFX. Outputs to master channel group.
    FMOD::ChannelGroup* mSFXChannelGroup = nullptr;

    // Channel group for VO. Outputs to master channel group.
    FMOD::ChannelGroup* mVOChannelGroup = nullptr;

    // Channel group for ambient (music, background audio). Outputs to master channel group.
    FMOD::ChannelGroup* mAmbientChannelGroup = nullptr;

    // Two channel groups for ambient playback. These output to the ambient channel group.
    // We need these separate channel groups for two reasons:
    // 1) Support fade in/out of ambient separate from user-defined volume levels.
    // 2) Support cross-fading between ambient tracks when changing scenes.
    struct FadeChannelGroup
    {
        float fadeDuration = 0.0f;
        float fadeTimer = 0.0f;
        float fadeTo = 0.0f;
        float fadeFrom = 0.0f;
        FMOD::ChannelGroup* channelGroup = nullptr;

        void Update(float deltaTime);
        void SetFade(float fadeTime, float targetVolume, float startVolume = -1.0f);
    };
    FadeChannelGroup mAmbientFadeChannelGroups[2];
    int mCurrentAmbientIndex = 0;
    
    // Volume multipliers for each audio type. This changes the range of possible volumes for a particular type of audio.
    // For example, if 0.8 is used, it means that "max volume" for that audio type is actually 80% of the "true max".
    // This is just helpful for achieving a good sounding mix. In particular, music tends to overpower SFX/VO, so compensate for that.
    const float kSFXVolumeMultiplier = 1.0f;
    const float kVOVolumeMultiplier = 1.0f;
    const float kAmbientVolumeMultiplier = 0.75f;
    
    // Sounds that are currently playing.
    std::vector<PlayingSoundHandle> mPlayingSounds;
    
    FMOD::ChannelGroup* GetChannelGroupForAudioType(AudioType audioType) const;
    
    PlayingSoundHandle CreateAndPlaySound(const char* buffer, int bufferLength, AudioType audioType, bool is3D = false);
    PlayingSoundHandle CreateAndPlaySound3D(const char* buffer, int bufferLength, AudioType audioType, const Vector3& position, float minDist, float maxDist);
};
