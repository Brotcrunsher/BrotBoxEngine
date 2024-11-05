#include "BBE/Microphone.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include "BBE/SimpleThread.h"

void bbe::Microphone::recordingMain()
{
    ALCdevice* mic = alcCaptureOpenDevice(nullptr, 44100, AL_FORMAT_MONO_FLOAT32, 4096);
    if (!mic)
    {
        bbe::Crash(bbe::Error::IllegalState, "Could not open mic!");
    }

    alcCaptureStart(mic);

    while (!stopRequested)
    {
        ALCint sampleNum = 0;
        alcGetIntegerv(mic, ALC_CAPTURE_SAMPLES, 1, &sampleNum);

        if (sampleNum > 0)
        {
            alcCaptureSamples(mic, samples.addRaw(sampleNum), sampleNum);
        }
        std::this_thread::yield();
    }

    alcCaptureStop(mic);
    alcCaptureCloseDevice(mic);
}

bbe::Microphone::~Microphone()
{
    if(isRecording()) stopRecording();
}

void bbe::Microphone::startRecording()
{
    if (isRecording()) bbe::Crash(bbe::Error::IllegalState, "Already recording!");

    recording = true;

    thread = std::thread(&bbe::Microphone::recordingMain, this);
    bbe::simpleThread::setName(thread, "BBE Microphone");
}

bbe::Sound bbe::Microphone::stopRecording()
{
    if (!isRecording()) bbe::Crash(bbe::Error::IllegalState, "Not recording!");

    stopRequested = true;
    thread.join();
    stopRequested = false;
    recording = false;

    bbe::Sound sound;
    sound.load(std::move(samples));
    samples.clear();

    return sound;
}

bool bbe::Microphone::isRecording() const
{
    return recording;
}
