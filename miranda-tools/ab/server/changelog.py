#! /usr/bin/env python

from os import system
from string import split
from sys import exit
import os
import httplib

#
# make sure the script is running in /home/egodust/ab/
#

os.chdir('/home/egodust/ab')

#
# there are two checkout.when's the first is generated by gen.py as it CVS checkouts
# all the modules, the second is hosted where the files generated by the client will be
# hosted, the second checkout.when is the date/time of the build aka last build.
# 
# That date is used to generate a partial changelog
#

c = httplib.HTTPConnection("miranda-icq.sourceforge.net")
c.request("GET", "/ab/checkout.when")
r = c.getresponse()
if r.status != 200:
	print 'Failed to fetch checkout.when', r.status, r.reason
	exit(1)
# the incoming data should be in YYYY-MM-DD<tab>H:M:S<tab>TZ format
stamp = split( r.read() )

#
# make sure cvs uses SSH
#
os.environ['CVS_RSH'] = 'ssh'

# now generate the delta changelog
CLFLAGS='--utc --file ChangeLog.part.tmp'
CVSROOT=':ext:miranda_ab@cvs.sourceforge.net:/cvsroot/miranda-icq'
rc = system('./cvs2cl.pl '+CLFLAGS+' -g -d' + CVSROOT + " -l \"-d'>=" + stamp[0] + ";today<='\"")
if rc != 0:
	print 'Failed to execute cvs2cl.pl'
	exit(rc)

#
# now the changelog has been generated, move it into place
#
rc = system('mv ChangeLog.part.tmp ChangeLog.part --force --reply=yes')
if rc != 0:
	print 'Failed to move changelog from ChangeLog.part.tmp to ChangeLog.part'
	exit(rc)