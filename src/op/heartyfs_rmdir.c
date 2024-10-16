#include "../heartyfs.h"

int main(int argc, char *argv[]) {
    printf("heartyfs_rmdir\n");

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
    struct heartyfs_directory *current_dir = superblock->root_dir;
    char dir_name[FILENAME_MAX];
    char *temp_input = argv[1];
    int diff = dir_string_check(temp_input, dir_name, buffer, &current_dir, bitmap);
    if (diff == 0)  // Check whether the input string directory equal to current directory string.
    {
        if (current_dir->type == 1)
        {
            if (current_dir->size <= 2) 
            {
                int target_block_id = current_dir->entries[0].block_id;
                if (remove_directory(superblock, buffer, current_dir, bitmap) == 1)
                {
                    // Mark Free
                    superblock->free_blocks++;
                    free_block(target_block_id, bitmap);
                }
            }
            else printf("Error: Please empty the directory %s first\n", current_dir->name);
        } 
        else printf("Error: The target entry is not a directory\n");
    }
    else printf("Error: No such a parent for directory: %s\n", dir_name);

    // Clean up
    msync(buffer, DISK_SIZE, MS_SYNC);
    munmap(buffer, DISK_SIZE);
    close(fd);

    return 0;
}
