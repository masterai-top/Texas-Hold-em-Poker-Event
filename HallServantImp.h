#ifndef _HallServantImp_H_
#define _HallServantImp_H_

//
#include "servant/Application.h"
#include "HallServant.h"
#include "XGameComm.pb.h"
#include "CommonCode.pb.h"
#include "CommonStruct.pb.h"
#include "Hall.pb.h"
#include "UserInfo.pb.h"
#include "GameRecord.pb.h"
#include "OuterFactoryImp.h"
#include "UserInfoProcessor.h"
#include "RoomProcessor.h"

//
namespace XGameComm
{
    class TPackage;
}

//
namespace XGameProto
{
    enum ActionName : int;
}

/**
 *大厅接口
 */
class HallServantImp : public hall::HallServant
{
public:
    /**
     *
     */
    HallServantImp() {}

    /**
     *
     */
    virtual ~HallServantImp() {}

    /**
     *
     */
    virtual void initialize();

    /**
     *
     */
    virtual void destroy();

public:
    //http请求处理接口
    virtual tars::Int32 doRequest(const vector<tars::Char> &reqBuf, const map<std::string, std::string> &extraInfo,  vector<tars::Char> &rspBuf, tars::TarsCurrentPtr current);
    //tcp请求处理接口
    virtual tars::Int32 onRequest(tars::Int64 lUin, const std::string &sMsgPack, const std::string &sCurServrantAddr, const JFGame::TClientParam &stClientParam, const JFGame::UserBaseInfoExt &stUserBaseInfo, tars::TarsCurrentPtr current);
    //获取用户基本信息
    virtual tars::Int32 getUserBasic(const userinfo::GetUserBasicReq &req, userinfo::GetUserBasicResp &resp, tars::TarsCurrentPtr current);
    //获取用户账户
    virtual tars::Int32 getUserAccount(const userinfo::GetUserAccountReq &req, userinfo::GetUserAccountResp &resp, tars::TarsCurrentPtr current);
    //更新用户信息
    virtual tars::Int32 UpdateUserInfo(const userinfo::UpdateUserInfoReq &req, userinfo::UpdateUserInfoResp &resp, tars::TarsCurrentPtr current);
    //添加用户
    virtual tars::Int32 createUser(const userinfo::InitUserReq &req, userinfo::InitUserResp &resp, tars::TarsCurrentPtr current);
    //修改用户账户
    virtual tars::Int32 updateUserAccount(const userinfo::UpdateUserAccountReq &req, userinfo::UpdateUserAccountResp &resp, tars::TarsCurrentPtr current);
    //更新道具(背包道具, 账户道具)
    virtual tars::Int32 modifyUserProps(const userinfo::ModifyUserPropsReq &req, userinfo::ModifyUserPropsResp &resp, tars::TarsCurrentPtr current);
    //获取指定道具数量(背包道具, 账户道具)
    virtual tars::Int32  getUserPropsById(long lUin, int propsId, tars::TarsCurrentPtr current);
    //创建房间
    virtual tars::Int32 createMatchRoom(const room::createMatchRoomReq &req, room::createMatchRoomResp &resp, tars::TarsCurrentPtr current);
    //更新房间
    virtual tars::Int32 updateMatchRoom(const room::updateMatchRoomReq &req, room::updateMatchRoomResp &resp, tars::TarsCurrentPtr current);
    //更新房间成员
    virtual tars::Int32 updateMatchRoomMember(const room::updateRoomMemberReq &req, room::updateRoomMemberResp &resp, tars::TarsCurrentPtr current);
    //删除成员
    virtual tars::Int32 deleteMatchRoomMember(const string &sRoomIndex, tars::TarsCurrentPtr current);
    //获取房间列表
    virtual tars::Int32 selectMatchRoomList(const room::selectMatchRoomListReq &req, room::selectMatchRoomListResp &resp, tars::TarsCurrentPtr current);
    //更新玩家经验
    virtual tars::Int32 updateUserExp(long lUid, config::E_EXP_CHANGE_TYPE eChangeType, tars::TarsCurrentPtr current);

public:
    int matchRoomList(const long lPlayerID, const HallProto::MatchRoomListReq &req, HallProto::MatchRoomListResp &resp);

public:
    //获取用户基本信息
    int onGetUserBasic(const XGameComm::TPackage &pkg, const UserInfoProto::GetUserBasicReq &getUserBasicReq, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    //批量获取用户基本信息
    int onListUserBasic(const XGameComm::TPackage &pkg, const UserInfoProto::ListUserBasicReq &listUserBasicReq, bool getfriend, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    //更新用户信息
    int onUpdateUserInfo(const XGameComm::TPackage &pkg, const UserInfoProto::UpdateUserInfoReq &updateUserInfoReq, const JFGame::UserBaseInfoExt &stUserBaseInfo, const std::string &sCurServrantAddr);
    // 获取比赛列表
    int onMatchRoomList(const XGameComm::TPackage &pkg, const HallProto::MatchRoomListReq &req, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    // 获取用户地址
    int onListUserAddress(const XGameComm::TPackage &pkg, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    // 增加用户地址
    int onAddUserAddress(const XGameComm::TPackage &pkg, const UserInfoProto::AddUserAddressReq &addUserAddressReq, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    // 更新用户地址
    int onUpdateUserAddress(const XGameComm::TPackage &pkg, const UserInfoProto::UpdateUserAddressReq &updateUserAddressReq, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    // 删除用户地址
    int onDeleteUserAddress(const XGameComm::TPackage &pkg, const UserInfoProto::DeleteUserAddressReq &deleteUserAddressReq, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    // 安全码校验
    int onAuthUserSafePwd(const XGameComm::TPackage &pkg, const UserInfoProto::AuthSafePwdReq &req, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    // 获取用户卡包
    int onListUserProps(const XGameComm::TPackage &pkg, const UserInfoProto::ListUserPropsReq &req, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    //
    int onUpdateUserRemark(const XGameComm::TPackage &pkg, const UserInfoProto::UpdateUserRemarkReq &req, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    // 查看资格卡信息
    int onShowPropInfo(const XGameComm::TPackage &pkg, const UserInfoProto::ShowPropInfoReq &req, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    // 资格卡登记使用
    int onMarkProInfo(const XGameComm::TPackage &pkg, const UserInfoProto::MarkPropInfoReq &req, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);
    // 我的备注列表
    int onListUserRemark(const XGameComm::TPackage &pkg, const std::string &sCurServrantAddr, tars::TarsCurrentPtr current);

public:
    //时钟周期回调
    tars::Int32 doCustomMessage(bool bExpectIdle = false);

private:
    //个人详细信息
    int getUserBasic(long uid, UserInfoProto::GetUserBasicResp &getUserBasicResp);
    //发送消息到客户端
    template<typename T>
    int toClientPb(const XGameComm::TPackage &tPackage, const std::string &sCurServrantAddr, XGameProto::ActionName actionName, XGameComm::MSGTYPE type, int serviceType, const T &t);
    //发送消息到客户端
    template<typename T>
    int toClientPb(const XGameComm::TPackage &tPackage, const std::string &sCurServrantAddr, XGameProto::ActionName actionName, int serviceType, const T &t);
    //发送消息到客户端
    template<typename T>
    int toClientPb(const XGameComm::TPackage &tPackage, const std::string &sCurServrantAddr, XGameProto::ActionName actionName, const T &t, int serviceType, XGameComm::MSGTYPE msgType = XGameComm::MSGTYPE::MSGTYPE_RESPONSE);
};
/////////////////////////////////////////////////////
#endif
