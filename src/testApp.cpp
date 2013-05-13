#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	ofBackground(255);	
	ofSetLogLevel(OF_LOG_VERBOSE);	
	
	serial.listDevices();
	deviceList = serial.getDeviceList();
	
	port_OK = false;
    xml_OK = false;
    init_OK = false; initCount = 0;
    network_OK = true;
    
    serial_errCount = 0;
    init_errCount = 0;
    network_errCount = 0;
    
    xml_OK = loadSettingXml();
    
	port_OK = setupSerial();	

	sensorReadString = "";
    sensorReadsJSON = "";
    responseStr = "null";
    initBuf = "";
    
    lastRequestedTime = 0;

	ofAddListener(httpUtils.newResponseEvent, this,&testApp::newResponse);
	httpUtils.start();

	run = false;
}




//--------------------------------------------------------------
void testApp::update(){
    errMsg = "";

    if (port_OK && xml_OK) {
        if (init_OK){
            if (run && network_OK) {
                network_OK = true;
                
                // every ${requestInterval} seconds, send request
                if ((ofGetElapsedTimef() - lastRequestedTime) > requestInterval) {
                    
                    requestToServer();					
                    lastRequestedTime = ofGetElapsedTimef();                                          
                    
                } else {
                    
                    sensorReadString = readSensorData();
                    sensorReadsJSON = toJSON(sensorReadString);	
                    
                }
            } else if (!network_OK){
                
                network_OK = false;
                errMsg += "Problem in server connection";
                // network error
                run = false;
            }
        }

        // wait 60 seconds at maximum for initialization
        else if (!init_OK && init_errCount < 120) {
            
            if ((ofGetElapsedTimef() - lastRequestedTime) > 0.3) {
                
                serialWrite('E'); 
                
                string readBuf = ofxGetSerialString(serial, '\n');
                responseStr = "initializing... 0%";
                
                if (ofSplitString(readBuf, ",").size() < 2) {
                    if (readBuf == initBuf) {
                        initCount++;
                        responseStr = "initializing... " + ofToString(10*initCount) + "%";
                    } else {
                        init_errCount++;
                        initBuf = readBuf;                
                    }
                
                    if (initCount > 9) {
                        deviceId = ofSplitString(initBuf, "\r")[0];
                        init_OK = true;
                    }
                }
                
                lastRequestedTime = ofGetElapsedTimef();                                                          
            } 
            
            
        } else {
            errMsg += "Problem in initialization.\n";
            run = false;
        }       
        
    } else {
        errMsg = "";
        if (!port_OK)
            errMsg += "Problem in opening port.\n";
        
        if (!xml_OK)
            errMsg += "Problem in loading data/settings.xml.\n";
        
        run = false;
    }
} 

//--------------------------------------------------------------
void testApp::draw(){
    
    
    ofSetColor(255, 0, 0);
    ofDrawBitmapString(errMsg, 50, 20);
    
    
	ofSetColor(150);	
    string msg;          
    msg += "available devices:\n";
    for (int i = 0; i < deviceList.size(); i++) {
        msg +=  ofToString(i) + " : " + deviceList.at(i).getDevicePath();
		if (i == portNumber)
			msg += " <-- Selected";
		msg += "\n";
    }    
    msg += "\n-----------------------\n\n";
	msg += "ID: " + deviceId + "\n";
	msg += "nickname: " + deviceNickname + "\n";

	msg += "\n-----------------------\n\n";
	msg += "server url:\n" + serverURL + "\n";
	msg += "request interval(sec): " + ofToString(requestInterval) + "\n";

	msg += "\n-----------------------\n\n";
	msg += "read: " + ofToString(sensorReadString) + "\n";
    msg += "server response: " + responseStr + "\n\n";

	run? msg += "Hit RETURN to stop running: RUNNING\n" : msg += "Hit RETURN to start running: READY\n";
    
    ofDrawBitmapString(msg, 50, 50);
}

string testApp::readSensorData() {
	serialWrite('B');
	return ofxGetSerialString(serial, '\n');
}

void testApp::serialWrite(char a) {
    if (serial.writeByte(a)) {
        serial_errCount = 0;
    }
    else
        serial_errCount++;
    
    if (serial_errCount > 5)
        port_OK = false; 
}

void testApp::requestToServer() {
	if (deviceId != "") {
        string action_url = serverURL + "?id=" + deviceId + "&nickname=" + deviceNickname ;
        
        if (sensorReadsJSON!="")
            action_url  += "&data=" + sensorReadsJSON;
        
        ofxHttpForm form;
        form.action = action_url;
        form.method = OFX_HTTP_POST;
      //  httpUtils.addForm(form);
        ofxHttpResponse res = httpUtils.submitForm(form);
        
        if (res.status == 200) {
            network_errCount = 0;
        } else {
            network_errCount++;
        }
        
        if (network_errCount < 5)
            network_OK = true;
        else 
            network_OK = false;
        
        sensorReadsJSON = "";	
	}
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 
    char writeByte;  
	if ( (key > 64 && key < 70) || (key > 96 && key < 102) ) {
        if ( (key == 'a') || (key == 'A') ) {
            writeByte = 'A';
        } else if ( (key == 'b') || (key == 'B') ) {
            writeByte = 'B'; 
        } else if ( (key == 'c') || (key == 'C') ) {
            writeByte = 'C';  
        } else if ( (key == 'd') || (key == 'D') ) {
            writeByte = 'D';
        } else if ( (key == 'e') || (key == 'E') ) {
            writeByte = 'E'; 
		}
		if (port_OK && xml_OK && init_OK && network_OK && run)
			serialWrite(writeByte); cout << writeByte << endl;
    }
	else if (OF_KEY_RETURN)
		run = run? false:true;
}


string testApp::ofxGetSerialString(ofSerial &serial, char until) {

    string str;
    char ch;
    int ttl=1000;
    
    while ((ch=serial.readByte())>0 && ttl-->0 && ch!=until) {
        str += ch; 
    }
    
    if (ch==until) {
        string tmp=str;
        str="";
        return tmp;
    } else {
        return "";
    }
}


//--------------------------------------------------------------
string testApp::toJSON(string data) {
    
    if (data == "")
        return "";
    
    vector<string> split= ofSplitString(ofSplitString(data, "\n")[0], ",");
    
	if (split.size() > 2) {
        string json_data = "{";
        for (int i = 0; i < 3 ; i++) {
            int value = 0;
            if (split[i] != "")
                value = ofToInt(split[i]);
            
            json_data += "\"sensor_" + ofToString(i) + "\":" + ofToString(value) + ",";
        }   
        json_data += + "\"timestamp\":" + ofToString(ofGetUnixTime()) + "}";
        return json_data;
    } 
    return "";
}

bool testApp::loadSettingXml() {
	ofxXmlSettings XML;	
	if( XML.loadFile("settings.xml") ){
        
		portNumber = XML.getValue("settings:port_number", 0);
		deviceNickname = XML.getValue("settings:nickname", "");
		serverURL = XML.getValue("settings:url", "");
		requestInterval = ofToFloat(XML.getValue("settings:server_request_interval", ""));
        
		// dump
		cout << "port_number: " << portNumber << endl;
		cout << "nickname: " <<  deviceNickname << endl;
		cout << "url: " << serverURL << endl;
		cout << "server_request_interval: " << requestInterval << endl;
		return true;
	}
	return false;
}


void testApp::newResponse(ofxHttpResponse & response){
	responseStr = ofToString(response.status) + ": " + (string)response.responseBody;
    cout << responseStr << endl;
    
    if (response.status == 200) {
        network_errCount = 0;
        if ((string)response.responseBody == "candy")
            serialWrite('A');
    } else {
        network_errCount++;
    }
    
    if (network_errCount < 5)
        network_OK = true;
    else 
        network_OK = false;
}

bool testApp::setupSerial() {
	int baud = 9600;
	return serial.setup(portNumber, baud);
}



//--------------------------------------------------------------
void testApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
	
}

