/*
hex2otf - Convert GNU Unifont .hex file to OpenType font

Copyright © 2022 何志翔 (He Zhixiang)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

NOTE: It is a violation of the license terms of this software
to delete or override license and copyright information contained
in the hex2otf.h file if creating a font derived from Unifont glyphs.
Fonts derived from Unifont can add names to the copyright notice
for creators of new or modified glyphs.
*/

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hex2otf.h"

#define VERSION "1.0"  // Program version, for "--version" option.

// This program assumes the execution character set is compatible with ASCII.

#define U16MAX 0xffff
#define U32MAX 0xffffffff

#define PRI_CP "U+%.4"PRIXFAST32

#ifndef static_assert
#define static_assert(a, b) (assert(a))
#endif

// Set or clear a particular bit.
#define BX(shift, x) ((uintmax_t)(!!(x)) << (shift))
#define B0(shift) BX((shift), 0)
#define B1(shift) BX((shift), 1)

#define GLYPH_MAX_WIDTH 16
#define GLYPH_HEIGHT 16
#define GLYPH_MAX_BYTE_COUNT (GLYPH_HEIGHT * GLYPH_MAX_WIDTH / 8)

// Count of pixels below baseline.
#define DESCENDER 2
#define ASCENDER (GLYPH_HEIGHT - DESCENDER)

// Font units per em.
#define FUPEM 64

// An OpenType font has at most 65536 glyphs.
#define MAX_GLYPHS 65536

// Name IDs 0-255 are used for standard names.
#define MAX_NAME_IDS 256

// Convert pixels to font units.
#define FU(x) ((x) * FUPEM / GLYPH_HEIGHT)

// Convert glyph byte count to pixel width.
#define PW(x) ((x) / (GLYPH_HEIGHT / 8))

typedef unsigned char byte;

// This type must be able to represent max(GLYPH_MAX_WIDTH, GLYPH_HEIGHT).
typedef int_least8_t pixels_t;

void
fail (const char *reason, ...)
{
    fputs ("ERROR: ", stderr);
    va_list args;
    va_start (args, reason);
    vfprintf (stderr, reason, args);
    va_end (args);
    putc ('\n', stderr);
    exit (EXIT_FAILURE);
}

// A buffer can act as a vector (when filled with 'store*' functions),
// or a temporary output area (when filled with 'cache*' functions).
// The 'store*' functions use native endian.
// The 'cache*' functions use big endian or other formats in OpenType.
// Beware of memory alignment.
typedef struct Buffer
{
    size_t capacity; // = 0 iff this buffer is free
    byte *begin, *next, *end;
} Buffer;

Buffer *allBuffers;
size_t bufferCount, nextBufferIndex;

void
initBuffers (size_t count)
{
    assert (count > 0);
    assert (bufferCount == 0); // uninitialized
    allBuffers = calloc (count, sizeof *allBuffers);
    if (!allBuffers)
        fail ("Failed to initialize buffers.");
    bufferCount = count;
    nextBufferIndex = 0;
}
void
cleanBuffers ()
{
    for (size_t i = 0; i < bufferCount; i++)
        if (allBuffers[i].capacity)
            free (allBuffers[i].begin);
    free (allBuffers);
    bufferCount = 0;
}
Buffer *
newBuffer (size_t initialCapacity)
{
    assert (initialCapacity > 0);
    Buffer *buf = NULL;
    size_t sentinel = nextBufferIndex;
    do
    {
        if (nextBufferIndex == bufferCount)
            nextBufferIndex = 0;
        if (allBuffers[nextBufferIndex].capacity == 0)
        {
            buf = &allBuffers[nextBufferIndex++];
            break;
        }
    } while (++nextBufferIndex != sentinel);
    if (!buf) // no existing buffer available
    {
        size_t newSize = sizeof (Buffer) * bufferCount * 2;
        void *extended = realloc (allBuffers, newSize);
        if (!extended)
            fail ("Failed to create new buffers.");
        allBuffers = extended;
        memset (allBuffers + bufferCount, 0, sizeof (Buffer) * bufferCount);
        buf = &allBuffers[bufferCount];
        nextBufferIndex = bufferCount + 1;
        bufferCount *= 2;
    }
    buf->begin = malloc (initialCapacity);
    if (!buf->begin)
        fail ("Failed to allocate %zu bytes of memory.", initialCapacity);
    buf->capacity = initialCapacity;
    buf->next = buf->begin;
    buf->end = buf->begin + initialCapacity;
    return buf;
}
void
ensureBuffer (Buffer *buf, size_t needed)
{
    if (buf->end - buf->next >= needed)
        return;
    ptrdiff_t occupied = buf->next - buf->begin;
    size_t required = occupied + needed;
    if (required < needed) // overflow
        fail ("Cannot allocate %zu + %zu bytes of memory.", occupied, needed);
    if (required > SIZE_MAX / 2)
        buf->capacity = required;
    else while (buf->capacity < required)
        buf->capacity *= 2;
    void *extended = realloc (buf->begin, buf->capacity);
    if (!extended)
        fail ("Failed to allocate %zu bytes of memory.", buf->capacity);
    buf->begin = extended;
    buf->next = buf->begin + occupied;
    buf->end = buf->begin + buf->capacity;
}
static inline size_t
countBufferedBytes (const Buffer *buf)
{
    return buf->next - buf->begin;
}
static inline void *
getBufferHead (const Buffer *buf)
{
    return buf->begin;
}
static inline void *
getBufferTail (const Buffer *buf)
{
    return buf->next;
}
static inline void *
getBufferSlot (Buffer *buf, size_t slotSize)
{
    ensureBuffer (buf, slotSize);
    void *slot = buf->next;
    buf->next += slotSize;
    return slot;
}
static inline void
resetBuffer (Buffer *buf)
{
    buf->next = buf->begin;
}
void
freeBuffer (Buffer *buf)
{
    free (buf->begin);
    buf->capacity = 0;
}

#define defineStore(name, type) \
void name (Buffer *buf, type value) \
{ \
    type *slot = getBufferSlot (buf, sizeof value); \
    *slot = value; \
}
defineStore (storeU8, uint_least8_t)
defineStore (storeU16, uint_least16_t)
defineStore (storeU32, uint_least32_t)
defineStore (storePixels, pixels_t)
#undef defineStore

void
cacheU (Buffer *buf, uint_fast32_t value, int bytes)
{
    assert (1 <= bytes && bytes <= 4);
    ensureBuffer (buf, bytes);
    switch (bytes)
    {
        case 4: *buf->next++ = value >> 24 & 0xff; // fall through
        case 3: *buf->next++ = value >> 16 & 0xff; // fall through
        case 2: *buf->next++ = value >> 8  & 0xff; // fall through
        case 1: *buf->next++ = value       & 0xff;
    }
}
void
cacheU8 (Buffer *buf, uint_fast8_t value)
{
    storeU8 (buf, value & 0xff);
}
void
cacheU16 (Buffer *buf, uint_fast16_t value)
{
    cacheU (buf, value, 2);
}
void
cacheU32 (Buffer *buf, uint_fast32_t value)
{
    cacheU (buf, value, 4);
}

void
cacheCFFOperand (Buffer *buf, int_fast32_t value)
{
    if (-107 <= value && value <= 107)
        cacheU8 (buf, value + 139);
    else if (108 <= value && value <= 1131)
    {
        cacheU8 (buf, (value - 108) / 256 + 247);
        cacheU8 (buf, (value - 108) % 256);
    }
    else if (-32768 <= value && value <= 32767)
    {
        cacheU8 (buf, 28);
        cacheU16 (buf, value);
    }
    else if (-2147483647 <= value && value <= 2147483647)
    {
        cacheU8 (buf, 29);
        cacheU32 (buf, value);
    }
    else
        assert (false); // other encodings are not used and omitted
    static_assert (GLYPH_MAX_WIDTH <= 107, "More encodings are needed.");
}

void
cacheZeros (Buffer *buf, size_t count)
{
    ensureBuffer (buf, count);
    memset (buf->next, 0, count);
    buf->next += count;
}

void
cacheBytes (Buffer *restrict buf, const void *restrict src, size_t count)
{
    ensureBuffer (buf, count);
    memcpy (buf->next, src, count);
    buf->next += count;
}

void
cacheBuffer (Buffer *restrict bufDest, const Buffer *restrict bufSrc)
{
    size_t length = countBufferedBytes (bufSrc);
    ensureBuffer (bufDest, length);
    memcpy (bufDest->next, bufSrc->begin, length);
    bufDest->next += length;
}

void
writeBytes (const byte bytes[], size_t count, FILE *file)
{
    if (fwrite (bytes, count, 1, file) != 1 && count != 0)
        fail ("Failed to write %zu bytes to output file.", count);
}

void
writeU16 (uint_fast16_t value, FILE *file)
{
    byte bytes[] =
    {
        (value >> 8) & 0xff,
        (value     ) & 0xff,
    };
    writeBytes (bytes, sizeof bytes, file);
}
void
writeU32 (uint_fast32_t value, FILE *file)
{
    byte bytes[] =
    {
        (value >> 24) & 0xff,
        (value >> 16) & 0xff,
        (value >>  8) & 0xff,
        (value      ) & 0xff,
    };
    writeBytes (bytes, sizeof bytes, file);
}

static inline void
writeBuffer (const Buffer *buf, FILE *file)
{
    writeBytes (getBufferHead (buf), countBufferedBytes (buf), file);
}


typedef const char *NameStrings[MAX_NAME_IDS];

typedef struct Glyph
{
    uint_least32_t codePoint; // undefined for glyph 0
    byte bitmap[GLYPH_MAX_BYTE_COUNT];
    uint_least8_t byteCount; // length of bitmap data
    bool combining; // whether this is a combining glyph
    pixels_t pos; // number of pixels the glyph should be moved to the right
                  // (negative number means moving to the left)
    pixels_t lsb; // left side bearing (x position of leftmost contour point)
} Glyph;

typedef struct Font
{
    Buffer *tables;
    Buffer *glyphs;
    uint_fast32_t glyphCount;
    pixels_t maxWidth;
} Font;

// An OpenType table.
// (https://docs.microsoft.com/en-us/typography/opentype/spec/otff#font-tables)
typedef struct Table
{
    uint_fast32_t tag;
    Buffer *content;
} Table;

enum LocaFormat {LOCA_OFFSET16 = 0, LOCA_OFFSET32 = 1};

static inline uint_fast32_t tagAsU32 (const char tag[static 4])
{
    uint_fast32_t r = 0;
    r |= (tag[0] & 0xff) << 24;
    r |= (tag[1] & 0xff) << 16;
    r |= (tag[2] & 0xff) << 8;
    r |= (tag[3] & 0xff);
    return r;
}

void
addTable (Font *font, const char tag[static 4], Buffer *content)
{
    Table *table = getBufferSlot (font->tables, sizeof (Table));
    table->tag = tagAsU32 (tag);
    table->content = content;
}

// Sort tables according to OpenType recommendations.
void
organizeTables (Font *font, bool isCFF)
{
    const char *const cffOrder[] = {"head","hhea","maxp","OS/2","name",
        "cmap","post","CFF ",NULL};
    const char *const truetypeOrder[] = {"head","hhea","maxp","OS/2",
        "hmtx","LTSH","VDMX","hdmx","cmap","fpgm","prep","cvt ","loca",
        "glyf","kern","name","post","gasp","PCLT","DSIG",NULL};
    const char *const *const order = isCFF ? cffOrder : truetypeOrder;
    Table *unordered = getBufferHead (font->tables);
    const Table *const tablesEnd = getBufferTail (font->tables);
    for (const char *const *p = order; *p; p++)
    {
        uint_fast32_t tag = tagAsU32 (*p);
        for (Table *t = unordered; t < tablesEnd; t++)
        {
            if (t->tag != tag)
                continue;
            if (t != unordered)
            {
                Table temp = *unordered;
                *unordered = *t;
                *t = temp;
            }
            unordered++;
            break;
        }
    }
}

struct TableRecord
{
    uint_least32_t tag, offset, length, checksum;
};
int
byTableTag (const void *a, const void *b)
{
    const struct TableRecord *const ra = a, *const rb = b;
    int gt = ra->tag > rb->tag;
    int lt = ra->tag < rb->tag;
    return gt - lt;
}
void
writeFont (Font *font, bool isCFF, const char *fileName)
{
    FILE *file = fopen (fileName, "wb");
    if (!file)
        fail ("Failed to open file '%s'.", fileName);
    const Table *const tables = getBufferHead (font->tables);
    const Table *const tablesEnd = getBufferTail (font->tables);
    size_t tableCount = tablesEnd - tables;
    assert (0 < tableCount && tableCount <= U16MAX);
    size_t offset = 12 + 16 * tableCount;
    uint_fast32_t totalChecksum = 0;
    Buffer *tableRecords =
        newBuffer (sizeof (struct TableRecord) * tableCount);
    for (size_t i = 0; i < tableCount; i++)
    {
        struct TableRecord *record =
            getBufferSlot (tableRecords, sizeof *record);
        record->tag = tables[i].tag;
        size_t length = countBufferedBytes (tables[i].content);
        #if SIZE_MAX > U32MAX
            if (offset > U32MAX)
                fail ("Table offset exceeded 4 GiB.");
            if (length > U32MAX)
                fail ("Table size exceeded 4 GiB.");
        #endif
        record->length = length;
        record->checksum = 0;
        const byte *p = getBufferHead (tables[i].content);
        const byte *const end = getBufferTail (tables[i].content);
        #define addByte(shift) \
            if (p == end) \
                break; \
            record->checksum += (uint_fast32_t)*p++ << (shift);
        for (;;)
        {
            addByte (24)
            addByte (16)
            addByte (8)
            addByte (0)
        }
        #undef addByte
        cacheZeros (tables[i].content, (~length + 1U) & 3U);
        record->offset = offset;
        offset += countBufferedBytes (tables[i].content);
        totalChecksum += record->checksum;
    }
    struct TableRecord *records = getBufferHead (tableRecords);
    qsort (records, tableCount, sizeof *records, byTableTag);
    // Offset Table
    uint_fast32_t sfntVersion = isCFF ? 0x4f54544f : 0x00010000;
    writeU32 (sfntVersion, file); // sfntVersion
    totalChecksum += sfntVersion;
    uint_fast16_t entrySelector = 0;
    for (size_t k = tableCount; k != 1; k >>= 1)
        entrySelector++;
    uint_fast16_t searchRange = 1 << (entrySelector + 4);
    uint_fast16_t rangeShift = (tableCount - (1 << entrySelector)) << 4;
    writeU16 (tableCount, file); // numTables
    writeU16 (searchRange, file); // searchRange
    writeU16 (entrySelector, file); // entrySelector
    writeU16 (rangeShift, file); // rangeShift
    totalChecksum += (uint_fast32_t)tableCount << 16;
    totalChecksum += searchRange;
    totalChecksum += (uint_fast32_t)entrySelector << 16;
    totalChecksum += rangeShift;
    // Table Records (always sorted by table tags)
    for (size_t i = 0; i < tableCount; i++)
    {
        // Table Record
        writeU32 (records[i].tag, file); // tableTag
        writeU32 (records[i].checksum, file); // checkSum
        writeU32 (records[i].offset, file); // offset
        writeU32 (records[i].length, file); // length
        totalChecksum += records[i].tag;
        totalChecksum += records[i].checksum;
        totalChecksum += records[i].offset;
        totalChecksum += records[i].length;
    }
    freeBuffer (tableRecords);
    for (const Table *table = tables; table < tablesEnd; table++)
    {
        if (table->tag == 0x68656164) // 'head' table
        {
            byte *begin = getBufferHead (table->content);
            byte *end = getBufferTail (table->content);
            writeBytes (begin, 8, file);
            writeU32 (0xb1b0afbaU - totalChecksum, file); // checksumAdjustment
            writeBytes (begin + 12, end - (begin + 12), file);
            continue;
        }
        writeBuffer (table->content, file);
    }
    fclose (file);
}

static inline byte
nibbleValue (char nibble)
{
    if (isdigit (nibble))
        return nibble - '0';
    nibble = toupper (nibble);
    return nibble - 'A' + 10;
}

// Read up to 6 hex digits and a colon from file.
// Return true if end of file is reached.
bool
readCodePoint (uint_fast32_t *codePoint, const char *fileName, FILE *file)
{
    *codePoint = 0;
    uint_fast8_t digitCount = 0;
    for (;;)
    {
        int c = getc (file);
        if (isxdigit (c) && ++digitCount <= 6)
        {
            *codePoint = (*codePoint << 4) | nibbleValue (c);
            continue;
        }
        if (c == ':' && digitCount > 0)
            return false;
        if (c == EOF)
        {
            if (digitCount == 0)
                return true;
            if (feof (file))
                fail ("%s: Unexpected end of file.", fileName);
            else
                fail ("%s: Read error.", fileName);
        }
        fail ("%s: Unexpected character: %#.2x.", fileName, (unsigned)c);
    }
}

void
readGlyphs (Font *font, const char *fileName)
{
    FILE *file = fopen (fileName, "r");
    if (!file)
        fail ("Failed to open file '%s'.", fileName);
    uint_fast32_t glyphCount = 1; // for glyph 0
    uint_fast8_t maxByteCount = 0;
    { // Hard code the .notdef glyph.
        const byte bitmap[] = "\0\0\0~fZZzvv~vv~\0\0"; // same as U+FFFD
        const size_t byteCount = sizeof bitmap - 1;
        assert (byteCount <= GLYPH_MAX_BYTE_COUNT);
        assert (byteCount % GLYPH_HEIGHT == 0);
        Glyph *notdef = getBufferSlot (font->glyphs, sizeof (Glyph));
        memcpy (notdef->bitmap, bitmap, byteCount);
        notdef->byteCount = maxByteCount = byteCount;
        notdef->combining = false;
        notdef->pos = 0;
        notdef->lsb = 0;
    }
    for (;;)
    {
        uint_fast32_t codePoint;
        if (readCodePoint (&codePoint, fileName, file))
            break;
        if (++glyphCount > MAX_GLYPHS)
            fail ("OpenType does not support more than %lu glyphs.",
                MAX_GLYPHS);
        Glyph *glyph = getBufferSlot (font->glyphs, sizeof (Glyph));
        glyph->codePoint = codePoint;
        glyph->byteCount = 0;
        glyph->combining = false;
        glyph->pos = 0;
        glyph->lsb = 0;
        for (byte *p = glyph->bitmap;; p++)
        {
            int h, l;
            if (isxdigit (h = getc (file)) && isxdigit (l = getc (file)))
            {
                if (++glyph->byteCount > GLYPH_MAX_BYTE_COUNT)
                    fail ("Hex stream of "PRI_CP" is too long.", codePoint);
                *p = nibbleValue (h) << 4 | nibbleValue (l);
            }
            else if (h == '\n' || (h == EOF && feof (file)))
                break;
            else if (ferror (file))
                fail ("%s: Read error.", fileName);
            else
                fail ("Hex stream of "PRI_CP" is invalid.", codePoint);
        }
        if (glyph->byteCount % GLYPH_HEIGHT != 0)
            fail ("Hex length of "PRI_CP" is indivisible by glyph height %d.",
                codePoint, GLYPH_HEIGHT);
        if (glyph->byteCount > maxByteCount)
            maxByteCount = glyph->byteCount;
    }
    if (glyphCount == 1)
        fail ("No glyph is specified.");
    font->glyphCount = glyphCount;
    font->maxWidth = PW (maxByteCount);
    fclose (file);
}

int
byCodePoint (const void *a, const void *b)
{
    const Glyph *const ga = a, *const gb = b;
    int gt = ga->codePoint > gb->codePoint;
    int lt = ga->codePoint < gb->codePoint;
    return gt - lt;
}

// Glyphs must be sorted by code point before calling this function.
void
positionGlyphs (Font *font, const char *fileName, pixels_t *xMin)
{
    *xMin = 0;
    FILE *file = fopen (fileName, "r");
    if (!file)
        fail ("Failed to open file '%s'.", fileName);
    Glyph *glyphs = getBufferHead (font->glyphs);
    const Glyph *const endGlyph = glyphs + font->glyphCount;
    Glyph *nextGlyph = &glyphs[1]; // predict and avoid search
    for (;;)
    {
        uint_fast32_t codePoint;
        if (readCodePoint (&codePoint, fileName, file))
            break;
        Glyph *glyph = nextGlyph;
        if (glyph == endGlyph || glyph->codePoint != codePoint)
        {
            // Prediction failed. Search.
            const Glyph key = { .codePoint = codePoint };
            glyph = bsearch (&key, glyphs + 1, font->glyphCount - 1,
                sizeof key, byCodePoint);
            if (!glyph)
                fail ("Glyph "PRI_CP" is positioned but not defined.",
                    codePoint);
        }
        nextGlyph = glyph + 1;
        char s[8];
        if (!fgets (s, sizeof s, file))
            fail ("%s: Read error.", fileName);
        char *end;
        const long value = strtol (s, &end, 10);
        if (*end != '\n' && *end != '\0')
            fail ("Position of glyph "PRI_CP" is invalid.", codePoint);
        // Currently no glyph is moved to the right,
        // so positive position is considered out of range.
        // If this limit is to be lifted,
        // 'xMax' of bounding box in 'head' table shall also be updated.
        if (value < -GLYPH_MAX_WIDTH || value > 0)
            fail ("Position of glyph "PRI_CP" is out of range.", codePoint);
        glyph->combining = true;
        glyph->pos = value;
        glyph->lsb = value; // updated during outline generation
        if (value < *xMin)
            *xMin = value;
    }
    fclose (file);
}

void
sortGlyphs (Font *font)
{
    Glyph *glyphs = getBufferHead (font->glyphs);
    const Glyph *const glyphsEnd = getBufferTail (font->glyphs);
    glyphs++; // glyph 0 does not need sorting
    qsort (glyphs, glyphsEnd - glyphs, sizeof *glyphs, byCodePoint);
    for (const Glyph *glyph = glyphs; glyph < glyphsEnd - 1; glyph++)
    {
        if (glyph[0].codePoint == glyph[1].codePoint)
            fail ("Duplicate code point: "PRI_CP".", glyph[0].codePoint);
        assert (glyph[0].codePoint < glyph[1].codePoint);
    }
}

enum ContourOp {OP_CLOSE, OP_POINT};
enum FillSide {FILL_LEFT, FILL_RIGHT};

void
buildOutline (Buffer *result, const byte bitmap[], const size_t byteCount,
    const enum FillSide fillSide)
{
    enum Direction {RIGHT, LEFT, DOWN, UP}; // order is significant

    // respective coordinate deltas
    const pixels_t dx[] = {1, -1, 0, 0}, dy[] = {0, 0, -1, 1};

    assert (byteCount % GLYPH_HEIGHT == 0);
    const uint_fast8_t bytesPerRow = byteCount / GLYPH_HEIGHT;
    const pixels_t glyphWidth = bytesPerRow * 8;
    assert (glyphWidth <= GLYPH_MAX_WIDTH);
    #if GLYPH_MAX_WIDTH < 32
        typedef uint_fast32_t row_t;
    #elif GLYPH_MAX_WIDTH < 64
        typedef uint_fast64_t row_t;
    #else
        #error GLYPH_MAX_WIDTH is too large.
    #endif
    row_t pixels[GLYPH_HEIGHT + 2] = {0};
    for (pixels_t row = GLYPH_HEIGHT; row > 0; row--)
        for (pixels_t b = 0; b < bytesPerRow; b++)
            pixels[row] = pixels[row] << 8 | *bitmap++;
    typedef row_t graph_t[GLYPH_HEIGHT + 1];
    graph_t vectors[4];
    const row_t *lower = pixels, *upper = pixels + 1;
    for (pixels_t row = 0; row <= GLYPH_HEIGHT; row++)
    {
        const row_t m = (fillSide == FILL_RIGHT) - 1;
        vectors[RIGHT][row] = (m ^ (*lower << 1)) & (~m ^ (*upper << 1));
        vectors[LEFT ][row] = (m ^ (*upper     )) & (~m ^ (*lower     ));
        vectors[DOWN ][row] = (m ^ (*lower     )) & (~m ^ (*lower << 1));
        vectors[UP   ][row] = (m ^ (*upper << 1)) & (~m ^ (*upper     ));
        lower++;
        upper++;
    }
    graph_t selection = {0};
    const row_t x0 = (row_t)1 << glyphWidth;
    #define getRowBit(rows, x, y)  ((rows)[(y)] &  x0 >> (x))
    #define flipRowBit(rows, x, y) ((rows)[(y)] ^= x0 >> (x))
    for (pixels_t y = GLYPH_HEIGHT; y >= 0; y--)
    {
        for (pixels_t x = 0; x <= glyphWidth; x++)
        {
            assert (!getRowBit (vectors[LEFT], x, y));
            assert (!getRowBit (vectors[UP], x, y));
            enum Direction initial;
            if (getRowBit (vectors[RIGHT], x, y))
                initial = RIGHT;
            else if (getRowBit (vectors[DOWN], x, y))
                initial = DOWN;
            else
                continue;
            static_assert ((GLYPH_MAX_WIDTH + 1) * (GLYPH_HEIGHT + 1) * 2 <=
                U16MAX, "potential overflow");
            uint_fast16_t lastPointCount = 0;
            for (bool converged = false;;)
            {
                uint_fast16_t pointCount = 0;
                enum Direction heading = initial;
                for (pixels_t tx = x, ty = y;;)
                {
                    if (converged)
                    {
                        storePixels (result, OP_POINT);
                        storePixels (result, tx);
                        storePixels (result, ty);
                    }
                    do
                    {
                        if (converged)
                            flipRowBit (vectors[heading], tx, ty);
                        tx += dx[heading];
                        ty += dy[heading];
                    } while (getRowBit (vectors[heading], tx, ty));
                    if (tx == x && ty == y)
                        break;
                    static_assert ((UP ^ DOWN) == 1 && (LEFT ^ RIGHT) == 1,
                        "wrong enums");
                    heading = (heading & 2) ^ 2;
                    heading |= !!getRowBit (selection, tx, ty);
                    heading ^= !getRowBit (vectors[heading], tx, ty);
                    assert (getRowBit (vectors[heading], tx, ty));
                    flipRowBit (selection, tx, ty);
                    pointCount++;
                }
                if (converged)
                    break;
                converged = pointCount == lastPointCount;
                lastPointCount = pointCount;
            }
            storePixels (result, OP_CLOSE);
        }
    }
    #undef getRowBit
    #undef flipRowBit
}

void
prepareOffsets (size_t *sizes)
{
    size_t *p = sizes;
    for (size_t *i = sizes + 1; *i; i++)
        *i += *p++;
    if (*p > 2147483647U) // offset not representable
        fail ("CFF table is too large.");
}
Buffer *
prepareStringIndex (const NameStrings names)
{
    Buffer *buf = newBuffer (256);
    assert (names[6]);
    const char *strings[] = {"Adobe", "Identity", names[6]};
    #define stringCount (sizeof strings / sizeof *strings)
    static_assert (stringCount <= U16MAX, "too many strings");
    size_t offset = 1;
    size_t lengths[stringCount];
    for (size_t i = 0; i < stringCount; i++)
    {
        assert (strings[i]);
        lengths[i] = strlen (strings[i]);
        offset += lengths[i];
    }
    int offsetSize = 1 + (offset > 0xff)
                       + (offset > 0xffff)
                       + (offset > 0xffffff);
    cacheU16 (buf, stringCount); // count
    cacheU8 (buf, offsetSize); // offSize
    cacheU (buf, offset = 1, offsetSize); // offset[0]
    for (size_t i = 0; i < stringCount; i++)
        cacheU (buf, offset += lengths[i], offsetSize); // offset[i + 1]
    for (size_t i = 0; i < stringCount; i++)
        cacheBytes (buf, strings[i], lengths[i]);
    #undef stringCount
    return buf;
}
void
fillCFF (Font *font, int version, const NameStrings names)
{
    // HACK: For convenience, CFF data structures are hard coded.
    assert (0 < version && version <= 2);
    Buffer *cff = newBuffer (65536);
    addTable (font, version == 1 ? "CFF " : "CFF2", cff);
    // Use fixed width integer for variables to simplify offset calculation.
    #define cacheCFF32(buf, x) (cacheU8 ((buf), 29), cacheU32 ((buf), (x)))
    // In Unifont, 16px glyphs are more common. This is used by CFF1 only.
    const pixels_t defaultWidth = 16, nominalWidth = 8;
    if (version == 1)
    {
        Buffer *strings = prepareStringIndex (names);
        size_t stringsSize = countBufferedBytes (strings);
        const char *cffName = names[6];
        assert (cffName);
        size_t nameLength = strlen (cffName);
        size_t namesSize = nameLength + 5;
        // These sizes must be updated together with the data below.
        size_t offsets[] = {4, namesSize, 45, stringsSize, 2, 5, 8, 32, 4, 0};
        prepareOffsets (offsets);
        { // Header
            cacheU8 (cff, 1); // major
            cacheU8 (cff, 0); // minor
            cacheU8 (cff, 4); // hdrSize
            cacheU8 (cff, 1); // offSize
        }
        assert (countBufferedBytes (cff) == offsets[0]);
        { // Name INDEX (should not be used by OpenType readers)
            cacheU16 (cff, 1); // count
            cacheU8 (cff, 1); // offSize
            cacheU8 (cff, 1); // offset[0]
            if (nameLength + 1 > 255) // must be too long; spec limit is 63
                fail ("PostScript name is too long.");
            cacheU8 (cff, nameLength + 1); // offset[1]
            cacheBytes (cff, cffName, nameLength);
        }
        assert (countBufferedBytes (cff) == offsets[1]);
        { // Top DICT INDEX
            cacheU16 (cff, 1); // count
            cacheU8 (cff, 1); // offSize
            cacheU8 (cff, 1); // offset[0]
            cacheU8 (cff, 41); // offset[1]
            cacheCFFOperand (cff, 391); // "Adobe"
            cacheCFFOperand (cff, 392); // "Identity"
            cacheCFFOperand (cff, 0);
            cacheBytes (cff, (byte[]){12, 30}, 2); // ROS
            cacheCFF32 (cff, font->glyphCount);
            cacheBytes (cff, (byte[]){12, 34}, 2); // CIDCount
            cacheCFF32 (cff, offsets[6]);
            cacheBytes (cff, (byte[]){12, 36}, 2); // FDArray
            cacheCFF32 (cff, offsets[5]);
            cacheBytes (cff, (byte[]){12, 37}, 2); // FDSelect
            cacheCFF32 (cff, offsets[4]);
            cacheU8 (cff, 15); // charset
            cacheCFF32 (cff, offsets[8]);
            cacheU8 (cff, 17); // CharStrings
        }
        assert (countBufferedBytes (cff) == offsets[2]);
        { // String INDEX
            cacheBuffer (cff, strings);
            freeBuffer (strings);
        }
        assert (countBufferedBytes (cff) == offsets[3]);
        cacheU16 (cff, 0); // Global Subr INDEX
        assert (countBufferedBytes (cff) == offsets[4]);
        { // Charsets
            cacheU8 (cff, 2); // format
            { // Range2[0]
                cacheU16 (cff, 1); // first
                cacheU16 (cff, font->glyphCount - 2); // nLeft
            }
        }
        assert (countBufferedBytes (cff) == offsets[5]);
        { // FDSelect
            cacheU8 (cff, 3); // format
            cacheU16 (cff, 1); // nRanges
            cacheU16 (cff, 0); // first
            cacheU8 (cff, 0); // fd
            cacheU16 (cff, font->glyphCount); // sentinel
        }
        assert (countBufferedBytes (cff) == offsets[6]);
        { // FDArray
            cacheU16 (cff, 1); // count
            cacheU8 (cff, 1); // offSize
            cacheU8 (cff, 1); // offset[0]
            cacheU8 (cff, 28); // offset[1]
            cacheCFFOperand (cff, 393);
            cacheBytes (cff, (byte[]){12, 38}, 2); // FontName
            // Windows requires FontMatrix in Font DICT.
            const byte unit[] = {0x1e,0x15,0x62,0x5c,0x6f}; // 1/64 (0.015625)
            cacheBytes (cff, unit, sizeof unit);
            cacheCFFOperand (cff, 0);
            cacheCFFOperand (cff, 0);
            cacheBytes (cff, unit, sizeof unit);
            cacheCFFOperand (cff, 0);
            cacheCFFOperand (cff, 0);
            cacheBytes (cff, (byte[]){12, 7}, 2); // FontMatrix
            cacheCFFOperand (cff, offsets[8] - offsets[7]); // size
            cacheCFF32 (cff, offsets[7]); // offset
            cacheU8 (cff, 18); // Private
        }
        assert (countBufferedBytes (cff) == offsets[7]);
        { // Private
            cacheCFFOperand (cff, FU (defaultWidth));
            cacheU8 (cff, 20); // defaultWidthX
            cacheCFFOperand (cff, FU (nominalWidth));
            cacheU8 (cff, 21); // nominalWidthX
        }
        assert (countBufferedBytes (cff) == offsets[8]);
    }
    else
    {
        assert (version == 2);
        // These sizes must be updated together with the data below.
        size_t offsets[] = {5, 21, 4, 10, 0};
        prepareOffsets (offsets);
        { // Header
            cacheU8 (cff, 2); // majorVersion
            cacheU8 (cff, 0); // minorVersion
            cacheU8 (cff, 5); // headerSize
            cacheU16 (cff, offsets[1] - offsets[0]); // topDictLength
        }
        assert (countBufferedBytes (cff) == offsets[0]);
        { // Top DICT
            const byte unit[] = {0x1e,0x15,0x62,0x5c,0x6f}; // 1/64 (0.015625)
            cacheBytes (cff, unit, sizeof unit);
            cacheCFFOperand (cff, 0);
            cacheCFFOperand (cff, 0);
            cacheBytes (cff, unit, sizeof unit);
            cacheCFFOperand (cff, 0);
            cacheCFFOperand (cff, 0);
            cacheBytes (cff, (byte[]){12, 7}, 2); // FontMatrix
            cacheCFFOperand (cff, offsets[2]);
            cacheBytes (cff, (byte[]){12, 36}, 2); // FDArray
            cacheCFFOperand (cff, offsets[3]);
            cacheU8 (cff, 17); // CharStrings
        }
        assert (countBufferedBytes (cff) == offsets[1]);
        cacheU32 (cff, 0); // Global Subr INDEX
        assert (countBufferedBytes (cff) == offsets[2]);
        { // Font DICT INDEX
            cacheU32 (cff, 1); // count
            cacheU8 (cff, 1); // offSize
            cacheU8 (cff, 1); // offset[0]
            cacheU8 (cff, 4); // offset[1]
            cacheCFFOperand (cff, 0);
            cacheCFFOperand (cff, 0);
            cacheU8 (cff, 18); // Private
        }
        assert (countBufferedBytes (cff) == offsets[3]);
    }
    { // CharStrings INDEX
        Buffer *offsets = newBuffer (4096);
        Buffer *charstrings = newBuffer (4096);
        Buffer *outline = newBuffer (1024);
        const Glyph *glyph = getBufferHead (font->glyphs);
        const Glyph *const endGlyph = glyph + font->glyphCount;
        for (; glyph < endGlyph; glyph++)
        {
            // CFF offsets start at 1
            storeU32 (offsets, countBufferedBytes (charstrings) + 1);

            pixels_t rx = -glyph->pos;
            pixels_t ry = DESCENDER;
            resetBuffer (outline);
            buildOutline (outline, glyph->bitmap, glyph->byteCount, FILL_LEFT);
            enum CFFOp {rmoveto=21, hmoveto=22, vmoveto=4, hlineto=6,
                vlineto=7, endchar=14};
            enum CFFOp pendingOp = 0;
            const int STACK_LIMIT = version == 1 ? 48 : 513;
            int stackSize = 0;
            bool isDrawing = false;
            pixels_t width = glyph->combining ? 0 : PW (glyph->byteCount);
            if (version == 1 && width != defaultWidth)
            {
                cacheCFFOperand (charstrings, FU (width - nominalWidth));
                stackSize++;
            }
            for (const pixels_t *p = getBufferHead (outline),
                 *const end = getBufferTail (outline); p < end;)
            {
                int s = 0;
                const enum ContourOp op = *p++;
                if (op == OP_POINT)
                {
                    const pixels_t x = *p++, y = *p++;
                    if (x != rx)
                    {
                        cacheCFFOperand (charstrings, FU (x - rx));
                        rx = x;
                        stackSize++;
                        s |= 1;
                    }
                    if (y != ry)
                    {
                        cacheCFFOperand (charstrings, FU (y - ry));
                        ry = y;
                        stackSize++;
                        s |= 2;
                    }
                    assert (!(isDrawing && s == 3));
                }
                if (s)
                {
                    if (!isDrawing)
                    {
                        const enum CFFOp moves[] = {0, hmoveto, vmoveto,
                            rmoveto};
                        cacheU8 (charstrings, moves[s]);
                        stackSize = 0;
                    }
                    else if (!pendingOp)
                        pendingOp = (enum CFFOp[]){0, hlineto, vlineto}[s];
                }
                else if (!isDrawing)
                {
                    // only when the first point happens to be (0, 0)
                    cacheCFFOperand (charstrings, FU (0));
                    cacheU8 (charstrings, hmoveto);
                    stackSize = 0;
                }
                if (op == OP_CLOSE || stackSize >= STACK_LIMIT)
                {
                    assert (stackSize <= STACK_LIMIT);
                    cacheU8 (charstrings, pendingOp);
                    pendingOp = 0;
                    stackSize = 0;
                }
                isDrawing = op != OP_CLOSE;
            }
            if (version == 1)
                cacheU8 (charstrings, endchar);
        }
        size_t lastOffset = countBufferedBytes (charstrings) + 1;
        #if SIZE_MAX > U32MAX
            if (lastOffset > U32MAX)
                fail ("CFF data exceeded size limit.");
        #endif
        storeU32 (offsets, lastOffset);
        int offsetSize = 1 + (lastOffset > 0xff)
                           + (lastOffset > 0xffff)
                           + (lastOffset > 0xffffff);
        // count (must match 'numGlyphs' in 'maxp' table)
        cacheU (cff, font->glyphCount, version * 2);
        cacheU8 (cff, offsetSize); // offSize
        const uint_least32_t *p = getBufferHead (offsets);
        const uint_least32_t *const end = getBufferTail (offsets);
        for (; p < end; p++)
            cacheU (cff, *p, offsetSize); // offsets
        cacheBuffer (cff, charstrings); // data
        freeBuffer (offsets);
        freeBuffer (charstrings);
        freeBuffer (outline);
    }
    #undef cacheCFF32
}

void
fillTrueType (Font *font, enum LocaFormat *format,
    uint_fast16_t *maxPoints, uint_fast16_t *maxContours)
{
    Buffer *glyf = newBuffer (65536);
    addTable (font, "glyf", glyf);
    Buffer *loca = newBuffer (4 * (font->glyphCount + 1));
    addTable (font, "loca", loca);
    *format = LOCA_OFFSET32;
    Buffer *endPoints = newBuffer (256);
    Buffer *flags = newBuffer (256);
    Buffer *xs = newBuffer (256);
    Buffer *ys = newBuffer (256);
    Buffer *outline = newBuffer (1024);
    Glyph *const glyphs = getBufferHead (font->glyphs);
    const Glyph *const glyphsEnd = getBufferTail (font->glyphs);
    for (Glyph *glyph = glyphs; glyph < glyphsEnd; glyph++)
    {
        cacheU32 (loca, countBufferedBytes (glyf));
        pixels_t rx = -glyph->pos;
        pixels_t ry = DESCENDER;
        pixels_t xMin = GLYPH_MAX_WIDTH, xMax = 0;
        pixels_t yMin = ASCENDER, yMax = -DESCENDER;
        resetBuffer (endPoints);
        resetBuffer (flags);
        resetBuffer (xs);
        resetBuffer (ys);
        resetBuffer (outline);
        buildOutline (outline, glyph->bitmap, glyph->byteCount, FILL_RIGHT);
        uint_fast32_t pointCount = 0, contourCount = 0;
        for (const pixels_t *p = getBufferHead (outline),
             *const end = getBufferTail (outline); p < end;)
        {
            const enum ContourOp op = *p++;
            if (op == OP_CLOSE)
            {
                contourCount++;
                assert (contourCount <= U16MAX);
                cacheU16 (endPoints, pointCount - 1);
                continue;
            }
            assert (op == OP_POINT);
            pointCount++;
            assert (pointCount <= U16MAX);
            const pixels_t x = *p++, y = *p++;
            uint_fast8_t pointFlags =
                + B1 (0) // point is on curve
                + BX (1, x != rx) // x coordinate is 1 byte instead of 2
                + BX (2, y != ry) // y coordinate is 1 byte instead of 2
                + B0 (3) // repeat
                + BX (4, x >= rx) // when x is 1 byte: x is positive;
                                  // when x is 2 bytes: x unchanged and omitted
                + BX (5, y >= ry) // when y is 1 byte: y is positive;
                                  // when y is 2 bytes: y unchanged and omitted
                + B1 (6) // contours may overlap
                + B0 (7) // reserved
            ;
            cacheU8 (flags, pointFlags);
            if (x != rx)
                cacheU8 (xs, FU (x > rx ? x - rx : rx - x));
            if (y != ry)
                cacheU8 (ys, FU (y > ry ? y - ry : ry - y));
            if (x < xMin) xMin = x;
            if (y < yMin) yMin = y;
            if (x > xMax) xMax = x;
            if (y > yMax) yMax = y;
            rx = x;
            ry = y;
        }
        if (contourCount == 0)
            continue; // blank glyph is indicated by the 'loca' table
        glyph->lsb = glyph->pos + xMin;
        cacheU16 (glyf, contourCount); // numberOfContours
        cacheU16 (glyf, FU (glyph->pos + xMin)); // xMin
        cacheU16 (glyf, FU (yMin)); // yMin
        cacheU16 (glyf, FU (glyph->pos + xMax)); // xMax
        cacheU16 (glyf, FU (yMax)); // yMax
        cacheBuffer (glyf, endPoints); // endPtsOfContours[]
        cacheU16 (glyf, 0); // instructionLength
        cacheBuffer (glyf, flags); // flags[]
        cacheBuffer (glyf, xs); // xCoordinates[]
        cacheBuffer (glyf, ys); // yCoordinates[]
        if (pointCount > *maxPoints)
            *maxPoints = pointCount;
        if (contourCount > *maxContours)
            *maxContours = contourCount;
    }
    cacheU32 (loca, countBufferedBytes (glyf));
    freeBuffer (endPoints);
    freeBuffer (flags);
    freeBuffer (xs);
    freeBuffer (ys);
    freeBuffer (outline);
}

void
fillBlankOutline (Font *font)
{
    Buffer *glyf = newBuffer (12);
    addTable (font, "glyf", glyf);
    // Empty table is not allowed, but an empty outline for glyph 0 suffices.
    cacheU16 (glyf, 0); // numberOfContours
    cacheU16 (glyf, FU (0)); // xMin
    cacheU16 (glyf, FU (0)); // yMin
    cacheU16 (glyf, FU (0)); // xMax
    cacheU16 (glyf, FU (0)); // yMax
    cacheU16 (glyf, 0); // instructionLength
    Buffer *loca = newBuffer (2 * (font->glyphCount + 1));
    addTable (font, "loca", loca);
    cacheU16 (loca, 0); // offsets[0]
    assert (countBufferedBytes (glyf) % 2 == 0);
    for (uint_fast32_t i = 1; i <= font->glyphCount; i++)
        cacheU16 (loca, countBufferedBytes (glyf) / 2); // offsets[i]
}

void
fillBitmap (Font *font)
{
    const Glyph *const glyphs = getBufferHead (font->glyphs);
    const Glyph *const glyphsEnd = getBufferTail (font->glyphs);
    size_t bitmapsSize = 0;
    for (const Glyph *glyph = glyphs; glyph < glyphsEnd; glyph++)
        bitmapsSize += glyph->byteCount;
    Buffer *ebdt = newBuffer (4 + bitmapsSize);
    addTable (font, "EBDT", ebdt);
    cacheU16 (ebdt, 2); // majorVersion
    cacheU16 (ebdt, 0); // minorVersion
    uint_fast8_t byteCount = 0; // unequal to any glyph
    pixels_t pos = 0;
    bool combining = false;
    Buffer *rangeHeads = newBuffer (32);
    Buffer *offsets = newBuffer (64);
    for (const Glyph *glyph = glyphs; glyph < glyphsEnd; glyph++)
    {
        if (glyph->byteCount != byteCount || glyph->pos != pos ||
            glyph->combining != combining)
        {
            storeU16 (rangeHeads, glyph - glyphs);
            storeU32 (offsets, countBufferedBytes (ebdt));
            byteCount = glyph->byteCount;
            pos = glyph->pos;
            combining = glyph->combining;
        }
        cacheBytes (ebdt, glyph->bitmap, byteCount);
    }
    const uint_least16_t *ranges = getBufferHead (rangeHeads);
    const uint_least16_t *rangesEnd = getBufferTail (rangeHeads);
    uint_fast32_t rangeCount = rangesEnd - ranges;
    storeU16 (rangeHeads, font->glyphCount);
    Buffer *eblc = newBuffer (4096);
    addTable (font, "EBLC", eblc);
    cacheU16 (eblc, 2); // majorVersion
    cacheU16 (eblc, 0); // minorVersion
    cacheU32 (eblc, 1); // numSizes
    { // bitmapSizes[0]
        cacheU32 (eblc, 56); // indexSubTableArrayOffset
        cacheU32 (eblc, (8 + 20) * rangeCount); // indexTablesSize
        cacheU32 (eblc, rangeCount); // numberOfIndexSubTables
        cacheU32 (eblc, 0); // colorRef
        { // hori
            cacheU8 (eblc, ASCENDER); // ascender
            cacheU8 (eblc, -DESCENDER); // descender
            cacheU8 (eblc, font->maxWidth); // widthMax
            cacheU8 (eblc, 1); // caretSlopeNumerator
            cacheU8 (eblc, 0); // caretSlopeDenominator
            cacheU8 (eblc, 0); // caretOffset
            cacheU8 (eblc, 0); // minOriginSB
            cacheU8 (eblc, 0); // minAdvanceSB
            cacheU8 (eblc, ASCENDER); // maxBeforeBL
            cacheU8 (eblc, -DESCENDER); // minAfterBL
            cacheU8 (eblc, 0); // pad1
            cacheU8 (eblc, 0); // pad2
        }
        { // vert
            cacheU8 (eblc, ASCENDER); // ascender
            cacheU8 (eblc, -DESCENDER); // descender
            cacheU8 (eblc, font->maxWidth); // widthMax
            cacheU8 (eblc, 1); // caretSlopeNumerator
            cacheU8 (eblc, 0); // caretSlopeDenominator
            cacheU8 (eblc, 0); // caretOffset
            cacheU8 (eblc, 0); // minOriginSB
            cacheU8 (eblc, 0); // minAdvanceSB
            cacheU8 (eblc, ASCENDER); // maxBeforeBL
            cacheU8 (eblc, -DESCENDER); // minAfterBL
            cacheU8 (eblc, 0); // pad1
            cacheU8 (eblc, 0); // pad2
        }
        cacheU16 (eblc, 0); // startGlyphIndex
        cacheU16 (eblc, font->glyphCount - 1); // endGlyphIndex
        cacheU8 (eblc, 16); // ppemX
        cacheU8 (eblc, 16); // ppemY
        cacheU8 (eblc, 1); // bitDepth
        cacheU8 (eblc, 1); // flags = Horizontal
    }
    { // IndexSubTableArray
        uint_fast32_t offset = rangeCount * 8;
        for (const uint_least16_t *p = ranges; p < rangesEnd; p++)
        {
            cacheU16 (eblc, *p); // firstGlyphIndex
            cacheU16 (eblc, p[1] - 1); // lastGlyphIndex
            cacheU32 (eblc, offset); // additionalOffsetToIndexSubtable
            offset += 20;
        }
    }
    { // IndexSubTables
        const uint_least32_t *offset = getBufferHead (offsets);
        for (const uint_least16_t *p = ranges; p < rangesEnd; p++)
        {
            const Glyph *glyph = &glyphs[*p];
            cacheU16 (eblc, 2); // indexFormat
            cacheU16 (eblc, 5); // imageFormat
            cacheU32 (eblc, *offset++); // imageDataOffset
            cacheU32 (eblc, glyph->byteCount); // imageSize
            { // bigMetrics
                cacheU8 (eblc, GLYPH_HEIGHT); // height
                const uint_fast8_t width = PW (glyph->byteCount);
                cacheU8 (eblc, width); // width
                cacheU8 (eblc, glyph->pos); // horiBearingX
                cacheU8 (eblc, ASCENDER); // horiBearingY
                cacheU8 (eblc, glyph->combining ? 0 : width); // horiAdvance
                cacheU8 (eblc, 0); // vertBearingX
                cacheU8 (eblc, 0); // vertBearingY
                cacheU8 (eblc, GLYPH_HEIGHT); // vertAdvance
            }
        }
    }
    freeBuffer (rangeHeads);
    freeBuffer (offsets);
}

void
fillHeadTable (Font *font, enum LocaFormat locaFormat, pixels_t xMin)
{
    Buffer *head = newBuffer (56);
    addTable (font, "head", head);
    cacheU16 (head, 1); // majorVersion
    cacheU16 (head, 0); // minorVersion
    cacheZeros (head, 4); // fontRevision (unused)
    // The 'checksumAdjustment' field is a checksum of the entire file.
    // It is later calculated and written directly in the 'writeFont' function.
    cacheU32 (head, 0); // checksumAdjustment (placeholder)
    cacheU32 (head, 0x5f0f3cf5); // magicNumber
    const uint_fast16_t flags =
        + B1 ( 0) // baseline at y=0
        + B1 ( 1) // LSB at x=0 (doubtful; probably should be LSB=xMin)
        + B0 ( 2) // instructions may depend on point size
        + B0 ( 3) // force internal ppem to integers
        + B0 ( 4) // instructions may alter advance width
        + B0 ( 5) // not used in OpenType
        + B0 ( 6) // not used in OpenType
        + B0 ( 7) // not used in OpenType
        + B0 ( 8) // not used in OpenType
        + B0 ( 9) // not used in OpenType
        + B0 (10) // not used in OpenType
        + B0 (11) // font transformed
        + B0 (12) // font converted
        + B0 (13) // font optimized for ClearType
        + B0 (14) // last resort font
        + B0 (15) // reserved
    ;
    cacheU16 (head, flags); // flags
    cacheU16 (head, FUPEM); // unitsPerEm
    cacheZeros (head, 8); // created (unused)
    cacheZeros (head, 8); // modified (unused)
    cacheU16 (head, FU (xMin)); // xMin
    cacheU16 (head, FU (-DESCENDER)); // yMin
    cacheU16 (head, FU (font->maxWidth)); // xMax
    cacheU16 (head, FU (ASCENDER)); // yMax
    // macStyle (must agree with 'fsSelection' in 'OS/2' table)
    const uint_fast16_t macStyle =
        + B0 (0) // bold
        + B0 (1) // italic
        + B0 (2) // underline
        + B0 (3) // outline
        + B0 (4) // shadow
        + B0 (5) // condensed
        + B0 (6) // extended
        //    7-15  reserved
    ;
    cacheU16 (head, macStyle);
    cacheU16 (head, GLYPH_HEIGHT); // lowestRecPPEM
    cacheU16 (head, 2); // fontDirectionHint
    cacheU16 (head, locaFormat); // indexToLocFormat
    cacheU16 (head, 0); // glyphDataFormat
}

void
fillHheaTable (Font *font, pixels_t xMin)
{
    Buffer *hhea = newBuffer (36);
    addTable (font, "hhea", hhea);
    cacheU16 (hhea, 1); // majorVersion
    cacheU16 (hhea, 0); // minorVersion
    cacheU16 (hhea, FU (ASCENDER)); // ascender
    cacheU16 (hhea, FU (-DESCENDER)); // descender
    cacheU16 (hhea, FU (0)); // lineGap
    cacheU16 (hhea, FU (font->maxWidth)); // advanceWidthMax
    cacheU16 (hhea, FU (xMin)); // minLeftSideBearing
    cacheU16 (hhea, FU (0)); // minRightSideBearing (unused)
    cacheU16 (hhea, FU (font->maxWidth)); // xMaxExtent
    cacheU16 (hhea, 1); // caretSlopeRise
    cacheU16 (hhea, 0); // caretSlopeRun
    cacheU16 (hhea, 0); // caretOffset
    cacheU16 (hhea, 0); // reserved
    cacheU16 (hhea, 0); // reserved
    cacheU16 (hhea, 0); // reserved
    cacheU16 (hhea, 0); // reserved
    cacheU16 (hhea, 0); // metricDataFormat
    cacheU16 (hhea, font->glyphCount); // numberOfHMetrics
}

void
fillMaxpTable (Font *font, bool isCFF, uint_fast16_t maxPoints,
    uint_fast16_t maxContours)
{
    Buffer *maxp = newBuffer (32);
    addTable (font, "maxp", maxp);
    cacheU32 (maxp, isCFF ? 0x00005000 : 0x00010000); // version
    cacheU16 (maxp, font->glyphCount); // numGlyphs
    if (isCFF)
        return;
    cacheU16 (maxp, maxPoints); // maxPoints
    cacheU16 (maxp, maxContours); // maxContours
    cacheU16 (maxp, 0); // maxCompositePoints
    cacheU16 (maxp, 0); // maxCompositeContours
    cacheU16 (maxp, 0); // maxZones
    cacheU16 (maxp, 0); // maxTwilightPoints
    cacheU16 (maxp, 0); // maxStorage
    cacheU16 (maxp, 0); // maxFunctionDefs
    cacheU16 (maxp, 0); // maxInstructionDefs
    cacheU16 (maxp, 0); // maxStackElements
    cacheU16 (maxp, 0); // maxSizeOfInstructions
    cacheU16 (maxp, 0); // maxComponentElements
    cacheU16 (maxp, 0); // maxComponentDepth
}

void
fillOS2Table (Font *font)
{
    Buffer *os2 = newBuffer (100);
    addTable (font, "OS/2", os2);
    cacheU16 (os2, 5); // version
    // HACK: Average glyph width is not actually calculated.
    cacheU16 (os2, FU (font->maxWidth)); // xAvgCharWidth
    cacheU16 (os2, 400); // usWeightClass = Normal
    cacheU16 (os2, 5); // usWidthClass = Medium
    const uint_fast16_t typeFlags =
        + B0 (0) // reserved
        // usage permissions, one of:
            // Default: Installable embedding
            + B0 (1) // Restricted License embedding
            + B0 (2) // Preview & Print embedding
            + B0 (3) // Editable embedding
        //    4-7   reserved
        + B0 (8) // no subsetting
        + B0 (9) // bitmap embedding only
        //    10-15 reserved
    ;
    cacheU16 (os2, typeFlags); // fsType
    cacheU16 (os2, FU (5)); // ySubscriptXSize
    cacheU16 (os2, FU (7)); // ySubscriptYSize
    cacheU16 (os2, FU (0)); // ySubscriptXOffset
    cacheU16 (os2, FU (1)); // ySubscriptYOffset
    cacheU16 (os2, FU (5)); // ySuperscriptXSize
    cacheU16 (os2, FU (7)); // ySuperscriptYSize
    cacheU16 (os2, FU (0)); // ySuperscriptXOffset
    cacheU16 (os2, FU (4)); // ySuperscriptYOffset
    cacheU16 (os2, FU (1)); // yStrikeoutSize
    cacheU16 (os2, FU (5)); // yStrikeoutPosition
    cacheU16 (os2, 0x080a); // sFamilyClass = Sans Serif, Matrix
    const byte panose[] =
    {
        2, // Family Kind = Latin Text
        11, // Serif Style = Normal Sans
        4, // Weight = Thin
        // Windows would render all glyphs to the same width,
        // if 'Proportion' is set to 'Monospaced' (as Unifont should be).
        // 'Condensed' is the best alternative according to metrics.
        6, // Proportion = Condensed
        2, // Contrast = None
        2, // Stroke = No Variation
        2, // Arm Style = Straight Arms
        8, // Letterform = Normal/Square
        2, // Midline = Standard/Trimmed
        4, // X-height = Constant/Large
    };
    cacheBytes (os2, panose, sizeof panose); // panose
    // HACK: All defined Unicode ranges are marked functional for convenience.
    cacheU32 (os2, 0xffffffff); // ulUnicodeRange1
    cacheU32 (os2, 0xffffffff); // ulUnicodeRange2
    cacheU32 (os2, 0xffffffff); // ulUnicodeRange3
    cacheU32 (os2, 0x0effffff); // ulUnicodeRange4
    cacheBytes (os2, "GNU ", 4); // achVendID
    // fsSelection (must agree with 'macStyle' in 'head' table)
    const uint_fast16_t selection =
        + B0 (0) // italic
        + B0 (1) // underscored
        + B0 (2) // negative
        + B0 (3) // outlined
        + B0 (4) // strikeout
        + B0 (5) // bold
        + B1 (6) // regular
        + B1 (7) // use sTypo* metrics in this table
        + B1 (8) // font name conforms to WWS model
        + B0 (9) // oblique
        //    10-15 reserved
    ;
    cacheU16 (os2, selection);
    const Glyph *glyphs = getBufferHead (font->glyphs);
    uint_fast32_t first = glyphs[1].codePoint;
    uint_fast32_t last = glyphs[font->glyphCount - 1].codePoint;
    cacheU16 (os2, first < U16MAX ? first : U16MAX); // usFirstCharIndex
    cacheU16 (os2, last  < U16MAX ? last  : U16MAX); // usLastCharIndex
    cacheU16 (os2, FU (ASCENDER)); // sTypoAscender
    cacheU16 (os2, FU (-DESCENDER)); // sTypoDescender
    cacheU16 (os2, FU (0)); // sTypoLineGap
    cacheU16 (os2, FU (ASCENDER)); // usWinAscent
    cacheU16 (os2, FU (DESCENDER)); // usWinDescent
    // HACK: All reasonable code pages are marked functional for convenience.
    cacheU32 (os2, 0x603f01ff); // ulCodePageRange1
    cacheU32 (os2, 0xffff0000); // ulCodePageRange2
    cacheU16 (os2, FU (8)); // sxHeight
    cacheU16 (os2, FU (10)); // sCapHeight
    cacheU16 (os2, 0); // usDefaultChar
    cacheU16 (os2, 0x20); // usBreakChar
    cacheU16 (os2, 0); // usMaxContext
    cacheU16 (os2, 0); // usLowerOpticalPointSize
    cacheU16 (os2, 0xffff); // usUpperOpticalPointSize
}

void
fillHmtxTable (Font *font)
{
    Buffer *hmtx = newBuffer (4 * font->glyphCount);
    addTable (font, "hmtx", hmtx);
    const Glyph *const glyphs = getBufferHead (font->glyphs);
    const Glyph *const glyphsEnd = getBufferTail (font->glyphs);
    for (const Glyph *glyph = glyphs; glyph < glyphsEnd; glyph++)
    {
        int_fast16_t aw = glyph->combining ? 0 : PW (glyph->byteCount);
        cacheU16 (hmtx, FU (aw)); // advanceWidth
        cacheU16 (hmtx, FU (glyph->lsb)); // lsb
    }
}

void
fillCmapTable (Font *font)
{
    Glyph *const glyphs = getBufferHead (font->glyphs);
    Buffer *rangeHeads = newBuffer (16);
    uint_fast32_t rangeCount = 0;
    uint_fast32_t bmpRangeCount = 1; // 1 for the last 0xffff-0xffff range
    glyphs[0].codePoint = glyphs[1].codePoint; // to start a range at glyph 1
    for (uint_fast16_t i = 1; i < font->glyphCount; i++)
    {
        if (glyphs[i].codePoint != glyphs[i - 1].codePoint + 1)
        {
            storeU16 (rangeHeads, i);
            rangeCount++;
            bmpRangeCount += glyphs[i].codePoint < 0xffff;
        }
    }
    Buffer *cmap = newBuffer (256);
    addTable (font, "cmap", cmap);
    // Format 4 table is always generated for compatibility.
    bool hasFormat12 = glyphs[font->glyphCount - 1].codePoint > 0xffff;
    cacheU16 (cmap, 0); // version
    cacheU16 (cmap, 1 + hasFormat12); // numTables
    { // encodingRecords[0]
        cacheU16 (cmap, 3); // platformID
        cacheU16 (cmap, 1); // encodingID
        cacheU32 (cmap, 12 + 8 * hasFormat12); // subtableOffset
    }
    if (hasFormat12) // encodingRecords[1]
    {
        cacheU16 (cmap, 3); // platformID
        cacheU16 (cmap, 10); // encodingID
        cacheU32 (cmap, 36 + 8 * bmpRangeCount); // subtableOffset
    }
    const uint_least16_t *ranges = getBufferHead (rangeHeads);
    const uint_least16_t *const rangesEnd = getBufferTail (rangeHeads);
    storeU16 (rangeHeads, font->glyphCount);
    { // format 4 table
        cacheU16 (cmap, 4); // format
        cacheU16 (cmap, 16 + 8 * bmpRangeCount); // length
        cacheU16 (cmap, 0); // language
        if (bmpRangeCount * 2 > U16MAX)
            fail ("Too many ranges in 'cmap' table.");
        cacheU16 (cmap, bmpRangeCount * 2); // segCountX2
        uint_fast16_t searchRange = 1, entrySelector = -1;
        while (searchRange <= bmpRangeCount)
        {
            searchRange <<= 1;
            entrySelector++;
        }
        cacheU16 (cmap, searchRange); // searchRange
        cacheU16 (cmap, entrySelector); // entrySelector
        cacheU16 (cmap, bmpRangeCount * 2 - searchRange); // rangeShift
        { // endCode[]
            const uint_least16_t *p = ranges;
            for (p++; p < rangesEnd && glyphs[*p].codePoint < 0xffff; p++)
                cacheU16 (cmap, glyphs[*p - 1].codePoint);
            uint_fast32_t cp = glyphs[*p - 1].codePoint;
            if (cp > 0xfffe)
                cp = 0xfffe;
            cacheU16 (cmap, cp);
            cacheU16 (cmap, 0xffff);
        }
        cacheU16 (cmap, 0); // reservedPad
        { // startCode[]
            for (uint_fast32_t i = 0; i < bmpRangeCount - 1; i++)
                cacheU16 (cmap, glyphs[ranges[i]].codePoint);
            cacheU16 (cmap, 0xffff);
        }
        { // idDelta[]
            const uint_least16_t *p = ranges;
            for (; p < rangesEnd && glyphs[*p].codePoint < 0xffff; p++)
                cacheU16 (cmap, *p - glyphs[*p].codePoint);
            uint_fast16_t delta = 1;
            if (p < rangesEnd && *p == 0xffff)
                delta = *p - glyphs[*p].codePoint;
            cacheU16 (cmap, delta);
        }
        { // idRangeOffsets[]
            for (uint_least16_t i = 0; i < bmpRangeCount; i++)
                cacheU16 (cmap, 0);
        }
    }
    if (hasFormat12) // format 12 table
    {
        cacheU16 (cmap, 12); // format
        cacheU16 (cmap, 0); // reserved
        cacheU32 (cmap, 16 + 12 * rangeCount); // length
        cacheU32 (cmap, 0); // language
        cacheU32 (cmap, rangeCount); // numGroups

        // groups[]
        for (const uint_least16_t *p = ranges; p < rangesEnd; p++)
        {
            cacheU32 (cmap, glyphs[*p].codePoint); // startCharCode
            cacheU32 (cmap, glyphs[p[1] - 1].codePoint); // endCharCode
            cacheU32 (cmap, *p); // startGlyphID
        }
    }
    freeBuffer (rangeHeads);
}

void
fillPostTable (Font *font)
{
    Buffer *post = newBuffer (32);
    addTable (font, "post", post);
    cacheU32 (post, 0x00030000); // version = 3.0
    cacheU32 (post, 0); // italicAngle
    cacheU16 (post, 0); // underlinePosition
    cacheU16 (post, 1); // underlineThickness
    cacheU32 (post, 1); // isFixedPitch
    cacheU32 (post, 0); // minMemType42
    cacheU32 (post, 0); // maxMemType42
    cacheU32 (post, 0); // minMemType1
    cacheU32 (post, 0); // maxMemType1
}

void
fillGposTable (Font *font)
{
    Buffer *gpos = newBuffer (16);
    addTable (font, "GPOS", gpos);
    cacheU16 (gpos, 1); // majorVersion
    cacheU16 (gpos, 0); // minorVersion
    cacheU16 (gpos, 10); // scriptListOffset
    cacheU16 (gpos, 12); // featureListOffset
    cacheU16 (gpos, 14); // lookupListOffset
    { // ScriptList table
        cacheU16 (gpos, 0); // scriptCount
    }
    { // Feature List table
        cacheU16 (gpos, 0); // featureCount
    }
    { // Lookup List Table
        cacheU16 (gpos, 0); // lookupCount
    }
}

void
fillGsubTable (Font *font)
{
    Buffer *gsub = newBuffer (38);
    addTable (font, "GSUB", gsub);
    cacheU16 (gsub, 1); // majorVersion
    cacheU16 (gsub, 0); // minorVersion
    cacheU16 (gsub, 10); // scriptListOffset
    cacheU16 (gsub, 34); // featureListOffset
    cacheU16 (gsub, 36); // lookupListOffset
    { // ScriptList table
        cacheU16 (gsub, 2); // scriptCount
        { // scriptRecords[0]
            cacheBytes (gsub, "DFLT", 4); // scriptTag
            cacheU16 (gsub, 14); // scriptOffset
        }
        { // scriptRecords[1]
            cacheBytes (gsub, "thai", 4); // scriptTag
            cacheU16 (gsub, 14); // scriptOffset
        }
        { // Script table
            cacheU16 (gsub, 4); // defaultLangSysOffset
            cacheU16 (gsub, 0); // langSysCount
            { // Default Language System table
                cacheU16 (gsub, 0); // lookupOrderOffset
                cacheU16 (gsub, 0); // requiredFeatureIndex
                cacheU16 (gsub, 0); // featureIndexCount
            }
        }
    }
    { // Feature List table
        cacheU16 (gsub, 0); // featureCount
    }
    { // Lookup List Table
        cacheU16 (gsub, 0); // lookupCount
    }
}

void
cacheStringAsUTF16BE (Buffer *buf, const char *str)
{
    for (const char *p = str; *p; p++)
    {
        byte c = *p;
        if (c < 0x80)
        {
            cacheU16 (buf, c);
            continue;
        }
        int length = 1;
        byte mask = 0x40;
        for (; c & mask; mask >>= 1)
            length++;
        if (length == 1 || length > 4)
            fail ("Ill-formed UTF-8 sequence.");
        uint_fast32_t codePoint = c & (mask - 1);
        for (int i = 1; i < length; i++)
        {
            c = *++p;
            if ((c & 0xc0) != 0x80) // NUL checked here
                fail ("Ill-formed UTF-8 sequence.");
            codePoint = (codePoint << 6) | (c & 0x3f);
        }
        const int lowerBits = length==2 ? 7 : length==3 ? 11 : 16;
        if (codePoint >> lowerBits == 0)
            fail ("Ill-formed UTF-8 sequence."); // sequence should be shorter
        if (codePoint >= 0xd800 && codePoint <= 0xdfff)
            fail ("Ill-formed UTF-8 sequence.");
        if (codePoint > 0x10ffff)
            fail ("Ill-formed UTF-8 sequence.");
        if (codePoint > 0xffff)
        {
            cacheU16 (buf, 0xd800 | (codePoint - 0x10000) >> 10);
            cacheU16 (buf, 0xdc00 | (codePoint & 0x3ff));
        }
        else
            cacheU16 (buf, codePoint);
    }
}
void
fillNameTable (Font *font, NameStrings nameStrings)
{
    Buffer *name = newBuffer (2048);
    addTable (font, "name", name);
    size_t nameStringCount = 0;
    for (size_t i = 0; i < MAX_NAME_IDS; i++)
        nameStringCount += !!nameStrings[i];
    cacheU16 (name, 0); // version
    cacheU16 (name, nameStringCount); // count
    cacheU16 (name, 2 * 3 + 12 * nameStringCount); // storageOffset
    Buffer *stringData = newBuffer (1024);
    // nameRecord[]
    for (size_t i = 0; i < MAX_NAME_IDS; i++)
    {
        if (!nameStrings[i])
            continue;
        size_t offset = countBufferedBytes (stringData);
        cacheStringAsUTF16BE (stringData, nameStrings[i]);
        size_t length = countBufferedBytes (stringData) - offset;
        if (offset > U16MAX || length > U16MAX)
            fail ("Name strings are too long.");
        // Platform ID 0 (Unicode) is not well supported.
        // ID 3 (Windows) seems to be the best for compatibility.
        cacheU16 (name, 3); // platformID = Windows
        cacheU16 (name, 1); // encodingID = Unicode BMP
        cacheU16 (name, 0x0409); // languageID = en-US
        cacheU16 (name, i); // nameID
        cacheU16 (name, length); // length
        cacheU16 (name, offset); // stringOffset
    }
    cacheBuffer (name, stringData);
    freeBuffer (stringData);
}

/*
    Print program version if invoked with the "--version" option,
    and then exit successfully.
*/
void
printVersion () {
    printf ("hex2otf (GNU Unifont) %s\n", VERSION);
    printf ("Copyright © 2022 何志翔 (He Zhixiang)\n");
    printf ("License GPLv2+: GNU GPL version 2 or later\n");
    printf ("<https://gnu.org/licenses/gpl.html>\n");
    printf ("This is free software: you are free to change and\n");
    printf ("redistribute it.  There is NO WARRANTY, to the extent\n");
    printf ("permitted by law.\n");

    exit (EXIT_SUCCESS);
}

/*
    Print help message if invoked with the "--help" option,
    and then exit successfully.
*/
void
printHelp () {
    printf ("Synopsis: hex2otf <options>:\n\n");
    printf ("    hex=<filename>        Specify Unifont .hex input file.\n");
    printf ("    pos=<filename>        Specify combining file. (Optional)\n");
    printf ("    out=<filename>        Specify output font file.\n");
    printf ("    format=<f1>,<f2>,...  Specify font format(s); values:\n");
    printf ("                             cff\n");
    printf ("                             cff2\n");
    printf ("                             truetype\n");
    printf ("                             blank\n");
    printf ("                             bitmap\n");
    printf ("                             gpos\n");
    printf ("                             gsub\n");
    printf ("\nExample:\n\n");
    printf ("    hex2otf hex=Myfont.hex out=Myfont.otf format=cff\n\n");
    printf ("For more information, consult the hex2otf(1) man page.\n\n");

    exit (EXIT_SUCCESS);
}

typedef struct Options
{
    bool truetype, blankOutline, bitmap, gpos, gsub;
    int cff; // 0 = no CFF outline; 1 = use 'CFF' table; 2 = use 'CFF2' table
    const char *hex, *pos, *out; // file names
    NameStrings nameStrings; // indexed directly by Name IDs
} Options;

const char *
matchToken (const char *operand, const char *key, char delimiter)
{
    while (*key)
        if (*operand++ != *key++)
            return NULL;
    if (!*operand || *operand++ == delimiter)
        return operand;
    return NULL;
}

Options
parseOptions (char *const argv[const])
{
    Options opt = {0}; // all options default to 0, false and NULL
    const char *format = NULL;
    struct StringArg
    {
        const char *const key;
        const char **const value;
    } strArgs[] =
    {
        {"hex", &opt.hex},
        {"pos", &opt.pos},
        {"out", &opt.out},
        {"format", &format},
        {NULL, NULL} // sentinel
    };
    for (char *const *argp = argv + 1; *argp; argp++)
    {
        const char *const arg = *argp;
        struct StringArg *p;
        const char *value = NULL;
        if (strcmp (arg, "--help") == 0)
            printHelp ();
        if (strcmp (arg, "--version") == 0)
            printVersion ();
        for (p = strArgs; p->key; p++)
            if ((value = matchToken (arg, p->key, '=')))
                break;
        if (p->key)
        {
            if (!*value)
                fail ("Empty argument: '%s'.", p->key);
            if (*p->value)
                fail ("Duplicate argument: '%s'.", p->key);
            *p->value = value;
        }
        else // shall be a name string
        {
            char *endptr;
            unsigned long id = strtoul (arg, &endptr, 10);
            if (endptr == arg || id >= MAX_NAME_IDS || *endptr != '=')
                fail ("Invalid argument: '%s'.", arg);
            endptr++; // skip '='
            if (opt.nameStrings[id])
                fail ("Duplicate name ID: %lu.", id);
            opt.nameStrings[id] = endptr;
        }
    }
    if (!opt.hex)
        fail ("Hex file is not specified.");
    if (opt.pos && opt.pos[0] == '\0')
        opt.pos = NULL; // Position file is optional. Empty path means none.
    if (!opt.out)
        fail ("Output file is not specified.");
    if (!format)
        fail ("Format is not specified.");
    for (const NamePair *p = defaultNames; p->str; p++)
        if (!opt.nameStrings[p->id])
            opt.nameStrings[p->id] = p->str;
    bool cff = false, cff2 = false;
    struct Symbol
    {
        const char *const key;
        bool *const found;
    } symbols[] =
    {
        {"cff", &cff},
        {"cff2", &cff2},
        {"truetype", &opt.truetype},
        {"blank", &opt.blankOutline},
        {"bitmap", &opt.bitmap},
        {"gpos", &opt.gpos},
        {"gsub", &opt.gsub},
        {NULL, NULL} // sentinel
    };
    while (*format)
    {
        const struct Symbol *p;
        const char *next = NULL;
        for (p = symbols; p->key; p++)
            if ((next = matchToken (format, p->key, ',')))
                break;
        if (!p->key)
            fail ("Invalid format.");
        *p->found = true;
        format = next;
    }
    if (cff + cff2 + opt.truetype + opt.blankOutline > 1)
        fail ("At most one outline format can be accepted.");
    if (!(cff || cff2 || opt.truetype || opt.bitmap))
        fail ("Invalid format.");
    opt.cff = cff + cff2 * 2;
    return opt;
}

int
main (int argc, char *argv[])
{
    initBuffers (16);
    atexit (cleanBuffers);
    Options opt = parseOptions (argv);
    Font font;
    font.tables = newBuffer (sizeof (Table) * 16);
    font.glyphs = newBuffer (sizeof (Glyph) * MAX_GLYPHS);
    readGlyphs (&font, opt.hex);
    sortGlyphs (&font);
    enum LocaFormat loca = LOCA_OFFSET16;
    uint_fast16_t maxPoints = 0, maxContours = 0;
    pixels_t xMin = 0;
    if (opt.pos)
        positionGlyphs (&font, opt.pos, &xMin);
    if (opt.gpos)
        fillGposTable (&font);
    if (opt.gsub)
        fillGsubTable (&font);
    if (opt.cff)
        fillCFF (&font, opt.cff, opt.nameStrings);
    if (opt.truetype)
        fillTrueType (&font, &loca, &maxPoints, &maxContours);
    if (opt.blankOutline)
        fillBlankOutline (&font);
    if (opt.bitmap)
        fillBitmap (&font);
    fillHeadTable (&font, loca, xMin);
    fillHheaTable (&font, xMin);
    fillMaxpTable (&font, opt.cff, maxPoints, maxContours);
    fillOS2Table (&font);
    fillNameTable (&font, opt.nameStrings);
    fillHmtxTable (&font);
    fillCmapTable (&font);
    fillPostTable (&font);
    organizeTables (&font, opt.cff);
    writeFont (&font, opt.cff, opt.out);
    return EXIT_SUCCESS;
}
