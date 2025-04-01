#ifndef PSTREE_H
#define PSTREE_H

#include<stdbool.h>

typedef struct ProcessNode{
    int pid;
    int ppid;//父进程ID
    char name[256];
    struct ProcessNode** children;  //子进程指针
    int child_count;    //子进程数量
    int child_capacity;     //子进程容量
} ProcessNode;

typedef struct ProcessTree{
    ProcessNode* root;
    ProcessNode ** all_processes; //所有进程的索引
    int process_count;  //进程总数
    int capacity;
} ProcessTree;
/* 如何去设计，先设计数据结构，再根据数据结构来设计函数从而达到构建一个程序的目的*/
void print_help(); //打印帮助信息

void print_version();   //打印版本信息

ProcessTree* collect_process_info(); //收集进程信息
bool read_process_info(ProcessTree* tree); //读取进程信息
void free_process_tree(ProcessTree* tree); //释放进程树

void build_process_tree(ProcessTree* tree); //构建进程树
void add_child_to_parent(ProcessNode* parent, ProcessNode* child); //将子进程添加到父进程

void sort_process_tree(ProcessTree* tree, bool numeric_sort); //对进程树进行排序
void sort_children(ProcessNode* node, bool numeric_sort); // 对子节点进行排序

void print_process_tree(ProcessTree* tree, bool show_pids); //打印进程树
void print_process_node(ProcessNode* node, int depth, bool show_pids, bool* is_last_child, int level); //打印进程节点

// 辅助函数
char* get_process_name(int pid);
int get_process_ppid(int pid);
int* get_children_pids(int pid, int* count);

void print_branch(bool* is_last_child, int depth);

#endif /* PSTREE_H */