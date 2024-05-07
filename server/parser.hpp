//
// SRAVZ LLC
//

#ifndef SRAVZ_WEB_PARSER_HPP
#define SRAVZ_WEB_PARSER_HPP

#include "util.hpp"

class parser : public boost::enable_shared_from_this<parser>
{
    boost::json::parse_options opt_{};
public:
    parser();
    boost::json::value parse(const std::string& s);
    enum class MsgType {
        SUBSCRIBE = 1,
        UNSUBSCRIBE = 2,
        MESSAGE =3,
        CreateChat=4,
        GetChatList=5,
        GetMessageList=6,
        DeleteUserFromChat=7,
        DeleteUserAccount=8,
        GetUserList=9,
        InviteToChat=10,
        GetUserInChatList=11
    };
};

#endif