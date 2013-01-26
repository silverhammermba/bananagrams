#!/usr/bin/env ruby
# get definitions and add to word list

unless File.exist? 'dictionary.txt'
	STDOUT.puts "Creating dictionary..."
	require 'fileutils'
	FileUtils.copy('words.txt', 'dictionary.txt')
end

STDOUT.puts "Reading words..."
dictionary = {}
File.open("dictionary.txt") do |list|
	list.each_line do |line|
		word, defn = line.strip.split(/\s+/, 2)
		dictionary[word] = defn
	end
end

total = dictionary.size

STDOUT.puts "Looking up words..."
dictionary.each.with_index do |pair, i|
	if not pair[1]
		defs = []
		`wn #{pair[0]} -over`.each_line do |line|
			if line =~ /\d+\. (.*) -- \(([^;]+).*\)/
				unless $1.split(?,).any? { |w| w != w.downcase }
					defs << $2.strip
				end
			end
		end
		unless defs.empty?
			dictionary[pair[0]] = defs.min_by(&:length)
		end
	end
	if rand < 0.01
		STDERR.print "\r#{i}/#{total}"
	end
end
STDERR.puts

STDOUT.puts "Writing words..."
File.open("dictionary.txt", "w") do |list|
	dictionary.each do |word, defn|
		if defn
			list.puts "#{word} #{defn}"
		else
			list.puts word
		end
	end
end

