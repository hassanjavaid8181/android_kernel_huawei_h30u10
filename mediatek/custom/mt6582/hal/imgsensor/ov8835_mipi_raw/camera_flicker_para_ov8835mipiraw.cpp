//[Sensor]
//name = OV8835mipiraw
//
//[Preview]
//read_freq = 208000000
//pixel_line = 3608
//column_length = 1175
//noise_a0 = 0.0000175
//noise_a1 = 0.0024
//
//[ZSD]
//read_freq = 260000000
//pixel_line = 3608
//column_length = 2375
//noise_a0 = 0.0000175
//noise_a1 = 0.0024
//
//[vPreview]
//read_freq = 260000000
//pixel_line = 3608
//column_length = 1775
//noise_a0 = 0.0000175
//noise_a1 = 0.0024

#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov8835mipiraw.h"
#include "camera_info_ov8835mipiraw.h"
#include "camera_custom_AEPlinetable.h"
//#include "camera_custom_flicker_table.h"
#include "camera_custom_flicker_para.h"
#include <cutils/xlog.h>


static void get_flicker_para_by_preview(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] = { 80, 100, 120, 140, 160, 170, 190, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {1117, 416, 777, -394};
  FLICKER_CUST_STATISTICS EV60_L50 = {1332, 438, 1157, -468};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};
  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=25;
  para->flickerSearchRange=20;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=2;
  para->freq_feature_index[1]=1;
}

static void get_flicker_para_by_ZSD(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 70, 90, 100, 110, 120, 140, 160, 190, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {856, 543, 1014, -462};
  FLICKER_CUST_STATISTICS EV60_L50 = {1021, 571, 1509, -536};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=24;
  para->flickerSearchRange=40;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=4;
  para->freq_feature_index[1]=2;
}

static void get_flicker_para_by_vPreview(FLICKER_CUST_PARA* para)
{
  int i;
  int freq[9] =  { 90, 100, 120, 130, 160, 170, 190, 210, 230};
  FLICKER_CUST_STATISTICS EV50_L50 = {-194, 4721, 381, -766};
  FLICKER_CUST_STATISTICS EV50_L60 = {1041, 446, 833, -412};
  FLICKER_CUST_STATISTICS EV60_L50 = {1242, 470, 1241, -486};
  FLICKER_CUST_STATISTICS EV60_L60 = {-162, 2898, 247, -642};

  for(i=0;i<9;i++)
  {
    para->flickerFreq[i]=freq[i];
  }
  para->flickerGradThreshold=23;
  para->flickerSearchRange=28;
  para->minPastFrames=3;
  para->maxPastFrames=14;
  para->EV50_L50=EV50_L50;
  para->EV50_L60=EV50_L60;
  para->EV60_L50=EV60_L50;
  para->EV60_L60=EV60_L60;
  para->EV50_thresholds[0]=-30;
  para->EV50_thresholds[1]=12;
  para->EV60_thresholds[0]=-30;
  para->EV60_thresholds[1]=12;
  para->freq_feature_index[0]=2;
  para->freq_feature_index[1]=1;
}


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;
namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetFlickerPara(MINT32 sensorMode, MVOID*const pDataBuf) const
{
	XLOGD("impGetFlickerPara+ mode=%d", sensorMode);
	XLOGD("prv=%d, vdo=%d, cap=%d, zsd=%d",
	    (int)e_sensorModePreview, (int)e_sensorModeVideoPreview, (int)e_sensorModeCapture, (int)e_sensorModeZsd );
	FLICKER_CUST_PARA* para;
	para =  (FLICKER_CUST_PARA*)pDataBuf;
	if(sensorMode==e_sensorModePreview)
		get_flicker_para_by_preview(para);
	else if(sensorMode==e_sensorModeZsd||
	   sensorMode==e_sensorModeCapture)
	{
		get_flicker_para_by_ZSD(para);
	}
	else if(sensorMode==e_sensorModeVideoPreview)
	{
		get_flicker_para_by_vPreview(para);
	}
	else
	{
		XLOGD("impGetFlickerPara ERROR ln=%d", __LINE__);
		return -1;
	}
	XLOGD("impGetFlickerPara-");
	return 0;
}
}

