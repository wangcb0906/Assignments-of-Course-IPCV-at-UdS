#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <ctype.h>

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                        DISCRETE FOURIER TRANSFORM                        */
/*                                                                          */
/*             (Copyright by Martin Welk, Pascal Peter, 1/2013              */
/*                       and Joachim Weickert, 4/2021)                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/*
  features:
  - DFT and FFT
  - two output images:
    (i)  logarithmic Fourier spectrum
    (ii) Fourier transform in double precision
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
}

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

long mylog2 

     (long n)               /* should be positive */

/*
  returns ld(n) if n is a power of 2; 
  returns -1    in all other cases. 
*/

{
long  ld;     /* ld(n) */
long  m;      /* auxiliary variable */

if (n <= 0)
   ld = -1;
else if (n == 1)
   ld = 0;
else
   {
   ld = 0;
   m  = 1; 
   do {
      m = 2 * m; 
      ld = ld + 1;
      }
   while (m < n);
   if (m > n) ld = -1;
   }

return (ld);
}

/*--------------------------------------------------------------------------*/

void FFT 

     (double   *vr,         /* real part of signal / Fourier coeff. */
      double   *vi,         /* imaginary part of signal / Fourier coeff. */
      long     n)           /* signal length, has to be power of 2 */ 

/*
  Fast Fourier Transform of a (complex) 1-D signal. 
  Based on the description in the Bronstein book.
  The signal length has to be a power of 2.
  Written by Martin Welk.
*/

{
const    float fa = sqrt(0.5);    /* frequently used renormalization factor */
long     p, q, r, j, k;           /* variables for indices and sizes */
long     nh, qq, qh;              /* variables for indices and sizes */
long     jq, jqh, jv, jn;         /* variables for indices and sizes */
long     logn;                    /* ld(n) */
long     m;                       /* auxiliary variable */
double   rh, ih, ch, chih;        /* for intermediate results */
double   *scrr, *scri, *exh;      /* auxiliary vectors */
double   *srr;                    /* point at source arrays, real part */ 
double   *sri;                    /* point at source arrays, imag. part */ 
double   *der;                    /* point at dest. arrays, real part */ 
double   *dei;                    /* point at dest. arrays, imag. part */
double   *swpp;                   /* used for pointer swapping */


/* ---- memory allocations ---- */

alloc_double_vector (&scrr, n);
alloc_double_vector (&scri, n);
alloc_double_vector (&exh,  n);


/* ---- initialisations ----*/

/* initialise pointers */
srr = vr; 
sri = vi; 
der = scrr; 
dei = scri; 

/* logn := ld(n) */
m = n;
logn = -1;
while (m >= 1)
      {
      m >>= 1;              /* m = m / 2 */
      logn ++;               
      }

/* initialise sizes */
nh = n>>1;                  /* n / 2 */ 
qh = nh>>1;                 /* n / 4 */ 

/* trigonometric values */
exh[0]  = 1.0;   exh[nh]    = 0.0;    /* exp (2 pi i 0.0 ) */
exh[qh] = 0.0;   exh[nh+qh] = 1.0;    /* exp (2 pi i 0.25) */
ch = -1.0;                            /* cos pi */
/* further trigonometric values will be computed by recursion */

/* other initialisations */
qq = n; 
q  = nh; 
qh = q>>1; 
p  = 1;


/* ---- loop through all levels ----*/

for (r=logn; r>=1; r--) 
    {
    /* iterate through levels */
    if (r < logn) 
       ch = sqrt (0.5 * (1.0 + ch));    /* recursion for cosines */
    for (j=0; j<p; j++) 
        {         
        /* iterate through columns */
        jq = j * qq; 
        jqh = jq >> 1;
        if (((j&1)==1) && (r<logn-1)) 
           {       
           /* recursion for exp(i*angle) */
           chih = 0.5 / ch;                    /* cosine inverse half */
           jv = jqh - q;                       
           jn = jqh + q; 
           if (jn == nh) 
              {                   
              /* use half-angle formula for exp */
              exh[jqh]    = (exh[jv] - 1.0) * chih;
              exh[jqh+nh] = exh[jv+nh] * chih;
              }
           else 
              {
              exh[jqh]    = (exh[jv]    + exh[jn]   ) * chih;
              exh[jqh+nh] = (exh[jv+nh] + exh[jn+nh]) * chih;
              }
           } /* if */
        for (k=0; k<q; k++) 
            {               
            /* iterate through rows */
            rh =  exh[jqh]    * srr[jq+k+q] + exh[jqh+nh] * sri[jq+k+q];
            ih = -exh[jqh+nh] * srr[jq+k+q] + exh[jqh]    * sri[jq+k+q];
            der[jqh+k]    = fa * (srr[jq+k] + rh);
            dei[jqh+k]    = fa * (sri[jq+k] + ih);
            der[jqh+nh+k] = fa * (srr[jq+k] - rh);
            dei[jqh+nh+k] = fa * (sri[jq+k] - ih);
            }
        } /* for j */
        
        /* swap array pointers */
        swpp = srr;   srr = der;   der = swpp;        
        swpp = sri;   sri = dei;   dei = swpp;

        /* adjust sizes */ 
        qq = q; 
        q = qh; 
        qh >>= 1; 
        p <<= 1;          
    }  /* for r */

/* copy f^ back, if ld(n) is odd */
if ((logn&1) == 1)                             
   for (j=0; j<n; j++) 
       {                 
       der[j] = srr[j]; 
       dei[j] = sri[j]; 
       }


/* ---- free memory ----*/

free_double_vector (scrr, n);
free_double_vector (scri, n);
free_double_vector (exh,  n);

return;

} /* FFT */
  
/* ----------------------------------------------------------------------- */

void DFT  

     (double   *vr,         /* real part of signal / Fourier coeff. */
      double   *vi,         /* imaginary part of signal / Fourier coeff. */
      long     n)           /* signal length (>0) */ 


/* 
 Discrete Fourier transform of a (complex) 1-D signal. Quadratic complexity.
 Does not require powers of 2 as signal length.
*/

{
long    i, j;              /* loop variables */
double  help1, help2;      /* time savers */
double  help3, c, s;       /* time savers */
double  *fr, *fi;          /* auxiliary vectors (real / imaginary part) */
     
 
/* ---- allocate memory ---- */

alloc_double_vector (&fr, n);
alloc_double_vector (&fi, n);


/* ---- copy (vr,vi) into (fr,fi) ---- */

for (i=0; i<=n-1; i++)
    {
    fr[i] = vr[i];
    fi[i] = vi[i];
    }


/* ---- time savers ---- */

help1 = - 2.0 * 3.14159265359 / (double)n;
help2 = 1.0 / sqrt ((double)n);

 
/* ---- perform DFT ---- */

for (i=0; i<=n-1; i++)
    {
    vr[i] = 0.0;
    vi[i] = 0.0;
    for (j=0; j<=n-1; j++)
        {
        help3 = help1 * i * j;
        c     = cos (help3);
        s     = sin (help3);
        vr[i] = vr[i] + fr[j] * c - fi[j] * s;
        vi[i] = vi[i] + fi[j] * c + fr[j] * s;
        }
    vr[i] = vr[i] * help2;
    vi[i] = vi[i] * help2;
    }


/* ---- free memory ---- */

free_double_vector (fr, n);
free_double_vector (fi, n);

return;

} /* DFT */

/* ---------------------------------------------------------------------- */

void FT2D  

     (double   **ur,        /* real part of image / Fourier coeff. */
      double   **ui,        /* imaginary part of image / Fourier coeff. */
      long     nx,          /* pixel number in x direction */ 
      long     ny)          /* pixel number in y direction */ 


/* 
  Two-dimensional discrete Fourier transform of a (complex) image.
  This algorithm exploits the separability of the Fourier transform. 
  Uses FFT when the pixel numbers are powers of 2, DFT otherwise.
*/


{
long    i, j;              /* loop variables */
long    n;                 /* max (nx, ny) */
long    logn;              /* ld(n) */
double  *vr, *vi;          /* real / imaginary signal or Fourier data */


/* ---- allocate memory for auxiliary vectors vr, vi ---- */

if (nx > ny) 
   n = nx; 
else 
   n = ny;

alloc_double_vector (&vr, n);
alloc_double_vector (&vi, n);


/* ---- transform along x direction ---- */

logn = mylog2 (nx);
for (j=0; j<=ny-1; j++)
    {
    /* write in 1-D vector */
    for (i=0; i<=nx-1; i++)
        {
        vr[i] = ur[i+1][j+1];
        vi[i] = ui[i+1][j+1];
        }

    /* apply Fourier transform */
    if (logn == -1)
       /* nx is not a power of 2; use DFT */
       DFT (vr, vi, nx); 
    else
       /* nx is a power of 2; use FFT */
       FFT (vr, vi, nx); 

    /* write back in 2-D image */
    for (i=0; i<=nx-1; i++)
        {
        ur[i+1][j+1] = vr[i];
        ui[i+1][j+1] = vi[i];
        }
    }


/* ---- transform along y direction ---- */

logn = mylog2 (ny);
for (i=0; i<=nx-1; i++)
    {
    /* write in 1-D vector */
    for (j=0; j<=ny-1; j++)
        {
        vr[j] = ur[i+1][j+1];
        vi[j] = ui[i+1][j+1];
        }

    /* apply Fourier transform */
    if (logn == -1) 
       /* ny is not a power of 2; use DFT */
       DFT (vr, vi, ny);
    else
       /* ny is a power of 2; use FFT */
       FFT (vr, vi, ny); 

    /* write back in 2-D image */
    for (j=0; j<=ny-1; j++)
        {
        ur[i+1][j+1] = vr[j];
        ui[i+1][j+1] = vi[j];
        }
    }


/* ---- free memory ---- */

free_double_vector (vr, n);
free_double_vector (vi, n);

return;

} /* FT2D */

/* ---------------------------------------------------------------------- */

void periodic_shift  

     (double   **u,         /* image, changed */
      long     nx,          /* pixel number in x direction */ 
      long     ny,          /* pixel number in y direction */
      long     xshift,      /* shift in x direction */ 
      long     yshift)      /* shift in y direction */ 

/*
  shifts an image u by the translation vector (xshift,yshift) 
  with 0 <= xshift <= nx-1 and 0 <= yshift <= ny-1
*/

{
long    i, j;         /* loop variables */
double  **f;          /* auxiliary image */

/* allocate memory */
alloc_double_matrix (&f, nx + 2, ny + 2);

/* shift in x direction */
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     if (i-xshift >= 1)
        f[i][j] = u[i-xshift][j];
     else 
        f[i][j] = u[i+nx-xshift][j];

/* shift in y direction */
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     if (j-yshift >= 1)
        u[i][j] = f[i][j-yshift];
     else 
        u[i][j] = f[i][j+ny-yshift];

/* free memory */
free_double_matrix (f, nx + 2, ny + 2);

return;

} /* periodic_shift */

/* ---------------------------------------------------------------------- */

void filter

     (long     nx,        /* image dimension in x direction */
      long     ny,        /* image dimension in y direction */
      double   **ur,      /* input: original real image */
      double   **ui)      /* input: original imaginary image */

/*
  allows to filter the Fourier coefficients
*/

{
long   i, j;        /* loop variables */
long   centre_x;    /* nx/2 + 1 */
long   centre_y;    /* ny/2 + 1 */

/* compute centres */
centre_x = nx/2 + 1;
centre_y = ny/2 + 1;

long r = 1;
long height = 0;

// printf ("filter radius, inside radius will not be filtered\n\
// r should be bigger than 1 (integer):     \n\n");
// read_long (&r);

printf ("filter height (integer):     \n\n");
read_long (&height);

/* filter Fourier coefficients */
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     {
      if((j >= centre_x - height && j <= centre_x + height) && (i <= centre_y - r || i >= centre_y + r)){
         ur[i][j] = 0;
         ui[i][j] = 0;
      }
      // if((i >= centre_x - height && i <= centre_x + height) && (j <= centre_y - r || j >= centre_y + r)){
      //    ur[i][j] = 0;
      //    ui[i][j] = 0;
      // }
     }

return;

} /* filter */

/* ---------------------------------------------------------------------- */

int main ()

{
char    in[80];               /* for reading data */
char    out1[80];             /* for reading data */
char    out2[80];             /* for reading data */
double  **ur, **ui;           /* real / imaginary image or Fourier data */
double  **w, **m;             /* logarithmic Fourier spectrum */
long    nx, ny;               /* image size in x, y direction */
long    i, j;                 /* loop variables */
double  help;                 /* auxiliary variable for rescaling */
double  max;                  /* maximum */
char    comments[1600];       /* string for comments */

printf ("\n");
printf ("FOURIER ANALYSIS\n\n");
printf ("**************************************************\n\n");
printf ("    Copyright 2021 by Joachim Weickert            \n");
printf ("    and 2013 by Martin Welk and Pascal Peter      \n");
printf ("    Dept. of Mathematics and Computer Science     \n");
printf ("    Saarland University, Saarbruecken, Germany    \n\n");
printf ("    All rights reserved. Unauthorized usage,      \n");
printf ("    copying, hiring, and selling prohibited.      \n\n");
printf ("    Send bug reports to                           \n");
printf ("    weickert@mia.uni-saarland.de                  \n\n");
printf ("**************************************************\n\n");


/* ---- read input image (pgm format P5) ---- */

printf ("input image (pgm):                     ");
read_string (in);
read_pgm_to_double (in, &nx, &ny, &ur);  /* also allocates memory for ur */

/* allocate memory and initialise imaginary image */
alloc_double_matrix (&ui, nx+2, ny+2);
alloc_double_matrix (&w,  nx+2, ny+2);
alloc_double_matrix (&m,  nx+2, ny+2);
for (j=0; j<=ny+1; j++)
 for (i=0; i<=nx+1; i++)
     ui[i][j] = 0.0;


/* ---- read parameters ---- */

printf ("output image 1 (log. spectrum) (pgm):  ");
read_string (out1);

printf ("output image 2 (backtransform) (pgm):  ");
read_string (out2);
printf ("\n");


/* ---- compute discrete Fourier transformation ---- */

printf ("computing Fourier transformation\n");
FT2D (ur, ui, nx, ny);


/* ---- shift lowest frequency in the centre ----*/

periodic_shift (ur, nx, ny, nx/2, ny/2);
periodic_shift (ui, nx, ny, nx/2, ny/2);


/* ---- manipulate the Fourier coefficients ---- */

filter (nx, ny, ur, ui);


/* ---- compute logarithmic spectrum ---- */

printf ("computing logarithmic spectrum\n");
max = 0.0;
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     {
     w[i][j] = log (1.0 + sqrt (ur[i][j] * ur[i][j] + ui[i][j] * ui[i][j]));
     if (w[i][j] > max) 
        max = w[i][j];
     }

/* rescale such that max(w[i][j])=255 */
if (max > 0.0)
   {
   help = 255.0 / max;
   for (i=1; i<=nx; i++)
    for (j=1; j<=ny; j++)
        w[i][j] = help * w[i][j];
   }


/* ---- shift lowest frequency back to the corners ----*/

periodic_shift (ur, nx, ny, nx-nx/2, ny-ny/2);
periodic_shift (ui, nx, ny, nx-nx/2, ny-ny/2);


/* ---- compute discrete Fourier backtransformation ---- */

printf ("computing Fourier backtransformation\n\n");

/* backtransformation = DFT of complex conjugated Fourier coefficients */
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     ui[i][j] = - ui[i][j];
FT2D (ur, ui, nx, ny);


/* ---- write output image 1 (log. spectrum) (pgm format P5) ---- */

/* generate comment string */
comments[0]='\0';
comment_line (comments, "# logarithmic Fourier spectrum\n");

/* write image */
write_double_to_pgm (w, nx, ny, out1, comments);
printf ("output image %s successfully written\n\n", out1);


/* ---- write output image 2 (backtransform) (pgm format P5) ---- */

/* generate comment string */
comments[0]='\0';
comment_line (comments, "# Fourier filtering\n");

/* write image */
write_double_to_pgm (ur, nx, ny, out2, comments);
printf ("output image %s successfully written\n\n", out2);


/* ---- free memory  ---- */

free_double_matrix (ur, nx+2, ny+2);
free_double_matrix (ui, nx+2, ny+2);
free_double_matrix (w,  nx+2, ny+2);
free_double_matrix (m,  nx+2, ny+2);

return(0);
}
