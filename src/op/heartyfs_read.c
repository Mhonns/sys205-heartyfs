/*
 * heartyfs_read.c
 * 
 * Brief
 * - This program reads the contents of a specified file within the HeartyFS filesystem.
 *   It identifies the file using the directory structure, accesses its inode, and prints
 *   the data stored in its blocks.
 * 
 * Data Structures:
 * - The superblock contains metadata about the filesystem, such as the root directory and
 *   a bitmap to track block status (free or occupied).
 * - Inodes represent files and store file attributes including name, type, size, and
 *   pointers to data blocks containing the actual file content.
 * - Data blocks store the content of files. Each inode points to these blocks.
 * 
 * Design Decisions:
 * - The program uses memory mapping (`mmap`) to access the disk image efficiently, allowing
 *   direct manipulation and reading of filesystem structures.
 * 
 *                                          Created by Nathadon Samairat 18 Oct 2024
 */

#include "../heartyfs.h"

int main(int argc, char *argv[]) 
{
    printf("heartyfs_read\n");

    // Validate the command
    if (argc <= 1)
    {
        printf("Usage: filename /path/to/dir\n");
        exit(2);
    }

    // Open the disk file
    int fd = open(DISK_FILE_PATH, O_RDWR);
    if (fd < 0) 
    {
        perror("Cannot open the disk file\n");
        exit(1);
    }

    // Map the disk file onto memory
    void *buffer = mmap(NULL, DISK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) 
    {
        perror("Cannot map the disk file onto memory\n");
        exit(1);
    }

    // Check whether the filesystem was initialized
    struct heartyfs_superblock *superblock = (struct heartyfs_superblock *) buffer;
    uint8_t *bitmap = (uint8_t *)(buffer + BLOCK_SIZE);
    if (strcmp(superblock->root_dir->name, "") == 0)
    {
        // Clean up
        cleanup(buffer, fd);
        printf("Error: File system have not been initialized yet\n");
        exit(-1);
    }

    // Check whether directory is exists or not
    struct heartyfs_directory *parent_dir = superblock->root_dir;
    char file_name[FILENAME_MAX];
    int diff = dir_string_check(argv[1], file_name, buffer, &parent_dir, bitmap);
    if (diff == 1)
    {
        if (parent_dir->type == 1)
        {
            int current_block_id = search_entry_in_dir(parent_dir, file_name);
            if (current_block_id > 1)
            {
                struct heartyfs_inode *inode = (struct heartyfs_inode *) (buffer + (current_block_id * BLOCK_SIZE));
                for (int i = 0; i < inode->size; i++)
                {
                    int target_block_id = inode->data_blocks[i];
                    struct heartyfs_data_block *datablock = (struct heartyfs_data_block *) (buffer + target_block_id * BLOCK_SIZE);
                    printf("Success block %d: %s\n", i, datablock->name);
                }
            }
            else printf("Error: The target is not found on the datablock: %s\n", file_name);
        } 
        else printf("Error: The parent is not a directory\n");
    }
    else printf("Error: No such a parent for the target file");

    // Clean up
    cleanup(buffer, fd);

    return 0;
}
