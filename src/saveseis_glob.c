/*------------------------------------------------------------------------
 *    prepares to write merged seismograms from all PEs to files
 *
 * ----------------------------------------------------------------------*/

#include "fd.h"
#include "globvar.h"


void saveseis_glob(FILE *fp, float **sectiondata, int **recpos, int ntr,
        float ** srcpos, int ishot, int ns, int sectiondatatype){

	extern int SEIS_FORMAT[6], RUN_MULTIPLE_SHOTS;
	extern char  SEIS_FILE[STRING_SIZE];

	char vxf[STRING_SIZE], vyf[STRING_SIZE], vzf[STRING_SIZE], curlf[STRING_SIZE], divf[STRING_SIZE], pf[STRING_SIZE],file_ext[5];
	int nsrc=1;

	switch (SEIS_FORMAT[0]){
	case 0: sprintf(file_ext,"sgy"); break;
	case 1: sprintf(file_ext,"su");  break;
	case 2: sprintf(file_ext,"txt"); break;
	case 3: sprintf(file_ext,"bin"); break;
	case 4: sprintf(file_ext,"sgy"); break;
	case 5: sprintf(file_ext,"sgy"); break;
	}

	/*note that "y" denotes the vertical coordinate*/

	if (RUN_MULTIPLE_SHOTS){
		sprintf(vxf,"%s_vx.%s.shot%d",SEIS_FILE,file_ext,ishot);
		sprintf(vyf,"%s_vy.%s.shot%d",SEIS_FILE,file_ext,ishot);
		sprintf(vzf,"%s_vz.%s.shot%d",SEIS_FILE,file_ext,ishot);
		sprintf(curlf,"%s_curl.%s.shot%d",SEIS_FILE,file_ext,ishot);
		sprintf(divf,"%s_div.%s.shot%d",SEIS_FILE,file_ext,ishot);
		sprintf(pf,"%s_p.%s.shot%d",SEIS_FILE,file_ext,ishot);
	}
	else{
		sprintf(vxf,"%s_vx.%s",SEIS_FILE,file_ext);
		sprintf(vyf,"%s_vy.%s",SEIS_FILE,file_ext);
		sprintf(vzf,"%s_vz.%s",SEIS_FILE,file_ext);
		sprintf(curlf,"%s_curl.%s",SEIS_FILE,file_ext);
		sprintf(divf,"%s_div.%s",SEIS_FILE,file_ext);
		sprintf(pf,"%s_p.%s",SEIS_FILE,file_ext);

	}

	switch (sectiondatatype){
	case 1 : /* particle velocities vx only */

		fprintf(fp,"\n PE 0 is writing %d merged seismogram traces (vx)   to  %s",ntr,vxf);
		//outseis_glob(fp,fopen(vxf,"w"),1,sectiondata,recpos,recpos_loc, ntr,srcpos,nsrc,ns,SEIS_FORMAT);
		outseis_glob(fp,fopen(vxf,"w"),sectiondata,recpos,recpos, ntr,srcpos,nsrc,ns,SEIS_FORMAT,ishot, 1);
		break;

	case 2 : /* particle velocities vy only */

		fprintf(fp,"\n PE 0 is writing %d merged seismogram traces (vy)   to  %s",ntr,vyf);
		outseis_glob(fp,fopen(vyf,"w"),sectiondata,recpos,recpos, ntr,srcpos,nsrc,ns,SEIS_FORMAT,ishot, 2);
		break;

	case 3 : /* particle velocities vx only */

		fprintf(fp,"\n PE 0 is writing %d merged seismogram traces (vz)   to  %s",ntr,vzf);
		outseis_glob(fp,fopen(vzf,"w"),sectiondata,recpos,recpos, ntr,srcpos,nsrc,ns,SEIS_FORMAT,ishot, 3);
		break;

	case 4 : /* pressure only */

		fprintf(fp,"\n PE 0 is writing %d merged seismogram traces (p)    to  %s",ntr,pf);
		outseis_glob(fp,fopen(pf,"w"),sectiondata,recpos,recpos, ntr,srcpos,nsrc,ns,SEIS_FORMAT,ishot, 0);
		break;

	case 5 : /* div only */

		fprintf(fp,"\n PE 0 is writing %d merged seismogram traces (dif)  to  %s",ntr,divf);
		outseis_glob(fp,fopen(divf,"w"),sectiondata,recpos,recpos, ntr,srcpos,nsrc,ns,SEIS_FORMAT,ishot, 0);
		break;
	case 6 : /* curl only */

		fprintf(fp,"\n PE 0 is writing %d merged seismogram traces (curl) to  %s",ntr,curlf);
		outseis_glob(fp,fopen(curlf,"w"),sectiondata,recpos,recpos, ntr,srcpos,nsrc,ns,SEIS_FORMAT,ishot, 0);
		break;
	}
}
