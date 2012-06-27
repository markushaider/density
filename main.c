/* Converting Grid-Density to Flash Input */
/* Markus Haider 27.06.2012                  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>

#define BUF 400

int main(int argc, char *argv[]) {

  /* open the time list */
  char * filename = "/home/markus/potentials/gadget_timer_175.txt";
  char * snapPath = "/home/markus/potentials/density/";

  /* number of refinement levels */
  int refLevels = 4;
  int res = 128;

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
  int startNumber=10;
  for(i=0;i<nFiles;i++) {
    for(j=0;j<refLevels;j++) {
      snapFile[i][j][0] = '\0';
      sprintf(snapName,"dmDensity_lev%i_%03i.a_den",j,i+startNumber);
      strcat(snapFile[i][j],snapPath);
      strcat(snapFile[i][j],snapName);
    }
  }
  /* print the snapshot file names */
  printf(" The Following snapshot files will be used:\n");
  for(i=0;i<nFiles;i++) {
    for(j=0;j<refLevels;j++) {
      printf(" %s\n",snapFile[i][j]);
    }
  }

  /* make the hdf5 file */
  hid_t file_id;
  filename="./density.hdf5";
  file_id = H5Fcreate(filename , H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  /* create group */
  hid_t group_id;
  group_id = H5Gcreate(file_id, "/Time", H5P_DEFAULT);
  /* creating attributes */
  /* number of output timesteps */
  hid_t dspace_id;
  hid_t a_id;
  dspace_id = H5Screate(H5S_SCALAR);
  a_id = H5Acreate(group_id,"nTimeSteps", H5T_STD_U32LE,
  			   dspace_id,H5P_DEFAULT);
  H5Awrite(a_id, H5T_STD_U32LE, &nFiles);
  H5Aclose(a_id);

  hsize_t length = nFiles;
  /* specify the dimensions of the dataset */
  dspace_id = H5Screate_simple(1,&length,NULL);
  /* create the dataset */
  hid_t dset_id;
  dset_id = H5Dcreate(group_id,"scaleFactor",H5T_IEEE_F64LE,dspace_id, H5P_DEFAULT);
  hsize_t offset = 0;
  hsize_t count = 1;
  hid_t memspace_id = H5Screate(H5S_SCALAR);
  for(i=0; i<nFiles; i++) {
    H5Sselect_hyperslab(dspace_id,H5S_SELECT_SET,&offset,NULL,&count,NULL);
    H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, memspace_id, dspace_id, H5P_DEFAULT, &snapshotTime[i]);
    offset += 1;
  }
  H5Sclose(memspace_id);
  H5Dclose(dset_id);
  H5Gclose(group_id);
  H5Fclose(file_id);




  /* load data */
  float *dens;
  dens = malloc(res*res*res*sizeof(float));
  /* open file */
  fp = fopen(snapFile[0][0],"r");
  if (fp == NULL) {
    printf("Could not open %s\n",filename);
    return -1;
  }
  fread(dens,sizeof(float),res*res*res,fp);


  fclose(fp);

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
