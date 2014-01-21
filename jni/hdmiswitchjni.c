#include <string.h>
#include <jni.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdlib.h>

#include <cutils/log.h>
#include <cutils/properties.h>

#include "hdmiswitchjni.h"
#define LOG_TAG "Hdmiswitchjni"

#ifndef LOGD
    #define LOGV ALOGV
    #define LOGD ALOGD
    #define LOGI ALOGI
    #define LOGW ALOGW
    #define LOGE ALOGE
#endif

#define  FBIOPUT_OSD_FREE_SCALE_ENABLE	0x4504
#define  FBIOPUT_OSD_FREE_SCALE_WIDTH	0x4505
#define  FBIOPUT_OSD_FREE_SCALE_HEIGHT	0x4506
#define  FBIOPUT_OSD_FREE_SCALE_MODE  0x4511
#define  FBIOPUT_OSD_WINDOW_AXIS  	0x4513

struct fb_var_screeninfo vinfo;
char daxis_str[32];
char vaxis_str[80];

int amsysfs_set_sysfs_str(const char *path, const char *val)
{
    int fd;
    int bytes;
    ALOGI("amsysfs_set_sysfs_str %s= %s\n", path,val);
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        bytes = write(fd, val, strlen(val));
        close(fd);
        return 0;
    } else {
   	 ALOGI("open %s failed\n", path,val);
    }
    return -1;
}


static lastDisplayMode=0;

int freeScale(int mode) {
	int fd0 = -1, fd1 = -1;
	int fd_daxis = -1, fd_vaxis = -1;
	int fd_fsaxis =-1, fd_winaxis=-1;
	int fd_fb = -1;
	int fd_video = -1;
	int fd_ppmgr = -1;
        int fd_ppmgr_rect = -1;
	int osd_width = 0, osd_height = 0;	
	int ret = -1;
	int x = 0, y = 0, w = 0, h = 0;
	int find_flag = 0;
	char freescale_str[32];

    int isM8 = 0;
	int isPortrait =0;
	char value[128];
	memset(value, 0 ,128);
	property_get("ro.module.dualscaler", value, "false");
	if(strstr(value,"true"))
	{
		isM8 =1;
		LOGI("hi,this is dualscaler, treat it better!");
	}
	memset(value, 0 ,128);
	property_get("ro.screen.portrait", value, "0");
	if(strstr(value,"true"))
	{
		isPortrait = 1;
	}


	
	
	//LOGI("freeScale: mode=%d", mode);
	if((fd0 = open("/dev/graphics/fb0", O_RDWR)) < 0) {
		LOGI("open /dev/graphics/fb0 fail.");
		goto exit;
	}
	if((fd1 = open("/dev/graphics/fb1", O_RDWR)) < 0) {
		LOGI("open /dev/graphics/fb1 fail.");
		goto exit;		
	}
	if((fd_vaxis = open("/sys/class/video/axis", O_RDWR)) < 0) {
		LOGI("open /sys/class/video/axis fail.");
		goto exit;	
	}
		
	if((fd_daxis = open("/sys/class/display/axis", O_RDWR)) < 0) {
		LOGI("open /sys/class/display/axis fail.");
		goto exit;	
	}	

	if((fd_fb = open("/dev/graphics/fb0", O_RDWR)) < 0) {
		LOGI("open /dev/graphics/fb0 fail.");
		goto exit;
	}
	
	if((fd_video = open("/sys/class/video/disable_video", O_RDWR)) < 0) {
		LOGI("open /sys/class/video/disable_video fail.");
	}	
		
	if((fd_ppmgr = open("/sys/class/ppmgr/ppscaler", O_RDWR)) < 0) {
		LOGI("open /sys/class/ppmgr/ppscaler fail.");	
	}

	if((fd_ppmgr_rect = open("/sys/class/ppmgr/ppscaler_rect", O_RDWR)) < 0) {
		LOGI("open /sys/class/ppmgr/ppscaler_rect fail.");	
	}
	
	if((fd_fsaxis = open("/sys/class/graphics/fb0/free_scale_axis", O_RDWR)) < 0) {
		LOGI("open /sys/class/graphics/fb0/free_scale_axis fail.");
		goto exit;	
	}
	
	if((fd_winaxis= open("/sys/class/graphics/fb0/window_axis", O_RDWR)) < 0) {
		LOGI("open /sys/class/graphics/fb0/window_axis fail.");
		goto exit;	
	}
	

	memset(vaxis_str,0,80);	
	if(fd_vaxis>=0){
		int ret_len = read(fd_vaxis, vaxis_str, sizeof(vaxis_str));
		if(ret_len>0){
			if(sscanf(vaxis_str,"%d %d %d %d",&x,&y,&w,&h)>0){
				w = w -x + 1;
				h = h -y + 1;
				find_flag = 1;	
				LOGI("set mode: vaxis: x:%d, y:%d, w:%d, h:%d.",x,y,w,h);
			}
		}
	}

	memset(daxis_str,0,32);	
	if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo) == 0) {
		osd_width = vinfo.xres;
		osd_height = vinfo.yres;
		sprintf(daxis_str, "0 0 %d %d 0 0 18 18", vinfo.xres, vinfo.yres);
																									
		//LOGI("osd_width = %d", osd_width);
		//LOGI("osd_height = %d", osd_height);
	} else {
		LOGI("get FBIOGET_VSCREENINFO fail.");
		goto exit;
	}
			LOGI("set mid mode=%d  lastDisplayMode =%d isPortrait=%d", mode,lastDisplayMode,isPortrait);	
    char screen_orientation[128];
    memset(screen_orientation, 0 ,128);
    property_get("ro.screen.orientation", screen_orientation, "false");
	switch(mode) {
		case 0:	//panel
			if(isPortrait==0)
			{
				if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
				//if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));	
				write(fd_daxis, daxis_str, strlen(daxis_str));				
				/*ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);		
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);*/
				amsysfs_set_sysfs_str("/sys/class/display/mode","panel");
				amsysfs_set_sysfs_str("/sys/class/video/disable_video","1");
				amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","1");
				amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale","0");
				amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","0");
				amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","0");
				amsysfs_set_sysfs_str("/sys/class/video/disable_video","0");
				if((fd_ppmgr >= 0)&&(find_flag)){
					write(fd_vaxis, vaxis_str, strlen(vaxis_str));
				}
				//if (fd_video >= 0) 	write(fd_video, "2", strlen("2"));
				ret = 0;		
			}else
			{
				amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","1");
				amsysfs_set_sysfs_str("/sys/class/graphics/fb2/clone","0");
				amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_on","0");
				amsysfs_set_sysfs_str("/sys/class/display/mode","panel");
				amsysfs_set_sysfs_str("/sys/class/display2/mode","null");
				amsysfs_set_sysfs_str("/sys/class/display2/venc_mux","0x0");
				amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale","0x0");
				amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","0");
			}
			//if (fd_video >= 0) 	write(fd_video, "2", strlen("2"));
			ret = 0;
			break;	
		case 1: //480p
			if(isM8==0)
			{
				if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
				if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
				if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
				if(fd_ppmgr_rect >= 0)
					write(fd_ppmgr_rect, "20 10 700 470 0", strlen("20 10 700 470 0"));
				else if(fd_vaxis >= 0)
					write(fd_vaxis, "20 10 700 470", strlen("20 10 700 470"));
				write(fd_daxis, daxis_str, strlen(daxis_str));
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height); 
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);				
				if((fd_ppmgr >= 0)&&(find_flag)){
					write(fd_vaxis, vaxis_str, strlen(vaxis_str));
				}
				if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "2", strlen("2"));
			}
			else
			{
				if(isPortrait==0)
				{
					/*ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,0);
					ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
					ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,1);
					ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);*/

					amsysfs_set_sysfs_str("/sys/class/display/mode","480p");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/freescale_mode", "0x1");
					memset(freescale_str,0,32); 
					sprintf(freescale_str, "0 0 %d %d ",osd_width, osd_height);
					write(fd_fsaxis, freescale_str, strlen(freescale_str));
					write(fd_winaxis, "20 10 700 470", strlen("20 10 700 470"));
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale","0x10001");
				}else
				{
					//if(lastDisplayMode!=0)
					if(1)
					{
						/*ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,0);
						ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
						ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,1);
						ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);*/
						memset(freescale_str,0,32); 
						sprintf(freescale_str, "0 0 %d %d ",osd_width, osd_height);

						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","1");

						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/clone","1");
						amsysfs_set_sysfs_str("/sys/class/display/mode","480p");		 //delete
						amsysfs_set_sysfs_str("/sys/class/display2/mode","null");
						amsysfs_set_sysfs_str("/sys/class/display2/mode","panel");
						amsysfs_set_sysfs_str("/sys/class/display2/venc_mux","0x2");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_canvas","0 0 767 1023");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_angle","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_on","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/freescale_mode", "0x1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale_axis","0 0 1023 767");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/window_axis","20 10 700 470");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale","0x10001");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","0");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","0");
					}else
					{
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/clone","0");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/clone","1");
						amsysfs_set_sysfs_str("/sys/class/display/mode","480p");		 //delete
						amsysfs_set_sysfs_str("/sys/class/display2/mode","panel");
						amsysfs_set_sysfs_str("/sys/class/display2/venc_mux","0x2");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_canvas","0 0 767 1023");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_angle","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_on","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/freescale_mode", "0x1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale_axis","0 0 1023 767");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/window_axis","20 10 700 470");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale","0x10001");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","0");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","0");
					}
				}
			}
			ret = 0;
			break;
		case 2: //720p
		
			if(isM8==0)
			{
				if (fd_ppmgr >= 0)	write(fd_ppmgr, "0", strlen("0"));
							if (fd_video >= 0)	write(fd_video, "1", strlen("1"));
							if (fd_ppmgr >= 0)	write(fd_ppmgr, "1", strlen("1"));
							if(fd_ppmgr_rect >= 0)
								write(fd_ppmgr_rect, "40 15 1240 705 0", strlen("40 15 1240 705 0"));
							else if(fd_vaxis >= 0)
								write(fd_vaxis, "40 15 1240 705", strlen("40 15 1240 705"));
							write(fd_daxis, daxis_str, strlen(daxis_str));
							ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
							ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
							ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
							ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);
							ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
							ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);
							if (fd_ppmgr >= 0)	write(fd_ppmgr, "1", strlen("1"));
							ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
							ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1); 		;	
							if((fd_ppmgr >= 0)&&(find_flag)){
								write(fd_vaxis, vaxis_str, strlen(vaxis_str));
							}
							if ((fd_video >= 0)&&(fd_ppmgr >= 0))	write(fd_video, "2", strlen("2"));
			}
			else
			{
				LOGI("-----jeff.yang 720p----");
				if(isPortrait==0)
				{
					/*ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,0);
					ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
					ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,1);
					ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);*/

					amsysfs_set_sysfs_str("/sys/class/display/mode","720p");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/freescale_mode", "0x1");
					
					memset(freescale_str,0,32); 
					sprintf(freescale_str, "0 0 %d %d ",osd_width, osd_height);
					write(fd_fsaxis, freescale_str, strlen(freescale_str));
					write(fd_winaxis, "40 15 1240 705", strlen("40 15 1240 705"));
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale", "0x10001");
					amsysfs_set_sysfs_str("sys/class/video/axis", "40 15 1240 705");
				}else
				{

					//if(lastDisplayMode!=0)
					if(1)
					{
						/*ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,0);
						ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
						ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,1);
						ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);*/
						memset(freescale_str,0,32); 
						sprintf(freescale_str, "0 0 %d %d ",osd_width, osd_height);

						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","1");

						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/clone","1");
						amsysfs_set_sysfs_str("/sys/class/display/mode","720p");
						amsysfs_set_sysfs_str("/sys/class/display2/mode","null");
						amsysfs_set_sysfs_str("/sys/class/display2/mode","panel");
						amsysfs_set_sysfs_str("/sys/class/display2/venc_mux","0x2");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_canvas","0 0 767 1023");
                        if(strstr(screen_orientation,"true"))
						    amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_angle","2");
                        else
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_angle","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_on","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/freescale_mode", "0x1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale_axis","0 0 1023 767");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/window_axis","40 15 1240 705");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale","0x10001");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","0");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","0");
					}else
					{
					
					amsysfs_set_sysfs_str("/sys/class/graphics/fb2/clone","0");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","1");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","1");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb2/clone","1");
					amsysfs_set_sysfs_str("/sys/class/display/mode","720p");
					amsysfs_set_sysfs_str("/sys/class/display2/mode","panel");
					amsysfs_set_sysfs_str("/sys/class/display2/venc_mux","0x2");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_canvas","0 0 767 1023");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_angle","1");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_on","1");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/freescale_mode", "0x1");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale_axis","0 0 1023 767");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/window_axis","0 0 1279 719");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale","0x10001");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","0");
					amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","0");

					}
				}
					
					

				
			}
			ret = 0;
			break;
		case 3: //1080i			
		case 4: //1080p
			if(isM8==0)
			{
				if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
				if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
				if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
				if(fd_ppmgr_rect >= 0)
					write(fd_ppmgr_rect, "40 20 1880 1060 0", strlen("40 20 1880 1060 0"));
				else if(fd_vaxis >= 0)
					write(fd_vaxis, "40 20 1880 1060", strlen("40 20 1880 1060"));
				write(fd_daxis, daxis_str, strlen(daxis_str));
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
				ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);				
				if((fd_ppmgr >= 0)&&(find_flag)){
					write(fd_vaxis, vaxis_str, strlen(vaxis_str));
				}
				if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "2", strlen("2"));
			}
			else
			{
				if(isPortrait==0)
				{
				/*ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,0);
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,1);
				ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);*/
				amsysfs_set_sysfs_str("/sys/class/display/mode","1080p");
				amsysfs_set_sysfs_str("/sys/class/graphics/fb0/freescale_mode", "0x1");
				
				memset(freescale_str,0,32); 
				sprintf(freescale_str, "0 0 %d %d ",osd_width, osd_height);
				write(fd_fsaxis, freescale_str, strlen(freescale_str));
				write(fd_winaxis, "40 20 1880 1060", strlen("40 20 1880 1060"));
				amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale", "0x10001");
				amsysfs_set_sysfs_str("sys/class/video/axis", "40 20 1880 1060");
				}else
				{
					//if(lastDisplayMode!=0)
					if(1)
					{
						/*ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,0);
						ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
						ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_MODE,1);
						ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);*/
						memset(freescale_str,0,32); 
						sprintf(freescale_str, "0 0 %d %d ",osd_width, osd_height);
						
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","1");

						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/clone","1");
						amsysfs_set_sysfs_str("/sys/class/display/mode","1080p");
						amsysfs_set_sysfs_str("/sys/class/display2/mode","null");
						amsysfs_set_sysfs_str("/sys/class/display2/mode","panel");
						amsysfs_set_sysfs_str("/sys/class/display2/venc_mux","0x2");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_canvas","0 0 767 1023");
                        if(strstr(screen_orientation,"true"))
						    amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_angle","2"); 
                        else
						    amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_angle","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_on","1");
						
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/freescale_mode", "0x1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale_axis", "0 0 1023 767");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/window_axis","40 20 1880 1060");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale","0x10001");
 					    amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","0");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","0");
					}else
					{
					LOGI("-----jeff 1080p single----");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/clone","0");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/clone","1");
						amsysfs_set_sysfs_str("/sys/class/display/mode","1080p");
						amsysfs_set_sysfs_str("/sys/class/display2/mode","panel");
						amsysfs_set_sysfs_str("/sys/class/display2/venc_mux","0x2");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_canvas","0 0 767 1023");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_angle","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/prot_on","1");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale_axis","0 0 1023 767");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/window_axis","0 0 1919 1079");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb0/free_scale","0x10001");
 					    amsysfs_set_sysfs_str("/sys/class/graphics/fb0/blank","0");
						amsysfs_set_sysfs_str("/sys/class/graphics/fb2/blank","0");
					}
				}
				
			}
			ret = 0;
			break;	
		default:			
			break;		
			
	}
	lastDisplayMode = mode;
	
exit:	
	close(fd0);
	close(fd1);
	close(fd_vaxis);
	close(fd_daxis);	
	close(fd_fb);
	close(fd_video);
	close(fd_ppmgr);
	close(fd_ppmgr_rect);
	return ret;
}
	
int FreeScaleForDisplay2(int mode) {
	int fd0 = -1, fd1 = -1;
	int fd_daxis = -1, fd_vaxis = -1;
	int fd_fb = -1;
	int fd_video = -1;
	int fd_ppmgr = -1;
        int fd_ppmgr_rect = -1;
	int osd_width = 0, osd_height = 0;	
	int ret = -1;
	int x = 0, y = 0, w = 0, h = 0;
	int find_flag = 0;
	
	//LOGI("freeScale: mode=%d", mode);
	if((fd0 = open("/dev/graphics/fb2", O_RDWR)) < 0) {
		LOGI("open /dev/graphics/fb2 fail.");
		goto exit;
	}

	if((fd_vaxis = open("/sys/class/video/axis", O_RDWR)) < 0) {
		LOGI("open /sys/class/video/axis fail.");
		goto exit;	
	}
		
	if((fd_daxis = open("/sys/class/display2/axis", O_RDWR)) < 0) {
		LOGI("open /sys/class/display2/axis fail.");
		goto exit;	
	}	

	if((fd_fb = open("/dev/graphics/fb2", O_RDWR)) < 0) {
		LOGI("open /dev/graphics/fb0 fail.");
		goto exit;
	}
	
	if((fd_video = open("/sys/class/video/disable_video", O_RDWR)) < 0) {
		LOGI("open /sys/class/video/disable_video fail.");
	}	
		
	if((fd_ppmgr = open("/sys/class/ppmgr/ppscaler", O_RDWR)) < 0) {
		LOGI("open /sys/class/ppmgr/ppscaler fail.");	
	}

	if((fd_ppmgr_rect = open("/sys/class/ppmgr/ppscaler_rect", O_RDWR)) < 0) {
		LOGI("open /sys/class/ppmgr/ppscaler_rect fail.");	
	}

	memset(vaxis_str,0,80);	
	if(fd_vaxis>=0){
		int ret_len = read(fd_vaxis, vaxis_str, sizeof(vaxis_str));
		if(ret_len>0){
			if(sscanf(vaxis_str,"%d %d %d %d",&x,&y,&w,&h)>0){
				w = w -x + 1;
				h = h -y + 1;
				find_flag = 1;	
				LOGI("set mode: vaxis: x:%d, y:%d, w:%d, h:%d.",x,y,w,h);
			}
		}
	}

	memset(daxis_str,0,32);	
	if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo) == 0) {
		osd_width = vinfo.xres;
		osd_height = vinfo.yres;
		sprintf(daxis_str, "0 0 %d %d 0 0 18 18", vinfo.xres, vinfo.yres);
																									
		//LOGI("osd_width = %d", osd_width);
		//LOGI("osd_height = %d", osd_height);
	} else {
		LOGI("get FBIOGET_VSCREENINFO fail.");
		goto exit;
	}
			
	switch(mode) {
		//LOGI("set mid mode=%d", mode);

		case 0:	//panel
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			//if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));	
			write(fd_daxis, daxis_str, strlen(daxis_str));				
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
            if(fd_vaxis >= 0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));
//			if((fd_ppmgr >= 0)&&(find_flag)){
//				write(fd_vaxis, vaxis_str, strlen(vaxis_str));
//			}
			//if (fd_video >= 0) 	write(fd_video, "2", strlen("2"));
			ret = 0;
			break;	
		case 1: //480p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "20 10 700 470 0", strlen("20 10 700 470 0"));
			else if(fd_vaxis >= 0)
				write(fd_vaxis, "20 10 700 470", strlen("20 10 700 470"));
			write(fd_daxis, daxis_str, strlen(daxis_str));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height); 
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);		
			if((fd_ppmgr >= 0)&&(find_flag)){
				write(fd_vaxis, vaxis_str, strlen(vaxis_str));
			}
			//if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "2", strlen("2"));
			ret = 0;
			break;
		case 2: //720p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "40 15 1240 705 0", strlen("40 15 1240 705 0"));
			else if(fd_vaxis >= 0)
				write(fd_vaxis, "40 15 1240 705", strlen("40 15 1240 705"));
			write(fd_daxis, daxis_str, strlen(daxis_str));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			if((fd_ppmgr >= 0)&&(find_flag)){
				write(fd_vaxis, vaxis_str, strlen(vaxis_str));
			}
			//if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "2", strlen("2"));
			ret = 0;
			break;
		case 3: //1080i			
		case 4: //1080p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "40 20 1880 1060 0", strlen("40 20 1880 1060 0"));
			else if(fd_vaxis >= 0)
				write(fd_vaxis, "40 20 1880 1060", strlen("40 20 1880 1060"));
			write(fd_daxis, daxis_str, strlen(daxis_str));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);		
			if((fd_ppmgr >= 0)&&(find_flag)){
				write(fd_vaxis, vaxis_str, strlen(vaxis_str));
			}
			//if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "2", strlen("2"));
			ret = 0;
			break;	
		default:			
			break;		
			
	}
	
exit:	
	close(fd0);
	close(fd1);
	close(fd_vaxis);
	close(fd_daxis);	
	close(fd_fb);
	close(fd_video);
	close(fd_ppmgr);
	close(fd_ppmgr_rect);
	return ret;
}


JNIEXPORT jint JNICALL Java_com_amlogic_HdmiSwitch_HdmiSwitch_freeScaleSetModeJni( JNIEnv * env,
																									jobject thiz, jint mode )
{	
		return freeScale(mode);
}						

#define log_print LOGI
int DisableFreeScale(int mode) {
	int fd0 = -1, fd1 = -1;
	int fd_daxis = -1, fd_vaxis = -1;
	int fd_ppmgr = -1,fd_ppmgr_rect = -1;
	int fd_video = -1;
	int osd_width = 0, osd_height = 0;	
	int ret = -1;
	
	log_print("DisableFreeScale: mode=%d", mode);	
	if(mode == 0) return 0;	
		
	if((fd0 = open("/dev/graphics/fb0", O_RDWR)) < 0) {
		log_print("open /dev/graphics/fb0 fail.");
		goto exit;
	}
	if((fd1 = open("/dev/graphics/fb1", O_RDWR)) < 0) {
		log_print("open /dev/graphics/fb1 fail.");
		goto exit;	
	}
		
	if((fd_daxis = open("/sys/class/display/axis", O_RDWR)) < 0) {
		log_print("open /sys/class/display/axis fail.");
		goto exit;
	}

	if((fd_ppmgr = open("/sys/class/ppmgr/ppscaler", O_RDWR)) < 0) {
		log_print("open /sys/class/ppmgr/ppscaler fail.");	
	}

	if((fd_ppmgr_rect = open("/sys/class/ppmgr/ppscaler_rect", O_RDWR)) < 0) {
		log_print("open /sys/class/ppmgr/ppscaler_rect fail.");	
	}

	if((fd_video = open("/sys/class/video/disable_video", O_RDWR)) < 0) {
		log_print("open /sys/class/video/disable_video fail.");
	}

	if((fd_vaxis = open("/sys/class/video/axis", O_RDWR)) < 0) {
		log_print("open /sys/class/video/axis fail.");
	}

	memset(daxis_str,0,32);	
	if(ioctl(fd0, FBIOGET_VSCREENINFO, &vinfo) == 0) {
		osd_width = vinfo.xres;
		osd_height = vinfo.yres;

		//log_print("osd_width = %d", osd_width);
		//log_print("osd_height = %d", osd_height);
	} else {
		log_print("get FBIOGET_VSCREENINFO fail.");
		goto exit;
	}
		
	switch(mode) {
		//log_print("set mid mode=%d", mode);

		case 1: //480p		
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			//if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));		
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);			
			sprintf(daxis_str, "0 0 %d %d 0 0 18 18", vinfo.xres, vinfo.yres);
			write(fd_daxis, daxis_str, strlen(daxis_str));		
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "0 0 0 0 1", strlen("0 0 0 0 1"));
			if(fd_vaxis >= 0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));
			ret = 0;
			break;
		case 2: //720p
			LOGI("----disableFreescale 720p-----");
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			//if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			sprintf(daxis_str, "%d %d %d %d %d %d 18 18", 1280>vinfo.xres ? (1280-vinfo.xres)/2 : 0, 
				720>vinfo.yres ? (720-vinfo.yres)/2 : 0,
				vinfo.xres, 
				vinfo.yres,
				1280>vinfo.xres ? (1280-vinfo.xres)/2 : 0,
				720>vinfo.yres ? (720-vinfo.yres)/2 : 0);
			write(fd_daxis, daxis_str, strlen(daxis_str));
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "0 0 0 0 1", strlen("0 0 0 0 1"));
			if(fd_vaxis >= 0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));
			ret = 0;
			break;
		case 3: //1080i			
		case 4: //1080p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			//if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			sprintf(daxis_str, "%d %d %d %d %d %d 18 18", 1920>vinfo.xres ? (1920-vinfo.xres)/2 : 0, 
				1080>vinfo.yres ? (1080-vinfo.yres)/2 : 0,
				vinfo.xres, 
				vinfo.yres,
				1920>vinfo.xres ? (1920-vinfo.xres)/2 : 0,
				1080>vinfo.yres ? (1080-vinfo.yres)/2 : 0);
			write(fd_daxis, daxis_str, strlen(daxis_str));	
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "0 0 0 0 1", strlen("0 0 0 0 1"));
			if(fd_vaxis >= 0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));
			ret = 0;
			break;	
		default:			
			break;					
	}	

	
	
exit:	
	close(fd0);
	close(fd1);
	close(fd_daxis);
	close(fd_vaxis);
	close(fd_ppmgr);
	close(fd_video);
	close(fd_ppmgr_rect);
	return ret;;

}
int DisableFreeScaleFB2(int mode) {
	int fd0 = -1, fd1 = -1;
	int fd_daxis = -1, fd_vaxis = -1;
	int fd_ppmgr = -1,fd_ppmgr_rect = -1;
	int fd_video = -1;
	int osd_width = 0, osd_height = 0;	
	int ret = -1;
	
	log_print("DisableFreeScale: mode=%d", mode);	
	if(mode == 0) return 0;	
		
	if((fd0 = open("/dev/graphics/fb2", O_RDWR)) < 0) {
		log_print("open /dev/graphics/fb2 fail.");
		goto exit;
	}
		
	if((fd_daxis = open("/sys/class/display2/axis", O_RDWR)) < 0) {
		log_print("open /sys/class/display2/axis fail.");
		goto exit;
	}

	if((fd_ppmgr = open("/sys/class/ppmgr/ppscaler", O_RDWR)) < 0) {
		log_print("open /sys/class/ppmgr/ppscaler fail.");	
	}

	if((fd_ppmgr_rect = open("/sys/class/ppmgr/ppscaler_rect", O_RDWR)) < 0) {
		log_print("open /sys/class/ppmgr/ppscaler_rect fail.");	
	}

	if((fd_video = open("/sys/class/video/disable_video", O_RDWR)) < 0) {
		log_print("open /sys/class/video/disable_video fail.");
	}

	if((fd_vaxis = open("/sys/class/video/axis", O_RDWR)) < 0) {
		log_print("open /sys/class/video/axis fail.");
	}

	memset(daxis_str,0,32);	
	if(ioctl(fd0, FBIOGET_VSCREENINFO, &vinfo) == 0) {
		osd_width = vinfo.xres;
		osd_height = vinfo.yres;

		//log_print("osd_width = %d", osd_width);
		//log_print("osd_height = %d", osd_height);
	} else {
		log_print("get FBIOGET_VSCREENINFO fail.");
		goto exit;
	}
		
	switch(mode) {
		//log_print("set mid mode=%d", mode);

		case 1: //480p		
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			//if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));		
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			sprintf(daxis_str, "0 0 %d %d 0 0 18 18", vinfo.xres, vinfo.yres);
			write(fd_daxis, daxis_str, strlen(daxis_str));		
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "0 0 0 0 1", strlen("0 0 0 0 1"));
			if(fd_vaxis >= 0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));
			ret = 0;
			break;
		case 2: //720p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			//if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			sprintf(daxis_str, "%d %d %d %d %d %d 18 18", 1280>vinfo.xres ? (1280-vinfo.xres)/2 : 0, 
				720>vinfo.yres ? (720-vinfo.yres)/2 : 0,
				vinfo.xres, 
				vinfo.yres,
				1280>vinfo.xres ? (1280-vinfo.xres)/2 : 0,
				720>vinfo.yres ? (720-vinfo.yres)/2 : 0);
			write(fd_daxis, daxis_str, strlen(daxis_str));
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "0 0 0 0 1", strlen("0 0 0 0 1"));
			if(fd_vaxis >= 0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));
			ret = 0;
			break;
		case 3: //1080i			
		case 4: //1080p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "0", strlen("0"));
			//if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			sprintf(daxis_str, "%d %d %d %d %d %d 18 18", 1920>vinfo.xres ? (1920-vinfo.xres)/2 : 0, 
				1080>vinfo.yres ? (1080-vinfo.yres)/2 : 0,
				vinfo.xres, 
				vinfo.yres,
				1920>vinfo.xres ? (1920-vinfo.xres)/2 : 0,
				1080>vinfo.yres ? (1080-vinfo.yres)/2 : 0);
			write(fd_daxis, daxis_str, strlen(daxis_str));	
			if(fd_ppmgr_rect >= 0)
				write(fd_ppmgr_rect, "0 0 0 0 1", strlen("0 0 0 0 1"));
			if(fd_vaxis >= 0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));
			ret = 0;
			break;	
		default:			
			break;					
	}	

	
	
exit:	
	close(fd0);
	close(fd1);
	close(fd_daxis);
	close(fd_vaxis);
	close(fd_ppmgr);
	close(fd_video);
	close(fd_ppmgr_rect);
	return ret;;

}


int EnableFreeScale(int mode) {
	int fd0 = -1, fd1 = -1;
	int fd_daxis = -1, fd_vaxis = -1;
	int fd_ppmgr = -1,fd_ppmgr_rect = -1;
	int fd_video = -1;
 	int osd_width = 0, osd_height = 0;	
	int ret = -1;
	
	//log_print("EnableFreeScale: mode=%d", mode);	
    if(mode == 0) return 0;		
		
	if((fd0 = open("/dev/graphics/fb0", O_RDWR)) < 0) {
		log_print("open /dev/graphics/fb0 fail.");
		goto exit;
	}
	if((fd1 = open("/dev/graphics/fb1", O_RDWR)) < 0) {
		log_print("open /dev/graphics/fb1 fail.");
		goto exit;	
	}
	if((fd_vaxis = open("/sys/class/video/axis", O_RDWR)) < 0) {
		log_print("open /sys/class/video/axis fail.");
		goto exit;		
	}
		
	if((fd_daxis = open("/sys/class/display/axis", O_RDWR)) < 0) {
		log_print("open /sys/class/display/axis fail.");
		goto exit;
	}

	if((fd_video = open("/sys/class/video/disable_video", O_RDWR)) < 0) {
		log_print("open /sys/class/video/disable_video fail.");
	}

	if((fd_ppmgr = open("/sys/class/ppmgr/ppscaler", O_RDWR)) < 0) {
		log_print("open /sys/class/ppmgr/ppscaler fail.");	
	}

	if((fd_ppmgr_rect = open("/sys/class/ppmgr/ppscaler_rect", O_RDWR)) < 0) {
		log_print("open /sys/class/ppmgr/ppscaler_rect fail.");	
	}

	memset(daxis_str,0,32);	
	if(ioctl(fd0, FBIOGET_VSCREENINFO, &vinfo) == 0) {
		osd_width = vinfo.xres;
		osd_height = vinfo.yres;
		sprintf(daxis_str, "0 0 %d %d 0 0 18 18", vinfo.xres, vinfo.yres);
		
		//log_print("osd_width = %d", osd_width);
		//log_print("osd_height = %d", osd_height);
	} else {
		log_print("get FBIOGET_VSCREENINFO fail.");
		goto exit;
	}
		
	switch(mode) {
		//log_print("set mid mode=%d", mode);

		case 1: //480p				
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			if(fd_ppmgr_rect >= 0){
				write(fd_ppmgr_rect, "20 10 700 470 0", strlen("20 10 700 470 0"));
				//if(fd_vaxis>=0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));				
			}else if(fd_vaxis >= 0){
				write(fd_vaxis, "20 10 700 470", strlen("20 10 700 470"));
			}
			write(fd_daxis, daxis_str, strlen(daxis_str));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height); 
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "1", strlen("1"));			
			ret = 0;
			break;
		case 2: //720p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			if(fd_ppmgr_rect >= 0){
				write(fd_ppmgr_rect, "40 15 1240 705 0", strlen("40 15 1240 705 0"));
				//if(fd_vaxis>=0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));				
			}else if(fd_vaxis >= 0){
				write(fd_vaxis, "40 15 1240 705", strlen("40 15 1240 705"));
			}
			write(fd_daxis, daxis_str, strlen(daxis_str));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height); 
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "1", strlen("1"));
			ret = 0;
			break;
		case 3: //1080i			
		case 4: //1080p
			if (fd_ppmgr >= 0) 	write(fd_ppmgr, "1", strlen("1"));
			if (fd_video >= 0) 	write(fd_video, "1", strlen("1"));
			if(fd_ppmgr_rect >= 0){
				write(fd_ppmgr_rect, "40 20 1880 1060 0", strlen("40 20 1880 1060 0"));
				//if(fd_vaxis>=0) write(fd_vaxis, "0 0 0 0", strlen("0 0 0 0"));				
			}else if(fd_vaxis >= 0){
				write(fd_vaxis, "40 20 1880 1060", strlen("40 20 1880 1060"));
			}
			write(fd_daxis, daxis_str, strlen(daxis_str));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height); 
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,osd_width);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,osd_height);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			if ((fd_video >= 0)&&(fd_ppmgr >= 0)) 	write(fd_video, "1", strlen("1"));		
			ret = 0;
			break;	
		default:			
			break;					
	}	



exit:	
	close(fd0);
	close(fd1);
	close(fd_vaxis);
	close(fd_daxis);	
	close(fd_ppmgr);
	close(fd_video);
	close(fd_ppmgr_rect);
	return ret;

}
																	
JNIEXPORT jint JNICALL Java_com_amlogic_HdmiSwitch_HdmiSwitch_DisableFreeScaleJni( JNIEnv * env,
																									jobject thiz, jint mode )
{	
		return DisableFreeScale(mode);
}
JNIEXPORT jint JNICALL Java_com_amlogic_HdmiSwitch_HdmiSwitch_EnableFreeScaleJni( JNIEnv * env,
																									jobject thiz, jint mode )
{	
		return EnableFreeScale(mode);
}


JNIEXPORT jint JNICALL Java_com_amlogic_HdmiSwitch_HdmiSwitch_freeScaleForDisplay2Jni( JNIEnv * env,
																									jobject thiz, jint mode )
{	
		return FreeScaleForDisplay2(mode);
}
JNIEXPORT jint JNICALL Java_com_amlogic_HdmiSwitch_HdmiSwitch_DisableFreeScaleForDisplay2Jni( JNIEnv * env,
																									jobject thiz, jint mode )
{	
		return DisableFreeScaleFB2(mode);
}
