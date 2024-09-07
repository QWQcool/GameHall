#ifndef _DBSQLITE_H_2022_8_21_
#define _DBSQLITE_H_2022_8_21_
#include <string>
#include <functional>
#include <cstdint>
struct sqlite3;
struct sqlite3_stmt;
namespace X {

	namespace DB
	{

		enum class Sqlite3Enum
		{
			SQLITE_OK = 0,   /* Successful result */
			/* beginning-of-error-codes */
			SQLITE_ERROR = 1,   /* Generic error */
			SQLITE_INTERNAL = 2,   /* Internal logic error in SQLite */
			SQLITE_PERM = 3,   /* Access permission denied */
			SQLITE_ABORT = 4,   /* Callback routine requested an abort */
			SQLITE_BUSY = 5,   /* The database file is locked */
			SQLITE_LOCKED = 6,   /* A table in the database is locked */
			SQLITE_NOMEM = 7,   /* A malloc() failed */
			SQLITE_READONLY = 8,   /* Attempt to write a readonly database */
			SQLITE_INTERRUPT = 9,   /* Operation terminated by sqlite3_interrupt()*/
			SQLITE_IOERR = 10,   /* Some kind of disk I/O error occurred */
			SQLITE_CORRUPT = 11,   /* The database disk image is malformed */
			SQLITE_NOTFOUND = 12,   /* Unknown opcode in sqlite3_file_control() */
			SQLITE_FULL = 13,   /* Insertion failed because database is full */
			SQLITE_CANTOPEN = 14,   /* Unable to open the database file */
			SQLITE_PROTOCOL = 15,   /* Database lock protocol error */
			SQLITE_EMPTY = 16,   /* Internal use only */
			SQLITE_SCHEMA = 17,   /* The database schema changed */
			SQLITE_TOOBIG = 18,   /* String or BLOB exceeds size limit */
			SQLITE_CONSTRAINT = 19,   /* Abort due to constraint violation */
			SQLITE_MISMATCH = 20,   /* Data type mismatch */
			SQLITE_MISUSE = 21,   /* Library used incorrectly */
			SQLITE_NOLFS = 22,   /* Uses OS features not supported on host */
			SQLITE_AUTH = 23,   /* Authorization denied */
			SQLITE_FORMAT = 24,   /* Not used */
			SQLITE_RANGE = 25,   /* 2nd parameter to sqlite3_bind out of range */
			SQLITE_NOTADB = 26,   /* File opened that is not a database file */
			SQLITE_NOTICE = 27,   /* Notifications from sqlite3_log() */
			SQLITE_WARNING = 28,   /* Warnings from sqlite3_log() */
			SQLITE_ROW = 100,  /* sqlite3_step() has another row ready */
			SQLITE_DONE = 101,  /* sqlite3_step() has finished executing */
		};
		//class IDBCell
		//{
		//public:
		//	void virtual DBExec(DBSqlite* pSqlite) = 0;
		////	void virtual Release() = 0;
		//};

		class DBSqlite
		{
		private:
			std::string _path;

			struct sqlite3* _pSqlite3;
			struct sqlite3_stmt* _pStmt; // 预编译 时使用

		public:
			DBSqlite();

			~DBSqlite();

			bool Create(std::string path);

			void Destroy();
			/*
			 * pError: 如果有错误则有返回
			 * callback:
			 *		column_size 数据库的字段数
			 *		column_value 列的值
			 *		column_name 字段名字
			 */
			int Exec(const char* sql, char** pError, std::function<void(int column_size, char* column_value[], char* column_name[])> callback = nullptr);

			// 获取最后一个错误
			void GetLastError(const char** pErr);

			//预编译相关
		public:
			// 创建一个预编译
			bool BeginPrecompiled(const char* sql, char** pError);

			// 关闭与编译 和 BeginPrecompiled是对应的 必须调用
			void EndPrecompiled();

			void PrecompiledBind(int index, int v);
			void PrecompiledBind(int index, int64_t v);
			void PrecompiledBind(int index, uint64_t v);

			void PrecompiledBind(int index, const char* v, int nLen);
			void PrecompiledBind(int index, const void* const  v, int nLen);

			// 执行预编译
			// ret:  SQLITE_DONE  SQLITE_ROW  SQLITE_ERR  SQLITE_BUSY
			// SQLITE_ROW 表示 后续还有内容可读 
			Sqlite3Enum StepPrecompiled();

			// 清空绑定的数据
			void ResetPrecompiled();

			bool PrecompiledGetValue(int index, uint64_t& v);
			bool PrecompiledGetValue(int index, int64_t& v);
			bool PrecompiledGetValue(int index, std::string& v);
			bool PrecompiledGetValue(int index, const void*& v);

			// 获取最后一个自增ID
			uint64_t GetLastInsertRowId();

		public:
			// SELECT 时使用
			int GetColumnCount();

			// 开启事务
			bool Begin();

			// 提交事务
			bool Commit();

			// 事务回滚
			bool Rollback();

		};
	}
}
#endif
