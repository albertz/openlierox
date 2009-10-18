#!/usr/bin/python

import sys, smtplib
import os

if len(sys.argv) < 6:
	print "Usage: " + sys.argv[0] + " <productname> <olxbin> <dumpdir> <minidumpid> <logfile>"
	exit(1)

appdir = os.path.dirname(sys.argv[0])
dumpfile = sys.argv[3] + "/" + sys.argv[4] + ".dmp"
olxbin = sys.argv[2]
productname = sys.argv[1]
logfile = sys.argv[5]

if not os.path.exists(olxbin):
	print "OpenLieroX bin", olxbin, "does not exist"
	exit(1)

if not os.path.exists(dumpfile):
	print "Dumpfile", dumpfile, "not found"
	exit(1)


msg = "To: openlierox@az2000.de\r\n"
msg = msg + "Subject: " + productname + " crash report\r\n"
msg = msg + "\r\n"

msg = msg + "\nSystem information:\n\n"

msg = msg + "os.uname = " + os.uname().__repr__() + "\n"
msg = msg + "os.name = " + os.name + "\n"
msg = msg + "os.environ = " + os.environ.__repr__() + "\n"

msg = msg + "\n\n\nCrash information:\n\n"

stream = os.popen("\"" + olxbin + "\" -minidump \"" + dumpfile + "\" 2>/dev/null")

while True:
    line = stream.readline()
    if not line:
        break
    msg = msg + line

msg = msg + "\n\n\n\n\nRecent console output:\n\n"

if os.path.exists(logfile):
	stream = open(logfile, "r")
	while True:
	    line = stream.readline()
	    if not line:
	        break
	    msg = msg + line
else:
	msg = msg + "logfile " + logfile + " not found!\n"



msg = msg + "\n\n\n\n\nCurrent config:\n\n"

try:
	from win32com.shell import shellcon, shell
	homedir = shell.SHGetFolderPath(0, shellcon.CSIDL_PERSONAL, 0, 0)

except ImportError: # quick semi-nasty fallback for non-windows/win32com case
	homedir = os.path.expanduser("~")

cfgfile = homedir + "/OpenLieroX/cfg/options.cfg"
if not os.path.exists(cfgfile): cfgfile = homedir + "/.OpenLieroX/cfg/options.cfg"
if not os.path.exists(cfgfile): cfgfile = homedir + "/Library/Application Support/OpenLieroX/cfg/options.cfg"

if os.path.exists(cfgfile):
	stream = open(cfgfile, "r")
	while True:
	    line = stream.readline()
	    if not line:
	        break
	    if line.lower().find("password") != -1:
	    	msg = msg + "<password was here>\n"
	    else:
	    	msg = msg + line
else:
	msg = msg + "OLX cfg/options.cfg not found!\n"



msg = msg + "\n\n\nReport finished.\n"

# The actual mail send
server = smtplib.SMTP('mail.az2000.de')
server.sendmail("openlierox@openlierox.net", "olxcrash@az2000.de", msg)
server.quit()

