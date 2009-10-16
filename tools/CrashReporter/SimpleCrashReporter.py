#!/usr/bin/python

import sys, smtplib
import os

if len(sys.argv) < 3:
	print "Usage: " + sys.argv[0] + " <dumpdir> <minidumpid>"
	exit(1)

appdir = os.path.dirname(sys.argv[0])
dumpfile = sys.argv[1] + "/" + sys.argv[2] + ".dmp"
reporter = appdir + "/crash_report"

if not os.path.exists(reporter):
	print "Reporter binary crash_report does not exist"
	exit(1)

if not os.path.exists(dumpfile):
	print "Dumpfile", dumpfile, "not found"
	exit(1)


msg = "To: openlierox@az2000.de\r\nSubject: OpenLieroX crash report\r\n"
msg = msg + "Crash information:\n\n"

stream = os.popen("\"" + reporter + "\" \"" + dumpfile + "\" 2>/dev/null")

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

