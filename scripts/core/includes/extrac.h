#ifndef __EXTRACT_H__
#define __EXTRACT_H__

#define HEADER_SIZE (72 * sizeof(int))
#define CRC_SIZE sizeof(int)
#define PKG_NAME_MAX_SIZE 11
#define MAX_NUMBER_OF_PKGS 1123
#define MAX_HASH_TABLE_SIZE ((int)((MAX_NUMBER_OF_PKGS * 0.60) + MAX_NUMBER_OF_PKGS))

#define WIN_SEP '\\'
#define UNIX_SEP '/'

#ifdef _WIN32
#define OS_SEP WIN_SEP
#else
#define OS_SEP UNIX_SEP
#endif

#define OUTPUT_PATH_NAME            \
    {                               \
        'o', 'u', 't', OS_SEP, '\0' \
    }

#define DATA_SIZE(TOTAL_BYTES_READ) ((TOTAL_BYTES_READ - HEADER_SIZE - CRC_SIZE) / sizeof(package_t))

#include "pkg.h"

package_t *read_index(char *file_name);
void rec_mkdir(char *path);
void unpack(package_t pkg_file);
char *zlib_decompress(char *data, uInt decompressed_size, uInt compressed_size);

#endif