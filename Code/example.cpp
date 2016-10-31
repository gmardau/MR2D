// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include "mr2d"
#include "models.h"

int main(int argc, char const *argv[])
{
	mr2d::mesh m;
	mr2d::display d(m);

	d.display_mesh(0);
	for(int i = 0; i < 10000; ++i) {
		std::list<mr2d::vertex>::iterator it[3];
		it[0] = m.add_vertex(0, (double)rand()/INT_MAX*10-5, (double)rand()/INT_MAX*10-5);
		it[1] = m.add_vertex(0, (double)rand()/INT_MAX*10-5, (double)rand()/INT_MAX*10-5);
		it[2] = m.add_vertex(0, (double)rand()/INT_MAX*10-5, (double)rand()/INT_MAX*10-5);
		m.add_triangle(it[0], it[1], it[2]);
		d.display_mesh(0);
		if(rand()%10 < 9) m._triangles.pop_front();
	}

	// std::list<mr2d::vertex>::iterator it[6];

	// it[3] = m.add_vertex(0, 0.5, 0);
	// it[4] = m.add_vertex(0, 1.5, 0);
	// it[5] = m.add_vertex(0, 1.5, 1);
	// m.add_triangle(it[3], it[4], it[5]);
	d.display_mesh(0);

	return 0;
}

// /* === Display help menu === */
// void display_help()
// {
// 	printf("Usage: ./main [OPTIONS]\n\n");
// 	printf("Options:\n");
// 	printf("  -h      display this help and exit\n");
// 	printf("  -i      input file\n");
// 	printf("  -m      method mode\n");
// 	printf("  -d      activate display\n");
// 	printf("  -t      collect execution times (-T writes to file)\n");
// 	printf("  -s      collect statistics (-S writes to file)\n");
// 	printf("  -Mfn    no. of farfield vertices in initial discretisation\n");
// 	printf("  -Mmn    no. of wing vertices in initial discretisation\n");
// 	printf("  -MG     gradation factor\n");
// 	printf("  -MB     minimum angle bound\n");
// 	printf("  -MH     length scale bound\n");
// 	printf("  -MT     Shewchuk's lenses angle\n");
// 	printf("  -MD     removing distance factor\n");

// 	exit(0);
// }

// /* === Process command line arguments === */
// void process_command(int argc, char const *argv[], int &f, int &m, bool &_d, int &_t, int &_s,
// 	int &fn, int &mn, double &G, double &B, double &H, double &T, double &D)
// {
// 	int i;
// 	for(i = 0; i < argc; i++) {
// 		     if(!strcmp(argv[i], "-h"))   display_help();
// 		else if(!strcmp(argv[i], "-i"))   f  = ++i;
// 		else if(!strcmp(argv[i], "-m"))   m  = atoi(argv[++i]);
// 		else if(!strcmp(argv[i], "-d"))   _d = 1;
// 		else if(!strcmp(argv[i], "-t"))   _t = 1;
// 		else if(!strcmp(argv[i], "-T"))   _t = 2;
// 		else if(!strcmp(argv[i], "-s"))   _s = 1;
// 		else if(!strcmp(argv[i], "-S"))   _s = 2;
// 		else if(!strcmp(argv[i], "-Mfn")) fn = atoi(argv[++i]);
// 		else if(!strcmp(argv[i], "-Mmn")) mn = atoi(argv[++i]);
// 		else if(!strcmp(argv[i], "-MG"))  G  = atof(argv[++i]);
// 		else if(!strcmp(argv[i], "-MB"))  B  = atof(argv[++i]);
// 		else if(!strcmp(argv[i], "-MH"))  H  = atof(argv[++i]);
// 		else if(!strcmp(argv[i], "-MT"))  T  = atof(argv[++i]);
// 		else if(!strcmp(argv[i], "-MD"))  D  = atof(argv[++i]); 
// 	}
// }
