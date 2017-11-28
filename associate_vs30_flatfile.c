#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "params.h"


/* Calculate distance and azimuth/back-azimuth between two (lon,lat) coordinates 
 * using delaz.f subroutine
 */

// function declaration
void delaz_(float *lat1, float *lon1, float *lat2, float *lon2, float *dist, float *az, float *baz);
int getcols( const char * const line, const char * const delim, char ***out_storage);
//void strip(char *s);
void assign_cols_stationfile(char **columns, float *staLon, float *staLat, char *staNet, char *staNm);
void assign_cols_vs30file(char **columns, float *lon, float *lat, char *network, char *name, char *vs30_1, char *vs30_2, char *vs30_3, char *vs30_4);
//void assign_cols_stationfile(char **columns, float *evMag, int *evYear, int *evMon, int *evDay, int *evHour, int *evMin, float *evSec);
//void assign_cols_catalog(char **columns, float *evMag, int *evYear, int *evMon, int *evDay, int *evHour, int *evMin, float *evSec, float *evMagSource, char *magString);
char * replace(char const * const original, char const * const pattern, char const * const replacement );
int compute_epochTime(int yearIn, int monthIn, int dayIn, int hourIn, int minIn, int secIn);


/*--------------------------------------------------------------------------*/
void assign_cols_stationfile(char **columns, float *staLon, float *staLat, char *staNet, char *staNm)
/*--------------------------------------------------------------------------*/
{
//
//  staNet=strcpy(staNet,columns[1]);
//  staNm=strcpy(staNm,columns[2]);
  strcpy(staNet,columns[1]);
  strcpy(staNm,columns[2]);
  *staLon=atof(columns[6]);
  *staLat=atof(columns[5]);
//  fprintf(stderr,"%s %s\n", columns[3], columns[4]);
//  fprintf(stderr,"assign_cols_stationfile, staNet/staNm/staLon/staLat: %s %s %f %f\n", staNet, staNm, *staLon, *staLat);

}

/*--------------------------------------------------------------------------*/
void assign_cols_vs30file(char **columns, float *lon, float *lat, char *network, char *name, char *vs30_1, char *vs30_2, char *vs30_3, char *vs30_4)
/*--------------------------------------------------------------------------*/
{
//
  network=strcpy(network,columns[4]);
  name=strcpy(name,columns[5]);
  *lon=atof(columns[1]);
  *lat=atof(columns[2]);
  vs30_1=strcpy(vs30_1,columns[6]);
  vs30_2=strcpy(vs30_2,columns[7]);
  vs30_3=strcpy(vs30_3,columns[8]);
  vs30_4=strcpy(vs30_4,columns[9]);
//  fprintf(stderr,"assign_cols_vs30file, network/name: %s %s\n", network, name);
//  fprintf(stderr,"assign_cols_vs30file, lon/lat: %f %f\n", *lon, *lat);
//  fprintf(stderr,"assign_cols_vs30file, vs30: %s %s %s %s\n", vs30_1, vs30_2, vs30_3, vs30_4);

}


/*--------------------------------------------------------------------------*/
int main (int argc, char *argv[])
/*--------------------------------------------------------------------------*/
{
  FILE *fp_stationFlatFile, *fp_vs30File, *fp_outputFile;
  int hlines, cnt1;
  int cols_found;
  int matchSta=0;
  float staLon, staLat, lon, lat;
  float az, baz, dist;
  char vs30_1[20], vs30_2[20], vs30_3[20], vs30_4[20];
  char network[5], staNet[5], name[5], staNm[5];
  char stationFlatFile[200], vs30File[200], outputFile[200];
  char buff[BUFFLEN], buff2[BUFFLEN];
  char **columns;
  char **columns2;
  char delim[] = ",";


/* CHECK INPUT ARGUMENTS */
  if ( argc != 4 ) {
    fprintf(stderr,"USAGE: %s [station/vs30 from flatfile] [EMT Station Vs30 information] [output file]\n", argv[0]);
    fprintf(stderr,"(e.g., %s station_vs30_flatfile.csv Stations_Vs30_add_sources.csv stations_corrected_vs30.csv\n", argv[0]);
    exit(1);
  }
  sscanf(argv[1],"%s", stationFlatFile);
  sscanf(argv[2],"%s", vs30File);
  sscanf(argv[3],"%s", outputFile);

// open files
  if ((fp_stationFlatFile = fopen(stationFlatFile, "r")) == NULL) {
    fprintf(stderr,"Could not open station list from flatfile, %s\n", stationFlatFile);
    exit(0);
  }
  fprintf(stderr,"Opened station list from flatfile, %s\n", stationFlatFile);
//
  if ((fp_vs30File = fopen(vs30File, "r")) == NULL) {
    fprintf(stderr,"Could not open EMT vs30 file, %s\n", vs30File);
    exit(0);
  }
  fprintf(stderr,"Opened EMT vs30 file, %s\n", vs30File);
  fp_outputFile = fopen(outputFile, "w");

// READ/APPEND EVENT-FLATFILE
// header lines
  hlines=1;
  for (cnt1=0; cnt1<hlines; cnt1++) {
    fgets(buff,BUFFLEN,fp_stationFlatFile);
    buff[strcspn(buff, "\r\n")] = 0;
    fprintf(fp_outputFile,"%s,Slope.Vs30,Parker,Hosseini,Zalachoris\n",buff);
  }
  cnt1=0;
// header line from vs30 file
  fgets(buff2,BUFFLEN,fp_vs30File);
// loop over lines from station file
  while( fgets(buff,BUFFLEN,fp_stationFlatFile) ) {
    if ( strlen(buff) > BUFFLEN ) {
      fprintf(stderr,"Increase BUFFLEN from %d.\n", (int)BUFFLEN);
      exit(1);
    }
    buff[strcspn(buff, "\r\n")] = 0;
//     
    columns = NULL;
    cols_found = getcols(buff, delim, &columns);
//fprintf(stderr,"buff: %s\n", buff);
//fprintf(stderr,"columns: %s\n", columns[0]);
    assign_cols_stationfile(columns, &staLon, &staLat, staNet, staNm);
    free(columns);
// read through vs30 file until match input event from flatfile
    while( fgets(buff2,BUFFLEN,fp_vs30File) ) {
      buff2[strcspn(buff2, "\r\n")] = 0;
      columns2 = NULL;
      cols_found = getcols(buff2, delim, &columns2);
      assign_cols_vs30file(columns2, &lon, &lat, network, name, vs30_1, vs30_2, vs30_3, vs30_4);
      free(columns2);
      delaz_(&staLat,&staLon,&lat,&lon,&dist,&az,&baz);
/*
if (dist<0.05) {
//      if ( (diffMag<0.1) && (diffSec<1)) {
fprintf(stderr,"%f\n", dist);
fprintf(stderr,"%f %f %f %f\n", lon, lat, staLon, staLat);
fprintf(stderr,"N1%sN1 N2%sN2\n", name, staNm);
fprintf(stderr,"strcmp(%s,%s) %d\n", name, staNm, strcmp(name,staNm));
} */
      if ((dist<0.05) && (strcmp(name,staNm)==0) ) {
        fprintf(stderr,"MATCH: ");
        fprintf(stderr,"dist=%.1f %s %s\n",dist, staNm, name);
        fprintf(stderr,"%s\n", buff); 
        fprintf(stderr,"%s\n\n", buff2); 
        fprintf(fp_outputFile,"%s,%s,%s,%s,%s\n",buff,vs30_1,vs30_2,vs30_3,vs30_4);
        cnt1++;
//        if ( cnt1 > 100 ) return 0;
        matchSta=1;
        break;
      }
    }
    if ( !matchSta ) fprintf(stderr,"NO MATCH: %s\n", buff); 
    matchSta=0;
    rewind(fp_vs30File);
  }


// close file
  fclose(fp_stationFlatFile); 
  fclose(fp_vs30File);
  fclose(fp_outputFile);


  return 0;
}
