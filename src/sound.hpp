#ifndef SOUND_HPP
#define SOUND_HPP

class SoundManager
{
	// is there a nice way to map to buffer/sound pairs?
	std::unordered_map<std::string, sf::SoundBuffer*> buffers;
	std::unordered_map<std::string, sf::Sound*> sounds;

	public:

	SoundManager()
	{
	}

	~SoundManager()
	{
		for (auto& pair : sounds)
		{
			delete pair.second;
			delete buffers[pair.first];
		}
	}

	// load a sound, return indicates if it was not yet loaded
	bool load(const std::string& filename)
	{
		if (sounds.count(filename))
		{
			return false;
		}
		else
		{
			auto buffer = new sf::SoundBuffer();
			auto sound = new sf::Sound();

			buffer->loadFromFile(filename);
			sound->setBuffer(*buffer);

			buffers[filename] = buffer;
			sounds[filename] = sound;

			return true;
		}
	}

	void play(const std::string& filename)
	{
		if (!sounds.count(filename))
			load(filename);

		sounds[filename]->play();
	}
};

#endif
