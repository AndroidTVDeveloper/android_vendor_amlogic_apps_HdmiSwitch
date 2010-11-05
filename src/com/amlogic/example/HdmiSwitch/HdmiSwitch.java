package com.amlogic.example.HdmiSwitch;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;

public class HdmiSwitch extends Activity {
	
	private static final String TAG = "HdmiSwitch";
	
	public static final String DISP_CAP_PATH = "/sys/class/amhdmitx/amhdmitx0/disp_cap";
	public static final String MODE_PATH = "/sys/class/display/mode";
	public static final String AXIS_PATH = "/sys/class/display/axis";
	
	private static final int CONFIRM_DIALOG_ID = 0;
	private static final int MAX_PROGRESS = 15;
	private static final int STOP_PROGRESS = -1;
	private int mProgress;
	private int mProgress2;
	private Handler mProgressHandler;
	
	private static final boolean HDMI_CONNECTED = true;
    private static final boolean HDMI_DISCONNECTED = false;
    private static boolean hdmi_stat = HDMI_DISCONNECTED;
    private static boolean hdmi_stat_old = HDMI_DISCONNECTED;
	
	private AlertDialog confirm_dialog;	
	private static String old_mode = "panel";

	private ListView lv;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        setContentView(R.layout.main);
        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.title_layout); 
        
        /* set window size */
        WindowManager wm = getWindowManager();
        Display display = wm.getDefaultDisplay();
        LayoutParams lp = getWindow().getAttributes();
        if (display.getHeight() > display.getWidth()) {
            //lp.height = (int) (display.getHeight() * 0.5);
            lp.width = (int) (display.getWidth() * 1.0);       	
        } else {
        	//lp.height = (int) (display.getHeight() * 0.75);
            lp.width = (int) (display.getWidth() * 0.5);            	
        }
        getWindow().setAttributes(lp);        
        
        /* close button listener */
        Button btn_close = (Button) findViewById(R.id.title_btn_right);  
        btn_close.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {				
				finish();
			}        	
        }); 
        
        /* check driver interface */        
        TextView tv = (TextView) findViewById(R.id.hdmi_state_str);        
        File file = new File(DISP_CAP_PATH);
        if (!file.exists()) {
        	tv.setText(getText(R.string.driver_api_err) + "[001]");
        	return;
        }
        file = new File(MODE_PATH);
        if (!file.exists()) {
        	tv.setText(getText(R.string.driver_api_err) + "[010]");
        	return;
        }
        file = new File(AXIS_PATH);
        if (!file.exists()) {
        	tv.setText(getText(R.string.driver_api_err) + "[100]");
        	return;
        }
        
        
        /* update hdmi_state_str*/
        if (isHdmiConnected())
        	tv.setText(getText(R.string.hdmi_state_str1));
        else
        	tv.setText(getText(R.string.hdmi_state_str2));
        
        /* setup video mode list */
        lv = (ListView) findViewById(R.id.listview); 
        SimpleAdapter adapter = new SimpleAdapter(this,getListData(),R.layout.list_item,        		
        		                new String[]{"item_text","item_img"},        		
        		                new int[]{R.id.item_text,R.id.item_img});        		
        lv.setAdapter(adapter);
        
        /* mode select listener */
        lv.setOnItemClickListener(new OnItemClickListener() {
			public void onItemClick(AdapterView<?> parent, View view, int pos,
					long id) {
				Map<String, Object> item = (Map<String, Object>)parent.getItemAtPosition(pos);
				if (item.get("item_img").equals(R.drawable.item_img_unsel)) {					
					old_mode = getCurMode();
					setMode((String)item.get("mode"));			
					updateListDisplay();					
					
					if ((String)item.get("mode") != "panel")
						showDialog(CONFIRM_DIALOG_ID);
				}
				
			}        	
        });    
        
        /* progress handler*/
        mProgressHandler = new HdmiSwitchProgressHandler(); 
        
    }
    
    /** onResume() */
    @Override
    public void onResume() {
    	super.onResume();    
    	
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
        
    	hdmi_stat_old = isHdmiConnected(); 
    	mProgress2 = 0;
    	mProgressHandler.sendEmptyMessageDelayed(1, 1000); 
    }
    
    /** onPause() */
    @Override
    public void onPause() {
    	super.onPause();
    	
    	mProgress = STOP_PROGRESS;  
    	mProgress2 = STOP_PROGRESS;
    }
   
    
    /** Confirm Dialog */
    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case CONFIRM_DIALOG_ID:
        	confirm_dialog =  new AlertDialog.Builder(HdmiSwitch.this)
                .setIcon(R.drawable.dialog_icon)
                .setTitle(R.string.dialog_title)
                .setPositiveButton(R.string.dialog_str_ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {   
                    	mProgress = STOP_PROGRESS;                    	
                        /* User clicked OK so do some stuff */
                    }
                })
                .setNegativeButton(R.string.dialog_str_cancel, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                    	mProgress = STOP_PROGRESS;                    	
                    	setMode(old_mode);
                    	updateListDisplay();                     	
                    	/* User clicked Cancel so do some stuff */                    	
                    }
                })
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
					
					public void onCancel(DialogInterface dialog) {						
						mProgress = STOP_PROGRESS;  										
					}
				})				
                .create();  
        	
            return confirm_dialog;
        }
        
		return null;    	
    }
    @Override
    protected void onPrepareDialog(int id, Dialog dialog) {
    	switch (id) {
    	case CONFIRM_DIALOG_ID: 
            WindowManager wm = getWindowManager();
            Display display = wm.getDefaultDisplay();
            LayoutParams lp = dialog.getWindow().getAttributes();
            if (display.getHeight() > display.getWidth()) {            	
            	lp.width = (int) (display.getWidth() * 1.0);       	
        	} else {        		
        		lp.width = (int) (display.getWidth() * 0.5);            	
        	}
            dialog.getWindow().setAttributes(lp);
        
            ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_NEGATIVE)
    		.setText(getText(R.string.dialog_str_cancel) 
    				+ " (" + MAX_PROGRESS + ")");            
            
            mProgress = 0;	                
            mProgressHandler.sendEmptyMessageDelayed(0, 1000);
            break;
    	}
    }  
	
    /** getListData */
    private List<Map<String, Object>> getListData() {    	
    	List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();	 
    	
    	for (String modeStr : getAllMode()) {
        	Map<String, Object> map = new HashMap<String, Object>();  
        	map.put("mode", modeStr);
        	map.put("item_text", getText((Integer)MODE_STR_TABLE.get(modeStr)));
        	if (modeStr.equals(getCurMode()))
        		map.put("item_img", R.drawable.item_img_sel);
        	else
        		map.put("item_img", R.drawable.item_img_unsel);
        	list.add(map);    		
    	}
    	
    	return list;
 	} 
    /** updateListDisplay */
    private void updateListDisplay() {
    	Map<String, Object> list_item;
    	for (int i = 0; i < lv.getAdapter().getCount(); i++) {						
			list_item = (Map<String, Object>)lv.getAdapter().getItem(i);    						
			if (list_item.get("mode").equals(getCurMode()))
				list_item.put("item_img", R.drawable.item_img_sel);
			else
				list_item.put("item_img", R.drawable.item_img_unsel);
		}  
    	((BaseAdapter) lv.getAdapter()).notifyDataSetChanged();  
    }
    
    /** updateActivityDisplay */
    private void updateActivityDisplay() {
    	/* update hdmi_state_str*/
        TextView tv = (TextView) findViewById(R.id.hdmi_state_str);
        if (isHdmiConnected())
        	tv.setText(getText(R.string.hdmi_state_str1));
        else
        	tv.setText(getText(R.string.hdmi_state_str2));
        
        /* update video mode list */
        lv = (ListView) findViewById(R.id.listview);        
        SimpleAdapter adapter = new SimpleAdapter(this,getListData(),R.layout.list_item,        		
        		                new String[]{"item_text","item_img"},        		
        		                new int[]{R.id.item_text,R.id.item_img});        		
        lv.setAdapter(adapter);    	
        
        ((BaseAdapter) lv.getAdapter()).notifyDataSetChanged();  
    }
    
    /** check hdmi connection*/
    public static boolean isHdmiConnected() {    
    	try {
    		BufferedReader reader = new BufferedReader(new FileReader(DISP_CAP_PATH), 256);
    		try {
    			return (reader.readLine() == null)? false : true;     			
    		} finally {
    			reader.close();
    		}   
    		
    	} catch (IOException e) { 
    		Log.e(TAG, "IO Exception when read: " + DISP_CAP_PATH, e);   
    		return false;
    	}  
    	
    }
    /** get all support mode*/
    private List<String> getAllMode() {
    	List<String> list = new ArrayList<String>();
    	String modeStr;
    	
    	list.add("panel");     	
    	
    	//list.add("480i");
    	//list.add("480p");
    	//list.add("720p");
    	//list.add("1080i");
    	//list.add("1080p");     	
    	try {
    		BufferedReader reader = new BufferedReader(new FileReader(DISP_CAP_PATH), 256);
    		try {
    			while ((modeStr = reader.readLine()) != null) {
    				modeStr = modeStr.split("\\*")[0]; //720p* to 720p
    				
    				if (MODE_STR_TABLE.containsKey(modeStr))
    					list.add(modeStr);	
    			}
    		} finally {
    			reader.close();
    		}   
    		
    	} catch (IOException e) { 
    		Log.e(TAG, "IO Exception when read: " + DISP_CAP_PATH, e);    		
    	}    	
    	
    	return list;
    }

	/** get current mode*/
    public static String getCurMode() {
    	String modeStr;
    	try {
    		BufferedReader reader = new BufferedReader(new FileReader(MODE_PATH), 32);
    		try {
    			modeStr = reader.readLine();  
    		} finally {
    			reader.close();
    		}    		
    		return (modeStr == null)? "panel" : modeStr;   	
    		
    	} catch (IOException e) { 
    		Log.e(TAG, "IO Exception when read: " + MODE_PATH, e);
    		return "panel";
    	}    	
    }
    /** set mode */
    public static int setMode(String modeStr) {   
    	//Log.i(TAG, "Set mode = " + modeStr);	
    	if (!modeStr.equals("panel")) {
    		if (!isHdmiConnected())
    			return 0;
    	}
    	if (modeStr.equals(getCurMode()))
    		return 0;
    	
    	
    	try {
    	BufferedWriter writer = new BufferedWriter(new FileWriter(MODE_PATH), 32);
    		try {
    			writer.write(modeStr + "\r\n");    			
    		} finally {
    			writer.close();
    		}  
    		
    		setAxis(MODE_AXIS_TABLE.get(modeStr));
    		
    		return 0;
    		
    	} catch (IOException e) { 
    		Log.e(TAG, "IO Exception when write: " + MODE_PATH, e);
    		return 1;
    	}
    	
    }
    
    /** set axis*/
    public static int setAxis(String axisStr) {
    	try {
        	BufferedWriter writer = new BufferedWriter(new FileWriter(AXIS_PATH), 32);
        		try {
        			writer.write(axisStr + "\r\n");
        		} finally {
        			writer.close();
        		}    		
        		return 0;
        		
        	} catch (IOException e) { 
        		Log.e(TAG, "IO Exception when write: " + AXIS_PATH, e);
        		return 1;
        	}    	
    }
    
    /** process handler */
    private class HdmiSwitchProgressHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
            case 0:		// confirm dialog 
                if (mProgress == STOP_PROGRESS) 
                	return;                     
                
                if (mProgress >= MAX_PROGRESS) {  
                	setMode(old_mode);
                	updateListDisplay(); 
                	confirm_dialog.dismiss();
                } else {
                    mProgress++;                    
                    confirm_dialog.getButton(AlertDialog.BUTTON_NEGATIVE)
                    	.setText(getText(R.string.dialog_str_cancel) 
                    			+ " (" + (MAX_PROGRESS - mProgress) + ")");
                    
                    mProgressHandler.sendEmptyMessageDelayed(0, 1000);                    
                }
                break;   
                
            case 1:		// hdmi check
            	if (mProgress2 == STOP_PROGRESS) 
                	return;  
            	
            	hdmi_stat = HdmiSwitch.isHdmiConnected(); 
                if (hdmi_stat_old == HDMI_DISCONNECTED) {            	 
                	if (hdmi_stat == HDMI_CONNECTED) {
                		hdmi_stat_old = hdmi_stat;
                		
                		if (confirm_dialog != null) {
                			mProgress = STOP_PROGRESS;
                			confirm_dialog.dismiss();
                		}
                		
                		updateActivityDisplay();
                	}
                } else {            	
                	if (hdmi_stat == HDMI_DISCONNECTED) {
                		hdmi_stat_old = hdmi_stat;  
                		
                		if (confirm_dialog != null) {
                			mProgress = STOP_PROGRESS;
                			confirm_dialog.dismiss();
                		}
                		
                		if (!HdmiSwitch.getCurMode().equals("panel"))
                     		HdmiSwitch.setMode("panel");
                		
                		updateActivityDisplay();
                	}
                }  
            	mProgressHandler.sendEmptyMessageDelayed(1, 3000); 
            	break;
            }
        }
    }
    
    /** mode <-> mode_str/axis */
	private static final Map<String, Object> MODE_STR_TABLE = new HashMap<String, Object>();
	private static final Map<String, String> MODE_AXIS_TABLE = new HashMap<String, String>();
	static {
		MODE_STR_TABLE.put("panel", R.string.mode_str_panel);
		MODE_STR_TABLE.put("480i", R.string.mode_str_480i);
		MODE_STR_TABLE.put("480p", R.string.mode_str_480p);
		MODE_STR_TABLE.put("576i", R.string.mode_str_576i);
		MODE_STR_TABLE.put("576p", R.string.mode_str_576p);
		MODE_STR_TABLE.put("720p", R.string.mode_str_720p);
		MODE_STR_TABLE.put("1080i", R.string.mode_str_1080i);
		MODE_STR_TABLE.put("1080p", R.string.mode_str_1080p);		
		
		MODE_AXIS_TABLE.put("panel", "0 0 800 480 0 0 18 18");
		MODE_AXIS_TABLE.put("480i", "0 0 800 480 0 0 18 18");
		MODE_AXIS_TABLE.put("480p", "0 0 800 480 0 0 18 18");
		MODE_AXIS_TABLE.put("576i", "0 48 800 480 0 48 18 18");
		MODE_AXIS_TABLE.put("576p", "0 48 800 480 0 48 18 18");
		MODE_AXIS_TABLE.put("720p", "240 120 800 480 240 120 18 18");
		MODE_AXIS_TABLE.put("1080i", "560 300 800 480 560 300 18 18");
		MODE_AXIS_TABLE.put("1080p", "560 300 800 480 560 300 18 18");
	}
    
}