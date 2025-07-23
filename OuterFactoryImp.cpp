#include <sstream>
#include "OuterFactoryImp.h"
#include "LogComm.h"
#include "HallServer.h"

//
using namespace wbl;

OuterFactoryImp::OuterFactoryImp() : _pFileConf(NULL)
{
    createAllObject();
}

OuterFactoryImp::~OuterFactoryImp()
{
    deleteAllObject();
}

void OuterFactoryImp::deleteAllObject()
{
    if (_pFileConf)
    {
        delete _pFileConf;
        _pFileConf = NULL;
    }
}

void OuterFactoryImp::createAllObject()
{
    try
    {
        //
        deleteAllObject();

        //本地配置文件
        _pFileConf = new tars::TC_Config();
        if (NULL == _pFileConf)
        {
            ROLLLOG_ERROR << "create config parser fail, ptr null." << endl;
            terminate();
        }

        // tars代理Factory,访问其他tars接口时使用
        _pProxyFactory = new OuterProxyFactory();
        if ((long int)NULL == _pProxyFactory)
        {
            ROLLLOG_ERROR << "create outer proxy factory fail, ptr null." << endl;
            terminate();
        }

        LOG_DEBUG << "init proxy factory succ." << endl;

        //
        FDLOG_COIN_CHANGE_LOG_FORMAT;
        FDLOG_PROPS_CHANGE_LOG_FORMAT;
        FDLOG_USER_CHANGE_LOG_FORMAT;
        FDLOG_RECHARGE_LOG_FORMAT;

        //加载配置
        load();
    }
    catch (TC_Exception &ex)
    {
        LOG->error() << ex.what() << endl;
        throw;
    }
    catch (exception &e)
    {
        LOG->error() << e.what() << endl;
        throw;
    }
    catch (...)
    {
        LOG->error() << "unknown exception." << endl;
        throw;
    }

    return;
}


//读取所有配置
void OuterFactoryImp::load()
{
    __TRY__

    //拉取远程配置
    g_app.addConfig(ServerConfig::ServerName + ".conf");

    WriteLocker lock(m_rwlock);

    _pFileConf->parseFile(ServerConfig::BasePath + ServerConfig::ServerName + ".conf");
    LOG_DEBUG << "init config file succ : " << ServerConfig::BasePath + ServerConfig::ServerName + ".conf" << endl;

    //代理配置
    readPrxConfig();
    printPrxConfig();

    //道具配置
    readPropsConfig();
    printPropsConfig();

    //经验配置
    readExperienceConfig();
    printExperienceConfig();

    __CATCH__
}

//代理配置
void OuterFactoryImp::readPrxConfig()
{
    _ConfigServantObj = (*_pFileConf).get("/Main/Interface/ConfigServer<ProxyObj>", "");
    _DBAgentServantObj = (*_pFileConf).get("/Main/Interface/DBAgentServer<ProxyObj>", "");
    _sPushServantObj = (*_pFileConf).get("/Main/Interface/PushServer<ProxyObj>", "");
    _Log2DBServantObj = (*_pFileConf).get("/Main/Interface/Log2DBServer<ProxyObj>", "");
    _GlobalServantObj = (*_pFileConf).get("/Main/Interface/GlobalServer<ProxyObj>", "");
    _GameRecordServantObj = (*_pFileConf).get("/Main/Interface/GameRecordServer<ProxyObj>", "");

}

void OuterFactoryImp::printPrxConfig()
{
    FDLOG_CONFIG_INFO << "_ConfigServantObj ProxyObj : " << _ConfigServantObj << endl;
    FDLOG_CONFIG_INFO << "_DBAgentServantObj ProxyObj : " << _DBAgentServantObj << endl;
    FDLOG_CONFIG_INFO << "_sPushServantObj ProxyObj: "      << _sPushServantObj << endl;
    FDLOG_CONFIG_INFO << "_Log2DBServantObj ProxyObj : " << _Log2DBServantObj << endl;
    FDLOG_CONFIG_INFO << "_GlobalServantObj ProxyObj : " << _GlobalServantObj << endl;
    FDLOG_CONFIG_INFO << "_GameRecordServantObj ProxyObj : " << _GameRecordServantObj << endl;
}

void OuterFactoryImp::readPropsConfig()
{
    auto pConfigServantPrx = getConfigServantPrx();
    if(!pConfigServantPrx)
    {
        LOG_ERROR << "pConfigServantPrx is nullptr "<<endl;
        return ;
    }

    if(pConfigServantPrx->listProps(mapPropsConfig) != 0)
    {
        LOG_ERROR << "read props err."<< endl;
        return;
    }

    return;
}

void OuterFactoryImp::printPropsConfig()
{
    FDLOG_CONFIG_INFO << "Props Config Size: " << mapPropsConfig.data.size() << endl;
}

void OuterFactoryImp::readExperienceConfig()
{
    auto pConfigServantPrx = getConfigServantPrx();
    if(!pConfigServantPrx)
    {
        LOG_ERROR << "pConfigServantPrx is nullptr "<<endl;
        return ;
    }

    if(pConfigServantPrx->listExpUpg(listExpUpgCfgResp) != 0)
    {
        LOG_ERROR << "read experience upgrade err."<< endl;
        return;
    }

    if(pConfigServantPrx->listExpChange(listExpChangeCfgResp) != 0)
    {
        LOG_ERROR << "read experience change err."<< endl;
        return;
    }

    return;
}

void OuterFactoryImp::printExperienceConfig()
{
    FDLOG_CONFIG_INFO << "Experience Upgrade Config Size: " << listExpUpgCfgResp.data.size() << endl;
    FDLOG_CONFIG_INFO << "Experience Change Config Size: " << listExpChangeCfgResp.data.size() << endl;
}

//游戏配置服务代理
const ConfigServantPrx OuterFactoryImp::getConfigServantPrx()
{
    if (!_ConfigServerPrx)
    {
        _ConfigServerPrx = Application::getCommunicator()->stringToProxy<ConfigServantPrx>(_ConfigServantObj);
        ROLLLOG_DEBUG << "Init _ConfigServantObj succ, _ConfigServantObj : " << _ConfigServantObj << endl;
    }

    return _ConfigServerPrx;
}

//数据库服务代理
const DBAgentServantPrx OuterFactoryImp::getDBAgentServantPrx(const int64_t uid)
{
    if (!_DBAgentServerPrx)
    {
        _DBAgentServerPrx = Application::getCommunicator()->stringToProxy<dbagent::DBAgentServantPrx>(_DBAgentServantObj);
        ROLLLOG_DEBUG << "Init _DBAgentServantObj succ, _DBAgentServantObj : " << _DBAgentServantObj << endl;
    }

    if (_DBAgentServerPrx)
    {
        return _DBAgentServerPrx->tars_hash(uid);
    }

    return NULL;
}

//数据库服务代理
const DBAgentServantPrx OuterFactoryImp::getDBAgentServantPrx(const string &key)
{
    if (!_DBAgentServerPrx)
    {
        _DBAgentServerPrx = Application::getCommunicator()->stringToProxy<dbagent::DBAgentServantPrx>(_DBAgentServantObj);
        ROLLLOG_DEBUG << "Init _DBAgentServantObj succ, _DBAgentServantObj : " << _DBAgentServantObj << endl;
    }

    if (_DBAgentServerPrx)
    {
        return _DBAgentServerPrx->tars_hash(tars::hash<string>()(key));
    }

    return NULL;
}

//PushServer代理
const push::PushServantPrx OuterFactoryImp::getPushServerPrx(const int64_t uid)
{
    if (!_pushServerPrx)
    {
        _pushServerPrx = Application::getCommunicator()->stringToProxy<push::PushServantPrx>(_sPushServantObj);
        LOG_DEBUG << "Init _sPushServantObj succ, _sPushServantObj: " << _sPushServantObj << endl;
    }

    if (_pushServerPrx)
    {
        return _pushServerPrx->tars_hash(uid);
    }

    return NULL;
}

//日志入库服务代理
const DaqiGame::Log2DBServantPrx OuterFactoryImp::getLog2DBServantPrx(const int64_t uid)
{
    if (!_Log2DBServerPrx)
    {
        _Log2DBServerPrx = Application::getCommunicator()->stringToProxy<DaqiGame::Log2DBServantPrx>(_Log2DBServantObj);
        ROLLLOG_DEBUG << "Init _Log2DBServantObj succ, _Log2DBServantObj : " << _Log2DBServantObj << endl;
    }

    if (_Log2DBServerPrx)
    {
        return _Log2DBServerPrx->tars_hash(uid);
    }

    return NULL;
}

//全局服务代理
const global::GlobalServantPrx OuterFactoryImp::getGlobalServantPrx(const int64_t uid)
{
    if (!_GlobalServerPrx)
    {
        _GlobalServerPrx = Application::getCommunicator()->stringToProxy<global::GlobalServantPrx>(_GlobalServantObj);
        ROLLLOG_DEBUG << "Init _GlobalServantObj succ, _GlobalServantObj : " << _GlobalServantObj << endl;
    }

    if (_GlobalServerPrx)
    {
        return _GlobalServerPrx->tars_hash(uid);
    }

    return NULL;
}

const gamerecord::GameRecordServantPrx OuterFactoryImp::getGameRecordServantPrx(const int64_t uid)
{
    if (!_GameRecordServerPrx)
    {
        _GameRecordServerPrx = Application::getCommunicator()->stringToProxy<gamerecord::GameRecordServantPrx>(_GameRecordServantObj);
        ROLLLOG_DEBUG << "Init _GameRecordServantObj succ, _GameRecordServantObj: " << _GameRecordServantObj << endl;
    }
    
    if (_GameRecordServerPrx)
    {
        return _GameRecordServerPrx->tars_hash(uid);
    }

    return NULL;
}

//
void OuterFactoryImp::asyncPushPropsInfo(map<long, push::PropsChangeNotify>& mapNotify)
{
    for(auto notify : mapNotify)
    {
        push::PushMsgReq pushMsgReq;
        push::PushMsg pushMsg;
        pushMsg.uid = notify.first;
        pushMsg.msgType = push::E_PUSH_MSG_TYPE_PROPS_CHANGE;
        pushMsg.changeType = 0;

        tobuffer(notify.second, pushMsg.vecData);
        pushMsgReq.msg.push_back(pushMsg);

        getPushServerPrx(notify.first)->async_pushMsg(NULL, pushMsgReq);

    }
}

//日志入库
void OuterFactoryImp::asyncLog2DB(const int64_t uid, const DaqiGame::TLog2DBReq &req)
{
    getLog2DBServantPrx(uid)->async_log2db(NULL, req);
}

//日志入库
void OuterFactoryImp::asyncLog2DB(const int64_t uid, const short type, const std::vector<string> &fields)
{
    DaqiGame::TLog2DBReq req;
    req.sLogType = type;
    req.vecLogData.push_back(fields);
    getLog2DBServantPrx(uid)->async_log2db(NULL, req);
}

//拆分字符串成整形
int OuterFactoryImp::splitInt(string szSrc, vector<int> &vecInt)
{
    split_int(szSrc, "[ \t]*\\|[ \t]*", vecInt);
    return 0;
}

/*
*   解析字符串
*   propsid:propscont;propsid:propscon
*/

int OuterFactoryImp::parseProps(string szSrc, map<string, string>& result)
{
    vector<string> vecItem = SEPSTR(szSrc, ";");
    for(auto item : vecItem)
    {
        vector<string> vecValue = SEPSTR(item, ":");
        if(vecValue.size() != 2)
        {
            continue;
        }
        result.insert(std::make_pair(vecValue[0], vecValue[1]));
    }
    return 0;
}

config::E_Props_Type OuterFactoryImp::getPropsTypeById(const int propsId)
{
    auto it = mapPropsConfig.data.find(propsId);
    if(it == mapPropsConfig.data.end())
    {
        return config::E_Props_Type::E_PROPS_TYPE_NONE;
    }
    return config::E_Props_Type(it->second.propsType);
}

string OuterFactoryImp::getPropsColById(const int propsId)
{
    auto it = mapPropsConfig.data.find(propsId);
    if(it == mapPropsConfig.data.end())
    {
        return "";
    }
    return it->second.propsCol;
}

int OuterFactoryImp::getPropsIdByCol(const string colName)
{
    for(auto item : mapPropsConfig.data)
    {
        if(item.second.propsCol == colName)
        {
            return item.first;
        }
    }
    return 0;
}

int OuterFactoryImp::getPropsInfoConfig(const int propsId, config::PropsInfoCfg &cfg)
{
    auto it = mapPropsConfig.data.find(propsId);
    if(it != mapPropsConfig.data.end())
    {
        cfg = it->second;
        return 0;
    }
    return -1;
}

int OuterFactoryImp::getExpMaxLevel()
{
    return listExpUpgCfgResp.iMaxLevel;
}

int OuterFactoryImp::getExpUpgByLevel(const int iLevel)
{
    auto it = listExpUpgCfgResp.data.find(iLevel);
    if(it == listExpUpgCfgResp.data.end())
    {
        return 0;
    }
    return it->second;
}

//
int OuterFactoryImp::getExpChangeByType(const config::E_EXP_CHANGE_TYPE eChangeType)
{
    auto it = listExpChangeCfgResp.data.find(eChangeType);
    if(it == listExpChangeCfgResp.data.end())
    {
        return 0;
    }
    return it->second;
}

//去除特殊字符
bool OuterFactoryImp::filterUtf8(unsigned char *str, int length)
{
    bool bRet = false;
    if (str == NULL || length == 0)
    {
        return bRet;
    }

    //计数
    int i = 0;
    for (; i < length; )
    {
        //offset
        int offset = utf8_char_len(str[i]);
        if (offset == 1)
        {
            if ((0x41 <= str[i] && str[i] <= 0x5a) || (0x30 <= str[i] && str[i] <= 0x39) || (0x61 <= str[i] && str[i] <= 0x7a) )
            {
                ;
            }
            else
            {
                return true;
            }
        }

        i += offset;
    }

    return bRet;
}

bool OuterFactoryImp::checkStrUtf8(const string &src)
{
    return filterUtf8((unsigned char *)src.c_str(), src.length());
}

// 取固定长度字符串
string OuterFactoryImp::subStrUtf8(const string &src, size_t bytes)
{
    string strRet = "";
    if (src.length() < 0 || bytes <= 0)
        return strRet;

    //计数
    size_t i = 0;
    for (; (i < src.length()); )
    {
        int offset = utf8_char_len(src[i]);
        if (i + offset > bytes)
            break;

        i += offset;
    }

    strRet.assign(src, 0, i);
    return strRet;
}

//
const unsigned char kFirstBitMask   = 128;  // 1000000
const unsigned char kSecondBitMask  = 64;   // 0100000
const unsigned char kThirdBitMask   = 32;   // 0010000
const unsigned char kFourthBitMask  = 16;   // 0001000
const unsigned char kFifthBitMask   = 8;    // 0000100

// 计算utf8字符偏移量
int OuterFactoryImp::utf8_char_len(char firstByte)
{
    std::string::difference_type offset = 1;
    // This means the first byte has a value greater than 127, and so is beyond the ASCII range.
    if (firstByte & kFirstBitMask)
    {
        // This means that the first byte has a value greater than 224, and so it must be at least a three-octet code point.
        if (firstByte & kThirdBitMask)
        {
            // This means that the first byte has a value greater than 240, and so it must be a four-octet code point.
            if (firstByte & kFourthBitMask)
                offset = 4;
            else
                offset = 3;
        }
        else
        {
            offset = 2;
        }
    }

    return offset;
}

// 获得时间秒数
int OuterFactoryImp::getTimeTick(const string &str)
{
    if (str.empty())
        return 0;

    struct tm tm_time;
    string sFormat("%Y-%m-%d %H:%M:%S");
    strptime(str.c_str(), sFormat.c_str(), &tm_time);
    return mktime(&tm_time);
}
////////////////////////////////////////////////////////////////////////////////

