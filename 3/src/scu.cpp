//
// Created by ZZY on 2021/10/14.
//
#include<stdio.h>
#include <iostream>
#include<map>
#include<string>
#include<stdlib.h>

using namespace std;

#include "utils/argparser.h"
#include "utils/commandline.h"

#include "./include/dcmtk/config/osconfig.h"
#include "./include/dcmtk/dcmnet/scu.h"
#include "./include/dcmtk/dcmnet/scp.h"
#include "./include/dcmtk/dcmnet/diutil.h"

using namespace std;

const static char* PEERTITILE = "CONQUESTSRV1";

const static map<string, string> cmdMap = {
        {"-host", "The address of the server"},
        {"-port",    "The port of the server"},
        {"-aet",     "Our AE Title"},
        {"-file",    "File input with full dir path"}
};


static bool findSOPClassAndInstanceInFile(
        const char *fname,
        char *sopClass,
        size_t sopClassSize,
        char *sopInstance,
        size_t sopInstanceSize);

int main(int argc, char *argv[]) {
    OFLog::configure(OFLogger::DEBUG_LOG_LEVEL);
    cout << "Welcome to Store SCU client of DICOM Redux" << endl;
    CommandLine args(cmdMap);
    args.parse(argc, argv);
    DcmSCU scu;
    try{
        scu.setAETitle(args["-aet"].c_str());
        scu.setPeerHostName(args["-host"].c_str());
        scu.setPeerPort(stoi(args["-port"]));
        scu.setPeerAETitle(PEERTITILE);
        OFList<OFString> xferSyntax;
        xferSyntax.push_back(UID_LittleEndianImplicitTransferSyntax);
        xferSyntax.push_back(UID_LittleEndianExplicitTransferSyntax);
        xferSyntax.push_back(UID_BigEndianExplicitTransferSyntax);
        scu.addPresentationContext(UID_VerificationSOPClass, xferSyntax);
//        scu.addPresentationContext(UID_CTImageStorage, xferSyntax);

        char* storeSopClass = new char[128];
        char* storeSopInstance = new char[128];
        bool doesContainStorageSOP = true;
        if (!findSOPClassAndInstanceInFile(args["-file"].c_str(), storeSopClass, 128, storeSopInstance, 128)){
            cout<<"Found no available SOP (Class/Instance) in file "<<args["-file"]<<endl;
            doesContainStorageSOP = false;
        } else if (dcmIsaStorageSOPClassUID(storeSopClass, ESSC_All)) {
            cout<<"Found no available Storage SOP (Class/Instance) in file "<<args["-file"]<<endl;
            doesContainStorageSOP = false;
        }
        if(doesContainStorageSOP)
            cout<<"Found available Storage SOP (Class/Instance) in file"<<storeSopClass<<" and "<<storeSopInstance<<endl ;
        else{
            cout<<"Adding an SOP Class for transfer to continue"<<endl ;
            storeSopClass = UID_CTImageStorage;
        }

        scu.addPresentationContext(storeSopClass, xferSyntax);

        OFCondition resp;
        resp = scu.initNetwork();
        if(resp.bad()){
            cout<<"Network init failed for\n"<<resp.text()<<endl;
            exit(9);
        }

        resp = scu.negotiateAssociation();
        if(resp.bad()){
            cout<<"Negotiate association failed for\n "<<resp.text()<<endl;
            exit(9);
        }


        auto pcid = scu.findPresentationContextID(storeSopClass, UID_LittleEndianExplicitTransferSyntax);
        if(pcid == 0) pcid = scu.findPresentationContextID(storeSopClass, UID_BigEndianExplicitTransferSyntax);
        if(pcid == 0) pcid = scu.findPresentationContextID(storeSopClass, UID_LittleEndianImplicitTransferSyntax);
        unsigned short statusCode;
        resp = scu.sendSTORERequest(pcid, args["-file"].c_str(), NULL, statusCode);
        if(resp.bad()){
            cout<<"C-STORE request failed for "<<resp.text()<<endl;
            exit(9);
        }
        cout<<"C-STORE transfer success"<<endl;

        scu.closeAssociation(DCMSCU_RELEASE_ASSOCIATION);

    } catch (std::out_of_range e) {
        cout <<"Args Error, "<< e.what() << endl;
        cout<<"Arg Usage: \n";
        args.printOutAllArgs();
        exit(9);
    }
    return 0;
}

static bool findSOPClassAndInstanceInFile(
        const char *fname,
        char *sopClass,
        size_t sopClassSize,
        char *sopInstance,
        size_t sopInstanceSize)
{
    DcmFileFormat ff;
    if (!ff.loadFile(fname, EXS_Unknown, EGL_noChange, DCM_MaxReadLength, ERM_autoDetect).good())
        return OFFalse;

    /* look in the meta-header first */
    OFBool found = DU_findSOPClassAndInstanceInDataSet(ff.getMetaInfo(), sopClass, sopClassSize, sopInstance, sopInstanceSize, false);

    if (!found)
        found = DU_findSOPClassAndInstanceInDataSet(ff.getDataset(), sopClass, sopClassSize, sopInstance, sopInstanceSize, false);

    return found;
}



