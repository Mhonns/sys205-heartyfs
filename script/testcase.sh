#!/bin/bash
bin/heartyfs_init

# Creating Directory Case
# /                     # Should return exists
# /dir1/dir2            # Should return error
# /dir1/dir2/dir4       # Should be able to create
#      /dir3/dir5
# /dir6

echo '\n--Creating Directory Case--\n'
echo '\nError cases\n'
bin/heartyfs_mkdir /
bin/heartyfs_mkdir /dir1/dir2

echo '\nValid cases\n'
echo "\ncase: /dir1"
bin/heartyfs_mkdir /dir1
echo "\ncase: /dir1/dir2"
bin/heartyfs_mkdir /dir1/dir2
echo "\ncase:/dir1/dir2/dir4"
bin/heartyfs_mkdir /dir1/dir2/dir4
echo "\ncase:/dir1/dir3"
bin/heartyfs_mkdir /dir1/dir3
echo "\ncase:/dir1/dir3/dir5"
bin/heartyfs_mkdir /dir1/dir3/dir5
echo "\ncase:/dir6"
bin/heartyfs_mkdir /dir6

# Removing Directory Cases
# /dir7             # Return does not exist
# /dir3             # It is not the empty dir
# /                 # Can not remove root
# /dir6             # be able to remove
# /dir1/dir2/dir4
# /dir1/dir3/dir5
# /dir3

echo '\n--Removing Directory Cases--\n'
echo '\nError cases\n'
echo "\ncase :/dir7"
bin/heartyfs_rmdir /dir7   
echo "\ncase :/"
bin/heartyfs_rmdir /
echo "\ncase :/dir3"
bin/heartyfs_rmdir /dir3

echo '\nValid cases\n'
bin/heartyfs_rmdir /dir6
bin/heartyfs_rmdir /dir1/dir2/dir4
bin/heartyfs_rmdir /dir1/dir3/dir5
bin/heartyfs_rmdir /dir1/dir3

# Recreating casae
# /dir1/dir2            # Recap
#      /dir3/dir5
#
# /dir1/dir2/dir4
#      /dir3/dir5

echo '\n--Recreate directory cases--\n'
bin/heartyfs_mkdir /dir1/dir2/dir4
bin/heartyfs_mkdir /dir1/dir3

# Creating file cases
# /                             # Can not create a root file
# /dir1/dir2/dir4/file1.txt
#      /dir3/dir5
#           /file3.txt
# /file2.txt
# /file2.txt                    # the file is already exists

echo '\n--Creating file cases--\n'
echo '\nError cases\n'
echo "\ncase :/"
bin/heartyfs_creat /

echo '\nValid cases\n'
echo "\ncase :/dir1/dir2/dir4/file1.txt"
bin/heartyfs_creat /dir1/dir2/dir4/file1.txt
echo "\ncase :/dir1/dir3/file3.txt"
bin/heartyfs_creat /dir1/dir3/file3.txt
echo "\ncase :/file2.txt"
bin/heartyfs_creat /file2.txt

echo '\nExtra: Error cases\n'
echo "\ncase :/file2.txt"
bin/heartyfs_creat /file2.txt

# Removing file cases
# /                             # Can not remove a root file
# /dir1                         # Can not remove directory
# /dir1/file2.txt               # Can not remove non existing file
# 
# /dir1/dir3/file3.txt
# /file2.txt
# /file2.txt

echo '\n--Removing file cases--\n'
echo '\nError cases\n'
echo "\ncase :/"
bin/heartyfs_rm /
echo "\ncase :/dir1"
bin/heartyfs_rm /dir1
echo "\ncase :/dir1/file2.txt"
bin/heartyfs_rm /dir1/file2.txt


echo '\nValid cases\n'
bin/heartyfs_rm  /dir1/dir3/file3.txt
echo "\ncase :/file2.txt"
bin/heartyfs_rm /file2.txt

echo '\nExtra Error cases\n'
echo "\ncase :/file2.txt"
bin/heartyfs_rm /file2.txt

# Write a file cases
# Add the text to /tmp/heartyfs_example.txt
# /dir1 /tmp/heartyfs_example.txt                       # error the target is not a directory
# /dir1/dir2/dir4/fil.txt /tmp/heartyfs_example.txt     # no such a file or directory
# /dir1/dir2/dir4/file1.txt /tmp/heartyfs_example.txt   

echo "I Love hearty filesystem!" > /tmp/heartyfs_example.txt

echo '\n--Write a file cases--\n'
echo '\nError cases\n'
echo "\ncase :/dir1 /tmp/heartyfs_example.txt"
bin/heartyfs_write /dir1 /tmp/heartyfs_example.txt
echo "\ncase :/dir1/dir2/dir4/fil.txt /tmp/heartyfs_example.txt"
bin/heartyfs_write /dir1/dir2/dir4/fil.txt /tmp/heartyfs_example.txt   

echo '\nValid cases\n'
echo "\ncase :/dir1/dir2/dir4/file1.txt /tmp/heartyfs_example.txt "
bin/heartyfs_write /dir1/dir2/dir4/file1.txt /tmp/heartyfs_example.txt   

# Read a file cases
# /dir1/dir2/dir4/fil.txt /tmp/heartyfs_example.txt  # no such a file fil.txt
# /dir1/dir2/dir4/file1.txt                          # read this file
echo '\n--Read a file cases--\n'
echo '\nError cases\n'
bin/heartyfs_read /dir1/dir2/dir4/fil.txt /tmp/heartyfs_example.txt 

echo '\nValid cases\n'
bin/heartyfs_read /dir1/dir2/dir4/file1.txt