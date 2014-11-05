#include "JSVMPsnr.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


 CJSVMPsnr::CJSVMPsnr(int width,int height, unsigned char* orgBuf,unsigned char* recBuf)
 {
	 
	 FILE* Orig = fopen("org.yuv","wb");
	 FILE* Rec = fopen("rec.yuv","wb");
	m_iHeight = height;
	m_iWidth = width;
	unsigned long iLen = m_iHeight*m_iWidth;
	
	fwrite(orgBuf,sizeof(unsigned char),iLen,Orig);
	fwrite(orgBuf+iLen,sizeof(unsigned char),iLen*1/4,Orig);
	fwrite(orgBuf+iLen*5/4,sizeof(unsigned char),iLen*1/4,Orig);
	fwrite(recBuf,sizeof(unsigned char),iLen,Rec);
	fwrite(recBuf+iLen,sizeof(unsigned char),iLen*1/4,Rec);
	fwrite(recBuf+iLen*5/4,sizeof(unsigned char),iLen*1/4,Rec);
	fclose(Orig);
	fclose(Rec);

	
 }
 CJSVMPsnr::~CJSVMPsnr()
{
	m_iHeight = 0;
	m_iWidth = 0;
	
	uninitYUVFrame();

}
void CJSVMPsnr::createColorComponent( ColorComponent* cc )
{
  if( ! ( cc->data = new unsigned char[cc->width * cc->height]))
  {
    fprintf(stderr, "\nERROR: memory allocation failed!\n\n");
    exit(-1);
  }
}

void CJSVMPsnr::deleteColorComponent( ColorComponent* cc )
{
  delete[] cc->data;
  cc->data = NULL;
}


void CJSVMPsnr::initYUVFrame()
{
	createFrame(&m_OrigFile,m_iWidth,m_iHeight);
    createFrame(&m_RecFile,m_iWidth,m_iHeight);
	
}
void CJSVMPsnr::uninitYUVFrame()
{
	deleteFrame(&m_OrigFile);
	deleteFrame(&m_RecFile);

}
void CJSVMPsnr::createFrame( YuvFrame* f, int width, int height )
{
  
  f->lum.width = width;    f->lum.height  = height;     createColorComponent( &f->lum );
  f->cb .width = width/2;  f->cb .height  = height/2;   createColorComponent( &f->cb  );
  f->cr .width = width/2;  f->cr .height  = height/2;   createColorComponent( &f->cr  );
}

void CJSVMPsnr::deleteFrame( YuvFrame* f )
{
  deleteColorComponent( &f->lum );
  deleteColorComponent( &f->cb  );
  deleteColorComponent( &f->cr  );
}
void CJSVMPsnr::readYUVFrame(FILE* org,FILE* rec)
{

	readFrame(&m_OrigFile,org);
	readFrame(&m_RecFile,rec);

}
double CJSVMPsnr::getFramePSNR()
{
	double psnrU;
	double psnrV;
	DOUBLE psnrY;
	int sequence_length;
	int width  = m_iWidth;
	int height = m_iHeight;

	initYUVFrame();
	FILE* m_fOrig = fopen("org.yuv","rb");
	FILE* m_fRec = fopen("rec.yuv","rb");
	readYUVFrame(m_fOrig,m_fRec);
	getPSNR(psnrY, psnrU, psnrV,m_OrigFile,m_RecFile);
	fclose(m_fOrig);
	fclose(m_fRec);
	return psnrY;

	


}

void CJSVMPsnr::readColorComponent( ColorComponent* cc, FILE* file )
{
  unsigned int size   = cc->width*cc->height;
  unsigned int rsize;

  rsize = (unsigned int)fread( cc->data, sizeof(unsigned char), size, file );

  if( size != rsize )
  {
    fprintf(stderr, "\nERROR: while reading from input file!\n\n");
    exit(-1);
  }
}

void CJSVMPsnr::writeColorComponent( ColorComponent* cc, FILE* file, int downScale )
{
  int outwidth  = cc->width   >> downScale;
  int outheight = cc->height  >> downScale;
  int wsize;

  for( int i = 0; i < outheight; i++ )
  {
    wsize = (int)fwrite( cc->data+i*cc->width, sizeof(unsigned char), outwidth, file );

    if( outwidth != wsize )
    {
      fprintf(stderr, "\nERROR: while writing to output file!\n\n");
      exit(-1);
    }
  }
}

double CJSVMPsnr::psnr( ColorComponent& rec, ColorComponent& org)
{
  unsigned char*  pOrg  = org.data;
  unsigned char*  pRec  = rec.data;
  double          ssd   = 0;
  int             diff;

  for  ( int r = 0; r < rec.height; r++ )
  {
    for( int c = 0; c < rec.width;  c++ )
    {
      diff  = pRec[c] - pOrg[c];
      ssd  += (double)( diff * diff );
    }
    pRec   += rec.width;
    pOrg   += org.width;
  }

  if( ssd == 0.0 )
  {
    return 99.99;
  }
  return ( 10.0 * log10( (double)rec.width * (double)rec.height * 65025.0 / ssd ) );
}

void CJSVMPsnr::getPSNR( double& psnrY, double& psnrU, double& psnrV, YuvFrame& rcFrameOrg, YuvFrame& rcFrameRec )
{

  psnrY = psnr( rcFrameRec.lum, rcFrameOrg.lum );
  psnrU = psnr( rcFrameRec.cb,  rcFrameOrg.cb  );
  psnrV = psnr( rcFrameRec.cr,  rcFrameOrg.cr  );
}

void CJSVMPsnr::readFrame( YuvFrame* f, FILE* file )
{
  readColorComponent( &f->lum, file );
  readColorComponent( &f->cb,  file );
  readColorComponent( &f->cr,  file );
}


//int main(int argc, char *argv[])
//{
//  int     acc = 10000;
//#define   OUT "%d,%04d"
//
//  //===== input parameters =====
//  int           stream          = 0;
//  unsigned int  width           = 0;
//  unsigned int  height          = 0;
//  unsigned int  temporal_stages = 0;
//  unsigned int  skip_at_start   = 0;
//  double        fps             = 0.0;
//  FILE*         org_file        = 0;
//  FILE*         rec_file        = 0;
//  FILE*         str_file        = 0;
//  char*         prefix_string   = 0;
//
//  //===== variables =====
//  unsigned int  index, skip, skip_between, sequence_length;
//  int           py, pu, pv, br;
//  double        bitrate = 0.0;
//  double        psnrY, psnrU, psnrV;
//  YuvFrame      cOrgFrame, cRecFrame;
//  double        AveragePSNR_Y = 0.0;
//  double        AveragePSNR_U = 0.0;
//  double        AveragePSNR_V = 0.0;
//  int		      	currarg = 5;
//  int			      rpsnr   = 0;
//
//
//  //===== read input parameters =====
//  print_usage_and_exit((argc < 5 || (argc > 11 )), argv[0]);
//  width             = atoi  ( argv[1] );
//  height            = atoi  ( argv[2] );
//  org_file          = fopen ( argv[3], "rb" );
//  rec_file          = fopen ( argv[4], "rb" );
//
//  if(( argc >=  6 ) && strcmp( argv[5], "-r" ) )
//  {
//    temporal_stages = atoi  ( argv[5] );
//    currarg++;
//  }
//  if(( argc >=  7 ) && strcmp( argv[6], "-r" ) )
//  {
//    skip_at_start   = atoi  ( argv[6] );
//    currarg++;
//  }
//  if(( argc >= 9 ) && strcmp( argv[7], "-r" ) )
//  {
//    str_file        = fopen ( argv[7], "rb" );
//	  print_usage_and_exit(!strcmp( argv[8], "-r" ), argv[0]);
//	  fps             = atof  ( argv[8] );
//    stream          = 1;
//	  currarg+=2;
//  }
//  if(( argc >= 10 ) && strcmp( argv[9], "-r" ) )
//  {
//    prefix_string   = argv[9];
//    currarg++;
//  }
//
//	if(currarg < argc )
//	{
//	  if(!strcmp( argv[currarg], "-r" ))
//		  rpsnr=1;
//	  else
//      print_usage_and_exit (true,argv[0],"Wrong number of argument!" );
//	}
//
//
//  //===== check input parameters =====
//  print_usage_and_exit  ( ! org_file,                                       argv[0], "Cannot open original file!" );
//  print_usage_and_exit  ( ! rec_file,                                       argv[0], "Cannot open reconstructed file!" );
//  print_usage_and_exit  ( ! str_file && stream,                             argv[0], "Cannot open stream!" );
//  print_usage_and_exit  ( fps <= 0.0 && stream,                             argv[0], "Unvalid frames per second!" );
//
//  //======= get number of frames and stream size =======
//  fseek(    rec_file, 0, SEEK_END );
//  fseek(    org_file, 0, SEEK_END );
//  size_t rsize = ftell( rec_file );
//  size_t osize = ftell( org_file );
//  fseek(    rec_file, 0, SEEK_SET );
//  fseek(    org_file, 0, SEEK_SET );
//
//  if (rsize < osize)
//    sequence_length = (unsigned int)((double)rsize/(double)((width*height*3)/2));
//   else
//    sequence_length = (unsigned int)((double)osize/(double)((width*height*3)/2));
//
//  if( stream )
//  {
//    fseek(  str_file, 0, SEEK_END );
//    bitrate       = (double)ftell(str_file) * 8.0 / 1000.0 / ( (double)(sequence_length << temporal_stages) / fps );
//    fseek(  str_file, 0, SEEK_SET );
//  }
//  skip_between    = ( 1 << temporal_stages ) - 1;
//
//  //===== initialization ======
//  createFrame( &cOrgFrame, width, height );
//  createFrame( &cRecFrame, width, height );
//
//  //===== loop over frames =====
//  for( skip = skip_at_start, index = 0; index < sequence_length; index++, skip = skip_between )
//  {
//    fseek( org_file, skip*width*height*3/2, SEEK_CUR);
//
//    readFrame       ( &cOrgFrame, org_file );
//    readFrame       ( &cRecFrame, rec_file );
//
//    getPSNR         ( psnrY, psnrU, psnrV, cOrgFrame, cRecFrame);
//    AveragePSNR_Y +=  psnrY;
//    AveragePSNR_U +=  psnrU;
//    AveragePSNR_V +=  psnrV;
//
//    py = (int)floor( acc * psnrY + 0.5 );
//    pu = (int)floor( acc * psnrU + 0.5 );
//    pv = (int)floor( acc * psnrV + 0.5 );
//    fprintf(stdout,"%d\t"OUT"\t"OUT"\t"OUT"\n",index,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//  }
//  fprintf(stdout,"\n");
//
//  py = (int)floor( acc * AveragePSNR_Y / (double)sequence_length + 0.5 );
//  pu = (int)floor( acc * AveragePSNR_U / (double)sequence_length + 0.5 );
//  pv = (int)floor( acc * AveragePSNR_V / (double)sequence_length + 0.5 );
//  br = (int)floor( acc * bitrate                                 + 0.5 );
//  if( stream )
//  {
//    if( prefix_string )
//    {
//      fprintf(stderr,"%s\t"OUT"\t"OUT"\t"OUT"\t"OUT"\n",prefix_string,br/acc,br%acc,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//      fprintf(stdout,"%s\t"OUT"\t"OUT"\t"OUT"\t"OUT"\n",prefix_string,br/acc,br%acc,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//    }
//    else
//    {
//      fprintf(stderr,OUT"\t"OUT"\t"OUT"\t"OUT"\n",br/acc,br%acc,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//      fprintf(stdout,OUT"\t"OUT"\t"OUT"\t"OUT"\n",br/acc,br%acc,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//    }
//  }
//  else
//  {
//    fprintf(stderr,"total\t"OUT"\t"OUT"\t"OUT"\n",py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//    fprintf(stdout,"total\t"OUT"\t"OUT"\t"OUT"\n",py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//  }
//
//  fprintf(stdout, "\n");
//
//
//  //===== finish =====
//  deleteFrame( &cOrgFrame );
//  deleteFrame( &cRecFrame );
//  fclose     ( org_file   );
//  fclose     ( rec_file   );
//  if( stream )
//  {
//    fclose   ( str_file   );
//  }
//
//  return (rpsnr*py);
//}
//
//
//int JSVMPsnr(int width,int height,char* OrgFile,char* RecFile)
//{
//  int     acc = 10000;
//#define   OUT "%d,%04d"
//
//  //===== input parameters =====
//  int           stream          = 0;
//  unsigned int  width           = 0;
//  unsigned int  height          = 0;
//  unsigned int  temporal_stages = 0;
//  unsigned int  skip_at_start   = 0;
//  double        fps             = 0.0;
//  FILE*         org_file        = 0;
//  FILE*         rec_file        = 0;
//  FILE*         str_file        = 0;
//  char*         prefix_string   = 0;
//
//  //===== variables =====
//  unsigned int  index, skip, skip_between, sequence_length;
//  int           py, pu, pv, br;
//  double        bitrate = 0.0;
//  double        psnrY, psnrU, psnrV;
//  YuvFrame      cOrgFrame, cRecFrame;
//  double        AveragePSNR_Y = 0.0;
//  double        AveragePSNR_U = 0.0;
//  double        AveragePSNR_V = 0.0;
//  int		      	currarg = 5;
//  int			      rpsnr   = 0;
//
//
//  //===== read input parameters =====
//  //print_usage_and_exit((argc < 5 || (argc > 11 )), argv[0]);
// /* width             = atoi  ( argv[1] );
//  height            = atoi  ( argv[2] );*/
//  org_file          = fopen ( OrgFile, "rb" );
//  rec_file          = fopen ( RecFile, "rb" );
//
// // if(( argc >=  6 ) && strcmp( argv[5], "-r" ) )
// // {
// //   temporal_stages = atoi  ( argv[5] );
// //   currarg++;
// // }
// // if(( argc >=  7 ) && strcmp( argv[6], "-r" ) )
// // {
// //   skip_at_start   = atoi  ( argv[6] );
// //   currarg++;
// // }
// // if(( argc >= 9 ) && strcmp( argv[7], "-r" ) )
// // {
// //   str_file        = fopen ( argv[7], "rb" );
//	//  print_usage_and_exit(!strcmp( argv[8], "-r" ), argv[0]);
//	//  fps             = atof  ( argv[8] );
// //   stream          = 1;
//	//  currarg+=2;
// // }
// // if(( argc >= 10 ) && strcmp( argv[9], "-r" ) )
// // {
// //   prefix_string   = argv[9];
// //   currarg++;
// // }
//
//	//if(currarg < argc )
//	//{
//	//  if(!strcmp( argv[currarg], "-r" ))
//	//	  rpsnr=1;
//	//  else
// //     print_usage_and_exit (true,argv[0],"Wrong number of argument!" );
//	//}
//
//
// // //===== check input parameters =====
// // print_usage_and_exit  ( ! org_file,                                       argv[0], "Cannot open original file!" );
// // print_usage_and_exit  ( ! rec_file,                                       argv[0], "Cannot open reconstructed file!" );
// // print_usage_and_exit  ( ! str_file && stream,                             argv[0], "Cannot open stream!" );
// // print_usage_and_exit  ( fps <= 0.0 && stream,                             argv[0], "Unvalid frames per second!" );
//
//  //======= get number of frames and stream size =======
//  fseek(    rec_file, 0, SEEK_END );
//  fseek(    org_file, 0, SEEK_END );
//  size_t rsize = ftell( rec_file );
//  size_t osize = ftell( org_file );
//  fseek(    rec_file, 0, SEEK_SET );
//  fseek(    org_file, 0, SEEK_SET );
//
//  if (rsize < osize)
//    sequence_length = (unsigned int)((double)rsize/(double)((width*height*3)/2));
//   else
//    sequence_length = (unsigned int)((double)osize/(double)((width*height*3)/2));
//
//  if( stream )
//  {
//    fseek(  str_file, 0, SEEK_END );
//    bitrate       = (double)ftell(str_file) * 8.0 / 1000.0 / ( (double)(sequence_length << temporal_stages) / fps );
//    fseek(  str_file, 0, SEEK_SET );
//  }
//  skip_between    = ( 1 << temporal_stages ) - 1;
//
//  //===== initialization ======
//  createFrame( &cOrgFrame, width, height );
//  createFrame( &cRecFrame, width, height );
//
//  //===== loop over frames =====
//  for( skip = skip_at_start, index = 0; index < sequence_length; index++, skip = skip_between )
//  {
//    fseek( org_file, skip*width*height*3/2, SEEK_CUR);
//
//    readFrame       ( &cOrgFrame, org_file );
//    readFrame       ( &cRecFrame, rec_file );
//
//    getPSNR         ( psnrY, psnrU, psnrV, cOrgFrame, cRecFrame);
//    AveragePSNR_Y +=  psnrY;
//    AveragePSNR_U +=  psnrU;
//    AveragePSNR_V +=  psnrV;
//
//    py = (int)floor( acc * psnrY + 0.5 );
//    pu = (int)floor( acc * psnrU + 0.5 );
//    pv = (int)floor( acc * psnrV + 0.5 );
//    fprintf(stdout,"%d\t"OUT"\t"OUT"\t"OUT"\n",index,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//  }
//  fprintf(stdout,"\n");
//
//  py = (int)floor( acc * AveragePSNR_Y / (double)sequence_length + 0.5 );
//  pu = (int)floor( acc * AveragePSNR_U / (double)sequence_length + 0.5 );
//  pv = (int)floor( acc * AveragePSNR_V / (double)sequence_length + 0.5 );
//  br = (int)floor( acc * bitrate                                 + 0.5 );
//  if( stream )
//  {
//    if( prefix_string )
//    {
//      fprintf(stderr,"%s\t"OUT"\t"OUT"\t"OUT"\t"OUT"\n",prefix_string,br/acc,br%acc,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//      fprintf(stdout,"%s\t"OUT"\t"OUT"\t"OUT"\t"OUT"\n",prefix_string,br/acc,br%acc,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//    }
//    else
//    {
//      fprintf(stderr,OUT"\t"OUT"\t"OUT"\t"OUT"\n",br/acc,br%acc,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//      fprintf(stdout,OUT"\t"OUT"\t"OUT"\t"OUT"\n",br/acc,br%acc,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//    }
//  }
//  else
//  {
//    fprintf(stderr,"total\t"OUT"\t"OUT"\t"OUT"\n",py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//    fprintf(stdout,"total\t"OUT"\t"OUT"\t"OUT"\n",py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
//  }
//
//  fprintf(stdout, "\n");
//
//
//  //===== finish =====
//  deleteFrame( &cOrgFrame );
//  deleteFrame( &cRecFrame );
//  fclose     ( org_file   );
//  fclose     ( rec_file   );
//  if( stream )
//  {
//    fclose   ( str_file   );
//  }
//
//  return (rpsnr*py);
//}
//
