#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <ctype.h>


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                          RGB TO YCBCR CONVERSION                         */
/*                                                                          */
/*    (Copyright by Andres Bruhn, 11/2007 and Joachim Weickert, 4/2021)     */
/*                                                                          */
/*--------------------------------------------------------------------------*/


/* 
  RGB to YCbCr conversion with optional subsampling of the chroma channels
*/


/*--------------------------------------------------------------------------*/

void alloc_double_vector

     (double **vector,   /* vector */
      long   n1)         /* size */

/*
  allocates memory for a double format vector of size n1
*/

{
*vector = (double *) malloc (n1 * sizeof(double));

if (*vector == NULL)
   {
   printf("alloc_double_vector: not enough memory available\n");
   exit(1);
   }

return;

}  /* alloc_double_vector */

/*--------------------------------------------------------------------------*/

void alloc_double_cubix

     (double ****cubix,  /* cubix */
      long   n1,         /* size in direction 1 */
      long   n2,         /* size in direction 2 */
      long   n3)         /* size in direction 3 */

/* 
  allocates memory for a double format cubix of size n1 * n2 * n3 
*/

{
long i, j;  /* loop variables */

*cubix = (double ***) malloc (n1 * sizeof(double **));

if (*cubix == NULL)
   {
   printf("alloc_double_cubix: not enough memory available\n");
   exit(1);
   }

for (i=0; i<n1; i++)
    {
    (*cubix)[i] = (double **) malloc (n2 * sizeof(double *));
    if ((*cubix)[i] == NULL)
       {
       printf("alloc_double_cubix: not enough memory available\n");
       exit(1);
       }
    for (j=0; j<n2; j++)
        {
        (*cubix)[i][j] = (double *) malloc (n3 * sizeof(double));
        if ((*cubix)[i][j] == NULL)
           {
           printf("alloc_double_cubix: not enough memory available\n");
           exit(1);
           }
        }
    }

return;

}  /* alloc_double_cubix */

/*--------------------------------------------------------------------------*/

void free_double_vector

     (double  *vector,    /* vector */
      long    n1)         /* size */

/*
  frees memory for a double format vector of size n1
*/

{

free(vector);
return;

}  /* free_double_vector */

/*--------------------------------------------------------------------------*/

void free_double_cubix

     (double ***cubix,   /* cubix */
      long   n1,         /* size in direction 1 */
      long   n2,         /* size in direction 2 */
      long   n3)         /* size in direction 3 */

/* 
  frees memory for a double format cubix of size n1 * n2 * n3 
*/

{
long i, j;   /* loop variables */

for (i=0; i<n1; i++)
 for (j=0; j<n2; j++)
     free(cubix[i][j]);

for (i=0; i<n1; i++)
    free(cubix[i]);

free(cubix);

return;

}  /* free_double_cubix */

/*--------------------------------------------------------------------------*/

void read_string

     (char *v)         /* string to be read */

/*
  reads a string v
*/

{
if (fgets (v, 80, stdin) == NULL)
{
   printf("could not read string, aborting\n");
   exit(1);
}

if (v[strlen(v)-1] == '\n')
   v[strlen(v)-1] = 0;

return;

}  /* read_string */

/*--------------------------------------------------------------------------*/

void read_long

     (long *v)         /* value to be read */

/*
  reads a long value v
*/

{
char   row[80];    /* string for reading data */

if (fgets (row, 80, stdin) == NULL)
{
   printf("could not read string, aborting\n");
   exit(1);
}

if (row[strlen(row)-1] == '\n')
   row[strlen(row)-1] = 0;
sscanf(row, "%ld", &*v);

return;

}  /* read_long */

/*--------------------------------------------------------------------------*/

void skip_white_space_and_comments 

     (FILE *inimage)  /* input file */

/*
  skips over white space and comments while reading the file
*/

{

int   ch = 0;   /* holds a character */
char  row[80];  /* for reading data */

/* skip spaces */
while (((ch = fgetc(inimage)) != EOF) && isspace(ch));
  
/* skip comments */
if (ch == '#')
   {
   if (fgets(row, sizeof(row), inimage))
      skip_white_space_and_comments (inimage);
   else
      {
      printf("skip_white_space_and_comments: cannot read file\n");
      exit(1);
      }
   }
else
   fseek (inimage, -1, SEEK_CUR);

return;

} /* skip_white_space_and_comments */

/*--------------------------------------------------------------------------*/

void read_pgm_or_ppm_to_double

     (const char  *file_name,    /* name of image file */
      long        *nc,           /* number of colour channels */
      long        *nx,           /* image size in x direction, output */
      long        *ny,           /* image size in y direction, output */
      double      ****u)         /* image, output */

/*
  reads a greyscale image (pgm format P5) or a colour image (ppm format P6);
  allocates memory for the double format image u;
  adds boundary layers of size 1 such that
  - the relevant image pixels in x direction use the indices 1,...,nx
  - the relevant image pixels in y direction use the indices 1,...,ny
*/

{
char  row[80];      /* for reading data */
long  i, j, m;      /* image indices */
long  max_value;    /* maximum color value */
FILE  *inimage;     /* input file */

/* open file */
inimage = fopen (file_name, "rb");
if (inimage == NULL)
   {
   printf ("read_pgm_or_ppm_to_double: cannot open file '%s'\n", file_name);
   exit(1);
   }

/* read header */
if (fgets (row, 80, inimage) == NULL)
   {
   printf ("read_pgm_or_ppm_to_double: cannot read file\n");
   exit(1);
   }

/* image type: P5 or P6 */
if ((row[0] == 'P') && (row[1] == '5'))
   {
   /* P5: grey scale image */
   *nc = 1;
   }
else if ((row[0] == 'P') && (row[1] == '6'))
   {
   /* P6: colour image */
   *nc = 3;
   }
else
   {
   printf ("read_pgm_or_ppm_to_double: unknown image format\n");
   exit(1);
   }

/* read image size in x direction */
skip_white_space_and_comments (inimage);
if (!fscanf (inimage, "%ld", nx))
   {
   printf ("read_pgm_or_ppm_to_double: cannot read image size nx\n");
   exit(1);
   }

/* read image size in y direction */
skip_white_space_and_comments (inimage);
if (!fscanf (inimage, "%ld", ny))
   {
   printf ("read_pgm_or_ppm_to_double: cannot read image size ny\n");
   exit(1);
   }

/* read maximum grey value */
skip_white_space_and_comments (inimage);
if (!fscanf (inimage, "%ld", &max_value))
   {
   printf ("read_pgm_or_ppm_to_long: cannot read maximal value\n");
   exit(1);
   }
fgetc(inimage);

/* allocate memory */
alloc_double_cubix (u, (*nc), (*nx)+2, (*ny)+2);

/* read image data row by row */
for (j = 1; j <= (*ny); j++)
 for (i = 1; i <= (*nx); i++)
  for (m = 0; m < (*nc); m++)
      (*u)[m][i][j] = (double) getc(inimage);

/* close file */
fclose(inimage);

}  /* read_pgm_or_ppm_to_double */

/*--------------------------------------------------------------------------*/

void comment_line

     (char* comment,       /* comment string (output) */
      char* lineformat,    /* format string for comment line */
      ...)                 /* optional arguments */

/* 
  Adds a line to the comment string comment. The string line can contain 
  plain text and format characters that are compatible with sprintf.
  Example call: 
  print_comment_line(comment, "Text %lf %ld", double_var, long_var).
  If no line break is supplied at the end of the input string, it is 
  added automatically.
*/

{
char     line[80];
va_list  arguments;

/* get list of optional function arguments */
va_start (arguments, lineformat);

/* convert format string and arguments to plain text line string */
vsprintf (line, lineformat, arguments);

/* add line to total commentary string */
strncat (comment, line, 80);

/* add line break if input string does not end with one */
if (line[strlen(line)-1] != '\n')
   strcat (comment, "\n"); 

/* close argument list */
va_end (arguments);

return;

}  /* comment_line */

/*--------------------------------------------------------------------------*/

void write_double_to_pgm_or_ppm

     (double  ***u,         /* colour image, unchanged */
      long    nc,           /* number of channels */
      long    nx,           /* size in x direction */
      long    ny,           /* size in y direction */
      char    *file_name,   /* name of ppm file */
      char    *comments)    /* comment string (set 0 for no comments) */

/*
  writes a double format image into a pgm P5 (greyscale) or 
  ppm P6 (colour) file;
*/

{
FILE           *outimage;  /* output file */
long           i, j, m;    /* loop variables */
double         aux;        /* auxiliary variable */
unsigned char  byte;       /* for data conversion */

/* open file */
outimage = fopen (file_name, "wb");
if (NULL == outimage)
   {
   printf("Could not open file '%s' for writing, aborting\n", file_name);
   exit(1);
   }

/* write header */
if (nc == 1)
   fprintf (outimage, "P5\n");               /* greyscale format */
else if (nc == 3)
   fprintf (outimage, "P6\n");               /* colour format */
else
   {
   printf ("unsupported number of channels\n");
   exit (0);
   }
if (comments != 0)
   fputs (comments, outimage);               /* comments */
fprintf (outimage, "%ld %ld\n", nx, ny);     /* image size */
fprintf (outimage, "255\n");                 /* maximal value */

/* write image data */
for (j=1; j<=ny; j++)
 for (i=1; i<=nx; i++)
  for (m=0; m<=nc-1; m++)
     {
     aux = u[m][i][j] + 0.499999;    /* for correct rounding */
     if (aux < 0.0)
        byte = (unsigned char)(0.0);
     else if (aux > 255.0)
        byte = (unsigned char)(255.0);
     else
        byte = (unsigned char)(aux);
     fwrite (&byte, sizeof(unsigned char), 1, outimage);
     }

/* close file */
fclose (outimage);

return;

}  /* write_double_to_pgm_or_ppm */

/*--------------------------------------------------------------------------*/

void analyse_colour_double

     (double  ***u,        /* image, unchanged */
      long    nc,          /* number of channels */
      long    nx,          /* pixel number in x direction */
      long    ny,          /* pixel number in y direction */
      double  *min,        /* minimum, output */
      double  *max,        /* maximum, output */
      double  *mean,       /* mean, output */
      double  *std)        /* standard deviation, output */

/*
  computes minimum, maximum, mean and standard deviation of a
  vector-valued double format image u
*/

{
long    i, j, m;    /* loop variables */
double  help1;      /* auxiliary variable */
double  help2;      /* auxiliary variable */
double  *vmean;     /* mean in each channel */


/* ---- allocate memory ---- */

alloc_double_vector (&vmean, nc);


/* ---- compute min, max, vmean, mean ---- */

*min  = u[0][1][1];
*max  = u[0][1][1];
*mean = 0.0;

for (m=0; m<=nc-1; m++)
    {
    help1 = 0.0;
    for (i=1; i<=nx; i++)
     for (j=1; j<=ny; j++)
         {
         if (u[m][i][j] < *min) *min = u[m][i][j];
         if (u[m][i][j] > *max) *max = u[m][i][j];
         help1 = help1 + u[m][i][j];
         }
    vmean[m] = help1 / (nx * ny);
    *mean = *mean + vmean[m];
    }

*mean = *mean / nc;


/* ---- compute standard deviation ---- */

*std = 0.0;
for (m=0; m<=nc-1; m++)
 for (i=1; i<=nx; i++)
  for (j=1; j<=ny; j++)
     {
     help2 = u[m][i][j] - vmean[m];
     *std  = *std + help2 * help2;
     }
*std = sqrt (*std / (nc * nx * ny));


/* ---- free memory ---- */

free_double_vector (vmean, nc);

return;

}  /* analyse_colour_double */

/*--------------------------------------------------------------------------*/

void RGB_to_YCbCr

     (double  ***u_RGB,     /* RGB image, input */
      double  ***u_YCbCr,   /* YCbCr image, output */  
      long    nx,           /* pixel number in x-direction */
      long    ny)           /* pixel number in y-direction */      

/*
  converts RGB to YCbCr colour space
*/

{
long    i, j;        /* loop variables */

/* compute the YCbCr values */ 
for (i=1;i<=nx;i++)
  for (j=1;j<=ny;j++)
      {
      /*
        SUPPLEMENT CODE HERE
      */
      }

return;

} /* RGB_to_YCbCr */

/*--------------------------------------------------------------------------*/

void YCbCr_to_RGB

     (double  ***u_YCbCr,   /* YCbCr image, input */  
      double  ***u_RGB,     /* RGB image, output */      
      long    nx,           /* pixel number in x-direction */
      long    ny)           /* pixel number in y-direction */      

/*
  converts YCbCr to RGB colour space
*/

{
long    i, j;        /* loop variables */

/* computes the RGB values */ 
for (i=1;i<=nx;i++)
  for (j=1;j<=ny;j++)
      {
      u_RGB[0][i][j] = 1.0   *  u_YCbCr[0][i][j]
                     + 0.0   * (u_YCbCr[1][i][j] - 127.5)
                     + 1.402 * (u_YCbCr[2][i][j] - 127.5);

      u_RGB[1][i][j] = 1.0   *  u_YCbCr[0][i][j]
                     - 0.344 * (u_YCbCr[1][i][j] - 127.5)
                     - 0.714 * (u_YCbCr[2][i][j] - 127.5);

      u_RGB[2][i][j] = 1.0   *  u_YCbCr[0][i][j]
                     + 1.773 * (u_YCbCr[1][i][j] - 127.5)
                     + 0.0   * (u_YCbCr[2][i][j] - 127.5);
      }

return;

} /* YCbCr_to_RGB */


/*--------------------------------------------------------------------------*/

void subsample_channel

     (double  **c,          /* image channel, changed */           
      long    nx,           /* pixel number in x-direction */
      long    ny,           /* pixel number in y-direction */    
      long    S)            /* subsample factor */


/*
  reduce resolution by averaging blocks of SxS neighbouring pixels
*/

{
long    i, j;        /* loop variables */
long    k, l;        /* loop variables */
double  sum;         /* summation variable */

/* replace SxS block by block average */ 
for (i=1;i<=nx;i+=S)
 for (j=1;j<=ny;j+=S)
     {
     /* initialise sum */
     sum = 0.0;

     /* compute block average */
     for (k=0; k<S; k++)
      for (l=0; l<S; l++)
          sum = sum + c[i+k][j+l];
     sum = sum / (S*S);   

     /* set all block entries to average */
     for (k=0;k<S;k++)
      for (l=0;l<S;l++)
          c[i+k][j+l] = sum;
     }

return;

} /* subsample_channel */

/*--------------------------------------------------------------------------*/

int main ()

{
char    in[80];               /* for reading data */
char    out[80];              /* for reading data */
double  ***u_RGB;             /* RGB image */
double  ***u_YCbCr;           /* YCbCr image */
long    nx, ny;               /* image size in x, y direction */ 
long    nc;                   /* number of channels in the image */
long    S;                    /* subsampling factor */
double  max, min;             /* largest, smallest grey value */
double  mean;                 /* average grey value */
double  std;                  /* standard deviation */
char    comments[1600];       /* string for comments */

printf ("\n");
printf ("RGB TO YCBCR CONVERSION\n\n");
printf ("**************************************************\n\n");
printf ("    Copyright 2021 by Joachim Weickert            \n");
printf ("    and 2007 by Andres Bruhn                      \n");
printf ("    Dept. of Mathematics and Computer Science     \n");
printf ("    Saarland University, Saarbruecken, Germany    \n\n");
printf ("    All rights reserved. Unauthorized usage,      \n");
printf ("    copying, hiring, and selling prohibited.      \n\n");
printf ("    Send bug reports to                           \n");
printf ("    weickert@mia.uni-saarland.de                  \n\n");
printf ("**************************************************\n\n");


/* ---- read input image (ppm format P6) ---- */

printf ("input image (ppm):                ");
read_string (in);
read_pgm_or_ppm_to_double (in, &nc, &nx, &ny, &u_RGB);  /* allocates memory */


/* ---- read parameters ---- */

printf ("subsampling factor (integer):     ");
read_long (&S);

printf ("output image (ppm):               ");
read_string (out);
printf ("\n");


/* ---- analyse input image ---- */

analyse_colour_double (u_RGB, nc, nx, ny, &min, &max, &mean, &std);
printf ("input image:\n");
printf ("minimum:       %8.2lf \n", min);
printf ("maximum:       %8.2lf \n", max);
printf ("mean:          %8.2lf \n", mean);
printf ("standard dev.: %8.2lf \n\n", std);


/* ---- check if image can be downsampled by a factor of S ----*/

if ((nx % S != 0) || (ny % S != 0))
   {
   printf ("\n\n image size does not allow downsampling by factor %ld! \n\n",S);
   return (0);
   }


/* ---- allocate memory for YCbCr image ---- */

alloc_double_cubix (&u_YCbCr, nc, nx+2, ny+2);


/* ---- process image ---- */

RGB_to_YCbCr (u_RGB, u_YCbCr, nx, ny);
subsample_channel (u_YCbCr[1], nx, ny, S);
subsample_channel (u_YCbCr[2], nx, ny, S);
YCbCr_to_RGB (u_YCbCr, u_RGB, nx, ny);


/* ---- analyse filtered image ---- */

analyse_colour_double (u_RGB, nc, nx, ny, &min, &max, &mean, &std);
printf ("processed image:\n");
printf ("minimum:       %8.2lf \n", min);
printf ("maximum:       %8.2lf \n", max);
printf ("mean:          %8.2lf \n", mean);
printf ("standard dev.: %8.2lf \n\n", std);


/* ---- write output image (ppm format P6) ---- */

/* generate comment string */
comments[0]='\0';
comment_line (comments, "# RGB to YCbCr conversion\n");
comment_line (comments, "# chroma subsampling factor: %2ld\n", S);

/* write image */
write_double_to_pgm_or_ppm (u_RGB, nc, nx, ny, out, comments);
printf ("output image %s successfully written\n\n", out);


/* ---- free memory  ---- */

free_double_cubix (u_RGB,   nc, nx+2, ny+2);
free_double_cubix (u_YCbCr, nc, nx+2, ny+2);

return(0);
}
