#ifndef AUDIO_H
#define AUDIO_H
#include "SDL2/SDL_mixer.h"

namespace App
{
	namespace Audio
	{
		enum class BuiltInSounds
		{
			SoundName1
		};

		enum class Soundtrack
		{
			Soundtrack1
		};

		class AudioPlayer
		{
		public:
			AudioPlayer();

			bool Init();

			void Play(BuiltInSounds), Play(Soundtrack);

			void StopMusic();
		};
	}
}

#endif