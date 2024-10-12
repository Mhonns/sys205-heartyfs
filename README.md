# SYS-205 Assessment 4 - Implementing the `heartyfs` file system

`heartyfs` is a very simple file system that has common file system structures: superblock, inodes, free bitmap, and data blocks. You are tasked to implement all of these structures along with 6 basic file system operations: `mkdir`, `rmdir`, `creat`, `rm`, `read`, and `write`.

`heartyfs` will be layed out on a linear array of 512-byte blocks. The array will be physically put into a 1-MB disk file at `/tmp/heartyfs`. Thus, in total, there will be 2048 blocks.

Please follow the following task list so that you can complete the assessment without being lost.

## Setting up
Please run the following command to initialize the disk file.

```sh
sh script/init_diskfile.sh
```

## Task #0 - Layout the blocks
There will be 2048 blocks in total for this `heartyfs`. We will address to each block using an integer starting at 0, 1, ..., 2047. Blocks 0 and 1 will be reserved for the `heartyfs` while other blocks are for either inode, data, or directory.

### Block 0 "The Superblock"
Block 0 will be used as superblock. The superblock typically contains metadata of the entire file system. Also, more importantly, it must be the first place before traversing into other directories/files. In other words, it must act as the "root directory."

For `heartyfs`, we will use the superblock as a root directory. All directories in `heartyfs` can be visualized as a C struct as follows:

```c
struct heartyfs_directory {
    int type;
    char name[28];
    int size;
    struct heartyfs_dir_entry entries[14];
};
```

The `type` for the `heartyfs`'s directory will always be `1`. The `size` refers to the number of used entries in the `entries` array. The `struct heartyfs_dir_entry` has the following detail.

```c
struct heartyfs_dir_entry {
    int block_id;           // 4 bytes
    char file_name[28];     // 28 bytes
};  // Overall: 32 bytes
```

You may notice the file name in `heartyfs` is limited at 27 characters (excluding the `\0`).

If you want to modify the directory structure, you can only add more fields, but please make sure that it does not exceed 512 bytes.

### Block 1 "The Bitmap"
As described in the lecture, the bitmap will keep track of all the free blocks in the `heartyfs`. You will need to maintain the free/occupied status of all the blocks (excluding Blocks 0 and 1). Note that there will be 2046 blocks, which should use only 2046 bits (or around 256 bytes). In other words, only half of the Block 1 should be used.

### Block 2+ "The inode"
The inode in `heartyfs` will be simpler than the inode in the common file system, for the sake of simplicity. It can be visualized as the C struct below:

```c
struct heartyfs_inode {
    int type;               // 4 bytes
    char name[28];          // 28 bytes
    int size;               // 4 bytes
    int data_blocks[119];   // 476 bytes
};  // Overall: 512 bytes
```

The `size` refers to the number of data blocks used in the `data_block` array. The value of the `type` for a regular file is `0`.

### Block 2+ "The Data Block"
The data block is simply the file content. You must preserve the first 4 bytes for the size of the data. The rest 508 bytes are devoted to the content of the files pointed from inodes. In other words,

```c
struct heartyfs_data_block {
    int size;               // 4 bytes
    char name[508];         // 508 bytes
};  // Overall: 512 bytes
```

**Note that all of these structures must be persistent on disk. In other words, you must always flush all of these into a disk file.**

## Task #1 - Initialize the Superblock and Bitmap (20 points)
When initializing the `heartyfs`, the superblock must be initialized. All the entries in the root directory must be cleared (i.e., it must have only `.` and `..` entries). Moreover, the bitmap must be all `1` at first (to represent that all the blocks are free).

You need to implement this behavior in the `heartyfs_init.c` file. When you compile and run the `heartyfs_init.c` file, you should get the newly created superblock and bitmap on your machine.

**Note that, before running the following commands, you will always need to do the memory mapping of the `/tmp/heartyfs`. (This is not necessary in the actual file system.)**

## Task #2 - `heartyfs_mkdir` (15 points)
You need to create a `heartyfs_mkdir` program to handle the directory creation in `heartyfs`. Users must be able to `heartyfs_mkdir` using the following shell command:

```sh
bin/heartyfs_mkdir /dir1/dir2/dir3/
```

The implementation of `heartyfs_mkdir` must do the following things:
- Check whether the `heartyfs` is initialized or not
- Check whether there is `/dir1/dir2/` in the `heartyfs` or not
  - This means there must be `/` (root directory)
  - In the root directory, there must be `dir1/` and, in `dir1/`, there must also be `dir2/`
- Check whether `/dir1/dir2/` is full or not
- Get a free block and initialize it as a directory block
  - Add `.` and `..` as the first two entries
  - Link `.` with the current block (`/dir1/dir2/dir3/`)
  - Link `..` with the parent directory block (`/dir1/dir2/`)
- Name the directory block `dir3/`
- Add new entry to the parent directory block

`heartyfs_mkdir` must report success or error according to its action.

## Task #3 - `heartyfs_rmdir` (10 points)
You need to create a `heartyfs_rmdir` program to handle the directory removal in `heartyfs`. Users must be able to `heartyfs_rmdir` using the following shell command:

```sh
bin/heartyfs_rmdir /dir1/dir2/dir3/
```

Basically, it will remove the entry `dir3/` from the directory block of `/dir1/dir2/` **if and only if there are no entries in the `dir3/`**. More importantly, the directory block `dir3/` must also be free (by changing the bitmap).

## Task #4 - `heartyfs_creat` (15 points)
You need to create a `heartyfs_creat` program to handle the file creation in `heartyfs`. Users must be able to `heartyfs_creat` using the following shell command:

```sh
bin/heartyfs_creat /dir1/dir2/dir3/abc.xyz
```

It must create a new inode corresponding to the `abc.xyz` file. The directory `/dir1/dir2/dir3/` must also have `abc.xyz` as one of its entries.

## Task #5 - `heartyfs_rm` (10 points)
You need to create a `heartyfs_rm` program to handle the file removal in `heartyfs`. Users must be able to `heartyfs_rm` using the following shell command:

```sh
bin/heartyfs_rm /dir1/dir2/dir3/abc.xyz
```

It must remove the inode corresponding to the `abc.xyz` file. The directory `/dir1/dir2/dir3/` must also remove the entry `abc.xyz`.

## Task #6 - `heartyfs_write` (15 points)
You need to create a `heartyfs_write` program to handle the file writing in `heartyfs`. Typically, `write` will be more complicated that it needs to deal with the file system's buffer. However, in `heartyfs`, `heartyfs_write` will be just copying all the contents of the file in an external file system to the file in the `heartyfs`.

Users must be able to `heartyfs_write` using the following shell command:

```sh
bin/heartyfs_write /dir1/dir2/dir3/abc.xyz /home/pnx/random.txt
```

Do not forget to make sure that the size of the file being copied must not exceed the hard limit of the `heartyfs`. (Hint: We do not have *(in)direct pointer blocks*)

## Task #7 - `heartyfs_read` (15 points)
You need to create a `heartyfs_read` program to handle the file reading in `heartyfs`. Typically, `read` will be more complicated that it needs to deal with the file system's buffer. However, `heartyfs`'s `heartyfs_read` will be very similar to the `cat` command. In other words, it just prints out the file content to the terminal.

Users must be able to `heartyfs_read` using the following shell command:

```sh
bin/heartyfs_read /dir1/dir2/dir3/abc.xyz
```

## Code Style
You should follow a good coding convention. In this class, please stick with the *CMU 15-213's Code Style*.

https://www.cs.cmu.edu/afs/cs/academic/class/15213-f24/www/codeStyle.html

## Grading
- 100% from all the tasks
- Score penalties can be given due to the code style

## References
- `NULL`
