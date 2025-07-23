#ifndef _OUTER_FACTORY_IMP_H_
#define _OUTER_FACTORY_IMP_H_

#include <string>
#include <map>

#include "servant/Application.h"
#include "globe.h"
#include "OuterFactory.h"
#include "util/tc_hash_fun.h"

//wbl
#include <wbl/regex_util.h>

//服务代理
#include "ConfigServant.h"
#include "DBAgentServant.h"
#include "PushServant.h"
#include "Log2DBServant.h"
#include "GlobalServant.h"
#include "GameRecordServant.h"
#include "GameRecordProto.h"
#include "LogComm.h"

//
using namespace config;
using namespace dataproxy;
using namespace dbagent;

//时区
//#define ONE_DAY_TIME (24*60*60)
//#define ZONE_TIME_OFFSET (8*60*60)

//
class OuterFactoryImp;
typedef TC_AutoPtr<OuterFactoryImp> OuterFactoryImpPtr;

//玩家初始化财富
typedef struct _UserInitWealth
{
    long gold;      //金币
    long diamond;   //钻石
    long score;     //积分
    long ticketNum; //奖券
} UserInitWealth;

//平台类型
typedef struct _TPlatformType
{
    map<int, string> mapPlatform;
} PlatformType;

/**
 * 外部工具接口对象工厂
 */
class OuterFactoryImp : public OuterFactory
{
private:
    /**
     *
    */
    OuterFactoryImp();

    /**
     *
    */
    ~OuterFactoryImp();

    //
    friend class HallServantImp;
    friend class HallServer;

public:
    //框架中用到的outer接口(不能修改):
    const OuterProxyFactoryPtr &getProxyFactory() const
    {
        return _pProxyFactory;
    }

    tars::TC_Config &getConfig() const
    {
        return *_pFileConf;
    }

public:
    //加载配置
    void load();

    //代理配置
    void readPrxConfig();
    void printPrxConfig();
    //道具配置
    void readPropsConfig();
    void printPropsConfig();
    //经验配置
    void readExperienceConfig();
    void printExperienceConfig();

public:
    //
    void asyncPushPropsInfo( map<long, push::PropsChangeNotify>& mapNotify);
    //日志入库
    void asyncLog2DB(const int64_t uid, const DaqiGame::TLog2DBReq &req);
    //
    void asyncLog2DB(const int64_t uid, const short type, const std::vector<string> &fields);

private:
    /**
     *
    */
    void createAllObject();

    /**
     *
    */
    void deleteAllObject();

public: // 获取代理
    //游戏配置服务代理
    const ConfigServantPrx getConfigServantPrx();
    //数据库代理服务代理
    const DBAgentServantPrx getDBAgentServantPrx(const int64_t uid);
    //数据库代理服务代理
    const DBAgentServantPrx getDBAgentServantPrx(const string &key);
    //PushServer代理
    const push::PushServantPrx getPushServerPrx(const int64_t uid);
    //日志入库服务代理
    const DaqiGame::Log2DBServantPrx getLog2DBServantPrx(const int64_t uid);
    //全局服务代理
    const global::GlobalServantPrx getGlobalServantPrx(const int64_t uid);
    //
    const gamerecord::GameRecordServantPrx getGameRecordServantPrx(const int64_t uid);

public:
    //拆分字符串成整形
    int splitInt(string szSrc, vector<int> &vecInt);
    //
    int parseProps(string szSrc, map<string, string>& result);
    //
    config::E_Props_Type getPropsTypeById(const int propsId);
    //
    string getPropsColById(const int propsId);
    //
    int getPropsIdByCol(const string colName);
    int getPropsInfoConfig(const int propsId, config::PropsInfoCfg &cfg);
    //
    int getExpMaxLevel();
    //
    int getExpUpgByLevel(const int iLevel);
    //
    int getExpChangeByType(const config::E_EXP_CHANGE_TYPE eChangeType);
    //去除特殊字符
    bool filterUtf8(unsigned char *str, int length);
    //检查字符串长度跟特殊字符
    bool checkStrUtf8(const string &src);
    // 取固定长度字符串
    string subStrUtf8(const string &src, size_t bytes);
    // 计算utf8字符偏移量
    int utf8_char_len(char firstByte);
    // 获得时间秒数
    int getTimeTick(const string &str);

private:
    //读写锁，防止脏读
    wbl::ReadWriteLocker m_rwlock;

private:
    //框架用到的共享对象(不能修改):
    tars::TC_Config *_pFileConf;
    tars::TC_Config *_pFileGoodsList;//道具配置
    OuterProxyFactoryPtr _pProxyFactory;

private: //业务自有的共享对象:
    //配置服务
    std::string _ConfigServantObj;
    ConfigServantPrx _ConfigServerPrx;
    //数据库服务
    std::string _DBAgentServantObj;
    DBAgentServantPrx _DBAgentServerPrx;
    //推送服务
    string _sPushServantObj;
    push::PushServantPrx _pushServerPrx;
    //日志服务
    std::string _Log2DBServantObj;
    DaqiGame::Log2DBServantPrx _Log2DBServerPrx;
    //全局服务
    std::string _GlobalServantObj;
    global::GlobalServantPrx _GlobalServerPrx;
    //
    std::string _GameRecordServantObj;
    gamerecord::GameRecordServantPrx _GameRecordServerPrx;

public:
    config::ListPropsInfoCfgResp mapPropsConfig;

    config::ListExpUpgCfgResp listExpUpgCfgResp;

    config::ListExpChangeCfgResp listExpChangeCfgResp;

};

////////////////////////////////////////////////////////////////////////////////
#endif


