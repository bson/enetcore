# Copyright (c) 2018 Jan Brittenson
# See LICENSE for details.

# This is really messy.  Needs cleaning up and simplification, big time.

import sys, os, string
import pprint

def print_val(id, prop, val, indent = "    "):
    key = "RUNE_%s" % id
    if prop != "":
        key = key + "_" + prop
    print "%s%-20s = %s," % (indent, key, val)


def transpose(lists):
   if not lists: return []
   return map(lambda *row: list(row), *lists)

# Page mode: one colum of 8 bits at a time
def gendata_transposed(rune):
    data = []
    for n in xrange(0, rune['height'] / 8):
        slice = rune['def'][n * 8 : n * 8 + 8]
        slice = transpose(slice)
        slice = [reduce(lambda a,b: (a << 1) + b, row) for row in slice]
        data.append(slice)

    return data


# Simple linear mode; simply pack into 8 bit bytes, left to right
# (little endian).  If not a multiple of 8 bits, the last byte is zero
# padded.
def gendata_flat(rune):
    data = []
    d = rune['def']
    stream = reduce(lambda a, b: a+b, rune['def'])

    mask = 1
    byte = 0
    for item in stream:
        if item: byte += mask
        mask <<= 1
        if mask == 0x100:
            data.append(byte)
            byte = 0
            mask = 1
    
    if mask != 1:
        data.append(byte)

    return data


def flatten(data):
    return sum(data, [])

def rle(data):
    result = []
    counted_octet = -1
    count = 0
    for octet in data:
        if count == 0:
            count = 1
            counted_octet = octet
        elif octet == counted_octet:
            count += 1
        else:
            if count == 1 and counted_octet == 0:
                result.append(0)
            else:
                result.append(count)
                result.append(counted_octet)
            count = 1
            counted_octet = octet

    if count > 0:
        if count == 1 and counted_octet == 0:
            result.append(0)
        else:
            result.append(count)
            result.append(counted_octet)

    return result


what    = 'data'
runes   = { }
id_list = []

opt_transpose = False
opt_rle = False
opt_suffix = ""
opt_start = True
opt_end = True
opt_map = False
opt_font = False
opt_collate = False
opt_fill = False

for arg in sys.argv[1:]:
    if arg == 'transpose':
        opt_transpose = True
    elif arg == 'rle':
        opt_rle = True
    elif arg == 'inc' or arg == 'data':
        what = arg
    elif arg == 'start':
        opt_end = False
    elif arg == 'end':
        opt_start = False
    elif arg == 'body':
        opt_start = False
        opt_end = False
    elif arg == 'map':
        opt_map = True
    elif arg == 'font':
        opt_font = True
    elif arg == 'collate':
        opt_collate = True
    elif arg == 'fill':
        opt_fill = True
    else:
        exit(1)

gen_inc  = (what == 'inc')
gen_data = (what == 'data')

rune = None
code = 0
filler = 0

while True:
    line = sys.stdin.readline()
    if line == "":
        break

    line = line[:-1]
    if line == "" or line[:1] == "#":
        continue

    pieces = line.split(' ')
    if pieces[0] == "NAME_SUFFIX":
        opt_suffix = "_%s" % pieces[1]
    elif pieces[0] == "ID":
        if not rune is None:
            if len(rune['def']) != rune['height']:
                print "ERROR: %s: wrong number of lines" % rune['id']
            runes[rune['id']] = rune
            id_list.append(rune['id'])
        rune = { 'id': pieces[1], 'def' : [], 'code': code, 'nokern': False }
        code += 1
    elif pieces[0] == "DIM":
        rune['width'] = (int)(pieces[1])
        rune['height'] = (int)(pieces[2])
    elif pieces[0] == "END":
        if not rune is None:
            if len(rune['def']) != rune['height']:
                print "ERROR: %s: wrong number of lines" % rune['id']
            runes[rune['id']] = rune
            id_list.append(rune['id'])
        break
    elif pieces[0] == "CODE":
        code = (int)(pieces[1])
        rune['code'] = code
        code += 1
    elif pieces[0] == "FLAGS":
        for piece in pieces[1:]:
            if piece == "NOKERN":
                rune['nokern'] = True;
    elif pieces[0] == "FILLER":
        filler = rune['id']
    else:
        rune['def'].append(map(lambda v: 1 if v != '-' else 0, pieces[:rune['width']]))

if opt_start:
    print "// -*- C -*-"
else:
    if not gen_inc:
        print

if gen_inc and opt_start:
    print "#ifndef _RUNES_H_"
    print "#define _RUNES_H_"
    print "enum Rune {"

n = 0
offset = 0
total_size = 0
for id in id_list:
    rune = runes[id]
    w = rune['width']
    h = rune['height']
    if opt_transpose:
        rune['data'] = flatten(gendata_transposed(rune))
    else:
        rune['data'] = gendata_flat(rune)
    if opt_rle:
        rune['data'] = rle(rune['data'])

    size = len(rune['data'])
    rune['dim'] = w*h/8
    rune['size'] = size
    rune['offset'] = offset
    runes[id] = rune

    if gen_inc:
        print_val(id, "", n)
        if not opt_font:
            print_val(id, "WIDTH", w)
            print_val(id, "HEIGHT", h)
            print_val(id, "OFFSET", offset)
            print_val(id, "SIZE", size)

    n += 1
    offset += size
    total_size += size

if gen_inc and opt_end:
    print "};"
    print "#endif // _RUNES_H_"

if gen_data:
    if opt_map and opt_start:
        print "#include \"ui/font/runes.h\""
    if opt_font and opt_start:
        print "#include \"ui/font/font.h\""

    print

    num_runes = n

    # Collation: sort by code
    if opt_collate:
        # Fill: pad unused codes with filler
        if opt_fill:
            codes_used = [runes[id]['code'] for id in id_list]
            for code in xrange(0, 128):
                if not code in codes_used:
                    new_id = "CODE%s" % code
                    f = runes[filler].copy()
                    f['id'] = new_id
                    f['code'] = code
                    runes[new_id] = f
                    id_list.append(new_id)
            num_runes = 128
        list.sort(id_list, key=lambda k: runes[k]['code'])
            
    last_id = id_list[-1]

    if opt_rle:
        print "const unsigned short rune_offset%s[RUNE_NUM] = {" % opt_suffix

        s = "    "
        n = 0
        for id in id_list:
            if n == 4:
                s += "\n    "
                n = 0
            n += 1
            s += "RUNE_%s_OFFSET" % id
            if id != last_id:
                s += ", "

        print s
        print "};\n\n"


        print "const unsigned char rune_size%s[RUNE_NUM] = {" % opt_suffix

        s = "    "
        n = 0
        for id in id_list:
            if n == 4:
                s += "\n    "
                n = 0
                n += 1
                s += "RUNE_%s_SIZE" % id
            if id != last_id:
                s += ", "

        print s
        print "};\n\n"


    if opt_map:
        print "const unsigned char rune_map%s[128] = {" % opt_suffix
        f = "RUNE_%s" % filler
        if filler != 0:
            if runes[filler]['nokern']:
                f = "0x80|RUNE_%s" % filler

        m = [f for i in xrange(0,128)]
        for id in id_list:
            rune = runes[id]
            code = rune['code']
            if rune['nokern']:
                m[code] = "0x80|RUNE_%s" % id
            else:
                m[code] = "RUNE_%s" % id

        pos = 0
        s = "    "
        for r in m:
            s += "%s, " % r
            pos += 1
            if len(s) > 64 or (pos % 16) == 0:
                print s
                s = "    "
                pos = 0

        if s != "    ":
            if id == last_id and s[-2:] == ", ":
                print s[:-2]
            else:
                print s
        print
        print "    // End of map\n};\n"
        
    print "// %d runes.  Total data size %d bytes." % (num_runes, total_size)
    if opt_rle:
        print "// This is run-length encoded.  One byte count, one byte value.  The exception is"
        print "// a single 0 which represents one zero."

    print "const unsigned char rune_data%s[] = {" % opt_suffix

    for id in id_list:
        rune = runes[id]
        data = rune['data']
        print "    // Rune: %s, code %d, offset %d, size %d" % \
            (id, rune['code'], rune['offset'], rune['size'])
        s = "    "
        pos = 0
        for octet in data:
            s += "0x%02x," % octet
            pos += 1
            if (pos % 16) == 0:
                print s
                s = "    "
                pos = 0
        if s != "    ": 
            if id == last_id and s[-1] == ",":
                print s[:-1]
            else:
                print s
        print

    print "    // End of list\n};\n"

    if opt_font:
        mapname = "NULL"
        if opt_map:
            mapname = "rune_map%s" % opt_suffix

        rle = "false"
        if opt_rle:
            rle = "true"

        print "const Font font%s = {%s, %s, %s, %s, %s, %s};" % \
            (opt_suffix, "rune_data%s" % opt_suffix, mapname,
             runes[last_id]['width'], runes[last_id]['height'],
             runes[last_id]['size'], rle)
