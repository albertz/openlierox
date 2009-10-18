#!/usr/bin/python

import sys, smtplib
import os

if len(sys.argv) < 4:
	print "Usage: " + sys.argv[0] + " <olxbin> <dumpdir> <minidumpid> <productname>"
	exit(1)

appdir = os.path.dirname(sys.argv[0])
dumpfile = sys.argv[2] + "/" + sys.argv[3] + ".dmp"
olxbin = sys.argv[1]
productname = sys.argv[4]

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

stream = os.popen("grep -i OpenLieroX /var/log/system.log 2>&1")

while True:
    line = stream.readline()
    if not line:
        break
    msg = msg + line

msg = msg + "\n\n\nReport finished.\n"

# The actual mail send
server = smtplib.SMTP('mail.az2000.de')
server.sendmail("openlierox@openlierox.net", "olxcrash@az2000.de", msg)
server.quit()

