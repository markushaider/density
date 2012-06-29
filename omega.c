/* Converting Grid-Density to Flash Input */
/* Markus Haider 27.06.2012                  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <hdf5.h>

#define BUF 400

int main(int argc, char *argv[]) {

  /* open the time list */
  float *dens;
  const int res = 128;
  dens = malloc(res*res*res*sizeof(float));

  double Mpc_si = 3.08568025E22;
  double H_si = 70.*1E3/Mpc_si;	/* insert the Hubble constant here */
  double G_si = 6.67384E-11;
  double rhoc_si = 3*H_si*H_si/(8.*3.14*G_si);
  double Msol_si = 1.98892E30;
  double boxSize = 20;		/* in Mpc */
  printf("Critical density in SI units: %g\n", rhoc_si);

  printf("Music for Omega_m: 0.276\n");
  printf("Music for Omega_b: 0.045\n");

  /* calculate conversion factor to SI */
  /* assuming that density is in units of 1E10 Msol/h / Mpc/h */
  double factor = 1E10*Msol_si/0.7/pow(Mpc_si/0.7,3);

  //  char * filename = "/home/markus/potentials/unittest/cut_conv000.a_den";
  char * filename = "/home/markus/potentials/density/dmDensity_lev0_175.a_den";
  FILE * fp;
  fp = fopen(filename,"r");
  if (fp == NULL) {
    printf("Could not open %s\n",filename);
    return -1;
  }
  fread(dens,sizeof(float),res*res*res,fp);
  fclose(fp);

  /* sum the densities */
  int i;
  double totalDens=0;
  for(i=0;i<res*res*res;i++) {
    totalDens+=(double)dens[i];
  }

  printf("Total Density: %g \n",totalDens);
  printf("Total Density times conversion factory: %g \n",totalDens*factor/(128*128*128));
  printf("Total Density times conversion factory in rhoc: %g \n",totalDens*factor/(128*128*128*rhoc_si));
  free(dens);
   
  return 0;

}
