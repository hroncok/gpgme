#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import absolute_import, division, unicode_literals

import gpg
import os
import os.path
import subprocess
import sys

# Copyright (C) 2018 Ben McGinnes <ben@gnupg.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License and the GNU
# Lesser General Public Licensefor more details.
#
# You should have received a copy of the GNU General Public License and the GNU
# Lesser General Public along with this program; if not, see
# <http://www.gnu.org/licenses/>.

print("""
This script exports one or more secret keys as both ASCII armored and binary
file formats, saved in files within the user's GPG home directory.

The gpg-agent and pinentry are invoked to authorise the export.
""")

if sys.platform == "win32":
    gpgconfcmd = "gpgconf.exe --list-dirs homedir"
else:
    gpgconfcmd = "gpgconf --list-dirs homedir"

a = gpg.Context(armor=True)
b = gpg.Context()
c = gpg.Context()

if len(sys.argv) >= 4:
    keyfile = sys.argv[1]
    logrus = sys.argv[2]
    homedir = sys.argv[3]
elif len(sys.argv) == 3:
    keyfile = sys.argv[1]
    logrus = sys.argv[2]
    homedir = input("Enter the GPG configuration directory path (optional): ")
elif len(sys.argv) == 2:
    keyfile = sys.argv[1]
    logrus = input("Enter the UID matching the secret key(s) to export: ")
    homedir = input("Enter the GPG configuration directory path (optional): ")
else:
    keyfile = input("Enter the filename to save the secret key to: ")
    logrus = input("Enter the UID matching the secret key(s) to export: ")
    homedir = input("Enter the GPG configuration directory path (optional): ")

if homedir.startswith("~"):
    if os.path.exists(os.path.expanduser(homedir)) is True:
        c.home_dir = os.path.expanduser(homedir)
    else:
        pass
elif os.path.exists(homedir) is True:
    c.home_dir = homedir
else:
    pass

if c.home_dir is not None:
    if c.home_dir.endswith("/"):
        gpgfile = "{0}{1}.gpg".format(c.home_dir, keyfile)
        ascfile = "{0}{1}.asc".format(c.home_dir, keyfile)
    else:
        gpgfile = "{0}/{1}.gpg".format(c.home_dir, keyfile)
        ascfile = "{0}/{1}.asc".format(c.home_dir, keyfile)
else:
    if os.path.exists(os.environ["GNUPGHOME"]) is True:
        hd = os.environ["GNUPGHOME"]
    else:
        try:
            hd = subprocess.getoutput(gpgconfcmd)
        except:
            process = subprocess.Popen(gpgconfcmd.split(),
                                       stdout=subprocess.PIPE)
            procom = process.communicate()
            if sys.version_info[0] == 2:
                hd = procom[0].strip()
            else:
                hd = procom[0].decode().strip()
    gpgfile = "{0}/{1}.gpg".format(hd, keyfile)
    ascfile = "{0}/{1}.asc".format(hd, keyfile)

try:
    a_result = a.key_export_secret(pattern=logrus)
    b_result = b.key_export_secret(pattern=logrus)
except:
    a_result = a.key_export_secret(pattern=None)
    b_result = b.key_export_secret(pattern=None)

if a_result is not None:
    with open(ascfile, "wb") as f:
        f.write(a_result)
    os.chmod(ascfile, 0o600)
else:
    pass

if b_result is not None:
    with open(gpgfile, "wb") as f:
        f.write(b_result)
    os.chmod(gpgfile, 0o600)
else:
    pass
