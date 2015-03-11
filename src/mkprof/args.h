/*********************************************************************
MakeProfiles - Create mock astronomical profiles.
MakeProfiles is part of GNU Astronomy Utilities (gnuastro) package.

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
#ifndef ARGS_H
#define ARGS_H

#include "commonargs.h"
#include "fixedstringmacros.h"





/* Definition parameters for the argp: */
const char *argp_program_version=SPACK_STRING"\n"COPYRIGHT
  "\n\nWritten by Mohammad Akhlaghi";
const char *argp_program_bug_address=PACKAGE_BUGREPORT;
static char args_doc[] = "[PSFimage] Catalog";





const char doc[] =
  /* Before the list of options: */
  TOPHELPINFO
  SPACK_NAME" will create a FITS image containing any number of mock "
  "astronomical profiles based on an input catalog. All the profiles "
  "will be built from the center outwards. First by 10000 random "
  "points, then by integration and finally central pixel "
  "position. The tolerance level specifies when to switch to a less "
  "accurate method.\n"
  MOREHELPINFO
  /* After the list of options: */
  "\v"
  PACKAGE_NAME" home page: "PACKAGE_URL;





/*
   Available letters (-V which is used by GNU is also removed):

   a d e f g j k l u v w
   A B C E F G H I J L M O Q R T U W Z

   Maximum integer used so far: 514.
*/
static struct argp_option options[] =
  {
    {
      0, 0, 0, 0,
      "Operating modes:",
      -1
    },





    {
      0, 0, 0, 0,
      "Input:",
      1
    },





    {
      0, 0, 0, 0,
      "Output:",
      2
    },
    {
      "naxis1",
      'x',
      "INT",
      0,
      "Number of pixels along first FITS axis.",
      2
    },
    {
      "naxis2",
      'y',
      "INT",
      0,
      "Number of pixels along second FITS axis.",
      2
    },
    {
      "oversample",
      's',
      "INT",
      0,
      "Scale of oversampling.",
      2
    },
    {
      "psfinimg",
      509,
      0,
      0,
      "PSF profiles made with all in output image.",
      2
    },
    {
      "individual",
      'i',
      0,
      0,
      "Build all profiles separately.",
      2
    },
    {
      "nomerged",
      'm',
      0,
      0,
      "Do not create a merged image of all profiles.",
      2
    },






    {
      0, 0, 0, 0,
      "Profiles:",
      3
    },
    {
      "numrandom",
      'r',
      "INT",
      0,
      "No. of random points in Monte Carlo integration.",
      3
    },
    {
      "tolerance",
      't',
      "FLT",
      0,
      "Tolerance to switch to less accurate method.",
      3
    },
    {
      "tunitinp",
      'p',
      0,
      0,
      "Truncation is in units of pixels, not radius.",
      3
    },
    {
      "xshift",
      'X',
      "FLT",
      0,
      "Shift profile centers and enlarge image, X axis.",
      3
    },
    {
      "yshift",
      'Y',
      "FLT",
      0,
      "Shift profile centers and enlarge image, Y axis.",
      3
    },
    {
      "prepforconv",
      'c',
      0,
      0,
      "Shift and expand based on first catalog PSF.",
      3
    },
    {
      "zeropoint",
      'z',
      "FLT",
      0,
      "Magnitude zero point.",
      3
    },






    {
      0, 0, 0, 0,
      "Catalog (column number, starting from zero):",
      4
    },
    {
      "xcol",
      500,
      "INT",
      0,
      "Center along first FITS axis (horizontal).",
      4
    },
    {
      "ycol",
      501,
      "INT",
      0,
      "Center along second FITS axis (vertical).",
      4
    },
    {
      "fcol",
      502,
      "INT",
      0,
      "Sersic (0), Moffat (1), Gaussian(2), Point (3).",
      4
    },
    {
      "rcol",
      503,
      "INT",
      0,
      "Effective radius or FWHM in pixels.",
      4
    },
    {
      "ncol",
      504,
      "INT",
      0,
      "Sersic index or Moffat beta.",
      4
    },
    {
      "pcol",
      505,
      "INT",
      0,
      "Position angle.",
      4
    },
    {
      "qcol",
      506,
      "INT",
      0,
      "Axis ratio.",
      4
    },
    {
      "mcol",
      507,
      "INT",
      0,
      "Magnitude.",
      4
    },
    {
      "tcol",
      508,
      "INT",
      0,
      "Truncation in units of --rcol, unless --tunitinp.",
      4
    },





    {
      0, 0, 0, 0,
      "WCS parameters:",
      5
    },
    {
      "crpix1",
      510,
      "FLT",
      0,
      "Pixel coordinate of reference point (axis 1).",
      5
    },
    {
      "crpix2",
      511,
      "FLT",
      0,
      "Pixel coordinate of reference point (axis 2).",
      5
    },
    {
      "crval1",
      512,
      "FLT",
      0,
      "Right ascension at reference point (degrees).",
      5
    },
    {
      "crval2",
      513,
      "FLT",
      0,
      "Declination at reference point (degrees).",
      5
    },
    {
      "resolution",
      514,
      "FLT",
      0,
      "Resolution of image (arcseconds/pixel).",
      5
    },


    {0}
  };



















/* Parse a single option: */
static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
  size_t tmp;

  /* Save the arguments structure: */
  struct mkprofparams *p = state->input;

  /* Set the pointer to the common parameters for all programs
     here: */
  state->child_inputs[0]=&p->cp;

  /* In case the user incorrectly uses the equal sign (for example
     with a short format or with space in the long format, then `arg`
     start with (if the short version was called) or be (if the long
     version was called with a space) the equal sign. So, here we
     check if the first character of arg is the equal sign, then the
     user is warned and the program is stopped: */
  if(arg && arg[0]=='=')
    argp_error(state, "Incorrect use of the equal sign (`=`). For short "
	       "options, `=` should not be used and for long options, "
	       "there should be no space between the option, equal sign "
	       "and value.");

  switch(key)
    {

    /* Operating modes:  */
    case 509:
      p->psfinimg=1;
      break;
    case 'i':
      p->individual=1;
      break;

    /* Output: */
    case 'x':
      sizetlzero(arg, &tmp, "naxis1", key, p->cp.spack, NULL, 0);
      p->naxes[0]=tmp;
      p->up.naxis1set=1;
      break;
    case 'y':
      sizetlzero(arg, &tmp, "naxis2", key, p->cp.spack, NULL, 0);
      p->naxes[1]=tmp;
      p->up.naxis2set=1;
      break;
    case 's':
      sizetlzero(arg, &p->oversample, "oversample", key,
		 p->cp.spack, NULL, 0);
      p->up.oversampleset=1;
      break;
    case 'm':
      p->nomerged=1;
      break;

    /* Profiles: */
    case 'r':
      sizetlzero(arg, &p->numrandom, "numrandom", key, p->cp.spack, NULL, 0);
      p->up.numrandomset=1;
      break;
    case 'l':
      floatl0(arg, &p->tolerance, "tolerance", key, p->cp.spack, NULL, 0);
      p->up.toleranceset=1;
      break;
    case 'z':
      floatl0(arg, &p->zeropoint, "zeropoint", key, p->cp.spack, NULL, 0);
      p->up.zeropointset=1;
      break;
    case 'p':
      p->tunitinp=1;
      p->up.tunitinpset=1;
      break;
    case 'c':
      p->up.prepforconv=1;
      p->up.prepforconvset=1;
      break;
    case 'X':
      sizetelzero(arg, &tmp, "xshift", key, p->cp.spack, NULL, 0);
      p->shift[0]=tmp;
      p->up.xshiftset=1;
      break;
    case 'Y':
      sizetelzero(arg, &tmp, "yshift", key, p->cp.spack, NULL, 0);
      p->shift[1]=tmp;
      p->up.yshiftset=1;
      break;

   /* Catalog */
    case 500:
      sizetelzero(arg, &p->xcol, "xcol", ' ', p->cp.spack, NULL, 0);
      p->up.xcolset=1;
      break;
    case 501:
      sizetelzero(arg, &p->ycol, "ycol", ' ', p->cp.spack, NULL, 0);
      p->up.ycolset=1;
      break;
    case 502:
      sizetelzero(arg, &p->fcol, "fcol", ' ', p->cp.spack, NULL, 0);
      p->up.fcolset=1;
      break;
    case 503:
      sizetelzero(arg, &p->rcol, "rcol", ' ', p->cp.spack, NULL, 0);
      p->up.rcolset=1;
      break;
    case 504:
      sizetelzero(arg, &p->ncol, "ncol", ' ', p->cp.spack, NULL, 0);
      p->up.ncolset=1;
      break;
    case 505:
      sizetelzero(arg, &p->pcol, "pcol", ' ', p->cp.spack, NULL, 0);
      p->up.pcolset=1;
      break;
    case 506:
      sizetelzero(arg, &p->pcol, "qcol", ' ', p->cp.spack, NULL, 0);
      p->up.qcolset=1;
      break;
    case 507:
      sizetelzero(arg, &p->mcol, "mcol", ' ', p->cp.spack, NULL, 0);
      p->up.mcolset=1;
      break;
    case 508:
      sizetelzero(arg, &p->tcol, "tcol", ' ', p->cp.spack, NULL, 0);
      p->up.tcolset=1;
      break;



    /* WCS parameters: */
    case 510:
      anydouble(arg, &p->crpix[0], "crpix1", key, p->cp.spack, NULL, 0);
      p->up.crpix1set=1;
      break;
    case 511:
      anydouble(arg, &p->crpix[1], "crpix2", key, p->cp.spack, NULL, 0);
      p->up.crpix2set=1;
      break;
    case 512:
      anydouble(arg, &p->crval[0], "crval1", key, p->cp.spack, NULL, 0);
      p->up.crval1set=1;
      break;
    case 513:
      anydouble(arg, &p->crval[1], "crval2", key, p->cp.spack, NULL, 0);
      p->up.crval2set=1;
      break;
    case 514:
      floatl0(arg, &p->resolution, "resolution", key, p->cp.spack, NULL, 0);
      p->up.resolutionset=1;
      break;






    /* Read the non-option arguments: */
    case ARGP_KEY_ARG:

      /* See what type of input value it is and put it in. */
      if( nameisfits(arg) )
	{
	  if(p->up.psfname)
	    argp_error(state, "Only one input FITS image (the PSF) "
		       "should be input. You have given more.");
	  else
	    p->up.psfname=arg;
	}
      else
	{
	  if(p->up.catname)
	    argp_error(state, "Only one catalog file can be given.");
	  else
	    p->up.catname=arg;
	}
      break;






    /* The command line options and arguments are finished. */
    case ARGP_KEY_END:
      if(p->cp.setdirconf==0 && p->cp.setusrconf==0
	 && p->cp.printparams==0)
	{
	  if(state->arg_num==0)
	    argp_error(state, "No argument given!");
	  if(p->up.catname==NULL)
	    argp_error(state, "No catalog provided!");
	}
      break;






    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}





/* Specify the children parsers: */
struct argp_child children[]=
  {
    {&commonargp, 0, NULL, 0},
    {0, 0, 0, 0}
  };





/* Basic structure defining the whole argument reading process. */
static struct argp thisargp = {options, parse_opt, args_doc,
			       doc, children, NULL, NULL};

#endif
