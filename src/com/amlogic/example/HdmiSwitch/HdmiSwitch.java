package com.amlogic.example.HdmiSwitch;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.os.Bundle;
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
	/**
	 * need chmod in rootfs/init.rc
	 * chmod 0777 (MODE_PATH)
	 * chmod 0777 (AXIS_PATH)
	 * */
	private static final String DISP_CAP_PATH = "/sys/class/amhdmitx/amhdmitx0/disp_cap";
	private static final String MODE_PATH = "/sys/class/display/mode";
	private static final String AXIS_PATH = "/sys/class/display/axis";

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
            lp.width = (int) (display.getWidth() * 0.75);       	
        } else {
        	//lp.height = (int) (display.getHeight() * 0.75);
            lp.width = (int) (display.getWidth() * 0.5);            	
        }
        getWindow().setAttributes(lp);        
        
        /* update hdmi_state_str*/
        TextView tv = (TextView) findViewById(R.id.hdmi_state_str);
        if (isHdmiConnected())
        	tv.setText(getResources().getString((Integer)R.string.hdmi_state_str1));
        else
        	tv.setText(getResources().getString((Integer)R.string.hdmi_state_str2));
        
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
					Map<String, Object> list_item;
					//Log.i(TAG, "Count = " + lv.getAdapter().getCount());
					for (int i = 0; i < lv.getAdapter().getCount(); i++) {						
						list_item = (Map<String, Object>)lv.getAdapter().getItem(i);
						list_item.put("item_img", R.drawable.item_img_unsel);
					}					
					item.put("item_img", R.drawable.item_img_sel);
					
					//Log.i(TAG, "Set mode = " + item.get("mode"));			
					setMode((String)item.get("mode"));			
					
					((BaseAdapter) lv.getAdapter()).notifyDataSetChanged();
				}
				
			}        	
        });

        /* close button listener */
        Button btn_close = (Button) findViewById(R.id.title_btn_right);
        btn_close.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {				
				finish();
			}        	
        });       
        
        
    }
    
    /** mode <-> mode_str*/
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
	
    /** getListData */
    private List<Map<String, Object>> getListData() {    	
    	List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();	 
    	
    	for (String modeStr : getAllMode()) {
        	Map<String, Object> map = new HashMap<String, Object>();  
        	map.put("mode", modeStr);
        	map.put("item_text", getResources().getString((Integer)MODE_STR_TABLE.get(modeStr)));
        	if (modeStr.equals(getCurMode()))
        		map.put("item_img", R.drawable.item_img_sel);
        	else
        		map.put("item_img", R.drawable.item_img_unsel);
        	list.add(map);    		
    	}
    	
    	return list;
 	}       
    
    /** check hdmi connection*/
    private boolean isHdmiConnected() { 
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
    	
    	//list.add("480p");
    	//list.add("720p");    	
    	//list.add("1080p");
    	
    	try {
    		BufferedReader reader = new BufferedReader(new FileReader(DISP_CAP_PATH), 256);
    		try {
    			while ((modeStr = reader.readLine()) != null) {
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
    private String getCurMode() {
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
    private int setMode(String modeStr) {
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
    private int setAxis(String axisStr) {
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
    
}