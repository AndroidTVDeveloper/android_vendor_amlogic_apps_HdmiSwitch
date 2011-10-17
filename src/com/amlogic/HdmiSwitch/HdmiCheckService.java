package com.amlogic.HdmiSwitch;

import java.io.File;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;


public class HdmiCheckService extends Service {
	private static final String TAG = "HdmiCheckService";
	private static final String ACTION_HDMISWITCH_MODE_CHANGED =
		"com.amlogic.HdmiSwitch.HDMISWITCH_MODE_CHANGED";	
    // Use a layout id for a unique identifier
    private static final int HDMI_NOTIFICATIONS = R.layout.main;
    
    //private PowerManager.WakeLock mWakeLock;
    
    private NotificationManager mNM;
    private Handler mProgressHandler;    
    
    private static final boolean HDMI_CONNECTED = true;
    private static final boolean HDMI_DISCONNECTED = false;
    private static boolean hdmi_stat = HDMI_DISCONNECTED;
    private static boolean hdmi_stat_old = HDMI_DISCONNECTED;
    
    @Override
    public void onCreate() {
        mNM = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        
        /* check driver interface */        
        File file = new File(HdmiSwitch.DISP_CAP_PATH);
        if (!file.exists()) {        	
        	return;
        }
        file = new File(HdmiSwitch.MODE_PATH);
        if (!file.exists()) {        	
        	return;
        }
        file = new File(HdmiSwitch.AXIS_PATH);
        if (!file.exists()) {        	
        	return;
        }
        
        /* hdmi check handler */
        mProgressHandler = new HdmiCheckHandler(); 
        
        /* start check after 5s */        
        mProgressHandler.sendEmptyMessageDelayed(0, 5000);
        
    }

    @Override
    public void onDestroy() {
        // Cancel the persistent notification.
        mNM.cancel(HDMI_NOTIFICATIONS);        
    }    

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    
    /** showNotification */
    private void showNotification(int moodId, int textId) {        
        CharSequence text = getText(textId);     
        
        Notification notification = new Notification(moodId, text, System.currentTimeMillis());  
        
        Intent intent = new Intent(this, HdmiSwitch.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP
				| Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
        		intent, 0);        
        notification.setLatestEventInfo(this, getText(R.string.app_name), text, contentIntent);
        
        mNM.notify(HDMI_NOTIFICATIONS, notification);
    }       

	/** sendTvOutIntent **/
	private void sendTvOutIntent( boolean plugin ) {
		Intent intent = new Intent(Intent.ACTION_TVOUT_EVENT);
		if(plugin){
			intent.putExtra(Intent.EXTRA_TVOUT_STATE, Intent.EXTRA_TVOUT_STATE_ON );
		}else{
        	intent.putExtra(Intent.EXTRA_TVOUT_STATE, Intent.EXTRA_TVOUT_STATE_OFF );
		}
        sendBroadcast(intent);
	}
	
    /** hdmi check handler */
    private class HdmiCheckHandler extends Handler {
    	 @Override
         public void handleMessage(Message msg) {
             super.handleMessage(msg);     
             
             hdmi_stat = HdmiSwitch.isHdmiConnected(); 
             if (hdmi_stat_old == HDMI_DISCONNECTED) {            	 
             	if (hdmi_stat == HDMI_CONNECTED) {
             		hdmi_stat_old = hdmi_stat;
             		
             		showNotification(R.drawable.stat_connected,
                             R.string.hdmi_state_str1);   
             		
             		/* run HdmiSwitch activity ? */
             	}
             } else {            	
             	if (hdmi_stat == HDMI_DISCONNECTED) {
             		hdmi_stat_old = hdmi_stat;   
             		
                 	if (!HdmiSwitch.getCurMode().equals("panel")) {
                 		sendBroadcast( new Intent(ACTION_HDMISWITCH_MODE_CHANGED));
                 		sendTvOutIntent(false);                 		
                 		// delay for finish intent              		
                 		try {
                 			Thread.currentThread().sleep(1000);
                 		} catch (InterruptedException e) {
                 			// TODO Auto-generated catch block
                 			e.printStackTrace();
                 		}  
                 		HdmiSwitch.setMode("panel");
                 	}
                 	
                 	showNotification(R.drawable.stat_connected,
                            R.string.hdmi_state_str2);
                	
                	mNM.cancel(HDMI_NOTIFICATIONS); 
                	
             	}
             }
             /* check per 3s */
             mProgressHandler.sendEmptyMessageDelayed(0, 3000);  
         }
    }
	
}