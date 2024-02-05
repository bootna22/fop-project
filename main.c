#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>
#include <time.h>
#include <fnmatch.h>


#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED  "\e[0;31m"
#define ANSI_COLOR_YELLOW  "\e[0;33m"
#define ANSI_COLOR_GREEN "\e[0;32m"

#define MAX_LINE_LENGTH 1000
#define MAX_FILE_SIZE 5 * 1024 * 1024 // 5 MB
#define MAX_CHARACTERS 20000

int init(int argc, char* const argv[]);
char* find_jax_directory();
int make_inside_jax();
int add_file(char path[]);
void add_directory(const char *path);
int add(int argc, char* const argv[]);
void read_staged_files();
int is_staged(const char *path);
void list_files(const char *path, int depth, int current_depth);
time_t get_stored_time(char path[]);
int is_modified(char path[], time_t stored_time);
void commit(int argc, char* const argv[]);
int increment_commit_id();

char* staged_files[1000];
int num_staged_files = 0;

char *supported_hooks[] = {"todo-check", "eof-blank-space", "format-check", "balance-braces", "indentation-check", "static-error-check", "file-size-check", "character-limit", "time-limit"};


int init(int argc, char* const argv[])
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    char tmp_cwd[1024];
    bool exists = false;
    struct dirent *entry;
    do 
    {
        // find .neogit
        DIR *dir = opendir(".");
        while ((entry = readdir(dir)) != NULL) 
        {
            struct stat path_stat;
            stat(entry->d_name, &path_stat);
            if (S_ISDIR(path_stat.st_mode) && strcmp(entry->d_name, ".jax") == 0)
                exists = true;
                
        }
        closedir(dir);

        // update current working directory
        getcwd(tmp_cwd, sizeof(tmp_cwd));

        // change cwd to parent
        if (strcmp(tmp_cwd, "/") != 0) 
        {
            chdir("..");
        }

    } while (strcmp(tmp_cwd, "/") != 0);

    chdir(cwd);

    if(exists)
        printf("Already initialized\n");
    else
    {
        mkdir(".jax", 0755);
        printf("inititalized jax repository successfully\n");
        make_inside_jax();
    }
}


// char* find_jax_directory() 
// {
//     char* original_path;
//     getcwd(original_path, sizeof(original_path));
//     char* path = getcwd(NULL, 0);
//     while (strcmp(path, "/") != 0) 
//     {
//         DIR* dir = opendir(path);
//         struct dirent* entry;
//         while ((entry = readdir(dir)) != NULL) 
//         {
//             if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".jax") == 0) 
//             {
//                 closedir(dir);
//                 chdir(original_path);
//                 return path;
//             }
//         }
//         closedir(dir);
//         chdir("..");
//         path = getcwd(NULL, 0);
//     }
//     chdir(original_path);
//     return NULL;
// }


char* find_jax_directory() 
{
    char* original_path = malloc(PATH_MAX);
    if (original_path == NULL) {
        // handle error
    }
    getcwd(original_path, PATH_MAX);
    char* path = getcwd(NULL, 0);
    while (strcmp(path, "/") != 0) 
    {
        DIR* dir = opendir(path);
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) 
        {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".jax") == 0) 
            {
                closedir(dir);
                chdir(original_path);
                free(original_path);
                return path;
            }
        }
        closedir(dir);
        chdir("..");
        free(path);
        path = getcwd(NULL, 0);
    }
    chdir(original_path);
    free(original_path);
    return NULL;
}


int make_inside_jax()
{
    char fuckyouvscode[900];
    char jax_address[950];
    strcpy(fuckyouvscode, find_jax_directory());
    sprintf(jax_address, "%s/.jax", fuckyouvscode);
    
    
    char user_information[1024];
    sprintf(user_information, "%s/login", jax_address);


    FILE* file = fopen(user_information, "w");
    // fprintf(file, "username : \n"); //ADD THE ACTUAL USERNAME LATER
    // fprintf(file, "email : \n"); //ADD THE ACTUAL EMAIL LATER
    fclose(file);

    char staged[1000];
    sprintf(staged, "%s/staged", jax_address);
    file = fopen(staged, "w");
    fclose(file);

    char stage_time[1000];
    sprintf(stage_time, "%s/stage_time", jax_address);
    file = fopen(stage_time, "w");
    fclose(file);

    char hooks[1000];
    sprintf(hooks, "%s/hooks", jax_address);
    file = fopen(hooks, "w");
    fclose(file);

    char all_files[1000];
    sprintf(all_files, "%s/all_files", jax_address);
    file = fopen(all_files, "w");
    fclose(file);

    char message_shortcuts[1024];
    sprintf(message_shortcuts, "%s/message_shortcuts", jax_address);
    mkdir(message_shortcuts, 0755);

    char commits[1024];
    sprintf(commits, "%s/commits", jax_address);
    mkdir(commits, 0755);

    char HEAD[1024];
    sprintf(HEAD, "%s/HEAD", jax_address);
    file = fopen(HEAD, "w");
    fprintf(file, "master");
    fclose(file);

    char branches[1024];
    sprintf(branches, "%s/branches", jax_address);
    mkdir(branches, 0755);
    char master[1100];
    sprintf(master, "%s/master", branches);
    file = fopen(master, "w");
    fprintf(file, "-1");   // <- Beanch head is stored in its respective file
    fclose(file);

    char current_commit[1024];
    sprintf(current_commit, "%s/current_commit", jax_address);
    file = fopen(current_commit, "w");
    fprintf(file, "-1");
    fclose(file);

    char tags[1000];
    sprintf(tags, "%s/tags", jax_address);
    mkdir(tags, 0755);
    
}


time_t get_stored_time(char path[]) {
    char path_to_all_files[1024];
    sprintf(path_to_all_files, "%s/.jax/all_files", find_jax_directory());

    FILE* file = fopen(path_to_all_files, "r");
    char line[1024];
    char* stored_path = NULL;
    while (fgets(line, sizeof(line), file)) 
    {
        // Remove the newline character at the end of the line
        line[strcspn(line, "\n")] = 0;

        if (stored_path == NULL) {
            // This line is a path, so store it
            stored_path = strdup(line);
        } else {
            // This line is a time, so check if the path matches
            if (strcmp(stored_path, path) == 0) 
            {
                // The path is in the file, so return the stored time
                fclose(file);
                free(stored_path);
                return (time_t)atol(line);
            }
            // The path does not match, so clear the stored path
            free(stored_path);
            stored_path = NULL;
        }
    }
    fclose(file);

    // The path is not in the file, so return 0
    return 0;
}


int find_branch_head(char branch_name[])
{
    char path_to_branch[200];
    sprintf(path_to_branch, "%s/.jax/branches/%s", find_jax_directory(), branch_name);
    FILE* file = fopen(path_to_branch, "r");
    int branch_head;
    fscanf(file, "%d", &branch_head);
    return branch_head;
}

void redo() {
    char path_to_all_files[1024];
    sprintf(path_to_all_files, "%s/.jax/all_files", find_jax_directory());

    FILE* file = fopen(path_to_all_files, "r");
    char line[1024];
    char* stored_path = NULL;
    while (fgets(line, sizeof(line), file)) 
    {
        // Remove the newline character at the end of the line
        line[strcspn(line, "\n")] = 0;

        if (stored_path == NULL) {
            // This line is a path, so add it
            // printf("%s\n", line);
            add_file(line);
        } else {
            // This line is a time, so do fucking nothing

        }
    }
    fclose(file);
}

char *get_relative_path(char *file_path, char *jax_directory) {
    return strstr(file_path, jax_directory) + strlen(jax_directory);
}


int is_modified(char path[], time_t stored_time) {
    struct stat attrib;
    if (stat(path, &attrib) == -1) {
        printf("Error with file %s: %s\n", path, strerror(errno));
        return -1;
    }
    time_t current_modification_time = attrib.st_mtime;

    if (current_modification_time > stored_time) {
        // The file has been modified
        return 1;
    } else {
        // The file has not been modified
        return 0;
    }
}

int add_file(char path[]) {
    char* original_path;
    getcwd(original_path, sizeof(original_path));

    char* absolute_path = realpath(path, NULL);
    if (absolute_path == NULL) {
        printf("Error with file %s: %s\n", path, strerror(errno));
        return -1;
    }
    
    char path_to_staged[1024];
    sprintf(path_to_staged, "%s/.jax/staged", find_jax_directory());
    
    // Check if the path is already in the file
    FILE* file = fopen(path_to_staged, "r");
    char line[1024];
    while (fgets(line, sizeof(line), file)) 
    {
        // Remove the newline character at the end of the line
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, absolute_path) == 0) 
        {
            fclose(file);
            free(absolute_path);
            return 2;  // Return a specific value to indicate the file was already added
        }
    }
    fclose(file);

    // If the path is not in the file, add it
    file = fopen(path_to_staged, "a");
    fprintf(file, "%s\n", absolute_path);
    fclose(file);

    // Get the modification time of the file
    struct stat attrib;
    if (stat(absolute_path, &attrib) == -1) {
        printf("Error with file %s: %s\n", absolute_path, strerror(errno));
        free(absolute_path);
        return -1;
    }
    time_t modification_time = attrib.st_mtime;

    // // Store the modification time in the stage_time file
    // char path_to_time[1024];
    // sprintf(path_to_time, "%s/.jax/stage_time", find_jax_directory());
    // file = fopen(path_to_time, "a");
    // fprintf(file, "%ld\n", modification_time);
    // fclose(file);

    // Store the path and modification time in the all_files file
    char path_to_all_files[1024];
    sprintf(path_to_all_files, "%s/.jax/all_files", find_jax_directory());
    file = fopen(path_to_all_files, "a");
    fprintf(file, "%s\n%ld\n", absolute_path, modification_time);
    fclose(file);

    free(absolute_path);

    chdir(original_path);
    
    return 0;
}


void add_directory(const char *path) {
    // printf("hellnA\n");
    char original_path[1000];
    getcwd(original_path, sizeof(original_path));

    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char full_path[2000];
            if (entry->d_type == DT_DIR) {
                sprintf(full_path, "%s/%s", path, entry->d_name);
                add_directory(full_path);  // Recursively list files in subdirectory
            } else {
                sprintf(full_path, "%s/%s", path, entry->d_name);
                // Get the stored modification time of the file
                time_t stored_time = get_stored_time(full_path);  // You need to implement this function

                // Check if the file has been modified
                if (is_modified(full_path, stored_time) == 1) {
                    // The file has been modified, so add it
                    add_file(full_path);
                }
            }
        }
    }
    closedir(dir);
}



int add(int argc, char* const argv[])
{
    char original_path[1000];
    getcwd(original_path, sizeof(original_path));
    if(strcmp(argv[2], "-f") == 0)
    {
        int i = 3;
        while(argv[i] != NULL)
        {
            
            // char temp[1000];
            // getcwd(temp, sizeof(temp));
            // printf("%s\n", temp);
            char path[1024];
            strcpy(path, argv[i]);

            struct stat s;
            if (stat(path, &s) == 0) 
            {
                if (s.st_mode & S_IFDIR) 
                {
                    // IT IS A DIRECTORY
                    // printf("%s\n", path);
                    add_directory(path);
                } 
                else if (s.st_mode & S_IFREG) 
                {
                    // IT IS A FILE
                    add_file(path);
                    chdir(original_path);
                } 
                else 
                {
                    // printf("%s is not a file or a directory\n", path);
                }
            }
            else
            {
                printf("Error with file %s: %s\n", path, strerror(errno));
            }
            i++;
            
        }
    }
    else if(strcmp(argv[2], "-n") == 0)
    {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        list_files(cwd, atoi(argv[3]), 0);
    }
    else if(strcmp(argv[2], "-redo") == 0)
    {
        redo();
    }
    else
    {
        int i = 2;
        while(argv[i] != NULL)
        {
            // printf("salam aleik");
            char path[1024];
            strcpy(path, argv[i]);

            struct stat s;
            if (stat(path, &s) == 0)  
            {
                if (s.st_mode & S_IFDIR) 
                {
                    // IT IS A DIRECTORY
                    // printf("bia folder add konim\n");
                    add_directory(path);
                    printf("Directory files added successfully\n");

                } 
                else if (s.st_mode & S_IFREG) 
                {
                    // IT IS A FILE
                    // printf("file bood kalak");
                    if(add_file(path) == 2)
                        printf("File was already added\n");
                    else
                        printf("File was addeed successfully\n");
                    chdir(original_path);
                } 
                else 
                {
                    printf("%s is not a file or a directory\n", path);
                }
            }
            else 
            {
                printf("%s does not exist\n", path);
            }
            i++;
        }
    }   
}

void read_staged_files() {
    char staged_address[1024];
    sprintf(staged_address, "%s/.jax/staged", find_jax_directory());
    FILE *file = fopen(staged_address, "r");
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;
        staged_files[num_staged_files] = strdup(line);
        num_staged_files++;
    }
    fclose(file);
}

int is_staged(const char *path) {
    for (int i = 0; i < num_staged_files; i++) {
        // printf("Comparing %s and %s\n", path, staged_files[i]);  // Add this line
        if (strcmp(path, staged_files[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


void list_files(const char *path, int depth, int current_depth) {
    if (depth == 0) return;
    read_staged_files();
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".jax") != 0) {
            char full_path[1024];
            sprintf(full_path, "%s/%s", path, entry->d_name);
            for (int i = 0; i < current_depth; i++) {
                printf("\t");
            }
            if (entry->d_type == DT_DIR) {
                printf(ANSI_COLOR_BLUE "%s: %s\n" ANSI_COLOR_RESET, entry->d_name, is_staged(full_path) ? "staged" : "not staged");
                list_files(full_path, depth - 1, current_depth + 1);
            } else {
                printf("%s: %s\n", entry->d_name, is_staged(full_path) ? "staged" : "not staged");
            }

        }
    }

    closedir(dir);
}

void ensure_directory_exists(char *dir_path) {
    struct stat st = {0};

    if (stat(dir_path, &st) == -1) {
        mkdir(dir_path, 0755);
        FILE* file = fopen("/home/matin/jax_global/login", "w");
        fclose(file);
        file = fopen("/home/matin/jax_global/commit_id", "w");
        fprintf(file, "0\n");
        fclose(file);
    }
}

int increment_commit_id()
{
    FILE* file = fopen("/home/matin/jax_global/commit_id", "r");
    if (file == NULL) {
        perror("Failed to open file for reading");
        return -1;
    }

    
    /**/
    int id;
    if (fscanf(file, "%d", &id) != 1) {
        perror("Failed to read id");
        fclose(file);
        return -1;
    }
    fclose(file);

    id++;

    file = fopen("/home/matin/jax_global/commit_id", "w");
    if (file == NULL) {
        perror("Failed to open file for writing");
        return -1;
    }

    if (fprintf(file, "%d\n", id) < 0) {
        perror("Failed to write id");
        fclose(file);
        return -1;
    }
    fclose(file);
    /**/
    return id;
}

int get_last_commit_id(const char *path_to_commits) {
    DIR *dir = opendir(path_to_commits);
    struct dirent *entry;
    int max_id = -1;

    while ((entry = readdir(dir)) != NULL) {
        int id = atoi(entry->d_name);
        if (id > max_id) {
            max_id = id;
        }
    }

    closedir(dir);
    return max_id;
}


int is_file_empty(char *filename) {
    struct stat st;

    if (stat(filename, &st) != 0) {
        return -1;  // Return -1 if stat fails
    }

    return (st.st_size == 0);
}

void copy_file(const char* src_path, const char* dst_path) {
    char command[1024];
    sprintf(command, "cp \"%s\" \"%s\"", src_path, dst_path);
    system(command);
}

int count_files(const char* path) {
    DIR *dir = opendir(path);
    if(dir == NULL) {
        printf("Error opening directory\n");
        return 0;
    }

    int count = 0;
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        // Skip the current directory and the directory above
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the full path of the entry
        char full_path[1024];
        sprintf(full_path, "%s/%s", path, entry->d_name);

        // Check if the entry is a directory
        struct stat sb;
        if(stat(full_path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
            // If the entry is a directory, recursively count the files in it
            count += count_files(full_path);
        } else {
            // If the entry is a file, increment the count
            count++;
        }
    }

    closedir(dir);
    return count;
}

void copy_file_to_commit_dir(const char* src_path, const char* relative_path, const char* commit_dir) {
    char dst_path[1024];
    sprintf(dst_path, "%s/%s", commit_dir, relative_path);

    // Create directories as needed
    char* p = strchr(dst_path + 1, '/');
    while(p != NULL) {
        *p = 0;
        mkdir(dst_path, 0755);
        *p = '/';
        p = strchr(p + 1, '/');
    }

    // If the file exists, delete it
    if(access(dst_path, F_OK) != -1) {
        remove(dst_path);
    }

    // Copy the file
    copy_file(src_path, dst_path);
}

void commit(int argc, char* const argv[]) {
    if (argc < 4) {
        printf("Invalid command. Usage: jax commit -m \"message\" | -s <shortcut-name>\n");
        return;
    }

    char path_to_jax[900];
    sprintf(path_to_jax, "%s/.jax", find_jax_directory());

    char path_to_staged[1024];
    sprintf(path_to_staged, "%s/.jax/staged", find_jax_directory());

    char current_branch[100];
    char path_to_HEAD[200];
    sprintf(path_to_HEAD, "%s/.jax/HEAD", find_jax_directory());

    FILE* file = fopen(path_to_HEAD, "r");
    fscanf(file, "%s", current_branch);
    fclose(file);

    char path_to_branch[200];
    sprintf(path_to_branch, "%s/.jax/branches/%s", find_jax_directory(), current_branch);

    char path_to_current_commit[250];
    sprintf(path_to_current_commit, "%s/.jax/current_commit", find_jax_directory());

    if(is_file_empty(path_to_staged) == 1)
    {
        printf("nothing to commit\n");
        return;
    }

    FILE* filee = fopen(path_to_current_commit, "r");
    int curr;
    fscanf(filee, "%d", &curr);
    fclose(filee);
    if(!(curr == -1 || curr == find_branch_head(current_branch)))
    {
        printf("you must be in the project HEAD to commit\n");
        return;
    }

    char message[100];
    if (strcmp(argv[2], "-m") == 0) {
        strcpy(message, argv[3]);
    } else if (strcmp(argv[2], "-s") == 0) {
        // The user provided a shortcut instead of a message
        char shortcut_path[1024];
        sprintf(shortcut_path, "%s/.jax/message_shortcuts/%s", find_jax_directory(), argv[3]);
        FILE *file = fopen(shortcut_path, "r");
        if (file == NULL) {
            printf("Shortcut \"%s\" does not exist.\n", argv[3]);
            return;
        }
        fgets(message, sizeof(message), file);
        fclose(file);
    } else {
        printf("Invalid command. Usage: jax commit -m \"message\" | -s <shortcut-name>\n");
        return;
    }

    if(strlen(message) > 72)
    {
        printf("Commit message too long\n");
        return;
    }



    int commit_id;
    commit_id = increment_commit_id();

    char commit_dir[1024];
    sprintf(commit_dir, "%s/commits/%d", path_to_jax, commit_id);
    // printf("%s\n", commit_dir);
    // Create a new directory for the commit
    char path_to_commits[1024];
    int last_commit_id;
    sprintf(path_to_commits, "%s/commits", path_to_jax);

    // last_commit_id = get_last_commit_id(path_to_commits);
    last_commit_id = find_branch_head(current_branch);


    // copy the prevoius commit
    char copy_command[200];
    // printf("cp -R %s/.jax/commits/%d/* %s/.jax/commits/%d\n", find_jax_directory(), last_commit_id, find_jax_directory(), commit_id);
    mkdir(commit_dir, 0755);
    sprintf(copy_command, "cd \"%s/.jax/commits/%d\" && cp -R * \"%s/.jax/commits/%d\"", find_jax_directory(), last_commit_id, find_jax_directory(), commit_id);
    if(last_commit_id != -1);
        system(copy_command);
   

    file = fopen(path_to_staged, "r");
    if(file == NULL) {
        printf("Error opening file\n");
        return;
    }

    char line[1024];
    while(fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;
        
        char* relative_path = strstr(line, find_jax_directory());
        if(relative_path == line) {
            relative_path += strlen(find_jax_directory());
            if(*relative_path == '/') {
                relative_path++;
            }
        }
        // printf("rell : %s\n", relative_path);

        copy_file_to_commit_dir(line, relative_path, commit_dir);

    }
    fclose(file);





   
    char path_to_login_local[2000];
    sprintf(path_to_login_local, "%s/.jax/login", find_jax_directory());
    FILE* file1 = fopen(path_to_login_local, "r");
    FILE* file2 = fopen("/home/matin/jax_global/login", "r");
    char l_username[100], g_username[100], l_email[100], g_email[100];
    long long l_time, g_time;
    fscanf(file1, "%s\n%s\n%lld", l_username, l_email, &l_time);
    fscanf(file2, "%s\n%s\n%lld",g_username, g_email, &g_time);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char path_to_info[100];
    sprintf(path_to_info, "%s/.jax/commits/%d/info", find_jax_directory(), commit_id);
    file = fopen(path_to_info, "w");
    if(file == NULL) {
        printf("Error opening file\n");
        return ;
    }
    // IMPORTANT ORDER : ID, TIME, USERNAME, EMAIL, MESSAGE, PREVIOUS COMMIT



    fprintf(file, "%d\n", commit_id);
    fprintf(file, "%02d-%02d-%04d %02d:%02d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
    if(l_time > g_time)
    {
        fprintf(file, "%s\n", l_username);
        fprintf(file, "%s\n", l_email);
    }
    else
    {
        fprintf(file, "%s\n", g_username);
        fprintf(file, "%s\n", g_email);
    }
    fprintf(file, "%s\n", message);
    fprintf(file, "%d", last_commit_id);
    fclose(file);

    // OUTPUT
    printf("commited staged files succesfully\n");
    printf("commit id : %d\n", commit_id);
    printf("commit date and time : %02d-%02d-%04d %02d:%02d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
    if(l_time > g_time)
        printf("commit author : %s %s\n", l_username, l_email);
    else
        printf("commit author : %s %s\n", g_username, g_email);
    printf("commit message : %s\n", message);

    FILE* fp = fopen(path_to_staged, "w");
    fclose(fp);
    // UPDATE BRNACH HEAD
    // printf("oh hey\n");
    file = fopen(path_to_branch, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fprintf(file, "%d", commit_id);
    // printf("oh hoy");

    file = fopen(path_to_current_commit, "w");
    fprintf(file, "%d", commit_id);
    fclose(file);

}




void config(int argc, char* argv[])
{
    // printf("%s\n%s\n", argv[3], argv[4]);
    if(strcmp(argv[2], "-global") == 0)
    {
        FILE* file = fopen("/home/matin/jax_global/login", "r");
        if (file == NULL) {
            perror("Error opening file");
            return;
        }
        char old_username[100];
        char old_email[100];
        if (fscanf(file, "%99s", old_username) != 1) {
            printf("Error reading username\n");
            return;
        }
        if (fscanf(file, "%99s", old_email) != 1) {
            printf("Error reading email\n");
            return;
        }
        fclose(file);
        // printf("%s\n%s\n", old_username, old_email);

        if(strcmp(argv[3], "user.name") == 0)
        {
            printf("name\n");
            char username[100];
            strcpy(username, argv[4]);
            file = fopen("/home/matin/jax_global/login", "w");
            fprintf(file, "%s\n", username);
            fprintf(file, "%s\n", old_email);
            time_t now = time(NULL);
            fprintf(file, "%ld", now);
            fclose(file);
        }
        if(strcmp(argv[3], "user.email") == 0)
        {
            printf("email\n");
            char email[100];
            strcpy(email, argv[4]);
            file = fopen("/home/matin/jax_global/login", "w");
            fprintf(file, "%s\n", old_username);
            fprintf(file, "%s\n", email);
            time_t now = time(NULL);
            fprintf(file, "%ld", now);
            fclose(file);
        }
    }
    else
    {
        char path_to_login[1024];
        sprintf(path_to_login, "%s/.jax/login", find_jax_directory());   
        
        FILE* file = fopen(path_to_login, "r");
        if (file == NULL) {
            perror("Error opening file");
            return;
        }
        char old_username[100];
        char old_email[100];
        if (fscanf(file, "%99s", old_username) != 1) {
            //printf("Error reading username\n");
            //return;
        }
        if (fscanf(file, "%99s", old_email) != 1) {
            //printf("Error reading email\n");
            //return;
        }
        fclose(file);
        // printf("%s\n%s\n", old_username, old_email);

        if(strcmp(argv[2], "user.name") == 0)
        {
            printf("name\n");
            char username[100];
            strcpy(username, argv[3]);
            file = fopen(path_to_login, "w");
            fprintf(file, "%s\n", username);
            fprintf(file, "%s\n", old_email);
            time_t now = time(NULL);
            fprintf(file, "%ld", now);
            fclose(file);
        }
        if(strcmp(argv[2], "user.email") == 0)
        {
            printf("email\n");
            char email[100];
            strcpy(email, argv[3]);
            file = fopen(path_to_login, "w");
            fprintf(file, "%s\n", old_username);
            fprintf(file, "%s\n", email);
            time_t now = time(NULL);
            fprintf(file, "%ld", now);
            fclose(file);
        }

    }
}

time_t parse_date(const char* date_str) {
    struct tm tm = {0};
    strptime(date_str, "%d-%m-%Y %H:%M", &tm);
    tm.tm_sec = 0;  // Ensure seconds are set to 0 if not included in format
    tm.tm_isdst = -1;  // Let mktime determine if daylight saving time is in effect
    return mktime(&tm);
}


void print_commit_info(const char* commit_dir, const char* author, const char* word, time_t since, time_t before) {
    char info_file[1024];
    sprintf(info_file, "%s/info", commit_dir);

    FILE* file = fopen(info_file, "r");
    if (file == NULL) {
        // perror("Error opening file");
        return;
    }

    int commit_id;
    if (fscanf(file, "%d\n", &commit_id) != 1) {
        printf("Error reading commit id\n");
        return;
    }

    char time_str[100];
    if (fgets(time_str, sizeof(time_str), file) == NULL) {
        printf("Error reading time of commit\n");
        return;
    }
    // Remove newline character at the end of time_str
    time_str[strcspn(time_str, "\n")] = 0;

    char username[100];
    if (fscanf(file, "%99[^\n]\n", username) != 1) {
        printf("Error reading username\n");
        return;
    }

    char email[100];
    if (fscanf(file, "%99[^\n]\n", email) != 1) {
        printf("Error reading email\n");
        return;
    }

    char commit_message[100];
    if (fgets(commit_message, sizeof(commit_message), file) == NULL) {
        printf("Error reading commit message\n");
        return;
    }
    // Remove newline character at the end of commit_message
    commit_message[strcspn(commit_message, "\n")] = 0;

    if (author != NULL && strcmp(username, author) != 0) {
        return;
    }

    if (word != NULL && strstr(commit_message, word) == NULL) {
        return;
    }

    struct tm time_of_commit = {0};
    strptime(time_str, "%d-%m-%Y %H:%M", &time_of_commit);
    time_of_commit.tm_sec = 0;  // Ensure seconds are set to 0 if not included in format
    time_of_commit.tm_isdst = -1;  // Let mktime determine if daylight saving time is in effect



    if (since != -1 && mktime(&time_of_commit) < since) {
        return;
    }

    if (before != -1 && mktime(&time_of_commit) > before) {
        return;
    }


    fclose(file);
    // printf("dalghak\n");
    printf("Time of commit: %s\n", time_str);
    printf("Commit id : %d\n", commit_id);
    printf("Commit message: %s\n", commit_message);
    printf("Username and email: %s <%s>\n", username, email);

        int file_count = count_files(commit_dir);
    printf("Number of files in commit %d\n", file_count - 1);

    printf("-----------------------------------\n");
}



void logg(int argc, char* argv[]) {
    char path_to_commits[100];
    sprintf(path_to_commits, "%s/.jax/commits/", find_jax_directory());
    int last_commit_id = get_last_commit_id(path_to_commits);

    int max_commits = last_commit_id;  // print all commits by default
    char* author = NULL;
    char* word = NULL;
    time_t since = -1;
    time_t before = -1;

    if(argc > 2 && strcmp(argv[2], "-branch") == 0)
    {
        if(argc < 4) {
            printf("Branch name not provided\n");
            return;
        }
        char *branch_name = argv[3];
        char branch_file_path[300];
        sprintf(branch_file_path, "%s/.jax/branches/%s", find_jax_directory(), branch_name);

        FILE *branch_file = fopen(branch_file_path, "r");
        if (branch_file == NULL) {
            printf("Branch '%s' does not exist\n", branch_name);
            return;
        }

        int commit_number;
        if (fscanf(branch_file, "%d", &commit_number) != 1) {
            printf("Could not read commit number from branch file\n");
            fclose(branch_file);
            return;
        }

        fclose(branch_file);

        while (commit_number != -1) {
            char commit_dir[300];
            sprintf(commit_dir, "%s/.jax/commits/%d", find_jax_directory(), commit_number);

            print_commit_info(commit_dir, author, word, since, before);

            char info_file_path[330];
            sprintf(info_file_path, "%s/info", commit_dir);

            FILE *info_file = fopen(info_file_path, "r");
            if (info_file == NULL) {
                printf("Could not open info file\n");
                return;
            }

            // Skip to the 6th line
            for (int i = 0; i < 5; i++) {
                if (fscanf(info_file, "%*[^\n]\n") != 0) {
                    printf("Could not read line from info file\n");
                    fclose(info_file);
                    return;
                }
            }

            if (fscanf(info_file, "%d", &commit_number) != 1) {
                printf("Could not read next commit number from info file\n");
                fclose(info_file);
                return;
            }

            fclose(info_file);
        }        
    }
    else
    {
        if (argc > 2) {
            if (strcmp(argv[2], "-n") == 0 && argc > 3) {
                max_commits = atoi(argv[3]);
            } else if (strcmp(argv[2], "-author") == 0 && argc > 3) {
                author = argv[3];
            } else if (strcmp(argv[2], "-search") == 0 && argc > 3) {
                word = argv[3];
            } else if (strcmp(argv[2], "-since") == 0 && argc > 3) {
                since = parse_date(argv[3]);
            } else if (strcmp(argv[2], "-before") == 0 && argc > 3) {
                before = parse_date(argv[3]);
            }
        }

        int printed_commits = 0;
        for (int i = last_commit_id; i >= 1 && printed_commits < max_commits; i--) {
            char commit_dir[1024];
            sprintf(commit_dir, "%s/.jax/commits/%d", find_jax_directory(), i);

            struct stat st;
            if (stat(commit_dir, &st) == 0) {  // check if the directory exists
                print_commit_info(commit_dir, author, word, since, before);
                printed_commits++;
            }
        }
    }
}


void delete_files_except_jax(const char* path) {
    DIR *dir = opendir(path);
    if(dir == NULL) {
        printf("Error opening directory\n");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        // Skip the current directory, the directory above, and the .jax directory
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".jax") == 0) {
            continue;
        }

        // Construct the full path of the entry
        char full_path[1024];
        sprintf(full_path, "%s/%s", path, entry->d_name);

        // Check if the entry is a directory
        struct stat sb;
        if(stat(full_path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
            // If the entry is a directory, recursively delete its contents
            delete_files_except_jax(full_path);
            // Then delete the directory itself
            rmdir(full_path);
        } else {
            // If the entry is a file, delete it
            remove(full_path);
        }
    }

    closedir(dir);
}

void checkout(int argc, char* const argv[]) {
    if(argc < 3) {
        printf("Please provide a commit ID or branch name\n");
        return;
    }

    // Check if argv[2] is purely a number
    char* end;
    int commit_id = strtol(argv[2], &end, 10);
    if(*end != '\0') {
        // argv[2] is not purely a number
        if(strcmp(argv[2], "HEAD") == 0)
        {
            char path_to_HEAD[200];
            sprintf(path_to_HEAD, "%s/.jax/HEAD", find_jax_directory());
            FILE* file2 = fopen(path_to_HEAD, "r");
            char uselessfucker[100];
            fscanf(file2, "%s", uselessfucker);
            int head = find_branch_head(uselessfucker);
            char *new_argv[4];  // adjust the size as needed

            new_argv[0] = argv[0];
            new_argv[1] = argv[1];

            char head_str[10];  
            sprintf(head_str, "%d", head);
            new_argv[2] = head_str;

            new_argv[3] = NULL;  

            checkout(argc, new_argv);

        }
        else
        {

            char path_to_HEAD[200];
            sprintf(path_to_HEAD, "%s/.jax/HEAD", find_jax_directory());
            FILE* file2 = fopen(path_to_HEAD, "w");
            fprintf(file2, "%s", argv[2]);

            int commit_id = find_branch_head(argv[2]);
            char *new_argv[4];  

            new_argv[0] = argv[0];
            new_argv[1] = argv[1];

            char head_str[10];  
            sprintf(head_str, "%d", commit_id);
            new_argv[2] = head_str;

            new_argv[3] = NULL;  

            checkout(argc, new_argv);

        }


        return;
    }

    char commit_dir[1024];
    sprintf(commit_dir, "%s/.jax/commits/%d", find_jax_directory(), commit_id);
    DIR* dir = opendir(commit_dir);
    if (dir) {
        // Directory exists
        closedir(dir);
    } else {
        printf("Commit ID %d does not exist\n", commit_id);
        return;
    }

    // Delete everything in the project except the .jax directory
    char path_to_jax[200];
    sprintf(path_to_jax, "%s", find_jax_directory());
    delete_files_except_jax(path_to_jax);

    char path_to_current_commit[250];
    sprintf(path_to_current_commit, "%s/.jax/current_commit", find_jax_directory());
    FILE* file = fopen(path_to_current_commit, "w");
    fprintf(file, "%d", commit_id);

    // Copy the contents of the commit to the project
    char copy_command[2000];
    // sprintf(copy_command, "cp -R %s/* \"%s\"", commit_dir, find_jax_directory());
    sprintf(copy_command, "cd \"%s\" && cp -R * \"%s\"", commit_dir, find_jax_directory());
    system(copy_command);
    sprintf(copy_command, "rm %s/info", find_jax_directory()); // WELL AKSHULLY THIS IS DELETE COMMAND nerd emoji
    system(copy_command);
}


void branch(int argc, char* argv[])
{
    char path_to_branches[200];
    sprintf(path_to_branches, "%s/.jax/branches", find_jax_directory());
    

    if(argc == 2)
    {
        // LIST ALL BRANCHES
        DIR *dir = opendir(path_to_branches);

        if (dir == NULL) {
            printf("Could not open directory\n");
            return;
        }

        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                printf("%s\n", entry->d_name);
        }

        closedir(dir);
    }
    else
    {
        // CREATE A BRANCH
        char name[100];
        strcpy(name, argv[2]);
        char path_to_current_commit[250];
        sprintf(path_to_current_commit, "%s/.jax/current_commit", find_jax_directory());
        int new_branch_head;
        char uselessfucker[100]; // glad to see uselessfucker back. long time no see :)
        FILE* file = fopen(path_to_current_commit, "r");
        if(fscanf(file, "%d", &new_branch_head) == 1)
        {
            // file contains a number
        }
        else
        {
            // the file contents was just HEAD
            char path_to_HEAD[200];
            sprintf(path_to_HEAD, "%s/.jax/HEAD", find_jax_directory());
            FILE* file2 = fopen(path_to_HEAD, "r");
            fscanf(file2, "%s", uselessfucker);
            new_branch_head = find_branch_head(uselessfucker);
            fclose(file2);
        }
        // printf("%d\n", new_branch_head);

        char new_branch[300];
        sprintf(new_branch, "%s/%s", path_to_branches, name);
        FILE* file3 = fopen(new_branch, "w");
        fprintf(file3, "%d", new_branch_head);
        fclose(file3);
        fclose(file);

    }

    
}


int compare_files(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "r");
    if (f1 == NULL) {
        printf("Could not open file %s\n", file1);
        return -1;
    }

    FILE *f2 = fopen(file2, "r");
    if (f2 == NULL) {
        printf("Could not open file %s\n", file2);
        fclose(f1);
        return -1;
    }

    int diff = 0;
    int n = 0, m = 0;

    char ch1 = getc(f1);
    char ch2 = getc(f2);

    while (ch1 != EOF && ch2 != EOF) {
        n++;
        if (ch1 != ch2) {
            diff++;
            break;
        }
        ch1 = getc(f1);
        ch2 = getc(f2);
    }

    fclose(f1);
    fclose(f2);

    return diff;
}



void status() {
    char path_to_commits[1000];
    sprintf(path_to_commits, "%s/.jax/commits/", find_jax_directory());
    int last_commit_id = get_last_commit_id(path_to_commits);

    char last_commit_dir[1000];
    sprintf(last_commit_dir, "%s/.jax/commits/%d", find_jax_directory(), last_commit_id);

    char project_dir[1000];
    strcpy(project_dir, find_jax_directory());

    DIR *dir;
    struct dirent *entry;

    // Check for files in the project directory
    if ((dir = opendir(project_dir)) == NULL) {
        printf("Could not open project directory\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, "info") == 0) {
            continue;  // Skip the 'info' file
        }

        char file_path[2000];
        sprintf(file_path, "%s/%s", project_dir, entry->d_name);

        struct stat st;
        if (stat(file_path, &st) == 0 && S_ISREG(st.st_mode)) {  // check if it's a regular file
            char status[3] = "-";
            // printf("file path : %s\n", file_path);
            read_staged_files();
            if (is_staged(file_path)) {  // check if the file is staged
                status[0] = '+';
                // printf("hello+++\n");
            }

            char commit_file_path[2000];
            sprintf(commit_file_path, "%s/%s", last_commit_dir, entry->d_name);
            if (access(commit_file_path, F_OK) != -1) {  // check if the file exists in the last commit
                if (compare_files(commit_file_path, file_path) == 0) {  // compare the file contents
                    status[1] = 'M';  // the file contents are different
                }
            } else {
                status[1] = 'A';  // the file does not exist in the last commit
            }

            printf("%s : %s\n", entry->d_name, status);
        }
    }

    closedir(dir);

    // Check for files in the last commit
    if ((dir = opendir(last_commit_dir)) == NULL) {
        printf("Could not open last commit directory\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, "info") == 0) {
            continue;  // Skip the 'info' file
        }

        char file_path[2000];
        sprintf(file_path, "%s/%s", last_commit_dir, entry->d_name);

        struct stat st;
        if (stat(file_path, &st) == 0 && S_ISREG(st.st_mode)) {  // check if it's a regular file
            char project_file_path[2000];
            sprintf(project_file_path, "%s/%s", project_dir, entry->d_name);
            if (access(project_file_path, F_OK) == -1) {  // check if the file exists in the project directory
                printf("%s : -D\n", entry->d_name);
            }
        }
    }

    closedir(dir);
}



void remove_file_from_staged(const char *file_path) {
    char staged_file_path[2000];
    sprintf(staged_file_path, "%s/.jax/staged", find_jax_directory());

    FILE *staged_file = fopen(staged_file_path, "r");
    if (staged_file == NULL) {
        printf("Could not open staged file\n");
        return;
    }

    // Read the contents of the staged file into memory
    char **lines = NULL;
    size_t len = 0;
    ssize_t read;
    char *line = NULL;
    size_t linecap = 0;
    while ((read = getline(&line, &linecap, staged_file)) != -1) {
        lines = realloc(lines, (len + 1) * sizeof(*lines));
        if (lines == NULL) {
            printf("Could not allocate memory\n");
            fclose(staged_file);
            return;
        }
        lines[len] = strdup(line);
        lines[len][strcspn(lines[len], "\n")] = 0;
        len++;
    }
    free(line);
    fclose(staged_file);

    // Write the contents back to the staged file, excluding the file to be unstaged
    staged_file = fopen(staged_file_path, "w");
    if (staged_file == NULL) {
        printf("Could not open staged file\n");
        return;
    }
    for (size_t i = 0; i < len; i++) {
        // printf("lines[i] : %s\n", lines[i]);
        if (strcmp(lines[i], file_path) != 0) {
            fprintf(staged_file, "%s\n", lines[i]);
        }
        free(lines[i]);
    }
    free(lines);
    fclose(staged_file);
}


void reset(int argc, char *argv[]) {
    int force = 0;
    int start_index = 2;

    if (argc > 2 && strcmp(argv[2], "-f") == 0) {
        force = 1;
        start_index = 3;
    }

    for (int i = start_index; i < argc; i++) {
        char *file_path;
        if (argv[i][0] == '/') {  // argv[i] is an absolute path
            file_path = argv[i];
        } else {  // argv[i] is a relative path
            char current_dir[2000];
            if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
                printf("Could not get current directory\n");
                return;
            }
            char relative_path[3000];
            sprintf(relative_path, "%s/%s", current_dir, argv[i]);
            file_path = realpath(relative_path, NULL);
            if (file_path == NULL) {
                printf("Could not resolve path %s\n", relative_path);
                continue;
            }
        }

        struct stat st;
        if (stat(file_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                DIR *dir;
                struct dirent *entry;

                if ((dir = opendir(file_path)) == NULL) {
                    printf("Could not open directory %s\n", file_path);
                    continue;
                }

                while ((entry = readdir(dir)) != NULL) {
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;  // Skip the '.' and '..' directories
                    }

                    char sub_file_path[2000];
                    sprintf(sub_file_path, "%s/%s", file_path, entry->d_name);

                    struct stat sub_st;
                    if (stat(sub_file_path, &sub_st) == 0) {
                        if (S_ISREG(sub_st.st_mode)) {
                            remove_file_from_staged(sub_file_path);
                        } else if (S_ISDIR(sub_st.st_mode)) {
                            char *sub_argv[] = {argv[0], argv[1], sub_file_path};
                            reset(3, sub_argv);
                        }
                    }
                }

                closedir(dir);
            } else if (S_ISREG(st.st_mode)) {
                remove_file_from_staged(file_path);
            }
        }

        if (argv[i] != file_path) {
            free(file_path);
        }
    }
}


int is_supported_hook(char *hook_id) {
    for (int i = 0; i < 9; i++) {
        if (strcmp(hook_id, supported_hooks[i]) == 0) {
            return 1;
        }
    }
    return 0;
}



// Function to check if a file is of a certain type
int is_file_type(char *file_path, char *file_type) {
    char *extension = strrchr(file_path, '.');
    return extension && strcmp(extension, file_type) == 0;
}

// Function to check if a file contains a certain phrase
int file_contains_phrase(char *file_path, char *phrase) {
    FILE *file = fopen(file_path, "r");
    char line[MAX_LINE_LENGTH];

    if (file == NULL) {
        printf("Failed to open file.\n");
        return 0;
    }

    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        if (strstr(line, phrase) != NULL) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// Function to check if a file ends with whitespace
int file_ends_with_whitespace(char *file_path) {
    FILE *file = fopen(file_path, "r");
    char ch;

    if (file == NULL) {
        printf("Failed to open file.\n");
        return 0;
    }

    fseek(file, -1, SEEK_END);
    ch = fgetc(file);

    fclose(file);
    return ch == ' ' || ch == '\t' || ch == '\n';
}

// Function to get the size of a file
long get_file_size(char *file_path) {
    FILE *file = fopen(file_path, "r");

    if (file == NULL) {
        printf("Failed to open file.\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    return size;
}

// Function to get the number of characters in a file
long get_num_characters(char *file_path) {
    FILE *file = fopen(file_path, "r");
    char ch;
    long count = 0;

    if (file == NULL) {
        printf("Failed to open file.\n");
        return -1;
    }

    while ((ch = fgetc(file)) != EOF) {
        count++;
    }

    fclose(file);

    return count;
}


// file-size-check function
void file_size_check(char *file_path) {
    long size = get_file_size(file_path);

    if (size <= MAX_FILE_SIZE) {
        printf(ANSI_COLOR_GREEN "PASSED\n" ANSI_COLOR_RESET);
    } else {
        printf(ANSI_COLOR_RED "FAILED\n" ANSI_COLOR_RESET);
    }
}

// character-limit function
void character_limit(char *file_path) {
    if (is_file_type(file_path, ".cpp") || is_file_type(file_path, ".c") || is_file_type(file_path, ".txt")) {
        long num_characters = get_num_characters(file_path);

        if (num_characters <= MAX_CHARACTERS) {
            printf(ANSI_COLOR_GREEN "PASSED\n" ANSI_COLOR_RESET);
        } else {
            printf(ANSI_COLOR_RED "FAILED\n" ANSI_COLOR_RESET);
        }
    } else {
        printf(ANSI_COLOR_YELLOW "SKIPPED\n" ANSI_COLOR_RESET);
    }
}



// todo-check function
void todo_check(char *file_path) {
    if (is_file_type(file_path, ".cpp") || is_file_type(file_path, ".c") || is_file_type(file_path, ".txt")) {
        if (file_contains_phrase(file_path, "TODO")) {
            printf(ANSI_COLOR_RED "FAILED\n" ANSI_COLOR_RESET);
        } else {
            printf(ANSI_COLOR_GREEN "PASSED\n" ANSI_COLOR_RESET);
        }
    } else {
        printf(ANSI_COLOR_YELLOW "SKIPPED\n" ANSI_COLOR_RESET);
    }
}

// eof-blank-space function
void eof_blank_space(char *file_path) {
    if (is_file_type(file_path, ".cpp") || is_file_type(file_path, ".c") || is_file_type(file_path, ".txt")) {
        if (file_ends_with_whitespace(file_path)) {
            printf(ANSI_COLOR_RED "FAILED\n" ANSI_COLOR_RESET);
        } else {
            printf(ANSI_COLOR_GREEN "PASSED\n" ANSI_COLOR_RESET);
        }
    } else {
        printf(ANSI_COLOR_YELLOW "SKIPPED\n" ANSI_COLOR_RESET);
    }
}


// format-check function
void format_check(char *file_path) {
    if (is_file_type(file_path, ".cpp") || is_file_type(file_path, ".c") || is_file_type(file_path, ".txt") || is_file_type(file_path, ".mp4") || is_file_type(file_path, ".mp3") || is_file_type(file_path, ".wav") || is_file_type(file_path, ".py")) {
        printf(ANSI_COLOR_GREEN "PASSED\n" ANSI_COLOR_RESET);
    } else {
        printf(ANSI_COLOR_RED "FAILED\n" ANSI_COLOR_RESET);
    }
}

// balance-braces function
void balance_braces(char *file_path) {
    if (is_file_type(file_path, ".cpp") || is_file_type(file_path, ".c") || is_file_type(file_path, ".txt")) {
        FILE *file = fopen(file_path, "r");
        char ch;
        int round_braces = 0, square_braces = 0, curly_braces = 0;

        if (file == NULL) {
            printf("Failed to open file.\n");
            return;
        }

        while ((ch = fgetc(file)) != EOF) {
            switch (ch) {
                case '(':
                    round_braces++;
                    break;
                case ')':
                    round_braces--;
                    break;
                case '[':
                    square_braces++;
                    break;
                case ']':
                    square_braces--;
                    break;
                case '{':
                    curly_braces++;
                    break;
                case '}':
                    curly_braces--;
                    break;
            }

            if (round_braces < 0 || square_braces < 0 || curly_braces < 0) {
                printf(ANSI_COLOR_RED "FAILED\n" ANSI_COLOR_RESET);
                fclose(file);
                return;
            }
        }

        if (round_braces == 0 && square_braces == 0 && curly_braces == 0) {
            printf(ANSI_COLOR_GREEN "PASSED\n" ANSI_COLOR_RESET);
        } else {
            printf(ANSI_COLOR_RED "FAILED\n" ANSI_COLOR_RESET);
        }

        fclose(file);
    } else {
        printf(ANSI_COLOR_YELLOW "SKIPPED\n" ANSI_COLOR_RESET);
    }
}

// Function to check code indentation
void check_indentation(char *file_path) {
    if (is_file_type(file_path, ".cpp") || is_file_type(file_path, ".c")) {
        char command[1024];
        sprintf(command, "indent -st \"%s\" > /dev/null 2>&1", file_path);

        //int result = system(command);

        // if (result == 0) {
        //     printf(ANSI_COLOR_GREEN "PASSED\n" ANSI_COLOR_RESET);
        // } else {
            printf(ANSI_COLOR_RED "FAILED\n" ANSI_COLOR_RESET);
        // }
    } else {
        printf(ANSI_COLOR_YELLOW "SKIPPED\n" ANSI_COLOR_RESET);
    }
}

// Function to check for static errors
void static_error_check(char *file_path) {
    if (is_file_type(file_path, ".cpp") || is_file_type(file_path, ".c")) {
        char command[1024];
        sprintf(command, "gcc -o /dev/null -fsyntax-only \"%s\" > /dev/null 2>&1", file_path);

        int result = system(command);

        if (result == 0) {
            printf(ANSI_COLOR_GREEN "PASSED\n" ANSI_COLOR_RESET);
        } else {
            printf(ANSI_COLOR_RED "FAILED\n" ANSI_COLOR_RESET);
        }
    } else {
        printf(ANSI_COLOR_YELLOW "SKIPPED\n" ANSI_COLOR_RESET);
    }
}



// Function to apply a hook to a file
void apply_hook(char *hook_id, char *file_path) {
    // Call the appropriate function based on the hook ID
    if (strcmp(hook_id, "todo-check") == 0) {
        todo_check(file_path);
    } else if (strcmp(hook_id, "eof-blank-space") == 0) {
        eof_blank_space(file_path);
    } else if (strcmp(hook_id, "format-check") == 0) {
        format_check(file_path);
    } else if (strcmp(hook_id, "balance-braces") == 0) {
        balance_braces(file_path);
    } else if (strcmp(hook_id, "file-size-check") == 0) {
        file_size_check(file_path);
    } else if (strcmp(hook_id, "character-limit") == 0) {
        character_limit(file_path);
    } else if (strcmp(hook_id, "static-error-check") == 0) {
        static_error_check(file_path);
    } else if (strcmp(hook_id, "indentation-check") == 0) {
        check_indentation(file_path);
    }
}

// Function to apply all active hooks to a file
void apply_hooks(char *file_path) {
    char path_to_hooks[1024];
    sprintf(path_to_hooks, "%s/.jax/hooks", find_jax_directory());

    FILE *file = fopen(path_to_hooks, "r");
    char hook_id[50];

    if (file == NULL) {
        printf("Failed to open hooks file.\n");
        return;
    }

    printf("File name: \"%s\"\n", get_relative_path(file_path, find_jax_directory()));
    while (fgets(hook_id, 50, file) != NULL) {
        // Remove the newline character at the end of the hook ID
        hook_id[strcspn(hook_id, "\n")] = 0;

        printf("\"%s\".......................................................", hook_id);
        apply_hook(hook_id, file_path);
    }

    fclose(file);
}


// jax pre-commit command
void jax_pre_commit() {
    char path_to_staged[1024];
    sprintf(path_to_staged, "%s/.jax/staged", find_jax_directory());

    FILE *file = fopen(path_to_staged, "r");
    char file_path[1024];

    if (file == NULL) {
        printf("Failed to open staged file.\n");
        return;
    }

    while (fgets(file_path, 1024, file) != NULL) {
        // Remove the newline character at the end of the file path
        file_path[strcspn(file_path, "\n")] = 0;


        apply_hooks(file_path);
    }

    fclose(file);
}



void precommit(int argc, char *argv[]) {
    char path_to_hooks[1024];
    sprintf(path_to_hooks, "%s/.jax/hooks", find_jax_directory());

    if (argc == 2) {
        jax_pre_commit();
        return;
    }

    if (strcmp(argv[2], "-f") == 0) {
        for (int i = 3; i < argc; i++) {
            char *file_path = realpath(argv[i], NULL);
            if (file_path == NULL) {
                printf("File \"%s\" does not exist.\n", argv[i]);
                continue;
            }

            read_staged_files();
            if (is_staged(file_path) == 1) {
                apply_hooks(file_path);
            } else {
                printf("File \"%s\" is not staged.\n", argv[i]);
            }
            free(file_path);
        }
        return;
    }

    char *action = argv[2];

    if (strcmp(action, "add") == 0 || strcmp(action, "remove") == 0) {
        if (argc < 5) {
            printf("Invalid command. Usage: jax pre-commit <add/remove> hook <hook-id>\n");
            return;
        }

        char *hook_id = argv[4];

        if (!is_supported_hook(hook_id)) {
            printf("Invalid hook ID. Please use one of the supported hook IDs.\n");
            return;
        }

        if (strcmp(action, "add") == 0) {
            FILE *file = fopen(path_to_hooks, "a");
            if (file == NULL) {
                printf("Failed to open hooks file.\n");
                return;
            }
            fprintf(file, "%s\n", hook_id);
            fclose(file);
        } else if (strcmp(action, "remove") == 0) {
            char temp[30];
            FILE *file = fopen(path_to_hooks, "r");
            FILE *tempfile = fopen("temp", "w");

            if (file == NULL || tempfile == NULL) {
                printf("Failed to open hooks file.\n");
                return;
            }

            while (fgets(temp, 30, file) != NULL) {
                if (strstr(temp, hook_id) == NULL)
                    fprintf(tempfile, "%s", temp);
            }

            fclose(file);
            fclose(tempfile);

            remove(path_to_hooks);
            rename("temp", path_to_hooks);
        }
    } else if (strcmp(argv[3], "list") == 0) {
        printf("Supported hooks:\n");
        for (int i = 0; i < 9; i++) {
            printf("%s\n", supported_hooks[i]);
        }
    } else if (strcmp(action, "applied") == 0) {
        char temp[30];
        FILE *file = fopen(path_to_hooks, "r");

        if (file == NULL) {
            printf("Failed to open hooks file.\n");
            return;
        }

        printf("Applied hooks:\n");
        while (fgets(temp, 30, file) != NULL) {
            printf("%s", temp);
        }

        fclose(file);
    } else {
        printf("Invalid action. Use 'add', 'remove', 'list', or 'applied'.\n");
    }
}

void grep(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Invalid command. Usage: jax grep -f <file> -p <word> [-c <commit-id>] [-n]\n");
        return;
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Error getting current working directory");
        return;
    }
    // printf("cwd : %s\n", cwd);

    char *file_path = NULL;
    char *word = NULL;
    char *commit_id = NULL;
    int print_line_numbers = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            file_path = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0) {
            word = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0) {
            commit_id = argv[++i];
        } else if (strcmp(argv[i], "-n") == 0) {
            print_line_numbers = 1;
        }
    }
    printf("word : %s\n", word);
    if (file_path == NULL || word == NULL) {
        printf("Invalid command. Usage: jax grep -f <file> -p <word> [-c <commit-id>] [-n]\n");
        return;
    }

    // char cwd[1024];
    // if (getcwd(cwd, sizeof(cwd)) == NULL) {
    //     perror("Error getting current working directory");
    //     return;
    // }
    // getcwd(cwd, sizeof(cwd));
    // printf("cwd b4jax : %s\n", cwd);
    char *jax_directory = find_jax_directory(); 
    // getcwd(cwd, sizeof(cwd));
    // printf("cwd after jax : %s\n", cwd);
    //printf("jax : %s\ncwd : %s\n", jax_directory, cwd);
    char *relative_path = cwd + strlen(jax_directory);
    if (*relative_path == '/') relative_path++; // Skip the leading '/'
    // printf("rel : %s\n", relative_path);


    if (commit_id != NULL) {
        char commit_path[1024];
        sprintf(commit_path, "%s/.jax/commits/%s/%s/%s", find_jax_directory(), commit_id, relative_path, file_path);
        file_path = commit_path;
    }
    else
    {

        char temp[1024];
        strcpy(temp, file_path);
        char *new_file_path = malloc(strlen(cwd) + strlen(temp) + 2); // +2 for the '/' and the null terminator
        if (new_file_path == NULL) {
        // handle error
        }
        sprintf(new_file_path, "%s/%s", cwd, temp);
        file_path = new_file_path;


    }
    // getcwd(cwd, sizeof(cwd));
    // printf("cwd2 : %s\n", cwd);
    // printf("path : %s\n", file_path);
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("File \"%s\" does not exist.\n", file_path);
        return;
    }
    printf("word : %s\n", word);

    char line[1024];
    int line_number = 1;
    while (fgets(line, 1024, file) != NULL) {
        // printf("%s\n", line);
        char *start = line;
        while ((start = strstr(start, word)) != NULL) {
            if (print_line_numbers) {
                printf(ANSI_COLOR_BLUE "%d:" ANSI_COLOR_RESET, line_number);
            }
            char *end = start + strlen(word);
            *start = '\0';
            printf("%s" ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "%s", line, word, end);
            start = end;
        }
        line_number++;
    }

    fclose(file);
}

void show_tag(char *tag_name) {
    char *jax_directory = find_jax_directory();
    char tag_path[1024];
    sprintf(tag_path, "%s/.jax/tags/%s", jax_directory, tag_name);

    FILE *file = fopen(tag_path, "r");
    if (file == NULL) {
        printf("Tag \"%s\" does not exist.\n", tag_name);
        return;
    }

    char line[1024];
    while (fgets(line, 1024, file) != NULL) {
        printf("%s", line);
    }

    fclose(file);
}


void tag(int argc, char *argv[]) {

    if (strcmp(argv[2], "show") == 0) {
        if (argc < 4) {
            printf("Invalid command. Usage: jax tag show <tag-name>\n");
            return;
        }
        show_tag(argv[3]);
        return;
    }

    if (strcmp(argv[2], "-a") != 0) {
        // List all tags
        char *jax_directory = find_jax_directory();
        char tags_directory[1024];
        sprintf(tags_directory, "%s/.jax/tags", jax_directory);

        DIR *dir = opendir(tags_directory);
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                printf("%s\n", entry->d_name);
            }
        }
        closedir(dir);
        return;
    }



    char *tag_name = NULL;
    char *message = NULL;
    char *commit_id = NULL;
    int force = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            tag_name = argv[++i];
        } else if (strcmp(argv[i], "-m") == 0) {
            message = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0) {
            commit_id = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0) {
            force = 1;
        }
    }

    if (tag_name == NULL) {
        printf("Invalid command. Usage: jax tag -a <tag-name> [-m <message>] [-c <commit-id>] [-f]\n");
        return;
    }

    char *jax_directory = find_jax_directory();
    char tag_path[1024];
    sprintf(tag_path, "%s/.jax/tags/%s", jax_directory, tag_name);


    if(commit_id == NULL)
    {
        char path_to_current_commit[1024];
        sprintf(path_to_current_commit, "%s/.jax/current_commit", jax_directory);
        // printf("path : %s\n", path_to_current_commit);
        FILE* file = fopen(path_to_current_commit, "r");
        if(file == NULL)
        {
            printf("error reading file\n");
            return;
        }
        commit_id = malloc(41); 
        if (commit_id == NULL) {
            printf("fuckyou\n");
        }
        fscanf(file, "%s", commit_id);
        fclose(file);
    }



    if (!force && access(tag_path, F_OK) != -1) {
        printf("Tag \"%s\" already exists. Use -f to overwrite it.\n", tag_name);
        return;
    }

    FILE *file = fopen(tag_path, "w");
    if (file == NULL) {
        printf("Could not create tag \"%s\".\n", tag_name);
        return;
    }

    // Get the author
    char path_to_login_local[2000];
    sprintf(path_to_login_local, "%s/.jax/login", jax_directory);
    FILE* file1 = fopen(path_to_login_local, "r");
    FILE* file2 = fopen("/home/matin/jax_global/login", "r");
    char l_username[100], g_username[100], l_email[100], g_email[100];
    long long l_time, g_time;
    fscanf(file1, "%s\n%s\n%lld", l_username, l_email, &l_time);
    fscanf(file2, "%s\n%s\n%lld",g_username, g_email, &g_time);
    fclose(file1);
    fclose(file2);

    char *username, *email;
    if (l_time > g_time) {
        username = l_username;
        email = l_email;
    } else {
        username = g_username;
        email = g_email;
    }

    // Get the current time
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // Write the tag data to the file
    fprintf(file, "Tag: %s\n", tag_name);
    fprintf(file, "Author: %s <%s>\n", username, email);
    fprintf(file, "Date: %04d-%02d-%02d %02d:%02d:%02d\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    if (commit_id != NULL) {
        fprintf(file, "Commit: %s\n", commit_id);
    }
    if (message != NULL) {
        fprintf(file, "Message: %s\n", message);
    }

    fclose(file);

    printf("Created tag \"%s\".\n", tag_name);
}

void set_shortcut(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Invalid command. Usage: jax set -m \"message\" -s shortcut-name\n");
        return;
    }

    char *message = NULL;
    char *shortcut_name = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0) {
            message = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0) {
            shortcut_name = argv[++i];
        }
    }
    if (message == NULL || shortcut_name == NULL) {
        printf("Invalid command. Usage: jax set -m \"message\" -s shortcut-name\n");
        return;
    }

    char *jax_directory = find_jax_directory();
    char shortcut_path[1024];
    sprintf(shortcut_path, "%s/.jax/message_shortcuts/%s", jax_directory, shortcut_name);

    if (access(shortcut_path, F_OK) != -1) {
        printf("Shortcut \"%s\" already exists. Choose a different name.\n", shortcut_name);
        return;
    }

    // printf("%s\n", shortcut_path);
    FILE *file = fopen(shortcut_path, "w");
    if (file == NULL) {
        printf("Could not create shortcut \"%s\".\n", shortcut_name);
        return;
    }

    fprintf(file, "%s\n", message);

    fclose(file);

    printf("Created shortcut \"%s\".\n", shortcut_name);
}

void replace_shortcut(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Invalid command. Usage: jax replace -m \"new message\" -s shortcut-name\n");
        return;
    }

    char *message = NULL;
    char *shortcut_name = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0) {
            message = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0) {
            shortcut_name = argv[++i];
        }
    }

    if (message == NULL || shortcut_name == NULL) {
        printf("Invalid command. Usage: jax replace -m \"new message\" -s shortcut-name\n");
        return;
    }

    char *jax_directory = find_jax_directory();
    char shortcut_path[1024];
    sprintf(shortcut_path, "%s/.jax/message_shortcuts/%s", jax_directory, shortcut_name);

    if (access(shortcut_path, F_OK) == -1) {
        printf("Shortcut \"%s\" does not exist.\n", shortcut_name);
        return;
    }

    FILE *file = fopen(shortcut_path, "w");
    if (file == NULL) {
        printf("Could not open shortcut \"%s\".\n", shortcut_name);
        return;
    }

    fprintf(file, "%s\n", message);

    fclose(file);

    printf("Replaced shortcut \"%s\".\n", shortcut_name);
}

void remove_shortcut(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Invalid command. Usage: jax remove -s shortcut-name\n");
        return;
    }

    char *shortcut_name = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            shortcut_name = argv[++i];
        }
    }

    if (shortcut_name == NULL) {
        printf("Invalid command. Usage: jax remove -s shortcut-name\n");
        return;
    }

    char *jax_directory = find_jax_directory();
    char shortcut_path[1024];
    sprintf(shortcut_path, "%s/.jax/message_shortcuts/%s", jax_directory, shortcut_name);

    if (access(shortcut_path, F_OK) == -1) {
        printf("Shortcut \"%s\" does not exist.\n", shortcut_name);
        return;
    }

    if (remove(shortcut_path) == 0) {
        printf("Removed shortcut \"%s\".\n", shortcut_name);
    } else {
        printf("Could not remove shortcut \"%s\".\n", shortcut_name);
    }
}




int main(int argc, char* argv[])
{
    ensure_directory_exists("/home/matin/jax_global");
    // int x = something(3);
    // printf("%d\n", x);
    // printf("%d\n", is_staged("/mnt/c/Users/matin/Desktop/Taklif Mabani/serie 8/shrek/ramze shrek.exe"));
    // printf("%d\n%d\n", is_modified("/mnt/c/Users/matin/Desktop/Taklif Mabani/serie 8/shrek/ramze shrek.c", 1706210054), is_modified("/mnt/c/Users/matin/Desktop/Taklif Mabani/serie 8/shrek/ramze shrek.exe", 1706209450));
    // printf("%d", increment_commit_id());

    // printf("address : %s\n", find_jax_directory());


    if(argc < 2)
    {
        perror("Invalid command\n");
    }

    if(strcmp(argv[1], "init") == 0)
    {
        init(argc, argv);
    }
    // IF I UNCOMMENT THESE LINES THE CODE COMPLETELY BREAKS WHAT THE FUCK????
    else if(find_jax_directory() == NULL)
    {
        printf("fatal : jax repository is not initialized\n");
    }
    else if(strcmp(argv[1], "add") == 0)
    {
        add(argc, argv);
    }
    else if(strcmp(argv[1], "commit") == 0)
    {
        commit(argc, argv);
    }
    else if(strcmp(argv[1], "evasion") == 0)
    {
        printf("ACTIVE: Jax enters Evasion, a defensive stance, for up to 2 seconds, causing all basic attacks against him to miss. Jax also takes 25%c reduced damage from all area of effect abilities. After 1 second, Jax can reactivate to end it immediately\n", '%');
        // :)
    }
    else if(strcmp(argv[1], "config") == 0)
    {
        config(argc, argv);
    }
    else if(strcmp(argv[1], "log") == 0)
    {
        logg(argc, argv);
    }
    else if(strcmp(argv[1], "checkout") == 0)
    {
        checkout(argc, argv);
    }
    else if(strcmp(argv[1], "branch") == 0)
    {
        branch(argc, argv);
    }
    else if(strcmp(argv[1], "status") == 0)
    {
        status();
    }
    else if(strcmp(argv[1], "reset") == 0)
    {
        reset(argc, argv);
    }
    else if(strcmp(argv[1], "pre-commit") == 0)
    {
        precommit(argc, argv);
    }
    else if(strcmp(argv[1], "grep") == 0)
    {
        grep(argc, argv);
    }
    else if(strcmp(argv[1], "tag") == 0)
    {
        tag(argc, argv);
    }
    else if(strcmp(argv[1], "set") == 0)
    {
        set_shortcut(argc, argv);
    }
    else if(strcmp(argv[1], "replace") == 0)
    {
        replace_shortcut(argc, argv);
    }
    else if(strcmp(argv[1], "remove") == 0)
    {
        remove_shortcut(argc, argv);
    }

    else
        perror("Invalid command\n");

}