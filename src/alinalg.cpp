#include "alinalg.hpp"
#include <iostream>
#include <vector>
#include <cstring>

#define PETSCOPTION_STR_LEN 30

namespace acfd {

template <int nvars>
StatusCode setupSystemMatrix(const UMesh2dh *const m, Mat *const A)
{
	StatusCode ierr = 0;
	ierr = MatCreate(PETSC_COMM_WORLD, A); CHKERRQ(ierr);
	ierr = MatSetType(*A, MATMPIBAIJ); CHKERRQ(ierr);
	ierr = MatSetFromOptions(*A); CHKERRQ(ierr);

	ierr = MatSetSizes(*A, PETSC_DECIDE, PETSC_DECIDE, m->gnelem()*nvars, m->gnelem()*nvars); 
	CHKERRQ(ierr);
	ierr = MatSetBlockSize(*A, nvars); CHKERRQ(ierr);
	
	std::vector<PetscInt> dnnz(m->gnelem());
	for(a_int iel = 0; iel < m->gnelem(); iel++)
	{
		dnnz[iel] = m->gnfael(iel)+1;
	}
	ierr = MatSeqBAIJSetPreallocation(*A, nvars, 0, &dnnz[0]); CHKERRQ(ierr);
	ierr = MatMPIBAIJSetPreallocation(*A, nvars, 0, &dnnz[0], 1, NULL); CHKERRQ(ierr);

	dnnz.resize(m->gnelem()*nvars);
	for(a_int iel = 0; iel < m->gnelem(); iel++)
	{
		for(int i = 0; i < nvars; i++) {
			dnnz[iel*nvars+i] = (m->gnfael(iel)+1)*nvars;
		}
	}
	
	ierr = MatSeqAIJSetPreallocation(*A, 0, &dnnz[0]); CHKERRQ(ierr);
	ierr = MatMPIAIJSetPreallocation(*A, 0, &dnnz[0], nvars, NULL); CHKERRQ(ierr);

	ierr = MatSetOption(*A, MAT_USE_HASH_TABLE, PETSC_TRUE); CHKERRQ(ierr);

	return ierr;
}

template StatusCode setupSystemMatrix<NVARS>(const UMesh2dh *const m, Mat *const A);
template StatusCode setupSystemMatrix<1>(const UMesh2dh *const m, Mat *const A);

template<int nvars>
MatrixFreeSpatialJacobian<nvars>::MatrixFreeSpatialJacobian(const Spatial<nvars> *const space, 
		const a_real epsilon)
	: spatial{space}, eps{epsilon}
{
	// TODO: Setup aux vector
}

template<int nvars>
StatusCode MatrixFreeSpatialJacobian<nvars>::apply(const Vec x, Vec y) const
{
	StatusCode ierr = 0;

	// TODO
	
	return ierr;
}

template class MatrixFreeSpatialJacobian<NVARS>;
template class MatrixFreeSpatialJacobian<1>;

}
