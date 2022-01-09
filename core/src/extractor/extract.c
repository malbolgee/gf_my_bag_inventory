#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <search.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <argp.h>
#include "../../includes/pkg.h"
#include "../../includes/extrac.h"
#include "../../includes/utils.h"
#include "../../includes/parser.h"

#ifndef _WIN32
    #include <zlib.h>
#else
    #include "../../../external/zlib/zlib.h"
#endif

ENTRY entry_data, *file_cache;

static strategy_fn_arr extract_strategy[] = {extract_all, extract_icon, extract_ride, extract_data};

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

#define D(fmt, ...) \
        do { if (DEBUG_TEST) fprintf(stderr, "%s:%d:%s(): \n" fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

int main(int argc, char **argv)
{

    int index = parse(argc, argv);
    char *pkg_path = get_pkg_path();
    package_t *data = read_index(pkg_path);

    struct stat st;
    stat(pkg_path, &st);
    size_t index_byte_size = st.st_size;
    index_byte_size = DATA_SIZE(index_byte_size);

    qsort(data, index_byte_size, sizeof(package_t), compare);

    hcreate(MAX_HASH_TABLE_SIZE);

    extract_strategy[index](data, index_byte_size);

    free(data);
    hdestroy();

    return 0;
}

package_t *read_index(char *file_name)
{

    D("%s", file_name);

    struct stat st = {0};
    stat(file_name, &st);
    size_t index_byte_size = st.st_size;

    ensure(((index_byte_size - HEADER_SIZE - CRC_SIZE) % sizeof(package_t)) == 0, "Invalid filesize");

    package_t *data = (package_t *)malloc(index_byte_size - HEADER_SIZE - CRC_SIZE);
    alloc_check(data, "Couldn't allocate data for package");

    FILE *pkg = fopen(file_name, "rb");
    open_file_check(pkg);

    fseek(pkg, HEADER_SIZE, SEEK_SET);
    fread(data, sizeof(package_t), index_byte_size - HEADER_SIZE - CRC_SIZE, pkg);

    fclose(pkg);

    return data;
}

char *zlib_decompress(char *data, unsigned int decompressed_size, unsigned int compressed_size)
{

    char *decompressed_data = (char *)malloc(decompressed_size);
    alloc_check(decompressed_data, "Could not allocate space for decompressed data");
    memset(decompressed_data, 0, decompressed_size);

    z_stream infstream = {0};

    infstream.avail_in = compressed_size;
    infstream.next_in = (Bytef *)data;
    infstream.avail_out = decompressed_size;
    infstream.next_out = (Bytef *)decompressed_data;

    inflateInit(&infstream);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);

    return decompressed_data;
}

void unpack(package_t pkg_file)
{

    char *pkg_name = (char *)malloc(PKG_NAME_MAX_SIZE);
    alloc_check(pkg_name, "Could not allocate space for pkg name");
    sprintf(pkg_name, "pkg%03d.pkg", pkg_file.pkg_num);

    entry_data.data = NULL;
    entry_data.key = pkg_name;

    file_cache = hsearch(entry_data, FIND);
    FILE *pkg = NULL;

    if (file_cache == NULL)
    {

        entry_data.data = (void *)fopen(pkg_name, "rb");
        open_file_check(entry_data.data);
        hsearch(entry_data, ENTER);
        pkg = (FILE *)entry_data.data;
    }
    else
        pkg = (FILE *)file_cache->data, free(entry_data.key);

    fseek(pkg, pkg_file.package_offset, SEEK_SET);

    char *data_buffer = (char *)malloc(pkg_file.package_bytes);
    memset(data_buffer, 0, pkg_file.package_bytes);

    fread(data_buffer, sizeof(char), pkg_file.package_bytes, pkg);

    char *decompressed_data = zlib_decompress(data_buffer, pkg_file.unpacked_size, pkg_file.package_bytes);

    const char *chosen_output_path = get_output_path();
    const int has_custom_output_path = strlen(chosen_output_path);
    const int output_path_size =
            has_custom_output_path > 0 ? has_custom_output_path
            + MAX_OUTPUT_PATH_SIZE + 1 : MAX_OUTPUT_PATH_SIZE;

    char output_path[output_path_size];
    memset(output_path, 0, output_path_size);

    if (has_custom_output_path)
        strncpy(output_path, chosen_output_path, has_custom_output_path + 1);
    else
        strncpy(output_path, DEFAULT_OUTPUT_PATH_NAME, DEFAULT_OUTPUT_PATH_SIZE);

    strcat(output_path, pkg_file.file_path);

#ifndef _WIN32
    replace_char(output_path, '\\', OS_SEP);
#endif

    struct stat st;

    if (stat(output_path, &st) == -1)
        rec_mkdir(output_path);

    FILE *output_file = fopen(strcat(output_path, pkg_file.file_name), "wb");

    fwrite(decompressed_data, sizeof(char), pkg_file.unpacked_size, output_file);

    fclose(output_file);
    free(data_buffer);
    free(decompressed_data);
}

void rec_mkdir(char *path)
{

    char *sep = strrchr(path, OS_SEP);
    if (sep != NULL)
    {

        *sep = 0;
        rec_mkdir(path);
        *sep = OS_SEP;
    }

#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

void extract_all(package_t *data, int size)
{

    D("%d", size);
    for (int i = 0; i < size; ++i)
        unpack_data(data, i, size);
}

void extract_icon(package_t *data, int size)
{

    D("%d", size);
    char *path_list[] = {UI_ITEM_ICON, UI_SKILL_ICON, UI_UI_ICON};

    for (int i = 0; i < size; ++i)
        if (in(data[i].file_path, path_list, 3))
            unpack_data(data, i, size);
}

void extract_ride(package_t *data, int size)
{

    D("%d", size);
    char *path_list[] = {RIDE_MTA};

    for (int i = 0; i < size; ++i)
        if (in(data[i].file_path, path_list, 1))
            unpack_data(data, i, size);
}

void extract_data(package_t *data, int size)
{

    D("%d", size);
    char *path_list[] = {DATA_DB, DATA_TRANSLATE};

    for (int i = 0; i < size; ++i)
        if (in(data[i].file_path, path_list, 2))
            unpack_data(data, i, size);
}

bool in(char *s1, char *s2[], int size)
{

    for (int i = 0; i < size; ++i)
        if (strcmp(s1, s2[i]) == 0)
            return true;

    return false;
}

void unpack_data(package_t *data, int index, int size)
{

    printf("\r%c[2K(%.2f%%) - [%s%s]\r", 27, ((100.0 * index) / size), data[index].file_path, data[index].file_name);
    fflush(stdout);
    unpack(data[index]);
}