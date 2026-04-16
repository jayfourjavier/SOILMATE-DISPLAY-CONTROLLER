#ifndef PLANTS_H
#define PLANTS_H

struct Plant
{
    const char *name;
    float n; // kg/ha
    float p;
    float k;
};

const Plant plantList[] = {
    {"TOMATO", 120, 60, 60},
    {"EGGPLANT", 120, 60, 60},
    {"OKRA", 90, 60, 60},
    {"CHILI PEPPER", 120, 60, 60},
    {"PECHAY", 90, 30, 30},
    {"KANGKONG", 60, 30, 30},
    {"LETTUCE", 90, 30, 30},
    {"CUCUMBER", 120, 60, 60},
    {"STRING BEANS", 30, 60, 60},
    {"BITTER GOURD", 120, 60, 60},
    {"BASIL", 60, 30, 30},
    {"SPRING ONION", 90, 60, 60},
    {"OREGANO", 30, 30, 30},
    {"CILANTRO", 60, 30, 30}};

const uint8_t plantCount = sizeof(plantList) / sizeof(plantList[0]);

#endif