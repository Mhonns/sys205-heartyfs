/*
 * heartyfs_write.c
 * 
 * Brief:
 * - The program will copy the text from the targetfile and 
 *   add into the created file system with the given directory
 * 
 * Data Structures:
 * - Uses a superblock structure to manage overall filesystem metadata, including
 *   free block count and bitmap for block availability.
 * - Inodes store metadata about files, including their allocated data blocks.
 * - A bitmap keeps track of occupied and free blocks for efficient allocation.
 *
 * Design Decisions:
 * - Uses memory mapping (`mmap`) to directly manipulate the filesystem stored in
 *   a disk file, allowing efficient read/write operations.
 *  
 *                                      Created by Nathadon Samairat 18 Oct 2024
 */
#include "../heartyfs.h"

/*
 * @brief Allocates a data block and updates the inode.
 * 
 * @param superblock Pointer to the superblock with free block info.
 * @param buffer     Memory area containing data blocks.
 * @param bitmap     Bitmap indicating block availability.
 * @param inode      Inode needing a new data block.
 * @param datablock  Pointer to store the address of the allocated block.
 * 
 * @return int       1 on success, -1 if no space or inode full.
 */
int allocate_datablock(struct heartyfs_superblock *superblock, void *buffer, 
                        uint8_t *bitmap, struct heartyfs_inode *inode,
                        struct heartyfs_data_block **datablock)
{
    if (superblock->free_blocks > 0)
    {
        if (inode->size < MAX_DATA_BLOCKS)
        {
            // Get a new datablock
            int free_block_id = find_free_block(bitmap);
            *datablock = (struct heartyfs_data_block *)(buffer + free_block_id * BLOCK_SIZE);
            inode->data_blocks[inode->size] = free_block_id;
            inode->size++;

            // Mark occupied
            superblock->free_blocks--;
            occupy_block(free_block_id, bitmap);
            return 1;
        }
        else 
        {
            // The data is still too large
            printf("Error: The file is larger than %d bytes\n", 
                    MAX_DATA_BLOCKS * DATA_BLOCK_SIZE);
            memset(inode->data_blocks, 0, sizeof(inode->data_blocks));
            return -1;
        }
    }
    else
    {
        printf("There is no space left to create a datablock\n");
        return -1;
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
                // Open the text file for reading
                FILE *read_file = fopen(argv[2], "r");  
                if (read_file == NULL) 
                {
                    // Clean up
                    cleanup(buffer, fd);
                    printf("Cannot map the disk file onto memory\n");
                    exit(1);
                }

                // Read the content DATA_BLOCK_SIZE by DATA_BLOCK_SIZE from the file
                struct heartyfs_inode *inode = (struct heartyfs_inode *) (buffer + (current_block_id * BLOCK_SIZE));
                char input_buffer[DATA_BLOCK_SIZE];
                size_t bytesRead;
                while ((bytesRead = fread(input_buffer, sizeof(char), DATA_BLOCK_SIZE, read_file)) > 0) 
                {
                    // Allocate a data block 
                    struct heartyfs_data_block *datablock = NULL;
                    int alloc_status = allocate_datablock(superblock, buffer, bitmap, inode, &datablock);
                    if (alloc_status != 1) 
                    {
                        cleanup(buffer, fd);
                        return -1;
                    }
                    // Copy the content to the data block name
                    snprintf(datablock->name, sizeof(datablock->name), "%s", input_buffer);
                    datablock->size = DATA_BLOCK_SIZE;
                }
                printf("Success: Copy the content from: %s to: %s\n", argv[1], argv[2]);
                fclose(read_file);
            }   
            else printf("Error: The target is not found on the datablock: %s\n", file_name);
        } 
        else printf("Error: The parent is not a directory\n");
    }
    else printf("Error: The target is not a file: %s\n", file_name);

    // Clean up
    cleanup(buffer, fd);

    return 0;
}