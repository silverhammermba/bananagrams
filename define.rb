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
			# parse words and definitions
			if line =~ /^\d+\. (\(\d+\) )?([^-]+) -- \(([^;:]+).*\)/
				# if no proper Nouns
				if $2.downcase == $2
					defs << $3.strip
				end
			end
		end
		unless defs.empty?
			norep = defs.reject { |defn| defn.downcase =~ /\b#{pair[0]}\b/ }
			dictionary[pair[0]] = (norep.empty? ? defs : norep).first
		end
	end
	if rand < 0.01
		STDERR.print "\r#{i}/#{total}"
	end
end
STDERR.puts "\r#{total}/#{total}"

STDOUT.puts "Writing words..."
File.open("dictionary.txt", "w") do |list|
	dictionary.each do |word, defn|
		if defn
			list.puts "#{word.upcase} #{defn}"
		else
			list.puts word.upcase
		end
	end
end

