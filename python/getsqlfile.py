#!/usr/bin/python
# -*- coding: utf-8 -*-

import os, sys, shutil

cwd = os.path.abspath( os.path.dirname( __file__ ) )
repodir = os.path.join( cwd, '.repo' )
S_repo = 'repo'
TRASHDIR = 'old_work_tree'

if not os.path.exists( os.path.join(repodir, S_repo) ):
    print >> sys.stderr, "Must run under repo work_dir root."
    sys.exit(1)

sys.path.insert( 0, os.path.join(repodir, S_repo) )
from manifest_xml import XmlManifest

manifest = XmlManifest( repodir )

if manifest.IsMirror:
    print >> sys.stderr, "Already mirror, exit."
    sys.exit(1)

fsql = open('projects.sql','w')

for project in manifest.projects.itervalues():
	fsql.writelines("insert into projects (description,submit_type,parent_name,name) values ( 'exdroid4.0', 'M','virtual/android-common', 'boxchip4.0/%s');\n" %  (project.name))

fsql.close()
print len(manifest.projects)

