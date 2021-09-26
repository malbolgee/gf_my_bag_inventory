#ifndef __PKG_H__
#define __PKG_H__

#include <sys/types.h>

#define MAX_PATH_LENGTH 260
#define MAX_NAME_LENGTH MAX_PATH_LENGTH

typedef struct _package
{

    u_int32_t unknown1;
    u_int32_t file_num;             /** File counter. */
    u_int32_t package_offset;       /** Package offset in the corresponding .pkg file. */
    u_int32_t index_offset;         /** Index offset inside the .idx file. */
    u_int32_t package_bytes;        /** Compressed data's size. */
    u_int32_t unknown2[0x4];
    time_t timestamp1;
    time_t timestamp2;
    time_t timestamp3;
    u_int32_t unpacked_size;         /** Decompressed data's size. */
    char file_name[MAX_NAME_LENGTH]; /** File name. */
    char file_path[MAX_PATH_LENGTH]; /** File path. */
    u_int32_t unknown3;
    u_int32_t pkg_num;               /** Package number. */

} __attribute__((packed)) package_t;

#endif