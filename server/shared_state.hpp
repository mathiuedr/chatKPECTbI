//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#ifndef BOOST_BEAST_EXAMPLE_WEBSOCKET_CHAT_MULTI_SHARED_STATE_HPP
#define BOOST_BEAST_EXAMPLE_WEBSOCKET_CHAT_MULTI_SHARED_STATE_HPP
#include <chrono>
#include <utility>
#include "util.hpp"
#include "parser.hpp"
#include "sqlite/sqlite3.h"

// Forward declaration
class websocket_session;
class symbol;

// Represents the shared server state
class shared_state : public boost::enable_shared_from_this<shared_state>
{
    std::string const doc_root_;
    std::string const topics_;
    parser parser_;
    std::string db_root_;
    enum class MessageType { Subscribe = 0, Unsubscribe = 1,Message=2 };
public:
    explicit
        shared_state(std::string doc_root, std::string topics);

    std::string const&
        doc_root() const noexcept
    {
        return doc_root_;
    }
    std::string const
        db_root() const noexcept
    {
        return db_root_;
    }
    // This mutex synchronizes all access to sessions_
    void createChat(websocket_session* session, std::string chatName, std::vector<std::string> invited);
    void getChatList(websocket_session* session);
    void getMessageList(websocket_session* session);
    std::mutex mutex_;
    std::map<std::string, boost::shared_ptr<symbol> > symbols_;
    boost::lockfree::spsc_queue<std::pair<std::string, std::string>, boost::lockfree::capacity<1024> > spsc_queue_;
    boost::lockfree::spsc_queue<std::tuple<parser::MsgType,uint32_t, uint32_t, std::string,int64_t,std::optional<std::vector<int>>>, boost::lockfree::capacity<1024> > spsc_queue_subscriber_;
    // Keep a list of all the connected clients
    std::unordered_map<std::string,websocket_session*> sess___;
    std::unordered_set<websocket_session*> sessions_;
    void join(websocket_session* session);
    void websocket_subscribe_to_symbols(websocket_session* session, std::string chatId);
    void websocket_unsubscribe_to_symbols(websocket_session* session);
    void deleteUserFromChat(websocket_session* session, int chatId);
    void deleteUserAccount(websocket_session* session);
    void getUserList(websocket_session* session);
    void newUser(std::string name,int id);
    void inviteToChat(int chatId, std::vector<int> userId,int parentUser);
    void getUserInChatList(websocket_session* session);
    void leave(websocket_session* session);
    void sendMsg(websocket_session* session,std::string message);
    void parse(std::string msg, websocket_session* session);
};

#endif