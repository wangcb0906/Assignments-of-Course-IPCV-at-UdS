#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <ctype.h>

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                        DISCRETE COSINE TRANSFORM                         */
/*                                                                          */
/*    (Copyright by Andres Bruhn, 11/2007 and Joachim Weickert, 5/2021)     */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/*
  Discrete Cosine Transform
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

void jpeg_multiply_block

     (double  **c_block)    /* coefficients of the DCT */

/*
  weights 8x8 coefficient block with JPEG weighting matrix
*/

{
c_block[0][0] *= 10;
c_block[0][1] *= 15;
c_block[0][2] *= 25;
c_block[0][3] *= 37;
c_block[0][4] *= 51;
c_block[0][5] *= 66;
c_block[0][6] *= 82;
c_block[0][7] *= 100;

c_block[1][0] *= 15;
c_block[1][1] *= 19;
c_block[1][2] *= 28;
c_block[1][3] *= 39;
c_block[1][4] *= 52;
c_block[1][5] *= 67;
c_block[1][6] *= 83;
c_block[1][7] *= 101;

c_block[2][0] *= 25;
c_block[2][1] *= 28;
c_block[2][2] *= 35;
c_block[2][3] *= 45;
c_block[2][4] *= 58;
c_block[2][5] *= 72;
c_block[2][6] *= 88;
c_block[2][7] *= 105;

c_block[3][0] *= 37;
c_block[3][1] *= 39;
c_block[3][2] *= 45;
c_block[3][3] *= 54;
c_block[3][4] *= 66;
c_block[3][5] *= 79;
c_block[3][6] *= 94;
c_block[3][7] *= 111;

c_block[4][0] *= 51;
c_block[4][1] *= 52;
c_block[4][2] *= 58;
c_block[4][3] *= 66;
c_block[4][4] *= 76;
c_block[4][5] *= 89;
c_block[4][6] *= 103;
c_block[4][7] *= 119;

c_block[5][0] *= 66;
c_block[5][1] *= 67;
c_block[5][2] *= 72;
c_block[5][3] *= 79;
c_block[5][4] *= 89;
c_block[5][5] *= 101;
c_block[5][6] *= 114;
c_block[5][7] *= 130;

c_block[6][0] *= 82;
c_block[6][1] *= 83;
c_block[6][2] *= 88;
c_block[6][3] *= 94;
c_block[6][4] *= 103;
c_block[6][5] *= 114;
c_block[6][6] *= 127;
c_block[6][7] *= 142;

c_block[7][0] *= 100;
c_block[7][1] *= 101;
c_block[7][2] *= 105;
c_block[7][3] *= 111;
c_block[7][4] *= 119;
c_block[7][5] *= 130;
c_block[7][6] *= 142;
c_block[7][7] *= 156;

return;

}  /* jpeg_multiply_block */

/*--------------------------------------------------------------------------*/

void equal_multiply_block

     (double  **c_block,    /* coefficients of the DCT */
      long    factor)       /* factor to multiply */

/*
  multiplies an 8x8 coefficient block with a given factor
*/

{
long i, j;        /* loop variables */

for (i=0; i<=7; i++)
 for (j=0; j<=7; j++)
     c_block[i][j] *= factor;

return;
}

/*--------------------------------------------------------------------------*/

void jpeg_divide_block

     (double  **c_block)    /* coefficients of the DCT */

/*
  weights 8x8 coefficient block with inverse JPEG weighting matrix
*/

{
c_block[0][0] /= 10;
c_block[0][1] /= 15;
c_block[0][2] /= 25;
c_block[0][3] /= 37;
c_block[0][4] /= 51;
c_block[0][5] /= 66;
c_block[0][6] /= 82;
c_block[0][7] /= 100;

c_block[1][0] /= 15;
c_block[1][1] /= 19;
c_block[1][2] /= 28;
c_block[1][3] /= 39;
c_block[1][4] /= 52;
c_block[1][5] /= 67;
c_block[1][6] /= 83;
c_block[1][7] /= 101;

c_block[2][0] /= 25;
c_block[2][1] /= 28;
c_block[2][2] /= 35;
c_block[2][3] /= 45;
c_block[2][4] /= 58;
c_block[2][5] /= 72;
c_block[2][6] /= 88;
c_block[2][7] /= 105;

c_block[3][0] /= 37;
c_block[3][1] /= 39;
c_block[3][2] /= 45;
c_block[3][3] /= 54;
c_block[3][4] /= 66;
c_block[3][5] /= 79;
c_block[3][6] /= 94;
c_block[3][7] /= 111;

c_block[4][0] /= 51;
c_block[4][1] /= 52;
c_block[4][2] /= 58;
c_block[4][3] /= 66;
c_block[4][4] /= 76;
c_block[4][5] /= 89;
c_block[4][6] /= 103;
c_block[4][7] /= 119;

c_block[5][0] /= 66;
c_block[5][1] /= 67;
c_block[5][2] /= 72;
c_block[5][3] /= 79;
c_block[5][4] /= 89;
c_block[5][5] /= 101;
c_block[5][6] /= 114;
c_block[5][7] /= 130;

c_block[6][0] /= 82;
c_block[6][1] /= 83;
c_block[6][2] /= 88;
c_block[6][3] /= 94;
c_block[6][4] /= 103;
c_block[6][5] /= 114;
c_block[6][6] /= 127;
c_block[6][7] /= 142;

c_block[7][0] /= 100;
c_block[7][1] /= 101;
c_block[7][2] /= 105;
c_block[7][3] /= 111;
c_block[7][4] /= 119;
c_block[7][5] /= 130;
c_block[7][6] /= 142;
c_block[7][7] /= 156;

return;

} /* jpeg_divide_block */

/*--------------------------------------------------------------------------*/

void round_block_coeff

     (double  **c_block)    /* coefficients of the DCT */

/*
  round entries of a 8x8 coefficient block
*/

{
long i, j;        /* loop variables */

for (i=0; i<=7; i++)
 for (j=0; j<=7; j++)
     c_block[i][j] = rint (c_block[i][j]);

return;
}

/*--------------------------------------------------------------------------*/

void equal_divide_block

     (double  **c_block,    /* coefficients of the DCT */
      long    factor)       /* factor to divide */

/*
  divides an 8x8 coefficient block by a given factor
*/

{
long i, j;        /* loop variables */

for (i=0; i<=7; i++)
 for (j=0; j<=7; j++)
     c_block[i][j] /= factor;

return;
}

/*--------------------------------------------------------------------------*/

void DCT_2d

     (double  **u,          /* image, unchanged */
      double  **c,          /* coefficients of the DCT */
      long    nx,           /* pixel number in x-direction */
      long    ny)           /* pixel number in y-direction */

/*
  computes DCT of input image
*/

{
long    i, j, m, p;    /* loop variables */
double  nx_1;          /* time saver */
double  ny_1;          /* time saver */
double  pi;            /* variable pi */
double  **tmp;         /* temporary image */
double  *cx, *cy;      /* arrays for coefficients */


/* ---- compute pi ---- */

pi = 2.0 * asin (1.0);


/* ---- set time savers ---- */

nx_1 = pi / (2.0 * nx);
ny_1 = pi / (2.0 * ny);


/* ---- allocate memory ---- */

alloc_double_matrix (&tmp, nx, ny);
alloc_double_vector (&cx, nx);
alloc_double_vector (&cy, ny);


/* ---- compute coefficients ---- */

cx[0] = sqrt (1.0 / nx);
for (p=1; p<nx; p++)
    cx[p] = sqrt (2.0 / nx);

cy[0] = sqrt (1.0 / ny);
for (p=1; p<ny; p++)
    cy[p] = sqrt (2.0 / ny);


/* ---- DCT in y-direction ---- */

for (i=0; i<nx; i++)
 for (p=0; p<ny; p++)
     {
      for(m=0; m<ny; m++)
       tmp[i][p] += u[i][m] * cy[p] * cos(ny_1 * (2 * m + 1) * p);
     }


/* ---- DCT in x-direction ---- */

for (p=0; p<nx; p++)
 for (j=0; j<ny; j++)
     {
      for(m=0; m<nx; m++)
       c[p][j] += tmp[m][j] * cx[p] * cos(nx_1 * (2 * m + 1) * p);
     }


/* ---- free memory ---- */

free_double_matrix (tmp, nx, ny);
free_double_vector (cx, nx);
free_double_vector (cy, ny);

return;

} /* DCT_2d */

/*--------------------------------------------------------------------------*/

void IDCT_2d

     (double  **u,          /* image, unchanged */
      double  **c,          /* coefficients of the DCT */
      long    nx,           /* pixel number in x-direction */
      long    ny)           /* pixel number in y-direction */

/*
  computes inverse DCT of input image
*/

{
long    i, j, m, p;    /* loop variables */
double  nx_1;          /* time saver */
double  ny_1;          /* time saver */
double  pi;            /* variable pi */
double  **tmp;         /* temporary image */
double  *cx, *cy;      /* arrays for coefficients */


/* ---- compute pi ---- */

pi = 2.0 * asin (1.0);


/* ---- set time savers ---- */

nx_1 = pi / (2.0 * nx);
ny_1 = pi / (2.0 * ny);


/* ---- allocate memory ---- */

alloc_double_matrix (&tmp, nx, ny);
alloc_double_vector (&cx, nx);
alloc_double_vector (&cy, ny);


/* ---- compute coefficients ---- */

cx[0] = sqrt (1.0 / nx);
for (m=1; m<nx; m++)
    cx[m] = sqrt (2.0 / nx);

cy[0] = sqrt (1.0 / ny);
for (m=1; m<ny; m++)
    cy[m] = sqrt (2.0 / ny);


/* ---- DCT in y-direction ---- */

for (i=0; i<nx; i++)
 for (m=0; m<ny; m++)
     {
      for(p=0; p<ny; p++)
       tmp[i][m] += cy[p] * c[i][p] * cos(ny_1 * (2 * m + 1) * p);
     }


/* ---- DCT in x-direction ---- */

for (m=0; m<nx; m++)
 for (j=0; j<ny; j++)
     {
      u[m][j] = 0;
      for(p=0; p<nx; p++)
       u[m][j] += cx[p] * tmp[p][j] * cos(nx_1 * (2 * m + 1) * p);
     }


/* ---- free memory ---- */

free_double_matrix (tmp, nx, ny);
free_double_vector (cx, nx);
free_double_vector (cy, ny);

return;

} /* IDCT_2d */

/*--------------------------------------------------------------------------*/

void remove_freq_2d

     (double  **c,          /* in and out: coefficients of the DCT */
      long    nx,           /* pixel number in x-direction */
      long    ny)           /* pixel number in y-direction */

/*
  removes frequencies
*/

{
long i, j;        /* loop variables */

/* set frequencies to zero */
for (i=0; i<nx; i++)
 for (j=0; j<ny; j++)
     if ((i >= nx / sqrt (10.0)) || (j >= ny / sqrt (10.0)))
        c[i][j] = 0.0;

return;
}

/*--------------------------------------------------------------------------*/

void blockwise_DCT_2d

     (double  **u,          /* in: image */
      double  **c,          /* out: coefficients of the DCT */
      long    nx,           /* pixel number in x-direction */
      long    ny)           /* pixel number in y-direction */

/*
  computes DCT in image blocks of size 8x8
*/

{
long    i, j, k, l;       /* loop variables */
double  **u_block;        /* 8x8 block */
double  **c_block;        /* 8x8 block */


/* ---- allocate memory for 8x8 blocks ---- */

alloc_double_matrix (&u_block, 8, 8);
alloc_double_matrix (&c_block, 8, 8);


/* ---- DCT on 8x8 blocks ---- */

for (i=0; i<nx; i+=8)
 for (j=0; j<ny; j+=8)
     {
     /* copy 8x8 block */
     for (k=0; k<=7; k++)
      for (l=0; l<=7; l++){
        u_block[k][l] = u[i+k][j+l];
        c_block[k][l] = 0;
      }


     /* DCT of 8x8 block */
     DCT_2d (u_block, c_block, 8, 8);

     /* copy back coefficients */
     for (k=0; k<=7; k++)
      for (l=0; l<=7; l++)
	        c[i+k][j+l] = c_block[k][l];

     }

/* ---- free memory for 8x8 block ---- */

free_double_matrix (u_block, 8, 8);
free_double_matrix (c_block, 8, 8);

return;

} /* blockwise_DCT_2d */

/*--------------------------------------------------------------------------*/

void blockwise_IDCT_2d

     (double  **u,          /* out: image */
      double  **c,          /* in: coefficients of the DCT */
      long    nx,           /* pixel number in x-direction */
      long    ny)           /* pixel number in y-direction */

/*
  computes inverse DCT in image blocks of size 8x8
*/

{
long    i, j, k, l;     /* loop variables */
double  **u_block;      /* 8x8 block */
double  **c_block;      /* 8x8 block */


/* ---- allocate memory for 8x8 blocks ---- */

alloc_double_matrix (&u_block, 8, 8);
alloc_double_matrix (&c_block, 8, 8);


/* ---- inverse DCT on 8x8 blocks ---- */

for (i=0; i<nx; i+=8)
 for (j=0; j<ny; j+=8)
     {
     /* copy 8x8 block */
     for (k=0; k<=7; k++)
      for (l=0; l<=7; l++){
        c_block[k][l] = c[i+k][j+l];
        u_block[k][l] = 0;
      }


     /* inverse DCT of 8x8 block */
     IDCT_2d (u_block, c_block, 8, 8);

     /* copy back coefficients */
     for (k=0; k<=7; k++)
      for (l=0; l<=7; l++)
	        u[i+k][j+l] = u_block[k][l];
     }


/* ---- free memory for 8x8 block ---- */

free_double_matrix (u_block, 8, 8);
free_double_matrix (c_block, 8, 8);

return;

} /* blockwise_IDCT_2d */

/*--------------------------------------------------------------------------*/

void blockwise_remove_freq_2d

     (double  **c,          /* in and out: coefficients of the DCT */
      long    nx,           /* pixel number in x-direction */
      long    ny)           /* pixel number in y-direction */

/*
  removes frequencies within 8x8 block
*/

{
long    i, j, k, l;       /* loop variables */
double  **c_block;        /* 8x8 block */


/* ---- allocate memory for 8x8 block ---- */

alloc_double_matrix (&c_block, 8, 8);


/* ---- scale coefficients blockwise ---- */

for (i=0; i<nx; i+=8)
 for (j=0; j<ny; j+=8)
     {
     /* copy 8x8 block */
     for (k=0; k<=7; k++)
      for (l=0; l<=7; l++)
          c_block[k][l] = c[i+k][j+l];

     /* set frequencies to zero */
     remove_freq_2d (c_block, 8, 8);

     /* copy back coefficients */
     for (k=0; k<=7; k++)
      for (l=0; l<=7; l++)
          c[i+k][j+l] = c_block[k][l];
     }


/* ---- free memory for 8x8 block ---- */

free_double_matrix (c_block, 8, 8);

return;

} /* blockwise_remove_freq_2d */

/*--------------------------------------------------------------------------*/

void blockwise_quantisation_jpeg_2d

     (double  **c,          /* in and out: coefficients of the DCT */
      long    nx,           /* pixel number in x-direction */
      long    ny)           /* pixel number in y-direction */

/*
  quantises coefficients of 8x8 block
*/

{
long    i, j, k, l;       /* loop variables */
double  **c_block;        /* 8x8 block */


/* ---- allocate memory for 8x8 block ---- */

alloc_double_matrix (&c_block, 8, 8);


/* ---- scale coefficients blockwise ---- */

for (i=0; i<nx; i+=8)
 for (j=0; j<ny; j+=8)
     {
     /* copy 8x8 block */
     for (k=0; k<=7; k++)
      for (l=0; l<=7; l++)
          c_block[k][l] = c[i+k][j+l];

     /* scale coefficients of 8x8 block */
     jpeg_divide_block (c_block);

     /* round coefficients */
     round_block_coeff (c_block);

     /* rescale coefficients of 8x8 block */
     jpeg_multiply_block (c_block);

     /* copy back coefficients */
     for (k=0; k<=7; k++)
      for (l=0; l<=7; l++)
          c[i+k][j+l] = c_block[k][l];
     }


/* ---- free memory for 8x8 block ---- */

free_double_matrix (c_block, 8, 8);

return;

} /* blockwise_quantisation_jpeg_2d */

/*--------------------------------------------------------------------------*/

void blockwise_quantisation_equal_2d

     (double **c,          /* in and out: coefficients of the DCT */
      long   nx,           /* pixel number in x-direction */
      long   ny)           /* pixel number in y-direction */

/*
 quantises coefficients of 8x8 block
*/

{
long    i, j, k, l;        /* loop variables */
double  **c_block;         /* 8x8 block */


/* ---- allocate memory for 8x8 block ---- */

alloc_double_matrix (&c_block, 8, 8);


/* ---- scale coefficients blockwise ---- */

for (i=0; i<nx; i+=8)
 for (j=0; j<ny; j+=8)
     {
     /* copy 8x8 block */
     for (k=0; k<=7; k++)
      for (l=0; l<=7; l++)
          c_block[k][l] = c[i+k][j+l];

     /* scale coefficients of 8x8 block */
     equal_divide_block (c_block, 40);

     /* round coefficients */
     round_block_coeff (c_block);

     /* rescale coefficients of 8x8 block */
     equal_multiply_block (c_block, 40);

     /* copy back coefficients */
     for (k=0; k<=7; k++)
      for (l=0; l<=7; l++)
          c[i+k][j+l] = c_block[k][l];
     }


/* ---- free memory for 8x8 block ---- */

free_double_matrix (c_block, 8, 8);

return;

} /* blockwise_quantisation_equal_2d */

/*--------------------------------------------------------------------------*/

int main ()

{
char    in[80];               /* for reading data */
char    out1[80];             /* for reading data */
char    out2[80];             /* for reading data */
double  **f;                  /* image */
double  **u;                  /* shifted image */
double  **c;                  /* DCT coefficients */
double  **c0;                 /* shifted DCT coefficients */
long    nx, ny;               /* image size in x, y direction */
long    i, j;                 /* loop variables */
long    flag;                 /* processing flag */
double  max, min;             /* largest, smallest grey value */
double  mean;                 /* average grey value */
double  std;                  /* standard deviation */
char    comments[1600];       /* string for comments */

printf ("\n");
printf ("DISCRETE COSINE TRANSFORM\n\n");
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


/* ---- read input image (pgm format P5) ---- */

printf ("input image (pgm):                ");
read_string (in);
read_pgm_to_double (in, &nx, &ny, &f);  /* also allocates memory for f */

/* check if image can be devided in blocks of size 8x8 */
if ((nx % 8 != 0) || (ny % 8 != 0))
   {
   printf ("\n\n Image size does not allow decomposition! \n\n");
   return(0);
   }


/* ---- read parameters ---- */

printf ("spectrum image (pgm):             ");
read_string (out1);

printf ("output image (pgm):               ");
read_string (out2);

printf ("\n\nyou have the following options:      \n");
printf ("\n    (1) DCT/IDCT of the whole image   ");
printf ("\n    (2) DCT/IDCT of 8x8 blocks        ");
printf ("\n    (3) DCT/IDCT of the whole image   ");
printf ("\n        with removal of frequencies   ");
printf ("\n    (4) DCT/IDCT of 8x8 blocks        ");
printf ("\n        with removal of frequencies   ");
printf ("\n    (5) DCT/IDCT of 8x8 blocks        ");
printf ("\n        with equal quantisation       ");
printf ("\n    (6) DCT/IDCT of 8x8 blocks        ");
printf ("\n        with JPEG quantisation        \n");
printf ("\nchose the processing mode (1-6):      ");

read_long (&flag);
printf("\n\n");


/* ---- allocate memory ---- */

alloc_double_matrix (&c, nx+2, ny+2);
alloc_double_matrix (&c0, nx, ny);
alloc_double_matrix (&u, nx, ny);


/* ---- analyse input image ---- */

analyse_grey_double (f, nx, ny, &min, &max, &mean, &std);
printf ("input image:\n");
printf ("minimum:       %8.2lf \n", min);
printf ("maximum:       %8.2lf \n", max);
printf ("mean:          %8.2lf \n", mean);
printf ("standard dev.: %8.2lf \n\n", std);


/* ---- make copy of input image with shifted index ---- */

for (j=0; j<ny; j++)
 for (i=0; i<nx; i++)
     u[i][j] = f[i+1][j+1];


/* ---- process image ---- */

switch(flag)
  {
  case 1 :
    /* perform DCT and IDCT for the whole image */
    DCT_2d (u, c0, nx, ny);
    IDCT_2d (u, c0, nx, ny);
    break;
  case 2 :
    /* perform DCT and IDCT in 8x8 blocks */
    blockwise_DCT_2d (u, c0, nx, ny);
    blockwise_IDCT_2d (u, c0, nx, ny);
    break;
  case 3 :
    /* perform DCT and IDCT for the whole image */
    /* remove frequencies */
    DCT_2d (u, c0, nx, ny);
    remove_freq_2d (c0, nx, ny);
    IDCT_2d (u, c0, nx, ny);
    break;
  case 4 :
    /* perform DCT and IDCT in 8x8 blocks */
    /* remove frequencies */
    blockwise_DCT_2d (u, c0, nx, ny);
    blockwise_remove_freq_2d (c0, nx, ny);
    blockwise_IDCT_2d (u, c0, nx, ny);
    break;
  case 5 :
    /* perform DCT and IDCT in 8x8 blocks */
    /* and use equal quantisation */
    blockwise_DCT_2d (u, c0, nx, ny);
    blockwise_quantisation_equal_2d (c0, nx, ny);
    blockwise_IDCT_2d (u, c0, nx, ny);
    break;
  case 6 :
    /* perform DCT and IDCT in 8x8 blocks */
    /* and use JPEG quantisation */
    blockwise_DCT_2d (u, c0, nx, ny);
    blockwise_quantisation_jpeg_2d (c0, nx, ny);
    blockwise_IDCT_2d (u, c0, nx, ny);
    break;
  default :
    printf ("option (%ld) not available! \n\n\n",flag);
    return(0);
  }


/* ---- shift image and spectrum back to the original index ---- */

for (j=0; j<ny; j++)
 for (i=0; i<nx; i++)
     {
     c[i+1][j+1] = c0[i][j];
     f[i+1][j+1] = u[i][j];
     }


/* ---- compute logarithmic spectrum of c ---- */

for (j=1; j<=ny; j++)
 for (i=1; i<=nx; i++)
     c[i][j] = log (1.0 + fabs (c[i][j]));


/* ---- normalise spectrum of c ---- */

analyse_grey_double (c, nx, ny, &min, &max, &mean, &std);

if (max != 0.0)
   for (j=1; j<=ny; j++)
    for (i=1; i<=nx; i++)
        c[i][j] = c[i][j] * 255.0 / max;


/* ---- analyse filtered image ---- */

analyse_grey_double (f, nx, ny, &min, &max, &mean, &std);
printf ("filtered image:\n");
printf ("minimum:       %8.2lf \n", min);
printf ("maximum:       %8.2lf \n", max);
printf ("mean:          %8.2lf \n", mean);
printf ("standard dev.: %8.2lf \n\n", std);


/* ---- write output image 1 (spectrum) (pgm format P5) ---- */

/* generate comment string */
comments[0]='\0';
comment_line (comments, "# Discrete Cosine Transform (spectrum)\n");
comment_line (comments, "# menu option: %8ld\n", flag);

/* write image */
write_double_to_pgm (c, nx, ny, out1, comments);
printf ("output image %s successfully written\n", out1);


/* ---- write output image 2 (image) (pgm format P5) ---- */

/* generate comment string */
comments[0]='\0';
comment_line (comments, "# Discrete Cosine Transform (image)\n");
comment_line (comments, "# menu option: %8ld\n", flag);

/* write image */
write_double_to_pgm (f, nx, ny, out2, comments);
printf ("output image %s successfully written\n\n", out2);


/* ---- free memory  ---- */

free_double_matrix (f, nx+2, ny+2);
free_double_matrix (c, nx+2, ny+2);
free_double_matrix (c0, nx, ny);
free_double_matrix (u, nx, ny);

return(0);

}  /* main */
