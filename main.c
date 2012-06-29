/* Converting Grid-Density to Flash Input */
/* Markus Haider 27.06.2012                  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <math.h>

#define BUF 400

int main(int argc, char *argv[]) {

  /* open the time list */
  char * filename = "/home/markus/potentials/gadget_timer_175.txt";
  char * snapPath = "/home/markus/potentials/density/";

  /* number of refinement levels */
  int refLevels = 5;
  int res = 128;
  float boxSize = 20000;
  float level[5][2] = { {0.00000,1.00000}, 
			{0.25000,0.75000},
			{0.37500,0.62500},
			{0.43750,0.56250},
			{0.46875,0.53125} };

  double Mpc_si = 3.08568025E22;
  double Msol_si = 1.98892E30;
  double H_si = 70.*1E3/Mpc_si;
  double G_si = 6.67384E-11;
  double rhoc_si = 3*H_si*H_si/(8.*3.14*G_si);
  double conversionFactor = 1E10*Msol_si/0.7/pow(Mpc_si/0.7,3)/rhoc_si;
  float cFactor = (float)conversionFactor;
 
  FILE * fp = fopen(filename,"r");
  if (fp == NULL) {
    printf("Could not open %s\n",filename);
    return -1;
  }

  /* Count the lines in the input time file */
  char ch='\0';
  char ch2='\0';
  char buf[200];
  int counter=0;
  while( ch != EOF ) {
    ch=fgetc(fp);
    if(ch=='\n'&&ch2!='\n')
      counter++;
    ch2=ch;
  }
  const int nFiles = counter;
  printf(" Snapshot time file has %i entries\n",nFiles);

  /* read the times */
  double * snapshotTime = (double*) calloc(nFiles,sizeof(double));
  int i;
  rewind(fp);
  for(i=0;i<nFiles;i++) {
    fscanf(fp, "%lf\n",&snapshotTime[i]);
  }
  fclose(fp);
  printf(" The snapshots are at the scale factors\n");
  for(i=0;i<nFiles;i++) {
    printf(" %g\n",snapshotTime[i]);
  }

  /* make an array of the snapshot file names*/
  /* first index is the file number */
  /* second index the refinement level */

  int j;
  char ***snapFile;
  snapFile = malloc(nFiles*sizeof(char**));
  for(i=0;i<nFiles;i++) {
    snapFile[i] = malloc(refLevels*sizeof(char*));
    for(j=0;j<refLevels;j++) {
      snapFile[i][j] = malloc(BUF*sizeof(char));
    }
  }

  char snapName[BUF];
  for(i=0;i<nFiles;i++) {
    for(j=0;j<refLevels;j++) {
      snapFile[i][j][0] = '\0';
      sprintf(snapName,"dmDensity_lev%i_%03i.a_den",j,i);
      strcat(snapFile[i][j],snapPath);
      strcat(snapFile[i][j],snapName);
    }
  }
  /* print the snapshot file names */
  /* printf(" The Following snapshot files will be used:\n"); */
  /* for(i=0;i<nFiles;i++) { */
  /*   for(j=0;j<refLevels;j++) { */
  /*     printf(" %s\n",snapFile[i][j]); */
  /*   } */
  /* } */

  /* make the hdf5 file */
  hid_t file_id;
  hid_t group_id;
  hid_t dspace_id;
  hid_t a_id;
  hid_t dset_id;
  hsize_t length = nFiles;
  hsize_t dim[3] = {res,res,res};
  hsize_t two = 2;
  char groupName[BUF];

  filename="./density.hdf5";
  file_id = H5Fcreate(filename , H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  /* create group */
  group_id = H5Gcreate(file_id, "/Time", H5P_DEFAULT);
  /* creating attribute number of timesteps */
  dspace_id = H5Screate(H5S_SCALAR);
  a_id = H5Acreate(file_id,"nTimeSteps", H5T_STD_U32LE,
  			   dspace_id,H5P_DEFAULT);
  H5Awrite(a_id, H5T_STD_U32LE, &nFiles);
  H5Aclose(a_id);
  a_id = H5Acreate(file_id,"Boxsize in kpc/h", H5T_IEEE_F32LE,
  			   dspace_id,H5P_DEFAULT);
  H5Awrite(a_id, H5T_NATIVE_FLOAT, &boxSize);
  H5Aclose(a_id);
  a_id = H5Acreate(file_id,"Density Unit is given in units of critical density (in SI)", H5T_IEEE_F64LE,
  			   dspace_id,H5P_DEFAULT);
  H5Awrite(a_id, H5T_NATIVE_DOUBLE, &rhoc_si);
  H5Aclose(a_id);
  dspace_id = H5Screate_simple(1, &two,NULL);
  for(i=0;i<refLevels;i++) {
    sprintf(groupName,"Extent Level %i",i);
    a_id = H5Acreate(file_id,groupName, H5T_IEEE_F32LE,
		     dspace_id,H5P_DEFAULT);
    H5Awrite(a_id, H5T_NATIVE_FLOAT, &level[i][0]);
    H5Aclose(a_id);
  }
  H5Sclose(dspace_id);

  /* specify the dimensions of the dataset */
  dspace_id = H5Screate_simple(1,&length,NULL);
  /* create the dataset */
  dset_id = H5Dcreate(group_id,"scaleFactor",H5T_IEEE_F64LE,dspace_id, H5P_DEFAULT);
  H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, dspace_id, dspace_id, H5P_DEFAULT, &snapshotTime[0]);
  H5Sclose(dspace_id);
  H5Dclose(dset_id);
  H5Gclose(group_id);
  H5Fclose(file_id);


  /* load data */
  float *dens;
  dens = malloc(res*res*res*sizeof(float));



  /* loop over the files we want to write into the hdf5 file */
  printf(" --------- writing data into hdf file --------- \n");
  file_id = H5Fopen(filename,H5F_ACC_RDWR,H5P_DEFAULT);
  group_id = H5Gcreate(file_id,"/Density",H5P_DEFAULT);
  H5Gclose(group_id);
  int k;
  char dsetName[BUF];
  for(i=0;i<nFiles;i++) {

    sprintf(groupName,"/Density/T=%03i",i);
    printf("groupName: %s\n",groupName);
    group_id = H5Gcreate(file_id,groupName,H5P_DEFAULT);

    for(j=0;j<refLevels;j++) {
      printf(" %s\n",snapFile[i][j]);
      /* open file */
      fp = fopen(snapFile[i][j],"r");
      if (fp == NULL) {
	printf("Could not open %s\n",snapFile[i][j]);
	return -1;
      }
      fread(dens,sizeof(float),res*res*res,fp);
      fclose(fp);

      /* convert the density to units in rhoc */
      for(k=0; k<res*res*res; k++) {
	dens[k]*=cFactor;
      }

      /* write the data into the hdf5 file */
      dspace_id = H5Screate_simple(3, dim, dim);
      sprintf(dsetName,"density%03i_level%i",i,j);
      printf("dsetName: %s\n",dsetName);
      dset_id = H5Dcreate(group_id,dsetName,H5T_IEEE_F32LE,dspace_id,H5P_DEFAULT);
      H5Dwrite(dset_id, H5T_NATIVE_FLOAT, dspace_id, dspace_id, H5P_DEFAULT, &dens[0]);
      H5Dclose(dset_id);
    }
    H5Gclose(group_id);
  }
  H5Fclose(file_id);
  /* free the density array */
  free(dens);



  /* free the snapFile */
  for (i=0; i<nFiles; i++) {
    for(j=0;j<refLevels;j++) {
      free(snapFile[i][j]);
    }
    free(snapFile[i]);
  }
  free(snapFile);
   
  return 0;

}
