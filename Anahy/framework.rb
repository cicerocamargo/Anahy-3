# A MESSY FRAMEWORK TO CREATE CLASS FILES WITH
# STATIC, READ&WRITE, AND READ-ONLY CLASS MEMBERS
# AND GETTERS AND SETTERS
# command_line example: 
# 	ruby cppNew.rb class ClassName static{ int:x, char:c } rw{ int:y, void*:d } r{ int:z }

def type(str)
	array = str.split("#")
	token = ""
	for i in 0..(array.size-2) do
		token << array[i]
	end
	#puts token
	return token
end

def var(str)
	array = str.split("#")
	token = array[array.size-1]
	#puts token
	return token
end

def getter_declaration(type, var)
	"#{type} get_#{var}();\n"
end

def setter_declaration(type, var)
	"void set_#{var}(#{type} new_value);\n"
end

def getter_definition(type, var, class_name)
	str = ""
	str << "#{type} #{class_name}::get_#{var}() {\n"
	str << "\treturn #{var};\n"
	str << "\}\n"
end

def setter_definition(type, var, class_name)
	str = ""
	str << "void #{class_name}::set_#{var}(#{type} new_value) {\n"
	str << "\t#{var} = new_value;\n"
	str << "\}\n"
end

def make_arg_list(rw,r)
	str = ""
	rw.each{|i| str << "#{type(i)} _#{var(i)}, " }
	r.each{|i| str << "#{type(i)} _#{var(i)}, " }
	if str != ""
		size = str.size
		str[size-2..size-1] = "" # remove the last ", "
	end
	return str
end

def make_class_declaration_file(label, static, rw, r)
	$declaration_file.print "#ifndef #{label.upcase}_H\n"
	$declaration_file.print "#define #{label.upcase}_H\n\n"
	
	$declaration_file.print "class #{label} {\n\n"
	#private mebers here
	static.each {|i| $declaration_file.print "\tstatic " + type(i) + " " + var(i) + ";\n" }
	rw.each {|i| $declaration_file.print "\t" + type(i) + " " + var(i) + ";\n" }
	r.each {|i| $declaration_file.print "\t" + type(i) + " " + var(i) + ";\n" }
	
	$declaration_file.print "\npublic:\n" 
	$declaration_file.print "\t#{label}(); // default constructor\n" 
	$declaration_file.print "\t#{label}(#{label}&); // copy-constructor\n"
	$declaration_file.print "\t#{label}(#{make_arg_list(rw,r)});\n" # custom constructor
	$declaration_file.print "\tvirtual ~#{label}();\n" # destructor
	
	$declaration_file.print "\n\t/* getters and setters */\n"
	rw.each { |i|
		$declaration_file.print "\t" + getter_declaration(type(i), var(i))
		$declaration_file.print "\t" + setter_declaration(type(i), var(i))
	}
	r.each { |i| $declaration_file.print "\t" + getter_declaration(type(i), var(i)) }
	
	$declaration_file.print "};\n\n"
	$declaration_file.print "#endif"	
end

def make_class_definiton_file(label, static, rw, r)
	$definition_file.print "#include \"#{label}.h\"\n"
	$definition_file.print "#define INIT_VALUE 0 /* <-- CHANGE THE PLACES WHERE THIS APPEARS! */\n\n"
	
	$definition_file.print "/* STATIC MEMBERS' ITIALIZATIONS */\n"
	static.each {|i| $definition_file.print "#{type(i)} #{label}::#{var(i)} = INIT_VALUE;\n" }
	
	$definition_file.print "\n/* PRIVATE METHODS' DEFINITIONS */\n\n\n"
	$definition_file.print "\n/* PUBLIC METHODS' DEFINITIONS */\n"
	$definition_file.print "#{label}::#{label}() {\n\t\n}\n\n" # default constructor
	$definition_file.print "#{label}::#{label}(#{label}& #{label.downcase}_obj) {\n\t\n}\n\n" # copy-constructor
	$definition_file.print "#{label}::#{label}(#{make_arg_list(rw, r)}) {\n\t\n}\n\n" #constructor
	$definition_file.print "#{label}::~#{label}() {\n\t\n}\n" # destructor
	
	$definition_file.print "\n/* getters and setters */"
	rw.each { |i|
		$definition_file.print "\n" + getter_definition(type(i), var(i), label)
		$definition_file.print "\n" + setter_definition(type(i), var(i), label)
	}
	r.each { |i| $definition_file.print "\n" + getter_definition(type(i), var(i), label) }
end

def build_members_array(array, it)
	it+=1
	str = ""
	while ARGV[it] != "}" do
		str << ARGV[it] << "#"
		it+=1
	end
	array.replace( str.split(";") )
	for i in array do
		i.sub!(/\A#/,"") # removes an initial '#'
		i.sub!(/#\Z/,"") # removes a final '#'
	end

	# array's elements are strings with tokens separated by a '#'
	# where the last token is the var name, and the remainder is the type
	return it+1
end

def make_class_files(label)
	it = 2 # an iterator
	static = []
	rw = []
	r = []
	while ARGV[it] != nil do
		case ARGV[it]
			when "static{":
				it = build_members_array(static, it)
			when "rw{":
				it = build_members_array(rw, it)
			when "r{":
				it = build_members_array(r, it)
			else
				puts "Unknown option: #{ARGV[it]}"
				return
		end
	end
	
#	puts static.to_s
#	puts rw
#	puts r
	
	make_class_definiton_file(label, static, rw, r)
	make_class_declaration_file(label, static, rw, r)
end

# MAIN

if ARGV[0] != nil && ARGV[1] != nil
	construct = ARGV[0].downcase
	label = ARGV[1]
end

$declaration_file = File.new(label + ".h", "w+")
$definition_file = File.new(label + ".cpp", "w+")

case construct
	when "class": make_class_files(label)		
	else puts "não sei o que fazer..."
end

# PRINT THE GENERATED FILES

puts "\n\t\tArquivo " + label + ".h\n\n"
$declaration_file.rewind
while line = $declaration_file.gets  
	puts line  
end

puts "\n\n\t\tArquivo " + label + ".cpp\n\n"
$definition_file.rewind  
while line = $definition_file.gets  
	puts line  
end


$declaration_file.close
$definition_file.close
