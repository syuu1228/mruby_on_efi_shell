
class Dir
  def each(&block)
    while s = self.read
      block.call(s)
    end
    self
  end

  def self.entries(path)
    a = []
      self.open(path) { |d|
      while s = d.read
        a << s
      end
    }
    a
  end

  def self.foreach(path, &block)
    if block
      self.open(path).each { |f| block.call(f) }
    else
      self.open(path).each
    end
  end

  def self.open(path, &block)
    if block
      d = self.new(path)
      begin
        block.call(d)
      end
    else
      self.new(path)
    end
  end
end

base_dir = "kernels"
kernel_options = "root=/dev/sda3"
kernels = []

Dir.foreach(base_dir) do |ent|
	next if ent == "." || ent == ".."
	kernels << ent
end

puts "** Boot Menu **"
(0...kernels.length).each do |i|
	puts "[#{i}] #{kernels[i]}"
end
loop do
		puts "Input number:"
		key = Shell.gets.chomp
		idx = key.to_i
		if kernels.length <= idx
			next
		end
		puts "#{base_dir}\\#{kernels[idx]} #{kernel_options}"
		Shell.exec "#{base_dir}\\#{kernels[idx]} #{kernel_options}"
		break
end
