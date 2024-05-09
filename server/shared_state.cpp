//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include "shared_state.hpp"
#include "websocket_session.hpp"
#include "symbol.hpp"
#include "sqlite/sqlite3.h"

shared_state::
shared_state(std::string doc_root, std::string db_root)
    : doc_root_(std::move(doc_root))
    , db_root_(db_root)
{
    sqlite3* db;
    sqlite3_open(db_root_.c_str(), &db);
    std::string sql = "SELECT id FROM Chat";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        
        std::string topic = std::to_string(sqlite3_column_int(stmt, 0));
        symbols_[topic] = boost::make_shared<symbol>(topic, "", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


void
shared_state::
join(websocket_session* session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "Inserting into session" << std::endl;
    sess___[std::to_string(session->getId())] = session;
    sessions_.insert(session);
}

void
shared_state::
websocket_subscribe_to_symbols(websocket_session* session, std::string chatId)
{
    
        // Lock shared state mutex
        // Insert into websocket set
        {
            std::lock_guard<std::mutex> lock(mutex_);
            session->topics.insert(chatId);
        }
        // Lock symbol mutex
        // Insert into symbol set
        std::lock_guard<std::mutex> lock(symbols_[chatId]->mutex_);
        // First websocket to subscribe to the symbol
        /*if (!symbols_[topic]->sessions_.size()) {
            spsc_queue_subscriber_.push(std::make_pair(parser::MsgType::SUBSCRIBE, topic));
        }*/
        symbols_[chatId]->join(session);
        
        std::cout << "Session size :" << symbols_[chatId]->sessions_.size() << std::endl;
}


void
shared_state::
websocket_unsubscribe_to_symbols(websocket_session* session)
{
        // Lock shared state mutex
        // Remove from websocket set
    
    std::string id;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (session->topics.empty()) return;
            id = *session->topics.begin();
            session->topics.erase(session->topics.begin());
        }
        std::lock_guard<std::mutex> lock(symbols_[id]->mutex_);
        symbols_[id]->leave(session);
}

void shared_state::deleteUserFromChat(websocket_session* session, int chatId)
{
    spsc_queue_subscriber_.push(std::make_tuple(parser::MsgType::DeleteUserFromChat, chatId, session->getId(), "",0,std::nullopt));
}

void shared_state::deleteUserAccount(websocket_session* session)
{
    spsc_queue_subscriber_.push(std::make_tuple(parser::MsgType::DeleteUserAccount, 0, session->getId(), "",0,std::nullopt));
    
}

void shared_state::getUserList(websocket_session* session)
{
    std::string sql = "SELECT Users.id,Users.name FROM Users";
    sqlite3_stmt* stmt = NULL;
    boost::json::object obj;
    boost::json::array arr;
    obj["topic"] = 1;
    int rc = sqlite3_prepare_v2(session->db, sql.c_str(), -1, &stmt, NULL);
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        boost::json::object ob;
        ob["userId"] = sqlite3_column_int(stmt, 0);
        ob["name"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        arr.emplace_back(ob);
    }
    obj["users"] = arr;
    sqlite3_finalize(stmt);
    boost::shared_ptr<std::string> ss = boost::make_shared<std::string>(boost::json::serialize(obj));
    session->send(ss);
}

void shared_state::newUser(std::string name, int id)
{
    boost::json::object obj;
    obj["topic"] = 0;
    obj["userId"] = id;
    obj["name"] = name;
    boost::shared_ptr<std::string> ss = boost::make_shared<std::string>(boost::json::serialize(obj));
    for (auto a : sessions_) {
        a->send(ss);
    }
}

void shared_state::inviteToChat(int chatId, std::vector<int> userId,int parentUser)
{
    spsc_queue_subscriber_.push(std::make_tuple(parser::MsgType::InviteToChat, chatId, parentUser, "", 0, 
        std::make_optional<std::vector<int>>(userId)));
}

void shared_state::getUserInChatList(websocket_session* session)
{
    std::string sql = "SELECT Users.id, Users.name FROM Users, UserInChat WHERE Users.id=UserInChat.userid AND UserInChat.chatid=?";
    sqlite3_stmt* stmt = NULL;
    boost::json::object obj;
    boost::json::array arr;
    obj["topic"] = 6;
    int rc = sqlite3_prepare_v2(session->db, sql.c_str(), -1, &stmt, NULL);
    if (session->topics.empty()) { return; }
    sqlite3_bind_int(stmt, 1, stoi(*session->topics.begin()));
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        boost::json::object ob;
        ob["id"] = sqlite3_column_int(stmt, 0);
        ob["user_name"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        arr.emplace_back(ob);
    }
    obj["users"] = arr;
    sqlite3_finalize(stmt);
    boost::shared_ptr<std::string> ss = boost::make_shared<std::string>(boost::json::serialize(obj));
    session->send(ss);
}

void shared_state::getChatList(websocket_session* session) {
    std::string sql = "SELECT Chat.id as chatId, Chat.name as chatName FROM Chat, UserInChat WHERE UserInChat.userid=? AND UserInChat.chatid=Chat.id";
    sqlite3_stmt* stmt = NULL;
    boost::json::object obj;
    boost::json::array arr;
    obj["topic"] = 2;
    int rc = sqlite3_prepare_v2(session->db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, session->getId());
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        boost::json::object ob;
        ob["chatId"] = sqlite3_column_int(stmt, 0);
        ob["name"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        arr.emplace_back(ob);
    }
    obj["chats"] = arr;
    sqlite3_finalize(stmt);
    boost::shared_ptr<std::string> ss = boost::make_shared<std::string>(boost::json::serialize(obj));
    session->send(ss);
}

void shared_state::createChat(websocket_session* session, std::string chatName, std::vector<std::string> invited) {
    std::string sql = "INSERT INTO Chat(name,adminid) VALUES(?,?)";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(session->db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, chatName.c_str(), std::strlen(chatName.c_str()), NULL);
    sqlite3_bind_int64(stmt, 2, session->getId());
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cout << "error\n";
        return;
    }
    sql = "SELECT id FROM Chat WHERE adminid=? ORDER BY id DESC LIMIT 1";
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    rc = sqlite3_prepare_v2(session->db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, session->getId());
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        int id = sqlite3_column_int(stmt, 0);
        boost::json::object ms;
        ms["topic"] = 3;
        ms["chatName"] = chatName;
        ms["chatId"] = id;
        boost::shared_ptr<std::string> ss = boost::make_shared<std::string>(boost::json::serialize(ms));
        invited.push_back(std::to_string(session->getId()));
        for (auto p : invited) {
            sql = "INSERT INTO UserInChat(chatid,userid,parentUser) VALUES(?,?,?)";
            sqlite3_clear_bindings(stmt);
            sqlite3_reset(stmt);
            rc = sqlite3_prepare_v2(session->db, sql.c_str(), -1, &stmt, NULL);
            sqlite3_bind_int(stmt, 1, id);
            sqlite3_bind_int(stmt, 2, std::stoi(p));
            sqlite3_bind_int(stmt, 3, session->getId());

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                sess___[p]->send(ss);
            }

        }
    }
    else {
        std::cout << "error\n";
    }
    sqlite3_finalize(stmt);
}

void shared_state::getMessageList(websocket_session* session) {
    std::string sql = "SELECT u.name,m.text,m.date,m.id,m.userid FROM Message m, UserInChat c ,Users u WHERE m.chatid=? AND u.id=m.userid AND c.userid = u.id AND c.chatid=m.chatid ORDER BY(date)";
    sqlite3_stmt* stmt = NULL;
    boost::json::object obj;
    boost::json::array arr;
    obj["topic"] = 4;
    int rc = sqlite3_prepare_v2(session->db, sql.c_str(), -1, &stmt, NULL);
    if (session->topics.empty()) { return; }
    sqlite3_bind_int(stmt, 1, stoi(*session->topics.begin()));
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        boost::json::object ob;
        ob["user_name"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        ob["text"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        ob["date"] = sqlite3_column_int64(stmt, 2);
        ob["msg_id"] = sqlite3_column_int(stmt, 3);
        ob["user_id"] = sqlite3_column_int(stmt, 4);
        arr.emplace_back(ob);
    }
    obj["messages"] = arr;
    sqlite3_finalize(stmt);
    boost::shared_ptr<std::string> ss = boost::make_shared<std::string>(boost::json::serialize(obj));
    session->send(ss);
}

void shared_state::sendMsg(websocket_session* session, std::string message) {
    // Put the message in a shared pointer so we can re-use it for each client
    auto const ss = boost::make_shared<std::string const>(std::move(message));
    if (session->topics.empty()) return;
    std::string chatId= *session->topics.begin();
    const auto p1 = std::chrono::system_clock::now();
    int64_t date = std::chrono::duration_cast<std::chrono::milliseconds>(
        p1.time_since_epoch()).count();
    spsc_queue_subscriber_.push(std::make_tuple(parser::MsgType::MESSAGE, stoi(chatId), session->getId(), *ss,date,std::nullopt));
    // Make a local list of all the weak pointers representing
    // the sessions, so we can do the actual sending without
    // holding the mutex:
    boost::json::object obj;
    obj["topic"] = 5;
    obj["Msg"] = *ss;
    //obj["userid"] = session->getId();
    sqlite3_stmt* stmt = NULL;
    std::string sql = "SELECT name FROM Users WHERE id=?";
    sqlite3_prepare_v2(session->db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, session->getId());
    sqlite3_step(stmt);
    obj["user_name"] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    obj["date"] = date;
    sqlite3_finalize(stmt);
    std::vector<boost::weak_ptr<websocket_session>> v;
    {
        std::lock_guard<std::mutex> lock_symbol(symbols_[chatId]->mutex_);
        v.reserve(symbols_[chatId]->sessions_.size());
        for (auto p : symbols_[chatId]->sessions_)
        {
            v.emplace_back(p->weak_from_this());
        }
    }
    // For each session in our local list, try to acquire a strong
    // pointer. If successful, then send the message on that session.
    for (auto const& wp : v) {
        if (auto sp = wp.lock()) {
            sp->send(boost::make_shared<std::string>(boost::json::serialize(obj)));
        }
    }
}

void
shared_state::
leave(websocket_session* session)
{
    
    // Remove the websocket from each symbol subscription
    // websocket_unsubscribe_symbols(session);
    websocket_unsubscribe_to_symbols(session);
    // Remove web socket from global session
    // std::cout << "Leaving session" << std::endl;
    std::lock_guard<std::mutex> lock(mutex_);
    sess___.erase(std::to_string(session->getId()));
    sessions_.erase(session);
}

void
shared_state::
parse(std::string msg, websocket_session* session)
{
    auto obj = parser_.parse(msg);
    switch (static_cast<parser::MsgType>(boost::json::value_to<int>(obj.at("ty"))))
    {
    case parser::MsgType::SUBSCRIBE:
    {
        std::string chatId = std::to_string(boost::json::value_to<int>(obj.at("to")));
        websocket_subscribe_to_symbols(session, chatId);
        break;
    }
    case parser::MsgType::UNSUBSCRIBE:
    {
        websocket_unsubscribe_to_symbols(session);
        break;
    }
    case parser::MsgType::MESSAGE:
    {
        //std::string chatId = boost::json::value_to<std::string>(obj.at("to"));
        std::string msg = boost::json::value_to<std::string>(obj.at("msg"));
        sendMsg(session, msg);
        break;
    }
    case parser::MsgType::CreateChat:
    {
        std::string chatName = boost::json::value_to<std::string>(obj.at("chatName"));
        std::vector<std::string> invited;
        std::string invited1= boost::json::value_to<std::string>(obj.at("Invited"));
        boost::json::array  arr= boost::json::parse(invited1).as_array();
        for (int i = 0; i < arr.size(); i++) {
            if (arr[i].is_int64()) {
                invited.push_back(std::to_string(boost::json::value_to<int>(arr[i])));
            }
            else if (arr[i].is_string()) {
                invited.push_back(boost::json::value_to<std::string>(arr[i]));
            }
            
        }
        createChat(session, chatName,  invited);
        break;
    }
    case parser::MsgType::GetChatList:
    {
        
        
        getChatList(session);
        break;
    }
    case parser::MsgType::GetMessageList:
    {
        getMessageList(session);
        break;
    }
    case parser::MsgType::DeleteUserFromChat:
    {
        int chatId = boost::json::value_to<int>(obj.at("chatId"));
        deleteUserFromChat(session, chatId);
        break;
    }
    case parser::MsgType::DeleteUserAccount:
    {
        deleteUserAccount(session);
        break;
    }
    case parser::MsgType::GetUserList:
    {
        getUserList(session);
        break;
    }
    case parser::MsgType::InviteToChat: {
        //int chatId = boost::json::value_to<int>(obj.at("chatId"));
        std::vector<int> invited;
        std::string invited1 = boost::json::value_to<std::string>(obj.at("Invited"));
        boost::json::array  arr = boost::json::parse(invited1).as_array();
        for (int i = 0; i < arr.size(); i++) {
            invited.push_back(boost::json::value_to<int>(arr[i]));
        }
        inviteToChat(std::stoi(*session->topics.begin()), invited, session->getId());
        break;
    }
    case parser::MsgType::GetUserInChatList:
    {
        getUserInChatList(session);
        break;
    }
    default:
        std::cout << "(Unsupported message type)" << std::endl;
        break;
    }
}