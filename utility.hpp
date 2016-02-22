//
//  utility.hpp
//  decentral
//
//  Created by Haoliang on 2/3/16.
//  Copyright © 2016 Haoliang. All rights reserved.
//

#ifndef utility_h
#define utility_h

#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "mongoose.h"

using namespace std;

unordered_map<int, string> status_code_mp = {
    {200, "OK"}, {204, "OK"}, {400, "Bad Request"}
};

//parse the command type from the uri
string get_command_type_from_uri(const char* uri) {
    string str(uri);
    size_t pos1 = str.find(' ');
    str = str.substr(0, pos1);
    size_t pos2 = str.find_last_of('/');
    return str.substr(pos2 + 1);
}

//get the node id parameters from token
uint64_t get_node_from_token(struct json_token* tokens, const char* key) {
    struct json_token* tk = find_json_token(tokens, key);
    uint64_t id = stoull(string(tk->ptr, tk->len));
    return id;
}

//generate result http header
string gen_result_http_header(int status_code, string status, size_t content_len) {
    ostringstream oss;
    oss << "HTTP/1.1 " << status_code << " " << status << "\r\n";
    oss << "Content-Length: " << content_len << "\r\n";
    oss << "Content-Type: application/json\r\n\r\n";
    return oss.str();
}

//generate get neighbors json result
string gen_neighbor_json_result(uint64_t node, vector<uint64_t>& nodes) {
    string json = "\"node_id\": " + to_string(node) + ",";
    string neighbors;
    for (int i = 0; i < nodes.size(); ++i) {
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

#endif /* utility_h */
