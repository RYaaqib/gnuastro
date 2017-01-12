/*********************************************************************
Table - View and manipulate a FITS table structures.
Table is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <akhlaghi@gnu.org>
Contributing author(s):
Copyright (C) 2016, Free Software Foundation, Inc.

Gnuastro is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Gnuastro is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with Gnuastro. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
#include <config.h>

#include <argp.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <fitsio.h>

#include <gnuastro/fits.h>
#include <gnuastro/table.h>
#include <gnuastro/linkedlist.h>

#include <nproc.h>               /* From Gnulib.                   */
#include <timing.h>              /* Includes time.h and sys/time.h */
#include <checkset.h>
#include <fixedstringmacros.h>

#include "main.h"

#include "ui.h"
#include "args.h"
#include "cite.h"


/* Set the file names of the places where the default parameters are
   put. */
#define SYSCONFIG_FILE SYSCONFIG_DIR "/" CONFIG_FILE
#define USERCONFIG_FILEEND USERCONFIG_DIR CONFIG_FILE
#define CURDIRCONFIG_FILE CURDIRCONFIG_DIR CONFIG_FILE











/**************************************************************/
/***************       Sanity Check         *******************/
/**************************************************************/
static void
ui_option_is_mandatory(char *name)
{
  error(EXIT_FAILURE, 0, "`%s' option is mandatory", name);
}





void
fill_params_from_options(struct tableparams *p)
{
  size_t i;

  /* Put the program's option values into the structure. */
  for(i=0; !gal_options_is_last(&options[i]); ++i)
    if( options[i].key && options[i].name )
      switch(options[i].key)
        {
        /* Inputs */
        case ARGS_OPTION_COLUMN_KEY:
          gal_linked_list_copy_stll(options[i].value, &p->columns);
          break;

        case ARGS_OPTION_SEARCHIN_KEY:
          if(options[i].value)
            p->searchin=gal_table_string_to_searchin(options[i].value);
          else ui_option_is_mandatory((char *)options[i].name);
          break;

        case ARGS_OPTION_IGNORECASE_KEY:
          p->ignorecase = *(unsigned char *)options[i].value;
          break;


        /* Output */
        case ARGS_OPTION_TABLETYPE_KEY:

          /* Set the tabletype parameter. */
          if(options[i].value)
            p->tabletype=gal_table_string_to_type(options[i].value);
          else if( gal_fits_name_is_fits(p->cp.output) )
            ui_option_is_mandatory((char *)options[i].name);


          /* If the output name was set and is a FITS file, make sure that
             the type of the table is not a `txt'. */
          if( p->cp.output && gal_fits_name_is_fits(p->cp.output)
              && ( p->tabletype !=GAL_TABLE_TYPE_AFITS
                   && p->tabletype !=GAL_TABLE_TYPE_BFITS ) )
            error(EXIT_FAILURE, 0, "desired output file `%s' is a FITS "
                  "file, but `tabletype' is not a FITS table type. "
                  "Please set it to `fits-ascii', or `fits-binary'",
                  p->cp.output);
          break;


        /* Operating mode */
        case ARGS_OPTION_INFORMATION_KEY:
          if(options[i].value)
            p->information = *(unsigned char *)options[i].value;
          break;


        default:
          error(EXIT_FAILURE, 0, "option key %d not recognized in "
                "`fill_params_from_options'", options[i].key);
        }
}





void
sanitycheck(struct tableparams *p)
{


}




















/**************************************************************/
/***************       Preparations         *******************/
/**************************************************************/
void
preparearrays(struct tableparams *p)
{
  char *numstr;
  int tabletype;
  gal_data_t *allcols;
  size_t i, numcols, numrows;

  /* If there were no columns specified, we want the full set of
     columns. */
  if(p->columns==NULL)
    {
      /* Read the table information for the number of columns and rows. */
      allcols=gal_table_info(p->up.filename, p->cp.hdu, &numcols,
                             &numrows, &tabletype);

      /* If there was no actual data in the file, then inform the user */
      if(allcols==NULL)
        error(EXIT_FAILURE, 0, "%s: no usable data rows", p->up.filename);

      /* If the user just wanted information, then print it. */
      if(p->information)
        {
          /* Print the file information. */
          printf("--------\n");
          printf("%s", p->up.filename);
          if(gal_fits_name_is_fits(p->up.filename))
            printf(" (hdu: %s)\n", p->cp.hdu);
          else
            printf("\n");

          /* Print each column's information. */
          gal_table_print_info(allcols, numcols, numrows);
        }

      /* Free the information from all the columns. */
      for(i=0;i<numcols;++i)
        gal_data_free(&allcols[i], 1);
      free(allcols);

      /* Add the number of columns to the list if the user wanted to print
         the columns (didn't just want their information. */
      if(p->information)
        {
          freeandreport(p);
          exit(EXIT_SUCCESS);
        }
      else
        for(i=1;i<=numcols;++i)
          {
            asprintf(&numstr, "%zu", i);
            gal_linkedlist_add_to_stll(&p->columns, numstr, 0);
          }
    }

  /* Reverse the list of column search criteria that we are looking for
     (since this is a last-in-first-out linked list, the order that
     elements were added to the list is the reverse of the order that they
     will be popped). */
  gal_linkedlist_reverse_stll(&p->columns);
  p->table=gal_table_read(p->up.filename, p->cp.hdu, p->columns,
                          p->searchin, p->ignorecase, p->cp.minmapsize);

  /* If there was no actual data in the file, then inform the user and
     abort. */
  if(p->table==NULL)
    error(EXIT_FAILURE, 0, "%s: no usable data rows (non-commented and "
          "non-blank lines)", p->up.filename);

  /* Now that the data columns are ready, we can free the string linked
     list. */
  gal_linkedlist_free_stll(p->columns, 1);
}



















/**************************************************************/
/************         Set the parameters          *************/
/**************************************************************/
void
setparams(int argc, char *argv[], struct tableparams *p)
{
  struct uiparams *up=&p->up;
  struct gal_options_common_params *cp=&p->cp;

  /* Set the non-zero initial values, the structure was initialized to
     have a zero value for all elements. */
  cp->numthreads = num_processors(NPROC_CURRENT);

  /* Initialize this utility's pointers to NULL. */
  p->columns=NULL;
  up->filename=NULL;

  /* Read the command-line arguments. */
  errno=0;
  if(argp_parse(&thisargp, argc, argv, 0, 0, p))
    error(EXIT_FAILURE, errno, "parsing arguments");

  /* Read the configuration files. */
  gal_options_config_files(PROG_EXEC, PROG_NAME, options,
                           gal_commonopts_options, &p->cp);

  /* Fill the parameters from the options. */
  fill_params_from_options(p);


  /* Do a sanity check. */
  sanitycheck(p);

  /* Print the necessary information if asked. Note that this needs to be
     done after the sanity check so un-sane values are not printed in the
     output state. */
  gal_options_print_state(PROG_NAME, bibtex, options,
                          gal_commonopts_options);

  /* Read/allocate all the necessary starting arrays */
  preparearrays(p);

  /* Free all the allocated spaces in the option structures. */
  gal_options_free(options);
  gal_options_free(gal_commonopts_options);
}




















/**************************************************************/
/************      Free allocated, report         *************/
/**************************************************************/
void
freeandreport(struct tableparams *p)
{
  /* Free the allocated arrays: */
  free(p->cp.hdu);
  free(p->cp.output);
  gal_data_free_ll(p->table);
}
