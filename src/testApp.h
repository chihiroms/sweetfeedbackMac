#pragma once

#include 	"ofMain.h"
#include "ofxHttpUtils.h"
#include "ofxXmlSettings.h"

class testApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    
        // initializatin
        void newResponse(ofxHttpResponse & response);    
        ofxHttpUtils httpUtils; 
        vector <ofSerialDeviceInfo> deviceList;
        ofSerial	serial;
        void        serialWrite(char a);
    
        bool		setupSerial();
        bool		loadSettingXml();
        
    
        // utils
        string		ofxGetSerialString(ofSerial &serial, char until);
        string		toJSON(string data);    

		string		readSensorData();
		void		requestToServer();
        
 
        // for error handling   
        bool		port_OK, xml_OK, init_OK, network_OK;
        int         serial_errCount, init_errCount, network_errCount, initCount;   
        string      errMsg, initBuf; 
        bool		run;   
    
        	
        // settings.xml data
        float       lastRequestedTime;
        float       requestInterval;    
		int         portNumber;		
        string      deviceNickname;
        string      serverURL;   
    
		
        // data from gumball device
        string      deviceId;
        string		sensorReadString, sensorReadsJSON;          
    
        string      responseStr;
};
