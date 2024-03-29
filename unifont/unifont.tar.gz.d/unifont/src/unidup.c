/**
   @file unidup.c

   @brief unidup - Check for duplicate code points in sorted unifont.hex file

   @author Paul Hardy, unifoundry <at> unifoundry.com, December 2007

   @copyright Copyright (C) 2007, 2008, 2013 Paul Hardy

   This program reads a sorted list of glyphs in Unifont .hex format
   and prints duplicate code points on stderr if any were detected.

   Synopsis: unidup < unifont_file.hex

             [Hopefully there won't be any output!]
*/
/*
   LICENSE:

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 2 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#define MAXBUF 256   ///< Maximum input line length - 1


/**
   @brief The main function.

   @param[in] argc The count of command line arguments.
   @param[in] argv Pointer to array of command line arguments.
   @return This program exits with status 0.
*/
int
main (int argc, char **argv)
{

   int ix, iy;
   char inbuf[MAXBUF];
   char *infile;   /* the input file name */
   FILE *infilefp; /* file pointer to input file */

   if (argc > 1) {
      infile = argv[1];
      if ((infilefp = fopen (infile, "r")) == NULL) {
         fprintf (stderr, "\nERROR: Can't open file %s\n\n", infile);
         exit (EXIT_FAILURE);
      }
   }
   else {
      infilefp = stdin;
   }

   ix = -1;

   while (fgets (inbuf, MAXBUF-1, infilefp) != NULL) {
      sscanf (inbuf, "%X", &iy);
      if (ix == iy) fprintf (stderr, "Duplicate code point: %04X\n", ix);
      else ix = iy;
   }
   exit (0);
}
