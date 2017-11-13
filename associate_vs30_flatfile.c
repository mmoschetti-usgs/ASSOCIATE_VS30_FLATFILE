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
  staNet=strcpy(staNet,columns[1]);
  staNm=strcpy(staNm,columns[2]);
  *staLon=atof(columns[3]);
  *staLat=atof(columns[4]);
//  fprintf(stderr,"assign_cols_flatfile, year/month/day/hour/min/sec/Mag: %d %d %d %d %d %.2f %f\n", *evYear, *evMon, *evDay, *evHour, *evMin, *evSec, *evMag);

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
//  fprintf(stderr,"assign_cols_catalog, year/month/day/hour/min/sec/Mag/MagSource/MagString: %d %d %d %d %d %.2f %f %f %s\n", *evYear, *evMon, *evDay, *evHour, *evMin, *evSec, *evMag, *evMagSource, magString);

}


/*--------------------------------------------------------------------------*/
int main (int argc, char *argv[])
/*--------------------------------------------------------------------------*/
{
  FILE *fp_stationFlatFile, *fp_vs30File, *fp_outputFile;
  int hlines, cnt1;
  int cols_found;
//  int evYear, evMon, evDay, evHour, evMin;
//  int evYear2, evMon2, evDay2, evHour2, evMin2;
//  int epochTimeFlatFile, epochTimeCatalog; 
//  int diffSec;
//  float evMag, evMag2, evSec, evSec2, evMagSource;
  float staLon, staLat, lon, lat;
  float az, baz, dist;
//  float diffMag;
  char vs30_1[20], vs30_2[20], vs30_3[20], vs30_4[20];
  char network[5], staNet[5], name[5], staNm[5];
  char stationFlatFile[200], vs30File[200], outputFile[200];
//  char magString[20];
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
  if ((fp_vs30File = fopen(vs30File, "r")) == NULL) {
    fprintf(stderr,"Could not open EMT vs30 file, %s\n", vs30File);
    exit(0);
  }
  fp_outputFile = fopen(outputFile, "w");

// READ/APPEND EVENT-FLATFILE
// header lines
  hlines=1;
  for (cnt1=0; cnt1<hlines; cnt1++) {
    fgets(buff,BUFFLEN,fp_stationFlatFile);
    fprintf(fp_outputFile,"%s",buff);
  }
  cnt1=0;
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
//void assign_cols_stationfile(char **columns, float *staLon, float *staLat, char *staNet, char *staNm);
//void assign_cols_vs30file(char **columns, float *lon, float *lat, char *network, char *name);
    assign_cols_stationfile(columns, &staLon, &staLat, staNet, staNm);
    free(columns);
//    epochTimeFlatFile=compute_epochTime(evYear,evMon,evDay,evHour,evMin,(int)evSec);
// read through vs30 file until match input event from flatfile
    while( fgets(buff2,BUFFLEN,fp_vs30File) ) {
      buff2[strcspn(buff2, "\r\n")] = 0;
      columns2 = NULL;
      cols_found = getcols(buff2, delim, &columns2);
      assign_cols_vs30file(columns2, &lon, &lat, network, name, vs30_1, vs30_2, vs30_3, vs30_4);
      free(columns2);
//      epochTimeCatalog=compute_epochTime(evYear2,evMon2,evDay2,evHour2,evMin2,(int)evSec2);
//      diffSec=abs(epochTimeFlatFile-epochTimeCatalog);
      delaz_(&staLat,&staLon,&lat,&lon,&dist,&az,&baz);
//      if ( (diffMag<0.1) && (diffSec<1)) {
      if ( dist<0.01 ) {
        fprintf(stderr,"MATCH: ");
        fprintf(stderr,"dist=%.1f\n",dist);
        fprintf(stderr,"%s\n", buff); 
        fprintf(stderr,"%s\n\n", buff2); 
        fprintf(fp_outputFile,"%s,%s,%s,%s,%s\n",buff,vs30_1,vs30_2,vs30_3,vs30_4);
        return 0;
        break;
      }
    }
    rewind(fp_vs30File);
  }


// close file
  fclose(fp_stationFlatFile); 
  fclose(fp_vs30File);
  fclose(fp_outputFile);


  return 0;
}
