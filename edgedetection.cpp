//to compile: g++ -Wall -o outputname filename.cpp                              
//./outputname 
//shortcut:g++ -Wall -o output *.cpp *.h

/*
*This c++ code takes .ppm files only because those are easiest to convert to pixel arrays
*sobel algorithm implemented
	*if objects on image are to close together detection is not as good
	*contrast makes a big difference
	(yellow, orange and green on white background are not well detected
	while red and blue are detected)
*/

#include <iostream>
#include <fstream>//for file reading and writing
#include <string>
#include <vector>
#include <cstdlib>
#include <cctype>
#include <limits>
#include <cmath>
#include <algorithm>

//#define dbg(str) (cout << "dbg " << str << endl)//debuging on                 
#define dbg(str) //debuging off 

using namespace std;

//======read ppm images=======
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

//===generate output image(ppm p6 format)====
template <typename T>
bool makeimage(const string& filename, vector<T>& pixels, int width, int height, size_t pixelcnt, size_t maxvalue){
    ofstream output(filename.c_str(),ios::binary);
    if(!output){
        cout<<"could not create output image"<<endl;
        return false;
    }
    //write PPM header(binary RGB)
    output<<"P6\n"<<width<<" "<<height<<"\n255\n";
    unsigned char scaled=0;
    for(size_t i=0;i<pixelcnt;i++){
        //unsigned char rgb[3]={pixels[i],pixels[i],pixels[i]};
	scaled=(unsigned char)((float)pixels[i]*255.0f/maxvalue);
	unsigned char rgb[3]={scaled,scaled,scaled};
	
    	output.write((char*)rgb,3);
    }
    if(!output){
        cout<<"error writing output image"<<endl;
        return false;
    }

    return true;
}


//======Sobel operator======
//how it works:
//use sobel gradients to compare a pixels brightness with the brightness of its neighbors
//the sobel mtxs encure that:
//bright pixels on left contribute negative values
//bright pixels on right contribute positive values
//if both sides have similar brightness, they cancel each other out.
//so, the mtx is measuring the brightness difference between the left and right sides.

void sobel(vector<unsigned char>& pixels, vector<unsigned char>& edges, vector<float>& gx, vector<float>& gy, vector<float>& mag, int width, int height, size_t pixelcnt){
    //horizontal gradient(Gx)
    int sobelx[3][3]={
        {-1,0,1},
        {-2,0,2},
        {-1,0,1}
    };

    //vertical gradient(Gy)
    int sobely[3][3]={
        {-1,-2,-1},
        {0,0,0},
        {1,2,1}
    };

    //working with 3x3 mtx so:
    //start at 1 because pixel(0,0) has no pixels to compare to above and to the left
    //end at height-1 and width-1 because last pixel has no pixels to compare to below and to the right
    for(int y=1;y<height-1;y++){
        for(int x=1;x<width-1;x++){

            int sumx=0;
            int sumy=0;
	    //compare pixel to all its imediate neighbors
            for(int j=-1;j<=1;j++){
                for(int i=-1;i<=1;i++){

                    int pixel=pixels[(size_t)(y+j)*width+(x+i)];
		    //multiply neighboring pixels with sobel mtx
                    sumx+=pixel*sobelx[j+1][i+1];
                    sumy+=pixel*sobely[j+1][i+1];
                }
            }

            size_t index=(size_t)y*width+x;

            gx[index]=sumx;//horizontal brightness change
            gy[index]=sumy;//vertical brightness change

	    //G=sqrt((Gx)^2+(Gy)^2)
            mag[index]=sqrt((float)(sumx*sumx+sumy*sumy));

            //threshold: only keep strong edges
            if(mag[index]>200){
                edges[index]=255;
            }else{
	    	edges[index]=0;
	    }
        }
    }
}


//=======
int main(){
    ifstream file("test-images/simpleshapesbigger.ppm",ios::binary);
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
        //cout<<"pixel R:"<<(int)r<<" G:"<<(int)g<<" B:"<<(int)b;
   
        //convert RGB to grayscale (brightness)
        int gray=(299*r+587*g+114*b)/1000;

        //scale to 0-255
        pixels[i]=gray*255/maxcolor;
	
	//print brightness value of pixel
        //cout<<" pixel:"<<(int)pixels[i]<<endl;
    }
    
    //image analysis
    vector<unsigned char> edges;
    vector<float> gx, gy, mag;
    edges.resize(pixelcnt,0);
    gx.resize(pixelcnt,0);
    gy.resize(pixelcnt,0);
    mag.resize(pixelcnt,0);

    sobel(pixels, edges, gx, gy, mag, width, height, pixelcnt);
    makeimage("sobeloutput.ppm", edges, width, height, pixelcnt, 255);
    makeimage("magnitude.ppm", mag, width, height, pixelcnt, 1443);
    cout<<"edge detection finished"<<endl;

return 0;
}
