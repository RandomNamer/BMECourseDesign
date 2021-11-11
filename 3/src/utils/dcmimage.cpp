//
// Created by ZZY on 2021/11/1.
//

#include "dcmimage.h"
#define PI 3.1415926


DcmFileFormat *openFile(const string filename) {
    DcmFileFormat* file = new DcmFileFormat;
    auto res = file->loadFile(filename.c_str(), EXS_Unknown, EGL_noChange, DCM_MaxReadLength, ERM_autoDetect);
    if (res.bad()) {
        cout << "Failed to read file for " << res.text() << endl;
        return nullptr;
    }
    return file;
}

bool storeFlippedImage(const string filename) {
    string newFileName = filename + "-flipped.dcm";
    DcmFileFormat *file = openFile(filename);
    if (file != nullptr) {
        auto dataset = file->getDataset();
        DcmElement *pixelData;
        DcmElement *rows;
        DcmElement *cols;
        dataset->findAndGetElement(DCM_PixelData, pixelData);
        dataset->findAndGetElement(DCM_Rows, rows);
        dataset->findAndGetElement(DCM_Columns, cols);

        Uint16 *imagePixelData;
        Uint16 *rowCount;
        Uint16 *colCount;
        pixelData->getUint16Array(imagePixelData);
        rows->getUint16Array(rowCount);
        cols->getUint16Array(colCount);

        Uint16 *curPixel = imagePixelData;
        Uint16 *correspondingPixel = imagePixelData + ((*rowCount - 1) * (*colCount));
        for (Uint16 r = 0; r < (*rowCount) / 2; r++) {
            for (Uint16 c = 0; c < *colCount; c++) {
                Uint16 tmp = *curPixel;
                *curPixel = *correspondingPixel;
                *correspondingPixel = tmp;
                curPixel++;
                correspondingPixel++;
            }
            correspondingPixel -= *colCount * 2;
        }
        file->loadAllDataIntoMemory();
        auto res = file->saveFile(newFileName.c_str(), EXS_Unknown, EET_ExplicitLength, EGL_recalcGL, EPD_noChange, 0,
                                  0, EWM_dataset);
        if (res.bad()) {
            cout << "Failed to write file for " << res.text() << endl;
            return false;
        }
        cout << "Flipped image to " << newFileName << endl;
        return true;
    }
    return false;
}

bool storeRotatedImage(const string filename, const int degree) {
    string newFileName = filename + "-rotated"+ to_string(degree)+".dcm";
    DcmFileFormat *file = openFile(filename);
    if (file != nullptr) {
        auto dataset = file->getDataset();
        DcmElement *pixelData;
        DcmElement *rows;
        DcmElement *cols;
        dataset->findAndGetElement(DCM_PixelData, pixelData);
        dataset->findAndGetElement(DCM_Rows, rows);
        dataset->findAndGetElement(DCM_Columns, cols);

        Uint16 *imagePixelData;
        Uint16 *rowCount;
        Uint16 *colCount;
        pixelData->getUint16Array(imagePixelData);
        rows->getUint16Array(rowCount);
        cols->getUint16Array(colCount);

        int *nw = new int;
        int *nh = new int;
        Uint16* newImage = rotate(imagePixelData, (int) *colCount, (int) *rowCount, (degree/180.0) * PI , nw, nh);
//        memcpy(imagePixelData, newImage, *nw * *nh);

        dataset->findAndDeleteElement(DCM_PixelData);
        dataset->findAndDeleteElement(DCM_Rows);
        dataset->findAndDeleteElement(DCM_Columns);

//        DcmPixelData el(DCM_PixelData);
//        el.createUint16Array(*nh * *nw , newImage);
       dataset->putAndInsertUint16Array(DCM_PixelData, newImage, *nw * *nh);
       dataset->putAndInsertUint16(DCM_Rows, *nh);
       dataset->putAndInsertUint16(DCM_Columns, *nw);
       auto res = dataset->saveFile(newFileName.c_str());

//        file->loadAllDataIntoMemory();
//        auto res = file->saveFile(newFileName.c_str(), EXS_Unknown, EET_ExplicitLength, EGL_recalcGL, EPD_noChange, 0,
//                                  0, EWM_dataset);
        if (res.bad()) {
            cout << "Failed to write file for " << res.text() << endl;
            return false;
        }
        cout << "Rotated image to " << newFileName << endl;
        return true;
    }
    return false;
}

us bilinearInterpolate(us *img, int w, int h, double x, double y) {
    int roundX = roundf(x);
    int roundY = roundf(y);
    if (roundX <= 0 || roundY <= 0 || roundX >= w || roundY >= h) return 0;
    double residueX = x - roundX;
    double residueY = y - roundY;
    auto item1 = (1.0 - residueX) * img[w * roundY + roundX] + residueX * img[w * roundY + roundX + 1];
    auto item2 = (1.0 - residueX) * img[w * (roundY + 1) + roundX] + residueX * img[w * (roundY + 1) + roundX + 1];
    auto interpolation = (1 - residueY) * item1 + residueY * item2;
    return (us) interpolation;
}

us *rotate(us *raw, us w, us h, double degree, int* nw, int* nh) {
    double fsin = sin(degree);
    double fcos = cos(degree);
    int newh = ceilf(abs(w * fsin) + abs(h * fcos));
    int neww = ceilf(abs(w * fcos) + abs(h * fsin));
    us *newImage = new(nothrow) us[neww * newh];
    if (!newImage) return nullptr;
    memset(newImage, 0, newh * neww);
    double xoffset = (w - neww * fcos - newh * fsin) / 2;
    double yoffset = (h + neww * fsin - newh * fcos) / 2;
    for (int y = 0; y < newh; y++) {
        for (int x = 0; x < neww; x++) {
            if(x%100 == 0 && y%100 == 0 ){
                assert(true);
            }
            double coordinateX = x * fcos + y * fsin + xoffset;
            double coordinateY = y * fcos - x * fsin + yoffset;
            newImage[neww * y + x] = bilinearInterpolate(raw, w, h, coordinateX, coordinateY);
        }
    }
    *nw= neww; *nh = newh;
    return newImage;
}




