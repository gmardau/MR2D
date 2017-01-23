#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sstream>
#include "mr2d"
#include "models.h"

#define EXECUTE(v_, t_, m_, d_) execute(argv, i, a, v_, t_, m_, d_, mr2d::MR2D<v_, t_, m_, d_>(nf, nm, L, R, G, B, H, D))

/* Forward function declarations */
template <typename T> void execute (char const *[], unsigned int, unsigned int, bool, bool, bool, bool, T);
void help_menu ();
void process_command (int, char const *[], unsigned int &, unsigned int &, bool &, bool &, bool &, bool &,
    unsigned int &, unsigned int &, double &, double &, double &, double &, double &, double &);
/* Forward function declarations */


/* === Main === */
int
main (int argc, char const *argv[])
{
	bool v = false, t = false, m = false, d = false;
	unsigned int i = 0, a = 0, nf = 6, nm = 300;
	double L = 30*M_PI/180, R = 1, G = 6, B = 30*M_PI/180, H = sqrt(2)/2, D = 1;

	process_command(argc, argv, i, a, v, t, m, d, nf, nm, L, R, G, B, H, D);

	if(!v) { if(!t) { if(!m) { if(!d) EXECUTE(0,0,0,0); else EXECUTE(0,0,0,1); }
	                  else   { if(!d) EXECUTE(0,0,1,0); else EXECUTE(0,0,1,1); } }
	         else   { if(!m) { if(!d) EXECUTE(0,1,0,0); else EXECUTE(0,1,0,1); }
	                  else   { if(!d) EXECUTE(0,1,1,0); else EXECUTE(0,1,1,1); } } }
	else   { if(!t) { if(!m) { if(!d) EXECUTE(1,0,0,0); else EXECUTE(1,0,0,1); }
	                  else   { if(!d) EXECUTE(1,0,1,0); else EXECUTE(1,0,1,1); } }
	         else   { if(!m) { if(!d) EXECUTE(1,1,0,0); else EXECUTE(1,1,0,1); }
	                  else   { if(!d) EXECUTE(1,1,1,0); else EXECUTE(1,1,1,1); } } }

	return 0;
}
/* === Main === */


/* === Program execution === */
template <typename T>
void
execute (char const *argv[], unsigned int i, unsigned int a, bool v, bool t, bool m, bool d, T example)
{
	farfield_circle farfield = farfield_circle(0.5, 0, 25);
	FILE *file =  i ? fopen(argv[i], "r") : fopen("input/rae2822/0.05/1", "r");
	char status[3][100] = {{"\0"}, {"\0"}, {"\0"}};
	unsigned int j, k, l, n, time_left;
	double A[2][8], avg_time;

	/* Iterations */
	fscanf(file, "%u", &n);
	for(j = 0; j < n; ++j) {

		/* Print status */
		sprintf(status[0], "[Iteration: %d/%d]", j+1, n);
		printf("\r%*c\r%s%s%s", 200, ' ', status[0], status[1], status[2]); fflush(stdout);

		/* Create model */
		for(k = 0; k < 2; ++k) for(l = 0; l < 8; ++l) fscanf(file, "%lf", &A[k][l]);
		model_cst_naca<8> model = model_cst_naca<8>(A);

		/* Call method */
		if(!j) example.generation(farfield, model);
		else {
			switch(a) {
				case 0: example.generation(farfield, model); break;
				case 1: example.template remodelling<0>(model); break;
				case 2: example.template remodelling<1>(model); break;
			}
		}

		/* Display mesh */
		if(v) example.template display<true>(true);
		/* Prepare status */
		if(t) {
			avg_time = example.template get_avg_time<true>(); time_left = (n-1-j) * avg_time + 0.5;
			sprintf(status[1], "  \u2219  [Avg. time: %.3lf ms] [Estimated time left: %dm %ds]",
			    avg_time*1000, time_left/60, time_left%60); }
		if(m)
			sprintf(status[2], "  \u2219  [Avg. preservation: %.2lf%%]", example.template get_avg_preservation<true>());
		/* Write diff to output file */
		if(d) example.template export_diff<true>((std::string("output/diff/") + std::to_string(j+1)).c_str());
	}
	fclose(file);

	/* Write times, metrics and mesh to output file */
	if(t) example.template export_timers <true>("output/timers");
	if(m) example.template export_metrics<true>("output/metrics");
	example.export_mesh("output/mesh");
	/* Print status */
	printf("\r%*c\r%s%s%s\n", 200, ' ', status[0], status[1], status[2]);
	/* Display mesh */
	if(v) example.template display<true>(false);
}
/* === Program execution === */


/* === Help menu === */
void
help_menu ()
{
	printf("Usage: ./main [OPTIONS]\n\n");
	printf("Options:\n");
	printf("  -h        display this and exit\n");
	printf("  -i []     path to input file; (default: \"./input/rae2822/0.05/1\")\n");
	printf("  -a []     algorithm '0-2'; (default: 0)\n");
	printf("  -v        activate display (visualiser)\n");
	printf("  -t        activate measurement of execution times\n");
	printf("  -m        activate collection of methods metrics\n");
	printf("  -d        activate generation of diff information\n");
	printf("  -nf []    no. of farfield vertices in discretisation; (default: 6)\n");
	printf("  -nm []    no. of model vertices in discretisation; (default: 300)\n");
	printf("  -L []     Shewchuk's lenses angle in degrees; (default: 30)\n");
	printf("  -R []     resolution factor; (default: 1)\n");
	printf("  -G []     gradation factor; (default: 6)\n");
	printf("  -B []     minimum angle bound in degrees: (default: 30)\n");
	printf("  -H []     length scale bound; (default: sqrt(2)/2)\n");
	printf("  -D []     remodelling removal distance; (default: 1)\n");

	exit(0);
}
/* === Help menu === */


/* === Process command line arguments === */
void
process_command (int argc, char const *argv[], unsigned int &i, unsigned int &a, bool &v, bool &t, bool &m, bool &d,
    unsigned int &nf, unsigned int &nm, double &L, double &R, double &G, double &B, double &H, double &D)
{
	for(int j = 1; j < argc; ++j)
		     if(!strcmp(argv[j], "-h"))  help_menu();
		else if(!strcmp(argv[j], "-i"))  i  = ++j;
		else if(!strcmp(argv[j], "-a"))  a  = atoi(argv[++j]);
		else if(!strcmp(argv[j], "-v"))  v  = true;
		else if(!strcmp(argv[j], "-t"))  t  = true;
		else if(!strcmp(argv[j], "-m"))  m  = true;
		else if(!strcmp(argv[j], "-d"))  d  = true;
		else if(!strcmp(argv[j], "-nf")) nf = atoi(argv[++j]);
		else if(!strcmp(argv[j], "-nm")) nm = atoi(argv[++j]);
		else if(!strcmp(argv[j], "-L"))  L  = atof(argv[++j])*M_PI/180;
		else if(!strcmp(argv[j], "-R"))  R  = atof(argv[++j]);
		else if(!strcmp(argv[j], "-G"))  G  = atof(argv[++j]);
		else if(!strcmp(argv[j], "-B"))  B  = atof(argv[++j])*M_PI/180;
		else if(!strcmp(argv[j], "-H"))  H  = atof(argv[++j]);
		else if(!strcmp(argv[j], "-D"))  D  = atof(argv[++j]);
}
/* === Process command line arguments === */
