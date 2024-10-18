/*
 * heartyfs_mkdir.c
 * 
 * Brief
 * - This program handles directory creation within the filesystem.
 *   The `create_directory` function sets up a new directory with appropriate
 *   entries ('.' and '..') and updates the parent directory with the new directory's
 *   entry if sufficient space is available.
 * 
 * Data Structures:
 * - `heartyfs_superblock`: Stores filesystem metadata, including free block information
 *   and the root directory reference.
 * - `heartyfs_directory`: Represents a directory structure with its type, size, name,
 *   and entries pointing to files or subdirectories.
 * - `bitmap`: Tracks which blocks are free or occupied.
 * 
 * Design Decisions:
 * - Uses `mmap` to map the disk file into memory for direct access to filesystem structures.
 * - Checks for available free blocks before creating a directory to ensure space availability.
 * - Updates the parent directory entries and marks the block occupied once a directory is created.
 * 
 *                                          Created by Nathadon Samairat 18 Oct 2024
 */

#include "../heartyfs.h"

/*
 * create_directory
 * - Initializes a new directory structure in the filesystem. It sets up 
 *   the directory with entries for the current (".") and parent ("..") directories.
 * 
 * @param superblock       Pointer to the superblock containing metadata and free block info.
 * @param buffer           Memory area containing the filesystem data.
 * @param target_name      The name of the new directory to be created.
 * @param target_block_id  The ID of the block where the new directory will be created.
 * @param parent_block_id  The ID of the parent directory block.
 * @param bitmap           Bitmap indicating block availability.
 * 
 * @return int             1 on success, -1 if the creation fails.
 */
int create_directory(struct heartyfs_superblock *superblock, void *buffer, 
                        char *target_name, uint8_t target_block_id, 
                        uint8_t parent_block_id, uint8_t *bitmap)
{
    struct heartyfs_directory *created_dir = (struct heartyfs_directory *)(buffer + BLOCK_SIZE * target_block_id);
    created_dir->type = 1;
    created_dir->size = 0;
    snprintf(created_dir->name, sizeof(created_dir->name), "%s", target_name);
    if (create_entry(superblock, created_dir, ".", target_block_id, bitmap) != 1)
    {
        return -1;
    }
    if (create_entry(superblock, created_dir, "..", parent_block_id, bitmap) != 1)
    {
        return -1;
    }
    printf("Success: The directory %s was created\n", created_dir->name);
    return 1;
}

int main(int argc, char *argv[]) {
    printf("heartyfs_mkdir\n");

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
    char dir_name[FILENAME_MAX];
    int diff = dir_string_check(argv[1], dir_name, buffer, &parent_dir, bitmap);
    if (diff == 1) // Check whether the input string directory is more than to current by 1 directory
    {
        // Check whether there is a free block available
        int free_block_id = find_free_block(bitmap);
        if (free_block_id > 0)
        {
            // Check and create an entry on the parent block if possible
            if (create_entry(superblock, parent_dir, dir_name, free_block_id, bitmap) == 1) 
            {
                // Check and create a directory if possible
                int parent_block_id = parent_dir->entries[0].block_id;
                if (create_directory(superblock, buffer, dir_name, 
                                        free_block_id, parent_block_id, bitmap) == 1)
                {
                    // Mark occupied
                    superblock->free_blocks--;
                    occupy_block(free_block_id, bitmap);
                }
            }
        }
        else printf("Error: There is no free block left in the disk\n");
    }
    else if (diff == 0) printf("Error: The directory has already existed\n");    
    else printf("Error: No such a parent for directory\n");

    // Clean up
    cleanup(buffer, fd);

    return 0;
}
