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


from workbench import filestamps
from testutils import FileDeleter
import os
import unittest
import time

class FileStampsTests(unittest.TestCase):
    def test_updated(self):
        fs = filestamps.FileStamps(None)
        self.assertFalse(fs.file_updated("/doesnotexist"))
        
        with FileDeleter() as fd:
            fname = fd.get_filename()
            # file should not exist yet
            self.assertFalse(fs.file_updated(fname))
            # create it
            open(fname, 'w').close()
            # file should now be 'more recent' (than nothing)
            self.assertTrue(fs.file_updated(fname))
            # Mark it, then it should no longer be more recent
            fs.mark_file(fname)
            self.assertFalse(fs.file_updated(fname))
            # Sleeps suck, but we do need to wait a tiny while
            time.sleep(1.1)
            # Touch it again, should be more recent again
            # Write a character (to make sure the action is not
            # optimized away)
            with open(fname, 'a') as f_out:
                f_out.write("foo\n")
            self.assertTrue(fs.file_updated(fname))
        
        # Out of scope, file is gone, should no longer be considered
        # updated
        self.assertFalse(fs.file_updated(fname))
    
    def test_save_load(self):
        with FileDeleter() as save_file:
            with FileDeleter(create=True) as fd1:
                with FileDeleter(create=True) as fd2:
                    save_fname = save_file.get_filename()
                    fd1_fname = fd1.get_filename()
                    fd2_fname = fd2.get_filename()
                    
                    fs1 = filestamps.FileStamps(save_fname)
                    fs1.mark_file(fd1_fname)
                    fs1.mark_file(fd2_fname)
                    
                    fs1.save()
                    
                    fs2 = filestamps.FileStamps(save_fname)
                    # Not having any data, the files should be
                    # considered 'updated' in 2, but not in 1
                    self.assertFalse(fs1.file_updated(fd1_fname))
                    self.assertFalse(fs1.file_updated(fd2_fname))
                    self.assertTrue(fs2.file_updated(fd1_fname))
                    self.assertTrue(fs2.file_updated(fd2_fname))
                    # Loading 2, should then contain same as 1
                    
                    fs2.load()
                    self.assertFalse(fs2.file_updated(fd1_fname))
                    self.assertFalse(fs2.file_updated(fd2_fname))


if __name__ == '__main__':
    unittest.main()
