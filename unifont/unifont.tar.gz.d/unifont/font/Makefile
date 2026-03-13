#
# Makefile to build a version of GNU Unifont with the
# unifoundry.com GNU Unifont utilities.  This Makefile
# converts unifont-$(VERSION).hex and related files into
# a set of final GNU Unifont font files.
#
# This software is released under version 2.0 of the GNU Public
# License, or (at your option) a later version of the GPL.
#
# Paul Hardy, 7 July 2008
#
# Commands will be interpreted with the Bourne Shell.
#
SHELL = /bin/sh
INSTALL = install
GZFLAGS = -f -9 -n

# FONTFORGE is only used if building TrueType fonts, not OpenType.
FONTFORGE = fontforge

PERL = 

PACKAGE = "unifont"
VERSION = 17.0.04

COPYRIGHT = "Copyright (C) 1998-2026 Roman Czyborra, Paul Hardy, \
Qianqian Fang, Andrew Miller, Johnnie Weaver, David Corbett, \
Ælla Chiana Moskopp, Rebecca Bettencourt, Minseo Lee, Ho-Seok Ee, et al. \
License: SIL Open Font License version 1.1 and GPLv2+: \
GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html> \
with the GNU Font Embedding Exception."

# Path to local Unifont sources.
SRCDIR ?= ../src

#
# Path to local unifont-bin utilities.  If your system doesn't
# use GNU's version of "make", use the first BINDIR definition.
# If using GNU's "make", use the second BINDIR definition.
#
BINDIR = ../bin
# BINDIR = $(CURDIR)/../bin
#
# The top-level directories for installing fonts on the system, and
# the installation directories for PCF, OpenType, and TrueType fonts.
#
USRDIR = usr
# USRDIR = usr/local

PREFIX      = $(DESTDIR)/$(USRDIR)
CONSOLEDEST = $(PREFIX)/share/consolefonts
FONTDEST    = $(PREFIX)/share/fonts
PCFDEST     = $(FONTDEST)/X11/misc
TTFDEST     = $(FONTDEST)/truetype/unifont
OTFDEST     = $(FONTDEST)/opentype/unifont
#
# Destination directory for compiled fonts.
#
COMPILED_DIR = compiled
#
# Destination directory for .bmp or .png representation of font
# as a 16-by-16 glyph array.  These image files are created for
# modification with graphics programs but not installed in a
# system directory.
#
BMPDIR = $(COMPILED_DIR)/bmp
PNGDIR = $(COMPILED_DIR)/png
#
# Choose one of these two graphics format for 16-by-16 glyph grids.
# GRAPHICS must be set to one of these; it cannot be blank for this
# make file to complete correctly.  If you don't want either format,
# use the BMP format because it builds faster thant he PNG format.
#
#GRAPHICS=png
GRAPHICS=bmp

GRAPHICS_DIR = $(COMPILED_DIR)/$(GRAPHICS)

#
# If you don't want to include blank glyphs in unassigned code points,
# uncomment the UNASSIGNED definition or override from the make command line.
#
UNASSIGNED = plane00/plane00-unassigned.hex
# UNASSIGNED =

#
# Non-printing glyphs.  There are only about 100 of these, and many end
# users do want to print representations of these glyphs, so they are
# included as an optional assignment.
#
NONPRINTING = plane00/plane00-nonprinting.hex
# NONPRINTING =

#
# Private Use Area glyphs.  Uncomment one of the lines below to include:
#
#   - Four-digit hexadecimal glyphs "pua.hex"
#   - Hangul Johab 6/3/1 glyphs in "hangul-base.hex", or
#   - Override PUA definition from the make command line.
#
# Note that selecting any of these options will override the use
# of the ConScript Unicode Registry (CSUR) and Under CSUR (UCSUR)
# glyphs from being part of a Unifont PUA section in a font.
#
HANGUL_PUA = plane00/hangul/hangul-base.hex
#
# PUA = plane00/pua.hex
# PUA = $(HANGUL_PUA)
PUA = 

#
# The remaining .hex files will be constant unless a customized font
# is being built.  Note that COPYLEFT contains the encoding for U+01F12F,
# which is in Unicode Plane 1 rather than Plane 0.  It is included in the
# base Plane 0 Unifont TTF font as an exception because of its significance.
#

#
# This is the main collection of glyphs, apart from the CJK glyphs
# from Wen Quan Yi and the Hangul syllables.
#
UNIFONTBASE = plane00/unifont-base.hex

#
# These are the files for building GNU Unifont with Qianqian Fang's
# Wen Quan Yi CJK ideographs.  This version provides complete coverage
# of the Unicode Basic Multilingual Plane.
#
CJK         = plane00/wqy.hex

#
# Hangul syllables, generated from files in plane00/hangul.
#
HANGUL      = plane00/hangul-syllables.hex

#
# Space glyphs are in a separate file because the graphics-to-Unifont
# hex format utilities doesn't know if they are single- or double-spaced.
#
SPACES      = plane00/spaces.hex

#
# This file is empty, but could allow for user overrides of individual
# glyphs from the basic Plane 0 set.
#
CUSTOM00    = plane00/custom00.hex

#
# Copyleft symbol, from Plane 1.
#
COPYLEFT    = plane00/copyleft.hex

#
# The set of Plane 0 Unifont glyphs from the above definitions.
#
UNIFILES = $(UNIFONTBASE) $(CJK) $(HANGUL) $(SPACES) $(UNASSIGNED) \
	   $(NONPRINTING) $(PUA) $(CUSTOM00)

#
# All Plane 0 glyphs except for the JIS X 0213 glyph file, for
# creating the unifont_sample file.
#
PLANE0 = copyleft.hex custom00.hex hangul-syllables.hex \
	 omit.hex plane00-nonprinting.hex plane00-unassigned.hex \
	 pua.hex spaces.hex unifont-base.hex wqy.hex

#
# JIS X 0213 hex files for Japanese.  The plane00 file contains glyphs
# that will replace those in the default Chinese version of Unifont.
#
# JISHEX = plane00/izmg16-plane00.hex plane02/izmg16-plane02.hex

#
# Locations of all Plane 2 and Plane 3 Japanese and Chinese glyphs.
#
# PLANE_2_JP_HEX = plane02/izmg16-plane02.hex
# PLANE_2_ZH_HEX = plane02/zh-plane02.hex
# PLANE_3_JP_HEX = plane03/jp-plane03.hex
# PLANE_3_ZH_HEX = plane03/zh-plane03.hex

#
# Plane 2 and Plane 3 Japanese (JP_UPPER_SUBSET_HEX) and
# Chinese (ZH_UPPER_SUBSET_HEX) collections for building
# composite upper-plane font files.
#
JP_UPPER_SUBSET_HEX = $(COMPILED_DIR)/jp-upper-subset.hex
ZH_UPPER_SUBSET_HEX = $(COMPILED_DIR)/zh-upper-subset.hex
JP_UPPER_SUPERSET_HEX = $(COMPILED_DIR)/jp++.hex
ZH_UPPER_SUPERSET_HEX = $(COMPILED_DIR)/zh++.hex

#
# Planes 1 through 14 (0x0E) are ordinary;
# Planes 15 (0x0F) and 16 (0x10) are Private Use Area
#
# UPPER_FILES = plane0[1E]/*.hex

#
# Definitions for building unifont_all-$(VERSION).hex
#
ALL_HEX_PLANE00 =     plane00/unifont-base.hex \
	              plane00/hangul-syllables.hex \
	              plane00/izmg16-plane00.hex \
	              plane00/plane00-nonprinting.hex \
	              plane00/plane00-unassigned.hex \
	              plane00/spaces.hex \
	              plane00/wqy.hex

ALL_HEX_PLANE00CSUR = plane00csur/plane00csur-nonprinting.hex \
	              plane00csur/noscript.hex \
	              plane00csur/plane00csur-spaces.hex \
	              plane00csur/plane00csur-unassigned.hex \
	              plane00csur/plane00csur.hex

ALL_HEX_PLANE01 =     plane01/plane01-nonprinting.hex \
	              plane01/plane01-noscript.hex \
	              plane01/plane01-space.hex \
	              plane01/plane01-unassigned.hex \
	              plane01/plane01.hex

ALL_HEX_PLANE02 =     plane02/izmg16-plane02.hex \
	              plane02/zh-plane02.hex

ALL_HEX_PLANE03 =     plane03/jp-plane03.hex \
	              plane03/zh-plane03.hex

ALL_HEX_PLANE0E =     plane0E/plane0E-nonprinting.hex \
	              plane0E/plane0E-unassigned.hex \
	              plane0E/plane0E.hex

ALL_HEX_PLANE0FCSUR = plane0Fcsur/plane0Fcsur-nonprinting.hex \
	              plane0Fcsur/plane0Fcsur-spaces.hex \
	              plane0Fcsur/plane0Fcsur-unassigned.hex \
	              plane0Fcsur/plane0Fcsur.hex

ALL_HEX_LOWER = $(ALL_HEX_PLANE00) $(ALL_HEX_PLANE00CSUR)

ALL_HEX_UPPER = $(ALL_HEX_PLANE01) $(ALL_HEX_PLANE02) $(ALL_HEX_PLANE03) \
	        $(ALL_HEX_PLANE0E) $(ALL_HEX_PLANE0FCSUR)

#
# This allows for a custom set of Plane 0 Private Use Area
# glyphs to be specified on the make command line, or defined
# in this make file.
#
ALL_HEX = $(ALL_HEX_LOWER) $(PUA) $(ALL_HEX_UPPER)

#
# All Constructed Language Script (ConScript) Unicode Registry
# (CSUR) glyphs, as maintained by Rebecca Bettencourt.
#
ALL_HEX_CSUR = $(ALL_HEX_PLANE00CSUR) $(ALL_HEX_PLANE0FCSUR)


#
# Location of the files containing lists of Unicode combining characters.
#
COMBINING = plane00/plane00-combining.txt

LOWER_COMBINING = plane00/plane00-combining.txt \
	          plane00csur/plane00csur-combining.txt

UPPER_COMBINING = plane0[1-E]/*combining.txt

CSUR_COMBINING  = plane00csur/plane00csur-combining.txt \
                  plane0Fcsur/plane0Fcsur-combining.txt

#
# All combining information files.
#
COMBINING_FILES = plane00/plane00-combining.txt \
	          plane00csur/plane00csur-combining.txt \
	          plane01/plane01-combining.txt \
	          plane0E/plane0E-combining.txt \
	          plane0Fcsur/plane0Fcsur-combining.txt

#
# Location of OpenType (OTF) & TrueType (TTF)  source directories,
# where OTF and TTF fonts are built, respectively.
#
OTFSRC = otfsrc
TTFSRC = ttfsrc

#
# Only use the PSF target if on GNU/Linux, not on BSD, because
# a BSD console font only allows 256 code points, not 512.
# However, this make file builds the PSF font to generate the
# superset of fonts for all systems.
#
PSF = psf
# PSF =


#
# End of preliminary definitions.  Now build the font files.
#

all: $(COMPILED_DIR)

#
# The $(COMPILED_DIR) goal uses dependencies on these targets:
#
# hex-stamp          marker that all .hex files are built
# bdf-stamp          marker that all BDF font files are built
# pcf-stamp          marker that the PCF font file is built
# psf-stamp          marker that the PSF font file is built
# html-stamp         marker that HTML coverage pages are built
# $(GRAPHICS)-stamp  marker that the BMP or PNG glyph grids are built
#                       (this expands to "bmp-stamp" or "png-stamp")
# opentype-stamp     marker that the OpenType font files are built
# bigpic-stamp       marker that the 64k glyph plane charts are built
# thumbnails-stamp   marker that thumbnails were created from the 64k glyphs
# coverage-stamp     marker that the HTML coverage pages are built
#
# ========================================
# Other stamps that this Makefile creates:
# ========================================
#
# dir-stamp          marker that top-level font build directories exist
#                    and that ../bin has been built
#
# Stamps for 16-by-16 grids of glyph images:
#
#    glyph-csur-stamp    marker for CSUR glyph grid set
#    glyph-dirs-stamp    marker for glyph grid set directory existence;
#                           (these are subdirectories under "bmp" or "png")
#    glyph-jp-stamp      marker for Japanese glyph grid images
#    glyph-t-stamp       marker for Unicode T-source CJK glyph grid images
#    glyph-wrapup-stamp  marker for final tweaking of glyph grids
#    glyph-zh-stamp      marker for Unicode G-source glyph grid images
#
# graphawks-stamp     marker that "graphawks-env" has been created, for
#                     running "font/hexadecimate.awk"
#
# Stamps that are dependencies of opentype-stamp:
#
#    otf-csur-stamp     CSUR OpenType fonts are built
#    otf-sample-stamp   Unifont sample book OpenType fonts are built
#    otf-stamp          Unifont Plane 0 OpenType fonts are built
#    otf-upper-stamp    Unifont upper Unicode plane fonts are built
#
# Stamps for TrueType fonts (not invoked by default):
#
#    truetype-stamp     Top-level TrueType font stamp; all are built
#    ttf-csur-stamp     CSUR TrueType fonts are built
#    ttf-stamp          Unifont Plane 0 TrueType fonts are built
#    ttf-upper-stamp    Unifont upper Unicode plane TrueType fonts are built
#

#
# A high-level sequence of events follows.  First, $(COMPILED_DIR) must
# be created via the dir-stamp target to hold the built font files.
# Levels of indentation represent predecessor build dependencies, and
# can serve as a guide for parallel building.  This list of goals
# generally follows the list of dependencies for the $(COMPILED_DIR)
# target below.
#
#    * Build the Unifont ".hex" format files.  For Plane 0 and
#      Plane 1, include "sample book" versions showing combining
#      characters with dashed circles for the "bigpic" and glyph
#      grid image files.  Finally,create the "hex-stamp" sentinel
#      file as the synchronization point for all dependent goals
#      listed below.
#      |
#      +--+ Build the BDF font files from the .hex files.
#      |    |
#      |    +--- Build the PCF font files from the BDF font files.
#      |    |
#      |    +--- Build the PSF APL console font from the BDF font files.
#      |
#      +--- Build the HTML tables showing Unicode plane coverage.
#      |
#      +--- Build the $(COMPILED_DIR)/bmp or $(COMPILED_DIR)/png graphics.
#      |
#      +--- Build the OpenType font files.
#      |
#      +--- Build the TrueType font files [not invoked by default].
#      |
#      +--- Build the "bigpic" glyph grids, one Unicode plane per big image.
#      |    |
#      |    +--- Create thumbnail images of the "bigpic" images.
#      |
#      +--- Create a coverage.txt format file showing Unicode block % coverage.
#
#    * Create a Unifont combining.txt format file that contains combining
#      character offset information for all combining code points in Unifont.
#      This is not dependent on the aggregate .hex files being built.
#
$(COMPILED_DIR): hex bdf pcf psf html \
	         $(GRAPHICS) opentype \
	         bigpic thumbnails coverage \
	         $(COMPILED_DIR)/unifont-combining-$(VERSION).txt

$(COMPILED_DIR)/unifont-combining-$(VERSION).txt: $(COMBINING_FILES)
	sort -u $(COMBINING_FILES) > $@

#
# Build the binary executables if not done already.
#
#
../bin-stamp:
	(cd .. ; make )

#
# Create the output directories if not present.
#
dir-stamp: ../bin-stamp
	if [ ! -d $(COMPILED_DIR) ] ; then \
	   mkdir -p $(COMPILED_DIR) ; \
	fi
	if [ ! -d precompiled ] ; then \
	   mkdir -p precompiled ; \
	fi
	touch $@

#
# Build the aggregate .hex font files
#
hex: hex-stamp

#
# This will update the hangul-syllables.hex file if the
# hangul-base.hex file has been updated.
#
# The hangul-syllables.hex input file is the only one that
# might get modified automatically.  All the others are as is.
# Proceed with forming all the combinations for the output
# .hex file.
#
# This is no longer invoked, in preparation for decoupling
# the jamo in font/plane00/hangul, which includes ancient
# as well as modern Hangul jamo, from the Hangul Syllables
# block (U+AC00..U+D7AF) in Unifont.
#
# plane00/hangul-syllables.hex: plane00/hangul/hangul-base.hex
# 	(cd plane00/hangul && make)

#
# The .hex files are created as a single goal because they build
# very quickly.  Trying to parallelize this woudld be overkill.
# In addition, most other targets trace back to hex-stamp as
# a prerequisite.
#
hex-stamp: dir-stamp \
	  ./build-cjk-upper \
	   $(ALL_HEX) $(COMBINING_FILES)
	# Create Unifont default hex file.
	sort $(UNIFILES) > $(COMPILED_DIR)/unifont-$(VERSION).hex
	# Create Tsource variation.
	awk -F: -f unipatch-hex.awk $(COMPILED_DIR)/unifont-$(VERSION).hex \
	   plane00/t-source.hex | sort |\
	   tee $(COMPILED_DIR)/unifont_t-$(VERSION).hex | \
	   $(BINDIR)/unigencircles \
	      $(COMBINING) plane00/plane00-nonprinting.hex \
              > $(COMPILED_DIR)/unifont_t_sample-$(VERSION).hex
	# Create Unifont sample version with combining circles.
	(cd plane00 && sort $(PLANE0)) | \
	   egrep -v "^FFF[EF]" | grep -v "^01F12F:" | \
	      $(BINDIR)/unigencircles $(COMBINING) \
	      plane00/plane00-nonprinting.hex \
	      > $(COMPILED_DIR)/unifont_sample-$(VERSION).hex
	# Create Chinese and Japanese versions of Plane 2 and Plane 3 glyphs.
	./build-cjk-upper
	# Prepare upper-plane glyphs.
	sort $(UPPER_COMBINING) > $(COMPILED_DIR)/upper_combining.txt
	sort plane0[1-E]/plane*-nonprinting.hex \
	   > $(COMPILED_DIR)/upper_nonprinting.hex
	# Generate files in plane01/all directory to create font sample book.
	(cd plane01 && make)
	sort plane01/plane01-all.hex plane02/zh-plane02.hex \
		plane0[3-E]/*.hex | \
	   grep -v "^0020:"       | \
	   egrep -v "^..FFF[EF]"  | \
	   $(BINDIR)/unigencircles $(COMPILED_DIR)/upper_combining.txt \
	   $(COMPILED_DIR)/upper_nonprinting.hex \
	   > $(COMPILED_DIR)/unifont_upper_sample-$(VERSION).hex
	# Build upper plane hex files with Chinese & Japanese variants.
	cp  plane01/plane01-all.hex  \
	    $(COMPILED_DIR)/unifont_upper-$(VERSION).hex
	cat $(ZH_UPPER_SUPERSET_HEX) \
	    >> $(COMPILED_DIR)/unifont_upper-$(VERSION).hex
	sort -u $(ALL_HEX_PLANE0E)   \
	    >> $(COMPILED_DIR)/unifont_upper-$(VERSION).hex
	# Create a .hex file with CSUR glyphs, without combining circles
	(cd plane00csur && make)
	grep -v "^0020:" plane00csur/plane00csur-all.hex | \
	   sort - $(COMPILED_DIR)/unifont-$(VERSION).hex \
	      > $(COMPILED_DIR)/unifont_csur_lower-$(VERSION).hex
	# Create a .hex file with only CSUR glyphs from all planes
	sort plane00csur/plane00csur-all.hex | sed -e 's/^/00/' \
	   > $(COMPILED_DIR)/unifont_csur-$(VERSION).hex
	(cd plane0Fcsur && make)
	sort plane0[1-F]csur/*.hex | grep -v "^0020:" \
	   >> $(COMPILED_DIR)/unifont_csur-$(VERSION).hex
	# Create a .hex file with all CSUR glyphs, with combining circles
	sort plane00csur/plane00csur-combining.txt \
	   >  $(COMPILED_DIR)/csur_combining.txt
	sort plane0[1-F]csur/plane0[1-F]csur-combining.txt \
	   >> $(COMPILED_DIR)/csur_combining.txt
	$(BINDIR)/unigencircles \
	     $(COMPILED_DIR)/csur_combining.txt \
	     plane00/plane00-nonprinting.hex \
	   < $(COMPILED_DIR)/unifont_csur-$(VERSION).hex \
	   > $(COMPILED_DIR)/unifont_csur_sample-$(VERSION).hex
	# Build Japanese version with glyphs from JIS X 0213:2004
	awk -F: -f unipatch-hex.awk $(COMPILED_DIR)/unifont-$(VERSION).hex \
	   plane00/izmg16-plane00.hex | sort |\
	   tee $(COMPILED_DIR)/unifont_jp-$(VERSION).hex | \
	   $(BINDIR)/unigencircles \
	      $(COMBINING) plane00/plane00-nonprinting.hex \
              > $(COMPILED_DIR)/unifont_jp_sample-$(VERSION).hex
	# Build upper plane hex files with Chinese & Japanese variants.
	cat $(JP_UPPER_SUBSET_HEX)   \
	    >> $(COMPILED_DIR)/unifont_jp-$(VERSION).hex
	#
	# Create the unifont_all-$(VERSION).hex file, covering all assigned
	# Unifont code points (including Private Use Area glyphs).
	#
	# First sort all the Plane 0 glyphs, and then add higher plane glyphs
	# to ensure that the Plane 0 glyphs all appear first.
	#
	# The grep -v "^0020:" removes the space glyph in plane01/plane01.hex,
	# which is there to provide a space glyph in an upper-plane only font.
	#
	sort -u -k 1,1 -t ':' $(ALL_HEX_LOWER) \
	       > $(COMPILED_DIR)/unifont_all-$(VERSION).hex
	cat $(ALL_HEX_UPPER) | grep -v "^0020:" | \
	   sort -u -k 1,1 -t ':' \
	      >> $(COMPILED_DIR)/unifont_all-$(VERSION).hex
	touch $@

#
# Build a BDF font file from the final .hex files.
# A BDF font can only cover Plane 0 glyphs.
#
bdf: bdf-stamp

bdf-stamp: hex-stamp
	# First make the default BDF font.  The font name will be "unifont".
	$(BINDIR)/hex2bdf --version "$(VERSION)" --copyright $(COPYRIGHT) \
	   $(COMPILED_DIR)/unifont-$(VERSION).hex \
	   >$(COMPILED_DIR)/unifont-$(VERSION).bdf
	gzip $(GZFLAGS) <$(COMPILED_DIR)/unifont-$(VERSION).bdf \
	           >$(COMPILED_DIR)/unifont-$(VERSION).bdf.gz
	# Make the Tsource BDF font.
	$(BINDIR)/hex2bdf --font "Unifont Tsource" \
	   --version "$(VERSION)" --copyright $(COPYRIGHT) \
	   $(COMPILED_DIR)/unifont_t-$(VERSION).hex \
	   >$(COMPILED_DIR)/unifont_t-$(VERSION).bdf
	gzip $(GZFLAGS) <$(COMPILED_DIR)/unifont_t-$(VERSION).bdf \
	           >$(COMPILED_DIR)/unifont_t-$(VERSION).bdf.gz
	# Now make a version with combining circles.  The font name
	# will be "unifont_sample" instead of "unifont" to distinguish
	# it from the default font.
	$(BINDIR)/hex2bdf --font "Unifont Sample" \
	   --version "$(VERSION)" --copyright $(COPYRIGHT) \
	     $(COMPILED_DIR)/unifont_sample-$(VERSION).hex \
	   > $(COMPILED_DIR)/unifont_sample-$(VERSION).bdf
	gzip $(GZFLAGS) <$(COMPILED_DIR)/unifont_sample-$(VERSION).bdf \
	           >$(COMPILED_DIR)/unifont_sample-$(VERSION).bdf.gz
	# Font with ConScript Unicode Registry (CSUR) glyphs added
	$(BINDIR)/hex2bdf --font "Unifont CSUR" \
	   --version "$(VERSION)" --copyright $(COPYRIGHT) \
	     $(COMPILED_DIR)/unifont_csur_lower-$(VERSION).hex \
	   > $(COMPILED_DIR)/unifont_csur-$(VERSION).bdf
	gzip $(GZFLAGS) <$(COMPILED_DIR)/unifont_csur-$(VERSION).bdf \
	           >$(COMPILED_DIR)/unifont_csur-$(VERSION).bdf.gz
	# Font with glyphs above Plane 0
	$(BINDIR)/hex2bdf --font "Unifont Upper" \
	   --version "$(VERSION)" --copyright $(COPYRIGHT) \
	     $(COMPILED_DIR)/unifont_upper-$(VERSION).hex \
	   > $(COMPILED_DIR)/unifont_upper-$(VERSION).bdf
	gzip $(GZFLAGS) <$(COMPILED_DIR)/unifont_upper-$(VERSION).bdf \
	           >$(COMPILED_DIR)/unifont_upper-$(VERSION).bdf.gz
	# Font with glyphs above Plane 0 with combining circles added
	$(BINDIR)/hex2bdf --font "Unifont Upper Sample" \
	   --version "$(VERSION)" --copyright $(COPYRIGHT) \
	     $(COMPILED_DIR)/unifont_upper_sample-$(VERSION).hex \
	   > $(COMPILED_DIR)/unifont_upper_sample-$(VERSION).bdf
	gzip $(GZFLAGS) <$(COMPILED_DIR)/unifont_upper_sample-$(VERSION).bdf \
	           >$(COMPILED_DIR)/unifont_upper_sample-$(VERSION).bdf.gz
	# Font with Japanese glyphs from JIS X 0213 standard
	$(BINDIR)/hex2bdf --font "Unifont Japanese" \
	   --version "$(VERSION)" --copyright $(COPYRIGHT) \
	     $(COMPILED_DIR)/unifont_jp-$(VERSION).hex \
	   > $(COMPILED_DIR)/unifont_jp-$(VERSION).bdf
	gzip $(GZFLAGS) <$(COMPILED_DIR)/unifont_jp-$(VERSION).bdf \
	           >$(COMPILED_DIR)/unifont_jp-$(VERSION).bdf.gz
	# Same Japanese version, but only with Plane 0 glyphs for bdftopcf
	grep "^....:" $(COMPILED_DIR)/unifont_jp-$(VERSION).hex |\
	   $(BINDIR)/hex2bdf --font "Unifont Japanese" \
	      --version "$(VERSION)" --copyright $(COPYRIGHT) \
	      > $(COMPILED_DIR)/unifont_jp-$(VERSION).bdf
	gzip $(GZFLAGS) <$(COMPILED_DIR)/unifont_jp-$(VERSION).bdf \
	           >$(COMPILED_DIR)/unifont_jp-$(VERSION).bdf.gz
	touch $@

#
# Build PCF font files from the BDF files.
#
pcf: pcf-stamp

pcf-stamp: bdf-stamp
	bdftopcf <$(COMPILED_DIR)/unifont-$(VERSION).bdf \
	         >$(COMPILED_DIR)/unifont-$(VERSION).pcf
	gzip $(GZFLAGS) $(COMPILED_DIR)/unifont-$(VERSION).pcf
	bdftopcf <$(COMPILED_DIR)/unifont_t-$(VERSION).bdf \
	         >$(COMPILED_DIR)/unifont_t-$(VERSION).pcf
	gzip $(GZFLAGS) $(COMPILED_DIR)/unifont_t-$(VERSION).pcf
	bdftopcf <$(COMPILED_DIR)/unifont_jp-$(VERSION).bdf \
	         >$(COMPILED_DIR)/unifont_jp-$(VERSION).pcf
	gzip $(GZFLAGS) $(COMPILED_DIR)/unifont_jp-$(VERSION).pcf
	bdftopcf <$(COMPILED_DIR)/unifont_sample-$(VERSION).bdf \
	         >$(COMPILED_DIR)/unifont_sample-$(VERSION).pcf
	gzip $(GZFLAGS) $(COMPILED_DIR)/unifont_sample-$(VERSION).pcf
	bdftopcf <$(COMPILED_DIR)/unifont_csur-$(VERSION).bdf \
	         >$(COMPILED_DIR)/unifont_csur-$(VERSION).pcf
	gzip $(GZFLAGS) $(COMPILED_DIR)/unifont_csur-$(VERSION).pcf
	bdftopcf <$(COMPILED_DIR)/unifont_jp-$(VERSION).bdf \
	         >$(COMPILED_DIR)/unifont_jp-$(VERSION).pcf
	gzip $(GZFLAGS) $(COMPILED_DIR)/unifont_jp-$(VERSION).pcf
	touch $@

#
# Make the PSF (console) font for APL (A Programming Language).
# The resulting APL glyphs are compatible with GNU APL and other
# APL implementations that support Unicoe APL characters.
#
# This creates a console font with up to 512 glyphs.  GNU/Linux
# supports such console fonts; BSD only supports 256 glyphs in
# a console font.
#
psf: psf-stamp

psf-stamp: $(COMPILED_DIR)/Unifont-APL8x16-$(VERSION).psf.gz
	touch $@

$(COMPILED_DIR)/Unifont-APL8x16-$(VERSION).psf.gz: bdf-stamp
	bdf2psf --fb \
	   $(COMPILED_DIR)/unifont-$(VERSION).bdf \
	   psf/apl-equivalents.txt \
	   psf/unifont-apl.txt \
	   512 \
	   $(COMPILED_DIR)/Unifont-APL8x16-$(VERSION).psf
	gzip $(GZFLAGS) $(COMPILED_DIR)/Unifont-APL8x16-$(VERSION).psf

#
# This is not invoked; unifont_all-$(VERSION).hex is installed
# uncompressed so GRUB has access to the uncompressed glyphs.
#
$(COMPILED_DIR)/unifont_all-$(VERSION).hex.gz: hex-stamp
	gzip -9 $(COMPILED_DIR)/unifont_all-$(VERSION).hex

#
# Print coverage of all scripts in Unifont.  Combine all Plane 0
# directories first to keep everything ordered properly.  Only keep
# the space glyph from the Plane 0 Unifont source hex file; it is
# duplicated in other files used to create separate fonts.  Don't
# use plane00/pua.hex (U+E000..U+F8FF) because we are including
# Plane 0 CSUR glyphs.  The built unifont_csur_lower-$(VERSION).hex
# includes all non-Private Use Area Plane 0 glyphs, so start with it
# as a baseline.  Remove the Plane 1 Copyleft glyph from the Plane 0
# set of hex files.
#
coverage: coverage-stamp

coverage-stamp: $(COMPILED_DIR)/coverage-$(VERSION).txt
	touch $@

$(COMPILED_DIR)/coverage-$(VERSION).txt: coverage-$(VERSION).txt

coverage-$(VERSION).txt: coverage.dat hex-stamp
	$(BINDIR)/unicoverage \
	   -i$(COMPILED_DIR)/unifont_all-$(VERSION).hex \
	   -ocoverage-$(VERSION).txt
	install -p coverage-$(VERSION).txt $(COMPILED_DIR)

#
# Print HTML page coverage in Plane 0, the Basic Multilingual Plane
# (BMP).  This is no longer invoked by default because Unifont now
# provides 100% coverage of Unicode Plane 0.
#
pagecount: $(COMPILED_DIR)/pagecount.html

$(COMPILED_DIR)/pagecount.html: hex-stamp
	$(BINDIR)/unipagecount -l < $(COMPILED_DIR)/unifont-$(VERSION).hex \
		                  > $(COMPILED_DIR)/pagecount.html

#
# Note: the directory path can be relative or absolute,
# and must end with a "/".  Alternatively, the path
# can be completely blank.  In that case, unihex2bmp
# will be located from the user's PATH environment variable.
#
graphawks-env:
	@echo "Defining variables for hexadecimate.awk:"
	# Location of Unifont package executables.
	echo "BINDIR:../../../$(BINDIR)/"  > graphawks-env
	# The 256-glyph grid graphics format: "png" or "bmp".
	echo "GRAPHICS:$(GRAPHICS)"       >> graphawks-env
	# The location of the PERL executable, or just "perl".
	echo "PERL:$(PERL)"               >> graphawks-env

graphawks-stamp: graphawks-env
	touch $@

#
# Create the "*.bmp" (Bitmap format) versions of the glyphs
# with 256 glyphs per file in a 16-by-16 grid.
#
bmp: bmp-stamp

bmp-stamp: graphawks-stamp glyph-dirs-stamp \
	   glyph-zh-stamp glyph-jp-stamp glyph-csur-stamp \
	   glyph-wrapup-stamp
	touch $@

#
# Create the PNG (Portable Network Graphics) versions of the glyphs
# with 256 glyphs per file in a 16-by-16 glyph grid.
#
png: png-stamp

png-stamp: graphawks-stamp glyph-dirs-stamp \
	   glyph-t-stamp glyph-zh-stamp glyph-jp-stamp glyph-csur-stamp \
	   glyph-wrapup-stamp
	touch $@

#
# Within GRAPHICS_DIR, create subdirectories csur, jp, and zh.
# Then within each of those, create plane00..plane0F, plane10
# to account for all possibilities with input .hex files.
#
glyph-dirs-stamp: hex-stamp
	if [ ! -d $(GRAPHICS_DIR) ] ; then \
	   mkdir -p $(GRAPHICS_DIR) ; \
	fi
	# t, zh, jp, and csur are the initial build directories.
	# The rest of the directories are for the final distribution.
	# zh and jp intermediate directories are deleted with "make clean".
	for i in t zh jp csur ; do \
	   if [ ! -d $(GRAPHICS_DIR)/$$i ] ; then \
	      mkdir $(GRAPHICS_DIR)/$$i ; \
	   fi ; \
	done
	touch $@

#
# Create the Unicode Plane 0 T-source variation.
#
glyph-t-stamp: graphawks-stamp glyph-dirs-stamp ./hexadecimate.awk
	@echo "Building $(GRAPHICS_DIR)/t"
	( cd $(GRAPHICS_DIR)/t ; \
	   cat ../../../graphawks-env \
	       ../../unifont_t_sample-$(VERSION).hex | \
	   awk -F: -f ../../../hexadecimate.awk | \
	   sh \
	 )
	# Change the filenames to the form uni....-t.* in Plane 0.
	( cd $(GRAPHICS_DIR)/t/plane00 ; \
	   for j in uni*.$(GRAPHICS) ; do \
	      mv $$j `basename $$j .${GRAPHICS}`-t.$(GRAPHICS) ; \
	   done \
	)
	mv $(GRAPHICS_DIR)/t/plane00 $(GRAPHICS_DIR)/plane00-t
	touch $@

#
# Create the Unicode Plane 0, Plane 1, and Plane 14 default glyph images.
# Also create the Plane 2 and Plane 3 Chinese glyph images.
#
glyph-zh-stamp: graphawks-stamp glyph-dirs-stamp ./hexadecimate.awk
	@echo "Building $(GRAPHICS_DIR)/zh"
	( cd $(GRAPHICS_DIR)/zh ; \
	   cat ../../../graphawks-env \
	       ../../unifont_sample-$(VERSION).hex \
	       ../../unifont_upper_sample-$(VERSION).hex | \
	   awk -F: -f ../../../hexadecimate.awk | \
	   sh \
	 )
	# These are the default Unifont glyphs.  Move them into
	# their respective locations.
	mv $(GRAPHICS_DIR)/zh/plane00 $(GRAPHICS_DIR)
	mv $(GRAPHICS_DIR)/zh/plane01 $(GRAPHICS_DIR)
	mv $(GRAPHICS_DIR)/zh/plane0E $(GRAPHICS_DIR)
	# Change the filenames to the form uni....-zh.* in Planes 2 & 3.
	for i in 2 3 ; do \
	   ( cd $(GRAPHICS_DIR)/zh/plane0$$i ; \
	      for j in uni*.$(GRAPHICS) ; do \
	         mv $$j `basename $$j .${GRAPHICS}`-zh.$(GRAPHICS) ; \
	      done \
	   ) ; \
	done
	mv $(GRAPHICS_DIR)/zh/plane02 $(GRAPHICS_DIR)/plane02-zh
	mv $(GRAPHICS_DIR)/zh/plane03 $(GRAPHICS_DIR)/plane03-zh
	#
	# Fill in missing Plane 1 16-by-16 glyph grid images.
	#
	for i in 0 1 2 3 4 5 6 7 8 9 A B C D E F ; do \
	   for j in 0 1 2 3 4 5 6 7 8 9 A B C D E F ; do \
	      if [ ! -f $(GRAPHICS_DIR)/plane01/uni01$$i$$j.$(GRAPHICS) ] ; \
	         then \
	         if [ "x$(GRAPHICS)" == "xpng" ] ; then \
	            $(PERL) $(BINDIR)/unihex2png -p 1$$i$$j \
	               -o $(GRAPHICS_DIR)/plane01/uni01$$i$$j.png \
	               < /dev/null ; \
	         elif [ "x$(GRAPHICS)" == "xbmp" ] ; then \
	            $(BINDIR)/unihex2bmp -p$$i \
	               -o$(GRAPHICS_DIR)/plane01/uni01$$i$$j.bmp \
	               < /dev/null ; \
	         fi ; \
	      fi ; \
	   done ; \
	done
	#
	# Fill in missing Chinese Plane 2 and Plane 3 16-by-16 glyph grid images.
	#
	for p in 2 3 ; do \
	   for i in 0 1 2 3 4 5 6 7 8 9 A B C D E F ; do \
	      for j in 0 1 2 3 4 5 6 7 8 9 A B C D E F ; do \
	         if [ ! -f $(GRAPHICS_DIR)/plane0$$p-zh/uni0$$p$$i$$j-zh.$(GRAPHICS) ];\
	            then \
	            if [ "x$(GRAPHICS)" == "xpng" ] ; then \
	               $(PERL) $(BINDIR)/unihex2png -p $$p$$i$$j \
	                  -o $(GRAPHICS_DIR)/plane0$$p-zh/uni0$$p$$i$$j-zh.png \
	                  < /dev/null ; \
	            elif [ "x$(GRAPHICS)" == "xbmp" ] ; then \
	               $(BINDIR)/unihex2bmp -p$$i \
	                  -o$(GRAPHICS_DIR)/plane0$$p-zh/uni0$$p$$i$$j-zh.png \
	                  < /dev/null ; \
	            fi ; \
	         fi ; \
	      done ; \
	   done ; \
	done
	touch $@

#
# Create the Unicode Plane 2 and Plane 3 Japanese glyph images.
#
glyph-jp-stamp: graphawks-stamp glyph-dirs-stamp ./hexadecimate.awk
	@echo "Building $(GRAPHICS_DIR)/jp"
	( cd $(GRAPHICS_DIR)/jp ; \
	   cat ../../../graphawks-env \
	       ../../unifont_jp_sample-$(VERSION).hex \
	       ../../jp-upper.hex | \
	   awk -F: -f ../../../hexadecimate.awk | \
	   sh \
	 )
	# Change the filenames to the form uni....-jp.* in Planes 0, 2, & 3.
	for i in 0 2 3 ; do \
	   ( cd $(GRAPHICS_DIR)/jp/plane0$$i ; \
	      for j in uni*.$(GRAPHICS) ; do \
	         mv $$j `basename $$j .${GRAPHICS}`-jp.$(GRAPHICS) ; \
	      done \
	   ) ; \
	done
	mv $(GRAPHICS_DIR)/jp/plane00 $(GRAPHICS_DIR)/plane00-jp
	mv $(GRAPHICS_DIR)/jp/plane02 $(GRAPHICS_DIR)/plane02-jp
	mv $(GRAPHICS_DIR)/jp/plane03 $(GRAPHICS_DIR)/plane03-jp
	#
	# Fill in missing Japanese Plane 2 and Plane 3 16-by-16 glyph grid images.
	#
	for p in 2 3 ; do \
	   for i in 0 1 2 3 4 5 6 7 8 9 A B C D E F ; do \
	      for j in 0 1 2 3 4 5 6 7 8 9 A B C D E F ; do \
	         if [ ! -f $(GRAPHICS_DIR)/plane0$$p-jp/uni0$$p$$i$$j-jp.$(GRAPHICS) ];\
	            then \
	            if [ "x$(GRAPHICS)" == "xpng" ] ; then \
	               $(PERL) $(BINDIR)/unihex2png -p $$i$$j \
	                  -o $(GRAPHICS_DIR)/plane0$$p-jp/uni0$$p$$i$$j-jp.png \
	                  < /dev/null ; \
	            elif [ "x$(GRAPHICS)" == "xbmp" ] ; then \
	               $(BINDIR)/unihex2bmp -p$$i \
	                  -o$(GRAPHICS_DIR)/plane0$$p-jp/uni0$$p$$i$$j-jp.png \
	                  < /dev/null ; \
	            fi ; \
	         fi ; \
	      done ; \
	   done ; \
	done
	touch $@

#
# Create the Plane 0 and Plane 15 CSUR/UCSUR Private Use Area images.
#
glyph-csur-stamp: graphawks-stamp glyph-dirs-stamp ./hexadecimate.awk
	@echo "Building $(GRAPHICS_DIR)/csur"
	( cd $(GRAPHICS_DIR)/csur ; \
	   cat ../../../graphawks-env \
	       ../../unifont_csur_sample-$(VERSION).hex | \
	   awk -F: -f ../../../hexadecimate.awk | \
	   sh \
	 )
	# Remove non-CSUR directories.
	# Move all CSUR glyphs to the csur top folder
	-mv $(GRAPHICS_DIR)/csur/plane00/uni00E*.$(GRAPHICS) \
	    $(GRAPHICS_DIR)/csur
	-mv $(GRAPHICS_DIR)/csur/plane00/uni00F[0-8].$(GRAPHICS) \
	    $(GRAPHICS_DIR)/csur
	-mv $(GRAPHICS_DIR)/csur/plane0F/*.$(GRAPHICS) \
	    $(GRAPHICS_DIR)/csur
	# Plane 10 has no CSUR/UCSUR glyphs in it yet.
	#-mv $(GRAPHICS_DIR)/csur/plane10/*.$(GRAPHICS) \
	#    $(GRAPHICS_DIR)/csur
	#
	# Fill in missing CSUR 16-by-16 glyph grid images.
	# For any  missing files for code points in the range
	# U+E000..U+F8FF and U+F0000..U+F2FFF, create empty grid images.
	#
	for i in "E0" "E1" "E2" "E3" "E4" "E5" "E6" "E7" \
	         "E8" "E9" "EA" "EB" "EC" "ED" "EE" "EF" \
	         "F0" "F1" "F2" "F3" "F4" "F5" "F6" "F7" "F8" ; do\
	   if [ ! -f $(GRAPHICS_DIR)/csur/uni00$$i.$(GRAPHICS) ] ; then \
	      if [ "x$(GRAPHICS)" == "xpng" ] ; then \
	         $(PERL) $(BINDIR)/unihex2png -p $$i \
	            -o $(GRAPHICS_DIR)/csur/uni00$$i.png \
	            < /dev/null ; \
	      elif [ "x$(GRAPHICS)" == "xbmp" ] ; then \
	         $(BINDIR)/unihex2bmp -p$$i \
	            -o$(GRAPHICS_DIR)/csur/uni00$$i.bmp \
	            < /dev/null ; \
	      fi ; \
	   fi ; \
	done
	for i in "F0" "F1" "F2" ; do \
	   for j in 0 1 2 3 4 5 6 7 8 9 A B C D E F ; do \
	      if [ ! -f $(GRAPHICS_DIR)/csur/uni0$$i$$j.$(GRAPHICS) ] ; \
	         then \
	         if [ "x$(GRAPHICS)" == "xpng" ] ; then \
	            $(PERL) $(BINDIR)/unihex2png -p $$i$$j \
	               -o $(GRAPHICS_DIR)/csur/uni0$$i$$j.png \
	               < /dev/null ; \
	         elif [ "x$(GRAPHICS)" == "xbmp" ] ; then \
	            $(BINDIR)/unihex2bmp -p$$i \
	               -o$(GRAPHICS_DIR)/csur/uni00$$i.bmp \
	               < /dev/null ; \
	         fi ; \
	      fi ; \
	   done ; \
	done
	touch $@

#
# Remove intermediate/placeholder glyph image files and directories.
# This target does nothing for now.
#
glyph-wrapup-stamp: glyph-zh-stamp glyph-jp-stamp glyph-csur-stamp
	#for i in 0 1 2 3 4 5 6 7 8 9 A B C D E F ; do \
	#   -rm -rf $(GRAPHICS_DIR)/csur/plane0$i; \
	#done
	#-rm -rf $(GRAPHICS_DIR)/csur/plane10
	touch $@

#
# Having built the .hex files, create HTML pages showing their
# color-coded completion for each 256-glyph block using the
# unipagecount Unifont utility.
#
html: html-stamp

html-stamp: hex-stamp
	#
	# Create HTML table showing color-coded percent coverage in Plane 0.
	#
	if [ "x$(GRAPHICS)" == "xbmp" ] ; then \
	   $(BINDIR)/unipagecount -h \
	      < $(COMPILED_DIR)/unifont-$(VERSION).hex | \
	      sed -e 's/<a /<a target="blank" /g' | \
	      sed -e 's/png/bmp/g'   > $(COMPILED_DIR)/plane00.html ; \
	elif [ "x$(GRAPHICS)" == "xpng" ] ; then \
	   $(BINDIR)/unipagecount -h \
	      < $(COMPILED_DIR)/unifont-$(VERSION).hex | \
	      sed -e 's/<a /<a target="blank" /g' | \
	         sed -e 's/png/$(GRAPHICS)/g' > $(COMPILED_DIR)/plane00.html ;\
	fi
	#
	# Create HTML table showing color-coded percent coverage in
	# T-source Plane 0.
	#
	if [ "x$(GRAPHICS)" == "xbmp" ] ; then \
	   $(BINDIR)/unipagecount -h \
	      < $(COMPILED_DIR)/unifont_t_sample-$(VERSION).hex | \
	      sed -e 's/Glyphs/Glyphs \&mdash; Japanese Version/' | \
	      sed -e 's/<a /<a target="blank" /g' | \
	      sed -e 's/plane00/plane00-t/g' | \
	      sed -e 's/png/bmp/g' | \
	      sed -e 's/\.bmp/-t.bmp/g' \
	                             > $(COMPILED_DIR)/plane00-t.html ; \
	elif [ "x$(GRAPHICS)" == "xpng" ] ; then \
	   $(BINDIR)/unipagecount -h \
	      < $(COMPILED_DIR)/unifont_t_sample-$(VERSION).hex | \
	      sed -e 's/Glyphs/Glyphs \&mdash; Japanese Version/' | \
	      sed -e 's/<a /<a target="blank" /g' | \
	      sed -e 's/plane00/plane00-t/g' | \
	      sed -e 's/\.png/-t.png/g' \
	                          > $(COMPILED_DIR)/plane00-t.html ; \
	fi
	#
	# Create HTML table showing color-coded percent coverage in
	# Japanese Plane 0.
	#
	if [ "x$(GRAPHICS)" == "xbmp" ] ; then \
	   $(BINDIR)/unipagecount -h \
	      < $(COMPILED_DIR)/unifont_jp_sample-$(VERSION).hex | \
	      sed -e 's/Glyphs/Glyphs \&mdash; Japanese Version/' | \
	      sed -e 's/<a /<a target="blank" /g' | \
	      sed -e 's/plane00/plane00-jp/g' | \
	      sed -e 's/png/bmp/g' | \
	      sed -e 's/\.bmp/-jp.bmp/g' \
	                             > $(COMPILED_DIR)/plane00-jp.html ; \
	elif [ "x$(GRAPHICS)" == "xpng" ] ; then \
	   $(BINDIR)/unipagecount -h \
	      < $(COMPILED_DIR)/unifont_jp_sample-$(VERSION).hex | \
	      sed -e 's/Glyphs/Glyphs \&mdash; Japanese Version/' | \
	      sed -e 's/<a /<a target="blank" /g' | \
	      sed -e 's/plane00/plane00-jp/g' | \
	      sed -e 's/\.png/-jp.png/g' \
	                          > $(COMPILED_DIR)/plane00-jp.html ; \
	fi
	#
	# Create HTML table showing color-coded percent coverage in Plane 1.
	#
	if [ "x$(GRAPHICS)" == "xbmp" ] ; then \
	   $(BINDIR)/unipagecount -P1 -h \
	      < plane01/plane01-all.hex | \
	      sed -e 's/<a /<a target="blank" /g' | \
	      sed -e 's/png/bmp/g'   > $(COMPILED_DIR)/plane01.html ; \
	elif [ "x$(GRAPHICS)" == "xpng" ] ; then \
	   $(BINDIR)/unipagecount -P1 -h \
	      < plane01/plane01-all.hex | \
	      sed -e 's/<a /<a target="blank" /g' | \
	         sed -e 's/png/$(GRAPHICS)/g' \
	             > $(COMPILED_DIR)/plane01.html ; \
	fi
	#
	# Copy Plane 2 and Plane 3 HTML tables, specially color-coded
	# to show empty ranges as gray because there are so many empty
	# ranges in Planes 2 and 3.
	#
	if [ "x$(GRAPHICS)" == "xbmp" ] ; then \
	   sed -e 's/png/bmp/g' plane02-jp.html \
	       > $(COMPILED_DIR)/plane02-jp.html ; \
	   sed -e 's/png/bmp/g' plane02-zh.html \
	       > $(COMPILED_DIR)/plane02-zh.html ; \
	   sed -e 's/png/bmp/g' plane03-zh.html \
	       > $(COMPILED_DIR)/plane03-zh.html ; \
	elif [ "x$(GRAPHICS)" == "xpng" ] ; then \
	   install -p plane02-jp.html $(COMPILED_DIR) ; \
	   install -p plane02-zh.html $(COMPILED_DIR) ; \
	   install -p plane03-zh.html $(COMPILED_DIR) ; \
	fi
	#
	# Final wrap-up to complete the HTML pages after
	# all font files have been built.
	#
	sed -e 's/<a /<a target="blank" /g' index.html | \
	   sed -e 's/png/$(GRAPHICS)/g' > $(COMPILED_DIR)/index.html
	cat $(COMBINING_FILES) \
	    > $(COMPILED_DIR)/unifont-combining-$(VERSION).txt
	@echo "Done creating HTML coverage tables."
	touch $@

#
# Build one bitmap of the entire Unifont Plane 0 set of glyphs.
# Use all the .hex files in plane00 so we add the contents of
# omit.hex (FFFE & FFFF).  Those two glyphs aren't included in the
# unifont_sample font because they cause problems with MS Windows,
# but we can put them in the picture.  Also add the Plane 0
# ConScript Unicode Registry (CSUR) glyphs in the Private Use Area.
# The CSUR glyphs are optional, appearing in the unifont_csur font
# files rather than the main unifont font files.
#
# Build bitmaps for the Japanese (JIS X 0213) versions of
# Plane 0 and Plane 1, the Chinese version of Plane 2, and
# the superset Chinese/Japanese Plane 3.
#
bigpic: bigpic-stamp

bigpic-stamp: hex-stamp
	cat $(COMPILED_DIR)/unifont_sample-$(VERSION).hex \
	    $(COMPILED_DIR)/unifont_csur_sample-$(VERSION).hex | \
	    grep -v "^01F12F" | \
	    $(BINDIR)/unifontpic -d120 \
	       > $(COMPILED_DIR)/unifont-$(VERSION).bmp
	    magick $(COMPILED_DIR)/unifont-$(VERSION).bmp \
	           -monochrome \
	           $(COMPILED_DIR)/unifont-$(VERSION).png
	    -rm $(COMPILED_DIR)/unifont-$(VERSION).bmp
	cat $(COMPILED_DIR)/unifont_jp_sample-$(VERSION).hex \
	    $(COMPILED_DIR)/unifont_csur_sample-$(VERSION).hex | \
	    grep -v "^01F12F" | \
	    $(BINDIR)/unifontpic -d120 \
	       > $(COMPILED_DIR)/unifont_jp-$(VERSION).bmp
	    magick $(COMPILED_DIR)/unifont_jp-$(VERSION).bmp \
	           -monochrome \
	           $(COMPILED_DIR)/unifont_jp-$(VERSION).png
	    -rm $(COMPILED_DIR)/unifont_jp-$(VERSION).bmp
	cat $(COMPILED_DIR)/unifont_t_sample-$(VERSION).hex \
	    $(COMPILED_DIR)/unifont_csur_sample-$(VERSION).hex | \
	    grep -v "^01F12F" | \
	    $(BINDIR)/unifontpic -d120 \
	       > $(COMPILED_DIR)/unifont_t-$(VERSION).bmp
	    magick $(COMPILED_DIR)/unifont_t-$(VERSION).bmp \
	           -monochrome \
	           $(COMPILED_DIR)/unifont_t-$(VERSION).png
	    -rm $(COMPILED_DIR)/unifont_t-$(VERSION).bmp
	$(BINDIR)/unifontpic -d120 -P1 \
	   < $(COMPILED_DIR)/unifont_upper_sample-$(VERSION).hex \
	   > $(COMPILED_DIR)/unifont_plane1-$(VERSION).bmp
	   magick $(COMPILED_DIR)/unifont_plane1-$(VERSION).bmp \
	          -monochrome \
	          $(COMPILED_DIR)/unifont_plane1-$(VERSION).png
	   -rm $(COMPILED_DIR)/unifont_plane1-$(VERSION).bmp
	$(BINDIR)/unifontpic -d120 -P2 \
	   < $(JP_UPPER_SUPERSET_HEX) \
	   > $(COMPILED_DIR)/unifont_plane2_jp-$(VERSION).bmp
	   magick $(COMPILED_DIR)/unifont_plane2_jp-$(VERSION).bmp \
	          -monochrome \
	          $(COMPILED_DIR)/unifont_plane2_jp-$(VERSION).png
	   -rm $(COMPILED_DIR)/unifont_plane2_jp-$(VERSION).bmp
	$(BINDIR)/unifontpic -d120 -P2 \
	   < $(ZH_UPPER_SUPERSET_HEX) \
	   > $(COMPILED_DIR)/unifont_plane2_zh-$(VERSION).bmp
	   magick $(COMPILED_DIR)/unifont_plane2_zh-$(VERSION).bmp \
	          -monochrome \
	          $(COMPILED_DIR)/unifont_plane2_zh-$(VERSION).png
	   -rm $(COMPILED_DIR)/unifont_plane2_zh-$(VERSION).bmp
	$(BINDIR)/unifontpic -d120 -P3 \
	   < $(ZH_UPPER_SUPERSET_HEX) \
	   > $(COMPILED_DIR)/unifont_plane3_zh-$(VERSION).bmp
	   magick $(COMPILED_DIR)/unifont_plane3_zh-$(VERSION).bmp \
	          -monochrome \
	          $(COMPILED_DIR)/unifont_plane3_zh-$(VERSION).png
	   -rm $(COMPILED_DIR)/unifont_plane3_zh-$(VERSION).bmp
	touch $@

#
# Create thumbnail JPEG images of the whole Unicode plane images
# that the "bigpic" target creates.
#
# Note: Thumbnails are generated using the ImageMagick package.
#
thumbnails: thumbnails-stamp

thumbnails-stamp: bigpic-stamp
	magick $(COMPILED_DIR)/$(PACKAGE)-$(VERSION).png \
	   -thumbnail 124x125 -type GrayScale -depth 8 \
	   $(COMPILED_DIR)/$(PACKAGE)-thumbnail-$(VERSION).jpg
	magick $(COMPILED_DIR)/$(PACKAGE)_t-$(VERSION).png \
	   -thumbnail 124x125 -type GrayScale -depth 8 \
	   $(COMPILED_DIR)/$(PACKAGE)_t-thumbnail-$(VERSION).jpg
	magick $(COMPILED_DIR)/$(PACKAGE)_jp-$(VERSION).png \
	   -thumbnail 124x125 -type GrayScale -depth 8 \
	   $(COMPILED_DIR)/$(PACKAGE)_jp-thumbnail-$(VERSION).jpg
	magick $(COMPILED_DIR)/$(PACKAGE)_plane1-$(VERSION).png \
	   -thumbnail 124x125 -type GrayScale -depth 8 \
	   $(COMPILED_DIR)/$(PACKAGE)_plane1-thumbnail-$(VERSION).jpg
	magick $(COMPILED_DIR)/$(PACKAGE)_plane2_jp-$(VERSION).png \
	   -thumbnail 124x125 -type GrayScale -depth 8 \
	   $(COMPILED_DIR)/$(PACKAGE)_plane2_jp-thumbnail-$(VERSION).jpg
	magick $(COMPILED_DIR)/$(PACKAGE)_plane2_zh-$(VERSION).png \
	   -thumbnail 124x125 -type GrayScale -depth 8 \
	   $(COMPILED_DIR)/$(PACKAGE)_plane2_zh-thumbnail-$(VERSION).jpg
	magick $(COMPILED_DIR)/$(PACKAGE)_plane3_zh-$(VERSION).png \
	   -thumbnail 124x125 -type GrayScale -depth 8 \
	   $(COMPILED_DIR)/$(PACKAGE)_plane3_zh-thumbnail-$(VERSION).jpg
	touch $@

#
# Create the OpenType font files.
#
opentype: opentype-stamp

opentype-stamp: otf-stamp otf-csur-stamp otf-upper-stamp otf-sample-stamp
	touch $@

otf: otf-stamp

#
# Build the Plane 0 OTF font files.
#
otf-stamp: $(COMPILED_DIR)/unifont-$(VERSION).otf \
	   $(COMPILED_DIR)/unifont_t-$(VERSION).otf \
	   $(COMPILED_DIR)/unifont_jp-$(VERSION).otf
	touch $@

#
# Build the default Unifont OTF file:
# - Copy the default unifont.hex version, to make an OpenType font
# - Add the Copyleft symbol to the Plane 0 Unifont OpenueType font
# - Remove Variation Selectors, U+FE00..U+FE0F.
#
$(COMPILED_DIR)/unifont-$(VERSION).otf: hex-stamp
	cat $(COMPILED_DIR)/unifont-$(VERSION).hex $(COPYLEFT) \
	    $(ZH_UPPER_SUBSET_HEX) | \
	    grep -v "^FE0.:" > $(OTFSRC)/unifont.hex
	install -p \
		plane00/plane00-combining.txt \
		$(OTFSRC)/combining.txt
	cd $(OTFSRC) && \
	   $(MAKE) FONTFILE="unifont" COMBINING="combining" \
		FONTNAME="Unifont" PSNAME="Unifont"
	mv $(OTFSRC)/unifont.otf \
		$(COMPILED_DIR)/unifont-$(VERSION).otf 
	rm $(OTFSRC)/unifont.hex \
	   $(OTFSRC)/combining.txt

#
# Build a version with T-source glyphs, adding Copyleft symbol.
#
$(COMPILED_DIR)/unifont_t-$(VERSION).otf: hex-stamp
	sort $(COPYLEFT) $(COMPILED_DIR)/unifont_t-$(VERSION).hex \
		> $(OTFSRC)/unifont_t.hex
	install -p \
		plane00/plane00-combining.txt \
		$(OTFSRC)/combining-t.txt
	cd $(OTFSRC) && \
	   $(MAKE) FONTFILE="unifont_t" COMBINING="combining-t" \
		FONTNAME="Unifont-T" PSNAME="Unifont-T"
	mv $(OTFSRC)/unifont_t.otf \
		$(COMPILED_DIR)/unifont_t-$(VERSION).otf 
	rm $(OTFSRC)/unifont_t.hex \
	   $(OTFSRC)/combining-t.txt

#
# Build a version with Japanese glyphs as drawn in JIS X 0213:2004,
# adding Copyleft symbol.
#
$(COMPILED_DIR)/unifont_jp-$(VERSION).otf: hex-stamp
	sort $(COPYLEFT) $(COMPILED_DIR)/unifont_jp-$(VERSION).hex \
		> $(OTFSRC)/unifont_jp.hex
	install -p \
		plane00/plane00-combining.txt \
		$(OTFSRC)/combining-jp.txt
	cd $(OTFSRC) && \
	   $(MAKE) FONTFILE="unifont_jp" COMBINING="combining-jp" \
		FONTNAME="Unifont-JP" PSNAME="Unifont-JP"
	mv $(OTFSRC)/unifont_jp.otf \
		$(COMPILED_DIR)/unifont_jp-$(VERSION).otf 
	rm $(OTFSRC)/unifont_jp.hex \
	   $(OTFSRC)/combining-jp.txt

#
# Build an OpenType font with the CSUR Private Use Area glyphs.
#
otf-csur: otf-csur-stamp

otf-csur-stamp: $(COMPILED_DIR)/unifont_csur-$(VERSION).otf
	touch $@

$(COMPILED_DIR)/unifont_csur-$(VERSION).otf: hex-stamp
	install -p \
		$(COMPILED_DIR)/unifont_csur-$(VERSION).hex \
		$(OTFSRC)/unifont_csur.hex
	sort -u plane00csur/plane00csur-combining.txt \
		>  $(OTFSRC)/combining_csur.txt
	sort -u plane0[1-F]csur/plane0[1-F]csur-combining.txt \
		>> $(OTFSRC)/combining_csur.txt
	cd $(OTFSRC) && \
	   $(MAKE) FONTFILE="unifont_csur" COMBINING="combining_csur" \
		FONTNAME="Unifont CSUR" PSNAME="UnifontCSUR"
	mv $(OTFSRC)/unifont_csur.otf \
		$(COMPILED_DIR)/unifont_csur-$(VERSION).otf 
	rm $(OTFSRC)/unifont_csur.hex \
	   $(OTFSRC)/combining_csur.txt

#
# Build OpenType font files that contain Unicode Planes 1 through 3,
# with the Plane 0 space glyph (U+0020) added.
#
otf-upper: otf-upper-stamp

otf-upper-stamp: $(COMPILED_DIR)/unifont_upper-$(VERSION).otf
	touch $@

$(COMPILED_DIR)/unifont_upper-$(VERSION).otf: hex-stamp
	# First, copy the ordinary version, to make an OpenType font.
	# Remove Variation Selectors, U+E0100..U+E01EF.
	grep -v "^0E01[0-E].:" $(COMPILED_DIR)/unifont_upper-$(VERSION).hex \
		             > $(OTFSRC)/unifont_upper.hex
	sort -u $(UPPER_COMBINING) > $(OTFSRC)/combining_upper.txt
	cd $(OTFSRC) && \
	   $(MAKE) FONTFILE="unifont_upper" COMBINING="combining_upper" \
		FONTNAME="Unifont Upper" PSNAME="UnifontUpper"
	mv $(OTFSRC)/unifont_upper.otf \
		$(COMPILED_DIR)/unifont_upper-$(VERSION).otf 
	rm $(OTFSRC)/unifont_upper.hex \
	   $(OTFSRC)/combining_upper.txt

#
# Make sample book OpenType fonts, with combining circles and
# no combining widths.
#
otf-sample: otf-sample-stamp

otf-sample-stamp: $(COMPILED_DIR)/unifont_sample-$(VERSION).otf      \
	          $(COMPILED_DIR)/unifont_t_sample-$(VERSION).otf     \
	          $(COMPILED_DIR)/unifont_jp_sample-$(VERSION).otf    \
	          $(COMPILED_DIR)/unifont_upper_sample-$(VERSION).otf
	touch $@

#
# Plane 0 sample font.
#
$(COMPILED_DIR)/unifont_sample-$(VERSION).otf: hex-stamp
	install -p \
		$(COMPILED_DIR)/unifont_sample-$(VERSION).hex \
		$(OTFSRC)/unifont_sample.hex
	> $(OTFSRC)/combining_sample.txt
	cd $(OTFSRC) && \
		$(MAKE) FONTFILE="unifont_sample" \
			COMBINING="combining_sample" \
			FONTNAME="Unifont Sample" PSNAME="UnifontSample"
	mv $(OTFSRC)/unifont_sample.otf \
		$(COMPILED_DIR)/unifont_sample-$(VERSION).otf 
	rm $(OTFSRC)/unifont_sample.hex \
	   $(OTFSRC)/combining_sample.txt

#
# Plane 0 T-source sample font.
#
$(COMPILED_DIR)/unifont_t_sample-$(VERSION).otf: hex-stamp
	install -p \
		$(COMPILED_DIR)/unifont_t_sample-$(VERSION).hex \
		$(OTFSRC)/unifont_t_sample.hex
	> $(OTFSRC)/combining_t_sample.txt
	cd $(OTFSRC) && \
		$(MAKE) FONTFILE="unifont_t_sample" \
			COMBINING="combining_t_sample" \
			FONTNAME="Unifont T Sample" PSNAME="UnifontTSample"
	mv $(OTFSRC)/unifont_t_sample.otf \
		$(COMPILED_DIR)/unifont_t_sample-$(VERSION).otf 
	rm $(OTFSRC)/unifont_t_sample.hex \
	   $(OTFSRC)/combining_t_sample.txt

#
# Plane 0 Japanese sample font.
#
$(COMPILED_DIR)/unifont_jp_sample-$(VERSION).otf: hex-stamp
	install -p \
		$(COMPILED_DIR)/unifont_jp_sample-$(VERSION).hex \
		$(OTFSRC)/unifont_jp_sample.hex
	> $(OTFSRC)/combining_jp_sample.txt
	cd $(OTFSRC) && \
		$(MAKE) FONTFILE="unifont_jp_sample" \
			COMBINING="combining_jp_sample" \
			FONTNAME="Unifont JP Sample" PSNAME="UnifontJPSample"
	mv $(OTFSRC)/unifont_jp_sample.otf \
		$(COMPILED_DIR)/unifont_jp_sample-$(VERSION).otf 
	rm $(OTFSRC)/unifont_jp_sample.hex \
	   $(OTFSRC)/combining_jp_sample.txt

#
# Plane 1 sample font.
#
$(COMPILED_DIR)/unifont_upper_sample-$(VERSION).otf: hex-stamp
	install -p \
		$(COMPILED_DIR)/unifont_upper_sample-$(VERSION).hex \
		$(OTFSRC)/unifont_upper_sample.hex
	> $(OTFSRC)/combining_upper_sample.txt
	cd $(OTFSRC) && \
		$(MAKE) FONTFILE="unifont_upper_sample" \
			COMBINING="combining_upper_sample" \
			FONTNAME="Unifont Sample" PSNAME="UnifontSample"
	mv $(OTFSRC)/unifont_upper_sample.otf \
		$(COMPILED_DIR)/unifont_upper_sample-$(VERSION).otf 
	rm $(OTFSRC)/unifont_upper_sample.hex \
	   $(OTFSRC)/combining_upper_sample.txt

#
# Build the TrueType font files.
#
# Note that $(TTFSRC) must exist, because it contains some source files
# Perform a "make && make clean" because ALL of the files would consume
# over 200 Megabytes if left around.  The .sfd file is about 100 Megabytes,
# and it is created from merging "[0-F].sfd", which take up another 100 MB.
#
truetype: truetype-stamp

truetype-stamp: ttf-stamp ttf-csur-stamp ttf-upper-stamp
	touch $@

#
# Build the Plane 0 TrueType font files.
#
ttf: ttf-stamp

ttf-stamp: hex-stamp
	# First, copy the ordinary version, to make a TrueType font.
	# Remove the Variation Selectors, U+FE00..U+FE0F
	grep -v "^FE0.:" $(COMPILED_DIR)/unifont-$(VERSION).hex \
	 	       >  $(TTFSRC)/unifont.hex
	# Add the Copyleft symbol to the Plane 0 Unifont TrueType font
	cat $(COPYLEFT) >> $(TTFSRC)/unifont.hex
	install -p \
		plane00/plane00-combining.txt \
		$(TTFSRC)/combining.txt
	cd $(TTFSRC) && \
	   $(MAKE) FONTFILE="unifont" COMBINING="combining" \
		FONTNAME="Unifont" PSNAME="Unifont"
	mv $(TTFSRC)/unifont.sfd $(COMPILED_DIR)/unifont-$(VERSION).sfd 
	mv $(TTFSRC)/unifont.ttf \
		$(COMPILED_DIR)/unifont-$(VERSION).ttf 
	rm $(TTFSRC)/unifont.hex \
	   $(TTFSRC)/combining.txt
	# Second, copy unifont_sample.bdf, to make an SBIT font.
	install -p \
		$(COMPILED_DIR)/unifont_sample-$(VERSION).bdf \
		$(TTFSRC)/unifont_sample.bdf
	cd $(TTFSRC) && \
	   $(MAKE) sbit FONTFILE="unifont_sample" COMBINING="" \
		FONTNAME="Unifont Sample" PSNAME="UnifontSample"
	mv $(TTFSRC)/unifont_sample.ttf \
		$(COMPILED_DIR)/unifont_sample-$(VERSION).ttf 
	rm $(TTFSRC)/unifont_sample.bdf
	# Third, make a version with T-source glyphs.
	sort $(COPYLEFT) $(COMPILED_DIR)/unifont_t-$(VERSION).hex | \
		grep -v "^FE0.:" > $(TTFSRC)/unifont_t.hex
	install -p \
		plane00/plane00-combining.txt \
		$(TTFSRC)/combining-t.txt
	cd $(TTFSRC) && \
	   $(MAKE) FONTFILE="unifont_t" COMBINING="combining-t" \
		FONTNAME="Unifont-T" PSNAME="Unifont-T"
	mv $(TTFSRC)/unifont_t.sfd $(COMPILED_DIR)/unifont_t-$(VERSION).sfd 
	mv $(TTFSRC)/unifont_t.ttf \
		$(COMPILED_DIR)/unifont_t-$(VERSION).ttf 
	rm $(TTFSRC)/unifont_t.hex \
	   $(TTFSRC)/combining-t.txt
	# Fourth, make a version with Japanese glyphs as drawn in
	# JIS X 0213:2004, adding Copyleft symbol.
	sort $(COPYLEFT) $(COMPILED_DIR)/unifont_jp-$(VERSION).hex | \
		grep -v "^FE0.:" > $(TTFSRC)/unifont_jp.hex
	install -p \
		plane00/plane00-combining.txt \
		$(TTFSRC)/combining-jp.txt
	cd $(TTFSRC) && \
	   $(MAKE) FONTFILE="unifont_jp" COMBINING="combining-jp" \
		FONTNAME="Unifont-JP" PSNAME="Unifont-JP"
	mv $(TTFSRC)/unifont_jp.sfd $(COMPILED_DIR)/unifont_jp-$(VERSION).sfd 
	mv $(TTFSRC)/unifont_jp.ttf \
		$(COMPILED_DIR)/unifont_jp-$(VERSION).ttf 
	rm $(TTFSRC)/unifont_jp.hex \
	   $(TTFSRC)/combining-jp.txt
	touch $@

#
# Build the TrueType CSUR Private Use Area font.
#
ttf-csur: ttf-csur-stamp

ttf-csur-stamp: hex-stamp
	install -p \
		$(COMPILED_DIR)/unifont_csur-$(VERSION).hex \
		$(TTFSRC)/unifont_csur.hex
	sort -u plane00csur/plane00csur-combining.txt \
		>  $(TTFSRC)/combining_csur.txt
	sort -u plane0[1-F]csur/plane0[1-F]csur-combining.txt \
		>> $(TTFSRC)/combining_csur.txt
	cd $(TTFSRC) && \
	   $(MAKE) FONTFILE="unifont_csur" COMBINING="combining_csur" \
		FONTNAME="Unifont CSUR" PSNAME="UnifontCSUR"
	mv $(TTFSRC)/unifont_csur.sfd \
		$(COMPILED_DIR)/unifont_csur-$(VERSION).sfd 
	mv $(TTFSRC)/unifont_csur.ttf \
		$(COMPILED_DIR)/unifont_csur-$(VERSION).ttf 
	gzip $(GZFLAGS) $(COMPILED_DIR)/unifont_csur-$(VERSION).sfd 
	rm $(TTFSRC)/unifont_csur.hex \
	   $(TTFSRC)/combining_csur.txt
	touch $@

#
# Build the TrueType Plane 1 to Plane 3 font files.
#
ttf-upper: ttf-upper-stamp

ttf-upper-stamp: hex-stamp
	# First, copy the ordinary version, to make a TrueType font.
	# Remove Variation Selectors, U+E0100..U+E01EF.
	grep -v "^0E01[0-E].:" $(COMPILED_DIR)/unifont_upper-$(VERSION).hex \
		         > $(TTFSRC)/unifont_upper.hex
	sort -u $(UPPER_COMBINING) > $(TTFSRC)/combining_upper.txt
	cd $(TTFSRC) && \
	   $(MAKE) FONTFILE="unifont_upper" COMBINING="combining_upper" \
		FONTNAME="Unifont Upper" PSNAME="UnifontUpper"
	mv $(TTFSRC)/unifont_upper.sfd \
		$(COMPILED_DIR)/unifont_upper-$(VERSION).sfd 
	mv $(TTFSRC)/unifont_upper.ttf \
		$(COMPILED_DIR)/unifont_upper-$(VERSION).ttf 
	gzip $(GZFLAGS) $(COMPILED_DIR)/unifont_upper-$(VERSION).sfd 
	rm $(TTFSRC)/unifont_upper.hex \
	   $(TTFSRC)/combining_upper.txt
	# Second, copy unifont_upper_sample.bdf, to make an SBIT font.
	install -p \
		$(COMPILED_DIR)/unifont_upper_sample-$(VERSION).bdf \
		$(TTFSRC)/unifont_upper_sample.bdf
	cd $(TTFSRC) && \
	   $(MAKE) sbit FONTFILE="unifont_upper_sample" COMBINING="" \
		FONTNAME="Unifont Upper Sample" PSNAME="UnifontUpperSample"
	mv $(TTFSRC)/unifont_upper_sample.ttf \
		$(COMPILED_DIR)/unifont_upper_sample-$(VERSION).ttf 
	rm $(TTFSRC)/unifont_upper_sample.bdf
	touch $@

#
# Definitions for the "precompiled" target below.  These are
# the pre-built font files that will be part of the Unifont
# package, for those who just want the font files and not all
# the source files to build them.
#
PRECOMPILED_FILES = \
	$(COMPILED_DIR)/unifont_all-$(VERSION).hex \
	$(COMPILED_DIR)/unifont-$(VERSION).hex \
	$(COMPILED_DIR)/unifont_sample-$(VERSION).hex \
	$(COMPILED_DIR)/unifont_csur-$(VERSION).hex \
	$(COMPILED_DIR)/unifont_jp-$(VERSION).hex \
	$(COMPILED_DIR)/unifont_jp_sample-$(VERSION).hex \
	$(COMPILED_DIR)/unifont_t-$(VERSION).hex \
	$(COMPILED_DIR)/unifont_t_sample-$(VERSION).hex \
	$(COMPILED_DIR)/unifont_upper-$(VERSION).hex \
	$(COMPILED_DIR)/unifont_upper_sample-$(VERSION).hex \
	$(COMPILED_DIR)/unifont-$(VERSION).bdf.gz \
	$(COMPILED_DIR)/unifont_sample-$(VERSION).bdf.gz \
	$(COMPILED_DIR)/unifont_jp-$(VERSION).bdf.gz \
	$(COMPILED_DIR)/unifont_t-$(VERSION).bdf.gz \
	$(COMPILED_DIR)/unifont-$(VERSION).pcf.gz \
	$(COMPILED_DIR)/unifont_sample-$(VERSION).pcf.gz \
	$(COMPILED_DIR)/unifont_csur-$(VERSION).pcf.gz \
	$(COMPILED_DIR)/unifont_jp-$(VERSION).pcf.gz \
	$(COMPILED_DIR)/unifont_t-$(VERSION).pcf.gz \
	$(COMPILED_DIR)/Unifont-APL8x16-$(VERSION).psf.gz \
	$(COMPILED_DIR)/unifont-$(VERSION).otf \
	$(COMPILED_DIR)/unifont_sample-$(VERSION).otf \
	$(COMPILED_DIR)/unifont_csur-$(VERSION).otf \
	$(COMPILED_DIR)/unifont_jp-$(VERSION).otf \
	$(COMPILED_DIR)/unifont_t-$(VERSION).otf \
	$(COMPILED_DIR)/unifont_jp_sample-$(VERSION).otf \
	$(COMPILED_DIR)/unifont_t_sample-$(VERSION).otf \
	$(COMPILED_DIR)/unifont_upper-$(VERSION).otf \
	$(COMPILED_DIR)/unifont_upper_sample-$(VERSION).otf \
	$(COMPILED_DIR)/coverage-$(VERSION).txt \
	$(COMPILED_DIR)/unifont-combining-$(VERSION).txt \
	$(COMPILED_DIR)/unifont-$(VERSION).png \
	$(COMPILED_DIR)/unifont_jp-$(VERSION).png \
	$(COMPILED_DIR)/unifont_t-$(VERSION).png \
	$(COMPILED_DIR)/unifont_plane1-$(VERSION).png

#
# Copy the newly created files from $(COMPILED_DIR) to the precompiled/
# directory.  This has to be called manually, because the precompiled/
# directory usually remains untouched.
#
precompiled: hex-stamp bdf-stamp pcf-stamp psf-stamp opentype-stamp \
	     bigpic-stamp coverage-stamp \
	     $(COMPILED_DIR)/unifont-combining-$(VERSION).txt
	rm -rf precompiled
	install -m0755 -d precompiled
	install -m0644 -p $(PRECOMPILED_FILES) precompiled

#
# This is set up for GNU/Linux.  Solaris places fonts in
# "/usr/share/fonts/TrueType".  Other Unices use other directories.
#
# The original Unifont font format was BDF, but PCF files compress
# BDF glyph information, so the BDF version is not installed.
#
# Pango has dropped support for TrueType fonts, so this Makefile builds
# and installs # the OpenType font files by default rather than the
# TrueType versions.
#
# After installing the new fonts, register them with X Window System using:
#
#      xset fp rehash
#
install:
	if [ x$(CURDIR) = x ] ; \
	then \
	   @echo "Fatal Error: CURDIR not defined -- define in Makefile." ; \
	   exit 1 ; \
	fi
	$(INSTALL) -m0755 -d $(CONSOLEDEST)
	$(INSTALL) -m0755 -d $(PCFDEST)
	$(INSTALL) -m0755 -d $(OTFDEST)
	if [ ! -d $(COMPILED_DIR) ] ; then \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/Unifont-APL8x16-$(VERSION).psf.gz   $(CONSOLEDEST)/Unifont-APL8x16.psf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont-$(VERSION).pcf.gz           $(PCFDEST)/unifont.pcf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_t-$(VERSION).pcf.gz         $(PCFDEST)/unifont_t.pcf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_jp-$(VERSION).pcf.gz        $(PCFDEST)/unifont_jp.pcf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_sample-$(VERSION).pcf.gz    $(PCFDEST)/unifont_sample.pcf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_csur-$(VERSION).pcf.gz      $(PCFDEST)/unifont_csur.pcf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont-$(VERSION).otf              $(OTFDEST)/unifont.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_t-$(VERSION).otf            $(OTFDEST)/unifont_t.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_jp-$(VERSION).otf           $(OTFDEST)/unifont_jp.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_csur-$(VERSION).otf         $(OTFDEST)/unifont_csur.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_upper-$(VERSION).otf        $(OTFDEST)/unifont_upper.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_sample-$(VERSION).otf       $(OTFDEST)/unifont_sample.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_t_sample-$(VERSION).otf     $(OTFDEST)/unifont_t_sample.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_jp_sample-$(VERSION).otf    $(OTFDEST)/unifont_jp_sample.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/precompiled/unifont_upper_sample-$(VERSION).otf $(OTFDEST)/unifont_upper_sample.otf ; \
	else \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/Unifont-APL8x16-$(VERSION).psf.gz   $(CONSOLEDEST)/Unifont-APL8x16.psf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont-$(VERSION).pcf.gz           $(PCFDEST)/unifont.pcf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_t-$(VERSION).pcf.gz         $(PCFDEST)/unifont_t.pcf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_jp-$(VERSION).pcf.gz        $(PCFDEST)/unifont_jp.pcf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_sample-$(VERSION).pcf.gz    $(PCFDEST)/unifont_sample.pcf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_csur-$(VERSION).pcf.gz      $(PCFDEST)/unifont_csur.pcf.gz ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont-$(VERSION).otf              $(OTFDEST)/unifont.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_t-$(VERSION).otf            $(OTFDEST)/unifont_t.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_jp-$(VERSION).otf           $(OTFDEST)/unifont_jp.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_csur-$(VERSION).otf         $(OTFDEST)/unifont_csur.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_upper-$(VERSION).otf        $(OTFDEST)/unifont_upper.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_sample-$(VERSION).otf       $(OTFDEST)/unifont_sample.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_t_sample-$(VERSION).otf     $(OTFDEST)/unifont_t_sample.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_jp_sample-$(VERSION).otf    $(OTFDEST)/unifont_jp_sample.otf ; \
	   $(INSTALL) -m0644 -p $(CURDIR)/$(COMPILED_DIR)/unifont_upper_sample-$(VERSION).otf $(OTFDEST)/unifont_upper_sample.otf ; \
	fi

#
# Remove some intermediate files.
#
clean:
	-rm -f *~
	-rm -rf $(GRAPHICS_DIR)/t
	-rm -rf $(GRAPHICS_DIR)/jp
	-rm -rf $(GRAPHICS_DIR)/zh
	-rm -rf $(GRAPHICS_DIR)/csur/plane*
	-rm -f graphawks-env
	# $(MAKE) -C plane00/hangul clean

#
# Note that distclean leaves the "precompiled" directory alone.
# This is intentional so the built font files are available without
# needing to build the entire font sets from source glyphs.
# The .DS files are created under macOS X.
#
distclean:
	-rm -rf $(COMPILED_DIR)
	# plane00/hangul is no longer building plane00/hangul-syllabes.hes
	# $(MAKE) -C plane00/hangul distclean
	$(MAKE) -C plane00csur distclean
	$(MAKE) -C plane01 distclean
	$(MAKE) -C ttfsrc distclean
	$(MAKE) -C otfsrc distclean
	-rm -f *-stamp
	-rm -f graphawks-env
	-rm -f *~
	-rm -rf .DS* ._.DS*

#
# Add $(COMPILED_DIR) to .PHONY so timestamp updates for it
# do not trigger an other targets unnecessarily.
#
.PHONY: all \
	$(COMPILED_DIR) \
	hex bdf pcf psf \
	coverage pagecount png bmp bigpic thumbnails \
	opentype otf otf-csur otf-upper otf-sample \
	truetype ttf ttf-csur ttf-upper \
	install clean distclean

