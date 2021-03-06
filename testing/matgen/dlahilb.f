      SUBROUTINE DLAHILB( N, NRHS, A, LDA, X, LDX, B, LDB, WORK, INFO)
!
!  -- LAPACK auxiliary test routine (version 3.2.2) --
!     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
!     Courant Institute, Argonne National Lab, and Rice University
*     June 2010
!
!     David Vu <dtv@cs.berkeley.edu>      
!     Yozo Hida <yozo@cs.berkeley.edu>      
!     Jason Riedy <ejr@cs.berkeley.edu>
!     D. Halligan <dhalligan@berkeley.edu>
!
      IMPLICIT NONE
!     .. Scalar Arguments ..
      INTEGER N, NRHS, LDA, LDX, LDB, INFO
!     .. Array Arguments ..
      DOUBLE PRECISION A(LDA, N), X(LDX, NRHS), B(LDB, NRHS), WORK(N)
!     ..
!
!  Purpose
!  =======
!
!  DLAHILB generates an N by N scaled Hilbert matrix in A along with
!  NRHS right-hand sides in B and solutions in X such that A*X=B.
!
!  The Hilbert matrix is scaled by M = LCM(1, 2, ..., 2*N-1) so that all
!  entries are integers.  The right-hand sides are the first NRHS 
!  columns of M * the identity matrix, and the solutions are the 
!  first NRHS columns of the inverse Hilbert matrix.
!
!  The condition number of the Hilbert matrix grows exponentially with
!  its size, roughly as O(e ** (3.5*N)).  Additionally, the inverse
!  Hilbert matrices beyond a relatively small dimension cannot be
!  generated exactly without extra precision.  Precision is exhausted
!  when the largest entry in the inverse Hilbert matrix is greater than
!  2 to the power of the number of bits in the fraction of the data type
!  used plus one, which is 24 for single precision.  
!
!  In single, the generated solution is exact for N <= 6 and has
!  small componentwise error for 7 <= N <= 11.
!
!  Arguments
!  =========
!
!  N       (input) INTEGER
!          The dimension of the matrix A.
!      
!  NRHS    (input) INTEGER
!          The requested number of right-hand sides.
!
!  A       (output) DOUBLE PRECISION array, dimension (LDA, N)
!          The generated scaled Hilbert matrix.
!
!  LDA     (input) INTEGER
!          The leading dimension of the array A.  LDA >= N.
!
!  X       (output) DOUBLE PRECISION array, dimension (LDX, NRHS)
!          The generated exact solutions.  Currently, the first NRHS
!          columns of the inverse Hilbert matrix.
!
!  LDX     (input) INTEGER
!          The leading dimension of the array X.  LDX >= N.
!
!  B       (output) DOUBLE PRECISION array, dimension (LDB, NRHS)
!          The generated right-hand sides.  Currently, the first NRHS
!          columns of LCM(1, 2, ..., 2*N-1) * the identity matrix.
!
!  LDB     (input) INTEGER
!          The leading dimension of the array B.  LDB >= N.
!
!  WORK    (workspace) DOUBLE PRECISION array, dimension (N)
!
!
!  INFO    (output) INTEGER
!          = 0: successful exit
!          = 1: N is too large; the data is still generated but may not
!               be not exact.
!          < 0: if INFO = -i, the i-th argument had an illegal value
!
!  =====================================================================

!     .. Local Scalars ..
      INTEGER TM, TI, R
      INTEGER M
      INTEGER I, J
      COMPLEX*16 TMP

!     .. Parameters ..
!     NMAX_EXACT   the largest dimension where the generated data is
!                  exact.
!     NMAX_APPROX  the largest dimension where the generated data has
!                  a small componentwise relative error.
      INTEGER NMAX_EXACT, NMAX_APPROX
      PARAMETER (NMAX_EXACT = 6, NMAX_APPROX = 11)

!     ..
!     .. External Functions
      EXTERNAL DLASET
      INTRINSIC DBLE
!     ..
!     .. Executable Statements ..
!
!     Test the input arguments
!
      INFO = 0
      IF (N .LT. 0 .OR. N .GT. NMAX_APPROX) THEN
         INFO = -1
      ELSE IF (NRHS .LT. 0) THEN
         INFO = -2
      ELSE IF (LDA .LT. N) THEN
         INFO = -4
      ELSE IF (LDX .LT. N) THEN
         INFO = -6
      ELSE IF (LDB .LT. N) THEN
         INFO = -8
      END IF
      IF (INFO .LT. 0) THEN
         CALL XERBLA('DLAHILB', -INFO)
         RETURN
      END IF
      IF (N .GT. NMAX_EXACT) THEN
         INFO = 1
      END IF

!     Compute M = the LCM of the integers [1, 2*N-1].  The largest
!     reasonable N is small enough that integers suffice (up to N = 11).
      M = 1
      DO I = 2, (2*N-1)
         TM = M
         TI = I
         R = MOD(TM, TI)
         DO WHILE (R .NE. 0)
            TM = TI
            TI = R
            R = MOD(TM, TI)
         END DO
         M = (M / TI) * I
      END DO

!     Generate the scaled Hilbert matrix in A
      DO J = 1, N
         DO I = 1, N
            A(I, J) = DBLE(M) / (I + J - 1)
         END DO
      END DO

!     Generate matrix B as simply the first NRHS columns of M * the
!     identity.
      TMP = DBLE(M)
      CALL DLASET('Full', N, NRHS, 0.0D+0, TMP, B, LDB)

!     Generate the true solutions in X.  Because B = the first NRHS
!     columns of M*I, the true solutions are just the first NRHS columns
!     of the inverse Hilbert matrix.
      WORK(1) = N
      DO J = 2, N
         WORK(J) = (  ( (WORK(J-1)/(J-1)) * (J-1 - N) ) /(J-1)  )
     $        * (N +J -1)
      END DO
      
      DO J = 1, NRHS
         DO I = 1, N
            X(I, J) = (WORK(I)*WORK(J)) / (I + J - 1)
         END DO
      END DO

      END
