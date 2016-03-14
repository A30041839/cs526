#ifndef _LOG_H
#define _LOG_H

#include <string>
#include <cstdio>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "graph.hpp"
#include "types.hpp"
#include "utility.hpp"

//24 bytes
struct log_entry_t {
    uint32_t opcode;
    uint64_t node1;
    uint64_t node2;
    
    log_entry_t() {
        opcode = 0;
        node1 = 0;
        node2 = 0;
    }
    
    log_entry_t(uint32_t _opcode, uint64_t _node1, uint64_t _node2) {
        opcode = _opcode;
        node1 = _node1;
        node2 = _node2;
    }
};

//4096 bytes
struct super_block_t {
    uint64_t checksum;
    uint32_t generation_num;
    uint32_t log_start;
    uint32_t log_size;
    uint32_t checkpoint_size;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
    
    log_entry_t reserved[169];
    
    super_block_t() {
        clear_block((void*)this);
    }
    
    uint64_t compute_checksum() {
        return compute_checksum_xor((void*)this);
    }
    
    void clear() {
        clear_block((void*)this);
    }
};

//4096 bytes
struct log_block_t {
    uint64_t checksum;
    uint32_t generation_num;
    uint32_t entry_cnt;
    
    log_entry_t log_entry[170];
    
    log_block_t() {
        clear_block((void*)this);
    }
    
    uint64_t compute_checksum() {
        return compute_checksum_xor((void*)this);
    }
    
    void clear() {
        clear_block((void*)this);
    }
};

//16 bytes
struct edge_t {
    uint64_t node1;
    uint64_t node2;
};

//4096 bytes
struct checkpt_block_t {
    uint32_t entry_cnt;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
    
    edge_t edges[255];
    
    checkpt_block_t() {
        clear_block((void*)this);
    }
    
    void clear() {
        clear_block((void*)this);
    }
    
};

class server_log {
private:
    const char* log;
    
    int fd = -1;
    super_block_t super_block;
    log_block_t cur_block;
    checkpt_block_t checkpt_block;
    
    uint32_t block_offset;
    
    struct Graph* graph = nullptr;
    
public:
    
    void bind_graph(struct Graph* g);
    
    void attach_log(const char* devfile);
    
    void init_server_log();
    
    void init_superblock();
    
    void format();
    
    void read_in_superblock(super_block_t* sb);
    
    void read_in_log_block(log_block_t* lb, uint32_t offset);
    
    void read_in_checkpt_block(checkpt_block_t* cb, uint32_t offset);
    
    void write_super_block(super_block_t* sb);
    
    void write_log_block(log_block_t* lb, uint32_t offset);
    
    void write_checkpt_block(checkpt_block_t* cb, uint32_t offset);
    
    void add_log_entry(uint32_t opcode, uint64_t node1, uint64_t node2);
    
    void recover_status();
    
    void recover_from_checkpoint();
    
    void play_log();
    
    void execute_log_entry(log_entry_t* entry);
    
    void add_checkpt_entry(uint64_t node1, uint64_t node2);
    
    void checkpoint();
    
    bool log_is_full();
    
    void close_log();
    
    ~server_log();
};
#endif
