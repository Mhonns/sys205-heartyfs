#include "heartyfs.h"

void free_block(int block_id, uint8_t *bitmap) 
{
    bitmap[block_id / 8] |= (1 << (block_id % 8));
}

void occupy_block(int block_id, uint8_t *bitmap) 
{
    bitmap[block_id / 8] &= ~(1 << (block_id % 8));
}

int status_block(int block_id, uint8_t *bitmap) 
{
    return (bitmap[block_id / 8] >> (block_id % 8)) & 1;
}

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
                *parent_dir = (struct heartyfs_directory *) (buffer + BLOCK_SIZE * parent_block_id);
                matched_depth++;
            }
            else if (parent_block_id == 0)
            {
                struct heartyfs_superblock *superblock = (struct heartyfs_superblock *) buffer;
                *parent_dir = superblock->root_dir;
            }
        }
        token = strtok(NULL, delimiter);
        depth++;
    }
    printf("Debug: Matched vs depth: %d vs %d\n", matched_depth, depth);
    int diff = depth - matched_depth;
    return diff;
}

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
        printf("Success: Created entry %s at %s with id %d now size %d\n", parent_dir->entries[size].file_name, 
                    parent_dir->name, parent_dir->entries[size].block_id, parent_dir->size);
        return 1;
    }
}

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

int remove_entry(struct heartyfs_directory *parent_dir, char *target_name)
{
    // remove entries from the parent and move the last item
    for (int i = 0; i < parent_dir->size; i++)
    {
        if (strcmp(parent_dir->entries[i].file_name, target_name) == 0)
        {
            // Move the last entry to the removed entry
            parent_dir->entries[i] = parent_dir->entries[parent_dir->size - 1];
            printf("Debug: moved %s to entry index %d\n", parent_dir->entries[i].file_name, i);
            parent_dir->size--;
            break;
        }
    }
}

int remove_directory(struct heartyfs_superblock *superblock, void *buffer, 
                        struct heartyfs_directory *target_dir, uint8_t *bitmap)
{
    char temp_dir_name[FILENAME_MAX];
    strcpy(temp_dir_name, target_dir->name);

    // remove detail from parent of target_dir
    int parent_block_id = target_dir->entries[1].block_id;
    struct heartyfs_directory *parent_dir = NULL;
    if (parent_block_id == 0) parent_dir = superblock->root_dir;
    else parent_dir = (struct heartyfs_directory *) (buffer + BLOCK_SIZE * parent_block_id);
    remove_entry(parent_dir, temp_dir_name);

    // remove detail from target dir (just in case)
    target_dir->type = 0;
    target_dir->name[0] = '\0'; 
    for (int i = 0; i < target_dir->size; i++)
    { 
        target_dir->entries[i].block_id = 0;
        target_dir->entries[i].file_name[0] = '\0';
    }
    remove_entry(target_dir, ".");
    remove_entry(target_dir, "..");

    // remove detail in target_dir
    printf("Success: The directory was %s removed\n", temp_dir_name);
    return 1;
}