#include "Audio.h"
#include "spdlog/spdlog.h"

App::Audio::AudioPlayer::AudioPlayer()
{

}

bool App::Audio::AudioPlayer::Init()
{
	spdlog::debug("AudioPlayer::Init()");

	//load audio here for now
	

	spdlog::trace("AudioPlayer::Init() success");
	return true;
}

void App::Audio::AudioPlayer::Play(BuiltInSounds Sound)
{
	Mix_Chunk* Chunk;

	switch (Sound)
	{
	default:
		return;
	}

	Mix_PlayChannel(-1, Chunk, 0);
}

void App::Audio::AudioPlayer::Play(Soundtrack Track)
{
	Mix_Music* Chunk;

	switch (Track)
	{
	
	default:
		return;
	}

	Mix_PlayMusic(Chunk, 999);
}

void App::Audio::AudioPlayer::StopMusic()
{
	Mix_FadeOutMusic(200);
}
