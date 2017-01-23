#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cmaes_interface.h"

#define F "rae2822.dat"
#define N 8

double fitfun(double *x, int dim);
bool is_feasible(double *x, int dim);
void file(char *name);
void cosine(double A[2][N]);
double f(double x, double *A, int index, bool option);
double S(double x, double *A, int index, bool option);
double B(double x, int i);
int K(int i);
int factorial(int x);
double pow2(double x, int n);

double NACA0012[2][8] = {{ 0.17,  0.16,  0.152,  0.155,  0.145,  0.135,  0.14,  0.14},
                         {-0.17, -0.16, -0.152, -0.155, -0.145, -0.135, -0.14, -0.14}};
double NACA2412[2][8] = {{ 0.17,  0.21,  0.2,  0.19,  0.19,  0.2,  0.19,  0.2},
                         {-0.17, -0.12, -0.11, -0.105, -0.1, -0.085, -0.07, -0.06}};
double RAE2822[2][8] = {{ 0.125, 0.145,  0.16, 0.165,  0.17,  0.185, 0.185,   0.2},
                        {-0.125, -0.14, -0.16, -0.13, -0.12, -0.075, -0.02, 0.065}};
double CLARKY[2][8] = {{ 0.18,  0.25,  0.24,   0.24,  0.26,   0.27,  0.25,   0.25},
                       {-0.12, -0.08, -0.05, -0.045, -0.04, -0.035, -0.03, -0.025}};

int P = 100, sizeu, sizel, out_count = 0;
double **ep, **b;
FILE *out_file;

/* the optimization loop */
int main(int argc, char **argv) {
	cmaes_t evo; /* an CMA-ES type struct or "object" */
	double *arFunvals, *const*pop, *xfinal;
	int i, n;

	out_file = fopen(argv[2], "w");
	fprintf(out_file, "     \n\n");
	file(argv[1]);
	/*if(argc == 2) file(argv[1]);
	else          cosine(RAE2822);*/

	/* Initialize everything into the struct evo, 0 means default */
	arFunvals = cmaes_init(&evo, 0, NULL, NULL, 0, 0, "cmaes_initials.par");
	//printf("%s\n", cmaes_SayHello(&evo));
	cmaes_ReadSignals(&evo, "cmaes_signals.par");	/* write header and initial values */
	int c = 0;

	for(n = 0; n < N; n++)
		fprintf(out_file, "%.15f\t", evo.sp.xstart[n]);
	fprintf(out_file, "\n%.15f\t", -evo.sp.xstart[0]);
	for(n = 0; n < N-1; n++)
		fprintf(out_file, "%.15f\t", evo.sp.xstart[n+N]);
	fprintf(out_file, "\n\n");

	/* Iterate until stop criterion holds */
	while(!cmaes_TestForTermination(&evo)) { 
		c++;
		/* generate lambda new search points, sample population */
		pop = cmaes_SamplePopulation(&evo); /* do not change content of pop */

		/* Here we may resample each solution point pop[i] until it
 	 becomes feasible. function is_feasible(...) needs to be
 	 user-defined.
 	 Assumptions: the feasible domain is convex, the optimum is
 	 not on (or very close to) the domain boundary, initialX is
 	 feasible and initialStandardDeviations are sufficiently small
 	 to prevent quasi-infinite looping. */

		for (i = 0; i < cmaes_Get(&evo, "popsize"); ++i) {
			while (!is_feasible(pop[i], (int) cmaes_Get(&evo, "dim")))
				cmaes_ReSampleSingle(&evo, i);
		}

		/* evaluate the new search points using fitfun */
		for (i = 0; i < cmaes_Get(&evo, "lambda"); ++i) {
			arFunvals[i] = fitfun(pop[i], (int) cmaes_Get(&evo, "dim"));
			out_count++;
			for(n = 0; n < N; n++)
				fprintf(out_file, "%.15f\t", pop[i][n]);
			fprintf(out_file, "\n%.15f\t", -pop[i][0]);
			for(n = 0; n < N-1; n++)
				fprintf(out_file, "%.15f\t", pop[i][n+N]);
			fprintf(out_file, "\n\n");
		}

		/* update the search distribution used for cmaes_SamplePopulation() */
		cmaes_UpdateDistribution(&evo, arFunvals);	

		/* read instructions for printing output or changing termination conditions */ 
		cmaes_ReadSignals(&evo, "cmaes_signals.par");
		fflush(stdout); /* useful in MinGW */
	}
	//printf("Stop:\n%s\n",	cmaes_TestForTermination(&evo)); /* print termination reason */
	cmaes_WriteToFile(&evo, "all", "allcmaes.dat");				 /* write final results */
	//printf("%d\n", c);
	/* get best estimator for the optimum, xmean */
	xfinal = cmaes_GetNew(&evo, "xmean"); /* "xbestever" might be used as well */
	cmaes_exit(&evo); /* release memory */ 

	out_count++;
	for(n = 0; n < N; n++)
		fprintf(out_file, "%.15f\t", xfinal[n]);
	fprintf(out_file, "\n%.15f\t", -xfinal[0]);
	for(n = 0; n < N-1; n++)
		fprintf(out_file, "%.15f\t", xfinal[n+N]);
	fprintf(out_file, "\n\n");

	/* do something with final solution and finally release memory */
	free(xfinal);

	fseek(out_file, 0, SEEK_SET);
	fprintf(out_file, "%-5d", out_count+1);
	// printf("%d\n", out_count);
	fclose(out_file);

	return 0;
}

double fitfun(double *x, int dim) {
	int i;
	double y, error = 0, individual[2][N];
	individual[0][0] = x[0]; individual[1][0] = -x[0];
	for(i = 1; i < N; i++) individual[0][i] = x[i];
	for(i = 1; i < N; i++) individual[1][i] = x[i+N-1];
	for(i = 0; i < P; i++) {
		if(i < sizeu) y = f(ep[i][0], individual[0], i, 1);
		else          y = f(ep[i][0], individual[1], i, 1);
		error += pow2(y-ep[i][1], 2);
	}
	//printf("%.15f\n", error);
	return error;
}

bool is_feasible(double *x, int dim) {
	int i;
	double xf, y, opp, individual[2][N], p;
	individual[0][0] = x[0]; individual[1][0] = -x[0];
	for(i = 1; i < N; i++) individual[0][i] = x[i];
	for(i = 1; i < N; i++) individual[1][i] = x[i+N-1];
	for(i = 1; i < 9999; i++) {
		p = double(i)/20000;
		xf = 1-2*((cos(p*2*M_PI) - 1) / (-4.0));
		y = f(xf, individual[0], i, 0);
		opp = f(xf, individual[1], i, 0);
		if(y <= opp) return 0;
	}
	return 1;

}

void file(char *name) {
	int i, j;
	char init[50];
	FILE *f = fopen(name, "r");

	fscanf(f, "%[^\n]s", init);
	fscanf(f, "%d. %d.", &sizeu, &sizel);
	P = sizeu+sizel;

	ep = (double **) malloc(P*sizeof(double *));
	b = (double **) malloc(P*sizeof(double *));
	for(i = 0; i < P; i++) {
		ep[i] = (double *) malloc(3*sizeof(double));
		b[i] = (double *) malloc(N*sizeof(double));

		fscanf(f, "%lf %lf", &ep[i][0], &ep[i][1]);
		if(i < sizeu) ep[i][2] = (1 - ep[i][0]) / 2.0;
		else          ep[i][2] = (1 + ep[i][0]) / 2.0;

		for(j = 0; j < N; j++)
			b[i][j] = B(ep[i][0], j);
	}
	fclose(f);
}
void cosine(double A[2][N]) {
	int i, j;
	double p = 0;

	sizeu = sizel = P/2;
	ep = (double **) malloc(P*sizeof(double *));
	b = (double **) malloc(P*sizeof(double *));
	for(i = 0; i < P; i++) {
		ep[i] = (double *) malloc(3*sizeof(double));
		b[i] = (double *) malloc(N*sizeof(double));
		p = double(i)/P;

		if(p < 0.5) {
			ep[i][2] = (cos(p*2*M_PI) - 1) / (-4.0);
			ep[i][0] = 1-2*ep[i][2];
			for(j = 0; j < N; j++)
				b[i][j] = B(ep[i][0], j);
			ep[i][1] = f(ep[i][0], A[0], i, 1);
		} else {
			ep[i][2] = (cos(p*2*M_PI) + 3) / 4.0;
			ep[i][0] = 2*ep[i][2]-1;
			for(j = 0; j < N; j++)
				b[i][j] = B(ep[i][0], j);
			ep[i][1] = f(ep[i][0], A[1], i, 1);
		}
	}
}

double f(double x, double *A, int index, bool option) {
	return sqrt(x)*(1-x) * S(x, A, index, option);
}
double S(double x, double *A, int index, bool option) {
	double total = 0;
	int i = 0;
	if(option) for(i = 0; i < N; i++) total += A[i] * b[index][i];
	else       for(i = 0; i < N; i++) total += A[i] * B(x, i);
	return total;
}
double B(double x, int i) {
	return K(i) * pow2(x, i) * pow2(1-x,N-1-i);
}
int K(int i) {
	return factorial(N-1)/(factorial(i) * factorial(N-1-i));
}
int factorial(int x) {
	int t = x, i = 0;
	if(x == 0) return 1;
	for(i = 1; i < x; i++) t *= i;
	return t;
}
double pow2(double x, int n) {
	double t = x;
	if(n == 0) return 1;
	switch(n) {
		case 8: t *= t;
		case 4: t *= t;
		case 2: t *= t; break;
		case 7: t *= x;
		case 6: t *= x;
		case 5: t *= x*x;
		case 3: t *= x*x;
	}
	return t;
}
