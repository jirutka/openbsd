! { dg-do compile }

      FUNCTION NEW_LOCK()
      USE OMP_LIB       ! or INCLUDE "omp_lib.h"
        INTEGER(OMP_LOCK_KIND), POINTER :: NEW_LOCK
!$OMP SINGLE
          ALLOCATE(NEW_LOCK)
          CALL OMP_INIT_LOCK(NEW_LOCK)
!$OMP END SINGLE COPYPRIVATE(NEW_LOCK)
      END FUNCTION NEW_LOCK
