/*
 * heartyfs_creat.c
 * 
 * Brief
 * - This program handles the creation of files within the filesystem.
 *   The `create_file` function initializes a new file (inode) structure and
 *   associates it with the specified block in the filesystem.
 * 
 * Data Structures:
 * - The superblock stores metadata about the filesystem, including information
 *   about available blocks and the root directory.
 * - Inodes are used to represent files. Each inode stores the file's name, type,
 *   and size.
 * - Directories contain entries for files and subdirectories, stored as structures
 *   with metadata such as block IDs and names.
 * 
 * Design Decisions:
 * - Uses memory mapping (`mmap`) for efficient access to the disk image and
 *   direct manipulation of filesystem structures.
 * 
 *                                          Created by Nathadon Samairat 18 Oct 2024
 */
#include "../heartyfs.h"


/*
 * @brief Initializes and creates a file within the filesystem.
 * 
 * @param buffer        Pointer to the memory-mapped disk buffer.
 * @param target_name   Name of the file to be created.
 * @param target_block_id Block ID where the file (inode) should be created.
 * 
 * @return int          Returns 1 on success, -1 on failure.
 */
int create_file(void *buffer, char *target_name, uint8_t target_block_id)
{
    struct heartyfs_inode *created_file = (struct heartyfs_inode *)(buffer + BLOCK_SIZE * target_block_id);
    created_file->type = 0;
    created_file->size = 0;
    snprintf(created_file->name, sizeof(created_file->name), "%s", target_name);
    printf("Success: The file %s was created\n", target_name);
    return 1;
}

int main(int argc, char *argv[]) 
{
    printf("heartyfs_creat\n");
    
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
    if (diff == 1) // Check whether the input string directory is more than to current by 1 directory
    {
        // Check whether there is a free block available
        int free_block_id = find_free_block(bitmap);
        if (free_block_id > 0)
        {
            // Check and create an entry on the parent block if possible
            if (create_entry(superblock, parent_dir, file_name, free_block_id, bitmap) == 1) 
            {
                // Check and create a file if possible
                int parent_block_id = parent_dir->entries[0].block_id;
                if (create_file(buffer, file_name, free_block_id) == 1)
                {
                    // Mark occupied
                    superblock->free_blocks--;
                    occupy_block(free_block_id, bitmap);
                }
            }
        }
    }
    else if (diff == 0) printf("Error: The file has already existed\n");    
    else printf("Error: No such a parent for the file\n");

    // Clean up
    cleanup(buffer, fd);
    
    return 0;
}
