#ifndef _connection_h_
#define _connection_h_

#include <v8.h>
#include <node.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <occi.h>
#include <oro.h>
#include "utils.h"
#include "nodeOracleException.h"
#include "executeBaton.h"

using namespace node;
using namespace v8;

/**
 * Wrapper for an OCCI Connection class so that it can be used in JavaScript
 */
class Connection: ObjectWrap {
public:
  static Persistent<FunctionTemplate> s_ct;
  static void Init(Handle<Object> target);
  static Handle<Value> New(const Arguments& args);

  // asynchronous execute method
  static Handle<Value> Execute(const Arguments& args);
  static void EIO_Execute(uv_work_t* req);
  static void EIO_AfterExecute(uv_work_t* req, int status);

  // synchronous execute method
  static Handle<Value> ExecuteSync(const Arguments& args);

  // asynchronous commit method
  static Handle<Value> Commit(const Arguments& args);
  static void EIO_Commit(uv_work_t* req);
  static void EIO_AfterCommit(uv_work_t* req, int status);

  // asynchronous rollback method
  static Handle<Value> Rollback(const Arguments& args);
  static void EIO_Rollback(uv_work_t* req);
  static void EIO_AfterRollback(uv_work_t* req, int status);

  static Handle<Value> SetAutoCommit(const Arguments& args);
  static Handle<Value> SetPrefetchRowCount(const Arguments& args);
  static Handle<Value> IsConnected(const Arguments& args);

  static Handle<Value> Close(const Arguments& args);

  void closeConnection();

  Connection();
  ~Connection();

  void setConnection(oracle::occi::Environment* environment,
      oracle::occi::StatelessConnectionPool* connectionPool,
      oracle::occi::Connection* connection);

  oracle::occi::Environment* getEnvironment() {
    return m_environment;
  }

private:
  static int SetValuesOnStatement(oracle::occi::Statement* stmt,
      std::vector<value_t*> &values);
  static void CreateColumnsFromResultSet(oracle::occi::ResultSet* rs,
      std::vector<column_t*> &columns);
  static row_t* CreateRowFromCurrentResultSetRow(oracle::occi::ResultSet* rs,
      std::vector<column_t*> &columns);
  static Local<Array> CreateV8ArrayFromRows(std::vector<column_t*> columns,
      std::vector<row_t*>* rows);
  static Local<Object> CreateV8ObjectFromRow(std::vector<column_t*> columns,
      row_t* currentRow);
  static void handleResult(ExecuteBaton* baton, Handle<Value> (&argv)[2]);

  // The OOCI environment
  oracle::occi::Environment* m_environment;

  // The underlying connection pool
  oracle::occi::StatelessConnectionPool* m_connectionPool;

  // The underlying connection
  oracle::occi::Connection* m_connection;

  // Autocommit flag
  bool m_autoCommit;

  // Prefetch row count
  int m_prefetchRowCount;
};

/**
 * Wrapper for an OCCI StatelessConnectionPool class so that it can be used in JavaScript
 */
class ConnectionPool: ObjectWrap {
public:
  static Persistent<FunctionTemplate> s_ct;

  static void Init(Handle<Object> target);
  static Handle<Value> New(const Arguments& args);
  static Handle<Value> GetInfo(const Arguments& args);
  static Handle<Value> Close(const Arguments& args);

  static Handle<Value> GetConnection(const Arguments& args);
  static void EIO_GetConnection(uv_work_t* req);
  static void EIO_AfterGetConnection(uv_work_t* req, int status);

  static Handle<Value> GetConnectionSync(const Arguments& args);

  void closeConnectionPool(
      oracle::occi::StatelessConnectionPool::DestroyMode mode =
          oracle::occi::StatelessConnectionPool::DEFAULT);

  ConnectionPool();
  ~ConnectionPool();

  void setConnectionPool(oracle::occi::Environment* environment,
      oracle::occi::StatelessConnectionPool* connectionPool);
  oracle::occi::Environment* getEnvironment() {
    return m_environment;
  }
  oracle::occi::StatelessConnectionPool* getConnectionPool() {
    return m_connectionPool;
  }

private:
  oracle::occi::Environment* m_environment;
  oracle::occi::StatelessConnectionPool* m_connectionPool;
};

/**
 * Baton for ConnectionPool
 */
class ConnectionPoolBaton {
public:
  ConnectionPoolBaton(oracle::occi::Environment* environment,
      ConnectionPool* connectionPool, v8::Handle<v8::Function>* callback) {
    this->environment = environment;
    this->connectionPool = connectionPool;
    this->callback = Persistent<Function>::New(*callback);
    this->connection = NULL;
    this->error = NULL;
  }

  ~ConnectionPoolBaton() {
    callback.Dispose();
  }

  oracle::occi::Environment* environment;
  ConnectionPool *connectionPool;
  oracle::occi::Connection* connection;
  v8::Persistent<v8::Function> callback;

  std::string* error;

};

#endif
