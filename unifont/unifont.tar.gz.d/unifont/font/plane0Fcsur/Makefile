
#
# Create the Plane 15 unassigned.hex file, with filler glyphs.
#
plane0Fcsur-unassigned.hex: plane0Fcsur-unassigned-ranges.txt
	set -e && \
	   grep "^[0-9A-Fa-f]" plane0Fcsur-unassigned-ranges.txt | \
	   awk 'NF == 1 {print "../../bin/unihexgen " $$1 " " $$1;} \
		NF == 2 {print "../../bin/unihexgen " $$1 " " $$2;}' | \
	   /bin/sh -s > plane0Fcsur-unassigned.hex

clean:

distclean: clean

.PHONY: clean distclean
