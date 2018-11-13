
#
# Copyright 2017 SIDN
# Written by Jelte Jansen
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

#
# Custom class to store
# and check file timestamps (modification time)
# 
# Essentially a key->value store, where the key is always
# the full path to the file, and the value is the mtime()
# of the file when 'mark()' was called.
#
# Provides a few convenience functions to use this data store
#
import os
import time

def fmtime(filename):
    if os.path.exists(filename):
        return int(os.stat(filename).st_mtime)
    else:
        return 0

def file_updated(source_files, target_file):
    # returns true if any of the source files have a newer mtime
    # than the target file, or if the target file does not exist
    # (nonexistent source files are ignored, but if no source files
    # exist and the target file does, this returns false)
    if not os.path.exists(target_file):
        return True
    target_mt = fmtime(target_file)
    for source_file in source_files:
        source_mt = fmtime(source_file)
        if source_mt > target_mt:
            return True
    return False

def files_updated(source_files, target_files):
    for target_file in target_files:
        if file_updated(source_files, target_file):
            return True
    return False
