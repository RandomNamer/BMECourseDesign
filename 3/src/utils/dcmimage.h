//
// Created by ZZY on 2021/11/1.
//

#ifndef DCMSTORESCU_DCMIMAGE_H
#define DCMSTORESCU_DCMIMAGE_H

#include<string>
#include<math.h>
using namespace std;

#include "../include/dcmtk/dcmdata/dcuid.h"
#include "../include/dcmtk/dcmjpeg/djrplol.h"
#include "../include/dcmtk/dcmjpeg/djrploss.h"
#include "../include/dcmtk/dcmjpeg/djencode.h"
#include "../include/dcmtk/dcmjpeg/djdecode.h"
#include "../include/dcmtk/dcmdata/dcfilefo.h"
#include "../include/dcmtk/dcmdata/dcdeftag.h"


typedef unsigned short us;
us* rotate(us* raw, us w, us h, double degree, int* nw, int* nh);
us bilinearInterpolate(us* img, int w, int h, int x, int y);
bool storeFlippedImage(const string filename);
bool storeRotatedImage(const string filename, const int degree);





#endif //DCMSTORESCU_DCMIMAGE_H
