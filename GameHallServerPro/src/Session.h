//
// Created by xmr on 2024/8/7.
//

#ifndef GAMESERVER_SESSION_H
#define GAMESERVER_SESSION_H

#include "XNet.h"
#include <unordered_map>
#include "cJSON.h"

class User;
struct UserInfo;

class Session : public X::TcpSocket {
public:
    Session();

    ~Session();

    int OnNetMsg(const char *const buff, int len) override;

    void OnDisconnect() override;

public:
    uint64_t GetNetId() { return _netId; }

    void SetUser(User *user) { _user = user; }

public:
    void SendJSON(cJSON *root);

	void CS_Register(cJSON* root);

	void SC_Register(bool status);

    void CS_Login(cJSON *root);

    void SC_Login(bool status);

    void CS_Logout(cJSON *root);

    void SC_Logout();

    void SC_PreemptLoginEvent(std::string& username, std::string& password);

    void CS_SurePreemptLoginEvent(cJSON* root);

    void SC_PreemptLoginSureEvent(bool bStatus, UserInfo userInfo);

    User* GetUser() { return _user; }

private:
    std::unordered_map<std::string, std::function<void(cJSON *root)>> _funcs;
    uint64_t _netId;
    User *_user;
};


#endif //GAMESERVER_SESSION_H
