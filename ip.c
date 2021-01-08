#ifndef _PLUGIN_API_H
#define _PLUGIN_API_H

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <regex.h>

#define PATTERN "^[0-9]{0,3}.[0-9]{0,3}.[0-9]{0,3}.[0-9]{0,3}$"

/*
    Структура, описывающая опцию, поддерживаемую плагином.
*/
struct plugin_option {
    /* Опция в формате, поддерживаемом getopt_long (man 3 getopt_long). */
    struct option opt;
    /* Описание опции, которое предоставляет плагин. */
    const char *opt_descr;
};

/*
    Структура, содержащая информацию о плагине.
*/
struct plugin_info {
    /* Название плагина */
    const char *plugin_name;
    /* Длина списка опций */
    size_t sup_opts_len;
    /* Список опций, поддерживаемых плагином */
    struct plugin_option *sup_opts;
};

/*
    Функция, позволяющая получить информацию о плагине.

    Аргументы:
        ppi - адрес структуры, которую заполняет информацией плагин.

    Возвращаемое значение:
        0 - в случае успеха,
        1 - в случае неудачи (в этом случае продолжать работу с этим плагином нельзя).
*/
int plugin_get_info(struct plugin_info* ppi)
{
    if (ppi == NULL)
    {
        return 1;
    }
    ppi->plugin_name = "ipv4";
    ppi->sup_opts_len = 1;
    struct plugin_option *ip_option = malloc(sizeof(struct plugin_option) * ppi->sup_opts_len);
    ip_option->opt_descr = "Searches ip address";
    struct option *ip_opt = malloc(sizeof(struct option));
    ip_opt->name = "ipv4-addr";
    ip_opt->has_arg = required_argument;
    ip_opt->flag = NULL;
    ip_opt->val = 0;
    ip_option->opt = *ip_opt;

    //ppi->sup_opts = ip_option;

    ppi->sup_opts = malloc(sizeof(struct plugin_option));
    ppi->sup_opts->opt.name = "ipv4-addr";
    ppi->sup_opts->opt.has_arg = required_argument;
    ppi->sup_opts->opt.flag = NULL;
    ppi->sup_opts->opt.val = 0;

    return 0;
};

/*
    Фунция, позволяющая выяснить, отвечает ли файл заданным критериям.

    Аргументы:
        in_opts - список опций (критериев поиска), которые передаются плагину.
           struct option {
               const char *name;
               int         has_arg;
               int        *flag;
               int         val;
           };
           Поле name используется для передачи имени опции, поле flag - для передачи
           значения опции (в виде строки). Если у опции есть аргумент, поле has_arg
           устанавливается в ненулевое значение.

        in_opts_len - длина списка опций.

        out_buff - буфер, предназначенный для возврата данных от плагина. В случае ошибки
            в этот буфер, если в данном параметре передано ненулевое значение,
            копируется сообщение об ошибке.

        out_buff_len - размер буфера. Если размера буфера оказалось недостаточно, функция
            завершается с ошибкой.

    Возвращаемое значение:
          0 - файл отвечает заданным критериям,
        > 0 - файл НЕ отвечает заданным критериям,
        < 0 - в процессе работы возникла ошибка (код ошибки).
*/
int plugin_process_file(const char *fname,
                        struct option *in_opts[],
                        size_t in_opts_len,
                        char *out_buff,
                        size_t out_buff_len)
{
    FILE *current_file;
    //printf("-- %s\n", fname);
    if (strstr(fname, "share") != NULL)
    {
    	return 10;
    }
    
    //printf("		%s\n", fname);
    //printf("11++\n");
    current_file = fopen(fname, "r");
    //printf("11++\n");
    //printf("--- %p\n", current_file);
    //printf("11++\n");
    if (current_file == NULL)
    {
        fclose(current_file);
        
        return -1;
    }
    
    //printf ("--%s\n", fname);

    //printf("%s\n", in_opts[0]->flag);

    if (in_opts[0]->flag == NULL)
    {
        printf("Error: missing argument\n");
        fclose(current_file);
        return -1;
    }

    char *ip1 = (char *) in_opts[0]->flag;
    //printf("%s\n", ip1);
    //mstrcpy(ip,

    regex_t preg;
    int err,regerr;
    err = regcomp (&preg, PATTERN, REG_EXTENDED);
    if (err != 0)
    {
        char buff[512];
        regerror(err, &preg, buff, sizeof(buff));
        printf("%s\n", buff);
    }

    regmatch_t pm;
    regerr = regexec (&preg, ip1, 0, &pm, 0);
    if (regerr == 0)
    {
        //printf("true\n");
    }
    else
    {
        //printf("false\n");
        char errbuf[512];
        regerror(regerr, &preg, errbuf, sizeof(errbuf));
        //printf("%s\n", errbuf);
        printf("Error: invalid syntax of IP address\n");
        strcpy(out_buff, "invalid syntax");
        //out_buff = "invalid syntax";
        
        return -1;
    }

    char *s;
    s = malloc(sizeof(char) * 255);
    if (s == NULL)
        {
        	return 10;
        }
        
        if (strlen(s) > 255)
        {
        	return 10;
        }
    //printf("--\n");
    //printf("%s\n", fname);
    int i = 0;
    while (feof(current_file) == 0)
    {
        i++;
        if (i > 400)
        {
            fclose(current_file);
            
            return -1;
        }
        //printf("%s\n", fname);
        //printf("--\n");
        int yy;
        //printf("-->\n");
        yy = fscanf(current_file, "%s", s);
        //printf("-->\n");
        //printf("== %p\n", s);
        if (yy == 0)
        {
        	return 10;
        }
        if (strlen(s) > 255)
        {
        	return 10;
        }
        
        if (s == NULL)
        {
        	return 0;
        }
        //printf("%s\n", s);
        //printf("-- %s\n", s);
        if (strstr(s, ip1) != NULL)
        {
            fclose(current_file);
            return 0;
        }
    }




    fclose(current_file);
    return 10;
};

#endif
