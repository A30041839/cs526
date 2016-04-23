#ifndef _RPCSENDER_CLIENT
#define _RPCSENDER_CLIENT 

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "graphserverRPC.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using graphserverRPC::AddNodeRequest;
using graphserverRPC::AddEdgeRequest;
using graphserverRPC::RemoveNodeRequest;
using graphserverRPC::RemoveEdgeRequest;
using graphserverRPC::RPCReply;
using graphserverRPC::rpcsender;

class rpcsenderClient {
  public:
    rpcsenderClient(std::shared_ptr<Channel> channel)
      : stub_(rpcsender::NewStub(channel)) {}

    // Assambles the client's payload, sends it and presents the response back
    // from the server.
    std::string SendAddNode(const std::string& node_id) {
      // Data we are sending to the server.
      AddNodeRequest request;
      request.set_node_id(node_id);

      // Container for the data we expect from the server.
      RPCReply reply;

      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = stub_->SendAddNode(&context, request, &reply);

      // Act upon its status.
      if (status.ok()) {
        return reply.message();
      } else {
        return "RPC failed";
      }
    }

    std::string SendAddEdge(const std::string& node_id_a, const std::string& node_id_b) {
      // Data we are sending to the server.
      AddEdgeRequest request;
      request.set_node_id_a(node_id_a);
      request.set_node_id_b(node_id_b);

      // Container for the data we expect from the server.
      RPCReply reply;

      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = stub_->SendAddEdge(&context, request, &reply);

      // Act upon its status.
      if (status.ok()) {
        return reply.message();
      } else {
        return "RPC failed";
      }
    }

    std::string SendRemoveNode(const std::string& node_id) {
      // Data we are sending to the server.
      RemoveNodeRequest request;
      request.set_node_id(node_id);

      // Container for the data we expect from the server.
      RPCReply reply;

      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = stub_->SendRemoveNode(&context, request, &reply);

      // Act upon its status.
      if (status.ok()) {
        return reply.message();
      } else {
        return "RPC failed";
      }
    }

    std::string SendRemoveEdge(const std::string& node_id_a, const std::string& node_id_b) {
      // Data we are sending to the server.
      RemoveEdgeRequest request;
      request.set_node_id_a(node_id_a);
      request.set_node_id_b(node_id_b);

      // Container for the data we expect from the server.
      RPCReply reply;

      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = stub_->SendRemoveEdge(&context, request, &reply);

      // Act upon its status.
      if (status.ok()) {
        return reply.message();
      } else {
        return "RPC failed";
      }
    }


  private:
    std::unique_ptr<rpcsender::Stub> stub_;
};
#endif
