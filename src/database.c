#include "database.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_LABEL_DATABASE_SIZE 100

void consctructLabelValuesDatabase(tDatabase *label_values)
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

void desctructLabelValuesDatabase(tDatabase *label_values)
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
        printf("\t%20s\t%s\n", label_values.table[i].line.str,
                               label_values.table[i].address);
    }
}


