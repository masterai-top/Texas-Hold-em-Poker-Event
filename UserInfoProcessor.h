#ifndef _UserInfo_Processor_H_
#define _UserInfo_Processor_H_

//
#include <util/tc_singleton.h>
#include "UserInfoProto.h"
#include "DBAgentProto.h"
#include "DataProxyProto.h"
#include "login.pb.h"
#include "UserInfo.pb.h"

#include "ConfigProto.h"

//
using namespace tars;
using namespace std;
using namespace userinfo;


#define MAX_NICKNAME_LEN    6
#define MAX_SIGNATURE_LEN   40   //最大签名长度
#define REMARK_WORD_COUNT   40   //备注最大长度

/**
 *UserInfo请求处理类
 *
 */
class UserInfoProcessor
{
public:
    /**
     *
    */
    UserInfoProcessor();

    /**
     *
    */
    ~UserInfoProcessor();

public:
    //初始化用户信息
    tars::Int32 initUser(const userinfo::InitUserReq &req, userinfo::InitUserResp &resp);
    //USER_INFO    = 21,     //#tb_user_info
    //查询
    int selectUserInfo(long uid, UserInfo &userinfo);
    //更新
    int updateUserInfo(long uid, const map<string, string> &updateInfo, UserInfo &mUserInfo, bool bInsert = false);
    //
    int updateUserWealth(long uid, const map<string, int> &updateInfo, long * curCount);
    //
    int selectUserWealth(long uid, const string &colName);
   
    //USER_ACCOUNT = 20,     //#tb_user_account
    //查询
    int selectUserAccount(long uid, UserAccount &useraccount);
    //
    int updateUserAccount(long uid, const map<string, string> &updateInfo, UserAccount &mUserAccount, bool bInsert = false);

    //USER_ADDRESS = 22
    // 查询
    int selectUserAddress(long uid, vector<UserAddress> &vAddress);
    // 重置状态
    int resetUserAddress(long uid);
    // 删除
    int deleteUserAddress(long gid);
    // 更新(新增)
    int updateUserAddress(long gid, const map<string, string> &updateAddress, bool bInsert = false);

    //道具相关api
    //
    int selectUserProps(long uid, vector<userinfo::PropsInfo> &vecPropsInfo);
    //
    int selectUserPropsByUUID(long uid, const string& uuid, userinfo::PropsInfo &propsInfo);
    //
    int selectUserPropsByID(long uid, const int propsId, vector<userinfo::PropsInfo> &vecPropsInfo);
    //
    int insertUserProps(long uid, const userinfo::PropsInfo &propsInfo);
    //
    int updateUserPropsByUUID(long uid, const string& uuid, const map<string, string> &updateInfo);
    //
    int updateUserPropsByID(long uid, const int propsId, const int count, const map<string, string> &updateInfo);
    //
    int getPropsCountById(long uid, int propsId);

    //玩家备注
    int updateUserRemark(long uid, long remark_uid, const string& content);
    //
    string selectUserRemark(long uid, long remark_uid);

    int listUserRemark(long uid, vector<userinfo::UserRemarkInfo> &vecRemarkInfo);
};

//singleton
typedef TC_Singleton<UserInfoProcessor, CreateStatic, DefaultLifetime> UserInfoProcessorSingleton;

#endif

