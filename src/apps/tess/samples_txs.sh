#!/bin/bash
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.

set -ex

# Get roles of current user
./client --pretty-print userrpc --req '{ "jsonrpc": "2.0", "id": 0, "method": "ROLES_GET"}'

# Create branch
./client --pretty-print userrpc --req '{ "jsonrpc": "2.0", "id": 0, "method": "CREATE_RELEASE_BRANCH", "params": { "repository": "ONNX", "branch": "v0.42", "policy": { "min_builds": 2 }, "info": { "arbitrary": ["user", "data"], "goes": "here" } }}'

# Get branch
./client --pretty-print userrpc --req '{ "jsonrpc": "2.0", "id": 0, "method": "GET_BRANCH", "params": { "repository": "ONNX", "branch": "v0.42" }}'

# Sign release 
./client --pretty-print userrpc --req '{ "jsonrpc": "2.0", "id": 0, "method": "SIGN_RELEASE_BRANCH", "params": { "repository": "ONNX", "branch": "v0.42", "pr": {}, "binary": [], "oe_sig_info": [] }}'

# Get release
./client --pretty-print userrpc --req '{ "jsonrpc": "2.0", "id": 0, "method": "GET_RELEASE", "params": { "release_id": 0 }}'

# Set github auth token to access tess-mockup repo
./client --pretty-print userrpc --req '{"jsonrpc": "2.0", "id": 0, "method": "SET_GITHUB_USER", "params": {"user_token": "a7cf346a5661801496299442cde471a34f531825"}}'

# Get details of PR
./client --pretty-print userrpc --req '{"jsonrpc": "2.0", "id": 0, "method": "GITHUB_GET", "params": {"path": "repos/ad-l/tess-mockup/pulls/3"}}'
