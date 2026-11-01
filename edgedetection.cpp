//to compile: g++ -Wall -o outputname filename.cpp                              
//./outputname 
//shortcut:g++ -Wall -o output *.cpp *.h

/*
*This c++ code takes .ppm files only because those are easiest to convert to pixel arrays
*/

#include <iostream>
#include <fstream>//for file reading and writing
#include <string>
#include <vector>
#include <cstdlib>
#include <cctype>
#include <limits>

//#define dbg(str) (cout << "dbg " << str << endl)//debuging on                 
#define dbg(str) //debuging off 

using namespace std;

//read the next PPM header value, skipping whitespaces and comments
bool readValue(ifstream& file, string& value){
   /* while(file >> value){
        if(value[0] == '#'){
            string comment;
            getline(file, comment);
        }else{
            return true;
        }
    }
    return false;
}*/

    //linux line ending:\n
    //windows line ending:\r\n
    value="";
    char c;
    //read comments and whitespaces
    while(file.get(c)){
        if(c=='#'){
            while(file.get(c) && c!='\n' && c!='\r'){}//skip commented line
            if(c=='\r' && file.peek()=='\n'){//if \r\n then also get \n
                file.get();
	    }
        }else if(!isspace((unsigned char)c)){
            value+=c;
            break;
        }
    }
    if(value.empty()){
        return false;
    }
    
    //read the rest of the line
    //note: isspace consideres \r and \n as whitespace
    while(file.get(c)){
        if(isspace((unsigned char)c)){
            //handle windows line endings
            if(c=='\r' && file.peek()=='\n'){
                file.get();
	    }
            break;
        }
        value+=c;
    }
    return true;
}


int main(){
    ifstream file("white-gray-black.ppm",ios::binary);
    if(!file){
        cout<<"could not open image"<<endl;
        return 1;
    }

    string format, value;
    int width, height, maxcolor;

    if(!readValue(file, format) || !readValue(file,value)){return 1;}
    width = atoi(value.c_str());

    if(!readValue(file, value)){return 1;}
    height = atoi(value.c_str());

    if(!readValue(file, value)){return 1;}
    maxcolor = atoi(value.c_str());
    
    if((format!="P3" && format!="P6") || width<=0 || height<=0 || maxcolor<=0 || maxcolor>65535){
        cout<<"invalid or unsupported PPM file"<<endl;
        return 1;
    }

    //check that image can fit in RAM
    size_t pixelcnt=(size_t)width*(size_t)height;
    if(pixelcnt>numeric_limits<size_t>::max()/sizeof(unsigned char)){
        cout<<"image too large"<<endl;
        return 1;
    }
    
    vector<unsigned char> pixels(pixelcnt);
    
    cout<<"image loaded succesfully"<<endl;
    cout <<"format:"<<format<<endl;
    cout<<"width:"<<width<<endl;
    cout<<"height:"<<height<<endl;
    cout<<"maxcolor:"<<maxcolor<<endl;
    
    //read RGB values and make one grayscale(brightness) value per pixel
    for(size_t i=0;i<pixelcnt;i++){
        int r, g, b;

        if(format=="P3"){
            //read text pixel values
            if(!readValue(file, value)){return 1;}
            r=atoi(value.c_str());

            if(!readValue(file, value)){return 1;}
            g=atoi(value.c_str());

            if(!readValue(file, value)){return 1;}
            b=atoi(value.c_str());
	    
        }else if(format=="P6"){
             //read binary pixel values
            if(maxcolor<256){
                unsigned char rgb[3];
                if(!file.read((char*)rgb, 3)){return 1;}
                r=rgb[0];
                g=rgb[1];
                b=rgb[2];
            }else{
                unsigned char rgb[6];
                if(!file.read((char*)rgb, 6)){return 1;}
                r=rgb[0]*256+rgb[1];
                g=rgb[2]*256+rgb[3];
                b=rgb[4]*256+rgb[5];
            }
        }

        //validate pixel values
        if(r<0 || r>maxcolor || g<0 || g>maxcolor || b<0 || b>maxcolor){
            cout<<"invalid pixel value"<<endl;
            return 1;
        }

	//print RGB value of first pixel
        cout<<"pixel R:"<<(int)r<<" G:"<<(int)g<<" B:"<<(int)b;
   
        //convert RGB to grayscale (brightness)
        int gray=(299*r+587*g+114*b)/1000;

        //scale to 0-255
        pixels[i]=gray*255/maxcolor;
	
	//print brightness value of first pixel
        cout<<" pixel:"<<(int)pixels[i]<<endl;
    }
    
    
    //image analysis
    
    

return 0;
}
