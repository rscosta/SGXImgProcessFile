#include <string.h>

#include "sgx_urts.h"
#include "ImgProcessEnclave_u.h"

#include "getopt.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cpuidh.h" //benchmark

#define VERSION   "1.0.180602"
#define AUTHOR   "Ricardo Costa"

#define MAX_FILE_NAME_SIZE  512

#define ENCLAVE_FILE "ImgProcessEnclave.signed.so"

int rotateImage(sgx_enclave_id_t eid, const char* input, const char* output, int mode)
{
    FILE *ifp = NULL;
    FILE *ofp = NULL;
    size_t fpRes = 0;
    
    if((ifp = fopen(input, "r")) == NULL)
    {
      printf("[ROTATE] Input File %s not found!\n",input);
      return -1;
    }

    if((ofp = fopen(output, "w+")) == NULL)
    {
      printf("[ROTATE] Error while creating output File %s\n",output);
      fclose(ifp);
      return -1;
    }

    fseek(ifp, 0, SEEK_END);
    
    long fsize = ftell(ifp);
    
    fseek(ifp, 0, SEEK_SET);  //same as rewind(f);
    
    unsigned char *inBuffer = (unsigned char*)malloc(fsize + 1);

    unsigned char *outBuffer = (unsigned char*)malloc(fsize + 1);
    
    size_t readRes = fread(inBuffer, fsize, 1, ifp);
    
    start_time();

    switch(mode)
    {
      case 1:
	sgxRotateImage180(eid, inBuffer, fsize, outBuffer, fsize);
      break;
      case 2:
	sgxRotateImageRight(eid, inBuffer, fsize, outBuffer, fsize);
      break;
    }
    
    end_time();

    fwrite(outBuffer, fsize, 1, ofp);

    fclose(ifp);
    fclose(ofp);

    printf("[ROTATE] Rotated image file (%s) created!\n",output);
    printf("[ROTATE] Final image process time: %6.6f seconds.\n", secs);
}

int mirrorImage(sgx_enclave_id_t eid, const char* input, const char* output)
{
    FILE *ifp = NULL;
    FILE *ofp = NULL;
    size_t fpRes = 0;
    
    if((ifp = fopen(input, "r")) == NULL)
    {
      printf("[MIRROR] Input File %s not found!\n",input);
      return -1;
    }

    if((ofp = fopen(output, "w+")) == NULL)
    {
      printf("[MIRROR] Error while creating output File %s\n",output);
      fclose(ifp);
      return -1;
    }

    fseek(ifp, 0, SEEK_END);
    
    long fsize = ftell(ifp);
    
    fseek(ifp, 0, SEEK_SET);  //same as rewind(f);
    
    unsigned char *inBuffer = (unsigned char*)malloc(fsize + 1);

    unsigned char *outBuffer = (unsigned char*)malloc(fsize + 1);
    
    size_t readRes = fread(inBuffer, fsize, 1, ifp);
    
    start_time();

    sgxMirrorImage(eid, inBuffer, fsize, outBuffer, fsize);
    
    end_time();

    fwrite(outBuffer, fsize, 1, ofp);

    fclose(ifp);
    fclose(ofp);

    printf("[MIRROR] Mirrored image file (%s) created!\n",output);
    printf("[MIRROR] Final image process time: %6.6f seconds.\n", secs);
}

void printDebug(const char *buf)
{
    printf("ENCLAVE: %s\n", buf);
}

void printAppUsage()
{
    printf("\nsgxImgProcessFile - App for Processing BMP Image File using Intel SGX. Version:%s Author:%s\n", VERSION, AUTHOR);  
    printf("Usage: sgxImgProcessFile [OPTIONS] [FILE]\n\n");
    printf("Options:\n");
    printf(" -r\trotate mode [right | 180]\n");
    printf(" -m\tmirror mode\n");
    printf(" -i\tinput file\n");
    printf(" -o\toutput file\n");
    printf("Example (Rotate 180): sgxImgProcessFile -r 180 -i [INPUT_FILE] -o [OUTPUT_FILE]\n");
}

int main(int argc, char *argv[])
{
    int option = 0;
    int mode = 0;
    int n_modes = 0;
    char inFileName[MAX_FILE_NAME_SIZE];
    char outFileName[MAX_FILE_NAME_SIZE];
    
    // Specifying the expected options 
    while ((option = getopt(argc, argv,"mr:i:o:")) != -1) {
        switch (option) {
	  case 'r' : 
	              if(optarg == NULL)
			exit(EXIT_FAILURE);
	       
		      if(strcmp(optarg,"180") == 0)
			mode = 1; /*Rotate 180 Mode */
	              else if(strcmp(optarg,"right") == 0)
			mode = 2; /*Rotate Right Mode */
	              else
			exit(EXIT_FAILURE);
		      
		      n_modes++;
                break;
          case 'm' : 
                      mode = 3; /*Mirror Mode */
		      n_modes++;
                break;
	  case 'i' : 
			if(optarg == NULL)
			  exit(EXIT_FAILURE);
			  
			  strncpy(inFileName, optarg, MAX_FILE_NAME_SIZE); 
                 break;
             case 'o' : 
			if(optarg == NULL)
			  exit(EXIT_FAILURE);
			  
			  strncpy(outFileName, optarg, MAX_FILE_NAME_SIZE); 
                 break;
	     default: printAppUsage(); 
                 exit(EXIT_FAILURE);
        }
    }
    
    //Check if it is invalid mode
    if ((mode == 0)||(n_modes != 1))
    {
	printAppUsage();
	exit(EXIT_FAILURE);
    }
      
     // Setup enclave 
    sgx_enclave_id_t eid;
    sgx_status_t ret;
    sgx_launch_token_t token = { 0 };
    int token_updated = 0;
	
    //Init enclave
    ret = sgx_create_enclave(ENCLAVE_FILE, SGX_DEBUG_FLAG, &token, &token_updated, &eid, NULL);
    
    if (ret != SGX_SUCCESS)
    {
	printf("sgx_create_enclave failed: %#x\n", ret);
	exit(EXIT_FAILURE);
    }

    switch(mode)
    {
      case 1: 
      case 2: 
	  rotateImage(eid, inFileName, outFileName, mode);
	break;
      case 3:
	  mirrorImage(eid, inFileName, outFileName);
	break;
    }
    
    //Destroy Enclave
    sgx_destroy_enclave(eid);
	
    return 0;
}

