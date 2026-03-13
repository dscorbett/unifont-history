
# hexadecimate.awk - divide Unifont .hex input into 16-by-16 glyph grid images
#
# Copyright (C) 2026 Paul Hardy
# License: GPL 2+
#

#
# LICENSE:
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#  
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
#    GNU General Public License for more details.
#  
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

#
# hexadecimate-bmp.awk - convert Unifont .hex format input into
# Bitmap (BMP) format glyph images on a 16-by-16 glyph grid.
# N.B.: Input .hex records MUST be in ascending sorted order.
#
# This needs to be run from a subdirectory where plane00..plane10
# will be created by this script, which will then contain the
# bitmaps for each plane.
#
# The input to this file should start wtih a first line of the form
#
#    BINDIR:<path-to-bin>
#    GRAPHICS:bmp        --or-- GRAPHICS:png
#    PERL:<path-to-perl> --or-- not included
#
# The BINDIR entry points to the location of the Unifont utilities
# that this package builds.
#
# <path-to-file> may be a relative or absolute path, and must end with
# a '/'.  This is a kludge to pass an environment variable to a non-GNU
# version of make that lacks support for reading environment variables.
#
# If the BINDiR line is omitted from the input, the PATH environment
# variable will be searched for any binary executables.
#
# Subsequent lines should be glyphs encoded in Unifont .hex format:
#
#    <unicode-code-poinnt>:<unifont-bitmap>
# 
# where <unicode-code-point> is a four-, five-, or six-digit
# hexadecimal code point.  This script will create output files
# organized by Unicode plane, for Planes 0 through 10 (headecimal).
#
# The output of this script should be piped to /bin/sh to create
# the output graphics files.
#
# See the Makefile in this directory for an example of use.
#
BEGIN {
	thisplane = "xx";
	thispage  = "xx";
	bindir = "";
	graphics = "bmp";
	perlpath = "perl";
	for (i = 0; i < 17; i++) {
	   printf("mkdir plane%02X\n", i);
	}
}
$1 ~ /BINDIR/ {
	if (NF > 1) {
	   bindir = $2;
	}
	next;
}
$1 ~ /GRAPHICS/ {
	if (NF > 1) {
	   graphics = $2;
	}
	next;
}
$1 ~ /PERL/ {
	if (NF > 1) {
	   perlpath = $2;
	}
	next;
}
{
	codept = toupper($1);
	#
	# The Unicode codepoint should be 4 or 6 digits long to match
	#  Unifont .hex file conventions.  If a codepoint is five digits
	# long, prepend a leading zero.
	#
	if (length(codept) == 5) {
	   codept = "0" codept;
	}
	if (length($1) == 6) {
	   plane = toupper(substr(codept,1,2));
	   page  = toupper(substr(codept,3,2));
	   code  = toupper(substr(codept,5,2));
        }
	else if (length(codept) == 4) {
	   plane = "00";
	   page  = toupper(substr(codept,1,2));
	   code  = toupper(substr(codept,3,2));
	}
	# Invalid codepoint length; skip this record.
	else {
	   next;
	}
	if ((plane != thisplane) || page != thispage) {
	   if (plane != "00" || page != "00") {
	      print "EOF";
	   }
	   if (graphics == "png") {
	      printf ("%s %sunihex2png -p %s%s -o plane%s/uni%s%s.png << EOF\n",
	              perlpath, bindir, plane, page, plane, plane, page);
	   }
	   else {
	      printf ("%sunihex2bmp -p%s%s -oplane%s/uni%s%s.bmp << EOF\n",
	              bindir, plane, page, plane, plane, page);
	   }
	   printf("%s:%s\n", $1, $2);  
	   thisplane = plane;
	   thispage  = page;
	}
	else {
	   printf("%s:%s\n", $1, $2);
	}
}
#
# End the last input file being piped to /bin/sh.
#
END	{
	print "EOF";
}
