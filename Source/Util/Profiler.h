#pragma once
#include <cstdint>
#include <vector>

class Profiler;
class ScopedProfiler;

//#define PROFILER_ENABLED

#if defined(PROFILER_ENABLED)
    #define PROFILER_BEGIN_FRAME() Profiler::BeginFrame();
    #define PROFILER_END_FRAME() Profiler::EndFrame();
    #define PROFILER_BEGIN_SAMPLE(x) Profiler::BeginSample(x);
    #define PROFILER_END_SAMPLE() Profiler::EndSample();
    #define PROFILER_SCOPED(x) ScopedProfiler x(#x);
#else
    #define PROFILER_BEGIN_FRAME()
    #define PROFILER_END_FRAME()
    #define PROFILER_BEGIN_SAMPLE(x)
    #define PROFILER_END_SAMPLE()
    #define PROFILER_SCOPED(x)
#endif

// These defines are ALWAYS available.
// Sometimes we want to time things even when not all profiling tools are enabled.
#define TIMER_SCOPED(x) Sample timerScopedSample(x);

// Core time tracker: returns amount of time since creation.
class Stopwatch
{
public:
    Stopwatch();
    double GetMilliseconds() const;

private:
    uint64_t mStartCounter = 0L;
};

// Named segment of time to track.
class Sample
{
public:
    Sample(const char* name);
    ~Sample();

private:
    const char* mName;
    Stopwatch mTimer;
};

class Profiler
{
public:
    static void BeginFrame();
    static void EndFrame();

    static void BeginSample(const char* name);
    static void EndSample();

private:
    // Counts what frame we're on.
    static uint64_t sFrameNumber;

    // Samples that are still in progress.
    static std::vector<Sample> sActiveSamples;
};

// Small class that just handles calling BeginSample/EndSample.
// Just create at start of function and you are set.
class ScopedProfiler
{
public:
    ScopedProfiler(const char* name) { Profiler::BeginSample(name); }
    ~ScopedProfiler() { Profiler::EndSample(); }
};