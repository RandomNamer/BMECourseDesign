//
// Created by ZZY on 2021/10/28.
//

#include<iostream>
#include<string>
#include<map>
#include<ctime>

using namespace std;

#include<sqlite3.h>

#include "utils/commandline.h"
#include "customstorescp.h"
#include "utils/dcmimage.h"
#include "include/dcmtk/dcmdata/dcuid.h"
#include "include/dcmtk/dcmjpeg/djrplol.h"
#include "include/dcmtk/dcmjpeg/djrploss.h"
#include "include/dcmtk/dcmjpeg/djencode.h"
#include "include/dcmtk/dcmjpeg/djdecode.h"

const bool isTest = false;

constexpr char DEFAULT_OUTPUT_DIR[] = "/Users/zzy/Downloads/SCPStore/";
constexpr char DEFAULT_DB_PATH[] = "./instance.db";
const static string TABLE_NAME = "INSTANCE";
const static string TABLE_STRUCT = "("\
                                   "SOP_INSTANCE_ID TEXT PRIMARY KEY  NOT NULL, "\
                                   "SOP_CLASS_ID TEXT NOT NULL, "\
                                   "STUDY TEXT, "\
                                   "PATIENT TEXT, "\
                                   "FILE_DIR TEXT NOT NULL, "\
                                   "ADDED_AT INTEGER NOT NULL "\
                                   ");";
const static string INSERT_SNIPPET = "INSERT INTO " + TABLE_NAME + " VALUES (";
constexpr bool LOSSLESS_JPEG_COMPRESSION = true;
constexpr int COMPRESS_QUALITY = 90;
constexpr auto JPEG_FORMAT = EXS_JPEGProcess14SV1;

const static map<string, string> cmdMap = {
        {"-port", "Choose a port to listen"},
        {"-aet",  "Our AE Title"},
        {"-dir",  "Output directory for file store"},
        {"-db",   "Directory for database, if not provided, will use the db file under executable folder."},
        {"-flip", "Save flipped file copy"},
        {"-cvt2jpg", "Save a copy that is properly converted to JPEG transfer syntax"},
        {"-rotate", "Rotate Image and save a copy, you may provide an int value for degree representation."}
};

CommandLine args(cmdMap);


sqlite3* initDb(const string &location);

static int sqliteGenericCallback(void *NotUsed, int argc, char **argv, char **azColName);

bool storeFlippedImage(const string filename);

bool storeJpgXferSyntaxImage(string filename);

void test(){
    string fn = "/Users/zzy/Downloads/myscpstore/MR.1.2.156.112605.189250946103241.20200725081430.4.5140.37";
//    storeRotatedImage(fn, 45);
    storeJpgXferSyntaxImage(fn);
}

int main(int argc, char *argv[]) {
//    OFLog::configure(OFLogger::DEBUG_LOG_LEVEL);
    cout << "Welcome to Store SCP Redux!" << endl;
    if(isTest){
        test();
        return 0;
    }
    args.parse(argc, argv);
    CustomStorageSCP scp;
    string dbLoc;
    sqlite3 *db;
    bool needsFlip = false;
    bool needsJpegConvert = false;
    int rotateDeg = 0;
    try {
        scp.setAETitle(args["-aet"].c_str());
        scp.setPort(stoi(args["-port"]));
//        scp.setVerbosePCMode(true);
        if (args.exists("-dir")) scp.setOutputDirectory(args["-dir"].c_str());
        else scp.setOutputDirectory(DEFAULT_OUTPUT_DIR);
        if (args.exists("-db")) dbLoc = args["-db"];
        else dbLoc = DEFAULT_DB_PATH;
        if(args.exists("-flip")) needsFlip = true;
        if(args.exists("-cvt2jpg")) needsJpegConvert = true;
        if(args.exists("-rotate")){
            rotateDeg = stoi(args["-rotate"]);
        }
    } catch (std::invalid_argument &e) {
        cout << "Args Error, " << e.what() << endl;
        cout << "Arg Usage: \n";
        args.printOutAllArgs();
        exit(9);
    }

    db = initDb(dbLoc);
    if(db != nullptr){
//      if(true){
        OFList<OFString> xferSyntaxes;
        xferSyntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
        xferSyntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
        xferSyntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
        for (int i = 0; i < numberOfDcmShortSCUStorageSOPClassUIDs; i++) {
            scp.addPresentationContext(dcmShortSCUStorageSOPClassUIDs[i], xferSyntaxes);
        }
        scp.setInstanceStoredListener([&db, &needsJpegConvert, &needsFlip, &rotateDeg](auto filename, auto sopClass, auto sopInst, auto dataset) -> bool {
            char* err;
            cout << "Received file " << filename << " with SOP class " << sopClass << " and SOP Instance UID " << sopInst<< endl;
            if(needsFlip) storeFlippedImage(filename);
            if(needsJpegConvert) storeJpgXferSyntaxImage(filename);
            if(rotateDeg > 0) storeRotatedImage(filename, rotateDeg);

            OFString patientName, studyId;
            dataset->findAndGetOFString(DCM_PatientName, patientName);
            dataset->findAndGetOFString(DCM_StudyInstanceUID, studyId);
            const string reconstructedSql = INSERT_SNIPPET + "'" + sopInst + "', " + "'" + sopClass + "', '" + studyId.c_str() + "', '" + patientName.c_str() + "', '" + filename + "', " + to_string(time(0)) + " ); ";
            int insertResult = sqlite3_exec(db, reconstructedSql.c_str(), sqliteGenericCallback, nullptr, &err);
            if(insertResult != SQLITE_OK){
                if(err != NULL) cout << "Insert failed " << err << endl;
                else cout<<"Insert failed with error code "<<insertResult<<endl;
                sqlite3_free(err);
            } else {
                cout<<"Insert Success\nCurrent table content:\n"<<endl;
                system((string("sqlite3 ") + DEFAULT_DB_PATH + " \"SELECT * FROM "+ TABLE_NAME + ";\"").c_str());
            }

            //TODO: Add dataset parse

            return true;
        });

        scp.listen();
    }
    sqlite3_close(db);
    return 0;
}

sqlite3* initDb(const string &location) {
    sqlite3 *pSqlite3;
    int res = sqlite3_open(location.c_str(), &pSqlite3);
    char *zErrMsg;
    if (res) {
        cout << "DB open error, " << sqlite3_errmsg(pSqlite3) << endl;
    } else {
        cout << "Successfully opened DB " << location << endl;
        char *reconstructedSql = const_cast<char *>(("CREATE TABLE IF NOT EXISTS " + TABLE_NAME + TABLE_STRUCT).c_str());
        res = sqlite3_exec(pSqlite3, reconstructedSql, sqliteGenericCallback, nullptr, &zErrMsg);
        if (res != SQLITE_OK) {
            cout << "Create/Open table failed " << zErrMsg << endl;
            sqlite3_free(zErrMsg);
        } else {
            cout << "Create/Open table success" << endl;
            return pSqlite3;
        }
    }
    return nullptr;
}

static int sqliteGenericCallback(void *NotUsed, int argc, char **argv, char **azColName) {
    printf("sqliteGenericCallback result:\n");
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

//bool storeFlippedImage(const string filename) {
//    string newFileName = filename + "-flipped.dcm";
//    DcmFileFormat file;
//    auto res = file.loadFile(filename.c_str(), EXS_Unknown, EGL_noChange, DCM_MaxReadLength, ERM_autoDetect);
//    if(res.bad()){
//        cout<<"Failed to read file for "<<res.text()<<endl;
//        return false;
//    }
//    auto dataset = file.getDataset();
//    DcmElement* pixelData;
//    DcmElement* rows;
//    DcmElement* cols;
//    dataset->findAndGetElement(DCM_PixelData, pixelData);
//    dataset->findAndGetElement(DCM_Rows, rows);
//    dataset->findAndGetElement(DCM_Columns, cols);
//
    Uint16* imagePixelData;
//    Uint16* rowCount;
//    Uint16* colCount;
//    pixelData->getUint16Array(imagePixelData);
//    rows->getUint16Array(rowCount);
//    cols->getUint16Array(colCount);
//
//    Uint16* curPixel = imagePixelData;
//    Uint16 *correspondingPixel = imagePixelData + ((*rowCount - 1) * (*colCount));
//    for(Uint16 r = 0; r < (*rowCount)/2; r++ ){
//        for(Uint16 c = 0; c < *colCount; c++ ){
//            Uint16 tmp = *curPixel;
//            *curPixel = *correspondingPixel;
//            *correspondingPixel = tmp;
//            curPixel ++;
//            correspondingPixel ++;
//        }
//        correspondingPixel -= *colCount * 2;
//    }
//    file.loadAllDataIntoMemory();
//    res = file.saveFile(newFileName.c_str(), EXS_Unknown, EET_ExplicitLength, EGL_recalcGL, EPD_noChange, 0, 0, EWM_dataset);
//    if(res.bad()){
//        cout<<"Failed to write file for "<<res.text()<<endl;
//        return false;
//    }
//    cout<<"Flipped image to "<<newFileName<<endl;
//    return true;
//}

bool storeJpgXferSyntaxImage(string filename) {
    const string newFileName = filename + "-jpeg.dcm";
    DJEncoderRegistration::registerCodecs();
    DJDecoderRegistration::registerCodecs();
    DcmFileFormat file;
    auto res = file.loadFile(filename.c_str(), EXS_Unknown, EGL_noChange, DCM_MaxReadLength, ERM_autoDetect);
    if(res.bad()){
        cout<<"Failed to read file for "<<res.text()<<endl;
        return false;
    }
    auto dataset = file.getDataset();
    DcmXfer originalXfer(dataset->getOriginalXfer());
    if(originalXfer.isEncapsulated()){
       cout<<"DICOM file is already compressed, converting to uncompressed transfer syntax first"<<endl;
        if (EC_Normal != dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL)){
            cout<<"no conversion from compressed original to uncompressed transfer syntax possible!"<<endl;
            return false;
        }
    }
    const char* sopClass;
    if(file.getMetaInfo()->findAndGetString(DCM_MediaStorageSOPClassUID, sopClass).good()){
        if(sopClass == UID_MediaStorageDirectoryStorage){
            cout<<"Directory storage cannot be casted into JPEG xfer syntax"<<endl;
            return false;
        }
    }
    DcmXfer jpgXferSyntax(JPEG_FORMAT);
    const DcmRepresentationParameter* rp =  new DJ_RPLossless(6,0);
    if(!LOSSLESS_JPEG_COMPRESSION) rp = new DJ_RPLossy(COMPRESS_QUALITY);
    if (dataset->chooseRepresentation(JPEG_FORMAT, rp).good() && dataset->canWriteXfer(JPEG_FORMAT))
    {
        cout<<"Output transfer syntax " << jpgXferSyntax.getXferName() << " can be written"<<endl;
    } else {
        cout<<"no conversion to transfer syntax " << jpgXferSyntax.getXferName() << " possible!"<<endl;
        return false;
    }
    file.loadAllDataIntoMemory();
    res = file.saveFile(newFileName.c_str(), JPEG_FORMAT, EET_ExplicitLength, EGL_recalcGL, EPD_noChange, 0, 0, EWM_updateMeta);
    if(res.bad()){
        cout<<"Failed to write file for "<<res.text()<<endl;
        return false;
    }
    cout<<"JPEG-ed image to "<<newFileName<<endl;
    DJDecoderRegistration::cleanup();
    DJEncoderRegistration::cleanup();
    return true;
}

