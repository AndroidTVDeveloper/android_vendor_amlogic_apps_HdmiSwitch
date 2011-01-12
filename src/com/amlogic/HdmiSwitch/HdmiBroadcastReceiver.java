package com.amlogic.HdmiSwitch;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class HdmiBroadcastReceiver extends BroadcastReceiver {
	private static final String ACTION_BOOT_COMPLETED =
		"android.intent.action.BOOT_COMPLETED";
	private static final String ACTION_FAST_SWITCH =
		"com.amlogic.HdmiSwitch.FAST_SWITCH";	
	private static final String ACTION_FREESCALE_BEFORE_VIDEO =
		"com.amlogic.HdmiSwitch.FREESCALE_BEFORE_VIDEO";	
	private static final String ACTION_FREESCALE_AFTER_VIDEO =
		"com.amlogic.HdmiSwitch.FREESCALE_AFTER_VIDEO";		
	
	@Override
	public void onReceive(Context context, Intent intent) {
		
		if (ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
			context.startService(new Intent(context, 
					HdmiCheckService.class));			
		}		
		if (ACTION_FAST_SWITCH.equals(intent.getAction())) {			 
			HdmiSwitch.fastSwitch();
		}	
		if (ACTION_FREESCALE_BEFORE_VIDEO.equals(intent.getAction())) {			 
			HdmiSwitch.doBeforePlayVideo();
		}	
		if (ACTION_FREESCALE_AFTER_VIDEO.equals(intent.getAction())) {			 
			HdmiSwitch.doAfterPlayVideo();
		}			
	}
	
}