#include "DBSqlite.h"
#include "sqlite3.h"
#include <cstring>


X::DB::DBSqlite::DBSqlite()
{
	_pSqlite3 = nullptr;
	_pStmt = nullptr;
}

X::DB::DBSqlite::~DBSqlite()
{
	Destroy();
}

bool X::DB::DBSqlite::Create(std::string path)
{
	_path = path;
	int ret = sqlite3_open(_path.c_str(), &_pSqlite3);
	return SQLITE_OK == ret;
}

void X::DB::DBSqlite::Destroy()
{
	if (_pSqlite3)
		sqlite3_close(_pSqlite3);
	_pSqlite3 = nullptr;
}


struct CallBack
{
	std::function<void(int column_size, char* column_value[], char* column_name[])> func;
};
static int _CallBack(void* data, int column_size, char* column_value[], char* column_name[])
{
	CallBack* CallBack_ = (CallBack*)data;
	CallBack_->func(column_size, column_value, column_name);
	return 0;
}

int X::DB::DBSqlite::Exec(const char* sql, char** pError, std::function<void(int column_size, char* column_value[], char* column_name[])> callback /*= nullptr*/)
{
	CallBack CallBack_;
	CallBack_.func = callback;
	//sqlite3*            : open 打开的数据库
	//const char* sql,    : 执行的sql功能语句
	//*callback,          : sql语句对应的回调函数
	//void* data,         : 传递给回调函数的 指针参数
	//char **errmsq       : 错误信息
	int nRet = sqlite3_exec(_pSqlite3, sql, &_CallBack, (void*)&CallBack_, pError);

	return nRet;
}

void X::DB::DBSqlite::GetLastError(const char** pErr)
{
	*pErr = sqlite3_errmsg(_pSqlite3);
}

bool X::DB::DBSqlite::BeginPrecompiled(const char* sql, char** pError)
{

	if (_pStmt) EndPrecompiled();

	// 生成预编译语句
	int nRet = sqlite3_prepare_v2(_pSqlite3, sql, (int)strlen(sql), &_pStmt, (const char**)pError);
	if (SQLITE_OK != nRet)
	{
		sqlite3_finalize(_pStmt); // 利用 sqlite3_prepare_v2 生成预编译语句 后必须调用的
		_pStmt = nullptr;
		return false;
	}
	return true;
}

void X::DB::DBSqlite::EndPrecompiled()
{
	if (_pStmt)
	{
		sqlite3_finalize(_pStmt); // 利用 sqlite3_prepare_v2 生成预编译语句 后必须调用的
		_pStmt = nullptr;
	}
}

void X::DB::DBSqlite::PrecompiledBind(int index, int v)
{
	sqlite3_bind_int(_pStmt, index, v);

}


void X::DB::DBSqlite::PrecompiledBind(int index, int64_t v)

{
	sqlite3_bind_int64(_pStmt, index, v);
}

void X::DB::DBSqlite::PrecompiledBind(int index, uint64_t v)
{
	PrecompiledBind(index, (int64_t)v);
}

void X::DB::DBSqlite::PrecompiledBind(int index, const char* v, int nLen)
{
	sqlite3_bind_text(_pStmt, index, v, nLen, NULL);
}

void X::DB::DBSqlite::PrecompiledBind(int index, const void* const v, int nLen)
{
	sqlite3_bind_blob(_pStmt, index, v, nLen, SQLITE_STATIC);
}

X::DB::Sqlite3Enum X::DB::DBSqlite::StepPrecompiled()
{
	int status = sqlite3_step(_pStmt);
	return X::DB::Sqlite3Enum(status); // 提交语句
}

void X::DB::DBSqlite::ResetPrecompiled()
{
	sqlite3_reset(_pStmt); // 清空绑定的数据
}

bool X::DB::DBSqlite::PrecompiledGetValue(int index, uint64_t& v)
{
	v = sqlite3_column_int64(_pStmt, index);
	return true;
}

bool X::DB::DBSqlite::PrecompiledGetValue(int index, int64_t& v)
{
	v = sqlite3_column_int64(_pStmt, index);
	return true;
}
bool X::DB::DBSqlite::PrecompiledGetValue(int index, std::string& v)
{
	v = (char*)sqlite3_column_text(_pStmt, index);
	return true;
}

bool X::DB::DBSqlite::PrecompiledGetValue(int index, const void*& v)
{
	v = sqlite3_column_blob(_pStmt, index);
	return v != NULL;
}

uint64_t X::DB::DBSqlite::GetLastInsertRowId()
{
	// const char* const table
	//std::string sql = "select last_insert_rowid() from ";
	//sql += table;
	//char* pErr = NULL;
	//uint64_t id = 0;
	//Exec(sql.c_str(), &pErr, [&id](int column_size, char* column_value[], char* column_name[])
	//	{
	//		id = sqlite3_int64()
	//	});
	return sqlite3_last_insert_rowid(_pSqlite3);
}

int X::DB::DBSqlite::GetColumnCount()
{
	int nRet = 0;
	if (_pStmt)
		nRet = sqlite3_column_count(_pStmt);
	return nRet;
}

bool X::DB::DBSqlite::Begin()
{
	char* pError = nullptr;
	return Exec("BEGIN TRANSACTION;", &pError);
}

bool X::DB::DBSqlite::Commit()
{
	char* pError = nullptr;
	return Exec("commit;", &pError);
}

bool X::DB::DBSqlite::Rollback()
{
	char* pError = nullptr;
	return Exec("rollback;", &pError);
}
