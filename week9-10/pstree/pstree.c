#include<stdio.h>
#include<getopt.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include "pstree.h"
#include<sys/stat.h>
#include<dirent.h>
//定义长选项
struct option long_options[]={
    {"show-pids",no_argument,0,'p'},
    {"numeric-sort",no_argument,0,'n'},
    {"version",no_argument,0,'V'},
    {"help",no_argument,0,'h'},
    {0,0,0,0}
};
bool is_num_dir(const char *name){
    //检查目录名是否全为数字
    for(int i=0; name[i] != '\0'; i++){
        if(name[i] < '0'|| name[i] >'9'){
            return false;        
        }         
    }
    return true;
    
}
void print_help(){
    printf("Usage: pstree [OPTION]\n");
    printf("Show process tree\n");
    printf("\n");
    printf("Options:\n");
    printf("  -p, --show-pids     Show process IDs\n");
    printf("  -n, --numeric-sort  Sort by process ID\n");
    printf("  -V, --version       Display version information\n");
    printf("  -h, --help          Display this help\n");
}

void print_version(){
    printf("pstree 1.0\n");
    //这里疑似还会出现 github 上已经提交了的
}
int main(int argc, char * const argv[]) {
    bool show_pids = false;  //-p 选项
    bool numeric_sort = false; //-n 选项
    int version_flag = 0;

    int opt;
    /*
    getopt_long 是 C 语言中用于解析命令行参数的一个函数，支持短选项（如 -h）和长选项（如 --help）。
    它是 getopt 的扩展版本，定义在 <getopt.h> 头文件中。
    int getopt_long(int argc, char *const argv[],
                const char *optstring,
                const struct option *longopts,
                int *longindex);*/
    while((opt = getopt_long(argc,argv,"pnVh",long_options,NULL))!=-1){
        switch(opt){
            case 'p':
                show_pids = true;
                break;
            case 'n':
                numeric_sort = true;
                break;
            case 'V':
                version_flag = 1;
                break;
            case 'h':
                print_help();
                return 0;
            case '?':
                printf("Invalid option\n");
                return 1;
            default:
                printf("Invalid option\n");
                return 1;
        }
    }

    if(version_flag){
        print_version();
        return 0;
    }

    if (optind < argc) {
        printf("Invalid option\n");
        return 1;
    }
    ProcessTree* tree = collect_process_info();//初始化这棵树
    if (!tree) {
        printf("Failed to collect process information\n");
        return 1;
    }
    
    // 执行排序(如果需要)
    if (numeric_sort) {
        sort_process_tree(tree, true);
    }
    
    // 打印进程树
    print_process_tree(tree, show_pids);
    
    // 释放内存
    free_process_tree(tree);
    
    return 0;
}

ProcessTree * collect_process_info(){
    ProcessTree * tree = (ProcessTree *)malloc(sizeof(ProcessTree));
    if(!tree){
        return NULL;
    }
    tree->root = NULL;
    tree->process_count = 0;
    tree->capacity = 5;
    tree->all_processes = (ProcessNode **)malloc(sizeof(ProcessNode*) * tree->capacity);
    if(!tree->all_processes){
        free(tree);
        return NULL;
    }//初始化完成
    if(!read_process_info(tree)){
        free_process_tree(tree);
        return NULL;
    } //这里是将read build 和 collect 分开构建封装
    build_process_tree(tree);
    return tree;
}

bool read_process_info(ProcessTree * tree){
    DIR* proc = opendir("/proc");
    if(proc == NULL){
        perror("目录打开失败");
        return false;  // 应该返回false而不是1
    }

    struct dirent *entry;
    while ((entry = readdir(proc)) != NULL){//这里是每读取一次
        // 检查数字目录
        if(is_num_dir(entry->d_name)){
            int pid = atoi(entry->d_name);
            
            // 修正内存分配
            ProcessNode *node = (ProcessNode *)malloc(sizeof(ProcessNode));//开始分配
            if(!node){
                closedir(proc);
                return false;
            }

            node->pid = pid;
            node->child_count = 0;
            node->child_capacity = 5;
            node->children = (ProcessNode **)malloc(sizeof(ProcessNode*) * node->child_capacity);
            if(!node->children){
                free(node);
                closedir(proc);
                return false;
            }
            
            // 获取进程名称
            char* name = get_process_name(pid);//这里调用的get process 的运作机理
            if (name) {
                strncpy(node->name, name, sizeof(node->name) - 1);
                node->name[sizeof(node->name) - 1] = '\0';
            } else {
                strcpy(node->name, "unknown");
            }
            
            // 获取父进程ID - 需要实现get_process_ppid函数
            node->ppid = get_process_ppid(pid);
            
            // 检查容量并扩容
            if (tree->process_count >= tree->capacity) {
                tree->capacity *= 2;
                ProcessNode** new_array = (ProcessNode**)realloc(tree->all_processes, 
                                              sizeof(ProcessNode*) * tree->capacity);
                if (!new_array) {
                    free(node->children);
                    free(node);
                    closedir(proc);
                    return false;
                }
                tree->all_processes = new_array;
            }
            
            // 添加节点到数组
            tree->all_processes[tree->process_count++] = node;
        }
    }
    
    int is_shutdown = closedir(proc);
    if(is_shutdown){
        perror("目录关闭失败");
        return false;  // 应该返回false而不是1
    }
    
    return true;
}

void build_process_tree(ProcessTree * tree){
    // 首先找出根进程（PID为1或没有父进程的进程）
    for(int i = 0; i < tree->process_count; i++){
        ProcessNode *node = tree->all_processes[i];
        if(node->pid == 1 || node->ppid == 0){
            tree->root = node;
            break;
        }
    }
    
    // 构建父子关系
    for(int i = 0; i < tree->process_count; i++){
        ProcessNode *current = tree->all_processes[i];
        for(int j = 0; j < tree->process_count; j++){
            if(i != j && tree->all_processes[j]->ppid == current->pid){
                add_child_to_parent(current, tree->all_processes[j]);
            }
        }
    }
}

char * get_process_name(int pid){
    char path[256];
    sprintf(path, "/proc/%d/stat", pid);//每次调用后其实都是一个新的char数组
    FILE *file = fopen(path,"r");//这里是选取path路径上打开，
    if(file == NULL){
        return NULL;
    }
    
    static char name[256];
    int dummy_pid;
    fscanf(file, "%d (%[^)])", &dummy_pid, name);  // 注意右括号的处理
    fclose(file);
    return name;
}

int get_process_ppid(int pid) {
    char path[256];
    sprintf(path, "/proc/%d/stat", pid);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return -1;
    }
    
    int dummy_pid, ppid;
    char comm[256];
    char state;
    
    // stat文件格式: pid (comm) state ppid ...
    fscanf(file, "%d (%[^)]) %c %d", &dummy_pid, comm, &state, &ppid);
    
    fclose(file);
    return ppid;
}


void free_process_tree(ProcessTree * tree){
    if (!tree) return;
    
    if (tree->all_processes) {
        for(int i = 0; i < tree->process_count; i++){
            if (tree->all_processes[i]) {
                free(tree->all_processes[i]->children);
                free(tree->all_processes[i]);
            }
        }
        free(tree->all_processes);
    }
    
    free(tree);
}

void print_process_tree(ProcessTree * tree, bool show_pids) {
    if (!tree || !tree->root) {
        printf("No process information available.\n");
        return;
    }
    
    // 创建一个足够大的数组跟踪每一层是否是最后一个子节点
    bool* is_last_child = (bool*)calloc(1000, sizeof(bool)); // 假设最大深度不超过1000
    if (!is_last_child) {
        printf("Memory allocation failed.\n");
        return;
    }
    
    print_process_node(tree->root, 0, show_pids, is_last_child, 0);
    
    free(is_last_child);
}

// 打印分支线条的辅助函数
void print_branch(bool* is_last_child, int depth) {
    for (int i = 0; i < depth; i++) {
        if (is_last_child[i]) {
            printf("    "); // 该位置已经是最后一个子节点，留空白
        } else {
            printf("│   "); // 该位置不是最后一个子节点，打印垂直线
        }
    }
}

void print_process_node(ProcessNode * node, int depth, bool show_pids, bool* is_last_child, int level) {
    // 对于根节点特殊处理
    if (depth == 0) {
        if (show_pids) {
            printf("%s(%d)\n", node->name, node->pid);
        } else {
            printf("%s\n", node->name);
        }
    } else {
        // 打印分支线条
        print_branch(is_last_child, depth - 1);
        
        // 打印当前节点的分支符号
        if (is_last_child[depth - 1]) {
            printf("└── "); // 最后一个子节点，使用角符号
        } else {
            printf("├── "); // 中间的子节点，使用T型符号
        }
        
        // 打印进程信息
        if (show_pids) {
            printf("%s(%d)\n", node->name, node->pid);
        } else {
            printf("%s\n", node->name);
        }
    }
    
    // 递归打印子进程
    for (int i = 0; i < node->child_count; i++) {
        // 设置当前层级的最后一个子节点标记
        is_last_child[depth] = (i == node->child_count - 1);
        
        print_process_node(node->children[i], depth + 1, show_pids, is_last_child, i);
    }
}

void sort_process_tree(ProcessTree * tree, bool numeric_sort) {
    if (!tree) return;
    
    // 对每个节点的子进程进行排序
    for (int i = 0; i < tree->process_count; i++) {
        sort_children(tree->all_processes[i], numeric_sort);
    }
}

void sort_children(ProcessNode * node, bool numeric_sort) {
    if (!node || node->child_count <= 1) return;
    
    // 简单的冒泡排序
    for (int i = 0; i < node->child_count - 1; i++) {
        for (int j = 0; j < node->child_count - i - 1; j++) {
            bool should_swap = false;
            
            if (numeric_sort) {
                // 按PID排序
                should_swap = (node->children[j]->pid > node->children[j+1]->pid);
            } else {
                // 按名称排序
                should_swap = (strcmp(node->children[j]->name, 
                              node->children[j+1]->name) > 0);
            }
            
            if (should_swap) {
                ProcessNode *temp = node->children[j];
                node->children[j] = node->children[j+1];
                node->children[j+1] = temp;
            }
        }
    }
}

void add_child_to_parent(ProcessNode* parent, ProcessNode* child) {
    // 检查是否需要扩展容量
    if(parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        ProcessNode** new_children = (ProcessNode**)realloc(parent->children, 
                                        sizeof(ProcessNode*) * parent->child_capacity);
        if(!new_children) {
            return; // 处理内存分配失败
        }
        parent->children = new_children;
    }
    
    // 添加子进程
    parent->children[parent->child_count++] = child;
}

int* get_children_pids(int pid, int* count) {
    // 设置初始值
    *count = 0;
    int capacity = 10;
    int* children = (int*)malloc(sizeof(int) * capacity);
    if(!children) {
        return NULL;
    }
    
    DIR* proc = opendir("/proc");
    if(!proc) {
        free(children);
        return NULL;
    }
    
    struct dirent* entry;
    while((entry = readdir(proc)) != NULL) {
        if(is_num_dir(entry->d_name)) {
            int child_pid = atoi(entry->d_name);
            int parent_pid = get_process_ppid(child_pid);
            
            if(parent_pid == pid) {
                // 扩容检查
                if(*count >= capacity) {
                    capacity *= 2;
                    int* new_array = (int*)realloc(children, sizeof(int) * capacity);
                    if(!new_array) {
                        free(children);
                        closedir(proc);
                        return NULL;
                    }
                    children = new_array;
                }
                
                children[(*count)++] = child_pid;
            }
        }
    }
    
    closedir(proc);
    return children;
}