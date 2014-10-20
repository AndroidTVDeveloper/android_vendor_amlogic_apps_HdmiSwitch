package com.amlogic.HdmiSwitch;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.KeyguardManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;
import android.view.WindowManagerPolicy;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.widget.Toast;

import android.view.KeyEvent;
import android.view.IWindowManager;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import android.hardware.input.IInputManager;
import android.app.SystemWriteManager;
import android.content.SharedPreferences;
public class HdmiBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "HdmiBroadcastReceiver";

    // Use a layout id for a unique identifier
    private static final int HDMI_NOTIFICATIONS = R.layout.main;

    private static final String ACTION_PLAYER_CRASHED = "com.farcore.videoplayer.PLAYER_CRASHED";
    private static SystemWriteManager sw = null; 
    private boolean mSystemReady = false;
    //private boolean mSwitchCompleted = false;
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceive: " + intent.getAction());
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {	
            mSystemReady = true;
            boolean plugged = isHdmiPlugged();
            sw = (SystemWriteManager) context.getSystemService("system_write");
            resetFreescaleStatus();
            if (plugged) {
                NotificationManager nM = (NotificationManager) context.getSystemService(context.NOTIFICATION_SERVICE);

                CharSequence text = context.getText(R.string.hdmi_state_str1);     
                Notification notification = new Notification(R.drawable.stat_connected, text, System.currentTimeMillis());  

                Intent it = new Intent(context, HdmiSwitch.class);
                it.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
                PendingIntent contentIntent = PendingIntent.getActivity(context, 0, it, 0);        
                notification.setLatestEventInfo(context, context.getText(R.string.app_name), text, contentIntent);

                nM.notify(HDMI_NOTIFICATIONS, notification);
                onHdmiPlugged(context);
            }  		
        } else if (WindowManagerPolicy.ACTION_HDMI_HW_PLUGGED.equals(intent.getAction())) {

            boolean plugged = intent.getBooleanExtra(WindowManagerPolicy.EXTRA_HDMI_HW_PLUGGED_STATE, false); 
            Log.d(TAG, "onReceive: " + plugged+" "+System.currentTimeMillis());
            if(plugged){
                NotificationManager nM = (NotificationManager) context.getSystemService(context.NOTIFICATION_SERVICE);

                CharSequence text = context.getText(R.string.hdmi_state_str1);     
                Notification notification = new Notification(R.drawable.stat_connected, text, System.currentTimeMillis());  

                Intent it = new Intent(context, HdmiSwitch.class);
                it.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
                PendingIntent contentIntent = PendingIntent.getActivity(context, 0, it, 0);        
                notification.setLatestEventInfo(context, context.getText(R.string.app_name), text, contentIntent);

                nM.notify(HDMI_NOTIFICATIONS, notification);
                onHdmiPlugged(context);
                //mSwitchCompleted =  true;
            }else{
                //do{
                    try {
                        Thread.currentThread().sleep(1000);
                    }catch (InterruptedException e){
                        e.printStackTrace();
                    }
                    if(isHdmiPlugged()){
                        return;
                    }
                //}while(!mSwitchCompleted);
                onHdmiUnplugged(context);

                NotificationManager nM = (NotificationManager) context.getSystemService(context.NOTIFICATION_SERVICE);
                nM.cancel(HDMI_NOTIFICATIONS); 
                //mSwitchCompleted = false;
            }
        } else if (ACTION_PLAYER_CRASHED.equals(intent.getAction())) {
            HdmiSwitch.onVideoPlayerCrashed();
        } else if (Intent.ACTION_USER_PRESENT.equals(intent.getAction())) {
            mSystemReady = true;
            if (SystemProperties.getBoolean("ro.app.hdmi.allswitch", false)) {
                if (isHdmiPlugged()) onHdmiPlugged(context);
            }
        }
    }


    private boolean isHdmiPlugged() {
        boolean plugged = false;
        // watch for HDMI plug messages if the hdmi switch exists
        if (new File("/sys/devices/virtual/switch/hdmi/state").exists()) {	
            final String filename = "/sys/class/switch/hdmi/state";
            FileReader reader = null;
            try {
                reader = new FileReader(filename);
                char[] buf = new char[15];
                int n = reader.read(buf);
                if (n > 1) {
                    plugged = 0 != Integer.parseInt(new String(buf, 0, n-1));
                }
            } catch (IOException ex) {
                Log.w(TAG, "Couldn't read hdmi state from " + filename + ": " + ex);
            } catch (NumberFormatException ex) {
                Log.w(TAG, "Couldn't read hdmi state from " + filename + ": " + ex);
            } finally {
                if (reader != null) {
                    try {
                        reader.close();
                    } catch (IOException ex) {
                    }
                }
            }
        }

        return plugged;    
    }

    private void onHdmiPlugged(Context context) {
    	SharedPreferences prefs = context.getSharedPreferences(HdmiSwitch.PRESS_KEY, Context.MODE_PRIVATE);

        if (SystemProperties.getBoolean("ro.vout.dualdisplay4", false)) {
            if (HdmiSwitch.getCurMode().equals("null")) {
                int autoSwitchEnabled = Settings.System.getInt(context.getContentResolver(),
                                                                Settings.System.HDMI_AUTO_SWITCH, 1);                
                if (autoSwitchEnabled != 1){
                    return;
                }

                // camera in-use
                String isCameraBusy = SystemProperties.get("camera.busy", "0");
                if (!isCameraBusy.equals("0")) {
                    Log.w(TAG, "onHdmiPlugged, camera is busy");
                    Toast.makeText(context,
                    context.getText(R.string.Toast_msg_camera_busy),
                    Toast.LENGTH_LONG).show();                     
                    return;
                }
                // keyguard on
                KeyguardManager mKeyguardManager = (KeyguardManager) context.getSystemService(context.KEYGUARD_SERVICE); 
                if (mKeyguardManager != null && mKeyguardManager.inKeyguardRestrictedInputMode()) {
                    Log.w(TAG, "onHdmiPlugged, keyguard on");
                    return;
                }
                Log.w(TAG, "onHdmiPlugged-----www-----:"+mSystemReady);
                if(!mSystemReady){
                    return;
                }
                if (SystemProperties.getBoolean("ro.vout.player.exit", true)) {
                    /// send BACK key to stop other player
                    sendKeyEvent(KeyEvent.KEYCODE_HOME);
                }                

                // show the cling when it auto connected
                if (SystemProperties.getBoolean("ro.module.singleoutput",false)) {
		        	Log.v(TAG, "singleoutput ok, dualdispaly4 false");
                	if (!prefs.getBoolean(HdmiCling.CLING_DISMISS_KEY_720P, false)) {
                		Intent i = new Intent(context, ShowCling.class);
                		i.putExtra("on_which", HdmiCling.CLING_DISMISS_KEY_720P);
                		i.putExtra("which_cling", "first");
                		i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                		context.startActivity(i);
                	}
                } else {
			        Log.v(TAG,"singleoutput false, display4 fasle");
		        }
                  
                HdmiSwitch.setMode("720p");
                Intent it = new Intent(WindowManagerPolicy.ACTION_HDMI_PLUGGED);
                it.putExtra(WindowManagerPolicy.EXTRA_HDMI_PLUGGED_STATE, true);
                context.sendStickyBroadcast(it);    		
            }          
            return;
        }


        if (!SystemProperties.getBoolean("ro.vout.dualdisplay", false)) {
            if (HdmiSwitch.getCurMode().equals("panel")) {
                int autoSwitchEnabled = Settings.System.getInt(context.getContentResolver(),
                                                                Settings.System.HDMI_AUTO_SWITCH, 1);                
                if (autoSwitchEnabled != 1){
                    return;
                }

                // screen on
                PowerManager powerManager = (PowerManager)context.getSystemService(context.POWER_SERVICE);
                if (!powerManager.isScreenOn()) {
                    Log.w(TAG, "onHdmiPlugged, screen is off");
                    return;
                }                        

                // in suspend process
                /* int brightness = Settings.System.getInt(context.getContentResolver(),
                Settings.System.SCREEN_BRIGHTNESS, 255);
                if (brightness > Integer.parseInt(HdmiSwitch.getBrightness())) {
                Log.w(TAG, "onHdmiPlugged, in suspend process");
                return;
                }*/


                // camera in-use
                String isCameraBusy = SystemProperties.get("camera.busy", "0");
                if (!isCameraBusy.equals("0")) {
                    Log.w(TAG, "onHdmiPlugged, camera is busy");
                    Toast.makeText(context,
                    context.getText(R.string.Toast_msg_camera_busy),
                    Toast.LENGTH_LONG).show();                     
                    return;
                }
                // keyguard on
                boolean mNotCheckKygd = SystemProperties.getBoolean("ro.module.dualscaler", false);
                KeyguardManager mKeyguardManager = (KeyguardManager) context.getSystemService(context.KEYGUARD_SERVICE); 
                if (mKeyguardManager != null && mKeyguardManager.inKeyguardRestrictedInputMode()&& !mNotCheckKygd) {
                    Log.w(TAG, "onHdmiPlugged, keyguard on");
                    return;
                }

                HdmiSwitch.setFb0Blank("1");

                if (SystemProperties.getBoolean("ro.vout.player.exit", true)) {
                    /// send BACK key to stop other player
                    sendKeyEvent(KeyEvent.KEYCODE_HOME);
                }
                Log.w(TAG, "onHdmiPlugged 720p");

                // show the cling when it auto connected
                if (SystemProperties.getBoolean("ro.module.singleoutput",false)) {
			        Log.v(TAG, "singleoutput ok, dualdispaly true");
                	if (!prefs.getBoolean(HdmiCling.CLING_DISMISS_KEY_720P, false)) {
                		Intent i = new Intent(context, ShowCling.class);
                		i.putExtra("on_which", HdmiCling.CLING_DISMISS_KEY_720P);
                		i.putExtra("which_cling", "first");
                		i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                		context.startActivity(i);
                	}
                } else {
			        Log.v(TAG, "singleouput false, dispaly true");
		        }

                HdmiSwitch.setMode("720p");
                Intent it = new Intent(WindowManagerPolicy.ACTION_HDMI_PLUGGED);
                it.putExtra(WindowManagerPolicy.EXTRA_HDMI_PLUGGED_STATE, true);
                context.sendStickyBroadcast(it);
                if ((SystemProperties.getBoolean("ro.vout.dualdisplay2", false)
                    || SystemProperties.getBoolean("ro.vout.dualdisplay3", false))
                    && !SystemProperties.getBoolean("ro.real.externaldisplay", false)) {                        
                    int dualEnabled = Settings.System.getInt(context.getContentResolver(),
                                                    Settings.System.HDMI_DUAL_DISP, 1);
                    HdmiSwitch.setDualDisplayStatic(true, (dualEnabled == 1));
                }
                if ( !(SystemProperties.getBoolean("ro.real.externaldisplay", false)
                    /*&& SystemProperties.getBoolean("ro.module.singleoutput", false)*/)){
                    HdmiSwitch.setFb0Blank("0");
                }
            }
        }
    }    

    private void onHdmiUnplugged(Context context) {
        if (SystemProperties.getBoolean("ro.vout.dualdisplay4", false)) {
            if (!HdmiSwitch.getCurMode().equals("null")) {
                if (SystemProperties.getBoolean("ro.vout.player.exit", true)) {
                    /// send BACK key to stop other player
                    sendKeyEvent(KeyEvent.KEYCODE_HOME);
                }                  

                HdmiSwitch.setMode("null");
                Intent it = new Intent(WindowManagerPolicy.ACTION_HDMI_PLUGGED);
                it.putExtra(WindowManagerPolicy.EXTRA_HDMI_PLUGGED_STATE, false);
                context.sendStickyBroadcast(it);                
            }
            return;
        }

        if (!SystemProperties.getBoolean("ro.vout.dualdisplay", false)) {
            if (!HdmiSwitch.getCurMode().equals("panel")) {
                HdmiSwitch.setVout2OffStatic();

                /// 1. send broadcast to stop player
                //                Intent it = new Intent(WindowManagerPolicy.ACTION_HDMI_PLUGGED);
                //                it.putExtra(WindowManagerPolicy.EXTRA_HDMI_PLUGGED_STATE, false);
                //                context.sendStickyBroadcast(it);   

                if (SystemProperties.getBoolean("ro.vout.player.exit", true)) {
                    /// 2. send BACK key to stop player
                    sendKeyEvent(KeyEvent.KEYCODE_HOME);
                }

                /// 3. kill player
                if (!SystemProperties.getBoolean("ro.vout.player.exit", true)) {
                    HdmiSwitch.setMode("panel");
                    Intent it = new Intent(WindowManagerPolicy.ACTION_HDMI_PLUGGED);
                    it.putExtra(WindowManagerPolicy.EXTRA_HDMI_PLUGGED_STATE, false);
                    if (HdmiSwitch.isExternalSinglePortraitDisplayJni()){
                        it.putExtra("videoplayer.need.pause", false);
                    }
                    context.sendStickyBroadcast(it);
                    if ((SystemProperties.getBoolean("ro.vout.dualdisplay2", false) 
                        || SystemProperties.getBoolean("ro.vout.dualdisplay3", false))
                        && !SystemProperties.getBoolean("ro.real.externaldisplay", false)) {                        
                        int dualEnabled = Settings.System.getInt(context.getContentResolver(),
                                                                    Settings.System.HDMI_DUAL_DISP, 1);
                        HdmiSwitch.setDualDisplayStatic(false, (dualEnabled == 1));
                    }  
                } else {   
                    context.startService(new Intent(context, 
                                        HdmiDelayedService.class));    
                }             
            }
        }    
    }

    /**
    * Send a single key event.
    *
    * @param event is a string representing the keycode of the key event you
    * want to execute.
    */
    private void sendKeyEvent(int keyCode) {
        int eventCode = keyCode;
        long now = SystemClock.uptimeMillis();
        try {
            KeyEvent down = new KeyEvent(now, now, KeyEvent.ACTION_DOWN, eventCode, 0);
            KeyEvent up = new KeyEvent(now, now, KeyEvent.ACTION_UP, eventCode, 0);
            (IInputManager.Stub
                .asInterface(ServiceManager.getService("input")))
            .injectInputEvent(down, 0);
            (IInputManager.Stub
                .asInterface(ServiceManager.getService("input")))
            .injectInputEvent(up, 0);
        } catch (RemoteException e) {
            Log.i(TAG, "DeadOjbectException");
        }
    }

    private void resetFreescaleStatus(){
        //when system power up, we need to reset freescale status in case the screen is crash
        if(sw.readSysfs("/sys/class/graphics/fb0/free_scale").contains("0x1")){
            //Log.d(TAG, "freescale has open,which means hdmi is plugging in .So don't set it");
            return;
        }else{
            sw.writeSysfs("/sys/class/graphics/fb0/freescale_mode", "0");
            sw.writeSysfs("/sys/class/graphics/fb0/free_scale","0");
        }
    }

}
