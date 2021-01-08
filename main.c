
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ftw.h>
#include <dlfcn.h>
#include <limits.h>
#include <errno.h>
#include "plugin_api.h"
#include <inttypes.h>
#include <limits.h>
#define _XOPEN_SOURCE 500
#define _DEFAULT_SOURCE



#define no_argument            0
#define required_argument      1
#define optional_argument      2

int option_count = 0;
char *Version = "0.88";
struct option *long_options;
int inversion_flag = 0;
int condition_flag = 0;
char *condition;
FILE *log;

int founds = 0;
int (*get_info)(struct plugin_info*);
int (*plugin_get_main_function)(const char *fname,
                           struct option *in_opts[],
                           size_t in_opts_len,
                           char *out_buff,
                           size_t out_buff_len);

struct plugin{
    void *plugin_file;
    struct plugin_info *plugin_info;
    int (*plugin_get_main_function)(const char *fname,
                                    struct option *in_opts[],
                                    size_t in_opts_len,
                                    char *out_buff,
                                    size_t out_buff_len);
    int option_number;
    int found_option;
};

struct plugin *plugins;


int getopt_long(int argc,
                char * const argv[],
                const char *optstring,
                const struct option *longopts,
                int *longindex);

int log_print(char *message)
{
	fprintf(log, "%s\n", message);
	return 0;
}


int option_append(struct option *list, char *namex, int has_argx, int *flagx, int valx)
{
    if (list == NULL)
    {
        printf("Failed to append option\n");
        return 1;
    }
    
    //printf("222\n");
    
    list[option_count].name = namex;
    list[option_count].has_arg = has_argx;
    list[option_count].flag = flagx;
    list[option_count].val = valx;

    option_count++;
    return 0;
}

int plugin_append(struct plugin *plugins, int pos, char *s)
{
    //plugins[pos].plugin_file = dlopen(s, RTLD_LAZY);
    //printf("---%s\n", current_plugin->d_name);
    printf("%s\n", s);
    //printf("%d\n", pos);
	//plugins[pos] = malloc(sizeof(struct plugin));
	char *ss = malloc(sizeof(2 + strlen(s)));
	strncat(ss, "./", 2);
	strncat(ss, s, strlen(s));
	//printf("+++ %s\n", ss);
    plugins[pos].plugin_file = dlopen(ss, RTLD_LAZY||RTLD_GLOBAL);
    if (plugins[pos].plugin_file == NULL)
    {
        printf("Error: could not open plugin\n");
        //printf("+11+\n");
        exit(1);
    }
    else 
    {
        plugins[pos].plugin_info = malloc(sizeof(struct plugin_info));
        get_info = dlsym(plugins[pos].plugin_file, "plugin_get_info");
        plugin_get_main_function = dlsym(plugins[pos].plugin_file, "plugin_process_file");
        int err_check;
        err_check = get_info(plugins[pos].plugin_info);
        if (err_check == 1) {
            //printf("+12+\n");
            printf("Error: could not open plugin\n");
            exit(1);
        }
        else
        {

            plugins[pos].option_number = option_count;

            //plugins[pos].plugin_info->sup_opts = malloc(sizeof(struct option));

            /*option_append(long_options,
                          plugins[pos].plugin_info->sup_opts->opt.name,
                          plugins[pos].plugin_info->sup_opts->opt.has_arg,
                          plugins[pos].plugin_info->sup_opts->opt.flag,
                          plugins[pos].plugin_info->sup_opts->opt.val);*/

            long_options[option_count].name = plugins[pos].plugin_info->sup_opts->opt.name;
            //long_options[option_count].has_arg = plugins[pos].plugin_info->sup_opts->opt.has_arg;
            //long_options[option_count].flag = plugins[pos].plugin_info->sup_opts->opt.flag;

            //long_options[option_count].has_arg = required_argument;
            //long_options[option_count].flag = NULL;

            long_options[option_count].val = plugins[pos].plugin_info->sup_opts->opt.val;
            option_count++;


            printf("Plugin found: %s\n", plugins[pos].plugin_info->plugin_name);
            printf("Plugin option: %s\n", plugins[pos].plugin_info->sup_opts->opt.name);
            printf("Plugin main function added\n");


        }
        if (plugin_get_main_function != NULL)
        {
            plugins[pos].plugin_get_main_function = plugin_get_main_function;
            plugins[pos].found_option = 0;
            //printf("3\n");
        }
        else
        {
            printf("Error: failed to add plugin\n");
            exit(1);
        }

    }

    return 0;
}

int show_help(int x)
{
    //TODO finish help option
    printf("-----------------DESCRIPTION----------------\n");
    printf("This program searches for plugins in *.so and uses\n");
    printf("them to find files with different criteria.\n");
    printf("-------------------OPTIONS------------------\n");
    printf("-h\t shows this page\n");
    printf("-v\t show version of program\n");
    printf("-P\t add path to plugins directory\n");
    printf("-l\t add path to log file\n");
    printf("-C\t add condition for searching\n");
    printf("-N\t inversion of searching criteria\n");
    printf("--------------------------------------------\n");
    exit(0);
    //return 0;
}

int show_version(int x)
{
    printf("Version %s\n", Version);
    log_print("-v found, showing version");
    return 0;
}




//TODO search dir



/* List the files in "dir_name". */

int search (const char * dir_name, struct plugin *p, int plugin_count)
{
    //sleep(1);
    //printf("Searching....\n");
    //sleep(1);
    int xxx;

    DIR * d;

    /* Open the directory specified by "dir_name". */

    d = opendir (dir_name);
    if (d == NULL)
    {
    	if (founds != 0)
    	{
    		return 0;
    	}
        printf("No path to start from\n");
        log_print("No path");
    	
        show_help(0);
        return 234;
    }

    /* Check it was opened. */
    if (! d) {
        fprintf (stderr, "Cannot open directory '%s': %s\n",
                 dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }

    while (1) {
        struct dirent * entry;
        
        const char * d_name;

        /* "Readdir" gets subsequent entries from "d". */
        entry = readdir (d);
        if (! entry) {
            /* There are no more entries in this directory, so break
               out of the while loop. */
            break;
        }
        d_name = entry->d_name;
        //printf ("%s/%s\n", dir_name, d_name);
        /* Print the name of the file and directory. */
        //printf ("%s/%s\n", dir_name, d_name);


        /* If you don't want to print the directories, use the
	   following line: */
        //printf("!!!\n");
        char *buff;
        size_t buff_size;
        buff = malloc(sizeof(char) * 25);
        buff_size = 25;
        int fl_veritas;

        if (strcmp(condition, "AND") == 0)
        {
            fl_veritas = 1;
        }
        //printf("!!!\n");
        if (strcmp(condition, "OR") == 0)
        {
            fl_veritas = 0;
        }

        //fl_veritas = 1;

        if (! (entry->d_type & DT_DIR))
        {
	        //printf ("%s/%s\n", dir_name, d_name);
            //printf ("--%s\n", d_name);
            char *gh;
	
            //gh = malloc(sizeof(char) * (strlen(dir_name) + strlen(d_name) + 10));
            gh = malloc(sizeof(char) * 355);
            
		//printf("11  %s\n", gh);
		/*
            strncat(gh, dir_name, strlen(dir_name));
            strncat(gh, "/", 1);
            strncat(gh, d_name, 100);
            */
            //printf("%s\n", gh);
/*
		//sprintf(gh, "%s/%s", dir_name, d_name);
		snprintf(gh, "%s", dir_name, 123);
		//snprintf(gh, "/", 123);
		sprintf(gh, "%s", d_name, 123);
		//printf("11  %s\n", gh);
		//printf("== %s\n", dir_name);
	*/
		snprintf(gh, 123, "%s/%s", dir_name, d_name);	
		log_print("Opening file");
		log_print(gh);
    
    
		//printf("== %s\n", gh);
            if (strstr(gh, ".txt") != NULL)
            {
            //printf ("%s/%s\n", dir_name, d_name);
                //printf("not 0\n");

                for (int i = 0; i < plugin_count; i++)
                {
                    if (p[i].found_option == 1)
                    {
                        //printf("111\n");
                        //printf("11++\n");
                        //printf("%s\n", p[i].plugin_info->plugin_name);
                        //printf("%s\n", p[i].plugin_info->sup_opts[0].opt.name);
                        xxx = p[i].plugin_get_main_function(gh,
                                                            &p[i].plugin_info->sup_opts,
                                                            p[i].plugin_info->sup_opts_len,
                                                            buff,
                                                            buff_size);
                        
                        //printf("111\n");
                        if (strcmp(condition, "AND") == 0)
                        {
                            if (xxx != 0)
                            {
                                fl_veritas = 0;
                                //break;
                            }
                        }
                        //printf("!!!\n");
                        if (strcmp(condition, "OR") == 0)
                        {
                            if (xxx == 0)
                            {
                                fl_veritas = 1;
                                //printf("%d\n", i);
                                break;
                            }
                        }
                    }
                    //printf("=== %s\n", condition);
                    //printf("-> %d\n", xxx);


                }
                if ((fl_veritas == 1) && (inversion_flag == 0))
                {
                    printf("Found in file: \t%s\n", gh);
                    log_print("Found");
    
                    founds++;
                }

                if ((fl_veritas == 0) && (inversion_flag != 0))
                {
                    printf("Found in file: \t%s\n", gh);
                    log_print("Found");
                    founds++;
                }
                //printf("%d\n", strlen(buff));
                if (strlen(buff) != 0)
                {
                    //printf("Error: error with plugin\n");
                    //printf("%s\n", buff);
                    //exit(1);
                }
            }
            //free(gh);

            //FILE *f;
            //f = fopen(strcat(strcat(dir_name, "/"), d_name), "r");
            //(f == NULL) ? printf("err\n") : printf("no err\n");


            //printf("%d\n", xxx);
            //TODO FINISH
	    }




        if (entry->d_type & DT_DIR) {

            /* Check that the directory is not "d" or d's parent. */

            if (strcmp (d_name, "..") != 0 &&
                strcmp (d_name, ".") != 0) {
                int path_length;
                char path[PATH_MAX];

                path_length = snprintf (path, PATH_MAX,
                                        "%s/%s", dir_name, d_name);
                //printf ("%s\n", path);
                if (path_length >= PATH_MAX) {
                    fprintf (stdout, "Path length has got too long.\n");
                    exit (EXIT_FAILURE);
                }
                /* Recursively call "list_dir" with the new path. */
                search(path, p, plugin_count);
            }
        }
    }
    /* After going through all the entries, close the directory. */
    if (closedir (d)) {
        fprintf (stderr, "Could not close '%s': %s\n",
                 dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
}



int main(int argc, char* argv[]) {
    int path_Option_Flag = 0;
    const char* short_options = "-:P:l:C:Nvh";
    long_options = malloc(sizeof(struct option) * 12);
    //int *option_flags = malloc(sizeof(int) * 12);
    char *plugins_path = "./";
    int cond; // AND = 0, OR = 1
    char *log_path = malloc(sizeof(char) * 100);
    log_path = ".";
    //condition = malloc(sizeof(char) * 10);
    //TODO create log file
	//printf("Startxxx\n");
    option_append(long_options, "help", no_argument, NULL, 'h');
    option_append(long_options, "version", no_argument, NULL, 'v');
    option_append(long_options, "inversion", no_argument, NULL, 'N');
    option_append(long_options, "path", required_argument, NULL, 'P');
    option_append(long_options, "logpath", required_argument, NULL, 'l');
    option_append(long_options, "condition", required_argument, NULL, 'C');
    
    

    
    for (int i = 0; i < 6; i++)
    {
        //printf("%s\t%d\n", long_options[i].name, long_options[i].val);
    }
    
    int rez;
    int option_index = -1;

    int flag_plugin_option_found = 0;
	
    while ((rez = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        //printf("%c\n", rez);
        switch (rez) {
            case 'h': {
                //show_help(0);
                //exit(0);
                break;
            };
            case 'v': {
                show_version(0);
                exit(0);
            };

            case 'P': {
                if (optarg) {
                    plugins_path = optarg;
                    //printf("%s\n", plugins_path);
                } else {
                    printf("Error: option -P required argument\n");
                    exit(1);
                }
                break;
            };

            case 'l': {
                if (optarg) {
                    log_path = optarg;
                    //printf("%s\n", log_path);
                } else {
                    printf("Error: option -l required argument\n");
                    exit(1);
                }
                break;
            }

            case 'N':
            {
                inversion_flag++;
                break;
            }
            case 'C':
            {
                condition_flag++;
                condition = optarg;
                if (condition == NULL)
                {

                    condition = "AND";
                }
                break;
            }

            case ':':
            {
                printf("Error: missing argument\n");
                exit(1);
            }

            case '?':
            default: {
                break;
            };
        };
    };

    if (condition == NULL)
    {

        condition = "AND";
    }




char *lg = malloc(sizeof(char) * 100);
strncat(lg, log_path, 100);

	strncat(lg, "/log.txt", 9);
	//snprintf(log_path, 9,"/log.txt");

	log = fopen(lg, "w");
	log_print("Start...");
	if (inversion_flag != 0)
	{
		log_print("Inversion: On");
	}
	
	



    option_index = -1;
    optind = 0;
    //printf("123123123\n");
    //int (*get_info)(struct plugin_info*);
    struct plugin_info *ppi = malloc(sizeof(struct plugin_info));

    DIR *plugins_directory;
    //printf("..%s\n", plugins_path);
    plugins_directory = opendir(plugins_path);
    //printf("---\n");
    if (plugins_directory == NULL)
    {
        printf("Error: could not open directory with plugins\n");
        log_print("Error: could not open directory with plugins");
        fclose(log);
        exit(1);
    }
    //printf("C = %s\n", condition);
    struct plugin *plugins;
    struct dirent *file_current;
    plugins = malloc(sizeof(struct plugin) * 8); //$$$
    int plugins_count = 0;
    //printf("---\n");
    char *so = ".so";
    //printf("+1+\n");

    while ((file_current = readdir(plugins_directory)) != NULL)
    {
        //printf("%s\n", file_current->d_name);
        if (strstr(file_current->d_name, so) != NULL)
        {
            //printf("%s\n", file_current->d_name);
            //printf("1\n");
            plugin_append(plugins, plugins_count, file_current->d_name);
            log_print("Found plugin");
            log_print("Plugin added");
            plugins_count++;
        }
        //printf("**  %p\n", file_current);
    };
    printf("Found %d plugin(s)\n", plugins_count);
    //printf("---------------------\n");
	log_print("End working with plugins, start...");




    for (int i = 6; i < 6 + plugins_count; i++)
    {
        long_options[i].has_arg = required_argument;
        long_options[i].flag = NULL;
        //printf("%s\t%d\n", long_options[i].name, long_options[i].val);
    }

    for (int i = 5 + plugins_count; i >= 6; i--)
    {
        //printf("%s\t%d\n", long_options[i].name, long_options[i].val);
    }


    //printf("--------\n");
    /*
    for (int i = 0; i < plugins_count; i++)
    {
        printf("%s\t%p\t%d\n", plugins[i].plugin_info->plugin_name, plugins[i].plugin_get_main_function, plugins[i].option_number);
        printf("%s\n", plugins[i].plugin_info->sup_opts[0].opt.name);
    }
     */

    //printf("%p\n", plugins[0].plugin_file);
    //printf("HERE\n");
    int found_plugin_options = 0;

    //printf("1\n");
    /*
    getopt_long(argc, argv, "+:P:l:C::Nvh", long_options, &option_index);
    printf("1\n");
    option_index = 0;
    optind = 0;
    getopt_long(argc, argv, "+:P:l:C::Nvh", long_options, &option_index);
    printf("1\n");
    */
    int its_ok = 0;
    //getopt_long(argc, argv, "+:P:l:C::Nvh", long_options, &option_index);
    //printf("1\n");
    option_append(long_options, NULL, 0, NULL, 0);

    while ((rez = getopt_long(argc, argv, "+:P:l:C:Nvh", long_options, &option_index)) != -1) {
        //printf("%d\n", rez);
        //printf("%d\n", option_index);
        //option_index = -1;
        //printf("%p\n", &option_index);
        //option_index = 6;
        //printf("==  %d\n", optind);

        switch (rez) {
            case ':':
            {
                printf("Error: missing argument\n");
                log_print("Error: missing argument");
                fclose(log);
                exit(1);
            }

            case 'h':
            {
                show_help(0);
                log_print("-h found, showing help");
                //fclose(log);
                break;
            }
            case 'v':
            {
                break;
            }
            case 'P':
            {
                break;
            }
            case 'l':
            {
                break;
            }
            case 'N':
            {
                break;
            }
            case 'C':
            {
                break;
            }


            case 0:
            {
            	log_print("Found plugin option");
                //fclose(log);
                //printf("HERE\n");
                //printf("%d\n", option_index);
                //option_index = 1;
                //option_index = 6;
                //printf("%d\n", plugins_count);

                for (int i = 0; i < plugins_count; i++)
                {
                    //printf("%d\n", rez);

                    if (plugins[i].found_option == 1)
                    {
                        //optind = 0;
                    }
                    //printf("%s\n", long_options[option_index].name);
                    if ((strcmp(plugins[i].plugin_info->sup_opts[0].opt.name, long_options[option_index].name) == 0) && (plugins[i].found_option == 0))
                    {
                        its_ok = 1;
                        //printf("YE$$\n");
                        //printf("%s\n", optarg);
                        flag_plugin_option_found++;
                        //optind = 0;
                        if (optarg == NULL)
                        {

                            printf("Error: missing argument\n");
                            log_print("Error: missing argument");
                		fclose(log);
                            exit(1);
                        }
                        else
                        {
                            plugins[i].plugin_info->sup_opts->opt.flag = optarg;
                            plugins[i].found_option = 1;
                            found_plugin_options++;
                            //printf("!!!\n");
                            //optind = 0;
                        }
                    }
                }
                //optind = 0;
                if (its_ok == 0)
                {
                    printf("Error: unknown option\n");
                    log_print("Error: unknown option");
                	fclose(log);
                    exit(1);
                }
                //option_index = -1;
                break;

            }
                if (flag_plugin_option_found == 0)
                {
                    printf("Error: plugins options not found\n");
                    log_print("Error: plugin option not found");
                	fclose(log);
                    exit(1);
                }


            case '?':
            {
                printf("Error: unknown option\n");
                log_print("Error: unknown option");
                	fclose(log);
                //break;
                exit(1);
                //printf("%s\n", optarg);
            }
            default: {
                printf("Error: unknown option\n");
                log_print("Error: unknown option");
                	fclose(log);
                //break;
                exit(1);
                break;
            };
        };
    };


    for (int i = 0; i < 2; i++)
    {
        //printf("%s\t%d\n", plugins[i].plugin_info->plugin_name, plugins[i].found_option);
    }

    //if ((argc == 1) || ())
    //printf("!!!\n");
    log_print("Searching...");
    log_print("\n");
    
    int search_checker = search(argv[argc - 1], plugins, plugins_count);
    
    printf("Found:\t%d\n", founds);
    log_print("ENDED");
    fclose(log);
    //printf("&&&&&&%d\n", search_checker);
    if (search_checker == 234)
    {
        //show_help(234);
        //printf("Error: error while searching\n");
        //exit(1);
    }





    return 0;
}
