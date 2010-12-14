#include <string.h>
#include <jni.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdlib.h>

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
			return 0;
		}
	}
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
			return 0;
		}
	}
	return -1;	
}

int scaleFb(int flag) {
	if((scaleFb0(flag) == 0) && (scaleFb1(flag) == 0))
		return 0;
	else
		return -1;	
}

jint
Java_com_amlogic_HdmiSwitch_HdmiSwitch_scaleFrameBufferJni( JNIEnv* env,
                                                  jobject thiz, jint flag )
{
    return scaleFb(flag);
}


