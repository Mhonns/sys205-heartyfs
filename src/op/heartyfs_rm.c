#include "../heartyfs.h"

void remove_file(void *buffer, uint8_t target_block_id)
{
    struct heartyfs_inode *target_file = (struct heartyfs_inode *) (buffer + BLOCK_SIZE * target_block_id);
    target_file->name[0] = '\0';
    target_file->size = 0;
    target_file->type = 0;
    memset(target_file->data_blocks, 0, sizeof(target_file->data_blocks));
}

int main(int argc, char *argv[]) 
{
    printf("heartyfs_rm\n");

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
    char *temp_input = argv[1];
    int diff = dir_string_check(temp_input, file_name, buffer, &parent_dir, bitmap);
    if (diff == 0)  // Check whether the input string directory equal to current directory string.
    {
        if (parent_dir->type == 1)
        {
            int parent_block_id = parent_dir->entries[0].block_id;
            int current_block_id = search_entry_in_dir(parent_dir, file_name);
            // remove an entry from the parent directory
            if (remove_entry(superblock, buffer, parent_block_id, file_name) == 1)
            {
                // remove all detail in the file
                remove_file(buffer, current_block_id);
                // Mark Free
                superblock->free_blocks++;
                free_block(current_block_id, bitmap);
            }
        } 
        else printf("Error: The parent is not a directory\n");
    }
    else printf("Error: No such a parent for file: %s\n", file_name);

    // Clean up
    cleanup(buffer, fd);

    return 0;
}
