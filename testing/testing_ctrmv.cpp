/*
    -- MAGMA (version 2.4.0) --
       Univ. of Tennessee, Knoxville
       Univ. of California, Berkeley
       Univ. of Colorado, Denver
       @date June 2018
 
       @generated from testing/testing_ztrmv.cpp, normal z -> c, Mon Jun 25 18:24:17 2018
       @author Chongxiao Cao
*/
// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// includes, project
#include "flops.h"
#include "magma_v2.h"
#include "magma_lapack.h"
#include "testings.h"


/* ////////////////////////////////////////////////////////////////////////////
   -- Testing ctrmv
*/
int main( int argc, char** argv)
{
    #ifdef HAVE_clBLAS
    #define dA(i_, j_)  dA, ((i_) + (j_)*ldda)
    #define dx(i_)      dx, ((i_))
    #else
    #define dA(i_, j_) (dA + (i_) + (j_)*ldda)
    #define dx(i_)     (dx + (i_))
    #endif
    
    TESTING_CHECK( magma_init() );
    magma_print_environment();

    real_Double_t   gflops, dev_perf, dev_time, cpu_perf, cpu_time;
    float          dev_error, work[1];
    magma_int_t N;
    magma_int_t Ak;
    magma_int_t sizeA;
    magma_int_t lda, ldda;
    magma_int_t ione     = 1;
    magma_int_t ISEED[4] = {0,0,0,1};
    
    magmaFloatComplex *hA, *hx, *hxdev;
    magmaFloatComplex_ptr dA, dx;
    magmaFloatComplex c_neg_one = MAGMA_C_NEG_ONE;
    int status = 0;
    
    magma_opts opts;
    opts.parse_opts( argc, argv );
    opts.lapack |= opts.check;  // check (-c) implies lapack (-l)
    
    // Allow 3*eps; complex needs 2*sqrt(2) factor; see Higham, 2002, sec. 3.6.
    float eps = lapackf77_slamch("E");
    float tol = 3 * eps;
    
    printf("%% If running lapack (option --lapack), CUBLAS error is computed\n"
           "%% relative to CPU BLAS result.\n\n");
    printf("%% uplo = %s, transA = %s, diag = %s \n",
           lapack_uplo_const(opts.uplo), lapack_trans_const(opts.transA),
           lapack_diag_const(opts.diag) );
    printf("%%   N   CUBLAS Gflop/s (ms)   CPU Gflop/s (ms)  CUBLAS error\n");
    printf("%%=================================================================\n");
    for( int itest = 0; itest < opts.ntest; ++itest ) {
        for( int iter = 0; iter < opts.niter; ++iter ) {
            N = opts.nsize[itest];
            gflops = FLOPS_CTRMM(opts.side, N, 1) / 1e9;

            lda = N;
            Ak = N;
            
            ldda = magma_roundup( lda, opts.align );  // multiple of 32 by default
            
            sizeA = lda*Ak;
            
            TESTING_CHECK( magma_cmalloc_cpu( &hA,    lda*Ak ));
            TESTING_CHECK( magma_cmalloc_cpu( &hx,    N      ));
            TESTING_CHECK( magma_cmalloc_cpu( &hxdev, N      ));
            
            TESTING_CHECK( magma_cmalloc( &dA, ldda*Ak ));
            TESTING_CHECK( magma_cmalloc( &dx, N       ));
            
            /* Initialize the matrices */
            lapackf77_clarnv( &ione, ISEED, &sizeA, hA );
            lapackf77_clarnv( &ione, ISEED, &N, hx );
            
            // for error checks
            float Anorm = lapackf77_clange( "F", &N, &N,    hA, &lda, work );
            float Xnorm = lapackf77_clange( "F", &N, &ione, hx, &N,   work );

            /* =====================================================================
               Performs operation using cuBLAS / clBLAS
               =================================================================== */
            magma_csetmatrix( Ak, Ak, hA, lda, dA(0,0), ldda, opts.queue );
            magma_csetvector( N, hx, 1, dx(0), 1, opts.queue );
            
            dev_time = magma_sync_wtime( opts.queue );
            magma_ctrmv( opts.uplo, opts.transA, opts.diag,
                         N,
                         dA(0,0), ldda,
                         dx(0),   1, opts.queue );
            dev_time = magma_sync_wtime( opts.queue ) - dev_time;
            dev_perf = gflops / dev_time;
            
            magma_cgetvector( N, dx(0), 1, hxdev, 1, opts.queue );
            
            /* =====================================================================
               Performs operation using CPU BLAS
               =================================================================== */
            if ( opts.lapack ) {
                cpu_time = magma_wtime();
                blasf77_ctrmv( lapack_uplo_const(opts.uplo), lapack_trans_const(opts.transA), lapack_diag_const(opts.diag),
                               &N,
                               hA, &lda,
                               hx, &ione );
                cpu_time = magma_wtime() - cpu_time;
                cpu_perf = gflops / cpu_time;
            }
            
            /* =====================================================================
               Check the result
               =================================================================== */
            if ( opts.lapack ) {
                // See testing_cgemm for formula. Here K = N.
                // initial C = 0, alpha = 1
                blasf77_caxpy( &N, &c_neg_one, hx, &ione, hxdev, &ione );
                dev_error = lapackf77_clange( "F", &N, &ione, hxdev, &N, work )
                            / (sqrt(float(N+2))*Anorm*Xnorm);
                
                bool okay = (dev_error < tol);
                status += ! okay;
                printf("%5lld   %7.2f (%7.2f)   %7.2f (%7.2f)    %8.2e   %s\n",
                       (long long) N,
                       dev_perf, 1000.*dev_time,
                       cpu_perf, 1000.*cpu_time,
                       dev_error, (okay ? "ok" : "failed"));
            }
            else {
                printf("%5lld   %7.2f (%7.2f)    ---   (  ---  )    ---     ---\n",
                       (long long) N,
                       dev_perf, 1000.*dev_time);
            }
            
            magma_free_cpu( hA );
            magma_free_cpu( hx );
            magma_free_cpu( hxdev );
            
            magma_free( dA );
            magma_free( dx );
            fflush( stdout );
        }
        if ( opts.niter > 1 ) {
            printf( "\n" );
        }
    }

    opts.cleanup();
    TESTING_CHECK( magma_finalize() );
    return status;
}
