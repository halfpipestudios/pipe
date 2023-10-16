#include "xaudio2_sound_mixer.h"
#include "memory_manager.h"

/* -------------------------------------------------- */
/*     XAudio Playing Sound   implementation          */
/* -------------------------------------------------- */

void XAudio2PlayingSound::Play(f32 volume, i32 count) {
}

void XAudio2PlayingSound::Play3DSound(SoundPosition listener, SoundPosition emiter) {
}

void XAudio2PlayingSound::Update3DSound(SoundPosition listener, SoundPosition emiter) {
}

void XAudio2PlayingSound::Stop() {
}

void XAudio2PlayingSound::IsPlaying() {
}

void XAudio2PlayingSound::SetVolume() {
}

/* -------------------------------------------------- */
/*     XAudio Sound Mixer implementation              */
/* -------------------------------------------------- */

void XAudio2SoundMixer::Initialize() {
}

void XAudio2SoundMixer::Terminate() {
}

void XAudio2SoundMixer::LoadSound(char *path) {
}

PlayingSound *XAudio2SoundMixer::AllocPlayingSound(char *name) {
    return nullptr;
}

void XAudio2SoundMixer::RemovePlayingSound(PlayingSound *sound) {
}
