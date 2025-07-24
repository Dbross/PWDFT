      subroutine dcfftb (n,c,wsave)
      double precision c(*), wsave(*)
C     wsave must be dimensioned at least 4*n+15 in the calling code (see FFTPACK docs)
c
      if (n .eq. 1) return
c
      iw1 = n+n+1
      iw2 = iw1+n+n
      call dcftb1 (n,c,wsave,wsave(iw1),wsave(iw2))
c
      return
      end
