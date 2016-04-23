#ifndef _RPCSENDER_SERVER
#define _RPCSENDER_SERVER

#include <iostream>
#include <memory>
#include <string>
#include <grpc++/grpc++.h>
#include <cstdlib>
#include "graphserverRPC.grpc.pb.h"
#include "rpcsender_client.cc"
#include "log.hpp"
#include "graph.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using graphserverRPC::AddNodeRequest;
using graphserverRPC::AddEdgeRequest;
using graphserverRPC::RemoveNodeRequest;
using graphserverRPC::RemoveEdgeRequest;
using graphserverRPC::RPCReply;
using graphserverRPC::rpcsender;

// Logic and data behind the server's behavior.
class rpcsenderServiceImpl final : public rpcsender::Service {
  Status SendAddNode(ServerContext* context, const AddNodeRequest* request,
      RPCReply* reply) override {
    if (slog->log_is_full()) {
      std::string prefix("Add node fail: log is full!");
      reply->set_message(prefix);
      return Status::CANCELLED;
    }else {
      if (grpc_client != nullptr) {
        string rep = grpc_client->SendAddNode(request->node_id());
        if (rep == "RPC failed") {
          std::string prefix("Add node fail: rpc failed!");
          reply->set_message(prefix);
          return Status::CANCELLED;
        }else {
          uint64_t node_id = strtoull(request->node_id().c_str(), nullptr, 10);
          int status_code = graph->addNode(node_id);
          if (status_code == 200) {
            slog->add_log_entry(OP_ADD_NODE, node_id, 0);
          }
        }
      }else {
        //this is the last node in the chain
        uint64_t node_id = strtoull(request->node_id().c_str(), nullptr, 10);
        int status_code = graph->addNode(node_id);
        if (status_code == 200) {
          slog->add_log_entry(OP_ADD_NODE, node_id, 0);
        }
      }
    }
    std::string prefix("Successfully added node: ");
    reply->set_message(prefix + request->node_id());
    return Status::OK;
  }

  Status SendAddEdge(ServerContext* context, const AddEdgeRequest* request,
      RPCReply* reply) override {
    if (slog->log_is_full()) {
      std::string prefix("Add edge fail: log is full!");
      reply->set_message(prefix);
      return Status::CANCELLED;
    }else {
      if (grpc_client != nullptr) {
        string rep = grpc_client->SendAddEdge(request->node_id_a(), request->node_id_b());
        if (rep == "RPC failed") {
          std::string prefix("Add edge fail: rpc failed!");
          reply->set_message(prefix);
          return Status::CANCELLED;
        }else {
          uint64_t node_id_a = strtoull(request->node_id_a().c_str(), nullptr, 10);
          uint64_t node_id_b = strtoull(request->node_id_b().c_str(), nullptr, 10);
          int status_code = graph->addEdge(node_id_a, node_id_b);
          if (status_code == 200) {
            slog->add_log_entry(OP_ADD_EDGE, node_id_a, node_id_b);
          }
        }
      }else {
        //this is the last node in the chain
        uint64_t node_id_a = strtoull(request->node_id_a().c_str(), nullptr, 10);
        uint64_t node_id_b = strtoull(request->node_id_b().c_str(), nullptr, 10);
        int status_code = graph->addEdge(node_id_a, node_id_b);
        if (status_code == 200) {
          slog->add_log_entry(OP_ADD_EDGE, node_id_a, node_id_b);
        }
      }
    }
    std::string prefix("Successfully added edge: ");
    reply->set_message(prefix + request->node_id_a() + "," + request->node_id_b());
    return Status::OK;
  }

  Status SendRemoveNode(ServerContext* context, const RemoveNodeRequest* request,
      RPCReply* reply) override {
    if (slog->log_is_full()) {
      std::string prefix("Remove node fail: log is full!");
      reply->set_message(prefix);
      return Status::CANCELLED;
    }else {
      if (grpc_client != nullptr) {
        string rep = grpc_client->SendRemoveNode(request->node_id());
        if (rep == "RPC failed") {
          std::string prefix("Remove node fail: rpc failed!");
          reply->set_message(prefix);
          return Status::CANCELLED;
        }else {
          uint64_t node_id = strtoull(request->node_id().c_str(), nullptr, 10);
          int status_code = graph->removeNode(node_id);
          if (status_code == 200) {
            slog->add_log_entry(OP_REMOVE_NODE, node_id, 0);
          }
        }
      }else {
        //this is the last node in the chain
        uint64_t node_id = strtoull(request->node_id().c_str(), nullptr, 10);
        int status_code = graph->removeNode(node_id);
        if (status_code == 200) {
          slog->add_log_entry(OP_REMOVE_NODE, node_id, 0);
        }
      }
    }

    std::string prefix("Successfully removed node: ");
    reply->set_message(prefix + request->node_id());
    return Status::OK;
  }

  Status SendRemoveEdge(ServerContext* context, const RemoveEdgeRequest* request,
      RPCReply* reply) override {
    if (slog->log_is_full()) {
      std::string prefix("Remove edge fail: log is full!");
      reply->set_message(prefix);
      return Status::CANCELLED;
    }else {
      if (grpc_client != nullptr) {
        string rep = grpc_client->SendRemoveEdge(request->node_id_a(), request->node_id_b());
        if (rep == "RPC failed") {
          std::string prefix("Remove edge fail: rpc failed!");
          reply->set_message(prefix);
          return Status::CANCELLED;
        }else {
          uint64_t node_id_a = strtoull(request->node_id_a().c_str(), nullptr, 10);
          uint64_t node_id_b = strtoull(request->node_id_b().c_str(), nullptr, 10);
          int status_code = graph->removeEdge(node_id_a, node_id_b);
          if (status_code == 200) {
            slog->add_log_entry(OP_REMOVE_EDGE, node_id_a, node_id_b);
          }
        }
      }else {
        //this is the last node in the chain
        uint64_t node_id_a = strtoull(request->node_id_a().c_str(), nullptr, 10);
        uint64_t node_id_b = strtoull(request->node_id_b().c_str(), nullptr, 10);
        int status_code = graph->removeEdge(node_id_a, node_id_b);
        if (status_code == 200) {
          slog->add_log_entry(OP_REMOVE_EDGE, node_id_a, node_id_b);
        }
      }
    }
    std::string prefix("Successfully removed edge: ");
    reply->set_message(prefix + request->node_id_a() + "," + request->node_id_b());
    return Status::OK;
  }

  public:
  struct Graph* graph = nullptr;
  server_log* slog = nullptr;
  rpcsenderClient* grpc_client = nullptr;

  void bind_graph(struct Graph* g) {
    graph = g;
  }

  void bind_log(server_log* log) {
    slog = log;
  }

  void bind_grpc_client(rpcsenderClient* cli) {
    grpc_client = cli;
  }
};
#endif
