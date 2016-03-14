#ifndef _UTILITY_H
#define _UTILITY_H

#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "mongoose.h"
#include "types.hpp"

using namespace std;

static unordered_map<int, string> status_code_mp = {
    {200, "OK"}, {204, "OK"}, {400, "Bad Request"},
    {507, "Checkpoint Needed"}
};

//parse the command type from the uri
static string get_command_type_from_uri(const char* uri) {
    string str(uri);
    size_t pos1 = str.find(' ');
    str = str.substr(0, pos1);
    size_t pos2 = str.find_last_of('/');
    return str.substr(pos2 + 1);
}

//get the node id parameters from token
static uint64_t get_node_from_token(struct json_token* tokens, const char* key) {
    struct json_token* tk = find_json_token(tokens, key);
    uint64_t id = stoull(string(tk->ptr, tk->len));
    return id;
}

//generate result http header
static string gen_result_http_header(int status_code, string status, size_t content_len) {
    ostringstream oss;
    oss << "HTTP/1.1 " << status_code << " " << status << "\r\n";
    oss << "Content-Length: " << content_len << "\r\n";
    oss << "Content-Type: application/json\r\n\r\n";
    return oss.str();
}

//generate get neighbors json result
static string gen_neighbor_json_result(uint64_t node, vector<uint64_t>& nodes) {
    string json = "\"node_id\": " + to_string(node) + ",";
    string neighbors;
    for (int i = 0; i < (int)nodes.size(); ++i) {
        neighbors.append(to_string(nodes[i]) + ",");
    }
    if (!neighbors.empty()) {
        neighbors.pop_back();
    }
    neighbors = "[" + neighbors + "]";
    json = json + "\"neighbors\": " + neighbors;
    json = "{" + json + "}";
    return json;
}

//clear a block
static uint64_t compute_checksum_xor(void* block_ptr) {
    uint64_t checksum = 0;
    for (int i = 1; i < BLOCK_SIZE / 8; ++i) {
        uint64_t* p = (uint64_t*)block_ptr + i;
        checksum ^= *p;
    }
    return checksum + CHECKSUM_OFFSET;
}

static void clear_block(void* block_ptr) {
    memset(block_ptr, 0, BLOCK_SIZE);
}
#endif /* utility_h */
