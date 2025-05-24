#include <stdio.h>
#include <string.h>
#include "db.h"

#define DB_FILE "coordinates.db"

int db_lookup(const char *key, char *out_value) {
    FILE *fp = fopen(DB_FILE, "r");
    if (!fp) return 0;

    char line[256], db_key[64], db_val[128];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%63s %127[^\\n]", db_key, db_val) == 2) {
            if (strcmp(db_key, key) == 0) {
                strcpy(out_value, db_val);
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}
