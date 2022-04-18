/*
    hex2otf.h - Header file for hex2otf.c

    NOTE: It is a violation of the license terms of this software
    to delete license and copyright information below if creating
    a font derived from Unifont glyphs.
*/
#ifndef _HEX2OTF_H_
#define _HEX2OTF_H_

#define UNIFONT_VERSION "14.0.03"

#define DEFAULT_ID0 "Copyright © 1998-2022 Roman Czyborra, Paul Hardy, \
Qianqian Fang, Andrew Miller, Johnnie Weaver, David Corbett, \
Nils Moskopp, Rebecca Bettencourt, et al."

#define DEFAULT_ID1 "Unifont"
#define DEFAULT_ID2 "Regular"
#define DEFAULT_ID5 "Version "UNIFONT_VERSION
#define DEFAULT_ID11 "https://unifoundry.com/unifont/"

#define DEFAULT_ID13 "Dual license: SIL Open Font License version 1.1, \
and GNU GPL version 2 or later with the GNU Font Embedding Exception."

#define DEFAULT_ID14 "http://unifoundry.com/LICENSE.txt, \
https://scripts.sil.org/OFL"

typedef struct NamePair
{
    int id;
    const char *str;
} NamePair;

#define NAMEPAIR(n) {(n), DEFAULT_ID##n}

const NamePair defaultNames[] =
{
    NAMEPAIR (0), // required (used in CFF)
    NAMEPAIR (1), // required (used in CFF)
    NAMEPAIR (2),
    NAMEPAIR (5),
    NAMEPAIR (11),
    NAMEPAIR (13),
    NAMEPAIR (14),
    {0, NULL} // sentinel
};

#undef NAMEPAIR

#endif
