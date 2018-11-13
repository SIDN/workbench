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


import tempfile
import os


class FileDeleter:
    # Context class to automatically delete temporary files
    # Can generate a random file name if necessary
    # If create is true, the file is created (empty)
    def __init__(self, filename=None, delete=True, create=False):
        self.filename = filename
        self.delete = delete
        self.create = create

    def get_filename(self):
        return self.filename
    
    def __enter__(self):
        if self.filename is None:
            # USE TEMPFILE
            self.filename = tempfile.mktemp()
        if self.create:
            open(self.filename, 'w').close()
        return self
    
    def __exit__(self, value, type, traceback):
        if self.delete:
            if os.path.exists(self.filename):
                os.unlink(self.filename)
        else:
            print("Not deleting %s" % self.filename)

