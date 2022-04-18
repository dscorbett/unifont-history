
SHELL = /bin/sh

BINDIR = ../../bin

HEX2OTF = $(BINDIR)/hex2otf

#
# Default values, if not set on the command line when make is invoked:
#
# FONTFILE:  Prefix of the file name for input and output files.
# FONTNAME:  Name of the font inside a TTF file.
# PSNAME:    PostScript name of the font inside a TTF file; can't have spaces.
# COMBINING: Prefix of the file containing a list of combining characters.
#
FONTFILE="unifont"
FONTNAME="Unifont"
PSNAME="Unifont"
COMBINING="combining"

#
# Designate language for TrueType text as American English.
#
#
TTF_LANG = 0x409
#
# The PostScript name of a font can't contain spaces--remove them.
# Could also use bash string replacement if you know you're using bash.
#

COPYRIGHT = "Copyright © 1998-2022 Roman Czyborra, Paul Hardy, \
Qianqian Fang, Andrew Miller, Johnnie Weaver, David Corbett, \
Nils Moskopp, Rebecca Bettencourt, et al."

VERSION = 14.0.03

all: otf

otf: $(FONTFILE).hex $(HEX2OTF)
	@echo "Creating OpenType font."
	$(HEX2OTF) hex=$(FONTFILE).hex pos=$(COMBINING).txt \
	   0="$(COPYRIGHT)" 1="GNU" 2="Regular" 4="$(FONTNAME) Medium" \
	   5="Version $(VERSION)" 6="$(PSNAME)Medium" \
	   11="https://unifoundry.com/unifont/" \
	   13=Dual license: SIL Open Font License version 1.1, and GNU GPL version 2 or later with the GNU Font Embedding Exception." \
	   14="http://gnu.org/licenses/gpl.html, https://scripts.sil.org/OFL" \
	   format=cff,bitmap,gpos out=$(FONTFILE).otf
	\rm -f $(FONTFILE).hex $(COMBINING).txt

#
# Delete files copied into this directory to build TTF fonts.
#
clean:
	\rm -f *.bdf *.hex *.txt

#
# Delete files created within this directory while building TTF fonts.
#
distclean: clean
	\rm -f *.otf

.PHONY: all otf clean distclean
