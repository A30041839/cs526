#include "log.hpp"
#include <string>
#include <cstdio>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <inttypes.h>
#include "graph.hpp"
#include "types.hpp"
#include "debug.hpp"

void server_log::bind_graph(struct Graph* g) {
  graph = g;
}

void server_log::attach_log(const string& devfile) {
  log = devfile.c_str();
  fd = open(log, O_RDWR);
  if (fd < 0) {
    print_debug("Open log disk failed.");
  }
}

void server_log::init_server_log() {
  print_debug("Init server log.");
  read_in_superblock(&super_block);
  if (super_block.checksum != super_block.compute_checksum()) {
    //the super block is not initialized
    init_superblock();
  }else {
    recover_status();
  }
}

void server_log::init_superblock() {
  print_debug("Init super block.");
  super_block.clear();
  super_block.log_start = 1;
  super_block.log_size = LOG_SEG_SIZE;
  super_block.checksum = super_block.compute_checksum();
  block_offset = 1;
  write_super_block(&super_block);
}

void server_log::format() {
  print_debug("Format system.");
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

void server_log::read_in_superblock(super_block_t* sb) {
  void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
  memcpy((void*)sb, addr, BLOCK_SIZE);
  munmap(addr, BLOCK_SIZE);
}

void server_log::read_in_log_block(log_block_t* lb, uint32_t offset) {
  void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, offset * BLOCK_SIZE);
  memcpy((void*)lb, addr, BLOCK_SIZE);
  munmap(addr, BLOCK_SIZE);
}

void server_log::read_in_checkpt_block(checkpt_block_t* cb, uint32_t offset) {
  void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, offset * BLOCK_SIZE);
  memcpy((void*)cb, addr, BLOCK_SIZE);
  munmap(addr, BLOCK_SIZE);
}

void server_log::write_super_block(super_block_t* sb) {
  void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
  memcpy(addr, (void*)sb, BLOCK_SIZE);
  msync(addr, BLOCK_SIZE, MS_SYNC);
  munmap(addr, BLOCK_SIZE);
}

void server_log::write_log_block(log_block_t* lb, uint32_t offset) {
  void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, offset * BLOCK_SIZE);
  memcpy(addr, (void*)lb, BLOCK_SIZE);
  msync(addr, BLOCK_SIZE, MS_SYNC);
  munmap(addr, BLOCK_SIZE);
}

void server_log::write_checkpt_block(checkpt_block_t* cb, uint32_t offset) {
  void* addr = mmap(NULL, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, offset * BLOCK_SIZE);
  memcpy(addr, (void*)cb, BLOCK_SIZE);
  msync(addr, BLOCK_SIZE, MS_SYNC);
  munmap(addr, BLOCK_SIZE);
}

void server_log::add_log_entry(uint32_t opcode, uint64_t node1, uint64_t node2) {
  char buf[100];
  switch (opcode) {
    case OP_ADD_NODE:
      sprintf(buf, "Add log entry: Add node %" PRIu64 ".", node1);
      print_debug(buf);
      break;
    case OP_ADD_EDGE:
      sprintf(buf, "Add log entry: Add edge <%" PRIu64 ",%" PRIu64 ">.", node1, node2);
      print_debug(buf);
      break;
    case OP_REMOVE_NODE:
      sprintf(buf, "Add log entry: Remove node %" PRIu64 ".", node1);
      print_debug(buf);
      break;
    case OP_REMOVE_EDGE:
      sprintf(buf, "Add log entry: Remove edge <%" PRIu64 ",%" PRIu64 ">.", node1, node2);
      print_debug(buf);
      break;
    default:
      break;
  }
    
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

void server_log::recover_status() {
  print_debug("Recovering status.");
  recover_from_checkpoint();
  play_log();
}

void server_log::recover_from_checkpoint() {
  print_debug("Recovering from checkpoint.");
  if (super_block.checkpoint_size == 0) {
    //no checkpoint
    print_debug("No checkpoint detected. Skip.");
    return;
  }
  print_debug("Reading checkpoint.");
  char buf[100];
  for (uint32_t i = 0; i < super_block.checkpoint_size; ++i) {
    read_in_checkpt_block(&checkpt_block, super_block.log_size + i);
    for (uint32_t k = 0; k < checkpt_block.entry_cnt; ++k) {
      uint64_t node1 = checkpt_block.edges[k].node1;
      uint64_t node2 = checkpt_block.edges[k].node2;
      if (node1 == node2) {
        //this is a node info
        sprintf(buf, "Reading checkpoint. Add node %" PRIu64 ".", node1);
        print_debug(buf);
        graph->g[node1] = unordered_set<uint64_t>();
      }else {
        //add edge from node1 to node2
        sprintf(buf, "Reading checkpoint. Add edge <%" PRIu64 ",%" PRIu64 ">.", node1, node2);
        print_debug(buf);
        graph->g[node1].insert(node2);
        graph->g[node2].insert(node1);
      }
    }
  }
}

void server_log::play_log() {
  print_debug("Playing log.");
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

void server_log::execute_log_entry(log_entry_t* entry) {
  print_debug("Executing log entry.");
  if (!entry) {
    return;
  }
  char buf[100];
  //read operation code
  switch (entry->opcode) {
    case OP_ADD_NODE:
      sprintf(buf, "Executing log entry. Add node %" PRIu64 ".", entry->node1);
      print_debug(buf);
      graph->addNode(entry->node1);
      break;
    case OP_ADD_EDGE:
      sprintf(buf, "Executing log entry. Add edge <%" PRIu64 ",%" PRIu64 ">.", entry->node1, entry->node2);
      print_debug(buf);
      graph->addEdge(entry->node1, entry->node2);
      break;
    case OP_REMOVE_NODE:
      sprintf(buf, "Executing log entry. Remove node %" PRIu64 ".", entry->node1);
      print_debug(buf);
      graph->removeNode(entry->node1);
      break;
    case OP_REMOVE_EDGE:
      sprintf(buf, "Executing log entry. Remove edge <%" PRIu64 ",%" PRIu64 ">.", entry->node1, entry->node2);
      print_debug(buf);
      graph->removeEdge(entry->node1, entry->node2);
      break;
    default:
      break;
  }
}

void server_log::add_checkpt_entry(uint64_t node1, uint64_t node2) {
  print_debug("Adding checkpoint entry.");
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

void server_log::checkpoint() {
  print_debug("Creating checkpoint.");
  //first we store all the node info to the checkpoint in the form
  //<node, node>
  //second we store all the edges in the graph by traversing
  //all edge pairs and store them in the checkpoint area
  //because graph is an undirected graph, every edge just store once
  //make sure small node id goes before large node id
  block_offset = super_block.log_size;
  //store all the nodes in a pair <node, node>
  for (auto& p : graph->g) {
    uint64_t n = p.first;
    add_checkpt_entry(n, n);
  }
  //store all the edges in the graph
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

bool server_log::log_is_full() {
  return block_offset >= super_block.log_size;
}

void server_log::close_log() {
  close(fd);
}

server_log::~server_log() {
  close_log();
}
