//
// Created by ZZY on 2021/11/1.
//

#ifndef DCMSTORESCU_CUSTOMSTORESCP_H
#define DCMSTORESCU_CUSTOMSTORESCP_H

#include <functional>
#include<stdio.h>

#include "include/dcmtk/dcmnet/dstorscp.h"
#include "include/dcmtk/dcmnet/scp.h"
#include "include/dcmtk/config/osconfig.h"


class CustomStorageSCP : public DcmStorageSCP {
public:
    void setInstanceStoredListener(std::function<bool(const char *, const char *, const char *, DcmDataset *)> l) {
        instanceStoredListener = l;
    }


protected:
    void notifyInstanceStored(const OFString &filename,
                              const OFString &sopClassUID,
                              const OFString &sopInstanceUID,
                              DcmDataset *dataset) const override {
        instanceStoredListener(filename.c_str(), sopInstanceUID.c_str(), sopInstanceUID.c_str(), dataset);
    }

private:
    std::function<bool(const char *, const char *, const char *, DcmDataset *)> instanceStoredListener =
            [](const char *filename, const char *, const char *, DcmDataset *) -> bool {
                std::cout << "Stored" << filename << std::endl;
                return true;
            };
};


#endif //DCMSTORESCU_CUSTOMSTORESCP_H
