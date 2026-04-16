#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#include "TFT_Handler.h"

// School Information
const School SCHOOL_INFO = {
    "PANGASINAN STATE UNIVERSITY",
    "URDANETA CITY CAMPUS"};

const char *COLLEGE = "COLLEGE OF ENGINEERING AND ARCHITECTURE";
const char *COURSE = "BS COMPUTER ENGINEERING";

// Project Information
const Project PROJECT_INFO = {
    "SOILMATE",
    "SMART FERTILIZER SYSTEM",
    "1.0.0"};

// Developers (names only)
const char *DEVELOPERS[] = {
    "MICHELLE ORLANDA",
    "MICCAH POQUIZ",
    "AILA DATANAGAN"};

const uint8_t DEVELOPER_COUNT = sizeof(DEVELOPERS) / sizeof(DEVELOPERS[0]);

#endif