import re
import os
import string
import sys

classes = {}
funcs = []

def openDocFile(name):
	f = file(os.path.join(pathToDocs, name.lower() + '.txt'), 'w')
	return f

def closeDocFile(f):
	f.close()

def makeLink(to, of):
	return '[[' + to.lower() + '|' + of + ']]'

def makeFuncLink(to, of = None):
	funcName = to.abbrev()
	if of == None:
		of = funcName
		if to.owner.name != 'global':
			of = to.owner.name + ':' + funcName
	return '[[' + to.owner.name.lower() + '#' + funcName + '|' + of + ']]'
	
class Func:
	def __init__(self, name, owner, version):
		self.name = name
		self.lines = []
		self.owner = owner
		self.literalMode = False
		self.version = version
		
	def addLine(self, text):
		t = text[1:-1]

		self.lines.append(t)
			
	def checkForReferences(self):
		for l in self.lines:
			for word in l.split(' '):
				if classes.has_key(word):
					classes[word].addReference(self)
					
	def abbrev(self):
		return self.name.split('(', 1)[0]
		
	def output(self, f):
		funcName = self.abbrev()
		f.write('===== ' + funcName + ' =====\n')
		if self.version != None:
			f.write('(available from version ' + self.version + ')\n')
		f.write('\n  ' + self.name + '\n\n')
		for l in self.lines:
			f.write(self.owner.linkify(l) + '\n')
		f.write('\n')
	
class Class:
	def __init__(self, name):
		self.name = name
		self.funcs = {}
		self.refs = []
		self.inherit = None
	
	def addFunc(self, func):
		self.funcs[func.abbrev()] = func
		
	def output(self):
		f = openDocFile(self.name)
		if self.name != 'global':
			f.write('====== Class ' + self.name + ' ======\n\n')
			if self.inherit != None:
				f.write('(inherits from ' + makeLink(self.inherit, self.inherit) + ')')
			f.write(' ')
		else:
			f.write('====== Global functions ======\n\n')
		if len(self.refs) > 0:
			f.write('===== See also: =====\n\n')
			seeAlsoLines = []
			for ref in self.refs:
				seeAlsoLines.append(makeFuncLink(ref))
			f.write(string.join(seeAlsoLines, ', '))
			f.write('\n\n')
		funcList = self.funcs.values()
		funcList.sort(lambda x, y: cmp(x.name.lower(), y.name.lower()))

		for func in funcList:
			func.output(f)
		closeDocFile(f)
		
	def linkify(self, s):
		newWords = []
		for word in s.split(' '):
			if classes.has_key(word):
				newWords.append(makeLink(word, word))
			elif self.funcs.has_key(word):
				newWords.append(makeFuncLink(self.funcs[word], word))
			else:
				newWords.append(word)
				
		return string.join(newWords, ' ')
		
	def addReference(self, ref):
		self.refs.append(ref)
	
	def inheritsFrom(self, c):
		self.inherit = c
		
def getClass(name):
	if classes.has_key(name):
		return classes[name]
	else:
		newClass = Class(name)
		classes[name] = newClass
		return newClass

	
def processFile(f):

	beginre = re.compile('^\/\*\!\s*(.+)\s*')
	beginSre = re.compile('^//\!\s*(.+)\s*')
	inheritse = re.compile('([^\s]*)\s*inherits\s*([^\s]*)')
	versionre = re.compile('version\s*([^\s]*)')
	endre = re.compile('^\*\/')
	
	curFunc = None
	version = None
	
	for line in f.readlines():
		if curFunc == None:
			m = beginre.match(line)
			if m:
				func = m.group(1)
				parts = func.split(':', 1)
				if len(parts) > 1:
					#Class
					c = getClass(parts[0])
					curFunc = Func(parts[1], c, version)
					c.addFunc(curFunc)
					funcs.append(curFunc)
				else:
					c = getClass("global")
					curFunc = Func(parts[0], c, version)
					c.addFunc(curFunc)
					funcs.append(curFunc)
				continue
			
			m = beginSre.match(line)
			if m:
				l = m.group(1)
				m = inheritse.match(l)
				if m:
					getClass(m.group(1)).inheritsFrom(m.group(2))
				m = versionre.match(l)
				if m:
					version = m.group(1)
					if version == "any":
						version = None
						
		else:
			if endre.match(line):
				curFunc = None
			else:
				curFunc.addLine(line)
				
def generate():

	for func in funcs:
		func.checkForReferences()
				
	for c in classes.itervalues():
		c.output()
	f = openDocFile('classes')
	f.write('====== Class reference ======\n\n')
	f.write('  * [[global|Global functions]]\n\n')
	
	classList = classes.values()
	classList.sort(lambda x, y: cmp(x.name.lower(), y.name.lower()))
	
	for c in classList:
		if c.name != 'global':
			f.write('  * ' + makeLink(c.name, c.name) + '\n')

	closeDocFile(f)
	
sourcePattern = re.compile('\.(cpp|c)$')
	
for i in os.listdir('.'):
	if sourcePattern.search(i):
		processFile(file(i))
		
pathToDocs = sys.argv[1]

try:
	os.makedirs(pathToDocs)
except:
	pass


generate()
