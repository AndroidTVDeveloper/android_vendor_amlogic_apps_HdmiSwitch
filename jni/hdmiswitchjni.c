#include <string.h>
#include <jni.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdlib.h>

#include <cutils/log.h>

#include "hdmiswitchjni.h"

struct fb_var_screeninfo vinfo1, vinfo2;

int scaleFb0(int flag) {
	int fd;
	
	if((fd = open("/dev/graphics/fb0", O_RDWR)) < 0)
		return -1;
		
	if(ioctl(fd, FBIOGET_VSCREENINFO, &vinfo1) == 0) {
		if(flag == 1) {
			vinfo1.xres = 1600;
			vinfo1.yres = 960;
	
		} else {
			vinfo1.xres = 800;
			vinfo1.yres = 480;
			vinfo1.xres_virtual = 800;
			vinfo1.yres_virtual = 960;
			
		}
		if(ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo1) == 0) {
			close(fd);
			return 0;
		}
	}
	close(fd);
	return -1;	
}

int scaleFb1(int flag) {
	int fd;
	
	if((fd = open("/dev/graphics/fb1", O_RDWR)) < 0)
		return -1;
		
	if(ioctl(fd, FBIOGET_VSCREENINFO, &vinfo2) == 0) {
		if(flag == 1) {
			vinfo2.xres = 36;
			vinfo2.yres = 36;

		} else {
			vinfo2.xres = 18;
			vinfo2.yres = 18;
			vinfo2.xres_virtual = 18;
			vinfo2.yres_virtual = 18;			

		}
		if(ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo2) == 0) {
			close(fd);
			return 0;
		}
	}
	close(fd);
	return -1;	
}

int scaleFb(int flag) {
	if((scaleFb0(flag) == 0) && (scaleFb1(flag) == 0))
		return 0;
	else
		return -1;	
}

JNIEXPORT jint JNICALL Java_com_amlogic_HdmiSwitch_HdmiSwitch_scaleFrameBufferJni( JNIEnv* env,
                                                  jobject thiz, jint flag )
{
    return scaleFb(flag);
}

#define  FBIOPUT_OSD_FREE_SCALE_ENABLE	0x4504
#define  FBIOPUT_OSD_FREE_SCALE_WIDTH		0x4505
#define  FBIOPUT_OSD_FREE_SCALE_HEIGHT	0x4506
#define  MID_800x480 1

int freeScale(int mode) {
	int fd0, fd1;
	int fd_daxis, fd_vaxis;
	int ret = -1;
	
	//LOGI("freeScale: mode=%d", mode);
	if((fd0 = open("/dev/graphics/fb0", O_RDWR)) < 0) {
		//LOGI("open /dev/graphics/fb0 fail.");
		return -1;
	}
	if((fd1 = open("/dev/graphics/fb1", O_RDWR)) < 0) {
		//LOGI("open /dev/graphics/fb1 fail.");
		return -1;		
	}
	if((fd_vaxis = open("/sys/class/video/axis", O_RDWR)) < 0) {
		//LOGI("open /sys/class/video/axis fail.");
		return -1;		
	}
		
	if((fd_daxis = open("/sys/class/display/axis", O_RDWR)) < 0) {
		//LOGI("open /sys/class/display/axis fail.");
		return -1;	
	}	
		
#ifdef MID_800x480
	switch(mode) {
		//LOGI("set mid mode=%d", mode);

		case 0:	//panel
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			write(fd_daxis, "0 0 800 480 0 0 18 18", strlen("0 0 800 480 0 0 18 18"));
			
			ret = 0;
			break;	
		case 1: //480p
			write(fd_vaxis, "20 10 700 470", strlen("20 10 700 470"));
			write(fd_daxis, "0 0 800 480 0 0 18 18", strlen("0 0 800 480 0 0 18 18"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);	
			
			ret = 0;
			break;
		case 2: //720p
			write(fd_vaxis, "40 15 1240 705", strlen("40 15 1240 705"));
			write(fd_daxis, "0 0 800 480 0 0 18 18", strlen("0 0 800 480 0 0 18 18"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);	
			
			ret = 0;
			break;
		case 3: //1080i			
		case 4: //1080p
			write(fd_vaxis, "40 20 1880 1060", strlen("40 20 1880 1060"));
			write(fd_daxis, "0 0 800 480 0 0 18 18", strlen("0 0 800 480 0 0 18 18"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);	
			
			ret = 0;
			break;	
		default:			
			break;		
			
	}	
#else		
	switch(mode) {
		//LOGI("set mode=%d", mode);

		case 0:	//panel

			ret = 0;
			break;	
		case 1: //480p
			write(fd_vaxis, "20 10 700 470", strlen("20 10 700 470"));
			write(fd_daxis, "0 0 1200 690 0 0 18 18", strlen("0 0 1200 690 0 0 18 18"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,1200);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,690);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,1200);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,690);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);	
			
			ret = 0;
			break;
		case 2: //720p
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			write(fd_daxis, "40 15 1200 690 40 15 18 18", strlen("40 15 1200 690 40 15 18 18"));
			
			ret = 0;
			break;
		case 3: //1080i			
		case 4: //1080p
			write(fd_vaxis, "40 20 1880 1060", strlen("40 20 1880 1060"));
			write(fd_daxis, "0 0 1200 690 0 0 18 18", strlen("0 0 1200 690 0 0 18 18"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,1200);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,690);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,1200);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,690);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);	
			
			ret = 0;
			break;	
		default:			
			break;		
			
	}	
#endif	
	
	close(fd0);
	close(fd1);
	close(fd_vaxis);
	close(fd_daxis);	
	return ret;
}
	


JNIEXPORT jint JNICALL Java_com_amlogic_HdmiSwitch_HdmiSwitch_freeScaleSetModeJni( JNIEnv * env,
																									jobject thiz, jint mode )
{	
		return freeScale(mode);
}						

int DisableFreeScale(int mode) {
	int fd0, fd1;
	int fd_daxis, fd_vaxis;
	int ret = -1;
	
	//LOGI("DisableFreeScale: mode=%d", mode);	
	if(mode == 0) return 0;		
		
	if((fd0 = open("/dev/graphics/fb0", O_RDWR)) < 0) {
		//LOGI("open /dev/graphics/fb0 fail.");
		return -1;
	}
	if((fd1 = open("/dev/graphics/fb1", O_RDWR)) < 0) {
		//LOGI("open /dev/graphics/fb1 fail.");
		return -1;		
	}
	if((fd_vaxis = open("/sys/class/video/axis", O_RDWR)) < 0) {
		//LOGI("open /sys/class/video/axis fail.");
		return -1;		
	}
		
	if((fd_daxis = open("/sys/class/display/axis", O_RDWR)) < 0) {
		//LOGI("open /sys/class/display/axis fail.");
		return -1;	
	}	
	
	switch(mode) {
		//LOGI("set mid mode=%d", mode);

		case 0:	//panel			
			ret = 0;
			break;	
		case 1: //480p			
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			write(fd_daxis, "0 0 800 480 0 0 18 18", strlen("0 0 800 480 0 0 18 18"));
			
			ret = 0;
			break;
		case 2: //720p
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			write(fd_daxis, "240 120 800 480 240 120 18 18", strlen("240 120 800 480 240 120 18 18"));
			
			ret = 0;
			break;
		case 3: //1080i			
		case 4: //1080p
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			write(fd_daxis, "560 300 800 480 560 300 18 18", strlen("560 300 800 480 560 300 18 18"));
			
			ret = 0;
			break;	
		default:			
			break;					
	}	
	
	close(fd0);
	close(fd1);
	close(fd_vaxis);
	close(fd_daxis);		
	return ret;
}

int EnableFreeScale(int mode) {
	int fd0, fd1;
	int fd_daxis, fd_vaxis;
	int ret = -1;
	
	//LOGI("EnableFreeScale: mode=%d", mode);	
	if(mode == 0) return 0;		
		
	if((fd0 = open("/dev/graphics/fb0", O_RDWR)) < 0) {
		//LOGI("open /dev/graphics/fb0 fail.");
		return -1;
	}
	if((fd1 = open("/dev/graphics/fb1", O_RDWR)) < 0) {
		//LOGI("open /dev/graphics/fb1 fail.");
		return -1;		
	}
	if((fd_vaxis = open("/sys/class/video/axis", O_RDWR)) < 0) {
		//LOGI("open /sys/class/video/axis fail.");
		return -1;		
	}
		
	if((fd_daxis = open("/sys/class/display/axis", O_RDWR)) < 0) {
		//LOGI("open /sys/class/display/axis fail.");
		return -1;	
	}		
	
	switch(mode) {
		//LOGI("set mid mode=%d", mode);

		case 0:	//panel
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			write(fd_daxis, "0 0 800 480 0 0 18 18", strlen("0 0 800 480 0 0 18 18"));
			
			ret = 0;
			break;	
		case 1: //480p
			write(fd_vaxis, "20 10 700 470", strlen("20 10 700 470"));
			write(fd_daxis, "0 0 800 480 0 0 18 18", strlen("0 0 800 480 0 0 18 18"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);	
			
			ret = 0;
			break;
		case 2: //720p
			write(fd_vaxis, "40 15 1240 705", strlen("40 15 1240 705"));
			write(fd_daxis, "0 0 800 480 0 0 18 18", strlen("0 0 800 480 0 0 18 18"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);	
			
			ret = 0;
			break;
		case 3: //1080i			
		case 4: //1080p
			write(fd_vaxis, "40 20 1880 1060", strlen("40 20 1880 1060"));
			write(fd_daxis, "0 0 800 480 0 0 18 18", strlen("0 0 800 480 0 0 18 18"));
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,0);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_WIDTH,800);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_HEIGHT,480);	
			ioctl(fd0,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);
			ioctl(fd1,FBIOPUT_OSD_FREE_SCALE_ENABLE,1);	
			
			ret = 0;
			break;	
		default:			
			break;					
	}	
	
	close(fd0);
	close(fd1);
	close(fd_vaxis);
	close(fd_daxis);		
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
