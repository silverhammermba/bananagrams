$dlls = {}
$missing = {}

def get_dlls file
  `x86_64-w64-mingw32-objdump -x #{file}`.each_line do |line|
    if line =~ /DLL Name: (.*)/
      dll = $1
      # find actual location of DLL
      unless File.exists? dll
        lib = File.join('/usr/x86_64-w64-mingw32/bin', dll)
        if File.exists? lib
          dll = lib
        else
          $missing[dll] = true
          next
        end
      end

      unless $dlls[dll]
        $dlls[dll] = true
        get_dlls dll
      end
    end
  end
end

ARGV.each do |arg|
  get_dlls arg
end

puts $dlls.keys.sort
STDERR.puts "Missing DLLS:", $missing.keys.sort
