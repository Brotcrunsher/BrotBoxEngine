#pragma once

#include <thread>
#include <mutex>

#include "../BBE/Sound.h"

namespace bbe
{
    class Microphone
    {
    private:
        bool recording = false;
        std::atomic_bool stopRequested = false;

        
        bbe::List<float> samples;
        std::thread thread;

        void recordingMain();

    public:
        Microphone() = default;
        ~Microphone();

        Microphone(const Microphone& other) = delete;
        Microphone(Microphone&& other) = delete;
        Microphone& operator=(const Microphone& other) = delete;
        Microphone& operator=(Microphone&& other) = delete;

        void startRecording();
        bbe::Sound stopRecording();

        bool isRecording() const;
    };
}
