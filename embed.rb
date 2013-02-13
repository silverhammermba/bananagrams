#!/usr/bin/env ruby
# convert dictionary.txt to an embeded C++ header

exit unless File.exists? 'dictionary.txt'

header = <<HPP
#include <map>
#include <string>

static const std::map<std::string, std::string> dictionary =
{
HPP

File.open('dictionary.txt') do |f|
	f.each_line do |line|
		s = line.split(' ', 2)
		s << "" if s.length == 1
		header << "\t{\"#{s[0]}\", #{s[1].strip.dump}},\n"
	end
end

header << "};\n"

File.open('dictionary.hpp', 'w') { |f| f.write(header) }
