package com.amlogic.HdmiSwitch;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.view.WindowManagerPolicy;

public class HdmiBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "HdmiBroadcastReceiver";

    // Use a layout id for a unique identifier
    private static final int HDMI_NOTIFICATIONS = R.layout.main;
        
    @Override
    public void onReceive(Context context, Intent intent) {

        if (WindowManagerPolicy.ACTION_HDMI_HW_PLUGGED.equals(intent.getAction())) {
            Log.d(TAG, "onReceive: " + intent.getAction());
            boolean plugged = intent.getBooleanExtra(WindowManagerPolicy.EXTRA_HDMI_PLUGGED_STATE, false); 
            if(plugged){
                NotificationManager nM = (NotificationManager) context.getSystemService(context.NOTIFICATION_SERVICE);
                
                CharSequence text = context.getText(R.string.hdmi_state_str1);     
                Notification notification = new Notification(R.drawable.stat_connected, text, System.currentTimeMillis());  

                Intent it = new Intent(context, HdmiSwitch.class);
                it.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
                PendingIntent contentIntent = PendingIntent.getActivity(context, 0, it, 0);        
                notification.setLatestEventInfo(context, context.getText(R.string.app_name), text, contentIntent);

                nM.notify(HDMI_NOTIFICATIONS, notification);
            }else{
                 if (!HdmiSwitch.getCurMode().equals("panel")) {
                    HdmiSwitch.setMode("panel");
                    Intent it = new Intent(WindowManagerPolicy.ACTION_HDMI_PLUGGED);
                    it.putExtra(WindowManagerPolicy.EXTRA_HDMI_PLUGGED_STATE, false);
                    context.sendStickyBroadcast(it);
                 }
                
                 NotificationManager nM = (NotificationManager) context.getSystemService(context.NOTIFICATION_SERVICE);
                 nM.cancel(HDMI_NOTIFICATIONS); 
            }
        }
    }

}