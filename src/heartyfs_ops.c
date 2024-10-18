/*
 * heartyfs_ops.c
 * 
 * Brief
 * - This program provides helper operations for managing filesystem blocks, directories, and entries.
 *   These utilities include block management, directory string checks, and operations for creating,
 *   removing, and searching entries within the filesystem.
 * 
 * Data Structures:
 * - Uses bitmaps to manage the status of blocks (free or occupied).
 * - Directories and inodes are used to structure and organize the filesystem's files.
 * 
 * Design Decisions:
 * - Functions are designed to interact directly with the memory-mapped filesystem, optimizing speed
 *   and efficiency for filesystem manipulation.
 * 
 *                                          Created by Nathadon Samairat 18 Oct 2024
 */

#include "heartyfs.h"

/*
 * @brief Marks the specified block as free in the bitmap.
 * 
 * @param block_id      The ID of the block to free.
 * @param bitmap        The bitmap tracking the status of blocks.
 */
void free_block(int block_id, uint8_t *bitmap) 
{
    bitmap[block_id / 8] |= (1 << (block_id % 8));
}

/*
 * @brief Marks the specified block as occupied in the bitmap.
 * 
 * @param block_id      The ID of the block to occupy.
 * @param bitmap        The bitmap tracking the status of blocks.
 */
void occupy_block(int block_id, uint8_t *bitmap) 
{
    bitmap[block_id / 8] &= ~(1 << (block_id % 8));
}

/*
 * @brief Checks the status of the specified block in the bitmap.
 * 
 * @param block_id      The ID of the block to check.
 * @param bitmap        The bitmap tracking the status of blocks.
 * 
 * @return int          1 if the block is free, 0 if occupied.
 */
int status_block(int block_id, uint8_t *bitmap) 
{
    return (bitmap[block_id / 8] >> (block_id % 8)) & 1;
}

/*
 * @brief Searches for a free block in the bitmap.
 *
 * @param bitmap        The bitmap tracking the status of blocks.
 * @return int          The ID of the first free block found, or -1 if no free block is available.
 */
int find_free_block(uint8_t *bitmap) 
{
    for (int i = 0; i < NUM_BLOCK; i++) 
    {
        if (status_block(i, bitmap) > 0) 
        {
            return i;
        }
    }
    return -1; // No free block found
}

/*
 * @brief -Parses the input directory string to verify the path and update the parent directory.
 *         It will also return the parent directory of the given string that match with the current structure
 * 
 * @param input_str         The directory path string to parse.
 * @param dir_name          The output name of the directory being checked.
 * @param buffer            The memory-mapped buffer of the disk image.
 * @param parent_dir        Pointer to the parent directory structure.
 * @param bitmap            bitmap tracking the status of blocks.
 * @return * int            The difference between the depth of the path and the matched depth.
 * 
 * eg.  input_str: /dir1/dir2, current structure /dir1/dir3 will 
 *          return 1 and the parent_dir is dir1
 *      input_str: /dir1/dir3, current structure /dir1/dir3 will
 *          return 0 and the parent_dir is dir3
 */
int dir_string_check(char *input_str, char *dir_name, void* buffer,
                        struct heartyfs_directory **parent_dir, uint8_t *bitmap)
{
    char delimiter[2] = "/";
    char* token = strtok(input_str, delimiter);
    int depth = 0;
    int matched_depth = 0;
    while (token != NULL) 
    {
        if (depth == matched_depth)
        {
            sscanf(token, "%s", dir_name);
            int parent_block_id = search_entry_in_dir(*parent_dir, dir_name);
            if (parent_block_id > 0)
            {
                struct heartyfs_directory *temp_dir = (struct heartyfs_directory *) (buffer + BLOCK_SIZE * parent_block_id);
                if (temp_dir->type == 1)    // check whether it is a directory or not
                {
                    *parent_dir = (struct heartyfs_directory *) (buffer + BLOCK_SIZE * parent_block_id);
                    matched_depth++;
                }
            }
            else if (parent_block_id == 0)
            {
                struct heartyfs_superblock *superblock = (struct heartyfs_superblock *) buffer;
                *parent_dir = superblock->root_dir;
                matched_depth++;
            }
        }
        token = strtok(NULL, delimiter);
        depth++;
    }
    printf("Debug: Matched vs depth: %d vs %d\n", matched_depth, depth);
    int diff = depth - matched_depth;
    return diff;
}

/*
 * @brief Searches for an entry with the specified name in the given directory.
 * 
 * @param parent_dir    The directory structure to search in.
 * @param target_name   The name of the entry to search for.
 * @return int          The block ID of the found entry, or -1 if not found.
 */
int search_entry_in_dir(struct heartyfs_directory *parent_dir, char *target_name)
{
    for (int i = 0; i < parent_dir->size; i++)
    {
        if (strcmp(parent_dir->entries[i].file_name, target_name) == 0)
        {
            return parent_dir->entries[i].block_id;
        }
    }
    return -1;
}

/*
 * @brief Creates a new entry in the specified parent directory.
 * 
 * @param superblock            The superblock structure of the filesystem.
 * @param parent_dir            The directory structure where the entry will be created.
 * @param target_name           The name of the entry to create.
 * @param target_block_id       The block ID assigned to the entry.
 * @param bitmap                The bitmap tracking the status of blocks.
 * @return int                  1 on success, -1 if the directory is full.
 */
int create_entry(struct heartyfs_superblock *superblock, struct heartyfs_directory *parent_dir, 
                    char *target_name, int target_block_id, uint8_t *bitmap)
{
    if (parent_dir->size > FILES_PER_DIR - 1)
    {
        printf("Error: The directory is full\n");
        return -1;
    }
    else
    {
        int size = parent_dir->size;
        snprintf(parent_dir->entries[size].file_name, sizeof(parent_dir->entries[size].file_name),
                    "%s", target_name);
        parent_dir->entries[size].block_id = target_block_id;
        parent_dir->size++;
        printf("Success: Created entry %s at %s with id %d\n", parent_dir->entries[size].file_name, 
                    parent_dir->name, parent_dir->entries[size].block_id);
        return 1;
    }
}

/*
 * @brief Removes the specified entry from a parent directory.
 * 
 * @param superblock            The superblock structure of the filesystem.
 * @param buffer                The memory-mapped buffer of the disk image.
 * @param parent_block_id       The block ID of the parent directory.
 * @param target_name           The name of the entry to remove.
 * @return int                  - 1 on success, -1 if the entry is not found or if the removal is denied.
 */
int remove_entry(struct heartyfs_superblock *superblock, void* buffer, 
                    int parent_block_id, char *target_name)
{
    struct heartyfs_directory *parent_dir = NULL;
    if (parent_block_id == 0) parent_dir = superblock->root_dir;
    else parent_dir = (struct heartyfs_directory *) (buffer + BLOCK_SIZE * parent_block_id);
    if (strcmp(target_name, ".") != 0 && strcmp(target_name, "..") != 0)
    {
        for (int i = 0; i < parent_dir->size; i++)
        {
            if (strcmp(parent_dir->entries[i].file_name, target_name) == 0)
            {
                // Move the last entry to the removed entry
                parent_dir->entries[i] = parent_dir->entries[parent_dir->size - 1];
                parent_dir->size--;
                printf("Success: Removed entry %s\n", target_name);
                return 1;
            }
        }
        printf("Error: Can not find the entry %s\n", target_name);
        return -1;
    }
    else
    {
        printf("Denied: Refuse to remove this entry %s\n", target_name);
        return -1;
    }
}

/*
 * @brief Cleans up resources, synchronizing and unmapping the buffer, and closing the file.
 * 
 * @param buffer        The memory-mapped buffer of the disk image.
 * @param fd            The file descriptor of the disk image.
 */
void cleanup(void *buffer, int fd) 
{
    if (buffer != NULL) {
        msync(buffer, DISK_SIZE, MS_SYNC); // Sync changes to the file
        munmap(buffer, DISK_SIZE);         // Unmap the memory
    }
    if (fd >= 0) {
        close(fd);                    // Close the file descriptor
    }
}