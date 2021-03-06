#include <string.h>
#include <intrin.h>

enum {
 SIMD_FACTOR         = 4,
 COLS_PER_LOOP       = 5,
 COLS_STEPS_PER_CORE = 3,
 SIMD_ELEM_PEC_COL   = COLS_PER_LOOP*COLS_STEPS_PER_CORE,
 bb_nCols            = SIMD_ELEM_PEC_COL*SIMD_FACTOR,
 bb_nRows            = 57,
 cc_nRows            = 32,
};

struct noncblas_sgemm_prm_t {
  int   M;
  int   lda;
  int   ldc;
  float alpha;
  __m128 bb[SIMD_ELEM_PEC_COL*bb_nRows];
  __m128 cc[cc_nRows*SIMD_ELEM_PEC_COL];
};

static void fma128_noncblas_sgemm_core(
 const noncblas_sgemm_prm_t* pPrm,
 const float  *A,
 float *C)
{
  int lda = pPrm->lda;
  int ldc = pPrm->ldc;
  int m;
  for (m = 0; m < pPrm->M-1; A += lda*2, C += ldc*2, m += 2) {
    float* Crow0 = C;
    float* Crow1 = C+ldc;
    for (int n = 0; n < SIMD_ELEM_PEC_COL; n += COLS_PER_LOOP) {
      const __m128 *Bcol = &pPrm->bb[n];
      __m128 a0 = _mm_broadcast_ss(&A[0]);
      __m128 a1 = _mm_broadcast_ss(&A[lda]);
      __m128 b;
        b = Bcol[0];
      __m128 acc00 = _mm_mul_ps(a0, b);
      __m128 acc01 = _mm_mul_ps(a1, b);

        b = Bcol[1];
      __m128 acc10 = _mm_mul_ps(a0, b);
      __m128 acc11 = _mm_mul_ps(a1, b);

        b = Bcol[2];
      __m128 acc20 = _mm_mul_ps(a0, b);
      __m128 acc21 = _mm_mul_ps(a1, b);

        b = Bcol[3];
      __m128 acc30 = _mm_mul_ps(a0, b);
      __m128 acc31 = _mm_mul_ps(a1, b);

        b = Bcol[4];
      __m128 acc40 = _mm_mul_ps(a0, b);
      __m128 acc41 = _mm_mul_ps(a1, b);

      for (int k = 1; k < bb_nRows; k += 2) {
        Bcol += SIMD_ELEM_PEC_COL;
        a0 = _mm_broadcast_ss(&A[k]);
        a1 = _mm_broadcast_ss(&A[k+lda]);

        b = Bcol[0];
        acc00 = _mm_fmadd_ps(a0, b, acc00);
        acc01 = _mm_fmadd_ps(a1, b, acc01);

        b = Bcol[1];
        acc10 = _mm_fmadd_ps(a0, b, acc10);
        acc11 = _mm_fmadd_ps(a1, b, acc11);

        b = Bcol[2];
        acc20 = _mm_fmadd_ps(a0, b, acc20);
        acc21 = _mm_fmadd_ps(a1, b, acc21);

        b = Bcol[3];
        acc30 = _mm_fmadd_ps(a0, b, acc30);
        acc31 = _mm_fmadd_ps(a1, b, acc31);

        b = Bcol[4];
        acc40 = _mm_fmadd_ps(a0, b, acc40);
        acc41 = _mm_fmadd_ps(a1, b, acc41);

        Bcol += SIMD_ELEM_PEC_COL;
        a0 = _mm_broadcast_ss(&A[k+1]);
        a1 = _mm_broadcast_ss(&A[k+lda+1]);

        b = Bcol[0];
        acc00 = _mm_fmadd_ps(a0, b, acc00);
        acc01 = _mm_fmadd_ps(a1, b, acc01);

        b = Bcol[1];
        acc10 = _mm_fmadd_ps(a0, b, acc10);
        acc11 = _mm_fmadd_ps(a1, b, acc11);

        b = Bcol[2];
        acc20 = _mm_fmadd_ps(a0, b, acc20);
        acc21 = _mm_fmadd_ps(a1, b, acc21);

        b = Bcol[3];
        acc30 = _mm_fmadd_ps(a0, b, acc30);
        acc31 = _mm_fmadd_ps(a1, b, acc31);

        b = Bcol[4];
        acc40 = _mm_fmadd_ps(a0, b, acc40);
        acc41 = _mm_fmadd_ps(a1, b, acc41);
      }
      __m128 alpha_ps = _mm_broadcast_ss(&pPrm->alpha);

      _mm_storeu_ps(&Crow0[SIMD_FACTOR*0], _mm_fmadd_ps(acc00, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*0])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*1], _mm_fmadd_ps(acc10, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*1])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*2], _mm_fmadd_ps(acc20, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*2])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*3], _mm_fmadd_ps(acc30, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*3])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*4], _mm_fmadd_ps(acc40, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*4])));

      _mm_storeu_ps(&Crow1[SIMD_FACTOR*0], _mm_fmadd_ps(acc01, alpha_ps, _mm_loadu_ps(&Crow1[SIMD_FACTOR*0])));
      _mm_storeu_ps(&Crow1[SIMD_FACTOR*1], _mm_fmadd_ps(acc11, alpha_ps, _mm_loadu_ps(&Crow1[SIMD_FACTOR*1])));
      _mm_storeu_ps(&Crow1[SIMD_FACTOR*2], _mm_fmadd_ps(acc21, alpha_ps, _mm_loadu_ps(&Crow1[SIMD_FACTOR*2])));
      _mm_storeu_ps(&Crow1[SIMD_FACTOR*3], _mm_fmadd_ps(acc31, alpha_ps, _mm_loadu_ps(&Crow1[SIMD_FACTOR*3])));
      _mm_storeu_ps(&Crow1[SIMD_FACTOR*4], _mm_fmadd_ps(acc41, alpha_ps, _mm_loadu_ps(&Crow1[SIMD_FACTOR*4])));

      Crow0 += COLS_PER_LOOP*SIMD_FACTOR;
      Crow1 += COLS_PER_LOOP*SIMD_FACTOR;
    }
  }
  if (m < pPrm->M) {
    float* Crow0 = C;
    for (int n = 0; n < SIMD_ELEM_PEC_COL; n += COLS_PER_LOOP) {
      const __m128 *Bcol = &pPrm->bb[n];
      __m128 acc00 = _mm_setzero_ps();
      __m128 acc10 = _mm_setzero_ps();
      __m128 acc20 = _mm_setzero_ps();
      __m128 acc30 = _mm_setzero_ps();
      __m128 acc40 = _mm_setzero_ps();
      for (int k = 0; k < bb_nRows; ++k) {
        __m128 a0 = _mm_broadcast_ss(&A[k]);
        __m128 b;

        b = Bcol[0];
        acc00 = _mm_add_ps(acc00, _mm_mul_ps(a0, b));

        b = Bcol[1];
        acc10 = _mm_add_ps(acc10, _mm_mul_ps(a0, b));

        b = Bcol[2];
        acc20 = _mm_add_ps(acc20, _mm_mul_ps(a0, b));

        b = Bcol[3];
        acc30 = _mm_add_ps(acc30, _mm_mul_ps(a0, b));

        b = Bcol[4];
        acc40 = _mm_add_ps(acc40, _mm_mul_ps(a0, b));

        Bcol += SIMD_ELEM_PEC_COL;
      }
      __m128 alpha_ps = _mm_broadcast_ss(&pPrm->alpha);

      _mm_storeu_ps(&Crow0[SIMD_FACTOR*0], _mm_fmadd_ps(acc00, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*0])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*1], _mm_fmadd_ps(acc10, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*1])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*2], _mm_fmadd_ps(acc20, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*2])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*3], _mm_fmadd_ps(acc30, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*3])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*4], _mm_fmadd_ps(acc40, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*4])));

      Crow0 += COLS_PER_LOOP*SIMD_FACTOR;
    }
  }
}

static void fma128_noncblas_sgemm_core_bottomRows(
 const noncblas_sgemm_prm_t* pPrm,
 const float *A,
 float *C,
 int nRows)
{
  int lda = pPrm->lda;
  int ldc = pPrm->ldc;
  int m;
  for (m = 0; m < pPrm->M-1; A += lda*2, C += ldc*2, m += 2) {
    float* Crow0 = C;
    float* Crow1 = C+ldc;
    for (int n = 0; n < SIMD_ELEM_PEC_COL; n += COLS_PER_LOOP) {
      const __m128 *Bcol = &pPrm->bb[n];
      __m128 acc00 = _mm_setzero_ps();
      __m128 acc01 = _mm_setzero_ps();
      __m128 acc10 = _mm_setzero_ps();
      __m128 acc11 = _mm_setzero_ps();
      __m128 acc20 = _mm_setzero_ps();
      __m128 acc21 = _mm_setzero_ps();
      __m128 acc30 = _mm_setzero_ps();
      __m128 acc31 = _mm_setzero_ps();
      __m128 acc40 = _mm_setzero_ps();
      __m128 acc41 = _mm_setzero_ps();
      for (int k = 0; k < nRows; ++k) {
        __m128 a0 = _mm_broadcast_ss(&A[k]);
        __m128 a1 = _mm_broadcast_ss(&A[k+lda]);
        __m128 b;

        b = Bcol[0];
        acc00 = _mm_fmadd_ps(a0, b, acc00);
        acc01 = _mm_fmadd_ps(a1, b, acc01);

        b = Bcol[1];
        acc10 = _mm_fmadd_ps(a0, b, acc10);
        acc11 = _mm_fmadd_ps(a1, b, acc11);

        b = Bcol[2];
        acc20 = _mm_fmadd_ps(a0, b, acc20);
        acc21 = _mm_fmadd_ps(a1, b, acc21);

        b = Bcol[3];
        acc30 = _mm_fmadd_ps(a0, b, acc30);
        acc31 = _mm_fmadd_ps(a1, b, acc31);

        b = Bcol[4];
        acc40 = _mm_fmadd_ps(a0, b, acc40);
        acc41 = _mm_fmadd_ps(a1, b, acc41);

        Bcol += SIMD_ELEM_PEC_COL;
      }
      __m128 alpha_ps = _mm_broadcast_ss(&pPrm->alpha);

      _mm_storeu_ps(&Crow0[SIMD_FACTOR*0], _mm_fmadd_ps(acc00, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*0])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*1], _mm_fmadd_ps(acc10, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*1])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*2], _mm_fmadd_ps(acc20, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*2])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*3], _mm_fmadd_ps(acc30, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*3])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*4], _mm_fmadd_ps(acc40, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*4])));

      _mm_storeu_ps(&Crow1[SIMD_FACTOR*0], _mm_fmadd_ps(acc01, alpha_ps, _mm_loadu_ps(&Crow1[SIMD_FACTOR*0])));
      _mm_storeu_ps(&Crow1[SIMD_FACTOR*1], _mm_fmadd_ps(acc11, alpha_ps, _mm_loadu_ps(&Crow1[SIMD_FACTOR*1])));
      _mm_storeu_ps(&Crow1[SIMD_FACTOR*2], _mm_fmadd_ps(acc21, alpha_ps, _mm_loadu_ps(&Crow1[SIMD_FACTOR*2])));
      _mm_storeu_ps(&Crow1[SIMD_FACTOR*3], _mm_fmadd_ps(acc31, alpha_ps, _mm_loadu_ps(&Crow1[SIMD_FACTOR*3])));
      _mm_storeu_ps(&Crow1[SIMD_FACTOR*4], _mm_fmadd_ps(acc41, alpha_ps, _mm_loadu_ps(&Crow1[SIMD_FACTOR*4])));

      Crow0 += COLS_PER_LOOP*SIMD_FACTOR;
      Crow1 += COLS_PER_LOOP*SIMD_FACTOR;
    }
  }
  if (m < pPrm->M) {
    float* Crow0 = C;
    for (int n = 0; n < SIMD_ELEM_PEC_COL; n += COLS_PER_LOOP) {
      const __m128 *Bcol = &pPrm->bb[n];
      __m128 acc00 = _mm_setzero_ps();
      __m128 acc10 = _mm_setzero_ps();
      __m128 acc20 = _mm_setzero_ps();
      __m128 acc30 = _mm_setzero_ps();
      __m128 acc40 = _mm_setzero_ps();
      for (int k = 0; k < nRows; ++k) {
        __m128 a0 = _mm_broadcast_ss(&A[k]);
        __m128 b;

        b = Bcol[0];
        acc00 = _mm_add_ps(acc00, _mm_mul_ps(a0, b));

        b = Bcol[1];
        acc10 = _mm_add_ps(acc10, _mm_mul_ps(a0, b));

        b = Bcol[2];
        acc20 = _mm_add_ps(acc20, _mm_mul_ps(a0, b));

        b = Bcol[3];
        acc30 = _mm_add_ps(acc30, _mm_mul_ps(a0, b));

        b = Bcol[4];
        acc40 = _mm_add_ps(acc40, _mm_mul_ps(a0, b));

        Bcol += SIMD_ELEM_PEC_COL;
      }
      __m128 alpha_ps = _mm_broadcast_ss(&pPrm->alpha);

      _mm_storeu_ps(&Crow0[SIMD_FACTOR*0], _mm_fmadd_ps(acc00, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*0])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*1], _mm_fmadd_ps(acc10, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*1])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*2], _mm_fmadd_ps(acc20, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*2])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*3], _mm_fmadd_ps(acc30, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*3])));
      _mm_storeu_ps(&Crow0[SIMD_FACTOR*4], _mm_fmadd_ps(acc40, alpha_ps, _mm_loadu_ps(&Crow0[SIMD_FACTOR*4])));

      Crow0 += COLS_PER_LOOP*SIMD_FACTOR;
    }
  }
}

static void fma128_noncblas_sgemm_core_rightmostColumns(
 noncblas_sgemm_prm_t* pPrm,
 const float  *A,
 float *C,
 int nCols, // 0 < nCols <  bb_nCols
 int nRows) // nRows <= bb_nRows
{
  int lda = pPrm->lda;
  int ldc = pPrm->ldc;
  int ldcc = ((nCols-1)/(COLS_PER_LOOP*SIMD_FACTOR) + 1)*COLS_PER_LOOP;
  for (int m0 = 0; m0 < pPrm->M; m0 += cc_nRows) {
    int mLast = m0 + cc_nRows <= pPrm->M ? m0 + cc_nRows : pPrm->M;
    // calculate partial results and store in cc
    __m128* pCc = pPrm->cc;
    int mLastEv = mLast & (-2);
    for (int m = m0; m < mLastEv; A += lda*2, m += 2) {
      for (int n = 0; n < ldcc; n += COLS_PER_LOOP) {
        const __m128 *Bcol = &pPrm->bb[n];
        __m128 acc00 = _mm_setzero_ps();
        __m128 acc01 = _mm_setzero_ps();
        __m128 acc10 = _mm_setzero_ps();
        __m128 acc11 = _mm_setzero_ps();
        __m128 acc20 = _mm_setzero_ps();
        __m128 acc21 = _mm_setzero_ps();
        __m128 acc30 = _mm_setzero_ps();
        __m128 acc31 = _mm_setzero_ps();
        __m128 acc40 = _mm_setzero_ps();
        __m128 acc41 = _mm_setzero_ps();
        for (int k = 0; k < nRows; ++k) {
          __m128 a0 = _mm_broadcast_ss(&A[k]);
          __m128 a1 = _mm_broadcast_ss(&A[k + lda]);
          __m128 b;

          b = Bcol[0];
          acc00 = _mm_fmadd_ps(a0, b, acc00);
          acc01 = _mm_fmadd_ps(a1, b, acc01);

          b = Bcol[1];
          acc10 = _mm_fmadd_ps(a0, b, acc10);
          acc11 = _mm_fmadd_ps(a1, b, acc11);

          b = Bcol[2];
          acc20 = _mm_fmadd_ps(a0, b, acc20);
          acc21 = _mm_fmadd_ps(a1, b, acc21);

          b = Bcol[3];
          acc30 = _mm_fmadd_ps(a0, b, acc30);
          acc31 = _mm_fmadd_ps(a1, b, acc31);

          b = Bcol[4];
          acc40 = _mm_fmadd_ps(a0, b, acc40);
          acc41 = _mm_fmadd_ps(a1, b, acc41);

          Bcol += SIMD_ELEM_PEC_COL;
        }
        __m128 alpha_ps = _mm_broadcast_ss(&pPrm->alpha);

        pCc[0]      = _mm_mul_ps(acc00, alpha_ps);
        pCc[1]      = _mm_mul_ps(acc10, alpha_ps);
        pCc[2]      = _mm_mul_ps(acc20, alpha_ps);
        pCc[3]      = _mm_mul_ps(acc30, alpha_ps);
        pCc[4]      = _mm_mul_ps(acc40, alpha_ps);
        pCc[ldcc+0] = _mm_mul_ps(acc01, alpha_ps);
        pCc[ldcc+1] = _mm_mul_ps(acc11, alpha_ps);
        pCc[ldcc+2] = _mm_mul_ps(acc21, alpha_ps);
        pCc[ldcc+3] = _mm_mul_ps(acc31, alpha_ps);
        pCc[ldcc+4] = _mm_mul_ps(acc41, alpha_ps);
        pCc += COLS_PER_LOOP;
      }
      pCc += ldcc;
    }
    if ((mLast & 1) != 0) {
      // last row of A
      for (int n = 0; n < ldcc; n += COLS_PER_LOOP) {
        const __m128 *Bcol = &pPrm->bb[n];
        __m128 acc00 = _mm_setzero_ps();
        __m128 acc10 = _mm_setzero_ps();
        __m128 acc20 = _mm_setzero_ps();
        __m128 acc30 = _mm_setzero_ps();
        __m128 acc40 = _mm_setzero_ps();
        for (int k = 0; k < nRows; ++k) {
          __m128 a0 = _mm_broadcast_ss(&A[k]);
          __m128 b;

          b = Bcol[0];
          acc00 = _mm_fmadd_ps(a0, b, acc00);

          b = Bcol[1];
          acc10 = _mm_fmadd_ps(a0, b, acc10);

          b = Bcol[2];
          acc20 = _mm_fmadd_ps(a0, b, acc20);

          b = Bcol[3];
          acc30 = _mm_fmadd_ps(a0, b, acc30);

          b = Bcol[4];
          acc40 = _mm_fmadd_ps(a0, b, acc40);

          Bcol += SIMD_ELEM_PEC_COL;
        }
        __m128 alpha_ps = _mm_broadcast_ss(&pPrm->alpha);

        pCc[0] = _mm_mul_ps(acc00, alpha_ps);
        pCc[1] = _mm_mul_ps(acc10, alpha_ps);
        pCc[2] = _mm_mul_ps(acc20, alpha_ps);
        pCc[3] = _mm_mul_ps(acc30, alpha_ps);
        pCc[4] = _mm_mul_ps(acc40, alpha_ps);
        pCc += COLS_PER_LOOP;
      }
    }

    // add partial result in cc to C
    pCc = pPrm->cc;
    for (int m = 0; m < mLast-m0; C += ldc, pCc += ldcc, ++m) {
      const float* res = (const float*)pCc;
      for (int n = 0; n < nCols; ++n) 
        C[n] += res[n];
    }
  }
}


static void fma128_noncblas_sgemm_multC(
 int M, int N,
 float beta, 
 float *C, int ldc)
{
  if (beta != 0) {
    for (int m = 0; m < M; ++m) {
      for (int n = 0; n < N; ++n)
        C[n] *= beta;
      C += ldc;
    }
  } else {
    for (int m = 0; m < M; ++m) {
      for (int n = 0; n < N; ++n)
        C[n] = 0;
      C += ldc;
    }
  }
}

void fma128_noncblas_sgemm(
 int M, int N, int K, 
 float alpha, 
 const float *A, int lda, 
 const float *B, int ldb,
 float beta, 
 float *C, int ldc)
{
  fma128_noncblas_sgemm_multC(M, N, beta, C, ldc);

  noncblas_sgemm_prm_t prm;
  prm.M      = M;
  prm.lda   = lda;
  prm.ldc   = ldc;
  prm.alpha = alpha;

  int n_Rsteps = K / bb_nRows;
  int n_Csteps = N / bb_nCols;
  int row = 0;
  for (int ri = 0; ri < n_Rsteps; ++ri) {
    int col = 0;
    for (int ci = 0; ci < n_Csteps; ++ci) {
      // process full rectangles
      const float* bSrc = &B[row*ldb + col];
      for (int i = 0; i < bb_nRows; ++i) {
        memcpy(&prm.bb[SIMD_ELEM_PEC_COL*i], bSrc, bb_nCols*sizeof(*B));
        bSrc += ldb;
      }
      fma128_noncblas_sgemm_core(&prm, &A[row], &C[col]);
      col += bb_nCols;
    }
    if (col < N) {
      // process rightmost rectangle of the full-height band
      const float* bSrc = &B[row*ldb + col];
      for (int i = 0; i < bb_nRows; ++i) {
        memcpy(&prm.bb[SIMD_ELEM_PEC_COL*i], bSrc, (N-col)*sizeof(*B));
        bSrc += ldb;
      }
      fma128_noncblas_sgemm_core_rightmostColumns(&prm, &A[row], &C[col], N-col, bb_nRows);
    }
    row += bb_nRows;
  }
  if (row < K) {
    // bottom band
    int col = 0;
    for (int ci = 0; ci < n_Csteps; ++ci) {
      // process full-width rectangles
      const float* bSrc = &B[row*ldb + col];
      for (int i = 0; i < K-row; ++i) {
        memcpy(&prm.bb[SIMD_ELEM_PEC_COL*i], bSrc, bb_nCols*sizeof(*B));
        bSrc += ldb;
      }
      fma128_noncblas_sgemm_core_bottomRows(&prm, &A[row], &C[col], K-row);
      col += bb_nCols;
    }
    if (col < N) {
      // process bottom-right corner rectangle
      const float* bSrc = &B[row*ldb + col];
      for (int i = 0; i < K-row; ++i) {
        memcpy(&prm.bb[SIMD_ELEM_PEC_COL*i], bSrc, (N-col)*sizeof(*B));
        bSrc += ldb;
      }
      fma128_noncblas_sgemm_core_rightmostColumns(&prm, &A[row], &C[col], N-col, K-row);
    }
  }
}
