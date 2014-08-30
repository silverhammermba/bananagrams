void Server::save(const std::string& filename)
{
	std::ofstream save_file(filename);

	// save game parameters
	save_file.write(dict_filename.c_str(), dict_filename.length() + 1);
	save_file.write(reinterpret_cast<const char*>(&num), sizeof num);
	save_file.write(reinterpret_cast<const char*>(&den), sizeof den);

	// save hand
	for (char ch = 'A'; ch <= 'Z'; ++ch)
	{
		if (hand.has_any(ch))
		{
			save_file.put(ch);
			uint8_t count = hand.count(ch);
			save_file.write(reinterpret_cast<const char*>(&count), sizeof count);
		}
	}

	save_file.put('\0');

	// save grid
	for (Tile* tile : grid.internal())
	{
		if (tile != nullptr)
		{
			auto pos = tile->get_grid_pos();
			save_file.put(tile->ch());
			save_file.write(reinterpret_cast<const char*>(&pos.x), sizeof pos.x);
			save_file.write(reinterpret_cast<const char*>(&pos.y), sizeof pos.y);
		}
	}

	save_file.close();
}
