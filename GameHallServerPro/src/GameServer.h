//
// Created by xmr on 2024/8/6.
//

#ifndef GAMESERVER_GAMESERVER_H
#define GAMESERVER_GAMESERVER_H

#include <cstdint>
#include <unordered_map>
#include <string>
class Session;

class IGameCell {
public:
    virtual ~IGameCell() {}

    virtual void Exec() = 0;

    virtual void Release() { delete this; };
};

class User;

class GameServer {
private:
    std::unordered_map<uint64_t, Session *> _sessions;
    std::unordered_map<std::string, User *> _user4usernames;
    std::unordered_map<uint64_t, User *> _user4uids;
public:
    static GameServer *Ins() 
    {
        static GameServer ins;
        return &ins;
    }

    void Post(IGameCell *cell);

    void Init();

    void AddSession(Session *session);

    void RemoveSession(Session *session);

    Session *FindNetIdSession(uint64_t id);

    void AddUser(User *user);

    void RemoveUser(User *user);

    void RemoveUserByUserId(int userId);

    User *FindUser(uint64_t uid);

    User *FindUser(std::string username);

    void ServerDeleteRoom(int roomId);

private:
    void OnPost(IGameCell *cell);

};


#endif //GAMESERVER_GAMESERVER_H
