#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include "common.h"
#include "sound_mixer.h"
#include "xaudio2_sound_mixer.h"

struct SoundManager {
    static SoundMixer *Get();
private:

    static XAudio2SoundMixer xaudio2;
}

#endif



