//
// Created by xmr on 2024/8/7.
//

#include "DBMgr.h"

void DBMgr::Init()
{
	_sqlite.Create("GameServer.db");
	int version = 0;
	char* pError = nullptr;

	LOG_INFO << "数据库初始化";

	// 检查 表是否存在
    _sqlite.Exec("select versio from dbver;", &pError,
                 [&version](int column_size, char *column_value[], char *column_name[])
                 {
                     version = atoi(column_value[0]);
                 });
    if (pError) LOG_ERROR << pError;

    LOG_INFO << "当前数据库版本号:" << version;
    // 没有创建数据库
    // 需要创建数据库
    if (version == 0)
    {

        LOG_INFO << "没有找到基础数据库 现在创建基础表 版本号:100";
		const char* sql = R"(
            CREATE TABLE `dbver` (
                `versio` int NULL,
                PRIMARY KEY (`versio`)
            );
        )";
		_sqlite.Exec(sql, &pError);
		if (pError)
		{
			LOG_ERROR << pError;
			// 处理错误
		}

		// 插入版本号
		_sqlite.Exec("INSERT INTO `dbver` VALUES ('100');", &pError);
		if (pError)
		{
			LOG_ERROR << pError;
			// 处理错误
		}

		// UUID生成ID的创建用户表
		//sql = R"(
  //      CREATE TABLE `tb_users` (
  //      `uid` bigint NOT NULL,
  //      `userName` TEXT UNIQUE,
  //      `password` TEXT,
  //      `nickname` TEXT,
  //      `token` TEXT,
  //      `regeditTime` INTEGER,
  //      `regeditIp` TEXT,
  //      `prevId` INTEGER,
  //      `status` INTEGER,
  //      `isDelete` INTEGER
  //      )                       
  //      )";
        // 创造用户表
		sql = R"(
        CREATE TABLE `tb_users` (
        `uid` INTEGER PRIMARY KEY AUTOINCREMENT,
        `userName` TEXT UNIQUE,
        `password` TEXT,
        `nickname` TEXT,
        `token` TEXT,
        `regeditTime` INTEGER,
        `regeditIp` TEXT,
        `uuId` bigint INTEGER,
        `status` INTEGER,
        `isDelete` INTEGER
        )                       
        )";

		_sqlite.Exec(sql, &pError);
		if (pError)
		{
			LOG_ERROR << pError;
			// 处理错误
		}


        // ============================
		// 创建登录日志表
		sql = R"(
            CREATE TABLE `tab_login_logs` (
                `_id` integer NOT NULL PRIMARY KEY AUTOINCREMENT,
                `uid` INTEGER NOT NULL,
                `time` integer NOT NULL,
                `ip` TEXT NOT NULL
            );
        )";
		_sqlite.Exec(sql, &pError);
		if (pError)
		{
			LOG_ERROR << pError;
			// 处理错误
		}

		sql = R"(
            CREATE INDEX `u_uid` ON `tab_login_logs` (`uid`);
        )";
		_sqlite.Exec(sql, &pError);
		if (pError)
		{
			LOG_ERROR << pError;
			// 处理错误
		}

		// 设置版本号
		version = 100;
    }

    EventLoopMgr::GetDBEL()->Init(std::bind(&DBMgr::OnPost, this, std::placeholders::_1));

    std::thread t([this]{
        while(true)
        {
            EventLoopMgr::GetDBEL()->LoopOnce();
        }
    });
    t.detach();
}

void DBMgr::OnPost(void *ptr)
{
    IDBCell *cell = (IDBCell *) ptr;
    cell->DBExec(&_sqlite);
    cell->Release();
}
