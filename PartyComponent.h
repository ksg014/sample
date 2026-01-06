#pragma once
#include "BaseComponent.h"
#include "Define/DefineErrorCode.h"

class Party;
class PartyComponent : public BaseComponent
{
public:
	PartyComponent(std::shared_ptr<BaseEntity> baseEntity);
	virtual ~PartyComponent();

	void Logout();
	void SetParty(std::shared_ptr<Party> party);
	
	std::shared_ptr<Party> GetParty() { return m_party; } 

public:
	ErrorCode PartyInviteReq(CUID targetCUID, const std::string& targetName);
	ErrorCode PartyTransferLeaderReq(CUID target);
	ErrorCode PartyLeaveReq();
	ErrorCode PartyExpelReq(CUID target);
	ErrorCode PartyJoinAcceptAck(CUID target);

private:
	void Release() override;

private:
	std::shared_ptr<Party> m_party = nullptr;
};

