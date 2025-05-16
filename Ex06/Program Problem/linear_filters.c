#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <ctype.h>

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*          GAUSSIAN-BASED HIGHPASS, LOWPASS AND BANDPASS FILTERS           */
/*                                                                          */
/*         (Copyright by Joachim Weickert and Pascal Peter, 5/2021)         */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/* 
  Gaussian-based highpass, lowpass and bandpass filters
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

void alloc_double_matrix

     (double ***matrix,  /* matrix */
      long   n1,         /* size in direction 1 */
      long   n2)         /* size in direction 2 */

/*
  allocates memory for a double format matrix of size n1 * n2 
*/

{
long i;    /* loop variable */

*matrix = (double **) malloc (n1 * sizeof(double *));

if (*matrix == NULL)
   {
   printf("alloc_double_matrix: not enough memory available\n");
   exit(1);
   }

for (i=0; i<n1; i++)
    {
    (*matrix)[i] = (double *) malloc (n2 * sizeof(double));
    if ((*matrix)[i] == NULL)
       {
       printf("alloc_double_matrix: not enough memory available\n");
       exit(1);
       }
    }

return;

}  /* alloc_double_matrix */

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

void free_double_matrix

     (double  **matrix,   /* matrix */
      long    n1,         /* size in direction 1 */
      long    n2)         /* size in direction 2 */

/*
  frees memory for a double format matrix of size n1 * n2
*/

{
long i;   /* loop variable */

for (i=0; i<n1; i++)
free(matrix[i]);

free(matrix);

return;

}  /* free_double_matrix */

/*--------------------------------------------------------------------------*/

void read_string

     (char *v)         /* string to be read */

/*
  reads a string v
*/

{
fgets (v, 80, stdin);

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

fgets (row, 80, stdin);
if (row[strlen(row)-1] == '\n')
   row[strlen(row)-1] = 0;
sscanf(row, "%ld", &*v);

return;

}  /* read_long */

/*--------------------------------------------------------------------------*/

void read_double

     (double *v)         /* value to be read */

/*
  reads a double value v
*/

{
char   row[80];    /* string for reading data */

fgets (row, 80, stdin);

if (row[strlen(row)-1] == '\n')
   row[strlen(row)-1] = 0;
sscanf(row, "%lf", &*v);

return;

}  /* read_double */

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

void read_pgm_to_double

     (const char  *file_name,    /* name of pgm file */
      long        *nx,           /* image size in x direction, output */
      long        *ny,           /* image size in y direction, output */
      double      ***u)          /* image, output */

/*
  reads a greyscale image that has been encoded in pgm format P5 to
  an image u in double format;
  allocates memory for the image u;
  adds boundary layers of size 1 such that
  - the relevant image pixels in x direction use the indices 1,...,nx
  - the relevant image pixels in y direction use the indices 1,...,ny
*/

{
char  row[80];      /* for reading data */
long  i, j;         /* image indices */
long  max_value;    /* maximum color value */
FILE  *inimage;     /* input file */

/* open file */
inimage = fopen (file_name, "rb");
if (inimage == NULL)
   {
   printf ("read_pgm_to_double: cannot open file '%s'\n", file_name);
   exit(1);
   }

/* read header */
if (fgets(row, 80, inimage) == NULL)
   {
   printf ("read_pgm_to_double: cannot read file\n");
   exit(1);
   }

/* image type: P5 */
if ((row[0] == 'P') && (row[1] == '5'))
   {
   /* P5: grey scale image */
   }
else
   {
   printf ("read_pgm_to_double: unknown image format\n");
   exit(1);
   }

/* read image size in x direction */
skip_white_space_and_comments (inimage);
if (!fscanf (inimage, "%ld", nx))
   {
   printf ("read_pgm_to_double: cannot read image size nx\n");
   exit(1);
   }

/* read image size in x direction */
skip_white_space_and_comments (inimage);
if (!fscanf (inimage, "%ld", ny))
   {
   printf ("read_pgm_to_double: cannot read image size ny\n");
   exit(1);
   }

/* read maximum grey value */
skip_white_space_and_comments (inimage);
if (!fscanf (inimage, "%ld", &max_value))
   {
   printf ("read_pgm_to_double: cannot read maximal value\n");
   exit(1);
   }
fgetc(inimage);

/* allocate memory */
alloc_double_matrix (u, (*nx)+2, (*ny)+2);

/* read image data row by row */
for (j=1; j<=(*ny); j++)
 for (i=1; i<=(*nx); i++)
     (*u)[i][j] = (double) getc(inimage);

/* close file */
fclose (inimage);

return;

}  /* read_pgm_to_double */

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

void write_double_to_pgm

     (double  **u,          /* image, unchanged */
      long    nx,           /* image size in x direction */
      long    ny,           /* image size in y direction */
      char    *file_name,   /* name of pgm file */
      char    *comments)    /* comment string (set 0 for no comments) */

/*
  writes a greyscale image in double format into a pgm P5 file
*/

{
FILE           *outimage;  /* output file */
long           i, j;       /* loop variables */
double         aux;        /* auxiliary variable */
unsigned char  byte;       /* for data conversion */

/* open file */
outimage = fopen (file_name, "wb");
if (NULL == outimage)
   {
   printf("could not open file '%s' for writing, aborting\n", file_name);
   exit(1);
   }

/* write header */
fprintf (outimage, "P5\n");                  /* format */
if (comments != 0)
   fputs (comments, outimage);               /* comments */
fprintf (outimage, "%ld %ld\n", nx, ny);     /* image size */
fprintf (outimage, "255\n");                 /* maximal value */

/* write image data */
for (j=1; j<=ny; j++)
 for (i=1; i<=nx; i++)
     {
     aux = u[i][j] + 0.499999;    /* for correct rounding */
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

}  /* write_double_to_pgm */

/*--------------------------------------------------------------------------*/

void analyse_grey_double

     (double  **u,         /* image, unchanged */
      long    nx,          /* pixel number in x direction */
      long    ny,          /* pixel number in y direction */
      double  *min,        /* minimum, output */
      double  *max,        /* maximum, output */
      double  *mean,       /* mean, output */
      double  *std)        /* standard deviation, output */

/*
  computes minimum, maximum, mean, and standard deviation of a greyscale
  image u in double format
*/

{
long    i, j;       /* loop variables */
double  help1;      /* auxiliary variable */
double  help2;      /* auxiliary variable */

/* compute maximum, minimum, and mean */
*min  = u[1][1];
*max  = u[1][1];
help1 = 0.0;
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     {
     if (u[i][j] < *min) *min = u[i][j];
     if (u[i][j] > *max) *max = u[i][j];
     help1 = help1 + u[i][j];
     }
*mean = help1 / (nx * ny);

/* compute standard deviation */
*std = 0.0;
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     {
     help2  = u[i][j] - *mean;
     *std = *std + help2 * help2;
     }
*std = sqrt(*std / (nx * ny));

return;

}  /* analyse_grey_double */

/*--------------------------------------------------------------------------*/

void gauss_conv 

    (double   sigma,     /* standard deviation of the Gaussian */
     long     btype,     /* type of boundary condition */
     double   prec,      /* cutoff at precision * sigma */
     long     nx,        /* image dimension in x direction */
     long     ny,        /* image dimension in y direction */
     double   hx,        /* pixel size in x direction */
     double   hy,        /* pixel size in y direction */
     double   **u)       /* input: original image ;  output: smoothed */


/*
  Gaussian convolution with a truncated and resampled Gaussian
*/


{
long    i, j, k, l, p;        /* loop variables */
long    length;               /* convolution vector: 0..length */
long    pmax;                 /* upper bound for p */
double  aux1, aux2;           /* time savers */
double  sum;                  /* for summing up */
double  *conv;                /* convolution vector */
double  *help;                /* row or column with dummy boundaries */


/* ----------------------- convolution in x direction -------------------- */

/* compute length of convolution vector */
length = (long)(prec * sigma / hx) + 1;

/* allocate memory for convolution vector */
alloc_double_vector (&conv, length+1);

/* compute entries of convolution vector */
aux1 = 1.0 / (sigma * sqrt(2.0 * 3.1415927));
aux2 = (hx * hx) / (2.0 * sigma * sigma);
for (i=0; i<=length; i++)
    conv[i] = aux1 * exp (- i * i * aux2);

/* normalisation */
sum = conv[0];
for (i=1; i<=length; i++)
    sum = sum + 2.0 * conv[i];
for (i=0; i<=length; i++)
    conv[i] = conv[i] / sum;

/* allocate memory for a row */
alloc_double_vector (&help, nx+length+length);

for (j=1; j<=ny; j++)
    {
    /* copy u in row vector */
    for (i=1; i<=nx; i++)
        help[i+length-1] = u[i][j];

    /* extend signal according to the boundary conditions */
    k = length;
    l = length + nx - 1;
    while (k > 0)
          {
          /* pmax = min (k, nx) */
          if (k < nx)
             pmax = k;
          else
             pmax = nx;
 
          /* extension on both sides */
          if (btype == 0) 
             /* reflecting b.c.: symmetric extension */
             for (p=1; p<=pmax; p++)
                 {
                 help[k-p] = help[k+p-1];
                 help[l+p] = help[l-p+1];
                 }
          else
             /* Dirichlet b.c.: antisymmetric extension */
             for (p=1; p<=pmax; p++)
                 {
                 help[k-p] = - help[k+p-1];
                 help[l+p] = - help[l-p+1];
                 }

          /* update k and l */
          k = k - nx;
          l = l + nx;
          }

    /* convolution step */
    for (i=length; i<=nx+length-1; i++)
        {
        /* compute convolution */
        sum = conv[0] * help[i];
        for (p=1; p<=length; p++)
            sum = sum + conv[p] * (help[i+p] + help[i-p]);
        /* write back */
        u[i-length+1][j] = sum;
        }
    } /* for j */

/* free memory */
free_double_vector (help, nx+length+length);
free_double_vector (conv, length + 1);


/* ----------------------- convolution in y direction -------------------- */

/* compute length of convolution vector */
length = (long)(prec * sigma / hy) + 1;

/* allocate memory for convolution vector */
alloc_double_vector (&conv, length + 1);

/* compute entries of convolution vector */
aux1 = 1.0 / (sigma * sqrt(2.0 * 3.1415927));
aux2 = (hy * hy) / (2.0 * sigma * sigma);
for (j=0; j<=length; j++)
    conv[j] = aux1 * exp (- j * j * aux2);

/* normalisation */
sum = conv[0];
for (j=1; j<=length; j++)
    sum = sum + 2.0 * conv[j];
for (j=0; j<=length; j++)
    conv[j] = conv[j] / sum;

/* allocate memory for a row */
alloc_double_vector (&help, ny+length+length);

for (i=1; i<=nx; i++)
    {
    /* copy u in column vector */
    for (j=1; j<=ny; j++)
        help[j+length-1] = u[i][j];

    /* extend signal according to the boundary conditions */
    k = length;
    l = length + ny - 1;
    while (k > 0)
          {
          /* pmax = min (k, ny) */
          if (k < ny)
             pmax = k;
          else
             pmax = ny;

          /* extension on both sides */
          if (btype == 0)
             /* reflecting b.c.: symmetric extension */
             for (p=1; p<=pmax; p++)
                 {
                 help[k-p] = help[k+p-1];
                 help[l+p] = help[l-p+1];
                 }
          else
             /* Dirichlet b.c.: antisymmetric extension */
             for (p=1; p<=pmax; p++)
                 {
                 help[k-p] = - help[k+p-1];
                 help[l+p] = - help[l-p+1];
                 }

          /* update k and l */
          k = k - ny;
          l = l + ny;
          }

    /* convolution step */
    for (j=length; j<=ny+length-1; j++)
        {
        /* compute convolution */
        sum = conv[0] * help[j];
        for (p=1; p<=length; p++)
            sum = sum + conv[p] * (help[j+p] + help[j-p]);
        /* write back */
        u[i][j-length+1] = sum;
        }
    } /* for i */

/* free memory */
free_double_vector (help, ny+length+length);
free_double_vector (conv, length+1);

return;

} /* gauss_conv */

/*--------------------------------------------------------------------------*/

void rescale 

     (double  **u,        /* input image */
      long    nx,         /* size in x direction */
      long    ny,         /* size in y direction */
      double  a,          /* smallest transformed grey level */
      double  b)          /* largest transformed grey level */

/* 
  affine rescaling of the grey values of u such that 
  min(u) -> a, and max(u) -> b. 
*/

{
long    i, j;       /* loop variables */
double  min, max;   /* extrema of u */

/* find extrema of u */
min = max = u[1][1];
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     {
     if (u[i][j] < min) min = u[i][j];
     if (u[i][j] > max) max = u[i][j];
     }

/* rescale */
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     u[i][j] = a + (u[i][j] - min) / (max - min) * (b - a);

return;

}  /* rescale */

/*--------------------------------------------------------------------------*/

void lowpass

     (double  sigma,      /* standard deviation of Gaussian */
      long    nx,         /* image dimension in x direction */
      long    ny,         /* image dimension in y direction */
      double  hx,         /* pixel size in x direction */
      double  hy,         /* pixel size in y direction */
      double  **u)        /* input: original; output: lowpass filtered */

/* 
  lowpass filter 
*/

{  
/* apply Gaussian convolution */
gauss_conv (sigma, 0, 3.0, nx, ny, hx, hy, u);

return;

}  /* lowpass */

/*--------------------------------------------------------------------------*/

void highpass

     (double   sigma,      /* standard deviation of Gaussian */
      long     nx,         /* image dimension in x direction */
      long     ny,         /* image dimension in y direction */
      double   hx,         /* pixel size in x direction */
      double   hy,         /* pixel size in y direction */
      double   **u)        /* input: original; output: highpass filtered */
      
/*
  highpass filter 
*/

{  
long    i, j;      /* loop variables */
double  **v;       /* Gaussian-smoothed image */

/* allocate memory */
alloc_double_matrix (&v, nx+2, ny+2);

/* copy image u to v */
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     v[i][j] = u[i][j];

/* apply Gaussian to temporary array */
gauss_conv (sigma, 0, 3.0, nx, ny, hx, hy, v);

/* compute highpass filter */
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++) 
     {
     /* SUPPLEMENT YOUR CODE HERE */
     }
  
/* free memory */
free_double_matrix (v, nx+2, ny+2);

return;

}  /* highpass */

/*--------------------------------------------------------------------------*/

void bandpass

     (double   sigma1,     /* standard deviation of first Gaussian */
      double   sigma2,     /* standard deviation of second Gaussian */
      long     nx,         /* image dimension in x direction */
      long     ny,         /* image dimension in y direction */
      double   hx,         /* pixel size in x direction */
      double   hy,         /* pixel size in y direction */
      double   **u)        /* input: original; output: bandpass filtered */

/* 
  bandpass filter with sigma1 > sigma2 
*/

{
long    i, j;   /* loop variables */
double  **v;    /* Gaussian-smoothed image with standard deviation sigma1 */
double  **w;    /* Gaussian-smoothed image with standard deviation sigma2 */

/* allocate memory */
alloc_double_matrix (&v, nx+2, ny+2);
alloc_double_matrix (&w, nx+2, ny+2);

/* copy f to v and w */
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++) 
     {
     v[i][j] = u[i][j];
     w[i][j] = u[i][j];
     }

/* Gaussian smoothing of v and w */
gauss_conv (sigma1, 0, 3.0, nx, ny, hx, hy, v);
gauss_conv (sigma2, 0, 3.0, nx, ny, hx, hy, w);

/* compute bandpass filter */
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++) 
     {
     /* SUPPLEMENT YOUR CODE HERE */
     }

/* free memory */
free_double_matrix (v, nx+2, ny+2);
free_double_matrix (w, nx+2, ny+2);

return;

}  /* bandpass */

/*--------------------------------------------------------------------------*/

int main ()

{
char    in[80];               /* for reading data */
char    out[80];              /* for reading data */
double  **u;                  /* image */
long    nx, ny;               /* image size in x, y direction */ 
double  sigma1;               /* standard deviation for first Gaussian */
double  sigma2;               /* standard deviation for second Gaussian */
long    filter;               /* variable for filter choice */
double  max, min;             /* largest, smallest grey value */
double  mean;                 /* average grey value */
double  std;                  /* standard deviation */
char    comments[1600];       /* string for comments */

printf ("\n");
printf ("GAUSSIAN-BASED HIGHPASS, LOWPASS, AND BANDPASS FILTERS\n\n");
printf ("*****************************************************\n\n");
printf ("    Copyright 2021 by Joachim Weickert               \n");
printf ("    and Pascal Peter                                 \n");
printf ("    Dept. of Mathematics and Computer Science        \n");
printf ("    Saarland University, Saarbruecken, Germany       \n\n");
printf ("    All rights reserved. Unauthorized usage,         \n");
printf ("    copying, hiring, and selling prohibited.         \n\n");
printf ("    Send bug reports to                              \n");
printf ("    weickert@mia.uni-saarland.de                     \n\n");
printf ("*****************************************************\n\n");


/* ---- read input image (pgm format P5) ---- */

printf ("input image (pgm):                           ");
read_string (in);
read_pgm_to_double (in, &nx, &ny, &u);


/* ---- read parameters ---- */

printf ("available linear filters:\n");
printf (" (0) lowpass             \n");
printf (" (1) highpass            \n");
printf (" (2) bandpass            \n");
printf ("your choice:                                 ");
read_long (&filter);

if ((filter == 0) || (filter == 1))
   {
   printf ("Gaussian standard deviation sigma:           ");
   read_double (&sigma1);
   }
if (filter == 2) 
   {
   printf ("larger Gaussian standard deviation sigma1:   ");
   read_double (&sigma1);
   printf ("smaller Gaussian standard deviation sigma2:  ");
   read_double (&sigma2);
   }

printf ("output image (pgm):                          ");
read_string (out);
printf ("\n");


/* ---- analyse input image ---- */

analyse_grey_double (u, nx, ny, &min, &max, &mean, &std);
printf ("input image:\n");
printf ("minimum:       %8.2lf \n", min);
printf ("maximum:       %8.2lf \n", max);
printf ("mean:          %8.2lf \n", mean);
printf ("standard dev.: %8.2lf \n\n", std);


/* ---- process image with linear filter ---- */

if (filter == 0) 
   {
   printf ("applying lowpass filter\n\n");
   lowpass (sigma1, nx, ny, 1.0, 1.0, u);
   }
else if (filter == 1) 
   {
   printf ("applying highpass filter\n\n");
   highpass (sigma1, nx, ny, 1.0, 1.0, u);
   } 
else if (filter == 2) 
   {
   printf ("applying bandpass filter\n\n");
   bandpass (sigma1, sigma2, nx, ny, 1.0, 1.0, u);
   }


/* ---- analyse filtered image ---- */

analyse_grey_double (u, nx, ny, &min, &max, &mean, &std);
printf ("filtered image:\n");
printf ("minimum:       %8.2lf \n", min);
printf ("maximum:       %8.2lf \n", max);
printf ("mean:          %8.2lf \n", mean);
printf ("standard dev.: %8.2lf \n\n", std);


/* ---- perform affine rescaling for highpass or bandpass filters ---- */

if (filter >= 1)
   rescale (u, nx, ny, 0.0, 255.0);


/* ---- write output image (pgm format P5) ---- */

/* generate comment string */
comments[0]='\0';
if (filter == 0)
   {
   comment_line (comments, "# Gaussian lowpass filter\n");
   comment_line (comments, "# input image:  %s\n", in);
   comment_line (comments, "# sigma:      %8.2lf\n", sigma1);
   comment_line (comments, "# min:        %8.2lf\n", min);
   comment_line (comments, "# max:        %8.2lf\n", max);
   comment_line (comments, "# mean:       %8.2lf\n", mean);
   comment_line (comments, "# st. dev.:   %8.2lf\n", std);
   }
else if (filter == 1)
   {
   comment_line (comments, "# Gaussian highpass filter\n");
   comment_line (comments, "# input image:  %s\n", in);
   comment_line (comments, "# sigma:      %8.2lf\n", sigma1);
   comment_line (comments, "# min:        %8.2lf\n", min);
   comment_line (comments, "# max:        %8.2lf\n", max);
   comment_line (comments, "# mean:       %8.2lf\n", mean);
   comment_line (comments, "# st. dev.:   %8.2lf\n", std);
   comment_line (comments, "# affinely rescaled to [0,255]");
   }
else if (filter == 2)
   {
   comment_line (comments, "# Gaussian bandpass filter\n");
   comment_line (comments, "# input image:  %s\n", in);
   comment_line (comments, "# sigma1:     %8.2lf\n", sigma1);
   comment_line (comments, "# sigma2:     %8.2lf\n", sigma2);
   comment_line (comments, "# min:        %8.2lf\n", min);
   comment_line (comments, "# max:        %8.2lf\n", max);
   comment_line (comments, "# mean:       %8.2lf\n", mean);
   comment_line (comments, "# st. dev.:   %8.2lf\n", std);
   comment_line (comments, "# affinely rescaled to [0,255]");
   }

/* write image */
write_double_to_pgm (u, nx, ny, out, comments);
printf ("output image %s successfully written\n\n", out);


/* ---- free memory  ---- */

free_double_matrix (u, nx+2, ny+2);

return(0);
}
