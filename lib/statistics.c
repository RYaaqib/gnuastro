/*********************************************************************
Statistical functions.
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
along with Gnuastro. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
#include <config.h>

#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <float.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <gnuastro/data.h>
#include <gnuastro/tile.h>
#include <gnuastro/fits.h>
#include <gnuastro/blank.h>
#include <gnuastro/qsort.h>
#include <gnuastro/arithmetic.h>
#include <gnuastro/statistics.h>

#include <gnuastro-internal/checkset.h>










/****************************************************************
 ********               Simple statistics                 *******
 ****************************************************************/
/* Return the number of non-blank elements in an array as a single element,
   uint64 type data structure. */
gal_data_t *
gal_statistics_number(gal_data_t *input)
{
  uint64_t counter=0;
  size_t dsize=1;
  gal_data_t *out=gal_data_alloc(NULL, GAL_TYPE_UINT64, 1, &dsize,
                                 NULL, 1, -1, NULL, NULL, NULL);

  /* If there is no blank values in the input, then the total number is
     just the size. */
  if(gal_blank_present(input, 0)) /* `{}' necessary for `else'. */
    { GAL_TILE_PARSE_OPERATE(input, NULL, 0, 1, {++counter;}); }
  else
    counter = input->size;

  /* Write the value into memory. */
  *((uint64_t *)(out->array)) = counter;
  return out;
}





/* Return the minimum (non-blank) value of a dataset in the same type as
   the dataset. Note that a NaN (blank in floating point) will fail on any
   comparison. So when finding the minimum or maximum, when the blank value
   is NaN, we can safely assume there is no blank value at all. */
gal_data_t *
gal_statistics_minimum(gal_data_t *input)
{
  size_t dsize=1, n=0;
  gal_data_t *out=gal_data_alloc(NULL, gal_tile_block(input)->type, 1,
                                 &dsize, NULL, 1, -1, NULL, NULL, NULL);

  /* Initialize the output with the maximum possible value. */
  gal_type_max(out->type, out->array);

  /* Parse the full input. */
  GAL_TILE_PARSE_OPERATE(input, out, 0, 0, {*o = *i < *o ? *i : *o; ++n;});

  /* If there were no usable elements, set the output to blank, then
     return. */
  if(n==0) gal_blank_write(out->array, out->type);
  return out;
}





/* Return the maximum (non-blank) value of a dataset in the same type as
   the dataset. See explanations of `gal_statistics_minimum'. */
gal_data_t *
gal_statistics_maximum(gal_data_t *input)
{
  size_t dsize=1, n=0;
  gal_data_t *out=gal_data_alloc(NULL, gal_tile_block(input)->type, 1,
                                 &dsize, NULL, 1, -1, NULL, NULL, NULL);

  /* Initialize the output with the minimum possible value. */
  gal_type_min(out->type, out->array);

  /* Parse the full input. */
  GAL_TILE_PARSE_OPERATE(input, out, 0, 0, {*o = *i > *o ? *i : *o; ++n;});

  /* If there were no usable elements, set the output to blank, then
     return. */
  if(n==0) gal_blank_write(out->array, out->type);
  return out;
}





/* Return the sum of the input dataset as a single element dataset of type
   float64. */
gal_data_t *
gal_statistics_sum(gal_data_t *input)
{
  size_t dsize=1, n=0;
  gal_data_t *out=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &dsize,
                                 NULL, 1, -1, NULL, NULL, NULL);

  /* Parse the dataset. Note that in `gal_data_alloc' we set the `clear'
     flag to 1, so it will be 0.0f. */
  GAL_TILE_PARSE_OPERATE(input, out, 0, 1, {++n; *o += *i;});

  /* If there were no usable elements, set the output to blank, then
     return. */
  if(n==0) gal_blank_write(out->array, out->type);
  return out;
}





/* Return the mean of the input dataset as a float64 type single-element
   dataset. */
gal_data_t *
gal_statistics_mean(gal_data_t *input)
{
  size_t dsize=1, n=0;
  gal_data_t *out=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &dsize,
                                 NULL, 1, -1, NULL, NULL, NULL);

  /* Parse the dataset. Note that in `gal_data_alloc' we set the `clear'
     flag to 1, so it will be 0.0f. */
  GAL_TILE_PARSE_OPERATE(input, out, 0, 1, {++n; *o += *i;});

  /* Above, we calculated the sum and number, so if there were any elements
     in the dataset (`n!=0'), divide the sum by the number, otherwise, put
     a blank value in the output. */
  if(n) *((double *)(out->array)) /= n;
  else gal_blank_write(out->array, out->type);
  return out;
}





/* Return the standard deviation of the input dataset as a single element
   dataset of type float64. */
gal_data_t *
gal_statistics_std(gal_data_t *input)
{
  size_t dsize=1, n=0;
  double s=0.0f, s2=0.0f;
  gal_data_t *out=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &dsize,
                                 NULL, 1, -1, NULL, NULL, NULL);

  /* Parse the input. */
  GAL_TILE_PARSE_OPERATE(input, out, 0, 1, {++n; s += *i; s2 += *i * *i;});

  /* Write the standard deviation into the output. */
  *((double *)(out->array)) = n ? sqrt( (s2-s*s/n)/n ) : GAL_BLANK_FLOAT64;
  return out;
}





/* Return the mean and standard deviation of a dataset in one run in type
   float64. The output is a two element data structure, with the first
   value being the mean and the second value the standard deviation. */
gal_data_t *
gal_statistics_mean_std(gal_data_t *input)
{
  size_t dsize=2, n=0;
  double s=0.0f, s2=0.0f;
  gal_data_t *out=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &dsize,
                                 NULL, 1, -1, NULL, NULL, NULL);

  /* Parse the input. */
  GAL_TILE_PARSE_OPERATE(input, out, 0, 1, {++n; s += *i; s2 += *i * *i;});

  /* Write in the output values and return. */
  ((double *)(out->array))[0] = n ? s/n                  : GAL_BLANK_FLOAT64;
  ((double *)(out->array))[1] = n ? sqrt( (s2-s*s/n)/n ) : GAL_BLANK_FLOAT64;
  return out;
}





/* The input is a sorted array with no blank values, we want the median
   value to be put inside the already allocated space which is pointed to
   by `median'. It is in the same type as the input. */
#define MED_IN_SORTED(IT) {                                             \
    IT *a=sorted->array;                                                \
    *(IT *)median = n%2 ? a[n/2]  : (a[n/2]+a[n/2-1])/2;                \
  }
static void
statistics_median_in_sorted_no_blank(gal_data_t *sorted, void *median)
{
  size_t n=sorted->size;

  switch(sorted->type)
    {
    case GAL_TYPE_UINT8:     MED_IN_SORTED( uint8_t  );    break;
    case GAL_TYPE_INT8:      MED_IN_SORTED( int8_t   );    break;
    case GAL_TYPE_UINT16:    MED_IN_SORTED( uint16_t );    break;
    case GAL_TYPE_INT16:     MED_IN_SORTED( int16_t  );    break;
    case GAL_TYPE_UINT32:    MED_IN_SORTED( uint32_t );    break;
    case GAL_TYPE_INT32:     MED_IN_SORTED( int32_t  );    break;
    case GAL_TYPE_UINT64:    MED_IN_SORTED( uint64_t );    break;
    case GAL_TYPE_INT64:     MED_IN_SORTED( int64_t  );    break;
    case GAL_TYPE_FLOAT32:   MED_IN_SORTED( float    );    break;
    case GAL_TYPE_FLOAT64:   MED_IN_SORTED( double   );    break;
    default:
      error(EXIT_FAILURE, 0, "%s: type code %d not recognized",
            __func__, sorted->type);
    }
}





/* Return the median value of the dataset in the same type as the input as
   a one element dataset. If the `inplace' flag is set, the input data
   structure will be modified: it will have no blank values and will be
   sorted (increasing). */
gal_data_t *
gal_statistics_median(gal_data_t *input, int inplace)
{
  size_t dsize=1;
  gal_data_t *nbs=gal_statistics_no_blank_sorted(input, inplace);;
  gal_data_t *out=gal_data_alloc(NULL, nbs->type, 1, &dsize, NULL, 1, -1,
                                 NULL, NULL, NULL);

  /* Write the median. */
  statistics_median_in_sorted_no_blank(nbs, out->array);

  /* Clean up (if necessary), then return the output */
  if(nbs!=input) gal_data_free(nbs);
  return out;
}




/* For a given size, return the index (starting from zero) that is at the
   given quantile.  */
size_t
gal_statistics_quantile_index(size_t size, double quantile)
{
  double floatindex;

  if(quantile<0.0f || quantile>1.0f)
    error(EXIT_FAILURE, 0, "%s: the input quantile should be between 0.0 "
          "and 1.0 (inclusive). You have asked for %g", __func__, quantile);

  /* Find the index of the quantile. */
  floatindex=(double)(size-1)*quantile;

  /*
  printf("quantile: %f, size: %zu, findex: %f\n", quantile, size, floatindex);
  */
  /* Note that in the conversion from float to size_t, the floor
     integer value of the float will be used. */
  if( floatindex - (int)floatindex > 0.5 )
    return floatindex+1;
  else
    return floatindex;
}





/* Return a single element dataset of the same type as input keeping the
   value that has the given quantile. */
gal_data_t *
gal_statistics_quantile(gal_data_t *input, double quantile, int inplace)
{
  size_t dsize=1, index;
  gal_data_t *nbs=gal_statistics_no_blank_sorted(input, inplace);
  gal_data_t *out=gal_data_alloc(NULL, nbs->type, 1, &dsize,
                                 NULL, 1, -1, NULL, NULL, NULL);

  /* Find the index of the quantile. */
  index=gal_statistics_quantile_index(nbs->size, quantile);

  /* Write the value at this index into the output. */
  memcpy(out->array, gal_data_ptr_increment(nbs->array, index, nbs->type),
         gal_type_sizeof(nbs->type));

  /* Clean up and return. */
  if(nbs!=input) gal_data_free(nbs);
  return out;
}





/* Return the index of the (first) point in the sorted dataset that has the
   closest value to `value' (which has to be the same type as the `input'
   dataset). */
#define STATS_QFUNC(IT) {                                               \
    IT *r, *a=nbs->array, *af=a+nbs->size, v=*((IT *)(value->array));   \
                                                                        \
    /* For a reference. Since we are comparing with the previous. */    \
    /* element, we need to start with the second element.*/             \
    r=a++;                                                              \
                                                                        \
    /* Increasing array: */                                             \
    if( *a < *(a+1) )                                                   \
      do if(*a>v) { if( v - *(a-1) < *a - v ) --a; break; } while(++a<af); \
                                                                        \
    /* Decreasing array. */                                             \
    else                                                                \
      do if(*a<v) { if( *(a-1) - v < v - *a ) --a; break; } while(++a<af); \
                                                                        \
    /* Set the difference. */                                           \
    if(a<af) index=a-r;                                                 \
  }
size_t
gal_statistics_quantile_function_index(gal_data_t *input, gal_data_t *value,
                                       int inplace)
{
  size_t index=-1;
  gal_data_t *nbs=gal_statistics_no_blank_sorted(input, inplace);

  /* A sanity check. */
  if(nbs->type!=value->type)
    error(EXIT_FAILURE, 0, "%s: the types of the input dataset and requested "
          "value have to be the same", __func__);

  /* Find the result: */
  switch(nbs->type)
    {
    case GAL_TYPE_UINT8:     STATS_QFUNC( uint8_t  );     break;
    case GAL_TYPE_INT8:      STATS_QFUNC( int8_t   );     break;
    case GAL_TYPE_UINT16:    STATS_QFUNC( uint16_t );     break;
    case GAL_TYPE_INT16:     STATS_QFUNC( int16_t  );     break;
    case GAL_TYPE_UINT32:    STATS_QFUNC( uint32_t );     break;
    case GAL_TYPE_INT32:     STATS_QFUNC( int32_t  );     break;
    case GAL_TYPE_UINT64:    STATS_QFUNC( uint64_t );     break;
    case GAL_TYPE_INT64:     STATS_QFUNC( int64_t  );     break;
    case GAL_TYPE_FLOAT32:   STATS_QFUNC( float    );     break;
    case GAL_TYPE_FLOAT64:   STATS_QFUNC( double   );     break;
    default:
      error(EXIT_FAILURE, 0, "%s: type code %d not recognized",
            __func__, nbs->type);
    }

  /* Clean up and return. */
  if(nbs!=input) gal_data_free(nbs);
  return index;
}





/* Return the quantile function of the given value as float64. */
gal_data_t *
gal_statistics_quantile_function(gal_data_t *input, gal_data_t *value,
                                 int inplace)
{
  double *d;
  size_t dsize=1;
  gal_data_t *out=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &dsize,
                                 NULL, 1, -1, NULL, NULL, NULL);
  size_t ind=gal_statistics_quantile_function_index(input, value, inplace);

  /* Note that counting of the index starts from 0, so for the quantile we
     should divided by (size - 1). */
  d=out->array;
  d[0] = ( ind==-1 ? NAN : ((double)ind) / ((double)(input->size - 1)) );
  return out;
}




















/*********************************************************************/
/*****************              Mode           ***********************/
/*********************************************************************/
/* Main structure to keep mode parameters. */
struct statistics_mode_params
{
  gal_data_t   *data;   /* Sorted input dataset with no blank values. */
  size_t        lowi;   /* Lower quantile of interval.                */
  size_t        midi;   /* Index of the mid-interval point.           */
  size_t        midd;   /* Maximum CDF distance at the middle point.  */
  size_t       highi;   /* Higher quantile of interval.               */
  float    tolerance;   /* Tolerance level to terminate search.       */
  size_t    numcheck;   /* Number of pixels after mode to check.      */
  size_t    interval;   /* Interval to check pixels.                  */
  float   mirrordist;   /* Distance after mirror to check ( x STD).   */
};





/* Macros for the mode finding algorithm. */
#define MODE_MIN_Q        0.01f  /* Mode search lower interval quantile.  */
#define MODE_MAX_Q        0.55f  /* Mode search higher interval quantile. */
#define MODE_GOOD_LQ      0.02f  /* Least acceptable mode quantile.       */
#define MODE_SYM_LOW_Q    0.01f  /* Lower quantile to get symmetricity.   */
#define MODE_GOLDEN_RATIO 1.618034f /* Golden ratio: (1+sqrt(5))/2.       */
#define MODE_TWO_TAKE_GR  0.38197f  /* 2 - Golden ratio.                  */
#define MODE_MIRROR_ABOVE (size_t)(-1) /* Mirror is above the result.     */




/*
  Given a mirror point (`m'), return the maximum distance between the
  mirror distribution and the original distribution.

  The basic idea behind finding the mode is comparing the mirrored CDF
  (where the mirror is a test for the mode) with the original CDF for a
  given point. The job of this function is to return the maximum distance,
  given a mirror point. It takes the index of the mirror that is to be
  checked, it then finds the maximum difference between the mirrored CDF
  about the given point and the input CDF.

  `zf` keeps the value at the mirror (zero) point.  `i` is used to count
  the pixels after the mirror in the mirror distribution. So `m+i` is the
  index of the mirrored distribution and mf=zf+(zf-a[m-i])=2*zf-a[m-i] is
  the mirrored flux at this point. Having found `mf', we find the `j` such
  that a[m+j] has the nearest flux to `mf`.

  The desired difference between the input CDF and the mirrored one
  for each `i` is then simply: `j-i`.

  Once `i` is incremented, `mf` will increase, so to find the new `j` we
  don't need to begin looking from `j=0`. Remember that the array is
  sorted, so the desired `j` is definitely larger than the previous
  `j`. So, if we keep the previous `j` in `prevj` then, all we have to do
  is to start incrementing `j` from `prevj`. This will really help in
  speeding up the job :-D. Only for the first element, `prevj=0`. */
#define MIRR_MAX_DIFF(IT) {                                             \
    IT *a=p->data->array, zf=a[m], mf=2*zf-a[m-i];                      \
                                                                        \
    /* When a[m+j]>mf, we have reached the last pixel to check. Now, */ \
    /* we just have to see which one of a[m+j-1] or a[m+j] is closer */ \
    /* to `mf'. We then change `j` accordingly and break out of the  */ \
    /* `j' loop. */                                                     \
    for(j=prevj;j<size-m;++j)                                           \
      if(a[m+j]>mf)                                                     \
        {                                                               \
          if( a[m+j]-mf < mf-a[m+j-1] )                                 \
            break;                                                      \
          else                                                          \
            {                                                           \
              j--;                                                      \
              break;                                                    \
            }                                                           \
        }                                                               \
  }

static size_t
mode_mirror_max_index_diff(struct statistics_mode_params *p, size_t m)
{
  /* The variables:
   i:        Index on mirror distribution.
   j:        Index on input distribution.
   prevj:    Index of previously checked point in the actual array.
   mf:       (in macro) Value that is approximately equal in both
             distributions.                                          */
  size_t i, j, absdiff, prevj=0, size=p->data->size;
  size_t  maxdiff=0, errordiff=p->mirrordist*sqrt(m);

  /*
  printf("###############\n###############\n");
  printf("### Mirror pixel: %zu (mirrordist: %f, sqrt(m): %f)\n", m,
         p->mirrordist, sqrt(m));
  printf("###############\n###############\n");
  */

  /* Go over the mirrored points. */
  for(i=1; i<p->numcheck && i<=m && m+i<size ;i+=p->interval)
    {
      /* Find `j': the index of the closest point in the original
         distribution that has a value similar to the mirror
         distribution. */
      switch(p->data->type)
        {
        case GAL_TYPE_UINT8:     MIRR_MAX_DIFF( uint8_t  );   break;
        case GAL_TYPE_INT8:      MIRR_MAX_DIFF( int8_t   );   break;
        case GAL_TYPE_UINT16:    MIRR_MAX_DIFF( uint16_t );   break;
        case GAL_TYPE_INT16:     MIRR_MAX_DIFF( int16_t  );   break;
        case GAL_TYPE_UINT32:    MIRR_MAX_DIFF( uint32_t );   break;
        case GAL_TYPE_INT32:     MIRR_MAX_DIFF( int32_t  );   break;
        case GAL_TYPE_UINT64:    MIRR_MAX_DIFF( uint64_t );   break;
        case GAL_TYPE_INT64:     MIRR_MAX_DIFF( int64_t  );   break;
        case GAL_TYPE_FLOAT32:   MIRR_MAX_DIFF( float    );   break;
        case GAL_TYPE_FLOAT64:   MIRR_MAX_DIFF( double   );   break;
        default:
          error(EXIT_FAILURE, 0, "%s: type code %d not recognized",
                __func__, p->data->type);
        }

      /*
      printf("i:%-5zu j:%-5zu diff:%-5d maxdiff: %zu\n",
             i, j, (int)j-(int)i, maxdiff);
      */

      /* The index of the actual CDF corresponding the the mirrored flux
         has been found. We want the mirrored distribution to be within the
         actual distribution, not beyond it, so the only acceptable results
         are when i<j. But we also have noise, so we can't simply use that
         as the criterion, small `j's with `i>j' are acceptable. So, only
         when `i>j+errordiff' the result is not acceptable! */
      if(i>j+errordiff)
        {
          maxdiff = MODE_MIRROR_ABOVE;
          break;
        }
      absdiff  = i>j ? i-j : j-i;
      if(absdiff>maxdiff) maxdiff=absdiff;

      prevj=j;
    }

  /* Return the maximum difference  */
  return maxdiff;
}





/* Find the mode through the Golden-section search. It is assumed that
   `mode_mirror_max_index_diff' has one minimum (within the statistical
   errors) in the function. To find that minimum, the golden section search
   algorithm is going to used. Read the Wikipedia article for a very nice
   introduction.

   In summary we will constantly be finding middle points in the given
   interval and thus decreasing the interval until a certain tolerance is
   reached.

   If the input interval is on points `a' and `b', then the middle point
   (lets call it `c', where c>a and c<b) to test should be positioned such
   that (b-c)/(c-a)=MODE_GOLDEN_RATIO. Once we open up this relation, we
   can find c using:

    c = ( b + MODE_GOLDEN_RATIO * a ) / ( 1 + MODE_GOLDEN_RATIO )

   We need a fourth point to be placed between. With this configuration,
   the probing point is located at: */
static size_t
mode_golden_section(struct statistics_mode_params *p)
{
  size_t di, dd;

  /* Find the probing point in the larger interval. */
  if(p->highi-p->midi > p->midi-p->lowi)
    di = p->midi + MODE_TWO_TAKE_GR * (float)(p->highi-p->midi);
  else
    di = p->midi - MODE_TWO_TAKE_GR * (float)(p->midi-p->lowi);

  /* Since these are all indexs (and positive) we don't need an absolute
     value, highi is also always larger than lowi! In some cases, the first
     (standard) condition might be satisfied, while highi-lowi<=2. In such
     cases, also jump out! */
  if( (p->highi - p->lowi) < p->tolerance*(p->midi+di)
      || (p->highi - p->lowi) <= 3)
    return (p->highi+p->lowi)/2;

  /* Find the maximum difference for this mirror point. */
  dd = mode_mirror_max_index_diff(p, di);

  /*------------------------------------------------------------------
  {
  static int counter=1;
  char outname[500], command[1000];
  char histsname[500], cfpsname[500];
  sprintf(outname, "%dcmp.pdf", counter);
  sprintf(cfpsname, "%dcfps.txt", counter);
  sprintf(histsname, "%dhists.txt", counter);
  gal_mode_make_mirror_plots(p->sorted, p->size, di, histsname, cfpsname);
  sprintf(command, "./plot.py %s %s %s", histsname, cfpsname, outname);
  system(command);
  }
  -------------------------------------------------------------------*/

  /*
  printf("lowi:%-5zu\tmidi:%-5zu(midd: %d)\thighi:%-5zu ----> "
         "dq: %-5zu di: %d\n",
         p->lowi, p->midi, (int)p->midd, p->highi,
         di, (int)dd);
  */

  /* +++++++++++++ Start of addition to the golden section search.

     The mirrored distribution's cumulative frequency plot has be lower
     than the actual's cfp. If it isn't, `di` will be MODE_MIRROR_ABOVE. In
     this case, the normal golden section minimization is not going to give
     us what we want. So we have this modification. In such cases, we want
     the search to go to the lower interval. */
  if(dd==MODE_MIRROR_ABOVE)
    {
      if( p->midi < di )
        {
          p->highi=di;
          return mode_golden_section(p);
        }
      else
        {
          p->highi=p->midi;
          p->midi=di;
          p->midd=dd;
          return mode_golden_section(p);
        }
    }
  /* End of addition to the golden section search. +++++++++++++*/

  /* This is the standard golden section search: */
  if(dd<p->midd)
    {
      if(p->highi-p->midi > p->midi-p->lowi)
        {
          p->lowi  = p->midi;
          p->midi  = di;
          p->midd  = dd;
          return mode_golden_section(p);
        }
      else
        {
          p->highi = p->midi;
          p->midi  = di;
          p->midd  = dd;
          return mode_golden_section(p);
        }
    }
  else
    {
      if(p->highi-p->midi > p->midi-p->lowi)
        {
          p->highi = di;
          return mode_golden_section(p);
        }
      else
        {
          p->lowi  = di;
          return mode_golden_section(p);
        }
    }
}





/* Once the mode is found, we need to do a quality control. This quality
   control is the measure of its symmetricity. Let's assume the mode index
   is at `m', since an index is just a count, from the Poisson
   distribution, the error in `m' is sqrt(m).

   Now, let's take `b' to be the first point that the difference between
   the cumulative distribution of the mirror and actual data deviate more
   than sqrt(m). For a scale parameter, lets assume that the index of 5% of
   `m` is `a`. We could have taken the distribution minimum, but the
   scatter in the minimum can be too high!

   Now, the "symmetricity" of the mode can be defined as: (b-m)/(m-a). For
   a completly symmetric mode, this should be 1. Note that the search for
   `b` only goes to the 95% of the distribution.  */
#define MODE_SYM(IT) {                                                  \
    IT *a=p->data->array, af=0, bf=0, mf=0, fi;                         \
                                                                        \
    /* Set the values at the mirror and at `a' (see above). */          \
    mf=a[m];                                                            \
    af=a[ gal_statistics_quantile_index(2*m+1, MODE_SYM_LOW_Q) ];       \
                                                                        \
    /* This loop is very similar to that of */                          \
    /* `mode_mirror_max_index_diff'. It will find the index where the */\
    /* difference between the two cumulative frequency plots exceeds */ \
    /* that of the error in the mirror index.*/                         \
    for(i=1; i<topi-m ;i+=1)                                            \
      {                                                                 \
        fi=2*mf-a[m-i];                                                 \
                                                                        \
        for(j=prevj;j<size-m;++j)                                       \
          if(a[m+j]>fi)                                                 \
            {                                                           \
              if( a[m+j]-fi < fi-a[m+j-1] )                             \
                break;                                                  \
              else                                                      \
                {                                                       \
                  j--;                                                  \
                  break;                                                \
                }                                                       \
            }                                                           \
                                                                        \
        if(i>j+errdiff || j>i+errdiff)                                  \
          {                                                             \
            bi=m+i;                                                     \
            break;                                                      \
          }                                                             \
        prevj=j;                                                        \
      }                                                                 \
                                                                        \
    /* bi==0 shows that no point with a larger difference could be */   \
    /* found. So bi should be set to the end of the search region. */   \
    if(bi==0) bi=topi;                                                  \
                                                                        \
    bf = *(IT *)b_val = a[bi];                                          \
    /*printf("%zu: %f,%f,%f\n", m, (double)af, (double)mf, (double)bf);*/ \
                                                                        \
    /* For a bad result, return 0 (which will not output any mode). */  \
    return bf==af ? 0 : (bf-mf)/(mf-af);                                \
  }
static double
mode_symmetricity(struct statistics_mode_params *p, size_t m, void *b_val)
{
  size_t i, j, bi=0, topi, errdiff, prevj=0, size=p->data->size;

  /* Set the basic constants. */
  topi = 2*m>size-1 ? size-1 : 2*m;
  errdiff = p->mirrordist * sqrt(m);

  /* Do the process. */
  switch(p->data->type)
    {
    case GAL_TYPE_UINT8:      MODE_SYM( uint8_t  );    break;
    case GAL_TYPE_INT8:       MODE_SYM( int8_t   );    break;
    case GAL_TYPE_UINT16:     MODE_SYM( uint16_t );    break;
    case GAL_TYPE_INT16:      MODE_SYM( int16_t  );    break;
    case GAL_TYPE_UINT32:     MODE_SYM( uint32_t );    break;
    case GAL_TYPE_INT32:      MODE_SYM( int32_t  );    break;
    case GAL_TYPE_UINT64:     MODE_SYM( uint64_t );    break;
    case GAL_TYPE_INT64:      MODE_SYM( int64_t  );    break;
    case GAL_TYPE_FLOAT32:    MODE_SYM( float    );    break;
    case GAL_TYPE_FLOAT64:    MODE_SYM( double   );    break;
    default:
      error(EXIT_FAILURE, 0, "%s: type code %d not recognized",
            __func__, p->data->type);
    }

  /* Control shouldn't reach here! */
  error(EXIT_FAILURE, 0, "%s: a bug! please contact us at %s so we can "
        "address the problem. Control must not have reached the end of this "
        "function", __func__, PACKAGE_BUGREPORT);
  return NAN;
}





/* Return the mode and related parameters in a float64 `gal_data_t' with
   the following elements in its array, the array:

      array[0]: mode
      array[1]: mode quantile.
      array[2]: symmetricity.
      array[3]: value at the end of symmetricity.

  The inputs are:

    - `input' is the input dataset, it doesn't have to be sorted and can
      have blank values.

    - `mirrordist' is the maximum distance after the mirror point to check
      as a multiple of sigma.

    - `inplace' is either 0 or 1. If it is 1 and the input array has blank
      values and is not sorted, then the removal of blank values and
      sorting will occur in-place (input will be modified): all blank
      elements in the input array will be removed and it will be sorted. */
gal_data_t *
gal_statistics_mode(gal_data_t *input, float mirrordist, int inplace)
{
  double *oa;
  size_t modeindex;
  size_t dsize=4, mdsize=1;
  struct statistics_mode_params p;
  int type=gal_tile_block(input)->type;
  gal_data_t *tmptype=gal_data_alloc(NULL, type, 1, &mdsize, NULL, 1, -1,
                                     NULL, NULL, NULL);
  gal_data_t *b_val=gal_data_alloc(NULL, type, 1, &mdsize, NULL, 1, -1,
                                   NULL, NULL, NULL);
  gal_data_t *out=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &dsize,
                                 NULL, 1, -1, NULL, NULL, NULL);


  /* A small sanity check. */
  if(mirrordist<=0)
    error(EXIT_FAILURE, 0, "%s: %f not acceptable as a value to "
          "`mirrordist'. Only positive values can be given to it",
          __func__, mirrordist);


  /* Make sure the input doesn't have blank values and is sorted.  */
  p.data=gal_statistics_no_blank_sorted(input, inplace);


  /* It can happen that the whole array is blank. In such cases,
     `p.data->size==0', so set all output elements to NaN and return. */
  oa=out->array;
  if(p.data->size==0) { oa[0]=oa[1]=oa[2]=oa[3]=NAN; return out; }


  /* Basic constants. */
  p.tolerance    = 0.01;
  p.mirrordist   = mirrordist;
  p.numcheck     = p.data->size/2;


  /* Fill in the interval: Checking every single element is over-kill, so
     if the dataset is large enough, we'll set an interval to only check
     elements at an interval (so only 1000 elements are checked). */
  p.interval = p.numcheck>1000 ? p.numcheck/1000 : 1;


  /* Set the lower and higher acceptable indexes for the mode based on
     quantiles. */
  p.lowi  = gal_statistics_quantile_index(p.data->size, MODE_MIN_Q);
  p.highi = gal_statistics_quantile_index(p.data->size, MODE_MAX_Q);


  /* Having set the low and higher interval values, we will set the first
     middle point and also the maximum distance on that point. This is
     necessary to start the iteration. */
  p.midi = ( ( (float)p.highi + MODE_GOLDEN_RATIO * (float)p.lowi )
             / ( 1 + MODE_GOLDEN_RATIO ) );
  p.midd = mode_mirror_max_index_diff(&p, p.midi);


  /* Do the golden-section search iteration, read the mode value from the
     input array and save it in the `tmptype' data structure that has the
     same type as the input. */
  modeindex = mode_golden_section(&p);
  memcpy( tmptype->array,
          gal_data_ptr_increment(p.data->array, modeindex, p.data->type),
          gal_type_sizeof(p.data->type) );


  /* Convert the mode (which is in the same type as the input at this
     stage) to float64. */
  tmptype=gal_data_copy_to_new_type_free(tmptype, GAL_TYPE_FLOAT64);


  /* Put the first three values into the output structure. */
  oa[0] = *((double *)(tmptype->array));
  oa[1] = ((double)modeindex) / ((double)(p.data->size-1));
  oa[2] = mode_symmetricity(&p, modeindex, b_val->array);


  /* If the symmetricity is good, put it in the output, otherwise set all
     output values to NaN. */
  if(oa[2]>GAL_STATISTICS_MODE_GOOD_SYM)
    {
      b_val=gal_data_copy_to_new_type_free(b_val, GAL_TYPE_FLOAT64);
      oa[3] = *((double *)(b_val->array));
    }
  else oa[0]=oa[1]=oa[2]=oa[3]=NAN;


  /* For a check:
  printf("mode: %g\nquantile: %g\nsymmetricity: %g\nsym value: %g\n",
         oa[0], oa[1], oa[2], oa[3]);
  */

  /* Clean up (if necessary), then return the output */
  if(p.data!=input) gal_data_free(p.data);
  gal_data_free(tmptype);
  gal_data_free(b_val);
  return out;
}





/* Make the mirror array. */
#define STATS_MKMIRROR(IT) {                                            \
    IT *a=noblank_sorted->array, *m=mirror->array;                      \
    IT zf=a[index];                                                     \
    *mirror_val=zf;                                                     \
    for(i=0;i<=index;++i) m[i]       = a[i];                            \
    for(i=1;i<=index;++i) m[index+i] = 2 * zf - m[index - i];           \
  }
static gal_data_t *
statistics_make_mirror(gal_data_t *noblank_sorted, size_t index,
                       double *mirror_val)
{
  size_t i, dsize = 2*index+1;
  gal_data_t *mirror=gal_data_alloc(NULL, noblank_sorted->type, 1, &dsize,
                                    NULL, 1, -1, NULL, NULL, NULL);

  /* Make sure the index is less than or equal to the number of
     elements. */
  if( index >= noblank_sorted->size )
    error(EXIT_FAILURE, 0, "%s: the index value must be less than or equal "
          "to the number of elements in the input, but it isn't: index: "
          "%zu, size of input: %zu", __func__, index, noblank_sorted->size);

  /* Fill in the mirror array. */
  switch(noblank_sorted->type)
    {
    case GAL_TYPE_UINT8:     STATS_MKMIRROR( uint8_t  );     break;
    case GAL_TYPE_INT8:      STATS_MKMIRROR( int8_t   );     break;
    case GAL_TYPE_UINT16:    STATS_MKMIRROR( uint16_t );     break;
    case GAL_TYPE_INT16:     STATS_MKMIRROR( int16_t  );     break;
    case GAL_TYPE_UINT32:    STATS_MKMIRROR( uint32_t );     break;
    case GAL_TYPE_INT32:     STATS_MKMIRROR( int32_t  );     break;
    case GAL_TYPE_UINT64:    STATS_MKMIRROR( uint64_t );     break;
    case GAL_TYPE_INT64:     STATS_MKMIRROR( int64_t  );     break;
    case GAL_TYPE_FLOAT32:   STATS_MKMIRROR( float    );     break;
    case GAL_TYPE_FLOAT64:   STATS_MKMIRROR( double   );     break;
    }

  /* Return the mirrored distribution. */
  return mirror;
}





/* Make a mirrored histogram and cumulative frequency plot with the mirror
   distribution of the input with a value at `value'.

   The output is a linked list of data structures: the first is the bins
   with one bin at the mirror point, the second is the histogram with a
   maximum of one and the third is the cumulative frequency plot. */
gal_data_t *
gal_statistics_mode_mirror_plots(gal_data_t *input, gal_data_t *value,
                                 size_t numbins, int inplace,
                                 double *mirror_val)
{
  gal_data_t *mirror, *bins, *hist, *cfp;
  gal_data_t *nbs=gal_statistics_no_blank_sorted(input, inplace);
  size_t ind=gal_statistics_quantile_function_index(nbs, value, inplace);


  /* If the given mirror was outside the range of the input, then index
     will be 0 (below the range) or -1 (above the range), in that case, we
     should return NULL. */
  if(ind==-1 || ind==0)
    return NULL;


  /* Make the mirror array. */
  mirror=statistics_make_mirror(nbs, ind, mirror_val);


  /* Set the bins for histogram and cdf. */
  bins=gal_statistics_regular_bins(mirror, NULL, numbins, *mirror_val);


  /* Make the histogram: set it's maximum value to 1 for a nice comparison
     with the CDF. */
  hist=gal_statistics_histogram(mirror, bins, 0, 1);


  /* Make the cumulative frequency plot. */
  cfp=gal_statistics_cfp(mirror, bins, 1);


  /* Set the pointers to make a table and return. */
  bins->next=hist;
  hist->next=cfp;
  return bins;
}



















/****************************************************************
 ********                      Sort                       *******
 ****************************************************************/
/* Check if the given dataset is sorted. Output values are:

     - 0: Dataset is not sorted.
     - 1: Dataset is sorted and increasing or equal.
     - 2: dataset is sorted and decreasing.                  */

#define IS_SORTED(IT) {                                                 \
    IT *aa=input->array, *a=input->array, *af=a+input->size-1;          \
    if(a[1]>=a[0]) do if( *(a+1) < *a ) break; while(++a<af);           \
    else           do if( *(a+1) > *a ) break; while(++a<af);           \
    return ( a==af                                                      \
             ? ( aa[1]>=aa[0]                                           \
                 ? GAL_STATISTICS_SORTED_INCREASING                     \
                 : GAL_STATISTICS_SORTED_DECREASING )                   \
             : GAL_STATISTICS_SORTED_NOT );                             \
  }

int
gal_statistics_is_sorted(gal_data_t *input)
{
  /* A one-element dataset can be considered, sorted, so we'll just return
     1 (for sorted and increasing). */
  if(input->size==1) return GAL_STATISTICS_SORTED_INCREASING;

  /* Do the check. */
  switch(input->type)
    {
    case GAL_TYPE_UINT8:     IS_SORTED( uint8_t  );    break;
    case GAL_TYPE_INT8:      IS_SORTED( int8_t   );    break;
    case GAL_TYPE_UINT16:    IS_SORTED( uint16_t );    break;
    case GAL_TYPE_INT16:     IS_SORTED( int16_t  );    break;
    case GAL_TYPE_UINT32:    IS_SORTED( uint32_t );    break;
    case GAL_TYPE_INT32:     IS_SORTED( int32_t  );    break;
    case GAL_TYPE_UINT64:    IS_SORTED( uint64_t );    break;
    case GAL_TYPE_INT64:     IS_SORTED( int64_t  );    break;
    case GAL_TYPE_FLOAT32:   IS_SORTED( float    );    break;
    case GAL_TYPE_FLOAT64:   IS_SORTED( double   );    break;
    default:
      error(EXIT_FAILURE, 0, "%s: type code %d not recognized",
            __func__, input->type);
    }

  /* Control shouldn't reach this point. */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s so we can fix the "
        "problem. Control must not have reached the end of this function",
        __func__, PACKAGE_BUGREPORT);
  return -1;
}





/* This function is ignorant to blank values, if you want to make sure
   there is no blank values, you can call `gal_blank_remove' first. */
#define STATISTICS_SORT(QSORT_F) {                                      \
    qsort(input->array, input->size, gal_type_sizeof(input->type), QSORT_F); \
  }
void
gal_statistics_sort_increasing(gal_data_t *input)
{
  switch(input->type)
    {
    case GAL_TYPE_UINT8:
      STATISTICS_SORT(gal_qsort_uint8_increasing);    break;
    case GAL_TYPE_INT8:
      STATISTICS_SORT(gal_qsort_int8_increasing);     break;
    case GAL_TYPE_UINT16:
      STATISTICS_SORT(gal_qsort_uint16_increasing);   break;
    case GAL_TYPE_INT16:
      STATISTICS_SORT(gal_qsort_int16_increasing);    break;
    case GAL_TYPE_UINT32:
      STATISTICS_SORT(gal_qsort_uint32_increasing);   break;
    case GAL_TYPE_INT32:
      STATISTICS_SORT(gal_qsort_int32_increasing);    break;
    case GAL_TYPE_UINT64:
      STATISTICS_SORT(gal_qsort_uint64_increasing);   break;
    case GAL_TYPE_INT64:
      STATISTICS_SORT(gal_qsort_int64_increasing);    break;
    case GAL_TYPE_FLOAT32:
      STATISTICS_SORT(gal_qsort_float32_increasing);  break;
    case GAL_TYPE_FLOAT64:
      STATISTICS_SORT(gal_qsort_float64_increasing);  break;
    default:
      error(EXIT_FAILURE, 0, "%s: type code %d not recognized",
            __func__, input->type);
    }
}





/* See explanations above `gal_statistics_sort_increasing'. */
void
gal_statistics_sort_decreasing(gal_data_t *input)
{
  switch(input->type)
    {
    case GAL_TYPE_UINT8:
      STATISTICS_SORT(gal_qsort_uint8_decreasing);    break;
    case GAL_TYPE_INT8:
      STATISTICS_SORT(gal_qsort_int8_decreasing);     break;
    case GAL_TYPE_UINT16:
      STATISTICS_SORT(gal_qsort_uint16_decreasing);   break;
    case GAL_TYPE_INT16:
      STATISTICS_SORT(gal_qsort_int16_decreasing);    break;
    case GAL_TYPE_UINT32:
      STATISTICS_SORT(gal_qsort_uint32_decreasing);   break;
    case GAL_TYPE_INT32:
      STATISTICS_SORT(gal_qsort_int32_decreasing);    break;
    case GAL_TYPE_UINT64:
      STATISTICS_SORT(gal_qsort_uint64_decreasing);   break;
    case GAL_TYPE_INT64:
      STATISTICS_SORT(gal_qsort_int64_decreasing);    break;
    case GAL_TYPE_FLOAT32:
      STATISTICS_SORT(gal_qsort_float32_decreasing);  break;
    case GAL_TYPE_FLOAT64:
      STATISTICS_SORT(gal_qsort_float64_decreasing);  break;
    default:
      error(EXIT_FAILURE, 0, "%s: type code %d not recognized",
            __func__, input->type);
    }
}





/* Return a dataset that has doesn't have blank values and is sorted. If
   the `inplace' value is set to 1, then the input array will be modified,
   otherwise, a new array will be allocated with the desired properties. So
   if it is already sorted and has blank values, the `inplace' variable is
   irrelevant.

   This function can also work on tiles, in that case, `inplace' is
   useless, because a tile doesn't own its dataset and the dataset is not
   contiguous.
*/
gal_data_t *
gal_statistics_no_blank_sorted(gal_data_t *input, int inplace)
{
  int sortstatus;
  gal_data_t *contig, *noblank, *sorted;


  /* If this is a tile, then first we have to copy it into a contiguous
     piece of memory. After this step, we will only be dealing with
     `contig' (for a contiguous patch of memory). */
  if(input->block)
    {
      /* Copy the input into a contiguous patch of memory. */
      contig=gal_data_copy(input);

      /* When the data was a tile, we have already copied the array into a
         separate allocated space. So to avoid any further copying, we will
         just set the `inplace' variable to 1. */
      inplace=1;
    }
  else contig=input;


  /* Make sure there is no blanks in the array that will be used. After
     this step, we won't be dealing with `input' any more, but with
     `noblank'. */
  if( gal_blank_present(contig, inplace) )
    {
      /* See if we should allocate a new dataset to remove blanks or if we
         can use the actual contiguous patch of memory. */
      noblank = inplace ? contig : gal_data_copy(contig);
      gal_blank_remove(noblank);

      /* If we are working in place, then mark that there are no blank
         pixels. */
      if(inplace)
        {
          noblank->flag |= GAL_DATA_FLAG_BLANK_CH;
          noblank->flag &= ~GAL_DATA_FLAG_HASBLANK;
        }
    }
  else noblank=contig;


  /* Make sure the array is sorted. After this step, we won't be dealing
     with `noblank' any more but with `sorted'. */
  sortstatus=gal_statistics_is_sorted(noblank);
  if( sortstatus )
    {
      sorted=noblank;
      sorted->status=sortstatus;
    }
  else
    {
      if(inplace) sorted=noblank;
      else
        {
          if(noblank!=input)    /* no-blank has already been allocated. */
            sorted=noblank;
          else
            sorted=gal_data_copy(noblank);
        }
      gal_statistics_sort_increasing(sorted);
      sorted->status=GAL_STATISTICS_SORTED_INCREASING;
    }


  /* Return final array. */
  return sorted;
}




















/****************************************************************
 ********     Histogram and Cumulative Frequency Plot     *******
 ****************************************************************/
/* Generate an array of regularly spaced elements.

   Input arguments:

     * The `input' set you want to apply the bins to. This is only
       necessary if the range argument is not complete, see below. If
       `range' has all the necessary information, you can pass a NULL
       pointer for `input'.

     * The `inrange' data structure keeps the desired range along each
       dimension of the input data structure, it has to be in float32
       type. Note that if

         - If you want the full range of the dataset (in any dimensions,
           then just set `range' to NULL and the range will be specified
           from the minimum and maximum value of the dataset.

         - If there is one element for each dimension in range, then it is
           viewed as a quantile (Q), and the range will be: `Q to 1-Q'.

         - If there are two elements for each dimension in range, then they
           are assumed to be your desired minimum and maximum values. When
           either of the two are NaN, the minimum and maximum will be
           calculated for it.

     * The number of bins: must be larger than 0.

     * `onebinstart' A desired value for onebinstart. Note that with this
        option, the bins won't start and end exactly on the given range
        values, it will be slightly shifted to accommodate this
        request.

  The output is a 1D array (column) of type double, it has to be double to
  account for small differences on the bin edges.
*/
gal_data_t *
gal_statistics_regular_bins(gal_data_t *input, gal_data_t *inrange,
                            size_t numbins, double onebinstart)
{
  size_t i;
  gal_data_t *bins, *tmp, *range;
  double *b, *ra, min=NAN, max=NAN, hbw, diff, binwidth;


  /* Some sanity checks. */
  if(numbins==0)
    error(EXIT_FAILURE, 0, "%s: `numbins' cannot be given a value of 0",
          __func__);


  /* Set the minimum and maximum values. */
  if(inrange && inrange->size)
    {
      /* Make sure we are dealing with a double type range. */
      if(inrange->type==GAL_TYPE_FLOAT64)
        range=inrange;
      else
        range=gal_data_copy_to_new_type(inrange, GAL_TYPE_FLOAT64);

      /* Set the minimum and maximum of the bins. */
      ra=range->array;
      if( (range->size)%2 )
        error(EXIT_FAILURE, 0, "%s: quantile ranges are not implemented yet",
              __func__);
      else
        {
          /* If the minimum isn't set (is blank), find it. */
          if( isnan(ra[0]) )
            {
              tmp=gal_data_copy_to_new_type_free(gal_statistics_minimum(input),
                                                 GAL_TYPE_FLOAT64);
              min=*((double *)(tmp->array));
              gal_data_free(tmp);
            }
          else min=ra[0];

          /* For the maximum, when it isn't set, we'll add a very small
             value, so all points are included. */
          if( isnan(ra[1]) )
            {
              tmp=gal_data_copy_to_new_type_free(gal_statistics_maximum(input),
                                                 GAL_TYPE_FLOAT64);
              max=*((double *)(tmp->array))+1e-6;
              gal_data_free(tmp);
            }
          else max=ra[1];
        }

      /* Clean up: if `range' was allocated. */
      if(range!=inrange) gal_data_free(range);
    }
  /* No range was given, find the minimum and maximum. */
  else
    {
      tmp=gal_data_copy_to_new_type_free(gal_statistics_minimum(input),
                                         GAL_TYPE_FLOAT64);
      min=*((double *)(tmp->array));
      gal_data_free(tmp);
      tmp=gal_data_copy_to_new_type_free(gal_statistics_maximum(input),
                                         GAL_TYPE_FLOAT64);
      max=*((double *)(tmp->array)) + 1e-6;
      gal_data_free(tmp);
    }


  /* Allocate the space for the bins. */
  bins=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &numbins, NULL,
                      0, input->minmapsize, "bin_center", input->unit,
                      "Center value of each bin.");


  /* Set central bin values. */
  b=bins->array;
  hbw = ( binwidth=(max-min)/numbins )/2;
  for(i=0;i<numbins;++i) b[i] = min + i*binwidth + hbw;


  /* Go over all the bins and stop when the sign of the two sides
     of one bin are different. */
  if( !isnan(onebinstart) )
    {
      for(i=0;i<numbins-1;++i)
        if( (b[i]-hbw) < onebinstart && (b[i+1]-hbw) > onebinstart) break;
      if( i != numbins-1 )
        {
          diff = onebinstart - (b[i]-hbw);
          for(i=0;i<numbins;++i)
            b[i] += diff;
        }
    }

  /* For a check:
  printf("min: %g\n", min);
  printf("max: %g\n", max);
  printf("onebinstart: %.10f\n", onebinstart);
  printf("binwidth: %g\n", binwidth);
  for(i=0;i<numbins;++i)
    printf("%zu: %.4f\t(%f, %f)\n", i, b[i], b[i]-hbw, b[i]+hbw);
  */

  /* Set the status of the bins to regular and return. */
  bins->status=GAL_STATISTICS_BINS_REGULAR;
  return bins;
}





/* Make a histogram of all the elements in the given dataset with bin
   values that are defined in the `inbins' structure (see
   `gal_statistics_regular_bins'). `inbins' is not mandatory, if you pass a
   NULL pointer, the bins structure will be built within this function
   based on the `numbins' input. As a result, when you have already defined
   the bins, `numbins' is not used. */

#define HISTOGRAM_TYPESET(IT) {                                         \
    IT *a=input->array, *af=a+input->size;                              \
    do if( *a>=min && *a<max) ++h[ (size_t)( (*a-min)/binwidth ) ];     \
    while(++a<af);                                                      \
  }

gal_data_t *
gal_statistics_histogram(gal_data_t *input, gal_data_t *bins, int normalize,
                         int maxone)
{
  size_t *h;
  float *f, *ff;
  gal_data_t *hist;
  double *d, min, max, ref=NAN, binwidth;


  /* Check if the bins are regular or not. For irregular bins, we can
     either use the old implementation, or GSL's histogram
     functionality. */
  if(bins==NULL)
    error(EXIT_FAILURE, 0, "%s: `bins' is NULL", __func__);
  if(bins->status!=GAL_STATISTICS_BINS_REGULAR)
    error(EXIT_FAILURE, 0, "%s: the input bins are not regular. Currently "
          "it is only implemented for regular bins", __func__);


  /* Check if normalize and `maxone' are not called together. */
  if(normalize && maxone)
    error(EXIT_FAILURE, 0, "%s: only one of `normalize' and `maxone' may "
          "be given", __func__);


  /* Allocate the histogram (note that we are clearning it so all values
     are zero. */
  hist=gal_data_alloc(NULL, GAL_TYPE_SIZE_T, bins->ndim, bins->dsize,
                      NULL, 1, input->minmapsize, "hist_number", "counts",
                      "Number of data points within each bin.");


  /* Set the minimum and maximum range of the histogram from the bins. */
  d=bins->array;
  binwidth=d[1]-d[0];
  max = d[bins->size - 1] + binwidth/2;
  min = d[0]              - binwidth/2;


  /* Go through all the elements and find out which bin they belong to. */
  h=hist->array;
  switch(input->type)
    {
    case GAL_TYPE_UINT8:     HISTOGRAM_TYPESET(uint8_t);     break;
    case GAL_TYPE_INT8:      HISTOGRAM_TYPESET(int8_t);      break;
    case GAL_TYPE_UINT16:    HISTOGRAM_TYPESET(uint16_t);    break;
    case GAL_TYPE_INT16:     HISTOGRAM_TYPESET(int16_t);     break;
    case GAL_TYPE_UINT32:    HISTOGRAM_TYPESET(uint32_t);    break;
    case GAL_TYPE_INT32:     HISTOGRAM_TYPESET(int32_t);     break;
    case GAL_TYPE_UINT64:    HISTOGRAM_TYPESET(uint64_t);    break;
    case GAL_TYPE_INT64:     HISTOGRAM_TYPESET(int64_t);     break;
    case GAL_TYPE_FLOAT32:   HISTOGRAM_TYPESET(float);       break;
    case GAL_TYPE_FLOAT64:   HISTOGRAM_TYPESET(double);      break;
    default:
      error(EXIT_FAILURE, 0, "%s: type code %d not recognized",
            __func__, input->type);
    }


  /* For a check:
  {
    size_t i, *hh=hist->array;
    for(i=0;i<hist->size;++i) printf("%-10.4f%zu\n", f[i], hh[i]);
  }
  */


  /* Find the reference to correct the histogram if necessary. */
  if(normalize)
    {
      /* Set the reference. */
      ref=0.0f;
      hist=gal_data_copy_to_new_type_free(hist, GAL_TYPE_FLOAT32);
      ff=(f=hist->array)+hist->size; do ref += *f++;   while(f<ff);

      /* Correct the name, units and comments. */
      free(hist->name); free(hist->unit); free(hist->comment);
      gal_checkset_allocate_copy("hist_normalized", &hist->name);
      gal_checkset_allocate_copy("frac", &hist->unit);
      gal_checkset_allocate_copy("Normalized histogram value for this bin.",
                                 &hist->comment);
    }
  if(maxone)
    {
      /* Calculate the reference. */
      ref=-FLT_MAX;
      hist=gal_data_copy_to_new_type_free(hist, GAL_TYPE_FLOAT32);
      ff=(f=hist->array)+hist->size;
      do ref = *f>ref ? *f : ref; while(++f<ff);

      /* Correct the name, units and comments. */
      free(hist->name); free(hist->unit); free(hist->comment);
      gal_checkset_allocate_copy("hist_maxone", &hist->name);
      gal_checkset_allocate_copy("frac", &hist->unit);
      gal_checkset_allocate_copy("Fractional histogram value for this bin "
                                 "when maximum bin value is 1.0.",
                                 &hist->comment);
    }


  /* Correct the histogram if necessary. */
  if( !isnan(ref) )
    { ff=(f=hist->array)+hist->size; do *f++ /= ref;   while(f<ff); }


  /* Return the histogram. */
  return hist;
}





/* Make a cumulative frequency plot (CFP) of all the elements in the given
   dataset with bin values that are defined in the `bins' structure (see
   `gal_statistics_regular_bins').

   The CFP is built from the histogram: in each bin, the value is the sum
   of all previous bins in the histogram. Thus, if you have already
   calculated the histogram before calling this function, you can pass it
   onto this function as the data structure in `bins->next'. If
   `bin->next!=NULL', then it is assumed to be the histogram. If it is
   NULL, then the histogram will be calculated internally and freed after
   the job is finished.

   When a histogram is given and it is normalized, the CFP will also be
   normalized (even if the normalized flag is not set here): note that a
   normalized CFP's maximum value is 1. */
gal_data_t *
gal_statistics_cfp(gal_data_t *input, gal_data_t *bins, int normalize)
{
  double sum;
  float *f, *ff, *hf;
  gal_data_t *hist, *cfp;
  size_t *s, *sf, *hs, sums;


  /* Check if the bins are regular or not. For irregular bins, we can
     either use the old implementation, or GSL's histogram
     functionality. */
  if(bins->status!=GAL_STATISTICS_BINS_REGULAR)
    error(EXIT_FAILURE, 0, "%s: the input bins are not regular. Currently "
          "it is only implemented for regular bins", __func__);


  /* Prepare the histogram. */
  hist = ( bins->next
           ? bins->next
           : gal_statistics_histogram(input, bins, 0, 0) );


  /* If the histogram has float32 type it was given by the user and is
     either normalized or its maximum was set to 1. We can only use it if
     it was normalized. If it isn't normalized, then we must ignore it and
     build the histogram here.*/
  if(hist->type==GAL_TYPE_FLOAT32)
    {
      sum=0.0f;
      ff=(f=hist->array)+hist->size; do sum += *f++;   while(f<ff);
      if(sum!=1.0f)
        hist=gal_statistics_histogram(input, bins, 0, 0);
    }


  /* Allocate the cumulative frequency plot's necessary space. */
  cfp=gal_data_alloc( NULL, hist->type, bins->ndim, bins->dsize,
                      NULL, 1, input->minmapsize,
                      ( hist->type==GAL_TYPE_FLOAT32
                        ? "cfp_normalized" : "cfp_number" ),
                      ( hist->type==GAL_TYPE_FLOAT32
                        ? "frac" : "count" ),
                      ( hist->type==GAL_TYPE_FLOAT32
                        ? "Fraction of data elements from the start to this "
                        "bin (inclusive)."
                        : "Number of data elements from the start to this "
                        "bin (inclusive).") );


  /* Fill in the cumulative frequency plot. */
  switch(hist->type)
    {
    case GAL_TYPE_SIZE_T:
      sums=0; hs=hist->array; sf=(s=cfp->array)+cfp->size;
      do sums = (*s += *hs++ + sums); while(++s<sf);
      break;

    case GAL_TYPE_FLOAT32:
      sum=0.0f; hf=hist->array; ff=(f=cfp->array)+cfp->size;
      do sum = (*f += *hf++ + sum);  while(++f<ff);
      break;

    default:
      error(EXIT_FAILURE, 0, "%s: type code %d not recognized",
            __func__, cfp->type);
    }


  /* Normalize the CFP if the user asked for it and it wasn't normalized
     until now. */
  if(normalize && cfp->type==GAL_TYPE_SIZE_T)
    {
      /* Find the sum, then divide the plot by it. Note that the sum must
         come from the histogram, not the CFP!*/
      sums=0;
      cfp=gal_data_copy_to_new_type_free(cfp, GAL_TYPE_FLOAT32);
      sf=(s=hist->array)+hist->size; do sums += *s++;   while(s<sf);
      ff=(f=cfp->array)+cfp->size;   do *f++ /= sums;   while(f<ff);

      /* Correct the name, units and comments. */
      free(cfp->name); free(cfp->unit); free(cfp->comment);
      gal_checkset_allocate_copy("cfp_normalized", &cfp->name);
      gal_checkset_allocate_copy("frac", &cfp->unit);
      gal_checkset_allocate_copy("Fraction of data elements from the start "
                                 "to this bin (inclusive).", &cfp->comment);
    }

  /* If the histogram was allocated here, free it. */
  if(hist!=bins->next) gal_data_free(hist);
  return cfp;
}




















/****************************************************************
 *****************         Outliers          ********************
 ****************************************************************/
/* Sigma-cilp a given distribution:

   Inputs:

     - `multip': multiple of the standard deviation,

     - `param' must be positive and determines the type of clipping:

         - param<1.0: interpretted as a tolerance level to stop clipping.

         - param>=1.0 and an integer: a specific number of times to do the
           clippping.

   Output elements (type FLOAT32):

     - 0: Number of points used.
     - 1: Median.
     - 2: Mean.
     - 3: Standard deviation.

  The way this function works is very simple: first it will sort the input
  (if it isn't sorted). Afterwards, it will recursively change the starting
  point of the array and its size, calcluating the basic statistics in each
  round to define the new starting point and size.
*/
#define SIGCLIP(IT) {                                                   \
    IT *a  = nbs->array, *af = a  + nbs->size;                          \
    IT *bf = nbs->array, *b  = bf + nbs->size - 1;                      \
                                                                        \
    /* Remove all out-of-range elements from the start of the array. */ \
    if(sortstatus==GAL_STATISTICS_SORTED_INCREASING)                    \
      do if( *a > (*med - (multip * *std)) )                            \
           { start=a; break; }                                          \
      while(++a<af);                                                    \
    else                                                                \
      do if( *a < (*med + (multip * *std)) )                            \
           { start=a; break; }                                          \
      while(++a<af);                                                    \
                                                                        \
    /* Remove all out-of-range elements from the end of the array. */   \
    if(sortstatus==GAL_STATISTICS_SORTED_INCREASING)                    \
      do if( *b < (*med + (multip * *std)) )                            \
           { size=b-a+1; break; }                                       \
      while(--b>=bf);                                                   \
    else                                                                \
      do if( *b > (*med - (multip * *std)) )                            \
           { size=b-a+1; break; }                                       \
      while(--b>=bf);                                                   \
  }

gal_data_t *
gal_statistics_sigma_clip(gal_data_t *input, float multip, float param,
                          int inplace, int quiet)
{
  void *start, *nbs_array;
  double *med, *mean, *std;
  uint8_t bytolerance = param>=1.0f ? 0 : 1;
  double oldmed=NAN, oldmean=NAN, oldstd=NAN;
  size_t num=0, one=1, four=4, size, oldsize;
  gal_data_t *median_i, *median_d, *out, *meanstd;
  int sortstatus, type=gal_tile_block(input)->type;
  gal_data_t *nbs=gal_statistics_no_blank_sorted(input, inplace);
  size_t maxnum = param>=1.0f ? param : GAL_STATISTICS_SIG_CLIP_MAX_CONVERGE;


  /* Some sanity checks. */
  if( multip<=0 )
    error(EXIT_FAILURE, 0, "%s: `multip', must be greater than zero. The "
          "given value was %g", __func__, multip);
  if( param<=0 )
    error(EXIT_FAILURE, 0, "%s: `param', must be greater than zero. The "
          "given value was %g", __func__, param);
  if( param >= 1.0f && ceil(param) != param )
    error(EXIT_FAILURE, 0, "%s: when `param' is larger than 1.0, it is "
          "interpretted as an absolute number of clips. So it must be an "
          "integer. However, your given value %g", __func__, param);


  /* Allocate the necessary spaces. */
  out=gal_data_alloc(NULL, GAL_TYPE_FLOAT32, 1, &four, NULL, 0,
                     input->minmapsize, NULL, NULL, NULL);
  median_i=gal_data_alloc(NULL, type, 1, &one, NULL, 0, input->minmapsize,
                          NULL, NULL, NULL);


  /* Print the comments. */
  if(!quiet)
    printf("%-8s %-10s %-15s %-15s %-15s\n",
           "round", "number", "median", "mean", "STD");


  /* Do the clipping, but first initialize the values that will be changed
     during the clipping: the start of the array and the array's size. */
  size=nbs->size;
  sortstatus=nbs->status;
  nbs_array=start=nbs->array;
  while(num<maxnum)
    {
      /* Find the median. */
      statistics_median_in_sorted_no_blank(nbs, median_i->array);
      median_d=gal_data_copy_to_new_type(median_i, GAL_TYPE_FLOAT64);

      /* Find the average and Standard deviation, note that both `start'
         and `size' will be different in the next round. */
      nbs->array = start;
      nbs->size = oldsize = size;
      meanstd=gal_statistics_mean_std(nbs);

      /* Put the three final values in usable (with a type) pointers. */
      med = median_d->array;
      mean = meanstd->array;
      std  = &((double *)(meanstd->array))[1];

      /* If the user wanted to view the steps, show it to them. */
      if(!quiet)
        printf("%-8zu %-10zu %-15g %-15g %-15g\n",
               num+1, size, *med, *mean, *std);

      /* If we are to work by tolerance, then check if we should jump out
         of the loop. Normally, `oldstd' should be larger than std, because
         the possible outliers have been removed. If it is not, it means
         that we have clipped too much and must stop anyway, so we don't
         need an absolute value on the difference! */
      if( bytolerance && num>0 && ((oldstd - *std) / *std) < param )
        break;

      /* Clip all the elements outside of the desired range: since the
         array is sorted, this means to just change the starting pointer
         and size of the array. */
      switch(type)
        {
        case GAL_TYPE_UINT8:     SIGCLIP( uint8_t  );   break;
        case GAL_TYPE_INT8:      SIGCLIP( int8_t   );   break;
        case GAL_TYPE_UINT16:    SIGCLIP( uint16_t );   break;
        case GAL_TYPE_INT16:     SIGCLIP( int16_t  );   break;
        case GAL_TYPE_UINT32:    SIGCLIP( uint32_t );   break;
        case GAL_TYPE_INT32:     SIGCLIP( int32_t  );   break;
        case GAL_TYPE_UINT64:    SIGCLIP( uint64_t );   break;
        case GAL_TYPE_INT64:     SIGCLIP( int64_t  );   break;
        case GAL_TYPE_FLOAT32:   SIGCLIP( float    );   break;
        case GAL_TYPE_FLOAT64:   SIGCLIP( double   );   break;
        default:
          error(EXIT_FAILURE, 0, "%s: type code %d not recognized",
                __func__, type);
        }

      /* Set the values from this round in the old elements, so the next
         round can compare with, and return then if necessary. */
      oldmed =  *med;
      oldstd  = *std;
      oldmean = *mean;
      ++num;

      /* Clean up: */
      gal_data_free(meanstd);
      gal_data_free(median_d);
    }

  /* If we were in tolerance mode and `num' and `maxnum' are equal (the
     loop didn't stop by tolerance), so the outputs should be NaN. */
  out->status=num;
  if( bytolerance && num==maxnum )
    {
      ((float *)(out->array))[0] = NAN;
      ((float *)(out->array))[1] = NAN;
      ((float *)(out->array))[2] = NAN;
      ((float *)(out->array))[3] = NAN;
    }
  else
    {
      ((float *)(out->array))[0] = size;
      ((float *)(out->array))[1] = oldmed;
      ((float *)(out->array))[2] = oldmean;
      ((float *)(out->array))[3] = oldstd;
    }

  /* Clean up and return. */
  nbs->array=nbs_array;
  if(nbs!=input) gal_data_free(nbs);
  return out;
}




#if 0
/* Using the cumulative distribution function this funciton will
   remove outliers from a dataset. */
void
gal_statistics_remove_outliers_flat_cdf(float *sorted, size_t *outsize)
{
  printf("\n ... in gal_statistics_remove_outliers_flat_cdf ... \n");
  exit(1);
  int firstfound=0;
  size_t size=*outsize, i, maxind;
  float *slopes, minslope, maxslope;

  /* Find a slopes array, think of the cumulative frequency plot when
     you want to think about slopes. */
  errno=0; slopes=malloc(size*sizeof *slopes);
  if(slopes==NULL)
    error(EXIT_FAILURE, errno, "%s: %zu bytes for slopes",
          __func__, size*sizeof *slopes);

  /* Calcuate the slope of the CDF and put it in the slopes array. */
  for(i=1;i<size-1;++i)
    slopes[i]=2/(sorted[i+1]-sorted[i-1]);

  /* Find the position of the maximum slope, note that around the
     distribution mode, the difference between the values varies less,
     so two neighbouring elements have the closest values, hence the
     largest slope (when their difference is in the denominator). */
  gal_statistics_f_max_with_index(slopes+1, size-2, &maxslope, &maxind);

  /* Find the minimum slope from the second element (for the first the
     slope is not defined. NOTE; maxind is one smaller than it should
     be because the input array to find it began from the second
     element. */
  gal_statistics_float_second_min(slopes+1, maxind+1, &minslope);

  /* Find the second place where the slope falls below `minslope`
     after the maximum position. When found, add it with one to
     account for error. Note that incase there are no outliers by this
     definition, then the final size will be equal to size! */
  for(i=maxind+1;i<size-1;++i)
    if(slopes[i]<minslope)
      {
        if(firstfound)
          break;
        else
          firstfound=1;
      }
  *outsize=i+1;

  /*
  for(i=0;i<size;++i)
    printf("%zu\t%.3f\t%.3f\n", i, arr[i], slopes[i]);
  printf("\n\nPlace to cut off for outliers is: %zu\n\n", *outsize);
  */

  free(slopes);
}
#endif
