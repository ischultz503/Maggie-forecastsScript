/* getWindFcst.c 
 * Written by David Siuta, Greg West, Tim Chui - UBC/BCH 
 * Last revision on 01 December 2014 by dsiuta.
 * Contact info: dsiuta@eos.ubc.ca or gwest@eos.ubc.ca
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

char *strdup(const char *src)
{
	char *tmp = malloc(strlen(src) + 1);
	if(tmp)
		strcpy(tmp, src);
	return tmp;
}
//Function "explode" (like in php) pilfered from Pablo on StackExchange
void explode(const char *src, const char *tokens, char ***list, size_t *len)
{   
	if(src == NULL || list == NULL || len == NULL)
		return;

	char *str, *copy, **_list = NULL, **tmp;
	*list = NULL;
	*len  = 0;

	copy = strdup(src);
	if(copy == NULL)
		return;

	str = strtok(copy, tokens);
	if(str == NULL)
		goto free_and_exit;

	_list = realloc(NULL, sizeof *_list);
	if(_list == NULL)
		goto free_and_exit;

	_list[*len] = strdup(str);
	if(_list[*len] == NULL)
		goto free_and_exit;
	(*len)++;


	while((str = strtok(NULL, tokens)))
	{   
		tmp = realloc(_list, (sizeof *_list) * (*len + 1));
		if(tmp == NULL)
		goto free_and_exit;
	
		_list = tmp;
	
		_list[*len] = strdup(str);
		if(_list[*len] == NULL)
			goto free_and_exit;
		(*len)++;
	}


free_and_exit:
	*list = _list;
	free(copy);
}

////////////////////////////////////////////////////////////////////
int main ( int argc, char *argv[] ) {


	//Read in date argument, all arguments required to run, or get a failure warning
	const char *sdate;
	const char *sh;
	const char *sl;
	const char *sfbeg;
	const char *sflength;
	const char *v;
	const char *prep;
	const char *postproc;
	const char *soutfreq;
	const char *stnp;
	const char *memp;

	if (argc == 12) {
		sdate = argv[1];
		sh = argv[2];
		sl = argv[3];
   	        sfbeg = argv[4];
		sflength = argv[5];
		v = argv[6];
		prep = argv[7];
		postproc = argv[8];
		soutfreq = argv[9];
		stnp = argv[10];
		memp = argv[11];
	}
  	else
	{
		perror("Expected arg: YYYYMMDD H LEVEL START_OFFSET(min) FLENGTH_FROM_START(hours) VARIABLE PreProc PostProc DESIRED_OUTPUT_FREQ(min) STNCONFIG_PATH MEMBERCONFIG_PATH \n");
		exit(EXIT_FAILURE);
	}
	
	int date = atoi(sdate);
	int h = atoi(sh);
	int l = atoi(sl);
	
	int flength=atoi(sflength);    //forecast length in hours (will create forecast output array length, can be input arg)
	int fbeg=atoi(sfbeg);	       //DESIRED BEGINNING FORECAST OFFSET.  540 IS 1AM PST.  
	int outfreq=atoi(soutfreq);    //DESIRED OUTPUT FREQUENCY IN MINUTES, USED TO DEFINE FORECAST ARRAY
	//Read in config files that have stn and ensemble member info
	//char stnpath[] = "/nfs/crypt/arena/users/model/setup/wpVerif2.cfg";
	//char mempath[] = "/nfs/crypt/arena/users/model/setup/windMems.cfg";
	char stnpath[100];
	strcpy(stnpath,stnp);
	char mempath[100];
	strcpy(mempath,memp);
	FILE *stnf;
	FILE *memf;
	char line[100];
	char **list;
	int k;
	size_t i, len;
	stnf = fopen(stnpath, "r");
	memf = fopen(mempath, "r");
	if(stnf == NULL) {
		perror("Error opening stnpath");
		exit(EXIT_FAILURE);
	}
	else
	{
		int ch, number_of_lines = 0; //here we read the number of lines in wpVerif2.cfg.  This tells the number of stations dynamically (ie flexible for adjustment

		do 
		{
			ch = fgetc(stnf);
			if(ch == '\n')
				number_of_lines++;
		} while (ch != EOF);
		fclose(stnf);
		stnf = fopen(stnpath, "r");
		//Read stn file line by line
		int stnid[number_of_lines];  //line is contents of wpVerif2.cfg stored as "character" variable type.  stnid array must be the same variable type.  this may have to be changed to int for API to work.
		k=0;
		while( fgets(line, 100, stnf) != NULL ) 
		{
			explode(line, " ", &list, &len);
			stnid[k]=atoi(list[3]);  //here we store the station ID's in an array (from wpVerif2.cfg)
			//printf("%d\n",stnid[k]);
			for(i = 0; i < len; ++i)
	    			free(list[i]);
				free(list);
				++k;
		}
   
	if(memf == NULL) {
		perror("Error opening stnpath");
		exit(EXIT_FAILURE);
	}
	else
	{
		int ch, members = 0; //here we read the number of lines in wpVerif2.cfg.  This tells the number of stations dynamically (ie flexible for adjustment

		do 
		{
			ch = fgetc(memf);
			if(ch == '\n')
				members++;
		} while (ch != EOF);
		fclose(memf);
		memf = fopen(mempath, "r");
		//Read stn file line by line
		char *modelid[members][50];  //line is contents of wpVerif2.cfg stored as "character" variable type.  stnid array must be the same variable type.  this may have to be changed to int for API to work.
		char *modelic[members][50];
		char *ic[members][50];
		float res[members];
		float *fcst[members];
		int *offset[members];
		int *size[members];
		int infreq[members];
		int stagger[members];
		int mlength[members];
		k=0;
		while( fgets(line, 100, memf) != NULL ) 
		{
			explode(line, " ", &list, &len);
			strcpy((char *)modelid[k],list[1]);  //here we store the model ID's in an array (from windMems.cfg)
			strcpy((char *)modelic[k],list[2]);
			strcpy((char *)ic[k],list[3]);
			res[k]=atof(list[4]);
			infreq[k]=atof(list[5]);
			stagger[k]=atoi(list[6]);
			mlength[k]=atoi(list[7]);
			fcst[k]=NULL;
			offset[k]=NULL;
			size[k]=0;
			for(i = 0; i < len; ++i)
	    		free(list[i]);
			free(list);
			++k;
			
		} 
    
	char PreProc[50];
	strcpy(PreProc,prep);
	
	
    	char PP[50];
	strcpy(PP,postproc);

    	int initt=h;
    	int level=l;
   	int maxoff=(60*flength)+fbeg;
	char var[50];
	strcpy(var,v);
    	int ind;
	int j;
	int size01=0;
	int sizes[members];

	
	//loop through get_forecast API
	//loop over stns (stn config file)
	for (ind=0; ind<number_of_lines; ind++) {
		char the_file[50];
                sprintf(the_file, "offsets_%d_%d.txt", date, stnid[ind]);
                const char* the_outfile=the_file;
                FILE *offset_file;
                offset_file=fopen(the_outfile, "a");
		//loop over ens mems (model config file)
		for (j=0; j<members; j++) {
			size01 = get_forecast_(&modelid[j], &res[j],&PreProc,&PP,&ic[j],&date,&initt,&var,&level,&stnid[ind],&maxoff,&fcst[j],&offset[j],NULL,NULL);
			sizes[j]=size01;
		        //printf("%d\n",sizes[j]); //checks amount of forecasts available
		}
		//need to store values beginning at offset 540 (1am PST)  This location in the forecast array varies by member
		int fend=fbeg + (flength * outfreq);  //this defines the ending dimension for the forecast output array below.  we need to organize the forecasts by offset and pick only those needed.
		int dims=(flength * (60 / outfreq)); 
		//Now we need to loop through the temporary forecast array (fcst) and decide to keep or discard irrelevent times.  kept times will be stored in a final forecast array that will be used to output to a file.
		//Outer loop is by members
		//inner loop searchs forecast offset and decides to keep or discard
		int mems,off,index,index2;
		//float forecast[members][dims];
		float forecast[members][dims+1];
		int count;
		int out_dims;
		int fcst_dims;
		char a_file[50];
		sprintf(a_file, "indexes_%d_%d.txt", date, stnid[ind]);
		const char* a_outfile=a_file;
		FILE *index_file;
		index_file=fopen(a_outfile, "a");
		///If the forecast does not exist at a station, no forecast or offsets are given from the upper loop.  Need to create an if clause that gets around this
 		
		for (mems=0; mems<members; mems++) {
			
			int i=0;
			//need to align members with different offset interval.  should be done by reading config file
			if ( sizes[mems] == 0 ) {
				
				for (count=0; count<dims; count++) {
					out_dims=count;
					fcst_dims=-999;
					forecast[mems][out_dims]=-999;
					fprintf(index_file, "%d %d %d %6.2f %s %6.2f\n", out_dims, fcst_dims, infreq[mems], forecast[mems][out_dims], modelic[mems], res[mems]);
				}
			}
			else {
				//for (off=0; off<fend; off+=outfreq) {
				for (off=0; off<=fend; off+=outfreq) {
					// print offsets to files
					fprintf(offset_file, "%d\n", offset[mems][i]);
					i++;
					////
					//if (offset[mems][i]==0) {
					//}
					if ( off < ((mlength[mems]+1)*60) ) {
						if ( offset[mems][(off/infreq[mems])] < fbeg ) {
						}
						else {
							if ( stagger[mems] == 0 ) {
								out_dims=((off-fbeg+infreq[mems])/outfreq);
								fcst_dims=((off+infreq[mems]-outfreq)/infreq[mems]);
								forecast[mems][out_dims]=fcst[mems][fcst_dims]/1.;
								fprintf(index_file, "%d %d %d %6.2f %s %6.2f\n", out_dims, fcst_dims, infreq[mems], forecast[mems][out_dims], modelic[mems], res[mems]);
							}
							else {
								out_dims=((off-fbeg+(stagger[mems]*infreq[mems])+infreq[mems])/outfreq);
								fcst_dims=(off/infreq[mems]);
								forecast[mems][out_dims]=fcst[mems][fcst_dims]/1.;
								fprintf(index_file, "%d %d %d %6.2f %s %6.2f\n", out_dims, fcst_dims, infreq[mems], forecast[mems][out_dims], modelic[mems], res[mems]);
							}
						}
					}
					else {
						out_dims=((off-fbeg)/outfreq);
						fcst_dims=-999;
						forecast[mems][out_dims]=-999;
						fprintf(index_file, "%d %d %d %6.2f %s %6.2f\n", out_dims, fcst_dims, infreq[mems], forecast[mems][out_dims], modelic[mems], res[mems]);
					}	
				}
			}
		}
		fclose(index_file);
		fclose(offset_file);
		//here we print the final forecast array, in columns, by ensemble member for output test.  rows=forecast offset from desired start
		int out,num;


		//need to open an output file to print the forecasts to.
		FILE *ofp;
		char file[50];
		for (out=0; out<dims; out++) {
			if ( out<(179*60/outfreq) ) {		
				sprintf(file, "%d_%d_%s.txt", date, stnid[ind],var);
				const char* outputFile=file; 
				ofp = fopen(outputFile, "a");
			}
			else if ( out>=(180*60/outfreq) && out<(181*60/outfreq) ) {
				sprintf(file, "%d_%d_%sday2.txt", date, stnid[ind],var);
				const char* outputFile=file;
				ofp = fopen(outputFile, "a");
			}
			else if ( out>=(182*60/outfreq) && out<(183*60/outfreq) ) {
				sprintf(file, "%d_%d_%sday3.txt", date, stnid[ind],var);
             			const char* outputFile=file;
             			ofp = fopen(outputFile, "a");
			}
			else if ( out>=(184*60/outfreq) && out<(185*60/outfreq) ) {
				sprintf(file, "%d_%d_%sday4.txt", date, stnid[ind],var);
             			const char* outputFile=file;
             			ofp = fopen(outputFile, "a");
            		}	
        		else if ( out>=(186*60/outfreq) && out<(187*60/outfreq) ) {
				sprintf(file, "%d_%d_%sday5.txt", date, stnid[ind],var);
             			const char* outputFile=file;
             			ofp = fopen(outputFile, "a");
            		}
			else if ( out>=(188*60/outfreq) && out<(189*60/outfreq) ) {
				sprintf(file, "%d_%d_%sday6.txt", date, stnid[ind],var);
             			const char* outputFile=file;
             			ofp = fopen(outputFile, "a");
            		}
			else if ( out>=(190*60/outfreq) && out<(191*60/outfreq) ) {
				sprintf(file, "%d_%d_%sday7.txt", date, stnid[ind],var);
             			const char* outputFile=file;
             			ofp = fopen(outputFile, "a");
            		}
			else {
				perror("Need to adjust available forecast length past 7 Days in script\n");
			        exit(EXIT_FAILURE);
			}
		
			for (num=0; num<members; num++) { 
				if ( forecast[num][out]<-50 || forecast[num][out]>500 ) {
					forecast[num][out]=-999;
				}
		
				fprintf(ofp,"%.2f ", forecast[num][out]);
	    		}
		
		
			fprintf(ofp,"\n");
		
			fclose(ofp);
	 		char cmd[100];
			sprintf(cmd, "sed -i 's/-999.00/-999/g' ""%s", file);
			system(cmd);
		}

	}//////////ADDED TO CLOSE LOOP AROUND API FUNCTION

fclose(memf);
}
fclose(stnf);
}
exit(EXIT_SUCCESS);
	}


