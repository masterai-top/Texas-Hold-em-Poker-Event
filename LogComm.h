#ifndef __HALL_LOGCOMM_H__
#define __HALL_LOGCOMM_H__

//
#include <util/tc_logger.h>
#include "servant/RemoteLogger.h"

//
using namespace tars;

//
#define ROLLLOG(level) (LOG->level() << "[" << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "] ")
#define ROLLLOG_DEBUG (ROLLLOG(debug))
#define ROLLLOG_INFO (ROLLLOG(info))
#define ROLLLOG_WARN (ROLLLOG(warn))
#define ROLLLOG_ERROR (ROLLLOG(error))

#define FUNC_ENTRY(in) (ROLLLOG(debug) << ">>>> Enter " << __FUNCTION__ << "() in(" << in << ")" << endl)
#define FUNC_EXIT(out, ret) (ROLLLOG(debug) << "<<<< Exit " << __FUNCTION__ << "() out[" << out << "], ret = " << ret << endl)

#define FDLOG_ERROR (FDLOG("error") << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|")
#define FDLOG_EXCEPT (FDLOG("except") << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|")

//配置信息
#define FDLOG_CONFIG_INFO (FDLOG("config_info") << "|")

//
#define TERMINATE_APPLICATION() {sleep(10);terminate();}

//金币变化日志
#define COIN_CHANGE_LOG_TOPIC "coin_change_log"
#define FDLOG_COIN_CHANGE_LOG (FDLOG("coin_change_log") << "|")

//物品变化日志
#define PROPS_CHANGE_LOG_TOPIC "props_change_log"
#define FDLOG_PROPS_CHANGE_LOG (FDLOG("props_change_log") << "|")

//积分变化日志
#define POINT_CHANGE_LOG_TOPIC "point_change_log"
#define FDLOG_POINT_CHANGE_LOG (FDLOG("point_change_log") << "|")

//玩家基础信息日志
#define USER_CHANGE_LOG_TOPIC "user_change_log"
#define FDLOG_USER_CHANGE_LOG (FDLOG("user_change_log") << "|")

//保险箱变化日志
#define SAFES_CHANGE_LOG_TOPIC "safes_change_log"
#define FDLOG_SAFES_CHANGE_LOG (FDLOG("safes_change_log") << "|")

//
#define FDLOG_INIT_FORMAT(x,y,z) (TarsTimeLogger::getInstance()->initFormatWithType<LogByMinute>(x,y,z))
#define FDLOG_COIN_CHANGE_LOG_FORMAT (FDLOG_INIT_FORMAT("coin_change_log", "%Y%m%d%H%M", 5))
#define FDLOG_PROPS_CHANGE_LOG_FORMAT (FDLOG_INIT_FORMAT("props_change_log", "%Y%m%d%H%M", 5))
#define FDLOG_USER_CHANGE_LOG_FORMAT (FDLOG_INIT_FORMAT("user_change_log", "%Y%m%d%H%M", 5))

//取收件箱数据请求
#define FDLOG_GET_MAIL_DATA_INFO (FDLOG("get_mail_data_info") << "|")
//发送邮件请求
#define FDLOG_SEND_MAIL_INFO (FDLOG("send_mail_info") << "|")
//收邮件请求
#define FDLOG_RECEIVE_MAIL_INFO (FDLOG("receive_mail_info") << "|")
//赠送货币日志
#define FDLOG_AWARDS_PRIZES_INFO (FDLOG("awards_prizes_info") << "|")

//充值日志
#define RECHARGE_LOG_TOPIC "recharge_log"
#define FDLOG_RECHARGE_LOG (FDLOG("recharge_log") << "|")
#define FDLOG_RECHARGE_LOG_FORMAT (FDLOG_INIT_FORMAT("recharge_log", "%Y%m%d%H%M", 5))

template<typename T>
std::string toString(T t) 
{
	ostringstream os;
	os << t;
	return os.str();
}

template<typename T, typename... Args>
std::string toString(T head, Args... args) 
{
	ostringstream os;
	os << head;
	return os.str() + toString(args...);
}

////
#define THROW_LOGIC_ERROR(...) toString("[", __FILE__, ":" , __LINE__ , ":" , __FUNCTION__ , "] ", __VA_ARGS__)

//接口性能边界值
#define COST_MS 100

//函数调用消耗时间
#define FUNC_COST_MS(consumStartMs) { long __costTime__ = (TNOWMS) - (consumStartMs);\
 if (__costTime__ > 10) {ROLLLOG_WARN << "scheduler consumTime: " << __costTime__ << endl;} }

#endif

