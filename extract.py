import sys
import os
import zlib
import struct
from operator import itemgetter

class Record(tuple):
    def __new__(cls, *result):
        if len(result) != 17:
            raise ValueError(
                "Requires 17 arguments. Recieved %d" % (len(result,)))
        return tuple.__new__(cls, result)

    unknown00 = property(itemgetter(0))
    filenum = property(itemgetter(1))
    package_offset = property(itemgetter(2))
    index_offset = property(itemgetter(3))
    package_bytes = property(itemgetter(4))
    unknown05 = property(itemgetter(5))
    unknown06 = property(itemgetter(6))
    unknown07 = property(itemgetter(7))
    unknown08 = property(itemgetter(8))
    timestamp1 = property(itemgetter(9))
    timestamp2 = property(itemgetter(10))
    timestamp3 = property(itemgetter(11))
    unpackedsize = property(itemgetter(12))
    name = property(itemgetter(13))
    path = property(itemgetter(14))
    unknown16 = property(itemgetter(15))
    pkgnum = property(itemgetter(16))


def read_index():
    index_byte_size = os.stat('pkg.idx')[6]  # st_size

    with open('pkg.idx', 'rb') as f:
        header = f.read(288)
        data = f.read(index_byte_size - 288 - 4)
        checksum = f.read(4)

    if len(data) % 592 != 0:
        raise IOError(
            "Invalid filesize: Not a multiple of %d plus %d" % (size, 288 + 4))

    files = parse_index(data)

    return header, files, checksum


def parse_index(data):
    format = '<9L3QL260s260s2L'
    size = struct.calcsize(format)  # 592 bytes
    files = []
    for offset in range(0, len(data), size):
        record = list(struct.unpack(format, data[offset:offset+size]))
        record[13] = record[13].partition('\0')[0]
        record[14] = record[14].partition('\0')[0]
        record = Record(*record)
        files.append(record)
    return files


file_cache = {}


def unpack(pkgnum, offset, size, filename):
    pkgname = "pkg%03d.pkg" % (pkgnum,)
    f = file_cache.get(pkgname)
    if f is None:
        f = file_cache[pkgname] = open(pkgname, "rb")
    f.seek(offset)
    data = f.read(size)
    data = data.decode('zlib')
    filename = os.path.join('out', filename)
    path, name = os.path.split(filename)
    if not os.path.isdir(path):
        os.makedirs(path)
    with open(filename, 'wb') as f:
        f.write(data)

# Lê os arquivos iniciais
headers, files, checksum = read_index()

n = len(files)
def sort_key(o): return (o.pkgnum, o.package_offset)


for i, rec in enumerate(sorted(files, key=sort_key)):
    fn = os.path.join(rec.path, rec.name)
    print('(%6.2f%%) Unpacking %s...' % (100.*i/n, fn))
    unpack(rec.pkgnum, rec.package_offset, rec.package_bytes, fn)
