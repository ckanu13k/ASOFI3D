/*------------------------------------------------------------------------
 *   loop over snapshotfiles which have to be merged.                                   

 *  ----------------------------------------------------------------------*/
#include <stdlib.h>

#include "fd.h"
#include "globvar.h"      /* definition of global variables  */


int main(int argc, char **argv) {

int i, j, k, ntr=0, itr, nsrc, l, c, ishot, imin, imax, kmin, kmax;
/*int h, safe, aperz, aperx, ks, is, nspap;*/
int * stype=NULL;
float vp, xsrc, ysrc, zsrc, tshift, xrec, yrec, zrec;
float Xr1, Xr2, Zr1, Zr2, xmin, xmax, zmin, zmax;
float xc, zc, rho;

float  ** srcpos=NULL, ** recpos=NULL;
char file_vp[STRING_SIZE], file_vp_part[STRING_SIZE], file_rho[STRING_SIZE], file_rho_part[STRING_SIZE];
char recfs[STRING_SIZE], shift_shot[STRING_SIZE];
char cline[256];
char *fileinp="";
FILE *fvpmod, *frhomod,*fvpmpart, *frhompart, * fpsrc, *fpr, *fshift;

if (argc != 2) {
    exit(1);    
}

fileinp = argv[1];
printf(" ***********************************************************\n");
printf(" This is program PART_MODEL. \n");
printf(" Merge of snapshot files from the parallel  \n 3-D Viscoelastic Finite Difference Modelling      \n");
printf("                                                            \n");
printf(" written by  T. Bohlen                          \n");
printf(" Geophysical Institute, Department of Physics,         \n");
printf(" Institute of Technology, Karlsruhe, Germany         \n");
printf(" http://www.gpi.kit.edu \n");
printf(" ***********************************************************\n");
printf("\n");
printf(" Syntax if executed from ./par directory: ../bin/snapmerge in_and_out/ASOFI3D.inp \n");
printf(" Input file for the snapmerge process from command line : %s \n",fileinp);

/* read parameters from parameter-file */
if ((FP=fopen(fileinp,"r"))==NULL) err(" Opening input file failed.");
else printf(" Opening input file was successful.\n\n");

/* read parameters from parameter-file */

//read json formated input file
read_par_json(stdout, fileinp);
fclose(FP);

NXG=NX;
NYG=NY;	
NZG=NZ;	

/* read source positions */
/* --------------------- */
if ((fpsrc=fopen(SOURCE_FILE,"r"))==NULL) err(" Source file could not be opened !");

char *pline;
pline = fgets(cline,255,fpsrc);
if (pline == NULL) {
    err("Cannot read the first line from the source file '%s'", SOURCE_FILE);
}
if (sscanf(cline,"%d",&nsrc)==0) fprintf(FP,"\n WARNING: Could not determine number of sources parameter sets in input file. Assuming %d.\n",(nsrc=0));
else printf(" Number of source positions specified in %s : %d \n",SOURCE_FILE,nsrc);

stype=ivector(1,nsrc); /* for unknown reasons, the pointer does not point to memory that has been allocated by a subroutine this way */
srcpos=fmatrix(1,6,1,nsrc);

for (l=1;l<=nsrc;l++){
    pline = fgets(cline,255,fpsrc);
    if (pline == NULL) {
        err("Cannot read the information on source %d/%d from the "
            "source file '%s'", l, nsrc, SOURCE_FILE);
    }

    sscanf(cline, "%f%f%f%f%f%f%i",
           &xsrc, &zsrc, &ysrc, &tshift,
           &srcpos[5][l], &srcpos[6][l], &stype[l]);
    srcpos[1][l]=xsrc;
    srcpos[2][l]=ysrc;
    srcpos[3][l]=zsrc;
    srcpos[4][l]=tshift;
}
fclose(fpsrc);

for (l=1;l<=nsrc;l++){
   printf(" %6.2e\t%6.2e\t%6.2e\t%6.2e\t%6.2e\t%6.2e\n", srcpos[1][l],srcpos[2][l],srcpos[3][l],srcpos[4][l],srcpos[5][l],srcpos[6][l]);}

sprintf(recfs,"%s",REC_FILE);

/* open file to save coordinate shifts */
sprintf(shift_shot,"shift_shot.dat");
fshift=fopen(shift_shot,"w"); 
fprintf(fshift,"ishot \t\t xc \t\t zc \n"); 
 
for(ishot=1;ishot<=nsrc;ishot++){
   
/* read receiver positions */
/* ----------------------- */   
Xr2=0.0;
Zr2=0.0;

Xr1=NXG*DX;
Zr1=NZG*DZ;
sprintf(REC_FILE,"%s.shot%d",recfs,ishot);

printf("\n Reading receiver positions from file: \n\t%s\n",REC_FILE);
fpr=fopen(REC_FILE,"r");
if (fpr==NULL) err(" Receiver file could not be opened !");
  ntr=0;
  while ((c=fgetc(fpr)) != EOF) {
        if (c=='\n') ++(ntr);
  }
  rewind(fpr);
        
	recpos=fmatrix(1,3,1,ntr);
    for (itr = 1; itr <= ntr; itr++) {
        if (fscanf(fpr, "%f%f%f\n", &xrec, &zrec, &yrec) != 3) {
            err("Cannot parse receiver's position from line %d of the "
                "receiver file '%s'", itr, REC_FILE);
        };
        recpos[1][itr]=xrec;
        recpos[2][itr]=yrec;
        recpos[3][itr]=zrec;

        /* find maximum values of xrec and zrec */
        if(xrec > Xr2){Xr2 = xrec;}
        if(zrec > Zr2){Zr2 = zrec;}

        /* find minimum values of xrec and zrec */
        if(xrec < Xr1){Xr1 = xrec;}
        if(zrec < Zr1){Zr1 = zrec;}
    }
	  
fclose(fpr);
		
printf("minimum xrec %f \n",Xr1);		
printf("maximum xrec %f \n",Xr2);
printf("minimum zrec %f \n",Zr1);		
printf("maximum zrec %f \n",Zr2);

/* calculate minimum and maximum points of the grid */

xmin = Xr1;
if(srcpos[1][ishot] < xmin){xmin = srcpos[1][ishot];}

xmax = Xr2;
if(srcpos[1][ishot] > xmax){xmax = srcpos[1][ishot];}

zmin = Zr1;
if(srcpos[3][ishot] < zmin){zmin = srcpos[3][ishot];}

zmax = Zr2;
if(srcpos[3][ishot] > zmax){zmax = srcpos[3][ishot];}

/* add gridpoints for the PML B.C., add a safety factor and calculate model dimensions */
imin = ((int)ceil((float)(xmin/DX))) - FW ;
imax = ((int)ceil((float)(xmax/DX))) + FW + 1;
kmin = ((int)ceil((float)(zmin/DZ))) - FW ;
kmax = ((int)ceil((float)(zmax/DZ))) + FW ;

/* size of the new model */
printf("Size of the new model for shot %d \n",ishot);
printf("NX = %d \n",(imax-imin)+1);
printf("NZ = %d \n",(kmax-kmin)+1);
printf("imin = %d \n",imin);
printf("imax = %d \n",imax);
printf("kmin = %d \n",kmin);
printf("kmax = %d \n",kmax);
printf("xmin = %f \n",(float)(imin*DX));
printf("xmax = %f \n",(float)(imax*DX));
printf("zmin = %f \n",(float)(kmin*DZ));
printf("zmax = %f \n",(float)(kmax*DZ));


/* calculate postion of the model origin [m] */

xc = (float)((imin-1)*DX);
zc = (float)((kmin-1)*DZ);

printf("xc = %f \n",xc);
printf("zc = %f \n",zc);

fprintf(fshift,"%d \t %f \t %f \n",ishot, xc, zc);

/* calculate new source and receiver positions on the sub-grid */
/* ----------------------------------------------------------- */
sprintf(REC_FILE,"%s_sub.shot%d",recfs,ishot);

printf("\n Writing receiver positions to file: \n\t%s\n",REC_FILE);
fpr=fopen(REC_FILE,"w");
	  for (itr=1;itr<=ntr;itr++){
				xrec=recpos[1][itr]-xc;
				yrec=recpos[2][itr];
				zrec=recpos[3][itr]-zc;
				
				fprintf(fpr,"%f \t %f \t %f \n",xrec, zrec, yrec);
	  }
fclose(fpr);

srcpos[1][ishot] = srcpos[1][ishot] - xc;
srcpos[3][ishot] = srcpos[3][ishot] - zc;


printf("Extracting and writing model for shot %d \n",ishot);

/* open global model files for vp and rho */
sprintf(file_vp,"%s.vp",MFILE);
fvpmod=fopen(file_vp,"r");

sprintf(file_vp_part,"%s_shot%d.vp",MFILE,ishot);
fvpmpart=fopen(file_vp_part,"w");

sprintf(file_rho,"%s.rho",MFILE);
frhomod=fopen(file_rho,"r");

sprintf(file_rho_part,"%s_shot%d.rho",MFILE,ishot);
frhompart=fopen(file_rho_part,"w");


size_t nelems;
for (k=1;k<=NZ;k++){
    /*printf("Reading slice %d in z-direction \n",k);*/
    for (i=1;i<=NX;i++){
        for (j=1;j<=NY;j++){
            nelems = fread(&vp, sizeof(float), 1, fvpmod);
            if (nelems != 1) {
                err("Could not read model file '%s'", file_vp);
            }

            nelems = fread(&rho, sizeof(float), 1, frhomod);
            if (nelems != 1) {
                err("Could not read model file '%s'", file_rho);
            }

            if((i>=imin)&&(i<=imax)&&(k>=kmin)&&(k<=kmax)){
                fwrite(&vp, sizeof(float), 1,fvpmpart);
                fwrite(&rho, sizeof(float), 1,frhompart);
            }

        }
    }
}
                 
				
fclose(fvpmod);
fclose(fvpmpart);
fclose(frhomod);
fclose(frhompart);

}

/* output of new source positions */
sprintf(SOURCE_FILE,"%s.sub",SOURCE_FILE);
printf("\n Writing source positions to file: \n\t%s\n",SOURCE_FILE);
fpsrc=fopen(SOURCE_FILE,"w");

fprintf(fpsrc,"%d \n",nsrc);
for (l=1;l<=nsrc;l++){

   fprintf(fpsrc,"%f \t %f \t %f \t %f \t %f \t %f \t %i \n",srcpos[1][l], srcpos[3][l], srcpos[2][l], 0.0, srcpos[5][l], srcpos[6][l], stype[l]);
       
}
fclose(fpsrc);
fclose(fshift);	
		
return 0;	

}
