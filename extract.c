#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <search.h>
#include <errno.h>
#include <zlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#define HEADER_SIZE (72 * sizeof(int))
#define CRC_SIZE sizeof(int)
#define PKG_NAME_MAX_SIZE 11
#define MAX_NUMBER_OF_PKGS 334
#define MAX_PATH_LENGTH 260
#define MAX_NAME_LENGTH MAX_PATH_LENGTH
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

typedef struct _package
{

    u_int32_t unknown1;
    u_int32_t file_num;       /** File counter. */
    u_int32_t package_offset; /** Package offset in the corresponding .pkg file. */
    u_int32_t index_offset;   /** Index offset inside the .idx file. */
    u_int32_t package_bytes;  /** Size of the compressed data. */
    u_int32_t unknown2[0x4];
    time_t timestamp1;
    time_t timestamp2;
    time_t timestamp3;
    u_int32_t unpacked_size;         /** Size of the decompressed data. */
    char file_name[MAX_NAME_LENGTH]; /** File name. */
    char file_path[MAX_PATH_LENGTH]; /** File path. */
    u_int32_t unknown3;
    u_int32_t pkg_num; /** Package number. */

} __attribute__((packed)) package_t;

#define DATA_SIZE(TOTAL_BYTES_READ) ((TOTAL_BYTES_READ - HEADER_SIZE - CRC_SIZE) / sizeof(package_t))

package_t *read_index(char *);

void usage();
void ensure(bool, char *);
void open_file_check(const void *);
void alloc_check(const void *, const char *);

int compare(const void *, const void *);

void rec_mkdir(char *);
void replace_char(char *, char, char);

void unpack(package_t);
char *zlib_decompress(char *, uInt, uInt);

ENTRY entry_data, *file_cache;

int main(int argc, char **argv)
{

    if (argc != 2)
        usage(), exit(EXIT_FAILURE);

    package_t *data = read_index(argv[1]);

    struct stat st;
    stat(argv[1], &st);
    size_t index_byte_size = st.st_size;
    index_byte_size = DATA_SIZE(index_byte_size);

    qsort(data, index_byte_size, sizeof(package_t), compare);

    hcreate(MAX_HASH_TABLE_SIZE);

    for (int i = 0; i < index_byte_size; ++i)
    {

        printf("\r%c[2K(%.2f%%) - [%s%s]\r", 27, ((100.0 * i) / index_byte_size), data[i].file_path, data[i].file_name);
        fflush(stdout);
        unpack(data[i]);
    }

    free(data);
    hdestroy();

    return 0;
}

package_t *read_index(char *file_name)
{

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

char *zlib_decompress(char *data, uInt decompressed_size, uInt compressed_size)
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

    char output_path[MAX_PATH_LENGTH] = OUTPUT_PATH_NAME;
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

void replace_char(char *string, char oc, char sc)
{

    char *ch = NULL;
    while ((ch = strchr(string, oc)))
        *ch = sc;
}

void alloc_check(const void *mem, const char *message)
{

    if (mem == NULL)
        perror(message), exit(EXIT_FAILURE);
}

void open_file_check(const void *mem)
{

    alloc_check(mem, "Couldn't open the file.");
}

void usage()
{

    puts("./extract <path_to_pkg.idx>");
}

void ensure(bool expression, char *message)
{

    if (expression == false)
        fprintf(stderr, "%s\n", message), exit(EXIT_FAILURE);
}

int compare(const void *a, const void *b)
{

    if (((package_t *)a)->pkg_num == ((package_t *)b)->pkg_num)
        if (((package_t *)a)->package_offset == ((package_t *)b)->package_offset)
            return 0;
        else if (((package_t *)a)->package_offset < ((package_t *)b)->package_offset)
            return -1;
        else
            return 1;
    else if (((package_t *)a)->pkg_num < ((package_t *)b)->pkg_num)
        return -1;
    else
        return 1;
}