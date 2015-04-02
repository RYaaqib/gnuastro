/*********************************************************************
overlap -- Find the overlap of a region and an image.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <akhlaghi@gnu.org>
Contributing author(s):
Copyright (C) 2015, Free Software Foundation, Inc.

Gnuastro is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Gnuastro is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with gnuastro. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
#ifndef BOX_H
#define BOX_H

/*                        IMPORTANT NOTE:
	 All the axises are based on the FITS standard, NOT C.
*/

void
ellipseinbox(double a, double b, double theta_rad, long *width);

void
borderfromcenter(double xc, double yc, long *width,
		 long *fpixel, long *lpixel);

int
overlap(long *naxes, long *fpixel_i, long *lpixel_i,
	long *fpixel_o, long *lpixel_o);

#endif