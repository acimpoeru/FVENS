#include <iostream>
#include <fstream>
#include "alinalg.hpp"
#include "aoutput.hpp"
#include "aodesolver.hpp"

using namespace amat;
using namespace std;
using namespace acfd;

int main(int argc, char* argv[])
{
	StatusCode ierr = 0;
	const char help[] = "Finite volume solver for the heat equation.\n\
		Arguments needed: FVENS control file and PETSc options file with -options_file,\n";

	ierr = PetscInitialize(&argc,&argv,NULL,help); CHKERRQ(ierr);

	if(argc < 2)
	{
		cout << "Please give a control file name.\n";
		return -1;
	}

	// Read control file
	ifstream control(argv[1]);

	string dum, meshfile, outf, logfile, visflux, reconst, linsolver, timesteptype, prec, lognresstr;
	double initcfl, endcfl, tolerance, lintol, firstcfl, firsttolerance, diffcoeff, bvalue;
	int maxiter, linmaxiterstart, linmaxiterend, rampstart, rampend, firstmaxiter, restart_vecs;
	short inittype, usestarter;
	short nbuildsweeps, napplysweeps;
	char mattype;
	bool lognres;

	control >> dum; control >> meshfile;
	control >> dum; control >> outf;
	control >> dum; control >> logfile;
	control >> dum; control >> lognresstr;
	control >> dum; control >> diffcoeff;
	control >> dum; control >> bvalue;
	control >> dum; control >> inittype;
	control >> dum; control >> visflux;
	control >> dum; control >> reconst;
	control >> dum; control >> timesteptype;
	control >> dum; control >> initcfl;
	control >> dum; control >> endcfl;
	control >> dum; control >> rampstart;
	control >> dum; control >> rampend;
	control >> dum; control >> tolerance;
	control >> dum; control >> maxiter;
	control >> dum; control >> usestarter;
	control >> dum; control >> firstcfl;
	control >> dum; control >> firsttolerance;
	control >> dum; control >> firstmaxiter;
	if(timesteptype == "IMPLICIT") {
		control >> dum;
		control >> dum; control >> mattype;
		control >> dum; control >> linsolver;
		control >> dum; control >> lintol;
		control >> dum; control >> linmaxiterstart;
		control >> dum; control >> linmaxiterend;
		control >> dum; control >> restart_vecs;
		control >> dum; control >> prec;
		control >> dum; control >> nbuildsweeps;
		control >> dum; control >> napplysweeps;
	}
	else {
		control >> dum;
		std::string residualsmoothingstr;
		control >> dum; control >> residualsmoothingstr;
		if(residualsmoothingstr == "YES")
			std::cout << "! Residual smoothing is not supported.\n";
	}
	control.close();

	if(lognresstr == "YES")
		lognres = true;
	else
		lognres = false;
	
	// rhs and exact soln
	
	std::function<void(const a_real *const, const a_real, const a_real *const, a_real *const)> rhs 
		= [diffcoeff](const a_real *const r, const a_real t, const a_real *const u, 
				a_real *const sourceterm)
		{ 
			sourceterm[0] = diffcoeff*8.0*PI*PI*sin(2*PI*r[0])*sin(2*PI*r[1]); 
		};
	
	auto uexact = [](const a_real *const r)->a_real { return sin(2*PI*r[0])*sin(2*PI*r[1]); };

	a_real err = 0;

	// Set up mesh

	UMesh2dh m;
	m.readGmsh2(meshfile);
	m.compute_topological();
	m.compute_areas();
	m.compute_jacobians();
	m.compute_face_data();

	// set up problem
	
	std::cout << "Setting up spatial scheme.\n";
	
	Diffusion<1>* prob = nullptr;
	Diffusion<1>* startprob = nullptr;
	if(visflux == "MODIFIEDAVERAGE") {
		prob = new DiffusionMA<1>(&m, diffcoeff, bvalue, rhs, reconst);
		startprob = new DiffusionMA<1>(&m, diffcoeff, bvalue, rhs, "NONE");
	}
	else {
		std::cout << " ! Viscous scheme not available!\n";
		std::abort();
	}

	Array2d<a_real> outputarr, dummy;
	
	std::cout << "\n***\n";
	
	// solution vector
	Vec u;

	// Initialize Jacobian for implicit schemes
	Mat M;
	ierr = setupSystemMatrix<1>(&m, &M); CHKERRQ(ierr);
	ierr = MatCreateVecs(M, &u, NULL); CHKERRQ(ierr);

	// initialize solver
	KSP ksp;
	ierr = KSPSetOperators(ksp, M, M); CHKERRQ(ierr);
	ierr = KSPSetUp(ksp); CHKERRQ(ierr);
	ierr = KSPSetFromOptions(ksp); CHKERRQ(ierr);

	const SteadySolverConfig tconf {
		lognres, logfile,
		initcfl, endcfl, rampstart, rampend,
		tolerance, maxiter,
		linmaxiterstart, linmaxiterend
	};

	SteadySolver<1> *time = nullptr;
	SteadySolver<1> *starttime = nullptr;
	
	if(timesteptype == "IMPLICIT") 
	{
		time = new SteadyBackwardEulerSolver<1>(prob, tconf, ksp);

		startprob->initializeUnknowns(u);

		if(usestarter != 0)
		{
			starttime = new SteadyBackwardEulerSolver<1>(startprob, tconf, ksp);

			// solve the starter problem to get the initial solution
			ierr = starttime->solve(u); CHKERRQ(ierr);

			delete starttime;
		}

		/* Solve the main problem using either the initial solution
		 * set by initializeUnknowns or the one computed by the starter problem.
		 */
		ierr = time->solve(u); CHKERRQ(ierr);
	}
	else {
		time = new SteadyForwardEulerSolver<1>(prob, u, tconf);
		
		startprob->initializeUnknowns(u);

		if(usestarter != 0)
		{
			starttime = new SteadyForwardEulerSolver<1>(startprob, u, tconf);

			// solve the starter problem to get the initial solution
			ierr = starttime->solve(u); CHKERRQ(ierr);

			delete starttime;
		}

		/* Solve the main problem using either the initial solution
		 * set by initializeUnknowns or the one computed by the starter problem.
		 */
		ierr = time->solve(u); CHKERRQ(ierr);
	}

	// postprocess

	const a_real *uarr;
	ierr = VecGetArrayRead(u, &uarr); CHKERRQ(ierr);
	for(int iel = 0; iel < m.gnelem(); iel++)
	{
		a_real rc[2]; rc[0] = 0; rc[1] = 0;
		for(int inode = 0; inode < m.gnnode(iel); inode++) {
			rc[0] += m.gcoords(m.ginpoel(iel,inode),0);
			rc[1] += m.gcoords(m.ginpoel(iel,inode),1);
		}
		rc[0] /= m.gnnode(iel); rc[1] /= m.gnnode(iel);
		a_real trueval = uexact(rc);
		err += (uarr[iel]-trueval)*(uarr[iel]-trueval)*m.garea(iel);
	}
	ierr = VecRestoreArrayRead(u, &uarr); CHKERRQ(ierr);

	err = sqrt(err);
	double h = 1.0/sqrt(m.gnelem());
	cout << "Log of Mesh size and error are " << log10(h) << "  " << log10(err) << endl;

	//amat::Array2d<a_real> dumv;
	//prob->postprocess_point(u, outputarr, dumv);
	//string scaname[1] = {"some-quantity"}; string vecname;
	//writeScalarsVectorToVtu_PointData(outf, m, outputarr, scaname, dummy, vecname);

	delete prob;
	delete startprob;
	delete time;

	ierr = KSPDestroy(&ksp); CHKERRQ(ierr);
	ierr = VecDestroy(&u); CHKERRQ(ierr);
	ierr = MatDestroy(&M); CHKERRQ(ierr);
	
	cout << "\n--------------- End --------------------- \n\n";
	ierr = PetscFinalize(); CHKERRQ(ierr);
	return ierr;
}
