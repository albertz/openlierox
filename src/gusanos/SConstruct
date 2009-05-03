import re
import os
import time
import types
import UserList

exp = Split('env')
sconscript = ['GUI',
			'Utility/util',
            'Console',
            'loadpng',
            'Goop',
            'OmfgScript',
            'liero2gus',
            #'lua',
			'lua51',
            #'panzer',
            'lighter',
            'http',
            #'IoVM',
			#'SmallLang',
			#'netlib',
			#'filetransfer',
			]
			
def is_List(e):
    return type(e) is types.ListType or isinstance(e, UserList.UserList)

class MyEnv(Environment):
	sourcePattern = re.compile('\.(cpp|c)$')
	
	def __init__(self):
		Environment.__init__(self)
		
		self.conf = ARGUMENTS.get('conf', 'posix')
		self.build = ARGUMENTS.get('build', 'release')
		self.subfolder = os.path.join(self.conf, self.build)
		self.noParsers = ARGUMENTS.get('no-parsers', False)
		
	def confLibs(self, l):
		return [self['LIB__' + lib] for lib in Split(l)]
		
	def getLibName(self, lib):
		return os.path.join('#lib', self.subfolder, lib)
		
	def getBinName(self, bin):
		if self.conf == 'mingw-cross':
			bin += '.exe'
		return os.path.join('#bin', self.conf, bin) # Put executable here so not to confuse
	
	#Builds all cpp and c files in this directory and returns a list of nodes
	def getObjects(self, dir='.'):
		if is_List(dir):
			l = []
			for d in dir:
				l += self.getObjects(d)
			return l
			
		#sourcePattern = re.compile('\.(cpp|c)$')
	
		buildDir = os.path.join(dir, '.build', self.subfolder)
		self.BuildDir(buildDir, dir, duplicate = 0)
		
		return [self.Object(os.path.join(buildDir, i))
				for i in os.listdir(dir)
					if MyEnv.sourcePattern.search(i)]
class MyConf(Configure):
	def __init__(self, env):
		Configure.__init__(self, env)
		self.d = "Import('env')\n"
		self.cpppaths = []
		self.libpaths = []
		
	def addLib(self, lib, val):
		self.d += "env['LIB__%s'] = '%s'\n" % (lib, val)
		env['LIB__' + lib] = val
		
	def addCPPPath(self, path):
		if path not in self.cpppaths:
			self.cpppaths.append(path)
			
	def addLibPath(self, path):
		if path not in self.libpaths:
			self.libpaths.append(path)
		
	def checkBoostLib(self, lib):
		for s in ['-gcc', '-gcc-mt', '-mgw']:
			if self.CheckLib(lib + s, language = 'C++'):
				self.addLib(lib, lib + s)
				return
				
		for s in ['-gcc', '-gcc-mt', '-mgw']:
			for prefix in ['/usr/lib', '/usr/local/lib', '/home/glip/liboob']:
				for suffix in ['.so', '.a']:
					path = os.path.join(prefix, 'lib' + lib + s + suffix)
					if os.path.exists(path):
						
						self.addLibPath(prefix)
						self.addLib(lib, lib + s)
						print "Found", lib, "in", prefix
						return
				
		print "Could not locate boost library", lib
		Exit(1)
		
	def checkLibs(self, libname, libs):
		for lib in libs:
			if self.CheckLib(lib, language = 'C++'):
				self.addLib(libname, lib)
				return
		
		for lib in libs:
			for prefix in ['/usr/lib', '/usr/local/lib']:
				for suffix in ['.so', '.a']:
					path = os.path.join(prefix, 'lib' + lib + suffix)
					if os.path.exists(path):
						
						self.addLibPath(prefix)
						self.addLib(libname, lib)
						print "Found", libname, "in", prefix
						return
					
		print "Could not locate library", libname, "or any variant"
		Exit(1)
		
	def checkHeader(self, header, altsuffix = []):
		if self.CheckCXXHeader(header):
			return
		for prefix in ['/usr/include', '/usr/local/include']:
			for suffix in [''] + altsuffix:
				#if self.CheckCXXHeader(os.path.join(prefix, suffix, header)):
				if os.path.exists(os.path.join(prefix, suffix, header)):
					path = os.path.join(prefix, suffix)
					env.Append(CPPPATH = path)
					self.addCPPPath(path)
					print "Found", header, "in", path
					return
					
		print "Could not locate header", header
		Exit(1)
		
	def write(self, name):
		for path in self.cpppaths:
			self.d += "env.Append(CPPPATH = ['%s'])\n" % path
			self.env.Append(CPPPATH = path)
		for path in self.libpaths:
			self.d += "env.Append(LIBPATH = ['%s'])\n" % path
			self.env.Append(LIBPATH = path)
		self.d += "Return('env')\n"
		f = open(name, 'w')
		f.write(self.d)
		f.close()
		return self.Finish()
		
env = MyEnv()

env.Append(
	CPPPATH = Split('. #http #loadpng #lua51 #Console #GUI #Utility #OmfgScript'),
	LIBPATH = [os.path.join('#lib', env.subfolder), os.path.join('#lib', env.conf)],
	CPPFLAGS = Split('-pipe -Wall -Wno-reorder')
)

if env.build == 'release':
	env.Append(CPPFLAGS = Split('-O3 -g -DNDEBUG -fomit-frame-pointer'))
elif env.build == 'debug':
	env.Append(CPPFLAGS = Split('-O0 -g -DDEBUG -DMAP_DOWNLOADING -DLOG_RUNTIME'))
elif env.build == 'dedserv':
	env.Append(CPPFLAGS = Split('-O3 -g -DNDEBUG -DDEDSERV -fomit-frame-pointer'))
elif env.build == 'dedserv-debug':
	env.Append(CPPFLAGS = Split('-O0 -g -DDEBUG -DDEDSERV -DLOG_RUNTIME'))

if env.conf == 'mingw-cross':
	mingwPath = ARGUMENTS.get('mingw-path', '/usr/local/mingw/')
	env.Append(
		CPPFLAGS = ['-DWINDOWS'],
		LIBS = ['stdc++']
		)
	env.Tool('crossmingw', ['SCons/Tools'])

if env.conf == 'lucas':
	env.Replace(
		CXX = 'g++-3.4',
	)
	
# Check if we need to configure

configName = 'Config-' + env.conf

#print env['LIBS']
if not os.path.exists(configName):
	
	print 'Configuring... '
	config = MyConf(env)
	config.checkBoostLib('boost_filesystem')
	config.checkBoostLib('boost_signals')
	config.checkLibs('fmod', ['fmod-3.74.1', 'fmod-3.74', 'fmod'])
	config.checkLibs('png', ['png13', 'png12', 'png'])
	config.checkLibs('zoidcom', ['zoidcom', 'zoidcom_mw'])
	config.checkLibs('z', ['z'])
	config.checkHeader('zoidcom.h', ['zoidcom'])
	config.checkHeader('fmod.h', ['fmod'])
	config.checkHeader('png.h', ['png'])
	config.checkHeader('zlib.h', ['zlib'])
	config.checkHeader('boost/utility.hpp', ['boost-1_33_1', 'boost-1_33', 'boost-1_32'])
	
	env = config.write(configName)
else:
	env = SConscript(configName, exports = exp)

# Build parser generator
parserGen = SConscript('parsergen/SConscript', exports = exp)

def parserGenEmitter(target, source, env):
	env.Depends(target, parserGen)
	return (target, source)
	
def parserBuilderFunc(target, source, env):
	os.system('%s %s %s' % (parserGen[0].abspath, str(source[0]), str(target[0]) + '.re'))
	os.system('re2c %s > %s' % (str(target[0]) + '.re', str(target[0])))
	return None
	
parserBuilder = Builder(action = parserBuilderFunc,
	emitter = parserGenEmitter,
	suffix = '.h', src_suffix = '.pg')
	
env['BUILDERS']['Parser'] = parserBuilder

# Build the rest
for i in sconscript:
	SConscript(i + '/SConscript', exports = exp)
            
if env.conf == 'mingw-cross':
	env.Append(BUILDERS = {'Strip' : Builder(action = os.path.join(mingwPath, 'bin', 'strip') + ' $SOURCE')})
	gus = env.Install('/usr/local/htdocs/stuff/gusanos-alpha', env.getBinName('gusanos'))
	env.Strip(gus)
	env.Alias('upload', '/usr/local/htdocs/stuff/gusanos-alpha')