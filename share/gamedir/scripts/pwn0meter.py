#!/usr/bin/python -u

import sys, time, cgi, os, random

f = open("pwn0meter.txt","r")
w = open("pwn0meter.html","w")
#w = sys.stdout
w.write("<HEAD><TITLE>Pwn0meter</TITLE></HEAD><BODY><H2>Pwn0meter</H2>\n")
w.write("<p>updated on %s</p>\n" % time.asctime())

# make random chat quotes
# doesn't matter if it fails, so surround by try/catch
try:
	# really hacky way to get latest logfile (assume that ls sorts by name)
	lastlogfile = "logs/" + os.popen("ls logs").read().splitlines()[-1]
	chatlogmark = "n: CHAT: "
	chatlines = os.popen("tail -n 10000 \"" + lastlogfile + "\" | grep \"" + chatlogmark + "\"").read().splitlines()

	chatstr = "<h3>Random chat quotes</h3>"
	rndstart = random.randint(0, len(chatlines) - 5)
	for i in xrange(rndstart, rndstart + 5):
		chatstr += cgi.escape(chatlines[i].replace(chatlogmark, "")) + "<br>"
	w.write(chatstr)
	
except:
	print "Unexpected error:", repr(sys.exc_info())
	pass

killers = {}
deaders = {}

for l in f.readlines():
	l = l.strip()
	if l == "":
		continue
	try:
		( time, deader, killer ) = l.split("\t")
	except:
		continue
	if killer.find("[CPU]") >= 0:
		continue
	if killer.find("The Third") >= 0:
		continue
	if killer.find("OpenLieroXor") >= 0:
		continue
	if not killer in killers.keys():
		killers[killer] = []
	if not deader in deaders.keys():
		deaders[deader] = []
	killers[killer].append(deader)
	deaders[deader].append(killer)
f.close()

#print killers

sorted = killers.keys()
def sortFunc(s1, s2):
	kills1 = len(killers[s1]) - killers[s1].count(s1)
	kills2 = len(killers[s2]) - killers[s2].count(s2)
	if kills1 < kills2: return 1
	if kills1 > kills2: return -1
	try:
		deaths1 = len(deaders[s1])
	except:
		deaths1 = 0
	try:
		deaths2 = len(deaders[s2])
	except:
		deaths2 = 0
	if deaths1 < deaths2: return -1
	if deaths1 > deaths2: return 1
	return 0
	
sorted.sort(cmp=sortFunc)

i = 1
for k in sorted:
	kills = len(killers[k])
	try:
		deaths = len(deaders[k])
	except:
		deatsh = 0
	suicides = killers[k].count(k)
	kills -= suicides
	deaths -= suicides
	w.write("%i. <B>%s</B>: %i kills %i deaths %i suicides, killed:" % 
		( i, cgi.escape(k), kills, deaths, suicides ))
	# Ugly killer sorting
	killedMax = {}
	for f in killers[k]:
		if not killers[k].count(f) in killedMax.keys():
			killedMax[killers[k].count(f)] = []
		if killedMax[killers[k].count(f)].count(f) == 0:
			killedMax[killers[k].count(f)].append(f)
	killedMax1 = killedMax.keys()
	killedMax1.sort(reverse=True)
	count = 0
	for f in killedMax1:
		for f1 in killedMax[f]:
			if f1 == k: # Don't write suicides
				continue
			count += 1
			if count >= 5:
				break
			if count != 1:
				w.write(",")
			w.write(" %s - %i" % ( cgi.escape(f1), f ) )
	w.write("<BR>\n")
	i += 1

w.write("</BODY>\n")
w.close()
