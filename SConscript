# Initialize build environment
env = Environment()

# Customize build settings
env.Append(CPPFLAGS = '-Wall -g -O3 -std=c++11 -mavx')
env.Append(CPPPATH = ['src/include', '/usr/include'])
#env.Append(LIBPATH = ['/usr/lib'])
#env.Append(LIBS = ['libc'])
#env.Append(LINKFLAGS = ['-Wl,--rpath,/usr/local/lib'])

# Get a list of all C++ source files
sources = [
	Glob('src/*.cpp'), 
	Glob('src/win/*.cpp'),
	Glob('src/x86_64/*.cpp')
]

headers = [
	Glob('include/*.h'),
	Glob('include/win/*.h'),
	Glob('include/x86_64/*.h')
]


defaultBuild = env.Program(target = 'xmem', source = sources)

Default(defaultBuild)
