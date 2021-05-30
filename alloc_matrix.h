#include <stdio.h>
#include <stdlib.h>

#define SUCCESS             0
#define MALLOC_ERROR        1

/******************************************************************************/
/** alloc_matrix(r,c,e, &Mstorage, &M, &err)
 *  If &err is SUCCESS, on return it allocated storage for two arrays in
 *  the heap. Mstorage is a linear array large enough to hold the elements of
 *  an r by c 2D matrix whose elements are e bytes long. The other, M, is a 2D
 *  matrix such that M[i][j] is the element in row i and column j.
 */
void alloc_matrix(
        int     nrows,          /* number of rows in matrix                   */
        int     ncols,          /* number of columns in matrix                */
        size_t  element_size,   /* number of bytes per matrix element         */
        void  **matrix_storage, /* address of linear storage array for matrix */
        void ***matrix,         /* address of start of matrix                 */
        int    *errvalue        /* return code for error, if any              */
        );



