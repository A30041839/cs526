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
  uint32_t checksum;
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
    memset((void*)this, 0, 4096);
  }

  uint32_t compute_checksum() {
    uint32_t checksum = 0;
    for (int i = 1; i < 1024; ++i) {
      uint32_t* ptr = (uint32_t*)this + i;
      checksum ^= *ptr;
    }
    return checksum + CHECKSUM_OFFSET;
  }

  void clear() {
    memset((void*)this, 0, 4096);
  }

};

//4096 bytes
struct log_block_t {
  uint32_t checksum;
  uint32_t generation_num;
  uint32_t entry_cnt;
  uint32_t reserved1;

  log_entry_t log_entry[170];

  log_block_t() {
    memset((void*)this, 0, 4096);
  }

  uint32_t compute_checksum() {
    uint32_t checksum = 0;
    for (int i = 1; i < 1024; ++i) {
      uint32_t* ptr = (uint32_t*)this + i;
      checksum ^= *ptr;
    }
    return checksum + CHECKSUM_OFFSET;
  }

  void clear() {
    memset((void*)this, 0, 4096);
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
    memset((void*)this, 0, 4096);
  }

  void clear() {
    memset((void*)this, 0, 4096);
  }

};

class server_log {
private:
  const char* log;

  super_block_t super_block;
  log_block_t cur_block;
  checkpt_block_t checkpt_block;

  int block_offset;

  struct Graph* graph = nullptr;

public:

  void bind_graph(struct Graph* g) {
    graph = g;
  } 

  void attach_log(const char* devfile) {
    log = devfile;
  }

  void init_server_log() {
    read_in_superblock(&super_block);
    if (super_block.checksum != super_block.compute_checksum()) {
      //the super block is not initialized
      init_superblock();
    }else {
      recover_status();
    }
  }

  void init_superblock() {
    super_block.clear();
    super_block.log_start = 1;
    super_block.log_size = LOG_SEG_SIZE;
    super_block.checksum = super_block.compute_checksum();
    block_offset = 1;
    write_super_block(&super_block);
  }

  void format() {
    read_in_superblock(&super_block);
    if (super_block.checksum != super_block.compute_checksum()) {
      init_superblock();
      return;
    }
    int old_generation_num = super_block.generation_num;
    super_block.clear();
    super_block.generation_num = old_generation_num + 1;
    super_block.log_start = 1;
    super_block.log_size = LOG_SEG_SIZE;
    super_block.checksum = super_block.compute_checksum();
    block_offset = 1;
    write_super_block(&super_block);
  }

  void read_in_superblock(super_block_t* sb) {
    int fd = open(log, O_RDWR);
    void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
    memcpy((void*)sb, addr, BLOCK_SIZE);
    munmap(addr, BLOCK_SIZE);
    close(fd);
  }

  void read_in_log_block(log_block_t* lb, uint32_t offset) {
    int fd = open(log, O_RDWR);
    void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, offset * BLOCK_SIZE);
    memcpy((void*)lb, addr, BLOCK_SIZE);
    munmap(addr, BLOCK_SIZE);
    close(fd);
  }

  void read_in_checkpt_block(checkpt_block_t* cb, uint32_t offset) {
    int fd = open(log, O_RDWR);
    void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, offset);
    memcpy((void*)cb, addr, BLOCK_SIZE);
    munmap(addr, BLOCK_SIZE);
    close(fd);
  }

  void write_super_block(super_block_t* sb) {
    int fd = open(log, O_RDWR);
    void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
    memcpy(addr, (void*)sb, BLOCK_SIZE);
    msync(addr, BLOCK_SIZE, MS_SYNC);
    munmap(addr, BLOCK_SIZE);
    close(fd);
  }

  void write_log_block(log_block_t* lb, uint32_t offset) {
    int fd = open(log, O_RDWR);
    void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, offset * BLOCK_SIZE);
    memcpy(addr, (void*)lb, BLOCK_SIZE);
    msync(addr, BLOCK_SIZE, MS_SYNC);
    munmap(addr, BLOCK_SIZE);
    close(fd);
  }

  void write_checkpt_block(checkpt_block_t* cb, uint32_t offset) {
    int fd = open(log, O_RDWR);
    void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, offset * BLOCK_SIZE);
    memcpy(addr, (void*)cb, BLOCK_SIZE);
    msync(addr, BLOCK_SIZE, MS_SYNC);
    munmap(addr, BLOCK_SIZE);
    close(fd);
  }

  void add_log_entry(uint32_t opcode, uint64_t node1, uint64_t node2) {
    if (cur_block.entry_cnt == 170) {
      //current log block is full
      block_offset++;
      cur_block.clear();
    }
    cur_block.generation_num = super_block.generation_num;
    log_entry_t new_log_entry(opcode, node1, node2);
    cur_block.log_entry[cur_block.entry_cnt++] = new_log_entry; 
    cur_block.checksum = cur_block.compute_checksum();
    write_log_block(&cur_block, block_offset);
  }

  void recover_status() {
    recover_from_checkpoint();
    play_log();
  }

  void recover_from_checkpoint() {
    if (super_block.checkpoint_size == 0) {
      //no checkpoint
      return;
    }
    for (uint32_t i = 0; i < super_block.checkpoint_size; ++i) {
      read_in_checkpt_block(&checkpt_block, super_block.log_size + i);
      for (uint32_t k = 0; k < checkpt_block.entry_cnt; ++k) {
        uint64_t node1 = checkpt_block.edges[k].node1;
        uint64_t node2 = checkpt_block.edges[k].node2;
        //add edge from node1 to node2
        graph->g[node1].insert(node2);
        graph->g[node2].insert(node1); 
      }
    }
  }

  void play_log() {
    uint32_t log_start = super_block.log_start;
    uint32_t log_size = super_block.log_size;
    log_block_t lb;
    uint32_t i = log_start;
    uint32_t lastsize = 0;
    for (; i < log_size; ++i) {
      read_in_log_block(&lb, i);
      //check checksum and generation number
      if (lb.checksum == lb.compute_checksum()
        && lb.generation_num == super_block.generation_num) {
        //this log block is valid
        lastsize = lb.entry_cnt;
        for (uint32_t k = 0; k < lb.entry_cnt; ++k) {
          execute_log_entry(&lb.log_entry[k]);
        }
      }else {
        //we've reached at the end of the log
        break;
      } 
    }
    block_offset = i;
    if (lastsize < 170 && lastsize > 0) {
      //last log block still has empty space for log entries
      block_offset--;
    }
  }

  void execute_log_entry(log_entry_t* entry) {
    if (!entry) {
      return;
    }
    //read operation code
    switch (entry->opcode) {
    case OP_ADD_NODE:
      graph->addNode(entry->node1);
      break;
    case OP_ADD_EDGE:
      graph->addEdge(entry->node1, entry->node2);
      break;
    case OP_REMOVE_NODE:
      graph->removeNode(entry->node1);
      break;
    case OP_REMOVE_EDGE:
      graph->removeEdge(entry->node1, entry->node2);
      break;
    default:
      break;
    }
  } 

  void add_checkpt_entry(uint64_t node1, uint64_t node2) {
    if (checkpt_block.entry_cnt == 255) {
      block_offset++;
      checkpt_block.clear();
    }
    edge_t new_edge;
    new_edge.node1 = node1;
    new_edge.node2 = node2;
    checkpt_block.edges[checkpt_block.entry_cnt++] = new_edge; 
    write_checkpt_block(&checkpt_block, block_offset);
  }

  void checkpoint() {
    //traverse all edge pairs and store them in the checkpoint area
    //because graph is an undirected graph, every edge just store once
    //make sure small node id goes before large node id
    printf("Creating checkpoint...");
    block_offset = super_block.log_size;
    for (auto& p : graph->g) {
      uint64_t n1 = p.first; 
      for (uint64_t n2 : p.second) {
        if (n1 < n2) {
          add_checkpt_entry(n1, n2);
        }
      }
    }
    super_block.checkpoint_size = block_offset - super_block.log_size + 1;
    super_block.generation_num++;
    super_block.checksum = super_block.compute_checksum();
    write_super_block(&super_block);
    block_offset = super_block.log_start;
  }

  bool log_is_full() {
    return block_offset >= super_block.log_size;
  }
};

#endif
