#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <unordered_map>
#include <cstdint>
#include "mongoose.h"
#include "graph.hpp"
#include "utility.hpp"
#include "log.hpp"
#include "types.hpp"

using namespace std;

static struct mg_serve_http_opts s_http_server_opts;
static int s_sig_num = 0;
static const struct mg_str s_post_method = MG_STR("POST");

struct Graph graph;

server_log slog;

static void signal_handler(int sig_num) {
  signal(sig_num, signal_handler);
  s_sig_num = sig_num;
}

static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix) {
  return uri->len > prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

static int is_equal(const struct mg_str *s1, const struct mg_str *s2) {
  return s1->len == s2->len && memcmp(s1->p, s2->p, s2->len) == 0;
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  static const struct mg_str api_prefix = MG_STR("/api/v1");
  struct http_message *hm = (struct http_message *) ev_data;
  struct json_token* tokens;
  string http_header;
  string json_result;
  string http_result;
  switch (ev) {
  case MG_EV_HTTP_REQUEST:
    if (has_prefix(&hm->uri, &api_prefix)) {
      if (is_equal(&hm->method, &s_post_method)){
        string request = get_command_type_from_uri(hm->uri.p);
        string param_json(hm->body.p, hm->body.len);
        tokens = parse_json2(param_json.c_str(), (int)param_json.size());

        if (request == "add_node") {
          if (slog.log_is_full()) {
            json_result = "";
            http_header = gen_result_http_header(507, status_code_mp[507], 0);
          }else {
            int status_code = graph.addNode(get_node_from_token(tokens, "node_id"));
            json_result = status_code == 200 ? param_json : "";
            http_header = gen_result_http_header(status_code, status_code_mp[status_code], json_result.size());
            if (status_code == 200) {
              slog.add_log_entry(OP_ADD_NODE, get_node_from_token(tokens, "node_id"), 0);
            }
          }
        }else if (request == "add_edge") {
          if (slog.log_is_full()) {
            json_result = "";
            http_header = gen_result_http_header(507, status_code_mp[507], 0);
          }else {
            int status_code = graph.addEdge(get_node_from_token(tokens, "node_a_id"), get_node_from_token(tokens, "node_b_id"));
            json_result = status_code == 200 ? param_json : "";
            http_header = gen_result_http_header(status_code, status_code_mp[status_code], json_result.size());
            if (status_code == 200) {
              slog.add_log_entry(OP_ADD_EDGE, get_node_from_token(tokens, "node_a_id"), get_node_from_token(tokens, "node_b_id"));
            }
          }
        }else if (request == "remove_node") {
          if (slog.log_is_full()) {
            json_result = "";
            http_header = gen_result_http_header(507, status_code_mp[507], 0);
          }else {
            int status_code = graph.removeNode(get_node_from_token(tokens, "node_id"));
            json_result = status_code == 200 ? param_json : "";
            http_header = gen_result_http_header(status_code, status_code_mp[status_code], json_result.size());
            if (status_code == 200) {
              slog.add_log_entry(OP_REMOVE_NODE, get_node_from_token(tokens, "node_id"), 0);
            }
          }
        }else if (request == "remove_edge") {
          if (slog.log_is_full()) {
            json_result = "";
            http_header = gen_result_http_header(507, status_code_mp[507], 0);
          }else {
            int status_code = graph.removeEdge(get_node_from_token(tokens, "node_a_id"), get_node_from_token(tokens, "node_b_id"));
            json_result = status_code == 200 ? param_json : "";
            http_header = gen_result_http_header(status_code, status_code_mp[status_code], json_result.size());
            if (status_code == 200) {
              slog.add_log_entry(OP_REMOVE_EDGE, get_node_from_token(tokens, "node_a_id"), get_node_from_token(tokens, "node_b_id"));
            }
          }
        }else if (request == "get_node") {
          pair<int, int> status = graph.getNode(get_node_from_token(tokens, "node_id"));
          char buf[1000];
          if (status.second == 1) {
            json_emit(buf, sizeof(buf), "{ s: T }", "in_graph");
          }else {
            json_emit(buf, sizeof(buf), "{ s: F }", "in_graph");
          }
          json_result = string(buf);
          http_header = gen_result_http_header(status.first, status_code_mp[status.first], json_result.size());
        }else if (request == "get_edge") {
          pair<int, int> status = graph.getEdge(get_node_from_token(tokens, "node_a_id"), get_node_from_token(tokens, "node_b_id"));
          char buf[1000];
          if (status.first == 200) {
            if (status.second == 1) {
              json_emit(buf, sizeof(buf), "{ s: T }", "in_graph");
            }else {
              json_emit(buf, sizeof(buf), "{ s: F }", "in_graph");
            }
            json_result = string(buf);
          }else {
            json_result = "";
          }
          http_header = gen_result_http_header(status.first, status_code_mp[status.first], json_result.size());
        }else if (request == "get_neighbors") {
          pair<int, vector<uint64_t>> status = graph.getNeighbors(get_node_from_token(tokens, "node_id"));
          if (status.first == 200) {
            json_result = gen_neighbor_json_result(get_node_from_token(tokens, "node_id"), status.second);
          }else {
            json_result = "";
          }
          http_header = gen_result_http_header(status.first, status_code_mp[status.first], json_result.size());
        }else if (request == "shortest_path") {
          pair<int, int> status = graph.shortestPath(get_node_from_token(tokens, "node_a_id"), get_node_from_token(tokens, "node_b_id"));
          char buf[1000];
          if (status.first == 200) {
            json_emit(buf, sizeof(buf), "{ s: i }", "distance", status.second);
            json_result = string(buf);
          }else {
            json_result = "";
          }
          http_header = gen_result_http_header(status.first, status_code_mp[status.first], json_result.size());
        }else if (request == "checkpoint") {
          if (slog.log_is_full()) {
            json_result = "";
            http_header = gen_result_http_header(507, status_code_mp[507], 0);
          }else {
            slog.checkpoint();
            json_result = "";
            http_header = gen_result_http_header(200, status_code_mp[200], 0);
          }
        }
        http_result = http_header + json_result;
        mg_printf(nc, "%s", http_result.c_str());
        free(tokens);
      }
    } else {
      mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
    }
    break;
  default:
    break;
  }
}

int main(int argc, const char * argv[]) {
  if (argc < 3) {
    cout << "Please provide a port number and devfile." << endl;
    return 0;
  }

  const char* s_http_port;
  const char* devfile;
  bool format = false;

  if (argc == 4) {
    if (strcmp(argv[1], "-f") == 0) {
      format = true;
    }
    s_http_port = argv[2];
    devfile = argv[3];
  }else {
    s_http_port = argv[1];
    devfile = argv[2];
  }

  slog.bind_graph(&graph);
  slog.attach_log(devfile);

  if (format) {
    slog.format();
  } else {
    slog.init_server_log();
  }

  struct mg_mgr mgr;
  struct mg_connection *nc;

  /* Open listening socket */
  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port, ev_handler);
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = "web_root";

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);


  /* Run event loop until signal is received */
  printf("Starting RESTful server on port %s\n", s_http_port);
  while (s_sig_num == 0) {
    mg_mgr_poll(&mgr, 1000);
  }



  printf("Exiting on signal %d\n", s_sig_num);

  return 0;
}
