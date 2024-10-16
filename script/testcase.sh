#!/bin/bash
bin/heartyfs_init

# Creating case
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

# Removing case
# /dir7             # Return does not exist
# /dir3             # It is not the empty dir
# /                 # Can not remove root
# /dir6             # be able to remove
# /dir1/dir2/dir4
# /dir1/dir3/dir5
# /dir3

echo '\n--Removing Cases--\n'
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

# Creating file case
# /dir1/dir2            # Recap
#      /dir3/dir5
#
# /dir1/dir2/dir4
#      /dir3/dir5

echo '\n--Recreate dir4 cases--\n'
bin/heartyfs_mkdir /dir1/dir2/dir4

echo '\n--Creating file cases--\n'