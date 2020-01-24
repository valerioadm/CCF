// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "consensus/pbft/pbftpreprepares.h"
#include "ds/ringbuffer_types.h"
#include "kv/kvtypes.h"

namespace pbft
{
  using Index = uint64_t;
  using Term = uint64_t;
  using NodeId = uint64_t;
  using Node2NodeMsg = uint64_t;
  using CallerId = uint64_t;
  using RequestId = uint64_t;

  enum PbftMsgType : Node2NodeMsg
  {
    pbft_message = 1000,
    pbft_append_entries,
    pbft_status_message
  };

#pragma pack(push, 1)
  struct PbftHeader
  {
    PbftMsgType msg;
    NodeId from_node;
  };

  struct AppendEntries : PbftHeader
  {
    Index idx;
    Index prev_idx;
  };

  struct StatusMessage : PbftHeader
  {
    Index idx;
  };

#pragma pack(pop)

  template <typename S>
  class Store
  {
  public:
    virtual ~Store() {}
    virtual S deserialise_views(
      const std::vector<uint8_t>& data,
      bool public_only = false,
      bool commit = true,
      Term* term = nullptr,
      ccf::Store::Tx* tx = nullptr) = 0;
    virtual void compact(Index v) = 0;
    virtual kv::Version current_version() = 0;
    virtual void commit_pre_prepare(
      const pbft::PrePrepare& pp,
      pbft::PrePreparesMap& pbft_pre_prepares_map,
      ccf::Store::Tx& tx) = 0;
  };

  template <typename T, typename S>
  class Adaptor : public pbft::Store<S>
  {
  private:
    std::weak_ptr<T> x;

  public:
    Adaptor(std::shared_ptr<T> x) : x(x) {}

    S deserialise_views(
      const std::vector<uint8_t>& data,
      bool public_only = false,
      bool commit = true,
      Term* term = nullptr,
      ccf::Store::Tx* tx = nullptr)
    {
      auto p = x.lock();
      if (p)
        return p->deserialise_views(data, public_only, commit, term, tx);

      return S::FAILED;
    }

    void commit_pre_prepare(
      const pbft::PrePrepare& pp,
      pbft::PrePreparesMap& pbft_pre_prepares_map,
      ccf::Store::Tx& tx)
    {
      while (true)
      {
        auto p = x.lock();
        if (p)
        {
          auto version = p->next_version();
          LOG_TRACE_FMT("Storing pre prepare at seqno {}", pp.seqno);
          auto success = p->commit(
            version,
            [&]() {
              tx.set_reserved_version(version);
              auto pp_view = tx.get_view(pbft_pre_prepares_map);
              pp_view->put(0, pp);
              return tx.commit_reserved();
            },
            false);
          if (success == kv::CommitSuccess::OK)
          {
            break;
          }
        }
      }
    }

    void compact(Index v)
    {
      auto p = x.lock();
      if (p)
      {
        p->compact(v);
      }
    }

    kv::Version current_version()
    {
      auto p = x.lock();
      if (p)
      {
        return p->current_version();
      }
      return kv::NoVersion;
    }
  };

  using PbftStore = pbft::Store<kv::DeserialiseSuccess>;
}