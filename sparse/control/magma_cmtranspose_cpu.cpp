/*
    -- MAGMA (version 2.4.0) --
       Univ. of Tennessee, Knoxville
       Univ. of California, Berkeley
       Univ. of Colorado, Denver
       @date June 2018

       @generated from sparse/control/magma_zmtranspose_cpu.cpp, normal z -> c, Mon Jun 25 18:24:27 2018
       @author Hartwig Anzt

*/
#include <cstdlib>
#include "magmasparse_internal.h"



/**
 * op(from[i], to[i]);
 */
template <typename Operator>
inline magma_int_t
magma_c_mtrans_template(
    magma_c_matrix A, 
    magma_c_matrix *B,
    Operator op,
    magma_queue_t queue )
{
    magma_int_t info = 0;
    
    magma_index_t *linked_list;
    magma_index_t *row_ptr;
    magma_index_t *last_rowel;
    
    // magma_int_t num_threads;
    // magma_int_t el_per_block;
    magma_cmfree( B, queue );
    B->ownership = MagmaTrue;
    
    B->storage_type = A.storage_type;
    B->memory_location = A.memory_location;
    
    B->num_rows = A.num_rows;
    B->num_cols = A.num_cols;
    B->nnz      = A.nnz;
    
    CHECK( magma_index_malloc_cpu( &linked_list, A.nnz ));
    CHECK( magma_index_malloc_cpu( &row_ptr, A.num_rows ));
    CHECK( magma_index_malloc_cpu( &last_rowel, A.num_rows ));
    CHECK( magma_index_malloc_cpu( &B->row, A.num_rows+1 ));
    CHECK( magma_index_malloc_cpu( &B->rowidx, A.nnz ));
    CHECK( magma_index_malloc_cpu( &B->col, A.nnz ));
    CHECK( magma_cmalloc_cpu( &B->val, A.nnz ) );
    
    magma_free_cpu( A.rowidx );
    
    CHECK( magma_cmatrix_addrowindex(&A, queue) );
    
    //#pragma omp parallel
    // {
    //     num_threads = omp_get_max_threads();
    // }
    
    //#pragma omp parallel for
    for( magma_int_t i=0; i<A.num_rows; i++ ){
        row_ptr[i] = -1;
    }
    //#pragma omp parallel for
    for( magma_int_t i=0; i<A.num_rows+1; i++ ){
        B->row[i] = 0;
    }
    
    //el_per_block = magma_ceildiv( A.num_rows, num_threads );

    //#pragma omp parallel
    {
        // magma_int_t id = omp_get_thread_num();
        for(magma_int_t i=0; i<A.nnz; i++ ){
            magma_index_t row = A.col[ i ];
            //if( (row < (id+1)*el_per_block) && (row >=(id)*el_per_block)  ){
            {
                if( row_ptr[row] == -1 ){
                    row_ptr[ row ] = i;
                    linked_list[ i ] = 0;
                    last_rowel[ row ] = i;
                } else {
                    linked_list[ last_rowel[ row ] ] = i;
                    linked_list[ i ] = 0;
                    last_rowel[ row ] = i;
                }
                B->row[row+1] = B->row[row+1] + 1;
            }
        }
    }
    
    // new rowptr
    B->row[0]=0;   
    magma_cmatrix_createrowptr( B->num_rows, B->row, queue );
    

    assert( B->row[B->num_rows] == A.nnz );
    
    //#pragma omp parallel for
    for( magma_int_t row=0; row<A.num_rows; row++){
        magma_int_t el = row_ptr[row];
        if( el>-1 ) {
            for( magma_int_t i=B->row[row]; i<B->row[row+1]; i++ ){
                op(A.val[el], B->val[i]);
                B->col[i] = A.rowidx[el];
                el = linked_list[el];
            }
        }
    }
    
cleanup:
    magma_free_cpu( row_ptr );
    magma_free_cpu( last_rowel );
    magma_free_cpu( linked_list );
    magma_free_cpu( A.rowidx );
    return info;
}


inline void cpy(const magmaFloatComplex &from, magmaFloatComplex &to) { to = from; }

/**
    Purpose
    -------

    Generates a transpose of A on the CPU.

    Arguments
    ---------

    @param[in]
    A           magma_c_matrix
                input matrix (CSR)

    @param[out]
    B           magma_c_matrix*
                output matrix (CSR)
    @param[in]
    queue       magma_queue_t
                Queue to execute in.

    @ingroup magmasparse_caux
    ********************************************************************/
extern "C" magma_int_t
magma_cmtranspose_cpu(
    magma_c_matrix A, 
    magma_c_matrix *B,
    magma_queue_t queue){
    
    magma_int_t info = 0;
    
    CHECK( magma_c_mtrans_template(A, B, cpy, queue) );
    
cleanup:
    return info;
}


inline void conjop(const magmaFloatComplex &from, magmaFloatComplex &to) { to = MAGMA_C_CONJ(from); }

/**
    Purpose
    -------

    Generates a transpose conjugate of A on the CPU.

    Arguments
    ---------

    @param[in]
    A           magma_c_matrix
                input matrix (CSR)

    @param[out]
    B           magma_c_matrix*
                output matrix (CSR)
    @param[in]
    queue       magma_queue_t
                Queue to execute in.

    @ingroup magmasparse_caux
    ********************************************************************/
extern "C" magma_int_t
magma_cmtransposeconj_cpu(
    magma_c_matrix A, 
    magma_c_matrix *B,
    magma_queue_t queue){
    
    magma_int_t info = 0;
    
    CHECK( magma_c_mtrans_template(A, B, conjop, queue) );
    
cleanup:
    return info;
}


inline void pass(const magmaFloatComplex &from, magmaFloatComplex &to) { }

/**
    Purpose
    -------

    Generates a transpose of the nonzero pattern of A on the CPU.

    Arguments
    ---------

    @param[in]
    A           magma_c_matrix
                input matrix (CSR)

    @param[out]
    B           magma_c_matrix*
                output matrix (CSR)
    @param[in]
    queue       magma_queue_t
                Queue to execute in.

    @ingroup magmasparse_caux
    ********************************************************************/
extern "C" magma_int_t
magma_cmtransposestruct_cpu(
    magma_c_matrix A, 
    magma_c_matrix *B,
    magma_queue_t queue){
    
    magma_int_t info = 0;
    
    CHECK( magma_c_mtrans_template(A, B, pass, queue) );
    
cleanup:
    return info;
}


inline void absval(const magmaFloatComplex &from, magmaFloatComplex &to) { to = MAGMA_C_MAKE(MAGMA_C_ABS(from), 0.0 ); }

/**
    Purpose
    -------

    Generates a transpose with absolute values of A on the CPU.

    Arguments
    ---------

    @param[in]
    A           magma_c_matrix
                input matrix (CSR)

    @param[out]
    B           magma_c_matrix*
                output matrix (CSR)
    @param[in]
    queue       magma_queue_t
                Queue to execute in.

    @ingroup magmasparse_caux
    ********************************************************************/
extern "C" magma_int_t
magma_cmtransposeabs_cpu(
    magma_c_matrix A, 
    magma_c_matrix *B,
    magma_queue_t queue){
    
    magma_int_t info = 0;
    
    CHECK( magma_c_mtrans_template(A, B, absval, queue) );
    
cleanup:
    return info;
}

