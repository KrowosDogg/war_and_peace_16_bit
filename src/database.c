#include "database.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define DEFAULT_LABEL_DATABASE_SIZE 100

void constructLabelValuesDatabase(tDatabase *label_values)
{
    if (label_values == NULL)
    {
        printf("Error: database pointer is NULL\n");
        exit(1);
    }
    label_values->table_real_size = 0;
    label_values->table_allocated_size = DEFAULT_LABEL_DATABASE_SIZE;
    label_values->table = (tCortage*)malloc(label_values->table_allocated_size*sizeof(tCortage));
}

void destructLabelValuesDatabase(tDatabase *label_values)
{
    if (label_values == NULL)
    {
        printf("Error: database pointer is NULL\n");
        exit(1);
    }
    free(label_values->table);
    label_values->table_allocated_size = 0;
    label_values->table_real_size = 0;
}

void setLabelValue(tDatabase *label_values,
                   tLine label,
                   tAddress address)
{
    if (label_values->table_real_size >= label_values->table_allocated_size)
    {
        printf("Error: number of labels is too large (FIXME -- should realloc memory for Database)");
        exit(2);
    }
    tCortage new_cortage = {label, address};
    label_values->table[label_values->table_real_size] = new_cortage;
    label_values->table_real_size += 1;
}

/*
*    return address of label position in memory
*/
tAddress getLabelValue(const char *label, tDatabase label_values)
{
    for(int i = 0; i < label_values.table_real_size; i++)
    {
        if (strcmp(label_values.table[i].line.str, label) == 0)
        {
            return label_values.table[i].address;
        }
    }
    //if not found label return -1
    return -1;
}

void printLabelValues(tDatabase label_values)
{
    printf("--- Labels database contents: ---\n");
    for(int i = 0; i < label_values.table_real_size; i++)
    {
        printf("\t%20s\t%d\n", label_values.table[i].line.str,
                               label_values.table[i].address);
    }
}

// --------------------------------- Methods for tAddressesDatabase ----------------------------

void constructAddressesDatabase(tAddressDatabase *addresses_values)
{
    constructLabelValuesDatabase(addresses_values);
}

void storeAddress(tAddressDatabase *addresses_values, tAddress address)
{
    if (addressInAddressesBase(address, *addresses_values))
        return; //nothing to do.

    if (addresses_values->table_real_size >= addresses_values->table_allocated_size)
    {
        printf("Error: number of addresses is too large (FIXME -- should realloc memory for Database)");
        exit(2);
    }

    tCortage new_cortage = {{""}, address};
    addresses_values->table[addresses_values->table_real_size] = new_cortage;
    addresses_values->table_real_size += 1;
}

//Hard code... 26 is a number of latin letters
//all labels are from two big latin letters
tLine generateLabel(int i)
{
    tLine label;
    if (i >= 26*26) {
        printf("ERROR: Label cannot be generated!\n");
        exit(1);
    }
    label.str[1] = 'A' + i%26;
    i /= 26;
    label.str[0] = 'A' + i%26;
    label.str[2] = 0;
    return label;
}

void generateAddressesLabels(tAddressDatabase addresses_values)
{
    //sort addresses with Bubble sort
    for(int prohod = 1; prohod < addresses_values.table_real_size; prohod++)
    {
        for(int i = 0; i < addresses_values.table_real_size - prohod; i++)
        {
            if (addresses_values.table[i].address > addresses_values.table[i + 1].address) {
                tCortage tmp = addresses_values.table[i];
                addresses_values.table[i] = addresses_values.table[i + 1];
                addresses_values.table[i + 1] = tmp;
            }
        }
    }
    for(int i = 0; i < addresses_values.table_real_size; i++) {
        addresses_values.table[i].line = generateLabel(i);
    }
}

void printAddressesValues(tAddressDatabase addresses_values)
{
    printLabelValues(addresses_values);
}

int addressInAddressesBase(tAddress address, tAddressDatabase addresses_values)
{
    for(int i = 0; i < addresses_values.table_real_size; i++)
    {
        if (addresses_values.table[i].address == address)
        {
            return 1;
        }
    }
    return 0;
}

tLine getAddressLabel(tAddress address, tAddressDatabase addresses_values)
{
    for(int i = 0; i < addresses_values.table_real_size; i++)
    {
        if (addresses_values.table[i].address == address)
        {
            return addresses_values.table[i].line;
        }
    }
    //if not found address return string of zero size
    tLine label;
    label.str[0] = 0;
    return label;
}
