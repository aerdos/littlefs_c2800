#include "F2837xD_device.h"
#include "lfs.h"
#include "lfs_util.h"


#define BLOCK_SIZE 128
#define NUM_BLOCKS 2
#define FLASH_SIZE (BLOCK_SIZE * NUM_BLOCKS)
unsigned char flash[FLASH_SIZE];
uint32_t flash_overflow = 0;

lfs_t lfs;
lfs_file_t file;


int read_lfs_wrapper(const struct lfs_config* c, lfs_block_t block, lfs_off_t offset, void* buf, lfs_size_t size_bytes)
{
    uint32_t addr = (block * c->block_size) + offset;

    for (uint32_t ii = 0; ii < size_bytes; ii++) {
        if ((addr + ii) < FLASH_SIZE) {
            ((unsigned char *)buf)[ii] = flash[addr + ii];
        }
        else {
            flash_overflow++;
        }
    }

    return 0; // TODO: add error code
}

int prog_lfs_wrapper(const struct lfs_config* c, lfs_block_t block, lfs_off_t offset, const void* buf, lfs_size_t size_bytes)
{
    uint32_t addr = (block * c->block_size) + offset;

    for (int ii = 0; ii < size_bytes; ii++) {
        if ((addr + ii) < FLASH_SIZE) {
            flash[addr + ii] = ((unsigned char*)buf)[ii];
        }
        else {
            flash_overflow++;
        }
    }

    return 0; // TODO: add error code
}

int erase_lfs_wrapper(const struct lfs_config* c, lfs_block_t block)
{
    uint32_t addr = (block * c->block_size);
    for (int ii = 0; ii < c->block_size; ii++) {
        flash[addr + ii] = 0xFF;
    }

    return 0; // TODO: add error code
}

int sync_lfs_wrapper(const struct lfs_config* c)
{
    return 0; // TODO: add error code
}

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
    // block device operations
    .read = read_lfs_wrapper,
    .prog  = prog_lfs_wrapper,
    .erase = erase_lfs_wrapper,
    .sync  = sync_lfs_wrapper,

    // block device configuration
    .read_size = 2, // all read operations are in multiples of this value (in bytes)
    .prog_size = 2, // all write operations are multple of this value (in bytes)
    .block_size = BLOCK_SIZE,
    .block_count = NUM_BLOCKS, // 1024 * 4 KByte blocks = 4 MBytes (per chip)
    .cache_size = 16,
    .lookahead_size = 16,
    .block_cycles = 500,
};

int test_1(const char *path) {
    int ret;
    lfs_size_t size;
    char buffer[10];

    size = strlen("Hi") + 1;
    strncpy((char*)buffer, "Hi", sizeof(buffer));

    ret = lfs_format(&lfs, &cfg);
    ret = lfs_mount(&lfs, &cfg);

    uint32_t uint32_a = 0x01234567;
    uint16_t uint16_a = 0xABCD;
    ret = lfs_file_open(&lfs, &file, path, LFS_O_RDWR | LFS_O_CREAT);
    ret = lfs_file_write(&lfs, &file, &uint32_a, sizeof(uint32_a));
    ret = lfs_file_write(&lfs, &file, &uint16_a, sizeof(uint16_a));
    ret = lfs_file_write(&lfs, &file, buffer, size);
    ret = lfs_file_close(&lfs, &file);

    // this is just to see how we would read binary data to an array, not used in test
    memset(buffer, 0, sizeof(buffer));
    ret = lfs_file_open(&lfs, &file, path, LFS_O_RDWR | LFS_O_CREAT);
    ret = lfs_file_read(&lfs, &file, buffer, 10);
    ret = lfs_file_close(&lfs, &file);

    // this is the same order as the writes, not used in the test
    uint32_a = uint16_a = 0;
    memset(buffer, 0, sizeof(buffer));
    ret = lfs_file_open(&lfs, &file, path, LFS_O_RDWR | LFS_O_CREAT);
    ret = lfs_file_read(&lfs, &file, &uint32_a, sizeof(uint32_a));
    ret = lfs_file_read(&lfs, &file, &uint16_a, sizeof(uint16_a));
    ret = lfs_file_read(&lfs, &file, buffer, size);
    ret = lfs_file_close(&lfs, &file);

    uint32_a = uint16_a = 0;
    memset(buffer, 0, sizeof(buffer));
    ret = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
    size = lfs_file_size(&lfs, &file);
    ret = lfs_file_seek(&lfs, &file, 2, LFS_SEEK_CUR); // offset is in native 16-bit bytes
    ret = lfs_file_read(&lfs, &file, &uint16_a, sizeof(uint16_a));
    ret = lfs_file_seek(&lfs, &file, 0, LFS_SEEK_SET); // should go back to the start of the file
    ret = lfs_file_read(&lfs, &file, &uint32_a, sizeof(uint32_a));
    ret = lfs_file_seek(&lfs, &file, 3, LFS_SEEK_SET);
    ret = lfs_file_read(&lfs, &file, buffer, 10); // only reads 3 16-bit bytes, that's all that's left
    ret = lfs_file_close(&lfs, &file);

    ret = lfs_unmount(&lfs);

    if (uint32_a == 0x01234567
            && uint16_a == 0xABCD
            && ret == 0
            && strcmp(buffer, "Hi") == 0
            && size == 6) {
        return 0;
    }
    return -1;
}

int main(void) {
    int ret;

    ret = test_1("fname_even");
    ret = test_1("fname_odd");

    if (ret == 0) {
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
