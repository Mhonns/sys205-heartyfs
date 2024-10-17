#include "../heartyfs.h"

int allocate_datablock(struct heartyfs_superblock *superblock, void *buffer, 
                        uint8_t bitmap, struct heartyfs_inode *inode,
                        struct heartyfs_data_block **datablock)
{
    if (superblock->free_blocks > 0)
    {
        int free_block_id = find_free_block(bitmap);
        *datablock = (buffer + DATA_BLOCK_SIZE * free_block_id);
        return 1;
    }
    else
    {
        printf("There is no space left to create a datablock\n");
    }
}

int main(int argc, char *argv[]) 
{
    printf("heartyfs_write\n");

    // Validate the command
    if (argc <= 2)
    {
        printf("Usage: filename /path/to/write_to /path/to/copy_from\n");
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
    char file_name[FILENAME_MAX];
    int diff = dir_string_check(argv[1], file_name, buffer, &parent_dir, bitmap);
    if (diff == 0) // Check whether the input string directory is more than to current by 1 directory
    {
        if (parent_dir->type == 1)
        {
            // Open the text file for reading
            FILE *read_file = fopen(argv[2], "r");  
            if (read_file == NULL) 
            {
                printf("Error: Can not open the file to read.\n");
                return -1;
            }

            // Move the character to the string name in datablock
            int current_block_id = search_entry_in_dir(parent_dir, file_name);
            struct heartyfs_inode *inode = (struct heartyfs_inode *) (buffer + (current_block_id * BLOCK_SIZE));
            int datablock_index = 0;
            int name_index = 0;
            int input_char;

            // Allocate a first block for the datablock
            struct heartyfs_data_block *datablock = NULL;
            int alloc_status = allocate_datablock(superblock, buffer, bitmap, inode, &datablock);
            if (alloc_status != 1) 
            {
                
                return -1;
            }

            // Read character by character in the file
            while ((input_char = fgetc(read_file)) != EOF) 
            {
                if (datablock_index >= inode->size) 
                {
                    // Clean up
                    cleanup(buffer, fd);

                    // The data is too large
                    printf("Error: The file is larger than %d bytes\n", 
                            MAX_DATA_BLOCKS * DATA_BLOCK_SIZE);
                    memset(inode->data_blocks, 0, sizeof(inode->data_blocks));
                    return -1;
                }
                
                printf("%c", (char)input_char);

                // increment to datablock is it is full

                if (name_index < inode->data_blocks[datablock_index].size) datablock_index++;
            }
            printf("\n");
            fclose(read_file);
            printf("Success: Copy the content of the file %s to the file %s\n", argv[1], argv[2]);
        } 
        else printf("Error: The parent is not a directory\n");
    }
    else printf("Error: No such a parent for file: %s\n", file_name);

    // Clean up
    cleanup(buffer, fd);

    return 0;
}
