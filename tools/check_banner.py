"""Check the serialized CGFX dictionaries using NintendoWare Patricia lookup.

Checking the Python objects by name is insufficient: pycgfx's __getitem__ is a
linear search, while HOME Menu follows the serialized dictionary's bit tree.
"""
import argparse
import json
import struct
from pathlib import Path


def check_cgfx(data):
    assert data[:4] == b'CGFX', 'Not a CGFX file'
    assert struct.unpack_from('<I', data, 12)[0] == len(data), 'CGFX size mismatch'
    assert len(data) <= 0x80000, 'CGFX exceeds HOME Menu decompression buffer'
    dictionaries = []
    start = 0
    while True:
        offset = data.find(b'DICT', start)
        if offset < 0:
            break
        start = offset + 4
        size, count = struct.unpack_from('<II', data, offset + 4)
        assert size == 12 + (count + 1) * 16, f'Invalid DICT at {offset:#x}'
        assert offset + size <= len(data), 'Truncated dictionary'
        nodes = []
        for index in range(count + 1):
            pos = offset + 12 + 16 * index
            bit, left, right, name_ptr, content_ptr = struct.unpack_from('<IHHii', data, pos)
            assert left <= count and right <= count, 'Invalid dictionary branch'
            name = ''
            if name_ptr:
                address = pos + 8 + name_ptr
                assert 0 <= address < len(data), 'Invalid name pointer'
                name = data[address:data.index(b'\0', address)].decode('utf-8')
            if index:
                assert name and content_ptr, 'Missing dictionary name/content'
                assert 0 <= pos + 12 + content_ptr < len(data), 'Invalid content pointer'
            nodes.append((bit, left, right, name))
        assert len({n[3] for n in nodes[1:]}) == count, 'Duplicate dictionary names'
        for target in nodes[1:]:
            name = target[3]
            key = name.encode('utf-8')
            parent, child = 0, nodes[0][1]
            visited = set()
            while nodes[parent][0] > nodes[child][0]:
                assert child not in visited, f'DICT cycle at {offset:#x}'
                visited.add(child)
                parent = child
                bit = nodes[child][0]
                value = key[bit // 8] if bit // 8 < len(key) else 0
                child = nodes[child][2 if (value >> (bit % 8)) & 1 else 1]
            assert nodes[child][3] == name, (
                f'DICT lookup at {offset:#x}: {name!r} resolves to {nodes[child][3]!r}')
        dictionaries.append({'offset': offset, 'entries': count})
    assert dictionaries, 'No dictionaries found'
    return {'cgfx_bytes': len(data), 'dictionaries': dictionaries,
            'lookups_passed': sum(d['entries'] for d in dictionaries)}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('cgfx', type=Path)
    args = parser.parse_args()
    print(json.dumps(check_cgfx(args.cgfx.read_bytes()), indent=2))
