#ifndef KRONMULT3_HPP
#define  KRONMULT3_HPP 1

#include "kroncommon.hpp"

#include "kgemm_nt.hpp"
#include "kronmult2.hpp"

//  -------------------------------------------
//  device function to evaluate
//  Y = kron(A1,...,A3)*X as
//  W(:,k) = X(:,k) * transpose(A1), k=1:nvec
//  Y = kron(A2,..,A3) * W
//  -------------------------------------------
template<typename T>
DEVICE_FUNCTION
void kronmult3( int const n, 
                int const nvec,
                T   const A1_[],
                T   const A2_[],
                T   const A3_[],
                T   X_[],
                T   Y_[],
                T   W_[],
	        int const lda_in = 0 )
// -----------------
// note A1 is n by n
//      X is (n^3 by nvec)
// -----------------
{
    int const lda = (lda_in == 0) ? n : lda_in;
    int const n2 = n*n;
    int const n3 = n*n2;

    int const ldX = n3;
    int const ldW = n3;


    auto X = [&] (int const i,
                  int const j) -> T& {
            return( X_[ indx2f(i,j,ldX) ] );
    };

    auto W = [&] (int const i,
                  int const j) -> T& {
            return( W_[ indx2f(i,j,ldW) ] );
    };

    for(int i=1; i <= nvec; i++) {
            T *Xi_ = &( X(1, i) );
            T *Wi_ = &( W(1, i) );
            int const ldXi = n2;
            int const ldWi = n2;
            // ----------------------------
            // Xi viewed as (n^2 by n) array
            // Wi viewed as (n^2 by n) array
            // ----------------------------

            auto Xi = [&] (int const i,
                           int const j) -> T& {
                    return(  Xi_[ indx2f(i,j,ldXi) ] );
            };

            auto Wi = [&] (int const i,
                           int const j) -> T& {
                    return(  Wi_[ indx2f(i,j,ldWi) ] );
            };
            // --------------------------------------------------------
            // Wi(1:n^2, 1:n) = Xi(1:n^2, 1:n) * transpose(A1(1:n,1:n))
            // --------------------------------------------------------
            int const mm = n2;
            int const nn = n;
            int const kk = n;
            T const alpha = 1;
            T const beta = 0;

            T const * const  Ap = &(Xi(1,1));
            T const * const  Bp = A1_;
            T       * const  Cp = &(Wi(1,1));

            int const ld1 = ldXi;
            int const ld2 = lda;
            int const ld3 = ldWi;

            kgemm_nt( mm,nn,kk, 
                      alpha, Ap, ld1,
                             Bp, ld2,
                      beta,  Cp, ld3 );
    };

    int const next_nvec = nvec * n;

    // --------------------------------
    // note now X_ is used as workspace
    // --------------------------------

    SYNCTHREADS;

    {
    kronmult2( n, next_nvec, 
               A2_, A3_, 
               W_,  Y_,   X_, lda );
    }

}


#endif
