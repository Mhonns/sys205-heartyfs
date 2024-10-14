#include "../heartyfs.h"

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
        perror("Error: File system have not been initialized yet");
        exit(-1);
    }

    // Check directory hierarchy
    struct heartyfs_directory *parent_dir = superblock->root_dir;
    char delimiter[2] = "/";
    char* token = strtok(argv[1], delimiter);
    char dir_name[FILENAME_MAX];
    int depth = 0;
    int matched_depth = 0;
    while (token != NULL) 
    {
        sscanf(token, "%s", dir_name);
        if (depth == matched_depth)
        {
            int parent_block_id = search_file_in_dir(parent_dir, dir_name, bitmap);
            if (parent_block_id != 0)
            {
                parent_dir = (struct heartyfs_directory *) (buffer + BLOCK_SIZE * parent_block_id);
                matched_depth++;
            }
        }
        token = strtok(NULL, delimiter);
        depth++;
    }

    printf("Matched vs depth: %d vs %d\n", matched_depth, depth);
    if (matched_depth == depth - 1) // If the parent directory exists
    {
        // Create a entry
        int free_block_id = find_free_block(bitmap);
        create_entry(superblock, parent_dir, dir_name, free_block_id, bitmap);
        // Create a directory
        int parent_block_id = parent_dir->entries[0].block_id;
        create_directory(superblock, buffer, dir_name, free_block_id, parent_block_id, bitmap);
    }
    else if (matched_depth == depth) printf("Error: The directory %s has already been existed\n", dir_name);
    else printf("Error: No such a parent directory for: %s\n", dir_name);

    // Clean up
    msync(buffer, DISK_SIZE, MS_SYNC);
    munmap(buffer, DISK_SIZE);
    close(fd);

    return 0;
}
