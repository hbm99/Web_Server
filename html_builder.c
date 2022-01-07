#include "html_builder.h"
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>

// Finds extensions of files
char *find_exts(char *filename) {

    char *dot = strrchr(filename, '.');

    if (!dot || filename == dot) {
        return NULL;
    }

    return dot;
}


int is_dir(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

int is_hide(char *name) {

    char *dot = strrchr(name, '.');
    return dot == name && strcmp(name, "..") != 0;
}


const char *table_row_create_string(char *href, char *value) {
    
    char *ret = malloc(512);
    sprintf(ret, "<td><a href=\"%s\">%s</a></td>", href, value);
    return ret;
}

const char *table_row_create_int(char *href, long value) {
    char *ret = malloc(512);
    sprintf(ret, "<td><a href=\"%s\">%ld</a></td>", href, value);
    return ret;
}

char *build_html(char *subpath, char *root) {

    char *path = calloc(512, sizeof(char));
    strcat(path, root);
    strcat(path, subpath);
    DIR *dir_handler = opendir(path);

    char *table = calloc(100000, sizeof(char));
    while (1) {
        struct dirent *file = readdir(dir_handler);
        if (file == NULL)
            break;


        char *name = file->d_name;


        if (is_hide(name)) continue;

        //set href
        char *href = calloc(512, sizeof(char));
        sprintf(href, "%s%s", subpath, name);
        char *filep = calloc(512, sizeof(char));
        sprintf(filep, "%s%s", path, name);


        //set fields
        char *fields = calloc(1024, sizeof(char));

        struct stat stats;
        stat(filep, &stats);

        char *st_mtimespec = ctime((const time_t *) &stats.st_mtime);
        char *st_birthtimespec = ctime((const time_t *) &stats.st_ctime);


        if (is_dir(filep)) {
            strcat(href, "/");
            // case .. in root
            if (strcmp(name, "..") == 0) {
                if (strlen(subpath) == 1)
                    strcat(fields, table_row_create_string(href, strdup("<p style=\"color:red;\"><b>&#8258; root folder</b></p>")));
                else
                    strcat(fields, table_row_create_string(href, strdup("<p style=\"color:green;\"><b>&#8592;</b></p>")));
                strcat(fields, table_row_create_string(href, "  "));
                strcat(fields, table_row_create_string(href, "  "));
                strcat(fields, table_row_create_string(href, "  "));
                strcat(fields, table_row_create_string(href, "  "));


            } else {
                strcat(fields, table_row_create_string(href, name));
                strcat(fields, table_row_create_string(href, "folder"));
                strcat(fields, table_row_create_string(href, "  "));
                strcat(fields, table_row_create_string(href, st_mtimespec));
                strcat(fields, table_row_create_string(href, st_birthtimespec));
            }
        } else {

            //deleting extension in the file name
            char *extn = find_exts(name);

            if (extn != NULL) {
                *strstr(name, extn) = NULL;
            }

            strcat(fields, table_row_create_string(href, name));
            if (extn == NULL) {
                extn = strdup(".file");
            }
            strcat(fields, table_row_create_string(href, extn + 1));


            //find size
            FILE *fp;
            long int size = 0;
            fp = fopen(filep, "r");
            fseek(fp, 0, SEEK_END);
            size = ftell(fp);
            fclose(fp);

            strcat(fields, table_row_create_int(href, size));
            strcat(fields, table_row_create_string(href, st_mtimespec));
            strcat(fields, table_row_create_string(href, st_birthtimespec));

        }


        char *row = calloc(5000, sizeof(char));
        sprintf(row, "<tr class='file'>%s</tr>", fields);
        strcat(table, row);

    }
    char *page = calloc(200000, sizeof(char));
    sprintf(page,
            "HTTP/1.1 200 OK \n\n <!doctype html><html><head><meta charset=\"UTF-8\"><title>Web Server</title><link rel=\"stylesheet\" href=\".style.css\"><script src=\".sorttable.js\"></script></head><body><div id=\"container\"><h1>Web Server \n <small>(SO)</small></h1><table class=\"sortable\" id = \"the_table\"><thead><tr><th onclick=\"sortTable(0)\">Filename </th><th onclick=\"sortTable(1)\">Type </th><th onclick=\"sortTable(2)\">Size </th><th onclick=\"sortTable(3)\">Modification Time </th><th onclick=\"sortTable(4)\">Creation Time </th></tr></thead> <tbody> %s </tbody></table></div></body></html>",
            table);
    
    return page;
}
