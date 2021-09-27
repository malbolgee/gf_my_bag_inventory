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

#define UI_ITEM_ICON "UI\\itemicon\\"
#define UI_SKILL_ICON "UI\\skillicon\\"
#define UI_UI_ICON "UI\\uiicon\\"
#define RIDE_MTA "ride\\"
#define DATA_DB "data\\db\\"
#define DATA_TRANSLATE "data\\Translate\\"

typedef void (*strategy_fn_arr) (package_t *, int);

#include "pkg.h"

package_t *read_index(char *file_name);
void rec_mkdir(char *path);
void unpack(package_t pkg_file);
char *zlib_decompress(char *data, uInt decompressed_size, uInt compressed_size);
bool in(char *s1, char *s2[], int size);
void unpack_data(package_t *data, int index, int size);

void extract_all(package_t *data, int size);
void extract_icon(package_t *data, int size);
void extract_ride(package_t *data, int size);
void extract_data(package_t *data, int size);

#endif