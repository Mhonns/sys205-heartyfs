#include "../heartyfs.h"

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
        msync(buffer, DISK_SIZE, MS_SYNC);
        munmap(buffer, DISK_SIZE);
        close(fd);
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
    else if (diff == 0) printf("Error: The directory %s has already existed\n", dir_name);    
    else printf("Error: No such a parent for directory: %s\n", dir_name);

    // Clean up
    cleanup(buffer, fd);

    return 0;
}
