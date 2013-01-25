#!/usr/bin/env ruby
# get definitions and add to word list

require 'net/http'
require 'rexml/document'

STDERR.puts "Reading words..."
dictionary = {}
File.open("defs.txt") do |list|
	list.each_line do |line|
		word, defn = line.strip.split(/\s+/, 2)
		dictionary[word] = defn
	end
end

uri = URI('http://services.aonaware.com')
# open a connection to the dictionary service
http = Net::HTTP.start(uri.host, uri.port)

count = 0
limit = 1000

STDERR.puts "Downloading definitions..."
dictionary.each do |word, defn|
	if not defn
		if count < limit
			response = http.post('/DictService/DictService.asmx/Define', "word=#{word}")
			if response.is_a? Net::HTTPOK
				doc = REXML::Document.new(response.body)
				doc.elements.each('WordDefinition/Definitions/Definition') do |definition|
					if definition.elements['Dictionary/Id'].text == 'wn' # WordNet seems good
						# clean up text
						text = definition.elements['WordDefinition'].text.gsub(/\s+/, ' ')
						# try to extract first definition
						if text =~ /(\d+)?: ([A-Za-z ()'",.]+)/
							dictionary[word] = $2.strip
							STDERR.puts "#{word}: #{dictionary[word]}"
							count += 1
						else
							STDERR.puts "Unrecognized definition format:\n#{word} #{text}"
						end
					end
				end
			else
				STDERR.puts "Bad response when defining #{word}"
			end
		else
			break
		end
	end
end

STDERR.puts "Writing words..."
File.open("defs.txt", "w") do |list|
	dictionary.each do |word, defn|
		if defn
			list.puts "#{word} #{defn}"
		else
			list.puts word
		end
	end
end

http.finish
