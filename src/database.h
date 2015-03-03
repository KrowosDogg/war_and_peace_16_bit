#ifndef DATABASE_H
#define DATABASE_H

#include "libarch.h"

typedef struct {
    tLine line;
    tAddress address;
} tCortage;

/*
*  contains pairs of (label, address) of types (tLine, tAddress)
*/
typedef struct {
    tCortage *table;
    unsigned int table_real_size;
    unsigned int table_allocated_size;
} tDatabase;

void consctructLabelValuesDatabase(tDatabase *label_values);

void desctructLabelValuesDatabase(tDatabase *label_values);

void setLabelValue(tDatabase *label_values,
                   tLine label,
                   tAddress address);

/*
*    return address of label position in memory
*           or -1 if label wasn't found
*/
tAddress getLabelValue(const char *label, tDatabase label_values);

void printLabelValues(tDatabase label_values);


#endif // DATABASE_H
