//
// Created by xmr on 2024/8/7.
//
#include "../Session.h"
#include "../DBMgr.h"
#include "Utils.h"
#include "SnowFlake.hpp"
#include "../GameServer.h"
#include "../User.h"

namespace 
{
    using snowflake_t = SnowFlake<1534832906275L>;
    snowflake_t g_snoFlake;
}


class Register_GameCell : public IGameCell {
public:
    uint64_t netId;
    bool bStatus;

    ~Register_GameCell() override 
    {

    }

    virtual void Exec() override 
    {
        Session* session = GameServer::Ins()->FindNetIdSession(netId);
        if (nullptr == session) {
            LOG_ERROR << "没有找到对应的 Session:" << netId;
            return;
        }
        session->SC_Register(bStatus);
    }

    virtual void Release() override
    {
        IGameCell::Release();
    }
};

class Register_DBCell : public IDBCell {

public:
    std::string username;
    std::string password;
    std::string nickname;
    uint64_t netId;

    Register_DBCell() {

    }

    ~Register_DBCell() override 
    {

    }

    virtual void DBExec(X::DB::DBSqlite* pSqlite) override 
    {

		const char* sql = u8R"(
    INSERT INTO "tb_users" (
        "userName", "password", "nickname", "token", "regeditTime",
        "regeditIp", "uuId", "status", "isDelete"
    )
    VALUES (
        ?, ?, ?, ?, CURRENT_TIMESTAMP,
        ?, ?, 1, 0
    ))";
        char* pError = NULL;

        bool bStatus = false;
        if (pSqlite->BeginPrecompiled(sql, &pError)) {
            std::string token = X::Utils::GeneratorUUID();
            //pSqlite->PrecompiledBind(1, g_snoFlake.NextId());
            pSqlite->PrecompiledBind(1, username.c_str(), username.length());
            pSqlite->PrecompiledBind(2, password.c_str(), password.length());
            pSqlite->PrecompiledBind(3, nickname.c_str(), nickname.length());
            pSqlite->PrecompiledBind(4, token.c_str(), token.length());
            pSqlite->PrecompiledBind(5, "127.0.0.1", strlen("127.0.0.1"));
            pSqlite->PrecompiledBind(6, g_snoFlake.NextId());
            auto e = pSqlite->StepPrecompiled();
            if (X::DB::Sqlite3Enum::SQLITE_ROW == e || X::DB::Sqlite3Enum::SQLITE_DONE == e) {
                bStatus = true;
            }
            pSqlite->EndPrecompiled();
        }
        if (pError)
            LOG_ERROR << pError;

        Register_GameCell* cell = new Register_GameCell;
        cell->netId = netId;
        cell->bStatus = bStatus;
        GameServer::Ins()->Post(cell);

    }

	virtual void Release() override
	{
		IDBCell::Release();
	}
};


void Session::CS_Register(cJSON* root) {

    const char* msg = cJSON_Print(root);

    LOG_INFO << "收到cJSON:\n" << msg;

    delete msg;

    cJSON* username = cJSON_GetObjectItem(root, "username");
    cJSON* password = cJSON_GetObjectItem(root, "password");
    cJSON* nickname = cJSON_GetObjectItem(root, "nickname");
    if (nickname == nullptr || username == nullptr || password == nullptr)
        return;

    if (nickname->type != cJSON_String || username->type != cJSON_String || password->type != cJSON_String)
        return;

    Register_DBCell* cell = new Register_DBCell();
    cell->username = username->valuestring;
    cell->password = password->valuestring;
    cell->nickname = nickname->valuestring;
    cell->netId = GetNetId();
    DBMgr::Post(cell);
}

void Session::SC_Register(bool status)
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "SC_Register");
    if (status)
    {
	cJSON_AddStringToObject(root, "msg", "Register Success");
	cJSON_AddItemToObject(root, "status", cJSON_CreateBool(status));
    }
    else
    {
    cJSON_AddStringToObject(root, "msg", "Register Fail");
	cJSON_AddItemToObject(root, "status", cJSON_CreateBool(status));
    }
	SendJSON(root);
	cJSON_Delete(root);
}