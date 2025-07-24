      subroutine drfftf (n,r,wsave)
      double precision r(*), wsave(*)
C     wsave must be dimensioned at least 2*n+15 in the calling code (see FFTPACK docs)
c
      if (n .eq. 1) return
c
      call drftf1 (n,r,wsave,wsave(n+1),wsave(2*n+1))
c
      return
      end
