/*------------------------------------------------------------------------
 * exchange of particle velocities at grid boundaries between processors
 * when using the standard staggered grid
 *
 *  ----------------------------------------------------------------------*/

#include "data_structures.h"
#include "fd.h"
#include "globvar.h"


double exchange_s(int nt, Tensor3d *s,
		float *** bufferlef_to_rig, float *** bufferrig_to_lef,
		float *** buffertop_to_bot, float *** bufferbot_to_top,
		float *** bufferfro_to_bac, float *** bufferbac_to_fro) {

	extern int NX, NY, NZ, POS[4], NPROCX, NPROCY, NPROCZ, BOUNDARY, MYID, FDORDER, LOG, INDEX[7];
	extern const int TAG1,TAG2,TAG3,TAG4,TAG5,TAG6;
	extern FILE *FP;
	extern int OUTNTIMESTEPINFO;

	MPI_Status status;	
	int i, j, k, l, n, nf1, nf2;
	double time=0.0, time1=0.0, time2=0.0;

        float ***sxx = s->xx;
        float ***syy = s->yy;
        float ***szz = s->zz;
        float ***sxy = s->xy;
        float ***syz = s->yz;
        float ***sxz = s->xz;

	nf1=(3*FDORDER/2)-1;
	nf2=nf1-1;


	if (LOG)
		if ((MYID==0) && ((nt+(OUTNTIMESTEPINFO-1))%OUTNTIMESTEPINFO)==0) time1=MPI_Wtime();

	/* top-bottom -----------------------------------------------------------*/	

	if (BOUNDARY || (POS[2]!=0))	/* no boundary exchange at top of global grid */
		for (i=1;i<=NX;i++){
			for (k=1;k<=NZ;k++){

				/* storage of top of local volume into buffer */
				n=1;
				for (l=1;l<=(FDORDER/2);l++){
					buffertop_to_bot[i][k][n++]  = syy[l][i][k];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					buffertop_to_bot[i][k][n++]  = sxy[l][i][k];
					buffertop_to_bot[i][k][n++]  = syz[l][i][k];
				}

			}
		}


	if (BOUNDARY || (POS[2]!=NPROCY-1))	/* no boundary exchange at bottom of global grid */
		for (i=1;i<=NX;i++){
			for (k=1;k<=NZ;k++){


				/* storage of bottom of local volume into buffer */
				n=1;
				for (l=1;l<=FDORDER/2;l++){
					bufferbot_to_top[i][k][n++]  = sxy[NY-l+1][i][k];
					bufferbot_to_top[i][k][n++]  = syz[NY-l+1][i][k];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					bufferbot_to_top[i][k][n++]  = syy[NY-l+1][i][k];
				}

			}
		}

	MPI_Sendrecv_replace(&buffertop_to_bot[1][1][1],NX*NZ*nf2,MPI_FLOAT,INDEX[3],TAG5,INDEX[4],TAG5,MPI_COMM_WORLD,&status);	
	MPI_Sendrecv_replace(&bufferbot_to_top[1][1][1],NX*NZ*nf1,MPI_FLOAT,INDEX[4],TAG6,INDEX[3],TAG6,MPI_COMM_WORLD,&status);

	if (BOUNDARY || (POS[2]!=NPROCY-1))	/* no boundary exchange at bottom of global grid */
		for (i=1;i<=NX;i++){
			for (k=1;k<=NZ;k++){

				n=1;
				for (l=1;l<=(FDORDER/2);l++){
					syy[NY+l][i][k] = buffertop_to_bot[i][k][n++];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					sxy[NY+l][i][k] = buffertop_to_bot[i][k][n++];
					syz[NY+l][i][k] = buffertop_to_bot[i][k][n++];
				}


			}
		}

	if (BOUNDARY || (POS[2]!=0))	/* no boundary exchange at top of global grid */
		for (i=1;i<=NX;i++){
			for (k=1;k<=NZ;k++){

				n=1;
				for (l=1;l<=FDORDER/2;l++){
					sxy[1-l][i][k] = bufferbot_to_top[i][k][n++];
					syz[1-l][i][k] = bufferbot_to_top[i][k][n++];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					syy[1-l][i][k] = bufferbot_to_top[i][k][n++];
				}

			}
		}






	/* left-right -----------------------------------------------------------*/	



	if ((BOUNDARY) || (POS[1]!=0))	/* no boundary exchange at left edge of global grid */
		for (j=1;j<=NY;j++){
			for (k=1;k<=NZ;k++){


				/* storage of left edge of local volume into buffer */
				n=1;
				for (l=1;l<=FDORDER/2;l++){
					bufferlef_to_rig[j][k][n++] =  sxx[j][l][k];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					bufferlef_to_rig[j][k][n++] =  sxy[j][l][k];
					bufferlef_to_rig[j][k][n++] =  sxz[j][l][k];
				}
			}
		}


	if ((BOUNDARY) || (POS[1]!=NPROCX-1))	/* no boundary exchange at right edge of global grid */
		for (j=1;j<=NY;j++){
			for (k=1;k<=NZ;k++){
				/* storage of right edge of local volume into buffer */
				n=1;

				for (l=1;l<=(FDORDER/2);l++){
					bufferrig_to_lef[j][k][n++] =  sxy[j][NX-l+1][k];
					bufferrig_to_lef[j][k][n++] =  sxz[j][NX-l+1][k];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					bufferrig_to_lef[j][k][n++] =  sxx[j][NX-l+1][k];
				}
			}
		}

	MPI_Sendrecv_replace(&bufferlef_to_rig[1][1][1],NY*NZ*nf2,MPI_FLOAT,INDEX[1],TAG1,INDEX[2],TAG1,MPI_COMM_WORLD,&status);
	MPI_Sendrecv_replace(&bufferrig_to_lef[1][1][1],NY*NZ*nf1,MPI_FLOAT,INDEX[2],TAG2,INDEX[1],TAG2,MPI_COMM_WORLD,&status);

	if ((BOUNDARY) || (POS[1]!=NPROCX-1))	/* no boundary exchange at right edge of global grid */
		for (j=1;j<=NY;j++){
			for (k=1;k<=NZ;k++){

				n=1;
				for (l=1;l<=(FDORDER/2);l++){
					sxx[j][NX+l][k] = bufferlef_to_rig[j][k][n++];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					sxy[j][NX+l][k] = bufferlef_to_rig[j][k][n++];
					sxz[j][NX+l][k] = bufferlef_to_rig[j][k][n++];
				}
			}
		}

	if ((BOUNDARY) || (POS[1]!=0))	/* no boundary exchange at left edge of global grid */
		for (j=1;j<=NY;j++){
			for (k=1;k<=NZ;k++){

				n=1;
				for (l=1;l<=(FDORDER/2);l++){
					sxy[j][1-l][k] = bufferrig_to_lef[j][k][n++];
					sxz[j][1-l][k] = bufferrig_to_lef[j][k][n++];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					sxx[j][1-l][k] = bufferrig_to_lef[j][k][n++];
				}
			}
		}






	/* front-back -----------------------------------------------------------*/
	if ((BOUNDARY) || (POS[3]!=0))	/* no boundary exchange at front side of global grid */
		for (i=1;i<=NX;i++){
			for (j=1;j<=NY;j++){


				/* storage of front side of local volume into buffer */
				n=1;
				for (l=1;l<=FDORDER/2;l++){
					bufferfro_to_bac[j][i][n++]  = szz[j][i][l];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					bufferfro_to_bac[j][i][n++]  =  syz[j][i][l];
					bufferfro_to_bac[j][i][n++]  =  sxz[j][i][l];
				}
			}
		}


	if ((BOUNDARY) || (POS[3]!=NPROCZ-1))	/* no boundary exchange at back side of global grid */
		for (i=1;i<=NX;i++){
			for (j=1;j<=NY;j++){

				/* storage of back side of local volume into buffer */
				n=1;
				for (l=1;l<=FDORDER/2;l++){
					bufferbac_to_fro[j][i][n++]  = syz[j][i][NZ-l+1];
					bufferbac_to_fro[j][i][n++]  = sxz[j][i][NZ-l+1];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					bufferbac_to_fro[j][i][n++]  = szz[j][i][NZ-l+1];
				}
			}
		}

	MPI_Sendrecv_replace(&bufferfro_to_bac[1][1][1],NX*NY*nf2,MPI_FLOAT,INDEX[5],TAG3,INDEX[6],TAG3,MPI_COMM_WORLD,&status);
	MPI_Sendrecv_replace(&bufferbac_to_fro[1][1][1],NX*NY*nf1,MPI_FLOAT,INDEX[6],TAG4,INDEX[5],TAG4,MPI_COMM_WORLD,&status);

	if ((BOUNDARY) || (POS[3]!=NPROCZ-1))	/* no boundary exchange at back side of global grid */
		for (i=1;i<=NX;i++){
			for (j=1;j<=NY;j++){

				n=1;
				for (l=1;l<=FDORDER/2;l++){
					szz[j][i][NZ+l] = bufferfro_to_bac[j][i][n++];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					syz[j][i][NZ+l] = bufferfro_to_bac[j][i][n++];
					sxz[j][i][NZ+l] = bufferfro_to_bac[j][i][n++];
				}
			}
		}


	if ((BOUNDARY) || (POS[3]!=0))	/* no boundary exchange at front side of global grid */
		for (i=1;i<=NX;i++){
			for (j=1;j<=NY;j++){

				n=1;
				for (l=1;l<=FDORDER/2;l++){
					syz[j][i][1-l] = bufferbac_to_fro[j][i][n++];
					sxz[j][i][1-l] = bufferbac_to_fro[j][i][n++];
				}

				for (l=1;l<=(FDORDER/2-1);l++){
					szz[j][i][1-l] = bufferbac_to_fro[j][i][n++];
				}

			}
		}

	if (LOG)
		if ((MYID==0) && ((nt+(OUTNTIMESTEPINFO-1))%OUTNTIMESTEPINFO)==0){
			time2=MPI_Wtime();
			time=time2-time1;
			fprintf(FP," Real time for stress tensor exchange: \t\t %4.2f s.\n",time);
		}
	return time;

}




















