#!/usr/bin/env ruby
# get definitions and add to word list

begin
  `wn`
rescue Errno::ENOENT
  $stderr.puts "WordNet not found: can't lookup definitions"
  exit 1
end

# normalized score val between min and max, flip if you want it low
def sc val, min, max, flip = false
  return 1.0 if min == max
  s = (val - min) / (max - min).to_f
  flip ? 1.0 - s : s
end

# look up a good definition using WordNet
def define word
  over = `wn #{word} -over`
  return nil if over.empty?

  definitions = {}

  type = nil
  over.each_line do |line|
    if line =~ /^Overview of (\w+)/
      type = $1
      next
    end

    syns, defn = line.split(' -- ', 2)
    next unless defn

    syns = syns[/^\d+\. (?:\(\d+\))?(.*)/, 1].strip.split(', ')
    defn = defn[/^\((.*)\)/, 1].split('; ')

    # get rid of proper nouns and acronyms
    next if syns.any? { |s| s =~ /[A-Z]/ }

    # get rid of usage examples
    defn.reject! { |d| d =~ /^"/ }
    next if defn.empty?

    definitions[type] ||= []
    definitions[type] << [syns, defn]
  end

  return nil if definitions.empty?

  # get bounds to normalize scores
  max_ctg = definitions.values.map(&:size).max
  min_ctg = definitions.values.map(&:size).min

  max_syn = definitions.values.map { |pairs| pairs.map { |syns, defns| syns.size }.max }.max
  min_syn = definitions.values.map { |pairs| pairs.map { |syns, defns| syns.size }.min }.min

  max_def = definitions.values.map { |pairs| pairs.map { |syns, defns| defns.map(&:size).max }.max }.max
  min_def = definitions.values.map { |pairs| pairs.map { |syns, defns| defns.map(&:size).min }.min }.min

  max_score = -1.0
  definition = nil

  # we want definitions in a large part-of-speech category, with many synonyms, and a short definition not using the defined word
  definitions.each do |pos, pairs|
    pair_sc = sc(pairs.size, min_ctg, max_ctg)
    pairs.each do |syns, defns|
      syn_sc = sc(syns.size, min_syn, max_syn)
      defns.each do |defn|
        score = pair_sc + syn_sc + sc(defn.size, min_def, max_def, true) + (defn =~ /\b#{word}\b/ ? 0.0 : 1.0)

        if score > max_score
          max_score = score
          definition = defn
        end
      end
    end
  end

  definition
end

$stderr.puts "Looking up words..."

$stdin.each_line.with_index do |line, i|
  word, defn = line.strip.split(/\s+/, 2)
  unless defn
    defn = define(word)
  end
  $stderr.puts "\rDefined #{i + 1} words..."
  $stdout.puts [word, defn].join(' ').strip
end
