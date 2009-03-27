#!/usr/bin/python

import os
import sys
import StringIO
import gzip
import getopt

os.chdir(os.path.dirname(sys.argv[0]) + "/../..")

startrev = 2 # excluding initial commit, that's not our work :)

try:
	opts, args = getopt.getopt(sys.argv[1:], "r:", ["rev="])
except getopt.GetoptError, err:
	# print help information and exit:
	print str(err) # will print something like "option -a not recognized"
	usage()
	sys.exit(2)
for o, a in opts:
	if o == "-r":
		startrev = int(a)
	

def svnInfo(revision):
	fullInfo = dict()
	for l in os.popen("svn info -r 2").read().splitlines():
		if len(l) == 0: continue
		node,value = l.split(": ")
		fullInfo[node] = value
	return (fullInfo["Last Changed Author"], fullInfo["Last Changed Date"])

def analyseData(preprint, data):
	print preprint, "size:", len(data)
	s = StringIO.StringIO()
	g = gzip.GzipFile(fileobj=s, mode="wb")
	gzipheadersize = s.len
	g.write(data)
	g.close()
	print preprint, "compressed size:", s.len
	print preprint, "compress rate:", len(data) // s.len

def analyseDiff(diffOut):
	addedLines = str()
	removedLines = str()
	for l in diffOut.splitlines():
		if len(l) == 0: continue
		if l.startswith("+"):
			addedLines += l[1:] + "\n"
		if l.startswith("-"):
			removedLines += l[1:] + "\n"
	analyseData(" added:", addedLines)
	analyseData(" removed:", removedLines)

def analyseSvnRev(rev):
	print "rev", rev, ":", svnInfo(rev)
	diffOut = os.popen("svn diff -r %i:%i -x --ignore-eol-style" % (rev - 1, rev)).read()
	analyseDiff(diffOut)
	return True

rev = startrev
while True:
	analyseSvnRev(rev)
	rev += 1
